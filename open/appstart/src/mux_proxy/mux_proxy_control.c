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

#include "mux_proxy_api.h"
#include "mux_proxy_pub.h"
#include "mux_proxy_control.h"
#include "mux_proxy_at.h"
#include "mux_proxy_ip.h"
#include "mux_proxy_log.h"
#include "mux_proxy_pass_through.h"

#include "net_api.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
static void MuxProxy_Control_MuxDevCB(BUS_MuxChannel *chl, BUS_EventMsg *msg, void* data, uint32_t event);

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define MUX_PROXY_CONTROL_MESSAGE_COUNT 10  //任务消息个数
#define MUX_PROXY_CONTROL_MESSAGE_SIZE   (sizeof(MuxProxy_Message))  //任务消息大小

//   状态标记位
#define MUX_PROXY_CTRL_MUX_UP_FLAG_MASK       (uint32_t)(1 << 0) // bit0 对应SPI MUX
#define MUX_PROXY_CTRL_MAC_UP_FLAG_MASK       (uint32_t)(1 << 1) // bit1 对应RNDIS
#define MUX_PROXY_CTRL_IP_UP_FLAG_MASK        (uint32_t)(1 << 2) // bit2 对应PDP
#define MUX_PROXY_CTRL_IP_SUSPEND_FLAG_MASK   (uint32_t)(1 << 3) // bit3 对应流控

static uint32_t g_MuxProxy_Control_Status = 0;

#define MUX_PROXY_STATUS_GET(mask)       (g_MuxProxy_Control_Status & (mask))
#define MUX_PROXY_STATUS_SET(mask)       (g_MuxProxy_Control_Status |= (mask))
#define MUX_PROXY_STATUS_CLR(mask)       (g_MuxProxy_Control_Status &= (~(mask)))

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static osMessageQueueId_t gp_MuxProxy_Control_MQID = NULL;
static osThreadId_t gp_MuxProxy_Control_TaskID = NULL;
static BUS_MuxChannel *g_MuxProxy_Control_MuxChl = NULL;

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static osStatus_t MuxProxy_Control_SendTaskMessage(uint8_t msgId, void *msgData, uint32_t msgDataLen)
{
    MuxProxy_Message message;

    message.msgId = msgId;
    message.msgData = msgData;
    message.msgDataLen = msgDataLen;

    OS_ASSERT(gp_MuxProxy_Control_MQID != NULL);
    return osMessageQueuePut(gp_MuxProxy_Control_MQID, &message, 0, 0);
}

//   向对端发送命令,cmdId参考 MuxProxy_ControlCommandId
static osStatus_t MuxProxy_Control_SendMuxCommand(uint32_t cmdId, uint32_t dataLen, void *msgData)
{
    MuxProxy_CommandInfoHeader *header;
    BUS_MuxErrCode err;
    header = osMallocAlign(MUX_CONTROL_COMMAND_INFO_HEADER_SIZE + dataLen, OS_CACHE_LINE_SZ);//   申请Cache Line对齐的内存, 设备可能会执行cache clean, 避免影响到别人的内存
    header->cmdId = cmdId;
    header->dataLen = dataLen;

    APP_MUX_PRINT_INFO("MuxProxy_Control_SendMuxCommand Mux Send [%d]\r\n", cmdId);

    if(dataLen != 0)
    {
        memcpy((uint8_t *)(header+1), (uint8_t *)msgData, dataLen);
    }
    //   由BUS Mux执行clean cache
    err = BUS_MuxSend(g_MuxProxy_Control_MuxChl, (uint8_t *)header, MUX_CONTROL_COMMAND_INFO_HEADER_SIZE + dataLen, true);
    if(err != BUSMUX_OK)
    {
        osFree(header);
        APP_MUX_PRINT_ERROR("MuxProxy_Control_SendMuxCommand Mux Send Fail %d\r\n", err);
        return osErrorResource;
    }
    return osOK;
}

//    释放Mux Event的内存
static void MuxProxy_Control_FreeEventMsg(BUS_EventMsg *eventMsg)
{
    uint8_t i;

    for(i = 0; i < eventMsg->num; i++)
    {
        osFree(eventMsg->data[i]);
    }
    return;
}

//    MUX发送完数据后回调,在这里释放内存
static void MuxProxy_Control_MuxSendCB(BUS_EventMsg *eventMsg)
{
    MuxProxy_Control_FreeEventMsg(eventMsg);
    return;
}

