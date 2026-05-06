
#include <stdio.h>
#include "onewire.h"


#include "drv_timer.h"
#include "drv_gpio.h"
#include "drv_pin.h"


//----------------------------------------------------------------
static BmsDecode_t *bmsRxPtr;
static BmsEncode_t *bmsTxPtr;

static osTimerId_t txTimer = NULL;
static osTimerId_t rxTimer = NULL;
//==============================================================================

bool isTransmitting = false;

static void txTimerCallBack(void *argument);
static void rxTimerCallBack(void *argument);
static void initOneWireGPIO(void);
static void setGPIODirection(uint8_t isOutput);
static void setGPIOLevel(uint8_t high);
static uint32_t readGPIOLevel(void);
static void bmsOneWireDecoder(BmsDecode_t *bms);
static void bmsOneWireEncoder(BmsEncode_t *bms);
int32_t bmsOneWireTxInit(BmsEncode_t *bms);
int32_t bmsOneWireRxInit(BmsDecode_t *bms);
int32_t bmsOneWireSendData(uint8_t * data, uint16_t len);
void bmsOneWireTxDeinit(void);
void bmsOneWireRxDeinit(void);


//发送定时器回调函数
static void txTimerCallBack(void *argument)
{
    //确保发送状态有效
    if(!isTransmitting || bmsTxPtr == NULL)
    {
        return;
    } 
    //执行编码和电平控制
    bmsOneWireEncoder(bmsTxPtr);

    //如果仍在发送状态，继续设置下一次定时
    if(isTransmitting)
    {
        //计算下一次中断需要的时间（us）
        uint32_t nextDelay = bmsTxPtr->nextDelay;
        //重新启动定时器（单次触发模式）
        if(nextDelay > 0)
        {
            osStatus_t status = osTimerStart(txTimer, nextDelay);
            if(status != osOK)
            {
                osPrintf("TX timer start failed , status = %d\r\n", status);
                isTransmitting = false;
            }
        }
        else
        {
            //无延时，立即处理
            bmsOneWireEncoder(bmsTxPtr);
        }
    }

}

//接收定时器回调函数
static void rxTimerCallBack(void *argument)
{
    //确保接收状态有效
    if(bmsRxPtr == NULL)
    {
        return ;
    }

    //获取当前时间（us）
    uint32_t currentTime = (uint32_t)osGetSysTimeUs();

    //获取当前GPIO电平状态
    bool currentLevel = readGPIOLevel();

    //检测边沿变化
    if(bmsRxPtr->lastLevel != currentLevel)
    {
        //保存上一次边沿信息
        bmsRxPtr->lastEdge = bmsRxPtr->currentEdge;
        bmsRxPtr->lastTimestamp = bmsRxPtr->currentTimestamp;

        //更新当前边沿信息
        bmsRxPtr->currentEdge = currentLevel ?
                        TIMER_CAPTURE_RISING_EDGE : TIMER_CAPTURE_FALLING_EDGE;
        bmsRxPtr->currentTimestamp = currentTime;
        //更新最后电平状态
        bmsRxPtr->lastLevel = currentLevel;

        //执行解码逻辑
        bmsOneWireDecoder(bmsRxPtr);


    }

}

//GPIO初始化（num、func使用宏定义改一下）
static void initOneWireGPIO(void)
{
    /*使用PD_GPIO_0作为数据传输引脚*/
    PIN_SetMux(ONE_WIRE_GPIO_RES, ONE_WIRE_GPIO_MUX);
    /*初始化为低电平*/
    GPIO_Write(ONE_WIRE_GPIO_RES, GPIO_LOW);
}


//设置GPIO方向
static void setGPIODirection(uint8_t isOutput)
{
    uint8_t dir = isOutput ;
    if(dir == 0)
    {
        GPIO_SetDir(ONE_WIRE_GPIO_RES, GPIO_INPUT);
    }
    if(dir == 1)
    {
        GPIO_SetDir(ONE_WIRE_GPIO_RES, GPIO_OUTPUT);
    }
   
}

