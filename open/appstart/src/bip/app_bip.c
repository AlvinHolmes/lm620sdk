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
#ifdef APP_BIP_SUPPORT
/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <os.h>
#include "at_parser.h"
#include "svc_api.h"
#include "usat_cdec.h"
#include "svc_usat.h"
#include "net_pub.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "app_bip.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
static int APP_BIP_NET_Start(uint8_t domainType, ip_addr_t remoteIp, uint16_t remotePort);
static int APP_BIP_NET_Send(uint8_t *data, uint16_t len);
static void APP_BIP_NET_Stop(void);
static void APP_BIP_NET_ReceiveDataProc(int fd, int len);
static void APP_BIP_NotifyOffWork(void);

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define APP_BIP_MESSAGE_QUEUE_MSG_MAX               5
#define APP_BIP_TASK_STACK_SIZE ((uint32_t) (1024*3))
#define APP_BIP_TASK_PRIORITY   (osPriorityBelowNormal)

#define NETWORK_TIMEOUT           ((uint32_t) (1 * 100))//ms
#define TIMES_FOR_NETWORK         ((uint32_t) (50*60))      //times:5min

#define APP_BIP_RECEIVE_BUFFER_MAX_LENGTH  (1500) // 可以跟 NETIF_MTU_LEN_DEFAULT  一样
#define APP_BIP_TERMINAL_RESPONSE_MAX_LENGTH  (255)  // 向卡发送的数据不会超过255

#define APP_BIP_USAT_FLAGS_CHANNEL_STATE_BIT_MASK   (1<<0)
#define APP_BIP_USAT_FLAGS_DATA_AVAILABLE_BIT_MASK  (1<<1)
#define APP_BIP_USAT_FLAGS_OFF_WORK_BIT_MASK        (1<<2)

#define APP_BIP_USAT_FLAGS_BIT_TEST1(BIT_MASK)  (g_app_bip_usatflags & (BIT_MASK))  // 检查某个bit是1
#define APP_BIP_USAT_FLAGS_BIT_SET1(BIT_MASK)  (g_app_bip_usatflags |= (BIT_MASK))  // 设置某个bit为1
#define APP_BIP_USAT_FLAGS_BIT_CLEAR0(BIT_MASK)  (g_app_bip_usatflags &= (uint8_t)(~BIT_MASK))   // 设置某个bit为0

//  打桩的服务器地址和端口
#define APP_BIP_SERVER_STUB 0
#define APP_BIP_SERVER_STUB_IP  "112.125.89.8"
#define APP_BIP_SERVER_STUB_PORT  44584
/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static osMessageQueueId_t *g_app_bip_message_q = NULL;
static osThreadId_t g_app_bip_task = NULL;
static osSlist_t g_app_bip_netdata = OS_SLIST_OBJECT_INIT(g_app_bip_netdata); //   从服务器来的数据
static osSlist_t g_app_bip_simdata = OS_SLIST_OBJECT_INIT(g_app_bip_simdata); //   从SIM卡来的数据
static int g_app_bip_socket_fd = -1;
static uint8_t g_app_bip_usatflags = 0; // 换卡后要更新

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
#define BASE_BEGIN
//  这个API是否可以返回固定值?
static u8_t APP_BIP_GetCid(void)
{
    return 1;//  仅支持1个通道
    #if 0
    struct netif *default_netif = netif_get_default();

    if(default_netif == NULL)
        return 0;
    else
        return netif_get_cid(default_netif);
    #endif
}

