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

 /************************************************************************************
  *                                 头文件
  ************************************************************************************/
#include "os.h"
#include "at_api.h"
#include "at_parser.h"
#include "app_pub.h"
#include "app_urc_monitor.h"

/************************************************************************************
 *                                 外部函数声明
 ************************************************************************************/
extern at_errno_t NET_MgrCallBack(char *pdata,uint32_t lenOfData);
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define APP_URC_HANDLER_MAX 5

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static osMutex_t g_APP_URC_HandlerMutext = NULL;  //   用于保护 g_APP_URC_Handler
static app_urc_handler g_APP_URC_Handler[APP_URC_HANDLER_MAX] = {0};
/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void APP_URC_MonitorLock(void)
{
    if(g_APP_URC_HandlerMutext != NULL)
    {
        osMutexAcquire(g_APP_URC_HandlerMutext, osWaitForever);
    }
    return;
}

static void APP_URC_MonitorUnLock(void)
{
    if(g_APP_URC_HandlerMutext != NULL)
    {
        osMutexRelease(g_APP_URC_HandlerMutext);
    }
    return;
}

static void APP_URC_CallHandler(const char *cmd, uint32_t cmdLen)
{
    uint32_t i;
    APP_URC_MonitorLock();
    for(i = 0; i< APP_URC_HANDLER_MAX; i++)
    {
        if(g_APP_URC_Handler[i] != NULL)
        {
            g_APP_URC_Handler[i](cmd, cmdLen);
        }
    }
    APP_URC_MonitorUnLock();
}


static int32_t APP_URC_MonitorProc(uint8_t channelId, char* data, uint32_t dataLen)
{
    APP_PRINT_INFO("APP_URC_MonitorProc [%d][%d][%s]\r\n", channelId, dataLen, data);

    NET_MgrCallBack(data,dataLen);
    APP_URC_CallHandler(data, dataLen); //  调用所有的注册的回调函数

    AT_DeviceSendDone(channelId);
    return 0;
}

/* 打开一个AT通道, 用于接收主动上报，所以只要注册主动上报的接口 */
static int32_t APP_URC_MonitorStart(void)
{
    AT_AllocChannel(AT_CHANNEL_ID_MONITOR, NULL, NULL, APP_URC_MonitorProc, AT_CHANNEL_ATTRS_NORBUF);
    AT_OpenChannel(AT_CHANNEL_ID_MONITOR);
    return 0;
}

static int32_t APP_URC_MonitorStop(void)
{
    AT_CloseChannel(AT_CHANNEL_ID_MONITOR);
    AT_FreeChannel(AT_CHANNEL_ID_MONITOR);
    return 0;
}

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief            处理主动上报的AT命令
 *
 * @param[in]       none
 *
 * @return
 ************************************************************************************
*/
int APP_URC_Monitor(void)
{
    g_APP_URC_HandlerMutext = (osMutex_t)osMutexNew(NULL);
    OS_ASSERT(g_APP_URC_HandlerMutext != NULL);

    APP_URC_MonitorStart();
    return 0;
}

/**
 ************************************************************************************
 * @brief           添加处理主动上报的AT命令处理回调
 *
 * @param[in]       主动上报回调处理函数
 *
 * @return   0: 成功   其他: 失败
 ************************************************************************************
*/
int APP_URC_AddHandler(app_urc_handler handler)
{
    uint32_t i;
    APP_URC_MonitorLock();
    //    查找一下是否已经注册
    for(i = 0; i< APP_URC_HANDLER_MAX; i++)
    {
        if(g_APP_URC_Handler[i] == handler)
        {
            break;
        }
    }
    if(i == APP_URC_HANDLER_MAX) //   没找到就添加
    {
        for(i = 0; i< APP_URC_HANDLER_MAX; i++)
        {
            if(g_APP_URC_Handler[i] == NULL)
            {
                g_APP_URC_Handler[i] = handler;
                break;
            }
        }
    }
    APP_URC_MonitorUnLock();

    return i == APP_URC_HANDLER_MAX ? osError : osOK;
}

/**
 ************************************************************************************
 * @brief           删除处理主动上报的AT命令处理回调
 *
 * @param[in]       主动上报回调处理函数
 *
 * @return   0: 成功   其他: 失败
 ************************************************************************************
*/
int APP_URC_DelHandler(app_urc_handler handler)
{
    uint32_t i;

    APP_URC_MonitorLock();
    for(i = 0; i< APP_URC_HANDLER_MAX; i++)
    {
        if(g_APP_URC_Handler[i] == handler)
        {
            g_APP_URC_Handler[i] = NULL;
            break;
        }
    }
    APP_URC_MonitorUnLock();

    return i == APP_URC_HANDLER_MAX ? osError : osOK;
}