//设置GPIO电平
static void setGPIOLevel(uint8_t high)
{
    uint8_t level = high;
    if(level == 0)
    {
        GPIO_Write(ONE_WIRE_GPIO_RES, GPIO_LOW);
    }
    if(level == 1)
    {
        GPIO_Write(ONE_WIRE_GPIO_RES, GPIO_HIGH);
    }
    
}

//读取GPIO电平
static uint32_t readGPIOLevel(void)
{
    return GPIO_Read(ONE_WIRE_GPIO_RES);
}


static void bmsOneWireDecoder(BmsDecode_t *bms)
{
    
    // 1. 计算时间差（微秒）
    uint32_t deltaTime = bms->currentTimestamp - bms->lastTimestamp;
    // osPrintf("state %d, edge %d, Delta: %d us\r\n", bms->state, bms->currentEdge, deltaTime);
    // 2. 备份当前状态（用于错误处理）
    BmsDecodeState_e stateBackup = bms->state;
    
    // 3. 状态机处理
    switch(bms->state) {
        case BMS_DECODE_STATE_INIT:
            // 4. 初始状态检测下降沿
            if (bms->currentEdge == TIMER_CAPTURE_FALLING_EDGE) {
                bms->state = BMS_DECODE_STATE_SYN_LOW;

                //记录低电平开始时间
                bms->synStartTime = bms->currentTimestamp;
                osPrintf("Falling edge detected\r\n");
            }
            break;
            
        case BMS_DECODE_STATE_SYN_LOW:
            
            // 5. 同步低电平后检测上升沿
            if (bms->currentEdge == TIMER_CAPTURE_RISING_EDGE) {
                // 6. 检查低电平时间是否在允许范围内
                if ((deltaTime >= BMS_SYN_L_MIN) && (deltaTime <= BMS_SYN_L_MAX)) {
                    bms->state = BMS_DECODE_STATE_SYN_HIGH;
                }
                else
                {
                    bms->state = BMS_DECODE_STATE_INIT;
                }
            }
            break;
            
        case BMS_DECODE_STATE_SYN_HIGH:
            
            // 7. 同步高电平后检测下降沿
            if (bms->currentEdge == TIMER_CAPTURE_FALLING_EDGE) {
                // 8. 检查高电平时间是否在允许范围内
                if ((deltaTime >= BMS_SYN_H_MIN) && (deltaTime <= BMS_SYN_H_MAX)) {
                    bms->state = BMS_DECODE_STATE_BIT_LOW;
                    bms->totalFrameCnt++;
                    bms->pkg.len = 0;
                    bms->bitCnt = 0;
                    osPrintf( "Sync received, starting frame\r\n");
                }
                else
                {
                    bms->state = BMS_DECODE_STATE_INIT;
                }
            }
            break;
            
        case BMS_DECODE_STATE_BIT_LOW:
            
            // 9. 数据位低电平后检测上升沿
            if (bms->currentEdge == TIMER_CAPTURE_RISING_EDGE) {
                // 10. 检查是否为逻辑1
                if ((deltaTime >= BMS_BIT1_L_MIN) && (deltaTime <= BMS_BIT1_L_MAX)) 
                {
                    bms->state = BMS_DECODE_STATE_BIT_HIGH;
                    bms->bit01Flag = 1;  // 标记为逻辑1
                }
                // 11. 检查是否为逻辑0
                else if ((deltaTime >= BMS_BIT0_L_MIN) && (deltaTime <= BMS_BIT0_L_MAX)) 
                {
                    bms->state = BMS_DECODE_STATE_BIT_HIGH;
                    bms->bit01Flag = 0;  // 标记为逻辑0
                }
                // 12. 检查是否为停止信号
                else if ((deltaTime >= BMS_STOP_L_MIN) && (deltaTime <= BMS_STOP_L_MAX)) 
                {
                    if (bms->pkg.len > 0)
                    {
                        bms->state = BMS_DECODE_STATE_INIT;
                        osPrintf("Frame received, len=%d", bms->pkg.len);
                        // 13. 调用回调函数处理完整帧
                        if (bms->callback != NULL) {
                            bms->callback(bms);
                        }
                        break;
                    }
                }
                else if (deltaTime > BMS_STOP_L_MAX)
                {
                    if(bms->pkg.len > 0)
                    {
                        bms->state = BMS_DECODE_STATE_INIT;
                        if (bms->callback != NULL) {
                            bms->callback(bms);
                        }
                        break;
                    }
                }
                else
                {
                    bms->state = BMS_DECODE_STATE_ERROR;
                }
                
            }
            break;
            
        case BMS_DECODE_STATE_BIT_HIGH:
            
            // 14. 数据位高电平后检测下降沿
            if (bms->currentEdge == TIMER_CAPTURE_FALLING_EDGE) 
            {
                uint8_t bitIdentify = 0;
                
                // 15. 根据之前识别的位类型检查高电平时间
                if (bms->bit01Flag) 
                {  // 逻辑1
                    if ((deltaTime >= BMS_BIT1_H_MIN) && (deltaTime <= BMS_BIT1_H_MAX)) 
                    {
                        // 16. 设置当前位为1
                        bms->pkg.payload[bms->pkg.len] |= (1 << bms->bitCnt);
                        bitIdentify = 1;
                    }
                    else
                    {
                        bms->state = BMS_DECODE_STATE_ERROR;
                    }
                } 
                else 
                {  // 逻辑0
                    if ((deltaTime >= BMS_BIT0_H_MIN) && (deltaTime <= BMS_BIT0_H_MAX)) 
                    {
                        // 17. 设置当前位为0
                        bms->pkg.payload[bms->pkg.len] &= ~(1 << bms->bitCnt);
                        bitIdentify = 1;
                    }
                    else
                    {
                        bms->state = BMS_DECODE_STATE_ERROR;
                    }
                }
                
                // 18. 位识别成功，更新状态
                if (bitIdentify) 
                {
                    bms->bitCnt++;
                    // 19. 检查是否完成一个字节
                    if (bms->bitCnt >= 8) 
                    {
                        bms->bitCnt = 0;
                        bms->pkg.len++;
                        // 20. 防止缓冲区溢出
                        if (bms->pkg.len >= bms->pkg.maxSize) 
                        {
                            bms->pkg.len = bms->pkg.maxSize - 1;
                        }
                    }
                    bms->state = BMS_DECODE_STATE_BIT_LOW;
                }
            }
            break;
            
        default:
            break;
    }
    
    // 21. 错误处理
    if (bms->state == BMS_DECODE_STATE_ERROR) 
    {
        osPrintf("Decoding error! State=%d, Delta=%dus, Len=%d, BitCnt=%d, CurEdge=%d, LastEdge=%d", 
                    stateBackup, deltaTime, bms->pkg.len, bms->bitCnt, 
                    bms->currentEdge, bms->lastEdge);
        bms->state = BMS_DECODE_STATE_INIT;
        bms->errorFrameCnt++;
    }
}