//   返回值参考 NET_IpState
//   只需要看default, default变了怎么办?
static bool APP_BIP_NetUp(NET_IpType ipType)
{
    struct netif *netif = netif_get_default();
    if(netif != NULL)
    {
        if(netif_is_up(netif))
        {
            return (netif->ip_state & ipType)? true : false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

static void APP_BIP_FreeDataList(osSlist_t *header)
{
    APP_BIP_Data *pData = NULL;
    osSlist_t *pNode = NULL;

    if(!osSlistIsEmpty(header))
    {
        osSlistForEach(pNode, header)
        {
            pData = osListEntry(pNode, APP_BIP_Data, node);
            osFree(pData);
        }
    }
    osSlistInit(header);
}

static uint32_t APP_BIP_DataListLength(osSlist_t *header)
{
    APP_BIP_Data *pData = NULL;
    osSlist_t *pNode = NULL;
    uint32_t len = 0;

    if(!osSlistIsEmpty(header))
    {
        osSlistForEach(pNode, header)
        {
            pData = osListEntry(pNode, APP_BIP_Data, node);
            len += pData->usedSize;
        }
    }

    return len;
}

static APP_BIP_Data* APP_BIP_GetDataListFirst(osSlist_t *header)
{
    APP_BIP_Data *pData = NULL;
    osSlist_t *pNode = NULL;

    if(osSlistIsEmpty(header))
    {
        return NULL;
    }

    pNode = osSlistFirst(header);
    pData = osListEntry(pNode, APP_BIP_Data, node);

    return pData;
}

static APP_BIP_Data* APP_BIP_AllocData(uint16_t totalSize)
{
    APP_BIP_Data *pData = NULL;
    uint32_t size = totalSize + sizeof(APP_BIP_Data);
    pData = osMalloc(size);
    if(pData != NULL)
    {
        memset(pData, 0, size);
        pData->data = (uint8_t *)(pData+1);
        pData->totalSize = totalSize;
        pData->usedSize = 0;
    }

    return pData;
}

static void APP_BIP_CleanData(APP_BIP_Data *pData)
{
    memset(pData->data, 0, pData->totalSize);
    pData->usedSize = 0;
}


#define BASE_END
#define USIM_BEGIN
static int APP_BIP_USAT_OpenChannelProc(uint8_t domainType, ip_addr_t remoteIp, uint16_t remotePort)
{
    return APP_BIP_NET_Start(domainType, remoteIp, remotePort);
}

static int APP_BIP_USAT_CloseChannelProc(void)
{
    APP_BIP_NET_Stop();
    return osOK;
}

static int APP_BIP_USAT_SendDataProc(uint8_t *data, uint16_t len)
{
    return APP_BIP_NET_Send(data, len);
}

//  主动式命令处理 begin
//   result是执行结果，当执行错误时要携带addResult
static int APP_BIP_USAT_OpenChannelTerminalResponse(S_Usat_Proc_Cmd *pProactiveCommand, uint8_t result, uint8_t addResult, uint8_t cid)
{
    S_Usat_TR_OpenChannel openChannelTR = {0};
    uint32_t terminalRsponseLen = 0;
    uint8_t terminalRsponsebuf[APP_BIP_TERMINAL_RESPONSE_MAX_LENGTH] = {0};
    int ret;

    openChannelTR.tBuffSize = pProactiveCommand->u.tOpenChan.tBuffSize;
    openChannelTR.tBearerDesc = pProactiveCommand->u.tOpenChan.tBearDesc;

    if(result == 0)
    {
        openChannelTR.tChanStat.bChanId = cid;
        openChannelTR.tChanStat.bLinkEsted = 1;
        terminalRsponseLen = Usat_EncTerminalResponse(terminalRsponsebuf, USAT_RST_PERFM_SUCCESS, NULL, &openChannelTR);
    }
    else
    {
        S_Usat_TR_AddRst tAddRst = {0}; //  BIP处理结果失败时，需要填写附加信息说明失败原因

        openChannelTR.tChanStat.bLinkEsted = 0;
        tAddRst.bLen = 1;
        tAddRst.abAddRst[0] = addResult;
        terminalRsponseLen = Usat_EncTerminalResponse(terminalRsponsebuf, result, &tAddRst, &openChannelTR);
    }

    ret = svc_usat_set_terminal_response((uint8_t *)terminalRsponsebuf, terminalRsponseLen);
    if(ret != 0)
    {
        BIP_PRINT_INFO("APP_BIP_USAT_OpenChannelTerminalResponse Send AT Error %d\r\n", ret);
    }

    return osOK;
}

//  open channel
static int APP_BIP_USAT_ProactiveCommandOpenChannel(S_Usat_Proc_Cmd *pProactiveCommand)
{
    uint8_t domainType = SOCK_STREAM;
    ip_addr_t remoteIp = {0};
    uint16_t remotePort = 0;
    NET_IpType ipType = 0;
    uint32_t net_timeout = 0;
    uint16_t bufferSize = APP_BIP_RECEIVE_BUFFER_MAX_LENGTH;
    int ret;

    // link
    BIP_PRINT_INFO("ProactiveCommand, ImdtLinkEst : %u\r\n", pProactiveCommand->tCmdDetail.uQual.tOpenChan.bImdtLinkEst);
    // APN
    BIP_PRINT_INFO("ProactiveCommand, APN.Tag : %u\r\n", pProactiveCommand->u.tOpenChan.tApn.bTag);
    // user
    BIP_PRINT_INFO("ProactiveCommand, Uerlog.Tag : %u\r\n", pProactiveCommand->u.tOpenChan.tUserLog.bTag);
    BIP_PRINT_INFO("ProactiveCommand, UerPwd.Tag : %u\r\n", pProactiveCommand->u.tOpenChan.tUserPwd.bTag);
    // bear desc
    BIP_PRINT_INFO("ProactiveCommand, BearDesc.Tag : %u\r\n", pProactiveCommand->u.tOpenChan.tBearDesc.tOrgParam.bTag);
    BIP_PRINT_INFO("ProactiveCommand, BearDesc.Len : %u\r\n", pProactiveCommand->u.tOpenChan.tBearDesc.tOrgParam.bLen);
    BIP_PRINT_INFO("ProactiveCommand, BearDesc.Type : %u\r\n", pProactiveCommand->u.tOpenChan.tBearDesc.tDecParam.bType);
    if(pProactiveCommand->u.tOpenChan.tBearDesc.tOrgParam.bTag != 0)
    {
        if (pProactiveCommand->u.tOpenChan.tBearDesc.tDecParam.bType != USAT_BEARER_DESC_TYPE_DEFAULT
            && pProactiveCommand->u.tOpenChan.tBearDesc.tDecParam.bType != USAT_BEARER_DESC_TYPE_EUTRAN_2)
        {
            BIP_PRINT_INFO("ProactiveCommand, Cannt Support BearDesc.Type\r\n");

            APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_TYPE_NOT_UNDERSTOOD_BY_TERM, BIP_ADD_RST_NULL, 0);
            return osErrorParameter;
        }
    }
    else
    {
        // no bear desc use as default
        BIP_PRINT_INFO("ProactiveCommand, NO BearDesc.Tag\r\n");
    }
    // buffersize
    BIP_PRINT_INFO("ProactiveCommand, BuffSize.Tag : %u\r\n", pProactiveCommand->u.tOpenChan.tBuffSize.bTag);
    BIP_PRINT_INFO("ProactiveCommand, BuffSize.Len : %u\r\n", pProactiveCommand->u.tOpenChan.tBuffSize.bLen);
    BIP_PRINT_INFO("ProactiveCommand, BuffSize.Size : %u\r\n", pProactiveCommand->u.tOpenChan.tBuffSize.wSize);
    if(pProactiveCommand->u.tOpenChan.tBuffSize.bTag != 0)
    {
        if(pProactiveCommand->u.tOpenChan.tBuffSize.wSize > APP_BIP_RECEIVE_BUFFER_MAX_LENGTH) //  超过终端支持的值，  协商成终端支持的大小
        {
            BIP_PRINT_INFO("ProactiveCommand, BuffSize.Size %d GT default\r\n", pProactiveCommand->u.tOpenChan.tBuffSize.wSize);
            bufferSize = APP_BIP_RECEIVE_BUFFER_MAX_LENGTH;
        }
        else
        {
            bufferSize = pProactiveCommand->u.tOpenChan.tBuffSize.wSize;
        }
    }
    else
    {
        BIP_PRINT_INFO("ProactiveCommand, Cannt Support No BufferSize\r\n");
        APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_PERFM_MISS_INFORM, BIP_ADD_RST_NULL, 0);
        return osErrorParameter;
    }
    // dest addr
    BIP_PRINT_INFO("ProactiveCommand, DestAddr.Tag : %u\r\n", pProactiveCommand->u.tOpenChan.tDestAddr.bTag);
    BIP_PRINT_INFO("ProactiveCommand, DestAddr.Len : %u\r\n", pProactiveCommand->u.tOpenChan.tDestAddr.bLen);
    BIP_PRINT_INFO("ProactiveCommand, DestAddr.Type : %u\r\n", pProactiveCommand->u.tOpenChan.tDestAddr.bType); //USAT_ADDRESS_TYPE_IPV4  USAT_ADDRESS_TYPE_IPV6

    if(pProactiveCommand->u.tOpenChan.tDestAddr.bTag != 0)
    {
        if(pProactiveCommand->u.tOpenChan.tDestAddr.bType == USAT_ADDRESS_TYPE_IPV4)
        {
            IP_ADDR4(&remoteIp,
            pProactiveCommand->u.tOpenChan.tDestAddr.abAddr[0],
            pProactiveCommand->u.tOpenChan.tDestAddr.abAddr[1],
            pProactiveCommand->u.tOpenChan.tDestAddr.abAddr[2],
            pProactiveCommand->u.tOpenChan.tDestAddr.abAddr[3]);
            ipType = NET_IP_4;

            BIP_PRINT_INFO("ProactiveCommand, DestAddr.addr : %s\r\n", ipaddr_ntoa(&remoteIp));
        }
        else if(pProactiveCommand->u.tOpenChan.tDestAddr.bType == USAT_ADDRESS_TYPE_IPV6)
        {
            //   SIM卡给出的是网络序
            IP_ADDR6(&remoteIp,
            *((u32_t *)pProactiveCommand->u.tOpenChan.tDestAddr.abAddr+0),
            *((u32_t *)pProactiveCommand->u.tOpenChan.tDestAddr.abAddr+1),
            *((u32_t *)pProactiveCommand->u.tOpenChan.tDestAddr.abAddr+2),
            *((u32_t *)pProactiveCommand->u.tOpenChan.tDestAddr.abAddr+3));
            ipType = NET_IP_6;

            BIP_PRINT_INFO("ProactiveCommand, DestAddr.addr : %s\r\n", ipaddr_ntoa(&remoteIp));
        }
        else
        {
            BIP_PRINT_INFO("ProactiveCommand, Cannt Support No DestAddr.bType\r\n");
            APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_TYPE_NOT_UNDERSTOOD_BY_TERM, BIP_ADD_RST_NULL, 0);
            return osErrorParameter;
        }
    }
    else
    {
        BIP_PRINT_INFO("ProactiveCommand, Cannt Support No DestAddr\r\n");
        APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_PERFM_MISS_INFORM, BIP_ADD_RST_NULL, 0);
        return osErrorParameter;
    }
    // tTransLevel
    BIP_PRINT_INFO("ProactiveCommand, TransLevel.Tag : %u\r\n", pProactiveCommand->u.tOpenChan.tTransLevel.bTag);
    BIP_PRINT_INFO("ProactiveCommand, TransLevel.TransLevel : %u\r\n", pProactiveCommand->u.tOpenChan.tTransLevel.bLen);
    //USAT_BIP_PROTOCAL_TYPE_UDP USAT_BIP_PROTOCAL_TYPE_TCP
    BIP_PRINT_INFO("ProactiveCommand, TransLevel.ProtocolType : %u\r\n", pProactiveCommand->u.tOpenChan.tTransLevel.bProtocolType);
    BIP_PRINT_INFO("ProactiveCommand, TransLevel.PortNum : %u\r\n", pProactiveCommand->u.tOpenChan.tTransLevel.wPortNum);

    if(pProactiveCommand->u.tOpenChan.tTransLevel.bTag != 0)
    {
        if(pProactiveCommand->u.tOpenChan.tTransLevel.bProtocolType == USAT_BIP_PROTOCAL_TYPE_UDP)
        {
            domainType = SOCK_DGRAM;
            BIP_PRINT_INFO("ProactiveCommand, domainType : UDP\r\n");
        }
        else if(pProactiveCommand->u.tOpenChan.tTransLevel.bProtocolType == USAT_BIP_PROTOCAL_TYPE_TCP)
        {
            domainType = SOCK_STREAM;
            BIP_PRINT_INFO("ProactiveCommand, domainType : TCP\r\n");
        }
        else
        {
            BIP_PRINT_INFO("ProactiveCommand, Cannt Support No TransLevel.ProtocolType\r\n");
            APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_TYPE_NOT_UNDERSTOOD_BY_TERM, BIP_ADD_RST_TRANS_LEVEL_ERR, 0);
            return osErrorParameter;
        }

        if(pProactiveCommand->u.tOpenChan.tTransLevel.wPortNum != 0)
        {
            remotePort = pProactiveCommand->u.tOpenChan.tTransLevel.wPortNum;
        }
        else
        {
            BIP_PRINT_INFO("ProactiveCommand, Cannt Support No TransLevel.PortNum\r\n");
            APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_TYPE_NOT_UNDERSTOOD_BY_TERM, BIP_ADD_RST_TRANS_LEVEL_ERR, 0);
            return osErrorParameter;
        }
    }
    else
    {
        BIP_PRINT_INFO("ProactiveCommand, Cannt Support No TransLevel\r\n");
        APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_PERFM_MISS_INFORM, BIP_ADD_RST_TRANS_LEVEL_ERR, 0);
        return osErrorParameter;
    }

    //   这里的轮询还有优化空间
    while(!APP_BIP_NetUp(ipType) && (net_timeout < TIMES_FOR_NETWORK))
    {
        if(((net_timeout * NETWORK_TIMEOUT) % 60000) == 0)
        {   //   一分钟打一次
            BIP_PRINT_ERROR("ProactiveCommand, Wait Net Up %d\r\n", net_timeout);
        }
        net_timeout++;
        osThreadSleepRelaxed(osTickFromMs(NETWORK_TIMEOUT), osNoWait);
    }

    if (!APP_BIP_NetUp(ipType))
    {
        BIP_PRINT_ERROR("ProactiveCommand, Net Down\r\n");
        APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_TYPE_NOT_UNDERSTOOD_BY_TERM, BIP_ADD_RST_NULL, 0);
        return osErrorResource;
    }

    // stub begin
    #if APP_BIP_SERVER_STUB
    remotePort = APP_BIP_SERVER_STUB_PORT;
    ipaddr_aton(APP_BIP_SERVER_STUB_IP, &remoteIp);
    #endif
    // stub end

    ret = APP_BIP_USAT_OpenChannelProc(domainType, remoteIp, remotePort);
    if(ret == 0)
    {
        APP_BIP_Data *pData = NULL;
        APP_BIP_FreeDataList(&g_app_bip_simdata); //   释放后按新的buffersize申请
        if(osSlistIsEmpty(&g_app_bip_simdata))
        {
            pData = APP_BIP_AllocData(bufferSize);
            if(pData != NULL)
            {
                osSlistAppend(&g_app_bip_simdata, &pData->node);

                pProactiveCommand->u.tOpenChan.tBuffSize.wSize = bufferSize; //  响应时设置成真正生效的buffersize
                APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_PERFM_SUCCESS, 0, APP_BIP_GetCid());
            }
            else
            {
                APP_BIP_NET_Stop();
                BIP_PRINT_INFO("ProactiveCommand, Memory Error\r\n");
                APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_TERM_CURT_UNABLE_PROCESS, BIP_ADD_RST_NULL, 0);
            }
        }
    }
    else
    {
        BIP_PRINT_INFO("ProactiveCommand, Net Error %d\r\n", ret);
        APP_BIP_USAT_OpenChannelTerminalResponse(pProactiveCommand, USAT_RST_NETWORK_CURT_UNABLE_PROCESS, BIP_ADD_RST_UNREACHABLE, 0);
    }

    return ret;
}

