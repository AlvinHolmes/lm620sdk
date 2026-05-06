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
#include "drv_edma.h"
#include "bus_mux_api.h"
#include "net_api.h"
#include "mux_proxy_api.h"
#include "mux_proxy_pub.h"
#include "mux_proxy_control.h"
#include "mux_proxy_ip.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
static void MuxProxy_IP_MuxDevCB(BUS_MuxChannel *chl, BUS_EventMsg *msg, void* data, uint32_t event);
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define DEBUG_PROXY_IP_ON 0
#define STATISTIC_PROXY_IP_ON 1
/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static BUS_MuxChannel *g_MuxProxy_IP_MuxChl = NULL;

#if STATISTIC_PROXY_IP_ON
static MuxProxy_IP_Statistic g_MuxProxy_IP_Statistic = { 0 };
#define STATIS_UP_RECV_INC(count)   (g_MuxProxy_IP_Statistic.up_recv += count)
#define STATIS_UP_SENT_INC(count)   (g_MuxProxy_IP_Statistic.up_sent += count)
#define STATIS_UP_DROP_INC(count)   (g_MuxProxy_IP_Statistic.up_drop += count)
#define STATIS_DOWN_RECV_INC(count) (g_MuxProxy_IP_Statistic.down_recv += count)
#define STATIS_DOWN_SENT_INC(count) (g_MuxProxy_IP_Statistic.down_sent += count)
#define STATIS_DOWN_DROP_INC(count) (g_MuxProxy_IP_Statistic.down_drop += count)
#else
#define STATIS_UP_RECV_INC(count)
#define STATIS_UP_SENT_INC(count)
#define STATIS_UP_DROP_INC(count)
#define STATIS_DOWN_RECV_INC(count)
#define STATIS_DOWN_SENT_INC(count)
#define STATIS_DOWN_DROP_INC(count)
#endif

void debug_hex_stream_print(char *prefiex, uint8_t *data, uint16_t len)
{
#define PRINT_LENGTH 32
    uint16_t out_buf_len = PRINT_LENGTH * 3+1;
    char *out_log = osMalloc(out_buf_len);
    uint16_t i=0, j=0;

    APP_MUX_PRINT_INFO("\r\n%s[%d] begin\r\n", prefiex, len);
    memset(out_log, 0, out_buf_len);
    for(i=0; i < len; i++)
    {
        osSprintf((char *)out_log + j*3, "%02X ", data[i]);
        j++;
        if(j == PRINT_LENGTH)
        {
            APP_MUX_PRINT_INFO("[%d--%d] %s\r\n",i-(j-1), i, out_log);
            memset(out_log, 0, out_buf_len);
            j = 0;
        }
    }
    if(j != 0)
    {
        APP_MUX_PRINT_INFO("[%d--%d] %s\r\n",i-(j-1), i, out_log);
    }
    APP_MUX_PRINT_INFO("%s[%d] end\r\n", prefiex, i);
    osFree(out_log);
}


/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
//    提供给LWIP收数据包
static osStatus_t MuxProxy_IP_Input(void *payload,uint16_t len)
{
    int8_t ret;
#if DEBUG_PROXY_IP_ON
    APP_MUX_PRINT_INFO("MuxProxy_IP_Input [%p][%d]\r\n", payload, len);
    for(uint16_t j = 0; j < len; j++)
    {
        APP_MUX_PRINT_INFO("%02X ", ((char *)payload)[j]);
    }
    APP_MUX_PRINT_INFO("\r\n");
#endif
    ret = NetDrv_UlDataSend(payload, len);
#if DEBUG_PROXY_IP_ON
    debug_hex_stream_print("MUX In", (uint8_t *)payload, len);
#endif
    if(ret != 0)
    {
        APP_MUX_PRINT_ERROR("MuxProxy_IP_Input ret[%d]\r\n", ret);
    }
    return ret;
}

//    提供给LWIP通知数据包
static osStatus_t MuxProxy_IP_InputNotify(void)
{
    int8_t ret;

    ret = NetDrv_UlMsgNotify();
    if(ret != 0)
    {
        APP_MUX_PRINT_ERROR("MuxProxy_IP_InputNotify ret[%d]\r\n", ret);
    }
    return osOK;
}

//    释放Mux Event的内存
static void MuxProxy_IP_FreeEventMsg(BUS_EventMsg *eventMsg)
{
    uint8_t i;
    //   数据发送完成后,释放内存
    for(i = 0; i < eventMsg->num; i++)
    {
        NET_PbufFree(eventMsg->data[i]);
    }
    return;
}