//编码器实现
static void bmsOneWireEncoder(BmsEncode_t *bms)
{

    //状态机，根据当前状态执行不同的操作
    switch(bms->state)
    {
        //同步状态
        case BMS_ENCODE_STATE_SYN:
            if(bms->synStopState == 0)
            {
                //设置GPIO为低电平（开始同步信号的低电平部分）
                setGPIOLevel(0);

                //更新同步状态到下一步
                bms->synStopState = 1;
                bms->nextDelay = BMS_SYN_L_STD;
            }
            else if (bms->synStopState == 1)
            {
                //设置GPIO为高电平（同步信号低电平结束，高电平开始）
                setGPIOLevel(1);
                //更新同步状态到下一步
                bms->synStopState = 2;

                bms->nextDelay = BMS_SYN_H_STD;
            }
            else{
                setGPIOLevel(0);
                //同步头信号发送完成， 切换到数据发送状态
                bms->state = BMS_ENCODE_STATE_DATA;

                //重置同步状态变量
                bms->synStopState = 0;

                //初始化位计数器
                bms->bitCnt = 0;

                //初始化字节计数器
                bms->txCnt = 0;

                //加载第一个字节数据
                bms->currentByte = bms->payload[0];
                bms->currentBitPos = 0;
                bms->currentBitState = 0;

                //立刻起下一次定时器中断
                bms->nextDelay = 1;

            }
            break;
        
        case BMS_ENCODE_STATE_DATA:
            if(bms->currentBitState == 0)
            {
                bms->currentBitValue = (bms->currentByte >> bms->currentBitPos) & 0x01;
                //设置GPIO为低电平（开始数据位的低电平部分）
                setGPIOLevel(0);

                bms->nextDelay = bms->currentBitValue ?
                        BMS_BIT1_L_STD : BMS_BIT0_L_STD;
                bms->currentBitState = 1;
            }
            else
            {
                //设置GPIO为高电平（数据位低电平结束，高电平开始）
                setGPIOLevel(1);
                //计算高电平时间（总周期减去低电平时间）
                uint32_t lowTime = bms->currentBitValue ? 
                            BMS_BIT1_L_STD : BMS_BIT0_L_STD;
                bms->nextDelay = BMS_BIT_PERIOD - lowTime;
                //准备处理下一位，右移字节，获取下一位

                bms->currentBitPos++;


                //检查是否完成一个字节（8bit）
                if(bms->currentBitPos >= 8)
                {
                    //重置位计数器
                    bms->currentBitPos = 0;
                    //增加字节计数器
                    bms->txCnt++;

                    //检查是否完成所有字节
                    if(bms->txCnt >= bms->txLen)
                    {
                        //所有数据发送完成，切换到停止状态
                        bms->state = BMS_ENCODE_STATE_STOP;
                        bms->synStopState = 0;
                    }
                    else
                    {
                        //加载下一个字节
                        bms->currentByte = bms->payload[bms->txCnt];
                    }
                }
                bms->currentBitState = 0;
            }
            break;
        case BMS_ENCODE_STATE_STOP:
            if(bms->synStopState == 0)
            {
                //设置GPIO为低电平（开始停止信号）
                setGPIOLevel(0);
                //更新停止状态
                bms->synStopState = 1;
                bms->nextDelay = BMS_STOP_L_STD;
            }
            else
            {
                //设置GPIO为高电平（开始停止信号）
                setGPIOLevel(1);

                //清除发送状态
                isTransmitting = false;

                //如果有回调，执行回调（通知发送完成）
                if(bms->callback != NULL)
                {
                    bms->callback();
                }             

                //回到空闲状态
                bms->state = BMS_ENCODE_STATE_IDLE;
                bms->nextDelay = 1;



                //重置停止状态
                bms->synStopState = 0;
            }
            break;
        case BMS_ENCODE_STATE_IDLE:
            if(bms->synStopState == 0)
            {
                setGPIOLevel(0);
            }
            break;
        default:
            break;

    }

}