static int APP_BIP_USAT_CloseChannelTerminalResponse(S_Usat_Proc_Cmd *pProactiveCommand)
{
    uint32_t terminalRsponseLen = 0;
    uint8_t terminalRsponsebuf[APP_BIP_TERMINAL_RESPONSE_MAX_LENGTH] = {0};
    int ret;

    terminalRsponseLen = Usat_EncTerminalResponse(terminalRsponsebuf, USAT_RST_PERFM_SUCCESS, NULL, NULL);

    ret = svc_usat_set_terminal_response((uint8_t *)terminalRsponsebuf, terminalRsponseLen);
    if(ret != 0)
    {
        BIP_PRINT_INFO("APP_BIP_USAT_CloseChannelTerminalResponse Send AT Error %d\r\n", ret);
    }

    return ret;
}
// close channel
static void APP_BIP_USAT_ProactiveCommandCloseChannel(S_Usat_Proc_Cmd *pProactiveCommand)
{
    APP_BIP_USAT_CloseChannelProc();
    APP_BIP_USAT_CloseChannelTerminalResponse(pProactiveCommand);
    APP_BIP_NotifyOffWork();
}

//   result是执行结果，当执行错误时要携带addResult
static int APP_BIP_USAT_ReceiveDataTerminalResponse(S_Usat_Proc_Cmd *pProactiveCommand, uint8_t result, uint8_t addResult, uint8_t *data, uint8_t dataLen, uint32_t leftLen)
{
    uint32_t terminalRsponseLen = 0;
    S_Usat_TR_ReceiveData receiveDataTR = {0};
    uint8_t terminalRsponsebuf[APP_BIP_TERMINAL_RESPONSE_MAX_LENGTH] = {0};
    int ret;

    if(result == 0)
    {
        receiveDataTR.tChData.bLen = dataLen;
        osMemcpy(receiveDataTR.tChData.abData, data, dataLen);
        receiveDataTR.tRemainDataLen = (uint8_t)(leftLen >= 255 ? 255 : leftLen); //  如果收到数据长度大于255，此处填255)
         BIP_PRINT_INFO("USAT_ReceiveDataResponse %d Data Begin 0x%x End 0x%x\r\n", dataLen, data[0], data[dataLen-1]);
        terminalRsponseLen = Usat_EncTerminalResponse(terminalRsponsebuf, USAT_RST_PERFM_SUCCESS, NULL, &receiveDataTR);
    }
    else
    {
        S_Usat_TR_AddRst addRst = {0}; //  BIP处理结果失败时，需要填写附加信息说明失败原因

        addRst.bLen = 1;
        addRst.abAddRst[0] = addResult;
        terminalRsponseLen = Usat_EncTerminalResponse(terminalRsponsebuf, result, &addRst, &receiveDataTR);
    }
    ret = svc_usat_set_terminal_response((uint8_t *)terminalRsponsebuf, terminalRsponseLen);
    if(ret != 0)
    {
        BIP_PRINT_INFO("APP_BIP_USAT_ReceiveDataTerminalResponse Send AT Error %d\r\n", ret);
    }

    return ret;
}
// receive data
static void APP_BIP_USAT_ProactiveCommandReceiveData(S_Usat_Proc_Cmd *pProactiveCommand)
{
    uint16_t sendLen = pProactiveCommand->u.tRcvData.tChDataLen.bDataSize;
    uint32_t leftLen;
    uint8_t *data;
    APP_BIP_Data* pData = APP_BIP_GetDataListFirst(&g_app_bip_netdata);
    uint32_t totalLen = APP_BIP_DataListLength(&g_app_bip_netdata);

    if(pData == NULL)
    {
        BIP_PRINT_INFO("APP_BIP_USAT_ProactiveCommandReceiveData NoData\r\n");
        APP_BIP_USAT_ReceiveDataTerminalResponse(pProactiveCommand, USAT_RST_TERM_CURT_UNABLE_PROCESS, BIP_ADD_RST_NULL, NULL, 0, 0);
        return;
    }

    //   buffer中剩余的数据不足SIM卡要求的数据，以剩余数据为准
    //   单个节点发送，不把两个节点数据混在一起发
    if(sendLen > pData->usedSize)
    {
        sendLen = pData->usedSize;
    }
    sendLen = (sendLen > USAT_RECEIVE_DATA_MAX_LEN) ? USAT_RECEIVE_DATA_MAX_LEN : sendLen;
    leftLen = pData->usedSize - sendLen;

    //leftLen = APP_BIP_DataListLength(&g_app_bip_netdata);
    //leftLen -= sendLen;//  总大小减去要发送的大小

    data = pData->data;

    BIP_PRINT_INFO("USAT CommandReceiveData first %d Send %d left %d total %d\r\n", pData->usedSize, sendLen,leftLen, totalLen);
    pData->status = APP_BIP_DATA_Sending;
    APP_BIP_USAT_ReceiveDataTerminalResponse(pProactiveCommand, USAT_RST_PERFM_SUCCESS, 0, data, sendLen, leftLen);

    //  更新buffer
    if(pData->usedSize == sendLen)
    {
        //  第一个节点数据发送完成，删除并释放
        osSlistRemove(&g_app_bip_netdata, &pData->node);
        osFree(pData);
    }
    else
    {
        //  第一个节点数据没有发送完成，移动数据
        memmove(pData->data, pData->data+sendLen, pData->usedSize - sendLen);
        pData->usedSize -= sendLen;
    }

}