void MuxProxy_Control_MuxCleanChannel(void)
{
    BUS_EventMsg msg;
    uint32_t remain = 0;
    BUS_MuxErrCode err;

    err = BUS_MuxRelease(g_MuxProxy_Control_MuxChl, &msg, &remain);

    while (err == BUSMUX_OK) {
        MuxProxy_Control_FreeEventMsg(&msg);
        err = BUS_MuxRelease(g_MuxProxy_Control_MuxChl, &msg, &remain);
    }
    APP_MUX_PRINT_INFO("MuxProxy_Control_MuxCleanChannel\r\n");
}

void MuxProxy_Control_MuxRestartChannel(void)
{
    BUS_MuxStart(g_MuxProxy_Control_MuxChl);//  重新恢复通道
    APP_MUX_PRINT_INFO("MuxProxy_Control_MuxRestartChannel\r\n");
}

//    MUX收到数据后回调
static void MuxProxy_Control_MuxReceiveCB(BUS_EventMsg *eventMsg)
{
    MuxProxy_CommandInfoHeader *header;
    uint8_t *data;
    uint8_t i;

    for(i = 0; i < eventMsg->num; i++)
    {
        header = (MuxProxy_CommandInfoHeader *)eventMsg->data[i];//先获取消息头，每个消息必须的内容
        if(header->dataLen != 0)
        {
            //  消息数据需要单独分配内存
            data = osMalloc(header->dataLen);
            OS_ASSERT(data != NULL);
            memcpy(data, eventMsg->data[i] + MUX_CONTROL_COMMAND_INFO_HEADER_SIZE, header->dataLen);
        }
        else
        {
            data = NULL;
        }
        MuxProxy_Control_SendTaskMessage(header->cmdId, data, header->dataLen);
    }
    osFree(eventMsg->addr);
    return;
}

//    MUX发送数据失败后回调
static void MuxProxy_Control_MuxErrorCB(BUS_EventMsg *eventMsg, uint32_t event)
{
    APP_MUX_PRINT_INFO("MuxProxy_Control_MuxErrorCB event[%d]\r\n", event);
    if(event == BUS_CHANNEL_TRANS_ERROR_EVENT)
    {
        uint8_t i;
        MuxProxy_CommandInfoHeader *header;
        //   数据发送失败后,也要释放内存
        for(i = 0; i < eventMsg->num; i++)
        {
            header = (MuxProxy_CommandInfoHeader *)eventMsg->data[i];
            APP_MUX_PRINT_INFO("MuxProxy_Control_MuxErrorCB cmdId[%d]\r\n", header->cmdId);
            osFree(eventMsg->data[i]);
        }
        // MuxProxy_Control_MuxDown(); 这里不直接down，等BusMux上报
    }
    return;
}

//   提供给MUX设备的回调函数，处理收到数据通知和数据发送完通知
static void MuxProxy_Control_MuxDevCB(BUS_MuxChannel *chl, BUS_EventMsg *msg, void *data, uint32_t event)
{
    (void)chl;
    (void)data;
    if(event == BUS_CHANNEL_TRANS_END_EVENT) // 发送完成,释放内存
    {
        MuxProxy_Control_MuxSendCB(msg);// data send done
    }
    else if(event == BUS_CHANNEL_DATA_IN_EVENT) // 收到新数据,转发消息给任务
    {
        MuxProxy_Control_MuxReceiveCB(msg);// receive data
    }
    else
    {
        MuxProxy_Control_MuxErrorCB(msg, event);
    }

    return;
}

//   通道握手
static uint32_t MuxProxy_Control_MuxCheck(void)
{
    return MuxProxy_Control_SendMuxCommand(MUX_PROXY_CTRLCMD_QUERY, 0, NULL);
}

//   MAC建立连接
static void MuxProxy_Control_LinkUp(void)
{
    //   通知网络link up
    APP_MUX_PRINT_INFO("MAC Link Up\r\n");
    MUX_PROXY_STATUS_SET(MUX_PROXY_CTRL_MAC_UP_FLAG_MASK);
    (void)NET_EthLinkConnCB(1);
}

//   MAC断开连接
static void MuxProxy_Control_LinkDown(void)
{
    //   通知网络link down
    APP_MUX_PRINT_INFO("MAC Link Down\r\n");
    MUX_PROXY_STATUS_CLR(MUX_PROXY_CTRL_MAC_UP_FLAG_MASK);
    (void)NET_EthLinkConnCB(0);
}

