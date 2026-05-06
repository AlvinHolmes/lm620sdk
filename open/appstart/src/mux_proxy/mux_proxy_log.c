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
#include "bus_mux_api.h"
#include "mux_proxy_pub.h"
#include "mux_proxy_log.h"
#include "comm.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static BUS_MuxChannel *g_MuxProxy_Log_MuxChl = NULL;
static osMessageQueueId_t g_muxProxyLogReadQueueId = NULL;

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
#if defined(ENABLE_SPI_SEND_LOG)
void MuxProxy_Log_ReadMsg(BUS_EventMsg *eventMsg)
{
    osStatus_t status;
    OS_ASSERT(NULL != eventMsg);
    memset(eventMsg, 0, sizeof(BUS_EventMsg));
    
    if (LOG_CHANNEL_SPI != g_commHandle.logChannel)
    {
        return;
    }

    while (g_muxProxyLogReadQueueId == NULL)
    {
        osThreadMsSleep(100);
    }

    if ((status = osMessageQueueGet(g_muxProxyLogReadQueueId, eventMsg, 0, osWaitForever)) == osOK)
    {
        // osPrintf("get msg, num:%d\r\n", eventMsg->num);
        g_commHandle.spiRecvProcCount += eventMsg->num;
        return;
    }
    else
    {
        // osPrintf("fail get msg, num:%d, ret:%d\r\n", eventMsg->num, status);
        eventMsg->num = 0;
    }
}

void MuxProxy_Log_FreeReadMsg(BUS_EventMsg *eventMsg)
{
    if (LOG_CHANNEL_SPI != g_commHandle.logChannel)
    {
        return;
    }
    
    OS_ASSERT(NULL != eventMsg);
    // osPrintf("free get msg, num:%d\r\n", eventMsg->num);
    if (eventMsg->addr)
    {
        osFree(eventMsg->addr);
    }
}
#endif

static void MuxProxy_Log_MuxReceive(BUS_EventMsg *eventMsg)
{
#if defined(ENABLE_SPI_SEND_LOG)
    if (LOG_CHANNEL_SPI != g_commHandle.logChannel)
    {
        osFree(eventMsg->addr);
        return;
    }
    
    if(osMessageQueuePut(g_muxProxyLogReadQueueId, eventMsg, 0, 0) != osOK)
    {
        // osPrintf("%s send fail\r\n", __FUNCTION__);
        g_commHandle.spiRecvDropCount += eventMsg->num;
        osFree(eventMsg->addr);
    }
    else
    {
        g_commHandle.spiRecvTotalCount += eventMsg->num;
    }
#endif

    return;
}

void MuxProxy_Log_MuxCleanChannel(void)
{
#if defined(ENABLE_SPI_SEND_LOG)
    BUS_EventMsg msg;
    uint32_t remain = 0;
    BUS_MuxErrCode err;

    err = BUS_MuxRelease(g_MuxProxy_Log_MuxChl, &msg, &remain);
    COMM_LogMuxDisConnect();
    
    while (err == BUSMUX_OK) {
        COMM_SpiTransDone(msg.num, msg.data, msg.len, OS_FALSE);
        err = BUS_MuxRelease(g_MuxProxy_Log_MuxChl, &msg, &remain);
    }
#endif
}

void MuxProxy_Log_MuxRestartChannel(void)
{
#if defined(ENABLE_SPI_SEND_LOG)
    BUS_MuxStart(g_MuxProxy_Log_MuxChl);//  重新恢复通道
    COMM_LogMuxConnect();
    APP_MUX_PRINT_INFO("MuxProxy_Log_MuxRestartChannel MuxStart\r\n");
#endif
}

//   提供给MUX设备的回调函数，处理收到数据通知和数据发送完通知
static void MuxProxy_Log_MuxDevCB(BUS_MuxChannel *chl, BUS_EventMsg *msg, void *data, uint32_t event)
{
#if defined(ENABLE_SPI_SEND_LOG)
    (void)chl;
    (void)data;

    if(event == BUS_CHANNEL_TRANS_END_EVENT) //   发送完成,释放内存
    {
        COMM_SpiTransDone(msg->num, msg->data, msg->len, OS_TRUE); // data send done
    }
    else if(event == BUS_CHANNEL_DATA_IN_EVENT) //   收到新数据,转发消息给任务
    {
        // osPrintf("recv msg log\r\n");
        MuxProxy_Log_MuxReceive(msg); // receive data
    }
    else if(event == BUS_CHANNEL_TRANS_ERROR_EVENT) //   数据发送失败
    {
        APP_MUX_PRINT_INFO("MuxProxy_Log_MuxErrorCB event[%d]\r\n", event);
        //   数据发送失败后,也要释放内存
        COMM_SpiTransDone(msg->num, msg->data, msg->len, OS_FALSE);
    }
#endif

    return;
}

//   向MUX发送数据
static int32_t MuxProxy_Log_MuxSend(uint8_t *data, uint32_t dataLen, bool_t immediately)
{
#if defined(ENABLE_SPI_SEND_LOG)
    if (NULL == g_MuxProxy_Log_MuxChl)
    {
        return osError;
    }
    BUS_MuxErrCode err;
    //   由BUS Mux执行clean cache
    // osPrintf("Log mux addr:%p, dataLen:%u\r\n", data, dataLen);
    err = BUS_MuxSend(g_MuxProxy_Log_MuxChl, data, dataLen, immediately);
    if(err != BUSMUX_OK)
    {
        APP_MUX_PRINT_ERROR("MuxProxy_Log_MuxSend Fail %d\r\n", err);
        return osError;
    }
#endif

    return osOK;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
int32_t MuxProxy_Log_MuxInit(void)
{
#if defined(ENABLE_SPI_SEND_LOG)
    if (g_muxProxyLogReadQueueId == NULL)
    {
        osMessageQueueAttr_t msgQueueAttr;
        memset(&msgQueueAttr, 0, sizeof(msgQueueAttr));
        msgQueueAttr.name = "logSpiReadQueue";
        g_muxProxyLogReadQueueId = osMessageQueueNew(32,
            sizeof(BUS_EventMsg), &msgQueueAttr);
        OS_ASSERT(g_muxProxyLogReadQueueId != NULL);
    }

    //  打开MUX channel
    // osPrintf("MuxProxy_Log_MuxInit\r\n");
    if (g_MuxProxy_Log_MuxChl == NULL)
    {
        g_MuxProxy_Log_MuxChl = MuxProxy_InsertBusMuxChannel(MUX_PROXY_CHANNEL_LOG, "log",
            MUX_PROXY_LOG_BUSMUX_CHANNEL_PRIORITY, MuxProxy_Log_MuxDevCB, NULL);
    }
    COMM_RegisterSpiChannel(MuxProxy_Log_MuxSend);
#endif

    return osOK;
}