//   result是执行结果，当执行错误时要携带addResult
static int APP_BIP_USAT_SendDataTerminalResponse(S_Usat_Proc_Cmd *pProactiveCommand, uint8_t result, uint8_t addResult, uint16_t leftLen)
{
    S_Usat_TR_SendData sendDataTR = {0};
    uint32_t terminalRsponseLen = 0;
    uint8_t terminalRsponsebuf[APP_BIP_TERMINAL_RESPONSE_MAX_LENGTH] = {0};
    int ret;

    sendDataTR.bBuffLeftLen = (uint8_t)(leftLen >= 255 ? 255 : leftLen); //数据缓存还有超过255字节的空间可用
    if(result == 0)
    {
        terminalRsponseLen = Usat_EncTerminalResponse(terminalRsponsebuf, USAT_RST_PERFM_SUCCESS, NULL, &sendDataTR);
    }
    else
    {
        S_Usat_TR_AddRst addRst = {0}; //  BIP处理结果失败时，需要填写附加信息说明失败原因

        addRst.bLen = 1;
        addRst.abAddRst[0] = addResult;
        terminalRsponseLen = Usat_EncTerminalResponse(terminalRsponsebuf, result, &addRst, &sendDataTR);
    }

    ret = svc_usat_set_terminal_response((uint8_t *)terminalRsponsebuf, terminalRsponseLen);
    if(ret != 0)
    {
        BIP_PRINT_INFO("APP_BIP_USAT_OpenChannelTerminalResponse Send AT Error %d\r\n", ret);
    }

    return ret;
}