// 发送初始化
int32_t bmsOneWireTxInit(BmsEncode_t *bms)
{
    // 1. 检查定时器是否已创建
    if (txTimer != NULL) 
    {
        osPrintf("TX timer already initialized\r\n");
        return -1;
    }
    
    // 2. 保存编码器指针
    bmsTxPtr = bms;
    bms->state = BMS_ENCODE_STATE_IDLE;
    
    // 3. 创建发送定时器属性
    osTimerAttr_t txTimerAttr = {
        .name = "OneWireTxTimer",
        .attr_bits = osTimerHrtimer,
        .cb_mem = NULL,
        .cb_size = 0,
    };
    
    // 4. 创建发送定时器
    txTimer = osTimerNew(txTimerCallBack, osTimerOnce, NULL, &txTimerAttr);
    if (txTimer == NULL) {
        osPrintf("Failed to create TX timer\r\n");
        return -1;
    }

    //5、配置GPIO为输出模式
    initOneWireGPIO();
    setGPIODirection(1);


    osPrintf("OneWire TX initialized successfully\r\n");

    return 0;
}

//接收初始化
int32_t bmsOneWireRxInit(BmsDecode_t *bms)
{
   // 1. 检查解码器参数有效性
    if (bms->pkg.payload == NULL || bms->pkg.maxSize < 4) {
        osPrintf("Invalid buffer for RX\r\n");
        return -1;
    }
    
    // 2. 检查定时器是否已创建
    if (rxTimer != NULL) {
        osPrintf("RX timer already initialized\r\n");
        return -1;
    }
    
    // 3. 保存解码器指针
    bmsRxPtr = bms;
    bms->state = BMS_DECODE_STATE_INIT;
    
    // 4. 初始化边沿检测相关变量
    bms->lastTimestamp = 0;
    bms->lastEdge = 0;
    bms->currentTimestamp = 0;
    bms->currentEdge = 0;

    bms->synStartTime = 0;
    bms->synHighStart = 0;
    
    // 5. 初始化最后电平状态
    bms->lastLevel = readGPIOLevel();
    
    // 6. 创建接收定时器属性
    osTimerAttr_t rxTimerAttr = {
        .name = "OneWireRxTimer",
        .attr_bits = osTimerHrtimer,
        .cb_mem = NULL,
        .cb_size = 0,
    };
    
    // 7. 创建接收定时器（周期性模式）
    rxTimer = osTimerNew(rxTimerCallBack, osTimerPeriodic, NULL, &rxTimerAttr);
    if (rxTimer == NULL) {
        osPrintf("Failed to create RX timer\r\n");
        return -1;
    }
    
    // 8. 配置GPIO为输入模式
    // initOneWireGPIO();
    PIN_SetMux(ONE_WIRE_GPIO_RES, ONE_WIRE_GPIO_MUX);
    PIN_SetPull(ONE_WIRE_GPIO_RES, PULL_UP);
    setGPIODirection(0);
    
    
    // 9. 启动接收定时器（50μs采样周期）
    osStatus_t status = osTimerStart(rxTimer, 50);  // 50微秒
    if (status != osOK) {
        osPrintf("Failed to start RX timer, status=%d\r\n", status);
        osTimerDelete(rxTimer);
        rxTimer = NULL;
        return -1;
    }
    
    
    // 10. 记录初始化成功日志
    osPrintf("OneWire RX initialized successfully\r\n");
    return 0;

}