//    MUX发送完数据后回调,在这里释放内存
static void MuxProxy_IP_MuxSendCB(BUS_EventMsg *eventMsg)
{
    MuxProxy_IP_FreeEventMsg(eventMsg);
    STATIS_DOWN_SENT_INC(eventMsg->num);
    return;
}

//   EDMA搬完后, 1.数据发送给网络; 2.释放SPI MUX的内存; 3.通知网络有数据
static void MuxProxy_IP_EdmaCB(void *args)
{
    uint8_t i;
    uint8_t success = 0;
    callBack_Info *callbackInfo = (callBack_Info *)args;
    index_format *edmaInfo = (index_format *)callbackInfo->param;

    for(i = 0; i < MUX_PROXY_IP_MAX_FROM_MUX + 1; i++)
    {
        if(edmaInfo[i].cfg.para_num == 0 && edmaInfo[i].dest_addr == 0) //最后一个节点
        {
            osFree((void *)(edmaInfo[i].source_addr)); //   释放SPI MUX内存, 从MUX过来的数据在一块内存中, 只要释放一次
            break;
        }
        else
        {
            osDCacheInvalidRange((void *)(edmaInfo[i].dest_addr), edmaInfo[i].cfg.para_num); //  清除cache中的数据, 保证后续软件访问最新的值
            if(MuxProxy_IP_Input((void *)(edmaInfo[i].dest_addr), edmaInfo[i].cfg.para_num)) //   发送数据给网络
            {
                //发送失败需要释放内存
                APP_MUX_PRINT_INFO("MuxProxy_IP_EdmaCB Error Len[%u]\r\n", edmaInfo[i].cfg.para_num);
                NET_PbufFree((void *)(edmaInfo[i].dest_addr));
                STATIS_UP_DROP_INC(1);
            }
            else
            {
                success++;
            }
        }
    }
    osFree(edmaInfo);
    osFree(callbackInfo);

    if(success != 0)
    {
        MuxProxy_IP_InputNotify(); //   通知网络
        STATIS_UP_SENT_INC(success);
    }
}

//    MUX收到数据后回调,使用DMA搬，添加eth header，发送
static void MuxProxy_IP_MuxReceiveCB(BUS_EventMsg *event)
{
    uint8_t i;
    uint32_t size = 0;
    index_format *edmaInfo = NULL;
    callBack_Info *callbackInfo = NULL;

    OS_ASSERT(event->num <= MUX_PROXY_IP_MAX_FROM_MUX);

    STATIS_UP_RECV_INC(event->num);

    size = sizeof(index_format) * (event->num + 1);//   多加1, 为了标识结束, 最后一个节点记录SPI MUX要释放的内存
    edmaInfo = osMalloc(size);
    OS_ASSERT(edmaInfo != NULL);
    memset(edmaInfo, 0 , size);

    for(i = 0; i < event->num; i++)
    {
        edmaInfo[i].dest_addr = (uint32_t)NET_PbufAlloc(event->len[i]);
        if(edmaInfo[i].dest_addr == 0)
        {
            APP_MUX_PRINT_INFO("%s, pbuf alloc fail, drop[%d]\r\n", __FUNCTION__, event->num-i);
            STATIS_UP_DROP_INC(event->num-i);
            break;
        }
        edmaInfo[i].source_addr = (uint32_t)event->data[i];
        edmaInfo[i].cfg.para_num = event->len[i];
    }

    if(i != 0) //   有数据要通过DMA搬运
    {
        //   在有效搬运节点后设置一个结束节点, 记录SPI MUX的内存
        edmaInfo[i].dest_addr = 0;
        edmaInfo[i].cfg.para_num = 0;
        edmaInfo[i].source_addr = (uint32_t)event->addr;
        //   有效节点的最后一个产生中断
        edmaInfo[i-1].cfg.int_enable = 1;

        callbackInfo = osMalloc(sizeof(callBack_Info));
        OS_ASSERT(callbackInfo != NULL);
        callbackInfo->cb = MuxProxy_IP_EdmaCB;
        callbackInfo->param = edmaInfo;
        if(EDMA_AddIndex(edmaInfo, i, callbackInfo) < 0)  // ICT 3874
        {
            APP_MUX_PRINT_INFO("%s, edma add index fail, drop[%d]\r\n", __FUNCTION__, i);
            STATIS_UP_DROP_INC(i);
            //   释放pbuf
            for(i = 0; i < event->num; i++)
            {
                if(edmaInfo[i].dest_addr != 0)
                {
                    NET_PbufFree((void *)edmaInfo[i].dest_addr);
                }
            }
            //   释放edmaInfo
            osFree(edmaInfo);
            //   释放callbackInfo
            osFree(callbackInfo);
            //   释放SPI MUX内存
            osFree(event->addr);
        }
    }
    else //   没有数据要通过DMA搬运, 直接释放SPI MUX内存
    {
        osFree(edmaInfo);
        osFree(event->addr);
    }

    return;
}