// send data
static void APP_BIP_USAT_ProactiveCommandSendData(S_Usat_Proc_Cmd *pProactiveCommand)
{
    uint8_t *data;
    uint16_t len;
    uint16_t leftLen;
    APP_BIP_Data* pData = APP_BIP_GetDataListFirst(&g_app_bip_simdata);

    OS_ASSERT(pData != NULL);

    data = pProactiveCommand->u.tSendData.tChData.abData;
    len = pProactiveCommand->u.tSendData.tChData.bLen;
    BIP_PRINT_INFO("APP_BIP_USAT_ProactiveCommandSendData len %d\r\n", len);
    if(len != 0)
    {
        if(len > pData->totalSize - pData->usedSize)
        {
            BIP_PRINT_ERROR("APP_BIP_USAT_ProactiveCommandSendData SIM Data size %d GT %d\r\n", len, pData->totalSize - pData->usedSize);
            APP_BIP_USAT_SendDataTerminalResponse(pProactiveCommand, USAT_RST_TERM_CURT_UNABLE_PROCESS, BIP_ADD_RST_NULL, pData->totalSize - pData->usedSize);
            return;
        }
        else
        {
            memcpy(pData->data + pData->usedSize, data, len);
            pData->usedSize += len;
        }
    }
    leftLen = pData->totalSize - pData->usedSize;

    if(pProactiveCommand->tCmdDetail.uQual.tSendData.bSendImdt == USAT_SEND_DATA_IMMEDIATELY)
    {
        if (APP_BIP_USAT_SendDataProc(pData->data, pData->usedSize) <= 0)
        {
            BIP_PRINT_ERROR("APP_BIP_USAT_ProactiveCommandSendData send %d to NET Fail\r\n", pData->usedSize);
            APP_BIP_USAT_SendDataTerminalResponse(pProactiveCommand, USAT_RST_NETWORK_CURT_UNABLE_PROCESS, BIP_ADD_RST_CHAN_CLOSED, leftLen);
        }
        else
        {
            BIP_PRINT_INFO("APP_BIP_USAT_ProactiveCommandSendData send %d to NET success\r\n", pData->usedSize);

            APP_BIP_CleanData(pData); // 发成功了清空但不释放
            APP_BIP_USAT_SendDataTerminalResponse(pProactiveCommand, USAT_RST_PERFM_SUCCESS, 0, leftLen);
        }
    }
    else
    {
        BIP_PRINT_INFO("APP_BIP_USAT_ProactiveCommandSendData store data all %d left %d\r\n", pData->usedSize, leftLen);
        APP_BIP_USAT_SendDataTerminalResponse(pProactiveCommand, USAT_RST_PERFM_SUCCESS, 0, leftLen);
    }
}

static int APP_BIP_USAT_GetChannelStatusTerminalResponse(S_Usat_Proc_Cmd *pProactiveCommand)
{
    S_Usat_TR_GetChanStatus getChaStatTR = {0};
    uint32_t terminalRsponseLen = 0;
    uint8_t terminalRsponsebuf[APP_BIP_TERMINAL_RESPONSE_MAX_LENGTH] = {0};
    int ret;

    getChaStatTR.tChanStat.bChanId = APP_BIP_GetCid();   //通道号
    if(g_app_bip_socket_fd >= 0 && getChaStatTR.tChanStat.bChanId != 0)
    {
        getChaStatTR.tChanStat.bLinkEsted = 1;//连接正常
    }
    else
    {
        getChaStatTR.tChanStat.bLinkEsted = 0;
        //getChaStatTR.tChanStat.bFurtherInfo = USAT_CHANNEL_STAT_LINK_DROPED; //网络异常断开或用户手动断开
    }
    getChaStatTR.tChanStat.bFurtherInfo = 0;

    terminalRsponseLen = Usat_EncTerminalResponse(terminalRsponsebuf, USAT_RST_PERFM_SUCCESS, NULL, &getChaStatTR);

    ret = svc_usat_set_terminal_response((uint8_t *)terminalRsponsebuf, terminalRsponseLen);
    if(ret != 0)
    {
        BIP_PRINT_INFO("APP_BIP_USAT_OpenChannelTerminalResponse Send AT Error %d\r\n", ret);
    }

    return osOK;
}

//  get channel status
static void APP_BIP_USAT_ProactiveCommandGetChannelStatus(S_Usat_Proc_Cmd *pProactiveCommand)
{
    APP_BIP_USAT_GetChannelStatusTerminalResponse(pProactiveCommand);
}

//  setup event list  记录SIM卡支持有event，不要回terminal response, PS自己回
static void APP_BIP_USAT_ProactiveCommandSetupEventList(S_Usat_Proc_Cmd *pProactiveCommand)
{
    if(pProactiveCommand->u.tEventList.bChanStat)
    {
        APP_BIP_USAT_FLAGS_BIT_SET1(APP_BIP_USAT_FLAGS_CHANNEL_STATE_BIT_MASK);
    }

    if(pProactiveCommand->u.tEventList.bDataAvail)
    {
        APP_BIP_USAT_FLAGS_BIT_SET1(APP_BIP_USAT_FLAGS_DATA_AVAILABLE_BIT_MASK);
    }
}