//发送数据
int32_t bmsOneWireSendData(uint8_t * data, uint16_t len)
{
   // 1. 检查发送状态和参数有效性
    if (isTransmitting) {
        osPrintf("Transmission already in progress\r\n");
        return -1;
    }
    if (data == NULL || len == 0) {
        osPrintf("Invalid data or length\r\n");
        return -1;
    }
    if (txTimer == NULL) {
        osPrintf("TX timer not initialized\r\n");
        return -1;
    }
    
    // 2. 设置发送参数
    bmsTxPtr->payload = data;
    bmsTxPtr->txLen = len;
    bmsTxPtr->synStopState = 0;
    bmsTxPtr->state = BMS_ENCODE_STATE_SYN;
    isTransmitting = true;

    // bmsTxPtr->nextDelay = BMS_SYN_L_STD;
    
    // 3. 配置GPIO为输出模式
    setGPIODirection(1);
    
    // 4. 启动定时器（1μs后开始）
    osStatus_t status = osTimerStart(txTimer, 1);  // 1微秒
    if (status != osOK) {
        osPrintf("Failed to start TX timer, status=%d\r\n", status);
        isTransmitting = false;
        return -1;
    }
    
    // 5. 记录发送开始日志
    osPrintf("Starting data transmission, len=%d\r\n", len);
    return 0;
}



void bmsOneWireTxDeinit(void)
{
    // 1. 停止发送定时器
    if (txTimer != NULL) {
        osTimerStop(txTimer);
    }
    
    // 2. 删除发送定时器
    if (txTimer != NULL) {
        osTimerDelete(txTimer);
        txTimer = NULL;
    }
    
    // 3. 重置发送状态
    isTransmitting = false;
    bmsTxPtr = NULL;
    
    // osPrintf("callback count %d\r\n", count);
    // 4. 记录反初始化日志
    osPrintf("OneWire TX deinitialized\r\n");
}

void bmsOneWireRxDeinit(void)
{
    // 1. 停止接收定时器
    if (rxTimer != NULL) {
        osTimerStop(rxTimer);
    }
    
    // 2. 删除接收定时器
    if (rxTimer != NULL) {
        osTimerDelete(rxTimer);
        rxTimer = NULL;
    }
    
    // 3. 重置接收指针
    bmsRxPtr = NULL;

    
    // 4. 记录反初始化日志
    osPrintf("OneWire RX deinitialized\r\n");
}
