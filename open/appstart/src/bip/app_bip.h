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

#ifndef __APP_BIP_H__
#define __APP_BIP_H__

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "slog_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

#if 1
#define BIP_PRINT_DEBUG(format, ...)  slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_APP, "[APP_BIP]"format, ##__VA_ARGS__)
#define BIP_PRINT_INFO(format, ...)   slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_APP, "[APP_BIP]"format, ##__VA_ARGS__)
#define BIP_PRINT_WARN(format, ...)   slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_APP, "[APP_BIP]"format, ##__VA_ARGS__)
#define BIP_PRINT_ERROR(format, ...)  slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_APP, "[APP_BIP]"format, ##__VA_ARGS__)
#else
#define BIP_PRINT_DEBUG(format, ...)   osPrintf("[DEBUG]""[APP_BIP]"format, ##__VA_ARGS__)
#define BIP_PRINT_INFO(format, ...)  osPrintf("[INFO]""[APP_BIP]"format, ##__VA_ARGS__)
#define BIP_PRINT_WARN(format, ...)   osPrintf("[WARN]""[APP_BIP]"format, ##__VA_ARGS__)
#define BIP_PRINT_ERROR(format, ...)   osPrintf("[ERROR]""[APP_BIP]"format, ##__VA_ARGS__)
#endif


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

typedef struct
{
    osSlist_t node;
    uint8_t *data;
    uint16_t usedSize;
    uint16_t totalSize;
    uint8_t  status; //  APP_BIP_DATA_STATUS
}APP_BIP_Data;

typedef enum {
    APP_BIP_DATA_IDLE = 0,  //数据没有任何处理
    APP_BIP_DATA_NOTIFY,    //数据已经通知USIM
    APP_BIP_DATA_Sending,   //数据正在发送
}APP_BIP_DATA_STATUS;

typedef enum{
    APP_BIP_USAT_PROACTIVE_COMMAND, // uicc上报主动式命令
    APP_BIP_USAT_END, // uicc上报end命令,一次会话结束
    APP_BIP_NET_RECEIVE_DATA, //  socket收到数据
    APP_BIP_NET_SENT_DATA, // socket数据发送完成
    APP_BIP_NET_DISCONNECT, // socket断开连接
    APP_BIP_NET_ERROR, // socket出错
    APP_BIP_OFF_WORK,  //   任务退出
} APP_BIP_EVENT;
/**
 ************************************************************************************
 *@brief                         事件消息内容
 ************************************************************************************
*/
typedef struct
{
    uint16_t          eventID;  //   事件ID，参考APP_BIP_EVENT
    void             *ptrParam;  //   事件参数
} APP_BIP_Event_Info;

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief   初始化需要的资源
 *
 * @param[in] none
 *
 * @return
 * @note
 ************************************************************************************
*/
void APP_BIP_Init(void);
/**
 ************************************************************************************
 * @brief 发消息
 *
 * @param[in]    event    事件
 * @param[in]    timeout 超时   osWaitForever or osNoWait
 *
 * @return
 * @note
 ************************************************************************************
*/
int APP_BIP_EventSend(APP_BIP_Event_Info event, uint32_t timeout);



#ifdef __cplusplus
}
#endif
#endif//__APP_BIP_H__


