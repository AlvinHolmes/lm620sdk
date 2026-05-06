#include "onewire.h"
#include "os.h"
#include "nr_micro_shell.h"
#include "stdbool.h"
#include "drv_gpio.h"
#include "drv_pin.h"

// ======================== 全局变量 ========================
static osSemaphoreId_t rxSemaphore = NULL;  // 接收完成信号量
static uint8_t receivedData[24];            // 接收缓冲区
static uint16_t receivedLength = 0;         // 接收数据长度
extern bool isTransmitting;
uint8_t testFrame[24] = {0};

static void receiveCallback(void *arg)
{
    BmsDecode_t *bms = (BmsDecode_t *)arg;
    // 1. 复制接收到的数据
    if (bms->pkg.len > 0 && bms->pkg.len <= sizeof(receivedData)) 
    {
        memcpy(receivedData, bms->pkg.payload, bms->pkg.len);
        receivedLength = bms->pkg.len;
        
        // 2. 释放信号量通知主线程
        if (rxSemaphore != NULL) 
        {
            osSemaphoreRelease(rxSemaphore);
        }
    }
}


static void buildTestFrame(uint8_t *frame)
{
    // 协议规定的24字节数据帧结构
    memset(frame, 0, 24);
    
    // 填充固定字段 (参考协议文档)
    frame[0] = 0xDD;   // 设备编码
    frame[1] = 0x00;   // 版本号
    frame[2] = 0x13;     // 数据长度
    frame[3] = 0x12;
    frame[4] = 0x34;
    
    // 示例数据 (控制器状态)
    frame[5] = 0x01;   // 电压 (36V -> 360 = 0x0168)
    frame[6] = 0xe0;
    frame[7] = 0x00;   // 电流 (1.0A)
    frame[8] = 0x64;
    frame[9] = 0x00;
    frame[10] = 0xff;
    frame[11] = 0x03;
    frame[12] = 0xe8;
    frame[13] = 0x12;  // 状态: MOS管故障
    frame[14] = 0x00;
    
    // 电池信息
    frame[15] = 0x03;  // 锰酸锂电池
    frame[16] = 0x30;    // 电压等级36V
    frame[17] = 0x14;    // 容量20AH
    frame[18] = 0x50;    // SOC 80%
    frame[19] = 0x00;
    frame[20] = 0x00;
    frame[21] = 0xaa;
    frame[22] = 0x05;
    frame[23] = 0xb8;
    
}



// 发送测试命令
void testOneWireSend(char argc, char **argv)
{

    static BmsEncode_t encoder;

    if (bmsOneWireTxInit(&encoder) != 0) 
    {
        osPrintf("TX init failed!\r\n");
        return;
    }
    


    buildTestFrame(testFrame);
    

    if (bmsOneWireSendData(testFrame, sizeof(testFrame))) 
    {
        osPrintf("Send failed!\r\n");
    } 
    else 
    {
        osPrintf("Sending 24-byte frame...\r\n");
        
      
        osTick_t startTick = osGetSysTimeUs();
        uint32_t timeoutOccurred = 0;
        
        while (isTransmitting) 
        {
            // 计算经过的毫秒数
            osTick_t currentTick = osGetSysTimeUs();
            uint64_t elapsedMs = currentTick - startTick;
            
            
            // 检查是否超时
            if (elapsedMs >= 500000) 
            {
                osPrintf("Send timeout!\r\n");
                timeoutOccurred = 1;
                break;
            }
            

            osDelay(osTickFromMs(1));
        }
        
        if (!timeoutOccurred) 
        {
            osPrintf("Send complete!\r\n");
        }
    }
    

    // bmsOneWireTxDeinit();
}
NR_SHELL_CMD_EXPORT(onesend, testOneWireSend);


// 接收测试命令
void testOneWireReceive(char argc, char **argv)
{

    if (rxSemaphore == NULL) 
    {
        osSemaphoreAttr_t attr = { .name = "RxSem" };
        rxSemaphore = osSemaphoreNew(1, 0, &attr);
    }
    

    static uint8_t rxBuffer[32];
    static BmsDecode_t decoder = 
    {
        .callback = receiveCallback,
        .pkg = { .payload = rxBuffer, .maxSize = sizeof(rxBuffer) }
    };
    
    if (bmsOneWireRxInit(&decoder) != 0) 
    {
        osPrintf("RX init failed!\r\n");
        return;
    }
    
    osPrintf("Waiting for data...\r\n");
    receivedLength = 0;  // 重置长度
    
    osStatus_t status = osSemaphoreAcquire(rxSemaphore, osWaitForever);
    // bmsOneWireRxDeinit(); 
    

    if (status == osOK) 
    {
        osPrintf("Received %d bytes:\r\n", receivedLength);
        for (int i = 0; i < receivedLength; i++) {
            osPrintf("%02X ", receivedData[i]);
            if ((i + 1) % 8 == 0) osPrintf("\r\n");
        }
        osPrintf("\r\n");
    } 
    else 
    {
        osPrintf("Receive timeout!\r\n");
    }


        if (receivedLength >= 24) 
        {
            osPrintf("Parsed data:\r\n");
            osPrintf("Device ID: 0x%02X\r\n", receivedData[0]);
            osPrintf("Version: %d\r\n", receivedData[1]);
            
            // 电压 = (高字节 << 8) | 低字节
            uint16_t voltage = (receivedData[5] << 8) | receivedData[6];
            osPrintf("Voltage: %.1fV\r\n", voltage / 10.0);
            
            // 电流 = (高字节 << 8) | 低字节
            uint16_t current = (receivedData[7] << 8) | receivedData[8];
            osPrintf("Current: %.1fA\r\n", current / 10.0);
            
            osPrintf("Battery SOC: %d%%\r\n", receivedData[18]);
        }
}
NR_SHELL_CMD_EXPORT(onerecv, testOneWireReceive);