//    MUX发送数据失败后回调
static void MuxProxy_IP_MuxErrorCB(BUS_EventMsg *eventMsg, uint32_t event)
{
    APP_MUX_PRINT_INFO("MuxProxy_IP_MuxErrorCB event[%d]\r\n", event);
    if(event == BUS_CHANNEL_TRANS_ERROR_EVENT)
    {
        MuxProxy_IP_FreeEventMsg(eventMsg);
        STATIS_DOWN_DROP_INC(eventMsg->num);
        // MuxProxy_Control_MuxDown(); 新方案的Down等busmux通知
    }
    return;
}

void MuxProxy_IP_MuxCleanChannel(void)
{
    BUS_EventMsg msg;
    uint32_t remain = 0;
    BUS_MuxErrCode err;

    err = BUS_MuxRelease(g_MuxProxy_IP_MuxChl, &msg, &remain);

    while (err == BUSMUX_OK) {
        MuxProxy_IP_FreeEventMsg(&msg);
        err = BUS_MuxRelease(g_MuxProxy_IP_MuxChl, &msg, &remain);
    }
    APP_MUX_PRINT_INFO("MuxProxy_IP_MuxCleanChannel\r\n");
}

void MuxProxy_IP_MuxRestartChannel(void)
{
    BUS_MuxStart(g_MuxProxy_IP_MuxChl);//  重新恢复通道
    APP_MUX_PRINT_INFO("MuxProxy_IP_MuxRestartChannel\r\n");
}

//   提供给MUX设备的回调函数，处理收到数据通知和数据发送完通知
static void MuxProxy_IP_MuxDevCB(BUS_MuxChannel *chl, BUS_EventMsg *msg, void *data, uint32_t event)
{
    (void)chl;
    (void)data;
    if(event == BUS_CHANNEL_TRANS_END_EVENT) // 发送完成,释放内存
    {
        MuxProxy_IP_MuxSendCB(msg);// data send done
    }
    else if(event == BUS_CHANNEL_DATA_IN_EVENT) // 收到新数据,转发消息给任务
    {
        MuxProxy_IP_MuxReceiveCB(msg);// receive data
    }
    else
    {
        MuxProxy_IP_MuxErrorCB(msg, event);
    }

    return;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
//    提供给LWIP发送数据包
int32_t MuxProxy_IP_Output(void *payload, uint16_t payloadLen)
{
    BUS_MuxErrCode err;
    //   从LWIP过来的pbuf已经32字节对齐, BUS Mux执行clean cache不会有问题
    err = BUS_MuxSend(g_MuxProxy_IP_MuxChl, payload, payloadLen, true);
    STATIS_DOWN_RECV_INC(1);
    if(err != BUSMUX_OK)
    {
        APP_MUX_PRINT_ERROR("MuxProxy_IP_Output Err[%d][%p][%d]\r\n", err, payload, payloadLen);
        STATIS_DOWN_DROP_INC(1);
    }
    return err;
}

int8_t MuxProxy_IP_MuxInit(void)
{
    APP_MUX_PRINT_INFO("MuxProxy_IP_MuxInit\r\n");
    //  打开MUX channel
    if(g_MuxProxy_IP_MuxChl == NULL)
    {
        g_MuxProxy_IP_MuxChl = MuxProxy_InsertBusMuxChannel(MUX_PROXY_CHANNEL_IP, "ip", MUX_PROXY_IP_BUSMUX_CHANNEL_PRIORITY,
                                        MuxProxy_IP_MuxDevCB, NULL);
    }
    return osOK;
}

void MuxProxy_IP_ShowStatistic(void)
{
#if STATISTIC_PROXY_IP_ON
    osPrintf("========Ip Statistic==========\r\n");
    osPrintf("Up   R[%u] S[%u] D[%u]\r\n", g_MuxProxy_IP_Statistic.up_recv, g_MuxProxy_IP_Statistic.up_sent, g_MuxProxy_IP_Statistic.up_drop);
    osPrintf("Down R[%u] S[%u] D[%u]\r\n", g_MuxProxy_IP_Statistic.down_recv, g_MuxProxy_IP_Statistic.down_sent, g_MuxProxy_IP_Statistic.down_drop);
#endif
}

void MuxProxy_IP_ClearStatistic(void)
{
#if STATISTIC_PROXY_IP_ON
    osMemset(&g_MuxProxy_IP_Statistic, 0 , sizeof(g_MuxProxy_IP_Statistic));
#endif
}