//   MUX建立连接,     对端存在
static void MuxProxy_Control_MuxConnect(void)
{
    if(MUX_PROXY_STATUS_GET(MUX_PROXY_CTRL_MUX_UP_FLAG_MASK) == 0)
    {
        APP_MUX_PRINT_INFO("MUX Link Up\r\n");
        MUX_PROXY_STATUS_SET(MUX_PROXY_CTRL_MUX_UP_FLAG_MASK);
        //  初始化其他通道
        MuxProxy_Control_MuxInit();
        MuxProxy_PassThrough_MuxInit();
        MuxProxy_AT_MuxInit();
        MuxProxy_IP_MuxInit();
        MuxProxy_Log_MuxInit();

        MuxProxy_Control_MuxRestartChannel();
        MuxProxy_AT_MuxRestartChannel();
        MuxProxy_IP_MuxRestartChannel();
        MuxProxy_Log_MuxRestartChannel();

        MuxProxy_AT_ChannelInit();
        MuxProxy_Control_MuxCheck();
    }
}

//   MUX断开连接, 对端不在
static void MuxProxy_Control_MuxDisConnect(void)
{
    if(MUX_PROXY_STATUS_GET(MUX_PROXY_CTRL_MUX_UP_FLAG_MASK) != 0)
    {
        APP_MUX_PRINT_INFO("MUX Link Down\r\n");
        MUX_PROXY_STATUS_CLR(MUX_PROXY_CTRL_MUX_UP_FLAG_MASK);
        MuxProxy_AT_ChannelDeInit();
        MuxProxy_Control_LinkDown();
        //释放mux通道积压的数据
        MuxProxy_Control_MuxCleanChannel();
        MuxProxy_Log_MuxCleanChannel();
        MuxProxy_AT_MuxCleanChannel();
        MuxProxy_IP_MuxCleanChannel();
    }
}

static void MuxProxy_Control_TaskEntry(void *parameter)
{
    MuxProxy_Message message;

    APP_MUX_PRINT_INFO("MuxProxy_Control_TaskEntry\r\n");

    while (1)
    {
        if(osMessageQueueGet(gp_MuxProxy_Control_MQID, &message, 0, osWaitForever) == osOK)
        {
            APP_MUX_PRINT_INFO("MuxProxy_Control_TaskEntry msgId %d\r\n", message.msgId);
            switch(message.msgId)
            {
                case MUX_PROXY_CTRLCMD_QUERY:
                {
                    MuxProxy_Control_MuxConnect();
                    MuxProxy_Control_SendMuxCommand(MUX_PROXY_CTRLCMD_ANSWER, 0, NULL);
                }
                break;
                case MUX_PROXY_CTRLCMD_ANSWER:
                {
                    MuxProxy_Control_MuxConnect();
                }
                break;
                case MUX_PROXY_CTRLCMD_MAC_CONNECT:
                {
                    MuxProxy_Control_MuxConnect();
                    MuxProxy_Control_LinkUp();
                }
                break;
                case MUX_PROXY_CTRLCMD_MAC_DISCONNECT:
                {
                    MuxProxy_Control_LinkDown();
                }
                break;
                case MUX_PROXY_CTRLCMD_CHL_STATUS:
                {
                    //   通道状态通知
                    MuxProxy_ChannelStatusInfo *status = message.msgData;
                    APP_MUX_PRINT_INFO("MuxProxy_Control_TaskEntry mux channel status[0x%x]\r\n", status->channel_status);
                    OS_ASSERT(status != OS_NULL);
                    osFree(status);
                }
                break;
                case MUX_PROXY_CTRLCMD_TEST:
                {
                    APP_MUX_PRINT_INFO("Receive Mux Test\r\n");
                }
                break;
                case MUX_PROXY_CTRLMSG_MUX_CONNECT:
                {
                    MuxProxy_Control_MuxConnect();
                }
                break;
                case MUX_PROXY_CTRLMSG_MUX_DISCONNECT:
                {
                    MuxProxy_Control_MuxDisConnect();
                }
                break;
                default:
                    break;
            }
        }
    }
}
/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
//    提供给网络配置IP状态，网关IP地址，网关MAC地址
void MuxProxy_Control_IpConnect(uint32_t ip4_NetProxyAddr, uint8_t NetProxyMacAddr[MUX_PROXY_MAC_ADDR_LEN])
{
    MuxProxy_GateWayInfo connect;

    osPrintf("%s start\r\n", __FUNCTION__);
    memset(&connect, 0 ,sizeof(connect));
    memcpy(connect.mac_addr, NetProxyMacAddr, MUX_PROXY_MAC_ADDR_LEN);
    if(ip4_NetProxyAddr != 0)
    {
        connect.ip_type = 4;
        connect.ip_addr[0] = ip4_NetProxyAddr;
    }
    else
    {
        connect.ip_type = 6;
    }
    MUX_PROXY_STATUS_SET(MUX_PROXY_CTRL_IP_UP_FLAG_MASK);
    MuxProxy_Control_SendMuxCommand(MUX_PROXY_CTRLCMD_IP_CONNECT, sizeof(connect), &connect);
    return;
}