static int APP_BIP_USAT_ProactiveCommandProc(SVC_USAT_ProactiveCommandInfo *info)
{
    S_Usat_Proc_Cmd proactiveCommand = {0};
    S_Usat_Proc_Cmd *pProactiveCommand = &proactiveCommand;
    uint8_t ret;

    BIP_PRINT_INFO("APP_BIP_USAT_ProactiveCommandProc\r\n");
    // 调用usat解码
    ret = Usat_DecProacCmd(&proactiveCommand, info->data, info->dataLen);
    if(ret == 0)
    {
        BIP_PRINT_INFO("APP_BIP_USAT_ProactiveCommandProc Type 0x%02x\r\n", proactiveCommand.tCmdDetail.bCmdType);
        if(proactiveCommand.tCmdDetail.bCmdType == USAT_PROAC_CMD_TYPE_OPEN_CHANNEL)
        {
            BIP_PRINT_INFO("APP_BIP_USAT Open Channel\r\n");
            APP_BIP_USAT_ProactiveCommandOpenChannel(pProactiveCommand);
        }
        else if(proactiveCommand.tCmdDetail.bCmdType == USAT_PROAC_CMD_TYPE_CLOSE_CHANNEL)
        {
            BIP_PRINT_INFO("APP_BIP_USAT Close Channel\r\n");
            APP_BIP_USAT_ProactiveCommandCloseChannel(pProactiveCommand);
        }
        else if(proactiveCommand.tCmdDetail.bCmdType == USAT_PROAC_CMD_TYPE_RECEIVE_DATA) // 发数据给SIM
        {
            BIP_PRINT_INFO("APP_BIP_USAT Receive Data\r\n");
            APP_BIP_USAT_ProactiveCommandReceiveData(pProactiveCommand);
        }
        else if(proactiveCommand.tCmdDetail.bCmdType == USAT_PROAC_CMD_TYPE_SEND_DATA) // 发数据给服务器
        {
            BIP_PRINT_INFO("APP_BIP_USAT Send Data\r\n");
            APP_BIP_USAT_ProactiveCommandSendData(pProactiveCommand);
        }
        else if(proactiveCommand.tCmdDetail.bCmdType == USAT_PROAC_CMD_TYPE_GET_CHANNEL_STATUS)
        {
            BIP_PRINT_INFO("APP_BIP_USAT Get Channel Status\r\n");
            APP_BIP_USAT_ProactiveCommandGetChannelStatus(pProactiveCommand);
        }
        else if(proactiveCommand.tCmdDetail.bCmdType == USAT_PROAC_CMD_TYPE_SETUP_EVENT_LIST)
        {
            BIP_PRINT_INFO("APP_BIP_USAT Setup Event List\r\n");
            APP_BIP_USAT_ProactiveCommandSetupEventList(pProactiveCommand);
        }
        else if(proactiveCommand.tCmdDetail.bCmdType == USAT_PROAC_CMD_TYPE_REFRESH)
        {
            BIP_PRINT_INFO("APP_BIP_USAT Refresh\r\n");
            //退出BIP应用？
        }
        else
        {
            BIP_PRINT_ERROR("ProactiveCommand, bCmdType : %u Not Process\r\n", proactiveCommand.tCmdDetail.bCmdType);
        }
    }
    else
    {
        BIP_PRINT_ERROR("ProactiveCommand, Usat decode fail : %u\r\n", ret);
    }

    return osOK;
}
//  主动式命令处理 end

//  Envelope 发送给卡 Begin
//  收到网侧数据时通知卡
//  cid通道号，
//  dataLen 数据长度
static int APP_BIP_USAT_EnvelopeDataAvailable(uint8_t cid, uint16_t dataLen)
{
    int ret;
    uint8_t evelopeLen = 0;
    uint8_t envelopebuf[APP_BIP_TERMINAL_RESPONSE_MAX_LENGTH] = {0};
    S_Usat_Env_DataAvailable dataAvailable = {0};

    if(!APP_BIP_USAT_FLAGS_BIT_TEST1(APP_BIP_USAT_FLAGS_DATA_AVAILABLE_BIT_MASK))
    {
        BIP_PRINT_INFO("APP_BIP_USAT_EnvelopeDataAvailable Not Support\r\n");
        return osOK;
    }

    dataAvailable.tChanStat.bChanId = cid;
    dataAvailable.tChanStat.bLinkEsted = 1;//连接已建立
    dataAvailable.tChDataLen = (dataLen >= 255 ? 255 : dataLen); //通道上收到的数据长度(如果收到数据长度大于255，此处填255)

    Usat_EncEnvelopeCommand(envelopebuf, &evelopeLen, USAT_DATA_AVAILABLE,  &dataAvailable);

    ret = svc_usat_set_envelope_command(envelopebuf, evelopeLen);
    if(ret != 0)
    {
        BIP_PRINT_INFO("APP_BIP_USAT_EnvelopeDataAvailable Send AT Error %d\r\n", ret);
    }
    return ret;
}

//  网络断开时通知卡
//  cid通道号，
//  linkstatus 连接状态 ：1连接，0断开
static int APP_BIP_USAT_EnvelopeChannelStatus(uint8_t cid, uint8_t linkstatus)
{
    int ret;
    uint8_t evelopeLen = 0;
    uint8_t envelopebuf[APP_BIP_TERMINAL_RESPONSE_MAX_LENGTH] = {0};
    S_Usat_Env_ChannelStatus chanStat = {0};

    if(!APP_BIP_USAT_FLAGS_BIT_TEST1(APP_BIP_USAT_FLAGS_CHANNEL_STATE_BIT_MASK))
    {
        BIP_PRINT_INFO("APP_BIP_USAT_EnvelopeChannelStatus Not Support\r\n");
        return osOK;
    }

    chanStat.tChanStat.bChanId = cid;
    chanStat.tChanStat.bLinkEsted = linkstatus;
    if(linkstatus == 0) //连接断开
    {
        chanStat.tChanStat.bFurtherInfo = USAT_CHANNEL_STAT_LINK_DROPED; //补充说明：连接已断开
    }

    Usat_EncEnvelopeCommand(envelopebuf, &evelopeLen, USAT_CHANNEL_STATUS,  &chanStat);

    ret = svc_usat_set_envelope_command(envelopebuf, evelopeLen);
    if(ret != 0)
    {
        BIP_PRINT_INFO("APP_BIP_USAT_EnvelopeChannelStatus Send AT Error %d\r\n", ret);
    }
    return ret;
}

static void APP_BIP_USAT_DataAvailable(void)
{
    APP_BIP_Data* pData = APP_BIP_GetDataListFirst(&g_app_bip_netdata);
    if(pData != NULL)
    {
        //  查看状态,没有通知卡
        if(pData->status == APP_BIP_DATA_IDLE)
        {
            BIP_PRINT_INFO("APP_BIP_USAT_DataAvailable len %d\r\n", pData->usedSize);
            APP_BIP_USAT_EnvelopeDataAvailable(APP_BIP_GetCid(), pData->usedSize);
            pData->status = APP_BIP_DATA_NOTIFY;
        }
    }
}

//  Envelope 发送给卡 End

// usat end 处理 begin
static int APP_BIP_USAT_EndProc(void)
{
    APP_BIP_USAT_DataAvailable(); // 检查是否还有数据

    return osOK;
}
// usat end 处理 end

