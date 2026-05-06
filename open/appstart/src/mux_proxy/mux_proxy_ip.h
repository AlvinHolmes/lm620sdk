/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-01     ict team          创建
 ************************************************************************************
 */


#ifndef __MUX_PROXY_IP_H__
#define __MUX_PROXY_IP_H__

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

#define MUX_PROXY_IP_MAX_FROM_MUX  8  //   从SPI MUX可以获取的IP包最大个数, 能让SPI MUX定义吗？

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum
{
    MUX_PROXY_IP_MUX_RX = 1,     //   收到MUX数据
    MUX_PROXY_IP_USB_RX ,     //   收到USB数据
} MuxProxy_IP_Command;


typedef struct
{
    uint8_t         num;                    // 包个数
    uint8_t         *data[MUX_PROXY_IP_MAX_FROM_MUX];
    uint32_t        len[MUX_PROXY_IP_MAX_FROM_MUX];
}MuxProxy_IP_EdmaInfo;

typedef struct
{
    //   up 2100 --> 2110
    uint32_t        up_recv;    //  收到的
    uint32_t        up_sent;    //  已经发送的
    uint32_t        up_drop;    //  丢弃的
    //   down 2110 --> 2100
    uint32_t        down_recv;  //  收到的
    uint32_t        down_sent;  //  已经发送的
    uint32_t        down_drop;  //  丢弃的
}MuxProxy_IP_Statistic;


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
int8_t MuxProxy_IP_MuxInit(void);
void MuxProxy_IP_MuxCleanChannel(void);
void MuxProxy_IP_MuxRestartChannel(void);
void MuxProxy_IP_ShowStatistic(void);
void MuxProxy_IP_ClearStatistic(void);


#ifdef __cplusplus
}
#endif
#endif//__MUX_PROXY_IP_H__


