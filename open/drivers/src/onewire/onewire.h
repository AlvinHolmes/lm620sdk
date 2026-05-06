#ifndef  BMS_ONEWIRE_H
#define  BMS_ONEWIRE_H
#include "os.h"

//波形时序定义,单位:1us
#define BMS_SYN_L_STD                    (10000)//同步信号低电平脉宽
#define BMS_SYN_L_RANGE                  (1500)//接收允许同步信号低电平脉宽的误差
#define BMS_SYN_L_MAX                    (BMS_SYN_L_STD+BMS_SYN_L_RANGE)//上限脉宽
#define BMS_SYN_L_MIN                    (BMS_SYN_L_STD-BMS_SYN_L_RANGE)//下限脉宽

#define BMS_SYN_H_STD                    (1000)//同步信号高电平脉宽
#define BMS_SYN_H_RANGE                  (150)
#define BMS_SYN_H_MAX                    (BMS_SYN_H_STD+BMS_SYN_H_RANGE)
#define BMS_SYN_H_MIN                    (BMS_SYN_H_STD-BMS_SYN_H_RANGE)

#define BMS_STOP_L_STD                   (5000)//停止信号低电平脉宽
#define BMS_STOP_L_RANGE                 (750)
#define BMS_STOP_L_MAX                   (BMS_STOP_L_STD+BMS_STOP_L_RANGE)
#define BMS_STOP_L_MIN                   (BMS_STOP_L_STD-BMS_STOP_L_RANGE)
#define BMS_STOP_H_MIN                   (50000)//停止信号高电平脉宽

#define BMS_BIT_PERIOD                   (2000)//数据位的周期
#define BMS_BIT1_L_STD                   (500)//逻辑1低电平脉宽
#define BMS_BIT1_L_RANGE                 (75)
#define BMS_BIT1_H_RANGE                 (225)
#define BMS_BIT1_L_MAX                   (BMS_BIT1_L_STD+BMS_BIT1_L_RANGE)
#define BMS_BIT1_L_MIN                   (BMS_BIT1_L_STD-BMS_BIT1_L_RANGE)
#define BMS_BIT1_H_MAX                   (BMS_BIT_PERIOD-BMS_BIT1_L_STD+BMS_BIT1_H_RANGE)
#define BMS_BIT1_H_MIN                   (BMS_BIT_PERIOD-BMS_BIT1_L_STD-BMS_BIT1_H_RANGE)

#define BMS_BIT0_L_STD                   (1500)//逻辑0低电平脉宽
#define BMS_BIT0_L_RANGE                 (225)
#define BMS_BIT0_H_RANGE                 (75)
#define BMS_BIT0_L_MAX                   (BMS_BIT0_L_STD+BMS_BIT0_L_RANGE)
#define BMS_BIT0_L_MIN                   (BMS_BIT0_L_STD-BMS_BIT0_L_RANGE)
#define BMS_BIT0_H_MAX                   (BMS_BIT_PERIOD-BMS_BIT0_L_STD+BMS_BIT0_H_RANGE)
#define BMS_BIT0_H_MIN                   (BMS_BIT_PERIOD-BMS_BIT0_L_STD-BMS_BIT0_H_RANGE)

//协议时间定义
#define TIMER_CAPTURE_RISING_EDGE       1
#define TIMER_CAPTURE_FALLING_EDGE      0

#define US_PER_MS                       1000

#define BOARD_PIN_RES_PRE_EXPAND(pin)           (&g_PIN_##pin##_Res)
#define PIN_MUX_GPIO_PRE_EXPAND(pin)            (PIN_##pin##_MUX_GPIO)
#define BOARD_PIN_RES(pin)                      BOARD_PIN_RES_PRE_EXPAND(pin)
#define PIN_MUX_GPIO(pin)                       PIN_MUX_GPIO_PRE_EXPAND(pin)


#define ONE_WIRE_GPIO                           PIN_21
#define ONE_WIRE_GPIO_RES                       BOARD_PIN_RES(ONE_WIRE_GPIO)
#define ONE_WIRE_GPIO_MUX                       PIN_MUX_GPIO(ONE_WIRE_GPIO)

typedef enum
{
    BMS_DECODE_STATE_INIT = 0U, //初始状态
    BMS_DECODE_STATE_SYN_LOW,   //同步低电平状态
    BMS_DECODE_STATE_SYN_HIGH,  //同步高电平状态
    BMS_DECODE_STATE_BIT_LOW,   //数据位低电平状态
    BMS_DECODE_STATE_BIT_HIGH,  //数据位高电平状态
    BMS_DECODE_STATE_ERROR,     //错误状态

} BmsDecodeState_e;

typedef struct
{
    uint8_t  *payload;      //数据缓冲区指针
    uint32_t maxSize;       //缓冲区最大容量
    uint32_t len;           //当前数据长度

}BmsPkg_t;

typedef void ( *BmsDecodeCb_t )(void *);    //解码完成回调

//解码结构体
typedef struct
{
    BmsDecodeState_e state;                    //当前解码状态

    uint32_t lastTimestamp;                     //上一次边沿时间戳（us）
    uint8_t lastEdge;                           //上一次边沿类型
    uint32_t currentTimestamp;                  //当前边沿时间戳（us）
    uint8_t currentEdge;                        //当前边沿类型
    uint8_t lastLevel;                          //上一次采样电平

    BmsDecodeCb_t callback;                     //数据接收完成回调
    BmsPkg_t pkg;                               //数据包
    uint32_t errorFrameCnt;                     //错误帧计数
    uint32_t totalFrameCnt;                     //总接收帧计数
    uint8_t  bitCnt;                            //当前字节位计数（0-7）
    uint8_t  bit01Flag;                         //当前值（0/1）

    uint32_t synStartTime;                      //同步头低电平起始时间
    uint32_t synHighStart;                      //同步头高电平起始时间

}BmsDecode_t;

typedef enum
{
    BMS_ENCODE_STATE_SYN = 0U,  //同步头状态
    BMS_ENCODE_STATE_DATA,      //数据发送状态
    BMS_ENCODE_STATE_STOP,      //停止信号状态
    BMS_ENCODE_STATE_IDLE,      //空闲状态

} BmsEncodeState_e;

typedef void ( *BmsEncodeCb_t )(void);//编码完成回调

typedef struct
{
    BmsEncodeState_e state;         //当前编码状态
    BmsEncodeCb_t callback;         //发送完成回调
    uint8_t  *payload;              //待发送数据指针
    uint16_t txLen;                 //待发送数据长度
    uint16_t txCnt;                 //已发送字节计数
    uint8_t  bitCnt;                //当前字节位计数
    uint8_t  synStopState;          //同步/停止状态子状态
    uint32_t nextDelay;             //下一个状态延时(us)
    uint8_t currentBitValue;
    uint8_t currentByte;
    uint8_t currentBitPos;
    uint8_t currentBitState;
}BmsEncode_t;


int32_t bmsOneWireRxInit(BmsDecode_t *bms);
int32_t bmsOneWireTxInit(BmsEncode_t *bms);
int32_t bmsOneWireSendData(uint8_t * data, uint16_t len);
void bmsOneWireRxDeinit(void);
void bmsOneWireTxDeinit(void);
#endif /* BMS_ONEWIRE_H */

