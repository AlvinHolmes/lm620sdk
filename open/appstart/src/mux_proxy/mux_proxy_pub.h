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

#ifndef __MUX_PROXY_PUB_H__
#define __MUX_PROXY_PUB_H__

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdint.h>
#include "slog_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define MUX_PROXY_CONTROL_TASK_STACK_SIZE 1024
#define MUX_PROXY_CONTROL_TASK_PRIORITY 11
#define MUX_PROXY_CONTROL_BUSMUX_CHANNEL_PRIORITY 0 // 最高优先级

#define MUX_PROXY_IP_TASK_STACK_SIZE 1024
#define MUX_PROXY_IP_TASK_PRIORITY 13
#define MUX_PROXY_IP_BUSMUX_CHANNEL_PRIORITY 2   //   第三优先级

#define MUX_PROXY_AT_TASK_STACK_SIZE 1024
#define MUX_PROXY_AT_TASK_PRIORITY 12
#define MUX_PROXY_AT_BUSMUX_CHANNEL_PRIORITY 1   //   第二优先级

#define MUX_PROXY_LOG_BUSMUX_CHANNEL_PRIORITY 3  //   最四优先级

#define MUX_PROXY_RAMDUMP_BUSMUX_CHANNEL_PRIORITY 4

#if 1
//#define APP_MUX_PRINT_INFO(format, ...)   slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_MUX, format, ##__VA_ARGS__)
#define APP_MUX_PRINT_INFO(format, ...)   slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_MUX, format, ##__VA_ARGS__)
#define APP_MUX_PRINT_ERROR(format, ...)  slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_MUX, format, ##__VA_ARGS__)
//#define APP_MUX_PRINT_WARN(format, ...)  slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_MUX, format, ##__VA_ARGS__)
#else
#define APP_MUX_PRINT_INFO(format, ...)   osPrintf(format, ##__VA_ARGS__)
#define APP_MUX_PRINT_ERROR(format, ...)  osPrintf(format, ##__VA_ARGS__)
#endif

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct
{
    uint8_t msgId;
    uint32_t msgDataLen;
    void *msgData;
} MuxProxy_Message;

typedef enum
{
    MUX_PROXY_CHANNEL_CONTROL = 0,
    MUX_PROXY_CHANNEL_AT = 1,
    MUX_PROXY_CHANNEL_IP = 2,
    MUX_PROXY_CHANNEL_LOG = 3,
    MUX_PROXY_CHANNEL_PASS_THROUGH = 4,
} MuxProxy_ChannelDef;

typedef enum
{
    MUX_PROXY_CHANNEL_CLOSE = 0,
    MUX_PROXY_CHANNEL_OPEN = 1,
} MuxProxy_ChannelStatus;

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
BUS_MuxChannel *MuxProxy_InsertBusMuxChannel(uint8_t id, const char *name, uint8_t priority, BUS_EventFunc callback, void *param);

#ifdef __cplusplus
}
#endif
#endif//__MUX_PROXY_PUB_H__