//    提供给网络配置IP状态
void MuxProxy_Control_IpDisconnect(void)
{
    MUX_PROXY_STATUS_CLR(MUX_PROXY_CTRL_IP_UP_FLAG_MASK);
    MuxProxy_Control_SendMuxCommand(MUX_PROXY_CTRLCMD_IP_DISCONNECT, 0, NULL);
    return;
}

//   用于IP流控, 暂停IP包发送
void MuxProxy_Control_IpPause(void)
{
    MUX_PROXY_STATUS_SET(MUX_PROXY_CTRL_IP_SUSPEND_FLAG_MASK);
    MuxProxy_Control_SendMuxCommand(MUX_PROXY_CTRLCMD_IP_PAUSE, 0, NULL);
    return;
}

//   用于IP流控, 恢复IP包发送
void MuxProxy_Control_IpResume(void)
{
    MUX_PROXY_STATUS_CLR(MUX_PROXY_CTRL_IP_SUSPEND_FLAG_MASK);
    MuxProxy_Control_SendMuxCommand(MUX_PROXY_CTRLCMD_IP_RESUME, 0, NULL);
    return;
}

//   判断链路是否UP
uint32_t MuxProxy_Control_isLinkUp(void)
{
    return MUX_PROXY_STATUS_GET(MUX_PROXY_CTRL_MAC_UP_FLAG_MASK) != 0;
}

//   MUX连接
int32_t MuxProxy_Control_MuxUp(void)
{
    return MuxProxy_Control_SendTaskMessage(MUX_PROXY_CTRLMSG_MUX_CONNECT, NULL, 0);
}

//   MUX断开
int32_t MuxProxy_Control_MuxDown(void)
{
    return MuxProxy_Control_SendTaskMessage(MUX_PROXY_CTRLMSG_MUX_DISCONNECT, NULL, 0);
}

//   判断MUX是否UP
uint32_t MuxProxy_Control_isMuxUp(void)
{
    return MUX_PROXY_STATUS_GET(MUX_PROXY_CTRL_MUX_UP_FLAG_MASK) != 0;
}

int32_t MuxProxy_Control_Init(void)
{
    // uint32_t ret = 0;

    osMessageQueueAttr_t msgQueueAttr;
    memset(&msgQueueAttr, 0, sizeof(msgQueueAttr));
    msgQueueAttr.name = "proxyctrlmq";
    gp_MuxProxy_Control_MQID = osMessageQueueNew(MUX_PROXY_CONTROL_MESSAGE_COUNT, MUX_PROXY_CONTROL_MESSAGE_SIZE, &msgQueueAttr);
    OS_ASSERT(gp_MuxProxy_Control_MQID != NULL);

    osThreadAttr_t atThreadAttr = {"proxyctrl", osThreadDetached, NULL, 0U, NULL, MUX_PROXY_CONTROL_TASK_STACK_SIZE, MUX_PROXY_CONTROL_TASK_PRIORITY, 0U, 0U};
    gp_MuxProxy_Control_TaskID = osThreadNew(MuxProxy_Control_TaskEntry, NULL, &atThreadAttr);
    OS_ASSERT(gp_MuxProxy_Control_TaskID != NULL);

    APP_MUX_PRINT_INFO("MuxProxy_Control_Init\r\n");

    return osOK;
}

int8_t MuxProxy_Control_MuxInit(void)
{
    APP_MUX_PRINT_INFO("MuxProxy_Control_MuxInit\r\n");
    //  打开MUX channel
    if (g_MuxProxy_Control_MuxChl == NULL)
    {
        g_MuxProxy_Control_MuxChl = MuxProxy_InsertBusMuxChannel(MUX_PROXY_CHANNEL_CONTROL, "ctrl", MUX_PROXY_CONTROL_BUSMUX_CHANNEL_PRIORITY,
                                MuxProxy_Control_MuxDevCB, NULL);
    }

    //   发个数据, 看看对端在不在, 只发一次
    // ret |= MuxProxy_Control_MuxCheck();
    // ret |= MuxProxy_Control_MuxCheck(); // 硬件第一次收发有问题, 这里再加发一次
    // OS_ASSERT(ret == osOK); Check动作等busmux通知UP后再执行

    return osOK;
}


//   检查通道是否正常
int32_t MuxProxy_Control_MuxTest(void)
{
    return MuxProxy_Control_SendMuxCommand(MUX_PROXY_CTRLCMD_TEST, 0, NULL);
}

void MuxProxy_Control_ShellShow(void)
{
    osPrintf("========Status==========\r\n");
    osPrintf("0x%X\r\n",g_MuxProxy_Control_Status);
}