#define USIM_END
#define NET_BEGIN
//  跟服务器交互 Begin
//  socket回调
static void APP_BIP_NET_SocketCallBack(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param)
{
    APP_BIP_Event_Info bipEvent = {0};

    BIP_PRINT_INFO("APP_BIP_NET_SocketCallBack : event [%d]\r\n", event);
    switch(event)
    {
        case SOCKET_TCP_EVENT_SENT:
            bipEvent.eventID = APP_BIP_NET_SENT_DATA;
            APP_BIP_EventSend(bipEvent, osWaitForever);
            break;

        case SOCKET_TCP_EVENT_RECV:
            if(len != 0)
            {
                bipEvent.eventID = APP_BIP_NET_RECEIVE_DATA;
                bipEvent.ptrParam = (void *)(intptr_t)len;
                APP_BIP_EventSend(bipEvent, 0);
            }
            else
            {
                bipEvent.eventID = APP_BIP_NET_DISCONNECT;
                APP_BIP_EventSend(bipEvent, 0);
            }
            break;

        case SOCKET_TCP_EVENT_CONNECTED:
            break;

        case SOCKET_TCP_EVENT_ERR:
        case SOCKET_NETIF_EVENT_DOWN:
            bipEvent.eventID = APP_BIP_NET_ERROR;
            APP_BIP_EventSend(bipEvent, osWaitForever);
            break;

        default:
            BIP_PRINT_INFO("APP_BIP_NET_SocketCallBack : event out of range [%d]\r\n", event);
            break;
    }

    return;
}

//  收到服务器的数据
static void APP_BIP_NET_ReceiveDataProc(int fd, int len)
{
    APP_BIP_Data *pData = NULL;
    int recv_len = 0;

    if(len == 0)
    {
        return;
    }

    pData = APP_BIP_AllocData(len);
    if(pData != NULL)
    {
        recv_len = lwip_recv(fd, pData->data, len, MSG_DONTWAIT);
        OS_ASSERT(recv_len == len);
        pData->usedSize = len;
    }
    else
    {
        // 内存没有了，怎么办？
        BIP_PRINT_ERROR("APP_BIP_NET_ReceiveDataProc Memory fail[%d]\r\n", len);
        OS_ASSERT(0);
        return;
    }
    BIP_PRINT_INFO("APP_BIP_NET_ReceiveDataProc len[%d] read[%d]\r\n", len, recv_len);

    // 这是个错误，说好的有数据，怎么读不到??
    if (recv_len <= 0)
    {
        return;
    }
    pData->status = APP_BIP_DATA_IDLE;
    osSlistAppend(&g_app_bip_netdata, &pData->node);

    APP_BIP_USAT_DataAvailable();

    return;
}

// 创建socket并连接服务器
static int APP_BIP_NET_Start(uint8_t domainType, ip_addr_t remoteIp, uint16_t remotePort)
{
    int fd = -1;
    int ret = -1;

    if(g_app_bip_socket_fd >= 0)
    {
        return osErrorResourceFull;
    }

    if (IP_IS_V4(&remoteIp))
    {
        fd = lwip_socket(AF_INET, domainType, IPPROTO_IP);
    }
    else
    {
        fd = lwip_socket(AF_INET6, domainType, IPPROTO_IP);
    }

    if (fd < 0)
    {
        BIP_PRINT_ERROR("APP_BIP_NET_Start socket fail\r\n");
        return osErrorResource;
    }

    if (IP_IS_V4(&remoteIp)) /* IPV4 */
    {
        struct sockaddr_in remoteSockAddr;
        memset(&remoteSockAddr, 0, sizeof(struct sockaddr_in));

        remoteSockAddr.sin_len = sizeof(remoteSockAddr);
        remoteSockAddr.sin_family = AF_INET;
        memcpy(&(remoteSockAddr.sin_addr), ip_2_ip4(&(remoteIp)), sizeof(struct in_addr));
        remoteSockAddr.sin_port = htons(remotePort);

        ret = lwip_connect(fd, (struct sockaddr *)&remoteSockAddr, sizeof(struct sockaddr));
    }
    else /* IPV6 */
    {
        struct sockaddr_in6 remoteSockAddr6;
        memset(&remoteSockAddr6, 0, sizeof(struct sockaddr_in6));

        remoteSockAddr6.sin6_len = sizeof(remoteSockAddr6);
        remoteSockAddr6.sin6_family = AF_INET6;
        memcpy(&(remoteSockAddr6.sin6_addr),ip_2_ip6(&(remoteIp)),sizeof(struct in6_addr));
        remoteSockAddr6.sin6_port = htons(remotePort);

        ret = lwip_connect(fd, (struct sockaddr *)&remoteSockAddr6, sizeof(struct sockaddr));
    }
    if (ret >= 0)
    {
        //int flags = 0;

        lwip_socket_register_callback(fd, APP_BIP_NET_SocketCallBack, NULL);
        //flags = lwip_fcntl(fd, F_GETFL, 0);
        //lwip_fcntl(fd, F_SETFL, flags | O_NONBLOCK); 不处理成非阻塞
        g_app_bip_socket_fd = fd;
    }
    else
    {
        BIP_PRINT_ERROR("APP_BIP_NET_Start socket connect fail\r\n");
        lwip_close(fd);
    }

    return ret;
}

//   socket send
static int APP_BIP_NET_Send(uint8_t *data, uint16_t len)
{
    int ret = osError;

    if(g_app_bip_socket_fd >= 0)
    {
        ret = lwip_send(g_app_bip_socket_fd, data, len, 0);
    }
    return ret;
}

//  close socket
static void APP_BIP_NET_Stop(void)
{
    if (g_app_bip_socket_fd >= 0)
    {
        lwip_socket_unregister_callback(g_app_bip_socket_fd);//CM 14297,回调函数残留在socket中
        lwip_close(g_app_bip_socket_fd);
        g_app_bip_socket_fd = -1;
    }

    return;
}
//  跟服务器交互 End
#define NET_END
#define TASK_BEGIN
//  进程间通信 Begin
/**
 ************************************************************************************
 * @brief 收消息
 *
 * @param[out]    event    事件结构
 * @param[in]    timeout 超时参数     osWaitForever or osNoWait
 *
 * @return
 * @note
 ************************************************************************************
*/
static int APP_BIP_EventRecv(APP_BIP_Event_Info *event, uint32_t timeout)
{
    osStatus_t err;

    if(g_app_bip_message_q != NULL)
    {
        err = osMessageQueueGetRelaxed(g_app_bip_message_q, (void *)event, 0, timeout, osWaitForever);
        return err;
    }
    else
    {
        BIP_PRINT_INFO("APP_BIP_EventRecv Queue NULL\r\n");
        return osErrorResource;
    }
}


static int APP_BIP_MessageQueue_Init(void)
{
    osMessageQueueAttr_t msgQueueAttr = {"app_bip", 0U, NULL, 0U, NULL, 0U};

    if(g_app_bip_message_q == NULL)
    {
        g_app_bip_message_q = osMessageQueueNew(APP_BIP_MESSAGE_QUEUE_MSG_MAX, sizeof(APP_BIP_Event_Info), &msgQueueAttr);
        OS_ASSERT(g_app_bip_message_q != NULL);
        if(OS_NULL == g_app_bip_message_q)
        {
            BIP_PRINT_ERROR("APP_BIP_MessageQueue_Init fail\r\n");
            return osError;
        }
        return osOK;
    }
    else
    {
        return osOK;
    }
}

