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
#define AT_PROXY_SUPPORT    1
/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "bus_mux_api.h"
#if AT_PROXY_SUPPORT
#include "at_api.h"
#else
#define    AT_DeviceSendDone(a)
#define    AT_DeviceReceived(a, b, c)
#define    AT_AllocChannel(a, b, c, d, e)
#define    AT_OpenChannel(a)
#define    AT_CloseChannel(a)
#define    AT_FreeChannel(a)
#endif
#include "mux_proxy_api.h"
#include "mux_proxy_pub.h"
#include "mux_proxy_control.h"
#include "mux_proxy_at.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
static void MuxProxy_AT_MuxDevCB(BUS_MuxChannel *chl, BUS_EventMsg *msg, void* data, uint32_t event);
static int32_t MuxProxy_AT_MuxSend(uint8_t channelId, char* data, uint32_t dataLen);
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static BUS_MuxChannel *g_MuxProxy_AT_MuxChl = NULL;
/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
//    MUX发送完数据后回调,在这里释放内存
static void MuxProxy_AT_MuxSendCB(BUS_EventMsg *eventMsg)
{
    OS_ASSERT(eventMsg->num == 1);
    AT_DeviceSendDone(AT_CHANNEL_ID_MUX); //   通知AT框架发送完成，AT框架发送是串行的，因此这里只会有一个数据
    return;
}

//    MUX收到数据后发送给AT框架，这里会发生数据copy
static void MuxProxy_AT_MuxReceiveCB(BUS_EventMsg *eventMsg)
{
    uint16_t i;
    for(i = 0; i < eventMsg->num; i++) //   一次收到多个数据
    {
        AT_DeviceReceived(AT_CHANNEL_ID_MUX, (char *)eventMsg->data[i], eventMsg->len[i]);
        #if AT_PROXY_SUPPORT == 0
        char *resp = "\r\nNot Support\r\n";
        MuxProxy_AT_MuxSend(0, resp, strlen(resp));
        #endif
    }
    // 释放内存
    osFree(eventMsg->addr);

    return;
}

//    MUX发送数据失败后回调
void MuxProxy_AT_MuxErrorCB(BUS_EventMsg *eventMsg, uint32_t event)
{
    APP_MUX_PRINT_INFO("MuxProxy_AT_MuxErrorCB event[%d]\r\n", event);
    if(event == BUS_CHANNEL_TRANS_ERROR_EVENT)
    {
        //   数据发送失败后,也要释放内存
        OS_ASSERT(eventMsg->num <= 1);
        if(eventMsg->num == 1)
        {
            AT_DeviceSendDone(AT_CHANNEL_ID_MUX); //   通知AT框架发送完成，AT框架发送是串行的，因此这里只会有一个数据
        }
        MuxProxy_Control_MuxDown();
    }
    return;
}

void MuxProxy_AT_MuxCleanChannel(void)
{
    BUS_EventMsg msg;
    uint32_t remain = 0;
    BUS_MuxErrCode err;

    err = BUS_MuxRelease(g_MuxProxy_AT_MuxChl, &msg, &remain);

    while (err == BUSMUX_OK) {
        OS_ASSERT(msg.num <= 1); //   最多只有一个数据缓存
        if(msg.num != 0)
        {
            AT_DeviceSendDone(AT_CHANNEL_ID_MUX);
        }
        err = BUS_MuxRelease(g_MuxProxy_AT_MuxChl, &msg, &remain);
    }
    APP_MUX_PRINT_INFO("MuxProxy_AT_MuxCleanChannel\r\n");
}

void MuxProxy_AT_MuxRestartChannel(void)
{
    BUS_MuxStart(g_MuxProxy_AT_MuxChl);//  重新恢复通道
    APP_MUX_PRINT_INFO("MuxProxy_AT_MuxRestartChannel\r\n");
}


//   提供给MUX设备的回调函数，处理收到数据通知和数据发送完通知
static void MuxProxy_AT_MuxDevCB(BUS_MuxChannel *chl, BUS_EventMsg *msg, void *data, uint32_t event)
{
    (void)chl;
    (void)data;

    if(event == BUS_CHANNEL_TRANS_END_EVENT) //   发送完成,释放内存
    {
        MuxProxy_AT_MuxSendCB(msg); // data send done
    }
    else if(event == BUS_CHANNEL_DATA_IN_EVENT) //   收到新数据,转发消息给任务
    {
        MuxProxy_AT_MuxReceiveCB(msg); // receive data
    }
    else
    {
        MuxProxy_AT_MuxErrorCB(msg, event);
    }

    return;
}

//   向MUX发送数据
static int32_t MuxProxy_AT_MuxSend(uint8_t channelId, char* data, uint32_t dataLen)
{
    BUS_MuxErrCode err;
    //   由BUS Mux执行clean cache
    err = BUS_MuxSend(g_MuxProxy_AT_MuxChl, (uint8_t *)data, dataLen, true);
    if(err != BUSMUX_OK)
    {
        APP_MUX_PRINT_ERROR("MuxProxy_AT_MuxSend Fail %d\r\n", err);
        AT_DeviceSendDone(channelId); //   通知AT框架释放内存
    }
    return osOK;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
int32_t MuxProxy_AT_MuxInit(void)
{
    //  打开MUX channel
    APP_MUX_PRINT_INFO("MuxProxy_AT_MuxInit\r\n");
    if(g_MuxProxy_AT_MuxChl == NULL)
    {
        g_MuxProxy_AT_MuxChl = MuxProxy_InsertBusMuxChannel(MUX_PROXY_CHANNEL_AT, "at", MUX_PROXY_AT_BUSMUX_CHANNEL_PRIORITY, MuxProxy_AT_MuxDevCB, NULL);
    }
    return osOK;
}

int32_t MuxProxy_AT_ChannelInit(void)
{
    AT_AllocChannel(AT_CHANNEL_ID_MUX, NULL, MuxProxy_AT_MuxSend, MuxProxy_AT_MuxSend, AT_CHANNEL_ATTRS_TIMER);
    AT_OpenChannel(AT_CHANNEL_ID_MUX);

    return osOK;
}

int32_t MuxProxy_AT_ChannelDeInit(void)
{
    AT_CloseChannel(AT_CHANNEL_ID_MUX);
    AT_FreeChannel(AT_CHANNEL_ID_MUX);

    return osOK;
}