static void APP_BIP_MessageQueue_DeInit(void)
{
    if(g_app_bip_message_q != NULL)
    {
        APP_BIP_Event_Info event;
        // 释放所有缓存消息的内存
        while (1)
        {
            if (osOK != APP_BIP_EventRecv(&event, osNoWait))
            {
                break;
            }
            else
            {
                if(event.ptrParam != NULL)
                {
                    osFree(event.ptrParam);
                }
            }
        }

        osMessageQueueDelete(g_app_bip_message_q);
        g_app_bip_message_q = NULL;
    }
}

static void APP_BIP_DataBuf_DeInit(void)
{
    APP_BIP_FreeDataList(&g_app_bip_netdata);
    osSlistInit(&g_app_bip_netdata);
    APP_BIP_FreeDataList(&g_app_bip_simdata);
    osSlistInit(&g_app_bip_simdata);
}

/**
 ************************************************************************************
 * @brief   发消息通知任务退出
 *
 * @param[in] none
 *
 * @return
 * @note
 ************************************************************************************
*/
static void APP_BIP_NotifyOffWork(void)
{
    APP_BIP_Event_Info event = {0};
    event.eventID = APP_BIP_OFF_WORK;

    APP_BIP_USAT_FLAGS_BIT_SET1(APP_BIP_USAT_FLAGS_OFF_WORK_BIT_MASK);
    APP_BIP_EventSend(event, osNoWait);
}

/**
 ************************************************************************************
 * @brief   任务退出时清理资源
 *
 * @param[in] none
 *
 * @return
 * @note
 ************************************************************************************
*/
static void APP_BIP_Task_Exit(void)
{
    if(g_app_bip_task == NULL)
    {
        return;
    }

    APP_BIP_MessageQueue_DeInit();
    APP_BIP_NET_Stop();
    APP_BIP_DataBuf_DeInit();
    g_app_bip_task = NULL;

    BIP_PRINT_INFO("APP_BIP_Task_Exit\r\n");
}
//  任务入口
static void APP_BIP_TaskEntry(void *argument)
{
    APP_BIP_Event_Info event;

    BIP_PRINT_INFO("APP_BIP_TaskEntry running\r\n");

    while (1)
    {
        if (osOK != APP_BIP_EventRecv(&event, osWaitForever))
        {
            BIP_PRINT_INFO("APP_BIP_TaskEntry recv err\r\n");
            osThreadSleepRelaxed(osTickFromMs(1000), osWaitForever);
            continue;
        }

        switch(event.eventID)
        {
            case APP_BIP_USAT_PROACTIVE_COMMAND:
            {
                SVC_USAT_ProactiveCommandInfo *info = event.ptrParam;
                BIP_PRINT_INFO("APP_BIP_TaskEntry proactive command length[%d]\r\n", info->dataLen);
                APP_BIP_USAT_ProactiveCommandProc(info);
            }
            break;

            case APP_BIP_USAT_END:
            {
                BIP_PRINT_INFO("APP_BIP_TaskEntry usat end\r\n");
                APP_BIP_USAT_EndProc();
            }
            break;

            case APP_BIP_NET_RECEIVE_DATA:
            {
                int len = (int)(intptr_t)event.ptrParam;
                APP_BIP_NET_ReceiveDataProc(g_app_bip_socket_fd, len);
                event.ptrParam = NULL;
            }
            break;

            case APP_BIP_NET_SENT_DATA:
            {

            }
            break;

            case APP_BIP_NET_ERROR:
            {
                APP_BIP_NET_Stop();
                APP_BIP_USAT_EnvelopeChannelStatus(APP_BIP_GetCid(), 0);
            }
            break;

            default:
            {
                BIP_PRINT_INFO("APP_BIP_TaskEntry recv_mq not support:%d\r\n", event.eventID);
            }
            break;
        }

        if(event.ptrParam != NULL)
        {
            osFree(event.ptrParam);
        }

        if(event.eventID == APP_BIP_OFF_WORK || APP_BIP_USAT_FLAGS_BIT_TEST1(APP_BIP_USAT_FLAGS_OFF_WORK_BIT_MASK)) // 退出任务
        {
            APP_BIP_USAT_FLAGS_BIT_CLEAR0(APP_BIP_USAT_FLAGS_OFF_WORK_BIT_MASK);
            APP_BIP_Task_Exit();
            return;
        }

    }
    return;
}

static int APP_BIP_Task_Init(void)
{
    osThreadAttr_t attr = {"app_bip", osThreadDetached, NULL, 0U, NULL, APP_BIP_TASK_STACK_SIZE, APP_BIP_TASK_PRIORITY, 0U, 0U};

    if(g_app_bip_task == NULL)
    {
        g_app_bip_task = osThreadNew(APP_BIP_TaskEntry, NULL, &attr);
        OS_ASSERT(g_app_bip_task != NULL);
        if(g_app_bip_task == NULL)
        {
            BIP_PRINT_ERROR("APP_BIP_Task_Init fail\r\n");
            return osError;
        }
        return osOK;
    }
    else
    {
        return osOK;
    }
}

static bool APP_BIP_Init_already(void)
{
    return g_app_bip_task != NULL;
}
/************************************************************************************
 *                                 外部函数定义
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
void APP_BIP_Init(void)
{
    int ret;

    if(APP_BIP_Init_already())
    {
        return;
    }

    ret = APP_BIP_MessageQueue_Init();
    if(ret != osOK)
    {
        return;
    }

    ret = APP_BIP_Task_Init();
    if(ret != osOK)
    {
        APP_BIP_MessageQueue_DeInit();
        APP_BIP_DataBuf_DeInit();
        return;
    }
    BIP_PRINT_INFO("APP_BIP_Init Success\r\n");
}
/**
 ************************************************************************************
 * @brief 发消息
 *
 * @param[in]    eventId    事件ID
 * @param[in]    param      事件对应的参数
 * @param[in]    timeout 超时   osWaitForever or osNoWait
 *
 * @return
 * @note
 ************************************************************************************
*/
int APP_BIP_EventSend(APP_BIP_Event_Info event, uint32_t timeout)
{
    osStatus_t err;

    if(g_app_bip_message_q != NULL)
    {
        err = osMessageQueuePutRelaxed(g_app_bip_message_q, (void *)&event, 0, timeout, osWaitForever);
    }
    else
    {
        err = osErrorIO;
    }

    return err;
}
// 进程间通信 end
#define TASK_END

#endif // APP_BIP_SUPPORT

