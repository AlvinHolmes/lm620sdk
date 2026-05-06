/*******************************************************************************
 * Copyright (c) 2014, 2017 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *   Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *   Ian Craggs - fix for #96 - check rem_len in readPacket
 *   Ian Craggs - add ability to set message handler separately #6
 *   XiaoWei Ji - modify the definition of Timer to adapt OneOS, so release it while end of use anywhere
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "MQTTClient.h"
#include "mqtt_api.h"
#include "lwip/sockets.h"


#define MQTT_DEFAULT_KEEP_ALIVE_INTERVAL        120     // 默认保活时间，单位s
#define MQTT_DEFAULT_PING_INTERVAL              120     // 默认心跳间隔时间，单位s
#define MQTT_DEFAULT_CLEAN_SESSION              0       // 默认会话类型
#define MQTT_DEFAULT_RETRANS_INTERVAL           20      // 默认数据包重传初始间隔时间，单位s
#define MQTT_DEFAULT_RETRANS_RETRY_TIMES        0       // 默认数据包传输超时后重发次数
#define MQTT_DEFAULT_PING_INTERVAL              120     // 默认心跳间隔时间，单位s
#define MQTT_DEFAULT_RECONN_TIMES               3       // 默认重连次数
#define MQTT_DEFAULT_RECONN_INTERVAL            20      // 默认重连间隔
#define MQTT_DEFAULT_RECONN_MODE                0       // 默认重连策略
#define MQTT_DEFAULT_CACHE_MODE                 0       // 接收MQTT消息缓存模式默认关
#define MQTT_MIN_COMMAND_TIMEOUT                5000    // 单位ms


/**
 * MqttContainerOf - return the member address of ptr, if the type of ptr is the
 * struct type.
 */
#define MqttContainerOf(ptr, type, member)      ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

osCompletion g_mqttCmpl;
osSlist_t g_mqttEventList; // 通过锁中断保护
Thread g_mqttThread = NULL;
MQTTClient* g_mqttClients[MQTT_MAX_CLIENT_CNT];
static Mutex g_mqttWorkMutex; // 保护MQTT任务和其他任务互斥
static MQTTCustomerClientIsValid g_mqttCustomerClientCheck = NULL; // 用户函数用于反馈Client句柄是否有效

int MQTT_Init(void);
int MQTTProcConnack(MQTTClient* c, MQTTConnackData* data);
int MQTTProcSuback(MQTTClient* c, int topicCnt, const char* topics[], int QoSs[],
    MQTT_MessageHandler messageHandler, MQTTSubackData* data);
void MQTTProcUnsubscribe(MQTTClient* c, int count, const char* topicFilters[]);
void MQTTProcPublishResponse(MQTTClient* c, int packetType, const char* topicName, MQTT_Message* message);
void MQTTProcConnEvent(MQTT_EventInfo *eventInfo);
void MQTTProcReConnEvent(MQTT_EventInfo *eventInfo);
void MQTTProcSubEvent(MQTT_EventInfo *eventInfo);
void MQTTProcUnsubEvent(MQTT_EventInfo *eventInfo);
void MQTTProcPubEvent(MQTT_EventInfo *eventInfo);
void MQTTProcPubResponseTimeoutEvent(MQTT_EventInfo *eventInfo);
void MQTTProcPubCompTimeoutEvent(MQTT_EventInfo *eventInfo);
void MQTTProcDiscEvent(MQTT_EventInfo *eventInfo);
void MQTTProcRecvEvent(MQTT_EventInfo *eventInfo);
int keepalive(MQTTClient* c);
void MQTTCloseSession(MQTTClient* c, int reconnFlag);
void MQTT_EventDefaultCallback(MQTT_Handle handle, void *userData, MQTTEventType eventType);
void MQTTSetReqInfo(MQTTClient* c, MQTT_EventInfo *eventInfo);
void MQTT_SocketRecvEventCallback(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param);

static char* MQTT_EventString(MQTTEventType event)
{
    switch(event)
    {
        case MQTT_EVENT_ERROR :                     return "ERROR";
        case MQTT_EVENT_TIMEOUT :                   return "TIMEOUT";
        case MQTT_EVENT_UNKNOWN :                   return "UNKNOWN";
        case MQTT_EVENT_CONN :                      return "CONN";
        case MQTT_EVENT_RECONN :                    return "RECONN";
        case MQTT_EVENT_CONN_ERR :                  return "CONN_ERR";
        case MQTT_EVENT_CONNACK :                   return "CONNACK";
        case MQTT_EVENT_CONN_TIMEOUT :              return "CONN_TIMEOUT";
        case MQTT_EVENT_RECONN_TIMEOUT :            return "RECONN_TIMEOUT";
        case MQTT_EVENT_PUB :                       return "PUB";
        case MQTT_EVENT_PUB_QOS0 :                  return "PUB_QOS0";
        case MQTT_EVENT_PUB_ERR :                   return "PUB_ERR";
        case MQTT_EVENT_PUBACK :                    return "PUBACK";
        case MQTT_EVENT_PUBACK_TIMEOUT :            return "PUBACK_TIMEOUT";
        case MQTT_EVENT_PUBACK_TIMEOUT_RETRANS :    return "PUBACK_TIMEOUT_RETRANS";
        case MQTT_EVENT_PUBREC :                    return "PUBREC";
        case MQTT_EVENT_PUBREC_TIMEOUT :            return "PUBREC_TIMEOUT";
        case MQTT_EVENT_PUBREC_TIMEOUT_RETRANS :    return "PUBREC_TIMEOUT_RETRANS";
        case MQTT_EVENT_PUBREL :                    return "PUBREL";
        case MQTT_EVENT_PUBCOMP :                   return "PUBCOMP";
        case MQTT_EVENT_PUBCOMP_TIMEOUT :           return "PUBCOMP_TIMEOUT";
        case MQTT_EVENT_PUBCOMP_TIMEOUT_RETRANS :   return "PUBCOMP_TIMEOUT_RETRANS";
        case MQTT_EVENT_PUB_TIMEOUT :               return "PUB_TIMEOUT";
        case MQTT_EVENT_SUB :                       return "SUB";
        case MQTT_EVENT_SUB_ERR :                   return "SUB_ERR";
        case MQTT_EVENT_SUBACK :                    return "SUBACK";
        case MQTT_EVENT_SUB_TIMEOUT :               return "SUB_TIMEOUT";
        case MQTT_EVENT_UNSUB :                     return "UNSUB";
        case MQTT_EVENT_UNSUB_ERR :                 return "UNSUB_ERR";
        case MQTT_EVENT_UNSUBACK :                  return "UNSUBACK";
        case MQTT_EVENT_UNSUB_TIMEOUT :             return "UNSUB_TIMEOUT";
        case MQTT_EVENT_PINGREQ :                   return "PINGREQ";
        case MQTT_EVENT_PINGRESP :                  return "PINGRESP";
        case MQTT_EVENT_DISC :                      return "DISC";
        case MQTT_EVENT_DISC_OK :                   return "DISC_OK";
        case MQTT_EVENT_NET_RECV :                  return "NET_RECV";
        case MQTT_EVENT_SESSION_CLOSED :            return "SESSION_CLOSED";
        case MQTT_EVENT_TIMER_TIMEOUT :             return "TIMER_TIMEOUT";
        case MQTT_EVENT_DROP :                      return "DROP";
        case MQTT_EVENT_NEW_MSG_RECV :              return "NEW_MSG_RECV";
        case MQTT_EVENT_NET_CLOSE :                 return "NET_CLOSE";
        default :                                   return "";
    }
}

static void NewMessageData(MQTT_MessageData* md, MQTTString* aTopicName, MQTT_Message* aMessage) {
    md->topicName = (MQTT_String *)aTopicName;
    md->message = aMessage;
}

static void MQTTGetConnectData(MQTTPacket_connectData *connectData, MQTT_ConnectOption *options)
{
    osMemcpy(connectData->struct_id, options->struct_id, sizeof(options->struct_id));

    connectData->struct_version = options->struct_version;
    connectData->MQTTVersion = options->MQTTVersion;

    connectData->clientID.cstring = options->clientID.cstring;
    connectData->clientID.lenstring.len = options->clientID.lenstring.len;
    connectData->clientID.lenstring.data = options->clientID.lenstring.data;

    connectData->keepAliveInterval = options->keepAliveInterval;
    connectData->cleansession = options->cleansession;
    connectData->willFlag = options->willFlag;

    osMemcpy(connectData->will.struct_id, options->will.struct_id, sizeof(options->struct_id));
    connectData->will.struct_version = options->will.struct_version;
    connectData->will.topicName.cstring = options->will.topicName.cstring;
    connectData->will.topicName.lenstring.len = options->will.topicName.lenstring.len;
    connectData->will.topicName.lenstring.data = options->will.topicName.lenstring.data;
    connectData->will.message.cstring = options->will.message.cstring;
    connectData->will.message.lenstring.len = options->will.message.lenstring.len;
    connectData->will.message.lenstring.data = options->will.message.lenstring.data;
    connectData->will.retained = options->will.retained;
    connectData->will.qos = options->will.qos;

    connectData->username.cstring = options->username.cstring;
    connectData->username.lenstring.len = options->username.lenstring.len;
    connectData->username.lenstring.data = options->username.lenstring.data;

    connectData->password.cstring = options->password.cstring;
    connectData->password.lenstring.len = options->password.lenstring.len;
    connectData->password.lenstring.data = options->password.lenstring.data;
}


static int getNextPacketId(MQTTClient *c) {
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}

static int getCurrentPacketId(MQTTClient *c) {
    return c->next_packetid;
}

static int sendPacket(MQTTClient* c, int length, Timer* timer)
{
    int rc = FAILURE,
        sent = 0;

    while (sent < length && !TimerIsExpired(timer) && NULL != c && NULL != c->ipstack)
    {
        rc = c->ipstack->mqttwrite(c->ipstack, &c->buf[sent], length, TimerLeftMS(timer));
        if (rc < 0)  // there was an error writing the data
            break;
        sent += rc;
    }
    if (sent == length)
    {
        // MQTT_PRINT_INFO("Start mqtt keepAliveTimer, client:%p!\r\n", c);
        TimerCountdown(&c->keepAliveTimer, c->pingInterval);
        rc = SUCCESS;
    }
    else
        rc = FAILURE;
    return rc;
}

//  需要在获取全局锁或屏蔽中断下使用
static MQTTClient* MQTT_SocketFindClient(int fd)
{
    MQTTClient *client = NULL;
    int i;

    for (i = 0; i < MQTT_MAX_CLIENT_CNT; i++)
    {
        if ((NULL != g_mqttClients[i]) && (NULL != (g_mqttClients[i])->ipstack)
            && (fd == (g_mqttClients[i])->ipstack->fd))
        {
            client = g_mqttClients[i];
            break;
        }
    }

    return client;
}

static bool MQTT_ClientIsValid(MQTT_Handle handle)
{
    int i;
    bool ret = false;

    if(handle == NULL)
    {
        return false;
    }

    for (i = 0; i < MQTT_MAX_CLIENT_CNT; i++)
    {
        if ((NULL != g_mqttClients[i]) && (handle == (MQTT_Handle)g_mqttClients[i]))
        {
            ret = true;
            break;
        }
    }

    //  无效时，调用用户的API检测Client是否有效
    if(!ret && g_mqttCustomerClientCheck != NULL)
    {
        ret = g_mqttCustomerClientCheck(handle);
    }

    return ret;
}

static void MQTTFreeProcessingReq(MQTTClient *c)
{
    MutexLock(&c->mutex);

    c->reqInfo.eventId = MQTT_EVENT_UNKNOWN;
    c->reqInfo.reqEndTick = osTickGet();
    if (c->reqInfo.buf)
    {
        osFree(c->reqInfo.buf);
        c->reqInfo.buf = NULL;
    }
    MQTT_PRINT_INFO("MQTTFreeProcessingReq, reqEndTick:%u, client:%p!\r\n",
        (uint32_t)(c->reqInfo.reqEndTick & 0xFFFFFFFF), c);

    MutexUnlock(&c->mutex);
}

void MQTT_WorkMutexInit(void)
{
    static int workmutex_init = OS_FALSE;
    if(!workmutex_init)
    {
        osPrintf("MQTT WorkMutexInit\r\n");
        MutexInit(&g_mqttWorkMutex);
        workmutex_init = OS_TRUE;
    }
}

void MQTT_CustomerClientCheckSet(MQTTCustomerClientIsValid func)
{
    g_mqttCustomerClientCheck = func;
}

MQTT_EventInfo* MQTTPopEvent()
{
    MQTT_EventInfo *eventInfo = NULL;
    struct osSlistNode *pNode = NULL;
    ubase_t level;

    level = osInterruptDisable();
    if (!osSlistIsEmpty(&g_mqttEventList))
    {
        pNode = osSlistFirst(&g_mqttEventList);
        osSlistRemove(&g_mqttEventList, pNode);
    }
    osInterruptEnable(level);

    eventInfo = (MQTT_EventInfo *)pNode;

    if(eventInfo != NULL)
    {
        //  这两事件中携带的fd需要转换成client句柄
        if(eventInfo->eventId == MQTT_EVENT_NET_RECV || eventInfo->eventId == MQTT_EVENT_NET_CLOSE)
        {
            eventInfo->client = MQTT_SocketFindClient((int)(intptr_t)eventInfo->client);
        }
    }

    return eventInfo;
}

void MQTTPushEvent(MQTT_EventInfo *eventInfo)
{
    ubase_t level;
    level = osInterruptDisable();
    osSlistAppend(&g_mqttEventList, &eventInfo->node);
    osInterruptEnable(level);

    osComplete(&g_mqttCmpl);
}

// 删除事件链表中跟 client 相关的事件
void MQTTCleanEvent(MQTTClient *client)
{
    MQTT_EventInfo *eventInfo = NULL;
    struct osSlistNode *pNode = NULL;
    ubase_t level;

    level = osInterruptDisable();
    if (!osSlistIsEmpty(&g_mqttEventList))
    {
        osSlistForEach(pNode, &g_mqttEventList)
        {
            eventInfo = (MQTT_EventInfo *)pNode;
            if(eventInfo->eventId == MQTT_EVENT_NET_RECV || eventInfo->eventId == MQTT_EVENT_NET_CLOSE)
            {
                if(client == MQTT_SocketFindClient((int)(intptr_t)eventInfo->client))
                {
                    eventInfo->eventId = MQTT_EVENT_UNKNOWN;
                    eventInfo->client = NULL;
                }
            }
            else
            {
                if(eventInfo->client == client)
                {
                    eventInfo->eventId = MQTT_EVENT_UNKNOWN;
                    eventInfo->client = NULL;
                }
            }
        }
    }
    osInterruptEnable(level);
}

void MQTTTimerOutEvent(MQTTClient *client, MQTTTimerOutProcFunc fp)
{
    MQTT_EventInfo *eventInfo = osMalloc(sizeof(MQTT_EventInfo) + sizeof(MQTT_TimerOutInfo));
    MQTT_TimerOutInfo *timerOutInfo = NULL;
    if (eventInfo)
    {
        eventInfo->client = client;
        eventInfo->eventId = MQTT_EVENT_TIMER_TIMEOUT;
        timerOutInfo = (MQTT_TimerOutInfo *)eventInfo->buf;
        timerOutInfo->fp = fp;

        MQTTPushEvent(eventInfo);
    }
}

// 在MQTT异步任务中调用
void MQTTProcRetransTimeoutEvent(MQTTClient* client)
{
    MQTT_PRINT_INFO("Mqtt retrans-timer timeout, client:%p!\r\n", client);

    if (client->reqInfo.buf)
    {
        MQTT_EventInfo *eventInfo = (MQTT_EventInfo *)client->reqInfo.buf;
        switch (eventInfo->eventId)
        {
        case MQTT_EVENT_PUB:
            {
                MQTT_PRINT_INFO("MQTTAsyncProcTask:wait puback/pubrec timeout event, client:%p!\r\n", client);
                MQTTProcPubResponseTimeoutEvent(eventInfo);
            }
            break;

        case MQTT_EVENT_PUBREL:
            {
                MQTT_PRINT_INFO("MQTTAsyncProcTask:wait pubcomp timeout event, client:%p!\r\n", client);
                MQTTProcPubCompTimeoutEvent(eventInfo);
            }
            break;

        default:
            break;
        }
    }
}

void MQTTRetransTimeout(void *parameter)
{
    MQTTClient *client = NULL;

    OS_ASSERT(NULL != parameter);
    client = MqttContainerOf(parameter, MQTTClient, retransTimer);

    MQTTTimerOutEvent(client, MQTTProcRetransTimeoutEvent);
}

void MQTTReportTimeoutEvent(MQTTClient* client)
{
    int reportTimeout = OS_TRUE;

    if ((NULL != client->eventReport) && (NULL != client->reqInfo.buf))
    {
        void *userData = NULL;
        MQTTEventType eventType = MQTT_EVENT_UNKNOWN;
        switch (client->reqInfo.eventId)
        {
        case MQTT_EVENT_CONN:
            {
                eventType = MQTT_EVENT_CONN_TIMEOUT;
            }
            break;

        case MQTT_EVENT_RECONN:
            {
                eventType = MQTT_EVENT_RECONN_TIMEOUT;
            }
            break;

        case MQTT_EVENT_SUB:
            {
                MQTT_SubInfo *subInfo = (MQTT_SubInfo *)(client->reqInfo.buf + sizeof(MQTT_EventInfo));
                userData = &subInfo->id;
                eventType = MQTT_EVENT_SUB_TIMEOUT;
            }
            break;

        case MQTT_EVENT_UNSUB:
            {
                MQTT_UnsubInfo *unsubInfo = (MQTT_UnsubInfo *)(client->reqInfo.buf + sizeof(MQTT_EventInfo));
                userData = &unsubInfo->id;
                eventType = MQTT_EVENT_UNSUB_TIMEOUT;
            }
            break;

        case MQTT_EVENT_PUB:
        case MQTT_EVENT_PUBREL:
            {
                eventType = MQTT_EVENT_PUB_TIMEOUT;
            }
            break;

        default:
            {
                reportTimeout = OS_FALSE;
                MQTT_PRINT_INFO("No timeout event, mqtt processingReq eventId:%d, client:%p!\r\n", client->reqInfo.eventId, client);
            }
            break;
        }

        if (reportTimeout)
        {
            client->eventReport((void *)client, userData, eventType);
        }
    }
    else
    {
        MQTT_PRINT_INFO("No eventReport and reqInfo buf, client:%p!\r\n", client);
    }
}

// 在MQTT异步任务中调用
void MQTTProcProcessingReqTimeoutEvent(MQTTClient* client)
{
    MQTT_PRINT_INFO("Mqtt processingReq timeout, client:%p!\r\n", client);

    MQTTReportTimeoutEvent(client);
    MQTTFreeProcessingReq(client);
}

void MQTTProcessingReqTimeout(void *parameter)
{
    MQTTClient *client = NULL;

    OS_ASSERT(NULL != parameter);
    client = MqttContainerOf(parameter, MQTTClient, processingReqTimeout);

    MQTTTimerOutEvent(client, MQTTProcProcessingReqTimeoutEvent);
}

void MQTTStartReconnTimer(MQTTClient* client)
{
    unsigned int timeout;

    if (0 == client->reconnMode)
    {
        timeout = client->reconnInterval;
    }
    else
    {
        timeout = client->reconnInterval * (client->currentReconnTimes);
    }
    MQTT_PRINT_INFO("Start mqtt reconn timer, timeout:%d, reconnTimes:%d, client:%p!\r\n",
        timeout, client->currentReconnTimes, client);
    TimerCountdown(&client->reConnTimer, timeout);
}
void MQTTStartReconn(MQTTClient* client)
{
    if (OS_FALSE == client->reconnEnable)
    {
        return;
    }

    // 启动重连定时器
    if (client->currentReconnTimes >= client->reconnTimes)
    {
        MQTT_PRINT_ERROR("Mqtt reconnTimes:%d, exceed maxReconnTimes:%d, client:%p!\r\n",
            client->currentReconnTimes, client->reconnTimes, client);
        if (NULL != client->eventReport)
        {
            client->eventReport((void *)client, NULL, MQTT_EVENT_SESSION_CLOSED);
        }

        return;
    }

    client->currentReconnTimes++;

    MQTTStartReconnTimer(client);
}

void MQTTCheckKeepAlive(MQTTClient* client)
{
    int rc = SUCCESS;

    if (keepalive(client) != SUCCESS)
    {
        rc = FAILURE;
    }

    MQTT_PRINT_INFO("rc:%d, isconnected:%d, client:%p\r\n", rc, client->isconnected, client);
    if ((SUCCESS != rc))
    {
        MQTTCloseSession(client, OS_TRUE);
        if (NULL != client->eventNotifyHandlerInSyncMode)
        {
            client->eventNotifyHandlerInSyncMode((MQTT_Handle)client, MQTT_NOTIFY_EVENT_LOST_KEEPALIVE, NULL);
        }
    }
}

void MQTTProcKeepAliveTimeoutEvent(MQTTClient* client)
{
    MQTT_PRINT_INFO("MQTTKeepAliveTimeout!, client:%p!\r\n", client);
    MQTTCheckKeepAlive(client);
}

void MQTTKeepAliveTimeout(void *parameter)
{
    MQTTClient *client = NULL;

    OS_ASSERT(NULL != parameter);
    client = MqttContainerOf(parameter, MQTTClient, keepAliveTimer);

    MQTTTimerOutEvent(client, MQTTProcKeepAliveTimeoutEvent);
}

// 在MQTT异步任务中调用
void MQTTProcReconnTimeoutEvent(MQTTClient* client)
{
    MQTT_PRINT_INFO("Mqtt reconn-timer timeout, client:%p!\r\n", client);

    if (!client->isconnected)
    {
        if(NULL == client->reqInfo.buf)
        {
            MQTT_EventInfo *eventInfo = osMalloc(sizeof(MQTT_EventInfo) + sizeof(MQTT_ConnInfo));
            if (eventInfo)
            {
                MQTT_ConnInfo *connInfo = (MQTT_ConnInfo *)eventInfo->buf;
                eventInfo->eventId = MQTT_EVENT_RECONN;
                eventInfo->client = client;
                connInfo->connectData = &client->connectData;

                MQTTPushEvent(eventInfo);
            }
        }
        else //  消息发送失败，重启定时器
        {
            MQTT_PRINT_INFO("Mqtt reconn-timer timeout, reqInfo confilict client:%p!\r\n", client);
            MQTTStartReconnTimer(client);
        }
    }
}

void MQTTReconnTimeout(void *parameter)
{
    MQTTClient *client = NULL;

    OS_ASSERT(NULL != parameter);
    client = MqttContainerOf(parameter, MQTTClient, reConnTimer);

    MQTTTimerOutEvent(client, MQTTProcReconnTimeoutEvent);
}

static int decodePacket(MQTTClient* c, int* value, int timeout)
{
    unsigned char i;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;
    do
    {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
        {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            goto exit;
        }

        if (NULL == c || NULL == c->ipstack)
        {
            goto exit;
        }

        rc = c->ipstack->mqttread(c->ipstack, &i, 1, timeout);
        if (rc != 1)
            goto exit;
        *value += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);
exit:
    return len;
}

static int readPacket(MQTTClient* c, Timer* timer)
{
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    if (NULL == c || NULL == c->ipstack)
    {
        return -1;
    }

    int rc = c->ipstack->mqttread(c->ipstack, c->readbuf, 1, 0);
    if (rc != 1)
        goto exit;

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, 0);
    len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    if (rem_len > (c->readbuf_size - len))
    {
        rc = BUFFER_OVERFLOW;
        goto exit;
    }

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if (NULL == c || NULL == c->ipstack)
    {
        return -2;
    }

    if (rem_len > 0 && (c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, 0) != rem_len)) {
        rc = 0;
        goto exit;
    }

    header.byte = c->readbuf[0];
    rc = header.bits.type;
exit:
    return rc;
}

// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
static char isTopicMatched(char* topicFilter, MQTTString* topicName)
{
    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;

    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}

int deliverMessage(MQTTClient* c, MQTTString* topicName, MQTT_Message* message)
{
    int i;
    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
        {
            if (c->messageHandlers[i].fp != NULL)
            {
                MQTT_MessageData md;
                NewMessageData(&md, topicName, message);
                c->messageHandlers[i].fp((void *)c, &md);
                rc = SUCCESS;
            }
        }
    }

    if (rc == FAILURE && c->defaultMessageHandler != NULL)
    {
        MQTT_MessageData md;
        NewMessageData(&md, topicName, message);
        c->defaultMessageHandler((void *)c, &md);
        rc = SUCCESS;
    }

    return rc;
}

int keepalive(MQTTClient* c)
{
    int rc = SUCCESS;

    if (c->ping_outstanding)
    {
        rc = FAILURE; /* PINGRESP not received in ping interval */
        if (NULL != c->eventReport)
        {
            c->eventReport((void *)c, NULL, MQTT_EVENT_PINGRESP);
        }
    }
    else
    {
        Timer timer = {0, 0, NULL};
        TimerInit(&timer, NULL);
        TimerCountdownMS(&timer, 1000);
        int len = MQTTSerialize_pingreq(c->buf, c->buf_size);
        if (len > 0 && (rc = sendPacket(c, len, &timer)) == SUCCESS) // send the ping packet
        {
            c->ping_outstanding = 1;
        }
        TimerRelease(&timer);
    }

    return rc;
}

void MQTTCleanSession(MQTTClient* c)
{
    if (c->cleansession)
    {
        int i = 0;

        for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        {
            c->messageHandlers[i].topicFilter = NULL;
            c->messageHandlers[i].fp = NULL;
        }
    }
}

void MQTTCloseSession(MQTTClient* c, int reconnFlag)
{
    MQTT_PRINT_WARN("MQTTCloseSession, reconnFlag:%d, client:%p!\r\n", reconnFlag, c);
    c->ping_outstanding = 0;
    c->isconnected = 0;
    MQTTCleanSession(c);

    TimerStop(&c->keepAliveTimer);
    TimerStop(&c->retransTimer);
    TimerStop(&c->processingReqTimeout);
    TimerStop(&c->reConnTimer);

    MQTTReportTimeoutEvent(c);
    MQTTFreeProcessingReq(c);

    if (OS_TRUE == reconnFlag)
    {
        MQTTStartReconn(c);
    }
    else
    {
        MQTTNetworkDisconnect(c->ipstack);
    }
}

bool_t MqttCheckRespPacketNoTimeout(MQTTClient *client, osTick_t packetTick)
{
    if (packetTick < client->reqInfo.reqStartTick || packetTick > client->reqInfo.reqEndTick)
    {
        MQTT_PRINT_WARN("Mqtt recv timeout readPacket, packetTickLow:%u, \
            reqStartTickLow:%u, reqEndTickLow:%u, client:%p!\r\n",
            (uint32_t)(packetTick & 0xFFFFFFFF),
            (uint32_t)(client->reqInfo.reqStartTick & 0xFFFFFFFF),
            (uint32_t)(client->reqInfo.reqEndTick & 0xFFFFFFFF),
            client);

        return OS_FALSE;
    }

    return OS_TRUE;
}

int MQTTCycle(MQTTClient* c, Timer* timer)
{
    int len = 0;
    int rc = SUCCESS;
    osTick_t packetTick;

    int packet_type = readPacket(c, timer);     /* read the socket, see what work is due */
    packetTick = osTickGet();
    MQTT_PRINT_INFO("MQTTCycle:packet_type:%d, client:%p\r\n", packet_type, c);

    switch (packet_type)
    {
        default:
            /* no more data to read, unrecoverable. Or read packet fails due to unexpected network error */
            rc = packet_type;
            goto exit;

        case 0: /* timed out reading packet */
            rc = SUCCESS;
            break;

        case CONNACK:
            {
                MQTTConnackData data;
                TimerStop(&c->processingReqTimeout);
                MQTT_PRINT_INFO("Stop mqtt conn req timeout timer, client:%p!\r\n", c);
                MQTTProcConnack(c, &data);
            }
            break;

        case PUBACK:
            {
                if (MqttCheckRespPacketNoTimeout(c, packetTick) && c->reqInfo.buf)
                {
                    MQTT_PubInfo *pubInfo = (MQTT_PubInfo *)(c->reqInfo.buf + sizeof(MQTT_EventInfo));
                    MQTTProcPublishResponse(c, PUBACK, pubInfo->topic, pubInfo->message);
                }
            }
            break;

        case SUBACK:
            {
                if (MqttCheckRespPacketNoTimeout(c, packetTick) && c->reqInfo.buf)
                {
                    MQTT_SubInfo *subInfo = (MQTT_SubInfo *)(c->reqInfo.buf + sizeof(MQTT_EventInfo));
                    MQTTSubackData data;
                    TimerStop(&c->processingReqTimeout);
                    MQTT_PRINT_INFO("Stop mqtt sub req timeout timer, client:%p!\r\n", c);
                    MQTTProcSuback(c, subInfo->topicCnt, subInfo->topics,
                        subInfo->QoSs, subInfo->messageHandler, &data);
                }
            }
            break;

        case UNSUBACK:
            {
                if (MqttCheckRespPacketNoTimeout(c, packetTick) && c->reqInfo.buf)
                {
                    MQTT_UnsubInfo *unsubInfo = (MQTT_UnsubInfo *)(c->reqInfo.buf + sizeof(MQTT_EventInfo));
                    TimerStop(&c->processingReqTimeout);
                    MQTT_PRINT_INFO("Stop mqtt unsub req timeout timer, client:%p!\r\n", c);
                    MQTTProcUnsubscribe(c, unsubInfo->topicCnt, unsubInfo->topics);
                }
            }
            break;

        case PUBLISH:
            {
                MQTTString topicName;
                MQTT_Message msg;
                int intQoS;
                msg.payloadlen = 0; /* this is a size_t, but deserialize publish sets this as int */
                if (MQTTDeserialize_publish(&msg.dup, &intQoS, &msg.retained, &msg.id, &topicName,
                    (unsigned char**)&msg.payload, (int*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
                {
                    goto exit;
                }
                msg.qos = (MQTT_QOS)intQoS;
                if (0 == c->cachedMode)
                {
                    deliverMessage(c, &topicName, &msg);
                }
                else
                {
                    int contentSize = msg.payloadlen + topicName.lenstring.len + 1; // +1是填充'\0'
                    int size = sizeof(MQTT_CachedMessage) + sizeof(MQTT_Message) + sizeof(MQTT_String) + contentSize;
                    int pos = 0;

                    if (contentSize > MQTT_MAX_CACHED_SIZE)
                    {
                        MQTT_PRINT_ERROR("Drop, exceed MQTT_MAX_CACHED_SIZE, client:%p!\r\n", c);
                    }
                    else
                    {
                        unsigned char *cacheBuf = NULL;
                        MQTT_CachedMessage *cachedMessage = NULL;
                        while (!osSlistIsEmpty(&c->cachedMessageList) && (c->cachedSize + contentSize) > MQTT_MAX_CACHED_SIZE)
                        {
                            struct osSlistNode *pNode = osSlistFirst(&c->cachedMessageList);
                            MQTT_CachedMessage *tmpCachedMsg = (MQTT_CachedMessage *)pNode;
                            c->cachedSize -= tmpCachedMsg->size;
                            c->cachedSize = (c->cachedSize < 0) ? 0 : c->cachedSize;
                            osSlistRemove(&c->cachedMessageList, pNode);

                            if (NULL != c->eventReport)
                            {
                                c->eventReport((void *)c, &tmpCachedMsg->data, MQTT_EVENT_DROP);
                            }
                            osFree(tmpCachedMsg);
                            MQTT_PRINT_WARN("Mqtt cache msg full!Remove oldest cache msg, client:%p!\r\n", c);
                        }

                        cacheBuf = (unsigned char *)osMalloc(size);
                        cachedMessage = (MQTT_CachedMessage *)cacheBuf;
                        if (cachedMessage)
                        {
                            MQTT_Message* tmpMessage = (MQTT_Message *)(cacheBuf + sizeof(MQTT_CachedMessage));
                            MQTT_String* tmpTopicName = (MQTT_String *)(cacheBuf + sizeof(MQTT_CachedMessage) + sizeof(MQTT_Message));

                            cachedMessage->data.message = tmpMessage;
                            cachedMessage->data.topicName = tmpTopicName;

                            tmpMessage->qos = msg.qos;
                            tmpMessage->retained = msg.retained;
                            tmpMessage->dup = msg.dup;
                            tmpMessage->id = msg.id;

                            pos = sizeof(MQTT_CachedMessage) + sizeof(MQTT_Message) + sizeof(MQTT_String);
                            tmpMessage->payload = (void *)(cacheBuf + pos);
                            tmpMessage->payloadlen = msg.payloadlen;
                            osMemcpy(tmpMessage->payload, msg.payload, msg.payloadlen);

                            tmpTopicName->cstring = NULL;
                            tmpTopicName->lenstring.len = topicName.lenstring.len;

                            pos = sizeof(MQTT_CachedMessage) + sizeof(MQTT_Message) + sizeof(MQTT_String) + msg.payloadlen;
                            tmpTopicName->lenstring.data = (char *)(cacheBuf + pos);
                            osMemcpy(tmpTopicName->lenstring.data, topicName.lenstring.data, topicName.lenstring.len);
                            tmpTopicName->lenstring.data[topicName.lenstring.len] = '\0';

                            cachedMessage->size = contentSize;
                            c->cachedSize += contentSize;
                            osSlistAppend(&c->cachedMessageList, &cachedMessage->node);

                            MQTT_PRINT_INFO("Mqtt new cached message recv, client:%p!\r\n", c);
                            if (NULL != c->eventReport)
                            {
                                c->eventReport((void *)c, &cachedMessage->data, MQTT_EVENT_NEW_MSG_RECV);
                            }
                        }
                        else
                        {
                            MQTT_MessageData tmpMessageData;
                            NewMessageData(&tmpMessageData, &topicName, &msg);

                            MQTT_PRINT_ERROR("Drop, fail to malloc, client:%p!\r\n", c);
                            if (NULL != c->eventReport)
                            {
                                c->eventReport((void *)c, &tmpMessageData, MQTT_EVENT_DROP);
                            }
                        }
                    }
                }

                if (msg.qos != MQTT_QOS0)
                {
                    if (msg.qos == MQTT_QOS1)
                        len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
                    else if (msg.qos == MQTT_QOS2)
                        len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
                    if (len <= 0)
                        rc = FAILURE;
                    else
                        rc = sendPacket(c, len, timer);
                    if (rc == FAILURE)
                        goto exit; // there was a problem
                }
            }
            break;

        case PUBREC:
            {
                if (MqttCheckRespPacketNoTimeout(c, packetTick) && c->reqInfo.buf)
                {
                    MQTT_PubInfo *pubInfo = (MQTT_PubInfo *)(c->reqInfo.buf + sizeof(MQTT_EventInfo));
                    MQTTProcPublishResponse(c, packet_type, pubInfo->topic, pubInfo->message);
                }
            }
            break;

        case PUBREL:
            {
                unsigned short mypacketid;
                unsigned char dup, type;
                if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                {
                    rc = FAILURE;
                }
                else if ((len = MQTTSerialize_ack(c->buf, c->buf_size, PUBCOMP, 0, mypacketid)) <= 0)
                {
                    rc = FAILURE;
                }
                else if ((rc = sendPacket(c, len, timer)) != SUCCESS) // send the PUBCOMP packet
                {
                    rc = FAILURE; // there was a problem
                }

                if (rc == FAILURE)
                {
                    goto exit; // there was a problem
                }
            }
            break;

        case PUBCOMP:
            {
                if (MqttCheckRespPacketNoTimeout(c, packetTick) && c->reqInfo.buf)
                {
                    MQTT_PubInfo *pubInfo = (MQTT_PubInfo *)(c->reqInfo.buf + sizeof(MQTT_EventInfo));
                    MQTTProcPublishResponse(c, PUBCOMP, pubInfo->topic, pubInfo->message);
                }
            }
            break;

        case PINGRESP:
            c->ping_outstanding = 0;
            MQTT_PRINT_INFO("mqtt recv pingresp ok, client:%p!\r\n", c);
            if (NULL != c->eventReport)
            {
                c->eventReport((void *)c, NULL, MQTT_EVENT_PINGRESP);
            }
            break;
    }

exit:
    if (rc == SUCCESS)
        rc = packet_type;
    else if (c->isconnected)
    {
        MQTTCloseSession(c, OS_TRUE);
    }
    return rc;
}

int MQTT_IsConnected(MQTT_Handle handle)
{
    MQTTClient* client = (MQTTClient *)handle;
    int isconnected;

    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return 0;
    }
    isconnected = client->isconnected;
    MutexUnlock(&g_mqttWorkMutex);

    return isconnected;
}

int MQTT_IsBusy(MQTT_Handle handle)
{
    int busy;
    MQTTClient* client = (MQTTClient *)handle;

    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return OS_FALSE;
    }

    busy = (NULL != client->reqInfo.buf) ? OS_TRUE : OS_FALSE;
    MutexUnlock(&g_mqttWorkMutex);

    return busy;

}

void MQTTWaitIdle(MQTTClient* client)
{
    while (MQTT_IsBusy((MQTT_Handle)client))
    {
        osThreadMsSleep(200);
        MQTT_PRINT_INFO("MQTTWaitIdle client:%p\r\n", client);
    }
}

int MQTTCheckState(MQTTClient* client)
{
    if (0 != client->syncFlag)
    {
        // 同步接口, 等待客户端空闲
        MQTTWaitIdle(client);
    }

    return MQTT_IsBusy((MQTT_Handle)client);
}

void MQTT_SocketRecvEventCallback(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param)
{
    MQTT_PRINT_INFO("MQTT_SocketRecvEventCallback fd:%d, event:%d\r\n", fd, event);
    switch(event)
    {
        case SOCKET_TCP_EVENT_ACCEPT:  /* 作为服务器, 已接收到客户端的连接 */
            {
                ;
            }
            break;

        case SOCKET_TCP_EVENT_SENT:  /* TCP 时, 已发送数据 */
            {
                ;
            }
            break;

        case SOCKET_TCP_EVENT_RECV:  /* 1.接收到的数据. 或者 2.接收到对端的 TCP 关闭连接. */
            {
                if(0 == len || NULL == p)
                {
                    MQTT_PRINT_WARN("Recv close event, fd:%d!\r\n", fd);
                    MQTT_EventInfo *eventInfo = (MQTT_EventInfo *)osMalloc(sizeof(MQTT_EventInfo));
                    if (eventInfo)
                    {
                        eventInfo->eventId = MQTT_EVENT_NET_CLOSE;
                        eventInfo->client = (MQTTClient *)(intptr_t)fd;
                        MQTTPushEvent(eventInfo);
                    }
                    else
                    {
                        MQTT_PRINT_ERROR("Recv close event, but fail to malloc, fd:%d!\r\n", fd);
                    }
                }
                else
                {
                    MQTT_EventInfo *eventInfo = (MQTT_EventInfo *)osMalloc(sizeof(MQTT_EventInfo) + sizeof(MQTT_RecvInfo));
                    if (eventInfo)
                    {
                        MQTT_RecvInfo *recvInfo = (MQTT_RecvInfo *)eventInfo->buf;
                        eventInfo->eventId = MQTT_EVENT_NET_RECV;
                        eventInfo->client = (MQTTClient *)(intptr_t)fd;
                        recvInfo->len = len;
                        MQTTPushEvent(eventInfo);
                    }
                    else
                    {
                        MQTT_PRINT_ERROR("Recv data, but fail to malloc, fd:%d!\r\n", fd);
                    }
                }
            }
            break;

        case SOCKET_TCP_EVENT_CONNECTED:  /* 作为客户端,   连接成功 */
            {
                ;
            }
            break;

        case SOCKET_TCP_EVENT_POLL:
            {
                ;
            }
            break;

        case SOCKET_TCP_EVENT_ERR:  /* TCP 错误提示, 如: 收到对端 RST */
        case SOCKET_NETIF_EVENT_DOWN:
            {
                MQTT_PRINT_WARN("Recv netif eventdown event %u, fd:%d!\r\n", event, fd);
                MQTT_EventInfo *eventInfo = (MQTT_EventInfo *)osMalloc(sizeof(MQTT_EventInfo));
                if (eventInfo)
                {
                    eventInfo->eventId = MQTT_EVENT_NET_CLOSE;
                    eventInfo->client = (MQTTClient *)(intptr_t)fd;
                    MQTTPushEvent(eventInfo);
                }
                else
                {
                    MQTT_PRINT_ERROR("Recv netif eventdown event, but fail to malloc, fd:%d!\r\n", fd);
                }
            }
            break;

        default:
            break;
    }

    return;
}

// 异步消息处理任务
static void MQTTAsyncProc(void* parm)
{
    while (1)
    {
        if (osOK == osWaitForCompletion(&g_mqttCmpl, osWaitForever))
        {
            MutexLock(&g_mqttWorkMutex);
            MQTT_EventInfo *eventInfo = MQTTPopEvent();

            if (NULL != eventInfo)
            {
                MQTTClient* client = eventInfo->client;

                if(!MQTT_ClientIsValid((MQTT_Handle)client))
                {
                    MQTT_PRINT_INFO("MQTTTask:recv invalid event\r\n");
                    osFree(eventInfo);
                }
                else
                {
                    MQTT_PRINT_INFO("MQTTTask:recv eventId:%d (%s), client:%p!\r\n", eventInfo->eventId, MQTT_EventString(eventInfo->eventId), eventInfo->client);
                    switch (eventInfo->eventId)
                    {
                    case MQTT_EVENT_CONN:
                        {
                            MQTTSetReqInfo(eventInfo->client, eventInfo);
                            MQTTProcConnEvent(eventInfo);
                        }
                        break;

                    case MQTT_EVENT_RECONN:
                        {
                            MQTTSetReqInfo(eventInfo->client, eventInfo);
                            MQTTProcReConnEvent(eventInfo);
                        }
                        break;

                    case MQTT_EVENT_SUB:
                        {
                            MQTTSetReqInfo(eventInfo->client, eventInfo);
                            MQTTProcSubEvent(eventInfo);
                        }
                        break;

                    case MQTT_EVENT_UNSUB:
                        {
                            MQTTSetReqInfo(eventInfo->client, eventInfo);
                            MQTTProcUnsubEvent(eventInfo);
                        }
                        break;

                    case MQTT_EVENT_PUB:
                        {
                            MQTTSetReqInfo(eventInfo->client, eventInfo);
                            MQTTProcPubEvent(eventInfo);
                        }
                        break;

                    case MQTT_EVENT_DISC:
                        {
                            MQTTSetReqInfo(eventInfo->client, eventInfo);
                            MQTTProcDiscEvent(eventInfo);
                        }
                        break;

                    case MQTT_EVENT_NET_RECV:
                        {
                            MQTTProcRecvEvent(eventInfo);
                            osFree(eventInfo);
                        }
                        break;

                    case MQTT_EVENT_TIMER_TIMEOUT:
                        {
                            MQTT_TimerOutInfo *timerOutInfo = (MQTT_TimerOutInfo *)eventInfo->buf;
                            MQTT_PRINT_INFO("MQTTAsyncProcTask:recv timer-timeout event, client:%p!\r\n", eventInfo->client);
                            timerOutInfo->fp(eventInfo->client);
                            osFree(eventInfo);
                        }
                        break;

                    case MQTT_EVENT_NET_CLOSE:
                        {
                            MQTTCloseSession(eventInfo->client, OS_FALSE);

                            // AT命令需要上报网络异常状态断开URC(+IMTSTAT)
                            if (NULL != eventInfo->client->eventReport)
                            {
                                eventInfo->client->eventReport((void *)eventInfo->client, NULL, MQTT_EVENT_NET_CLOSE);
                            }

                            if (NULL != eventInfo->client->connLostHandlerInSyncMode)
                            {
                                eventInfo->client->connLostHandlerInSyncMode((MQTT_Handle)eventInfo->client, NULL);
                                MQTT_PRINT_WARN("Mqtt recv close event, connection lost, client:%p!\r\n", eventInfo->client);
                            }

                            if (NULL != eventInfo->client->eventNotifyHandlerInSyncMode)
                            {
                                eventInfo->client->eventNotifyHandlerInSyncMode((MQTT_Handle)eventInfo->client, MQTT_NOTIFY_EVENT_CLOSE_SESSION, NULL);
                                MQTT_PRINT_WARN("Mqtt notify close event, connection lost, client:%p!\r\n", eventInfo->client);
                            }
                            osFree(eventInfo);
                        }
                        break;

                    default:
                        osFree(eventInfo);
                        break;
                    }
                }
            }
            MutexUnlock(&g_mqttWorkMutex);
        }
        else
        {
            MQTT_PRINT_ERROR("Wait mqtt cmpl error!\r\n");
            osThreadMsSleep(100);
        }
    }
}

void MQTTSetReqInfo(MQTTClient* c, MQTT_EventInfo *eventInfo)
{
    OS_ASSERT(NULL == c->reqInfo.buf);

    c->reqInfo.reqStartTick = osTickGet();
    c->reqInfo.reqEndTick = OS_TICK_MAX;
    c->reqInfo.eventId = eventInfo->eventId;
    c->reqInfo.buf = (uint8_t *)eventInfo;
}

// there will be a blocking call in sync mode, wait for eventType
int MQTTWaitEventInSyncMode(MQTTClient* c, int eventType)
{
    int rc = FAILURE;

    if (0 != c->syncFlag)
    {
        char* recvBuf = NULL;

        MQTT_PRINT_INFO("Wait for sync req proc, eventType:%d (%s), client:%p!\r\n", eventType, MQTT_EventString(eventType), c);
        if (osOK == osMbRecv(c->recvSyncEventMb, (ubase_t*)&recvBuf, osWaitForever))
        {
            MQTTEventType *pEventType = (MQTTEventType *)recvBuf;
            rc = *pEventType;
            osFree(recvBuf);
        }
        else
        {
            //  在 recvSyncEventMb 释放时，osMbRecv会返回-1，去掉断言，按出错处理
            MQTT_PRINT_ERROR("Wait for sync response, MB Error client:%p!\r\n", c);
            rc = MQTT_EVENT_ERROR;
            //OS_ASSERT(0);
        }

        MQTT_PRINT_INFO("Mqtt recv sync req response, result:%d (%s), eventType:%d (%s), client:%p!\r\n",
                            rc, MQTT_EventString(rc), eventType, MQTT_EventString(eventType), c);
        return (rc == eventType) ? MQTT_OK : MQTT_ERR;
    }
    else
    {
        return MQTT_OK;
    }
}

int MQTTProcConnack(MQTTClient* c, MQTTConnackData* data)
{
    int rc = FAILURE;
    MQTTEventType eventType;
    int reCode;

    data->rc = 0;
    data->sessionPresent = 0;
    if (MQTTDeserialize_connack(&data->sessionPresent, &data->rc, c->readbuf, c->readbuf_size) == 1)
    {
        rc = data->rc;
        reCode = (int)data->rc;
    }
    else
    {
        rc = FAILURE;
        reCode = -1;
    }

    if (rc == SUCCESS)
    {
        c->isconnected = 1;
        c->ping_outstanding = 0;
        c->currentReconnTimes = 0;
        eventType = MQTT_EVENT_CONNACK;

        if (0 != c->syncFlag)
        {
            // 同步模式
            c->pingInterval = c->connectData.keepAliveInterval; // 如果 keepAliveInterval 是0呢？？0表示不需要心跳，TimerCountdown中timeout是0不会配置
        }

        MQTT_PRINT_INFO("connack, start mqtt keepAliveTimer, client:%p!\r\n", c);
        TimerCountdown(&c->keepAliveTimer, c->pingInterval);
        if (NULL != c->eventNotifyHandlerInSyncMode)
        {
            c->eventNotifyHandlerInSyncMode((MQTT_Handle)c, MQTT_NOTIFY_EVENT_CONNECT, NULL);
        }
    }
    else
    {
        eventType = MQTT_EVENT_CONN_ERR;
    }

    if (c->eventReport)
    {
        c->eventReport((MQTT_Handle)c, (void *)&reCode, eventType);
    }

    MQTTFreeProcessingReq(c);

    return rc;
}

int MQTTConnectWithResults(MQTTClient* c, MQTTPacket_connectData* options)
{
    Timer connect_timer = {0, 0, NULL};
    int rc = FAILURE;
    int len = 0;

    MutexLock(&c->mutex);

    if (c->isconnected) /* don't send connect packet again if we are already connected */
        goto exit;

    TimerInit(&connect_timer, NULL);
    TimerCountdownMS(&connect_timer, c->command_timeout_ms);

    OS_ASSERT(NULL != options);

    c->keepAliveInterval = options->keepAliveInterval;
    c->cleansession = options->cleansession;
    if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &connect_timer)) != SUCCESS)  // send the connect packet
        goto exit; // there was a problem

exit:
    TimerRelease(&connect_timer);
    MutexUnlock(&c->mutex);


    return rc;
}

int MQTTConnectFunc(MQTTClient* c, MQTTPacket_connectData* options)
{
    return MQTTConnectWithResults(c, options);
}

int MQTT_Connect(MQTT_Handle handle, MQTT_ConnectOption* options)
{
    int ret = MQTT_OK;
    MQTTClient *c = (MQTTClient *)handle;

    if (NULL == options)
    {
        return MQTT_ERR_PARAMETER;
    }

    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_INVALID_HANDLE;
    }

    if (c->isconnected)
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_OK;
    }

    if (NULL == c->ipstack)
    {
        Network *pNetwork = osMalloc(sizeof(Network));
        if (NULL != pNetwork)
        {
            ret = MQTTNetworkInit(pNetwork, options->host, options->port, options->ca_crt);
            if (0 != ret)
            {
                osFree(pNetwork);
                MutexUnlock(&g_mqttWorkMutex);
                return MQTT_ERR_PARAMETER;
            }

            ret = MQTTNetworkConnect(pNetwork);
            if (0 != ret)
            {
                osFree(pNetwork);
                MutexUnlock(&g_mqttWorkMutex);
                return MQTT_ERR_NET_CONNECT;
            }
            c->ipstack = pNetwork;
        }
    }
    #if 0
    // AT 命令ipstack是命令创建的，不能修改其值。OpenCPU方式，如果发现参数变了，建议销毁后重新创建
    else
    {
        ret = MQTTNetworkSetParam(c->ipstack, options->host, options->port, options->ca_crt);
        if (0 != ret)
        {
            MutexUnlock(&g_mqttWorkMutex);
            return MQTT_ERR_PARAMETER;
        }
    }
    #endif

    if (!MQTTCheckState(c))
    {
        MQTT_EventInfo *eventInfo = osMalloc(sizeof(MQTT_EventInfo) + sizeof(MQTT_ConnInfo));
        if (eventInfo)
        {
            MQTT_ConnInfo *connInfo = (MQTT_ConnInfo *)eventInfo->buf;
            eventInfo->eventId = MQTT_EVENT_CONN;
            eventInfo->client = c;
            connInfo->connectData = &c->connectData;
            MQTTGetConnectData(connInfo->connectData, options);
            MQTTPushEvent(eventInfo);
        }
        else
        {
            ret = MQTT_ERR_RESOURCE;
        }
    }
    else
    {
        ret = MQTT_ERR_BUSY;
    }
    MutexUnlock(&g_mqttWorkMutex);
    ret = MQTTWaitEventInSyncMode(c, MQTT_EVENT_CONNACK);

    return ret;
}

int MQTTSetMessageHandler(MQTTClient* c, const char* topicFilter, MQTT_MessageHandler messageHandler)
{
    int rc = FAILURE;
    int i = -1;

    /* first check for an existing matching slot */
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != NULL && strcmp(c->messageHandlers[i].topicFilter, topicFilter) == 0)
        {
            if (messageHandler == NULL) /* remove existing */
            {
                c->messageHandlers[i].topicFilter = NULL;
                c->messageHandlers[i].fp = NULL;
            }
            rc = SUCCESS; /* return i when adding new subscription */
            break;
        }
    }
    /* if no existing, look for empty slot (unless we are removing) */
    if (messageHandler != NULL) {
        if (rc == FAILURE)
        {
            for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
            {
                if (c->messageHandlers[i].topicFilter == NULL)
                {
                    rc = SUCCESS;
                    break;
                }
            }
        }
        if (i < MAX_MESSAGE_HANDLERS)
        {
            c->messageHandlers[i].topicFilter = topicFilter;
            c->messageHandlers[i].fp = messageHandler;
        }
    }
    return rc;
}

void MQTT_SetEventCallback(MQTT_Handle handle, MQTTEventCallback eventCallback)
{
    MQTTClient* c = (MQTTClient *)handle;
    if (c && (0 == c->syncFlag))
    {
        c->eventReport = eventCallback;
    }
}

int MQTTProcSuback(MQTTClient* c, int topicCnt, const char* topics[], int QoSs[],
    MQTT_MessageHandler messageHandler, MQTTSubackData* data)
{
    int rc = FAILURE;
    int i;
    int count = 0;
    unsigned short mypacketid = 0;
    MQTTEventType eventType;

    if (MQTTDeserialize_suback(&mypacketid, MAX_MESSAGE_HANDLERS - 1, &count, (int*)&data->grantedQoS, c->readbuf, c->readbuf_size) == 1)
    {
        for (i = 0; i < count; i++)
        {
            if (data->grantedQoS[i] != MQTT_SUBFAIL)
            {
                rc = MQTTSetMessageHandler(c, topics[i], messageHandler);
                if (SUCCESS != rc)
                {
                    break;
                }
            }
        }
    }
    else
    {
        MQTT_PRINT_ERROR("MQTTDeserialize_suback error\r\n");
    }

    eventType = (rc == 0) ? MQTT_EVENT_SUBACK : MQTT_EVENT_SUB_ERR;
    if (c->eventReport)
    {
        c->eventReport((MQTT_Handle)c, &mypacketid, eventType);
    }

    MQTTFreeProcessingReq(c);

    return rc;
}

int MQTTSubscribeWithResults(MQTTClient* c, unsigned short id, int count, const char* topicFilters[], int QoSs[],
       MQTT_MessageHandler messageHandler, MQTTSubackData* data)
{
    int rc = FAILURE;
    Timer timer = {0, 0, NULL};
    int len = 0;
    MQTTString topic[count];
    int i;
    unsigned short packetid;

    for (i = 0; i < count; i++)
    {
        topic[i].cstring = (char *)topicFilters[i];
        topic[i].lenstring.len = 0;
        topic[i].lenstring.data = NULL;
    }

    MutexLock(&c->mutex);

    if (!c->isconnected)
        goto exit;

    TimerInit(&timer, NULL);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    packetid = (id == 0) ? getNextPacketId(c) : id;
    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, packetid, count, topic, QoSs);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit;             // there was a problem

exit:
    TimerRelease(&timer);
    if (rc == FAILURE)
    {
        MQTTCloseSession(c, OS_TRUE);
    }

    MutexUnlock(&c->mutex);


    return rc;
}


int MQTTSubscribeFunc(MQTTClient* c, unsigned short id, int count, const char* topicFilters[], int QoSs[],
       MQTT_MessageHandler messageHandler)
{
    MQTTSubackData data;
    return MQTTSubscribeWithResults(c, id, count, topicFilters, QoSs, messageHandler, &data);
}

int MQTT_Subscribe(MQTT_Handle handle, int count, const char* topicFilters[], int QoSs[],
       MQTT_MessageHandler messageHandler)
{
    MQTTClient *c = (MQTTClient *)handle;
    int i;
    int ret = MQTT_OK;

    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_INVALID_HANDLE;
    }

    if (!c->isconnected)
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_UNCONN;
    }

    if (!MQTTCheckState(c))
    {
        MQTT_EventInfo *eventInfo = osMalloc(sizeof(MQTT_EventInfo) + sizeof(MQTT_SubInfo));
        if (eventInfo)
        {
            MQTT_SubInfo *subInfo = (MQTT_SubInfo *)eventInfo->buf;
            eventInfo->eventId = MQTT_EVENT_SUB;
            eventInfo->client = c;
            subInfo->id = 0;
            subInfo->topicCnt = count;

            for (i = 0; i < count; i++)
            {
                subInfo->topics[i] = topicFilters[i];
                subInfo->QoSs[i] = QoSs[i];
            }

            subInfo->messageHandler = messageHandler;
            MQTTPushEvent(eventInfo);
        }
        else
        {
            ret = MQTT_ERR_RESOURCE;
        }
    }
    else
    {
        ret = MQTT_ERR_BUSY;
    }
    MutexUnlock(&g_mqttWorkMutex);
    ret = MQTTWaitEventInSyncMode(c, MQTT_EVENT_SUBACK);

    return ret;
}

int MQTT_SubscribeWithId(MQTT_Handle handle, unsigned short id,
        int count, const char* topicFilters[], int QoSs[],
        MQTT_MessageHandler messageHandler)
{
    MQTTClient* c = (MQTTClient *)handle;
    int i;
    int ret = MQTT_OK;

    if (!c->isconnected)
    {
        return MQTT_ERR_UNCONN;
    }

    if (!MQTTCheckState(c))
    {
        MQTT_EventInfo *eventInfo = osMalloc(sizeof(MQTT_EventInfo) + sizeof(MQTT_SubInfo));
        if (eventInfo)
        {
            MQTT_SubInfo *subInfo = (MQTT_SubInfo *)eventInfo->buf;
            eventInfo->eventId = MQTT_EVENT_SUB;
            eventInfo->client = c;
            subInfo->id = id;
            subInfo->topicCnt = count;

            for (i = 0; i < count; i++)
            {
                subInfo->topics[i] = topicFilters[i];
                subInfo->QoSs[i] = QoSs[i];
            }

            subInfo->messageHandler = messageHandler;
            MQTTPushEvent(eventInfo);
        }
        else
        {
            ret = MQTT_ERR_RESOURCE;
        }
    }
    else
    {
        ret = MQTT_ERR_BUSY;
    }

    ret = MQTTWaitEventInSyncMode(c, MQTT_EVENT_SUBACK);

    return ret;
}

void MQTTProcUnsubscribe(MQTTClient* c, int count, const char* topicFilters[])
{
    MQTTEventType eventType;
    unsigned short mypacketid = 0;  // should be the same as the packetid above
    if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1)
    {
        /* remove the subscription message handler associated with this topic, if there is one */
        int i;
        for (i = 0; i < count; i++)
        {
            MQTTSetMessageHandler(c, topicFilters[i], NULL);
        }
        eventType = MQTT_EVENT_UNSUBACK;
    }
    else
    {
        eventType = MQTT_EVENT_UNSUB_ERR;
    }

    if (c->eventReport)
    {
        c->eventReport((MQTT_Handle)c, &mypacketid, eventType);
    }

    MQTTFreeProcessingReq(c);
}

int MQTTUnsubscribeFunc(MQTTClient* c, unsigned short id, int count, const char* topicFilters[])
{
    int rc = FAILURE;
    Timer timer = {0, 0, NULL};
    MQTTString topic[count];
    int i;
    int len = 0;
    unsigned short packetid;

    for (i = 0; i < count; i++)
    {
        topic[i].cstring = (char *)topicFilters[i];
        topic[i].lenstring.len = 0;
        topic[i].lenstring.data = NULL;
    }

    MutexLock(&c->mutex);

    if (!c->isconnected)
        goto exit;

    TimerInit(&timer, NULL);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    packetid = (id == 0) ? getNextPacketId(c) : id;
    if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, packetid, count, &topic[0])) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

exit:
    TimerRelease(&timer);
    if (rc == FAILURE)
    {
        MQTTCloseSession(c, OS_TRUE);
    }

    MutexUnlock(&c->mutex);

    return rc;
}

int MQTT_Unsubscribe(MQTT_Handle handle, int count, const char* topicFilters[])
{
    MQTTClient *c = (MQTTClient *)handle;
    int i;
    int ret = MQTT_OK;

    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_INVALID_HANDLE;
    }

    if (!c->isconnected)
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_UNCONN;
    }

    if (!MQTTCheckState(c))
    {
        MQTT_EventInfo *eventInfo = osMalloc(sizeof(MQTT_EventInfo) + sizeof(MQTT_UnsubInfo));
        if (eventInfo)
        {
            MQTT_UnsubInfo *unsubInfo = (MQTT_UnsubInfo *)eventInfo->buf;
            eventInfo->eventId = MQTT_EVENT_UNSUB;
            eventInfo->client = c;
            unsubInfo->id = 0;
            unsubInfo->topicCnt = count;

            for (i = 0; i < count; i++)
            {
                unsubInfo->topics[i] = topicFilters[i];
            }

            MQTTPushEvent(eventInfo);
        }
        else
        {
            ret = MQTT_ERR_RESOURCE;
        }
    }
    else
    {
        ret = MQTT_ERR_BUSY;
    }
    MutexUnlock(&g_mqttWorkMutex);
    ret = MQTTWaitEventInSyncMode(c, MQTT_EVENT_UNSUBACK);

    return ret;
}

int MQTT_UnsubscribeWithId(MQTT_Handle handle, unsigned short id, int count, const char* topicFilters[])
{
    MQTTClient* c = (MQTTClient* )handle;
    int i;
    int ret = MQTT_OK;

    if (!c->isconnected)
    {
        return MQTT_ERR_UNCONN;
    }

    if (!MQTTCheckState(c))
    {
        MQTT_EventInfo *eventInfo = osMalloc(sizeof(MQTT_EventInfo) + sizeof(MQTT_UnsubInfo));
        if (eventInfo)
        {
            MQTT_UnsubInfo *unsubInfo = (MQTT_UnsubInfo *)eventInfo->buf;
            eventInfo->eventId = MQTT_EVENT_UNSUB;
            eventInfo->client = c;
            unsubInfo->id = id;
            unsubInfo->topicCnt = count;

            for (i = 0; i < count; i++)
            {
                unsubInfo->topics[i] = topicFilters[i];
            }

            MQTTPushEvent(eventInfo);
        }
        else
        {
            ret = MQTT_ERR_RESOURCE;
        }
    }
    else
    {
        ret = MQTT_ERR_BUSY;
    }

    ret = MQTTWaitEventInSyncMode(c, MQTT_EVENT_UNSUBACK);

    return ret;
}

void MQTTProcPublishResponse(MQTTClient* c, int packetType, const char* topicName, MQTT_Message* message)
{
    int rc = SUCCESS;
    MQTTEventType eventType;

    if (message->qos == MQTT_QOS1)
    {
        // 停止超时重传定时器
        MQTT_PRINT_INFO("QOS1:Stop retransTimer, client:%p!\r\n", c);
        TimerStop(&c->retransTimer);
        MQTT_PRINT_INFO("QOS1:Stop pub req timeout timer, client:%p!\r\n", c);
        TimerStop(&c->processingReqTimeout);

        if (PUBACK == packetType)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            rc = SUCCESS;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
            {
                rc = FAILURE;
                goto exit; // there was a problem
            }
        }
        else
        {
            rc = FAILURE;
            OS_ASSERT(0);
        }

        eventType = (SUCCESS == rc) ? MQTT_EVENT_PUBACK : MQTT_EVENT_PUB_ERR;
        if (NULL != c->eventReport)
        {
            MQTT_String topic = {NULL, {0, NULL}};
            MQTT_MessageData data;

            topic.cstring = (char *)topicName;
            data.topicName = &topic;
            data.message = message;
            c->eventReport((void *)c, &data, eventType);
        }

        MQTTFreeProcessingReq(c);
    }
    else if (message->qos == MQTT_QOS2)
    {
        // 停止超时重传定时器
        MQTT_PRINT_INFO("QOS2:Stop retransTimer, client:%p!\r\n", c);
        TimerStop(&c->retransTimer);
        MQTT_PRINT_INFO("QOS2:Stop pub req timeout timer, client:%p!\r\n", c);
        TimerStop(&c->processingReqTimeout);

        switch (packetType)
        {
        case PUBREC:
            {
                int len;
                unsigned short mypacketid;
                unsigned char dup, type;

                rc = SUCCESS;
                if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                {
                    rc = FAILURE;
                    goto exit; // there was a problem
                }

                MQTT_PRINT_INFO("recv packetType:%d, client:%p\r\n", packetType, c);
                eventType = (SUCCESS == rc) ? MQTT_EVENT_PUBREC : MQTT_EVENT_PUB_ERR;
                if (NULL != c->eventReport)
                {
                    MQTT_String topic = {NULL, {0, NULL}};
                    MQTT_MessageData data;

                    topic.cstring = (char *)topicName;
                    data.topicName = &topic;
                    data.message = message;
                    c->eventReport((void *)c, &data, eventType);
                }

                if (SUCCESS == rc)
                {
                    if ((len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREL, 0, mypacketid)) <= 0)
                    {
                        rc = FAILURE;
                        goto exit; // there was a problem
                    }
                    else
                    {
                        Timer timer = {0, 0, NULL};
                        TimerInit(&timer, NULL);
                        TimerCountdownMS(&timer, c->command_timeout_ms);
                        // send the PUBREL packet
                        if ((rc = sendPacket(c, len, &timer)) != SUCCESS)
                        {
                            TimerRelease(&timer);
                            rc = FAILURE; // there was a problem
                            goto exit; // there was a problem
                        }
                        else
                        {
                            TimerRelease(&timer);
                            if (c->retryTimes > 0)
                            {
                                unsigned int timeout;
                                c->currentRetryTimes = 0;
                                timeout = c->retransInterval * ((2 << (c->currentRetryTimes)) - 1);
                                MQTT_PRINT_INFO("First start mqtt pubrel retrans req timer, timeout:%u, retryTimes:%d, client:%p\r\n",
                                    timeout, c->currentRetryTimes, c);
                                TimerCountdown(&c->retransTimer, timeout);

                                c->reqInfo.eventId = MQTT_EVENT_PUBREL;
                            }
                        }
                    }
                }
            }
            break;

        case PUBCOMP:
            {
                unsigned short mypacketid;
                unsigned char dup, type;
                rc = SUCCESS;
                if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                {
                    rc = FAILURE;
                    goto exit; // there was a problem
                }

                eventType = (SUCCESS == rc) ? MQTT_EVENT_PUBCOMP : MQTT_EVENT_PUB_ERR;
                if (NULL != c->eventReport)
                {
                    MQTT_String topic = {NULL, {0, NULL}};
                    MQTT_MessageData data;

                    topic.cstring = (char *)topicName;
                    data.topicName = &topic;
                    data.message = message;
                    c->eventReport((void *)c, &data, eventType);
                }

                MQTTFreeProcessingReq(c);
            }
            break;

        default:
            OS_ASSERT(0);
            break;
        }
    }

exit:
    if ((rc == FAILURE) && c->isconnected)
    {
        MQTTCloseSession(c, OS_TRUE);
    }
}

int MQTTPublishFunc(MQTTClient* c, const char* topicName, MQTT_Message* message)
{
    int rc = FAILURE;
    Timer timer = {0, 0, NULL};
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;

    MutexLock(&c->mutex);

    if (!c->isconnected)
        goto exit;

    TimerInit(&timer, NULL);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    if (message->qos == MQTT_QOS1 || message->qos == MQTT_QOS2)
    {
        if (message->id == 0)
            message->id = getNextPacketId(c);
    }

    len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id,
              topic, (unsigned char*)message->payload, message->payloadlen);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

exit:
    TimerRelease(&timer);
    if (rc == FAILURE)
    {
        MQTTCloseSession(c, OS_TRUE);
    }

    MutexUnlock(&c->mutex);


    return rc;
}

int MQTT_Publish(MQTT_Handle handle, const char* topic, MQTT_Message* message)
{
    MQTTClient *c = (MQTTClient *)handle;
    int ret = MQTT_OK;

    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_INVALID_HANDLE;
    }

    if (!c->isconnected)
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_UNCONN;
    }

    if (message->qos != MQTT_QOS0 && message->qos != MQTT_QOS1 && message->qos != MQTT_QOS2)
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_PARAMETER;
    }

    if (!MQTTCheckState(c))
    {
        MQTT_EventInfo *eventInfo = osMalloc(sizeof(MQTT_EventInfo) + sizeof(MQTT_PubInfo));
        if (eventInfo)
        {
            MQTT_PubInfo *pubInfo = (MQTT_PubInfo *)eventInfo->buf;
            eventInfo->eventId = MQTT_EVENT_PUB;
            eventInfo->client = c;
            pubInfo->topic = topic;
            pubInfo->message = message;
            MQTTPushEvent(eventInfo);
        }
        else
        {
            ret = MQTT_ERR_RESOURCE;
        }
    }
    else
    {
        ret = MQTT_ERR_BUSY;
    }
    MutexUnlock(&g_mqttWorkMutex);
    switch (message->qos)
    {
    case MQTT_QOS0:
        {
            ret = MQTTWaitEventInSyncMode(c, MQTT_EVENT_PUB_QOS0);
        }
        break;

    case MQTT_QOS1:
        {
            ret = MQTTWaitEventInSyncMode(c, MQTT_EVENT_PUBACK);
        }
        break;

    case MQTT_QOS2:
        {
            ret = MQTTWaitEventInSyncMode(c, MQTT_EVENT_PUBCOMP);
        }
        break;

    default:
        break;
    }

    return ret;
}

int MQTTDisconnectFunc(MQTTClient* c)
{
    int rc = FAILURE;
    Timer timer = {0, 0, NULL};     // we might wait for incomplete incoming publishes to complete
    int len = 0;

    if(!c->isconnected) //  当lwip断开和MQTT_EVENT_DISC同时发生时，后者会引发异常，所以这里增加判断
    {
        MQTTFreeProcessingReq(c);
        return rc;
    }

    MutexLock(&c->mutex);
    TimerInit(&timer, NULL);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    len = MQTTSerialize_disconnect(c->buf, c->buf_size);
    if (len > 0)
        rc = sendPacket(c, len, &timer);            // send the disconnect packet
    TimerRelease(&timer);
    MQTTCloseSession(c, OS_FALSE);

    MutexUnlock(&c->mutex);

    return rc;
}

int MQTT_Disconnect(MQTT_Handle handle)
{
    MQTTClient *c = (MQTTClient *)handle;
    int ret = MQTT_OK;

    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_INVALID_HANDLE;
    }

    if (!c->isconnected)
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_OK;
    }

    if (!MQTTCheckState(c))
    {
        MQTT_EventInfo *eventInfo = osMalloc(sizeof(MQTT_EventInfo));
        if (eventInfo)
        {
            eventInfo->eventId = MQTT_EVENT_DISC;
            eventInfo->client = c;

            MQTTPushEvent(eventInfo);
        }
        else
        {
            ret = MQTT_ERR_RESOURCE;
        }
    }
    else
    {
        ret = MQTT_ERR_BUSY;
    }
    MutexUnlock(&g_mqttWorkMutex);
    ret = MQTTWaitEventInSyncMode(c, MQTT_EVENT_DISC_OK);

    return ret;
}

void MQTT_QueryCacheMessage(MQTT_Handle handle, uint32_t *count, uint32_t *totalLen)
{
    MQTTClient* client = (MQTTClient *)handle;
    int i;
    struct osSlistNode *pNode = osSlistFirst(&client->cachedMessageList);
    struct osSlistNode *pNextNode = NULL;

    *count = osSlistLen(&client->cachedMessageList);
    *totalLen = 0;

    for (i = 0; i < *count; i++)
    {
        if (NULL != pNode)
        {
            MQTT_CachedMessage *cacheMsg = (MQTT_CachedMessage *)pNode;
            *totalLen += cacheMsg->data.topicName->lenstring.len;
            *totalLen += cacheMsg->data.message->payloadlen;
            pNextNode = osSlistNext(pNode);
            pNode = pNextNode;
        }
        else
        {
            break;
        }
    }
}

int MQTT_ReadCacheMessage(MQTT_Handle handle, uint32_t count, MQTT_MessageData* data[])
{
    MQTTClient* client = (MQTTClient *)handle;
    int i;
    uint32_t readCount = 0;
    struct osSlistNode *pNode = NULL;
    struct osSlistNode *pNextNode = NULL;

    MutexLock(&client->mutex);
    pNode= osSlistFirst(&client->cachedMessageList);
    for (i = 0; i < count; i++)
    {
        if (NULL != pNode)
        {
            MQTT_CachedMessage *cacheMsg = (MQTT_CachedMessage *)pNode;
            data[i] = &cacheMsg->data;
            pNextNode = osSlistNext(pNode);
            pNode = pNextNode;
            readCount++;
        }
        else
        {
            break;
        }
    }
    MutexUnlock(&client->mutex);

    return readCount;
}

void MQTT_FreeCacheMessage(MQTT_Handle handle, uint32_t count)
{
    MQTTClient* client = (MQTTClient *)handle;
    int i;

    MutexLock(&client->mutex);
    for (i = 0; i < count; i++)
    {
        if (!osSlistIsEmpty(&client->cachedMessageList))
        {
            struct osSlistNode *pNode = osSlistFirst(&client->cachedMessageList);
            MQTT_CachedMessage *cacheMsg = (MQTT_CachedMessage *)pNode;
            client->cachedSize -= cacheMsg->size;
            client->cachedSize = (client->cachedSize >= 0) ? client->cachedSize : 0;
            osSlistRemove(&client->cachedMessageList, pNode);
            osFree(cacheMsg);
        }
        else
        {
            break;
        }
    }
    MutexUnlock(&client->mutex);
}

void MQTTProcConnEvent(MQTT_EventInfo *eventInfo)
{
    int rc = FAILURE;
    MQTT_ConnInfo *connInfo = (MQTT_ConnInfo *)eventInfo->buf;
    MQTTClient *client = eventInfo->client;

    MQTT_PRINT_INFO("MQTTProcConnEvent:proc connReq event, client:%p!\r\n", eventInfo->client);
    if (client->ipstack != NULL && client->ipstack->handle == (uintptr_t)(-1))
    {
        rc = MQTTNetworkConnect(client->ipstack);
        if (0 != rc)
        {
            MQTT_PRINT_ERROR("Establish network failed, check param, client:%p!\r\n", client);
            if (client->eventReport)
            {
                client->eventReport((MQTT_Handle)client, NULL, MQTT_EVENT_CONN_ERR);
            }

            goto conn_err_free;
        }
        MQTT_PRINT_INFO("Establish network success, client:%p!\r\n", client);
    }
    else
    {
        MQTT_PRINT_INFO("Network is opened, client:%p!\r\n", client);
    }

    if ((rc = MQTTConnectFunc(client, connInfo->connectData)) != 0)
    {
        MQTT_PRINT_ERROR("MQTT connect failed, rc:%d, client:%p\r\n", rc, client);
        if (client->eventReport)
        {
            client->eventReport((MQTT_Handle)client, NULL, MQTT_EVENT_CONN_ERR);
        }

        goto conn_err_free;
    }

    MQTT_PRINT_INFO("Start mqtt conn req timeout timer, client:%p!\r\n", client);
    TimerCountdownMS(&client->processingReqTimeout, client->command_timeout_ms);

    return;

conn_err_free:
    MQTTFreeProcessingReq(client);
}

void MQTTProcReConnEvent(MQTT_EventInfo *eventInfo)
{
    int rc = FAILURE;
    MQTT_ConnInfo *connInfo = (MQTT_ConnInfo *)eventInfo->buf;
    MQTTClient *client = eventInfo->client;
    MQTT_PRINT_INFO("MQTTProcReConnEvent:proc reconnReq event, client:%p!\r\n", eventInfo->client);

    MQTTNetworkDisconnect(client->ipstack);

    rc = MQTTNetworkConnect(client->ipstack);
    if (0 != rc)
    {
        MQTT_PRINT_ERROR("Re-establish network failed, check param, client:%p!\r\n", client);
        if (client->eventReport)
        {
            client->eventReport((MQTT_Handle)client, NULL, MQTT_EVENT_CONN_ERR);
        }

        goto reconn_err_free;
    }
    MQTT_PRINT_INFO("Re-establish network success, client:%p!\r\n", client);

    if ((rc = MQTTConnectFunc(client, connInfo->connectData)) != 0)
    {
        MQTT_PRINT_ERROR("MQTT reconnect failed, rc:%d, client:%p\r\n", rc, client);
        if (client->eventReport)
        {
            client->eventReport((MQTT_Handle)client, NULL, MQTT_EVENT_CONN_ERR);
        }

        goto reconn_err_free;
    }

    MQTT_PRINT_INFO("Start mqtt reconn req timeout timer, client:%p!\r\n", client);
    TimerCountdownMS(&client->processingReqTimeout, client->command_timeout_ms);

    return;

reconn_err_free:
    MQTTFreeProcessingReq(client);
    // eventInfo 在上一步已经释放，不能再使用了，所有使用缓存的client
    MQTTStartReconn(client);
}

void MQTTProcSubEvent(MQTT_EventInfo *eventInfo)
{
    int rc = FAILURE;
    MQTT_SubInfo *subInfo = (MQTT_SubInfo *)eventInfo->buf;
    MQTTClient *client = eventInfo->client;
    MQTT_PRINT_INFO("MQTTProcSubEvent:proc subReq event, client:%p!\r\n", eventInfo->client);

    if ((rc = MQTTSubscribeFunc(client, subInfo->id, subInfo->topicCnt, subInfo->topics,
        subInfo->QoSs, subInfo->messageHandler)) != 0)
    {
        MQTT_PRINT_ERROR("MQTT subscribe failed, rc:%d, client:%p\r\n", rc, client);
        if (client->eventReport)
        {
            client->eventReport((MQTT_Handle)client, &subInfo->id, MQTT_EVENT_SUB_ERR);
        }

        MQTTFreeProcessingReq(client);

        return;
    }

    MQTT_PRINT_INFO("Start mqtt sub req timeout timer, client:%p\r\n", client);
    TimerCountdownMS(&client->processingReqTimeout, client->command_timeout_ms);
}

void MQTTProcUnsubEvent(MQTT_EventInfo *eventInfo)
{
    int rc = FAILURE;
    MQTT_UnsubInfo *unsubInfo = (MQTT_UnsubInfo *)eventInfo->buf;
    MQTTClient *client = eventInfo->client;
    MQTT_PRINT_INFO("MQTTProcUnsubEvent:proc unsubReq event, client:%p!\r\n", eventInfo->client);

    if ((rc = MQTTUnsubscribeFunc(client, unsubInfo->id, unsubInfo->topicCnt, unsubInfo->topics)) != 0)
    {
        MQTT_PRINT_ERROR("MQTT unsubscribe failed, rc:%d, client:%p\r\n", rc, client);
        if (client->eventReport)
        {
            client->eventReport((MQTT_Handle)client, &unsubInfo->id, MQTT_EVENT_UNSUB_ERR);
        }

        MQTTFreeProcessingReq(client);

        return;
    }

    MQTT_PRINT_INFO("Start mqtt unsub req timeout timer, client:%p!\r\n", client);
    TimerCountdownMS(&client->processingReqTimeout, client->command_timeout_ms);
}

void MQTTProcPubEvent(MQTT_EventInfo *eventInfo)
{
    int rc = FAILURE;
    MQTT_PubInfo *pubInfo = (MQTT_PubInfo *)eventInfo->buf;
    MQTTClient *client = eventInfo->client;
    MQTT_PRINT_INFO("MQTTProcPubEvent:proc pubReq event, client:%p!\r\n", eventInfo->client);

    if ((rc = MQTTPublishFunc(client, pubInfo->topic, pubInfo->message)) != 0)
    {
        MQTT_PRINT_ERROR("MQTT pub failed, rc:%d, client:%p\r\n", rc, client);
        if (client->eventReport)
        {
            client->eventReport((MQTT_Handle)client, NULL, MQTT_EVENT_PUB_ERR);
        }

        MQTTFreeProcessingReq(client);

        return;
    }

    if (MQTT_QOS0 == pubInfo->message->qos)
    {
        MQTT_PRINT_INFO("MQTT pub Qos0 ok, client:%p\r\n", client);
        if (client->eventReport)
        {
            client->eventReport((MQTT_Handle)client, NULL, MQTT_EVENT_PUB_QOS0);
        }

        // QoS=0, 无后续异步消息上报
        MQTTFreeProcessingReq(client);
    }
    else
    {
        if (client->retryTimes > 0)
        {
            // 启动超时重传定时器
            unsigned int retransTimeout;

            client->currentRetryTimes = 0;
            retransTimeout = client->retransInterval * ((2 << (client->currentRetryTimes)) - 1);
            MQTT_PRINT_INFO("First start mqtt retrans timer, timeout:%d, retryTimes:%d, client:%p!\r\n",
                retransTimeout, client->currentRetryTimes, client);
            TimerCountdown(&client->retransTimer, retransTimeout);
        }
        else
        {
            MQTT_PRINT_INFO("Start mqtt pub req timeout timer, client:%p!\r\n", client);
            TimerCountdownMS(&client->processingReqTimeout, client->command_timeout_ms);
        }
    }
}

void MQTTProcPubResponseTimeoutEvent(MQTT_EventInfo *eventInfo)
{
    int rc = FAILURE;
    unsigned int timeout;
    MQTTClient *client = eventInfo->client;
    MQTT_PubInfo *pubInfo = (MQTT_PubInfo *)eventInfo->buf;

    OS_ASSERT(MQTT_QOS1 == pubInfo->message->qos || MQTT_QOS2 == pubInfo->message->qos);

    if (client->currentRetryTimes >= client->retryTimes)
    {
        MQTT_PRINT_ERROR("MQTT retrans retryTimes:%d, exceed maxRetryTimes:%d, client:%p\r\n",
            client->currentRetryTimes, client->retryTimes, client);
        if (client->eventReport)
        {
            MQTTEventType eventType = (MQTT_QOS1 == pubInfo->message->qos) ? MQTT_EVENT_PUBACK_TIMEOUT : MQTT_EVENT_PUBREC_TIMEOUT;
            client->eventReport((MQTT_Handle)client, NULL, eventType);
        }

        goto pub_response_err_free;
    }

    if ((rc = MQTTPublishFunc(client, pubInfo->topic, pubInfo->message)) != 0)
    {
        MQTT_PRINT_ERROR("MQTT repub failed, rc:%d, client:%p\r\n", rc, client);
        if (client->eventReport)
        {
            client->eventReport((MQTT_Handle)client, NULL, MQTT_EVENT_PUB_ERR);
        }

        goto pub_response_err_free;
    }

    // 启动超时重传定时器
    client->currentRetryTimes++;
    OS_ASSERT(client->currentRetryTimes >= 0 && client->currentRetryTimes <= 3);
    timeout = client->retransInterval * ((2 << (client->currentRetryTimes)) - 1);
    MQTT_PRINT_INFO("Start mqtt pubReq-retrans timer, timeout:%u, retryTimes:%d, client:%p!\r\n",
        timeout, client->currentRetryTimes, client);
    TimerCountdown(&client->retransTimer, timeout);

    if (client->eventReport)
    {
        MQTTEventType eventType = (MQTT_QOS1 == pubInfo->message->qos) ? MQTT_EVENT_PUBACK_TIMEOUT_RETRANS : MQTT_EVENT_PUBREC_TIMEOUT_RETRANS;
        client->eventReport((MQTT_Handle)client, NULL, eventType);
    }

    return;

pub_response_err_free:
    MQTTFreeProcessingReq(client);
}

void MQTTProcPubCompTimeoutEvent(MQTT_EventInfo *eventInfo)
{
    int len;
    MQTTClient *client = eventInfo->client;
    MQTT_PubInfo *pubInfo = (MQTT_PubInfo *)eventInfo->buf;
    unsigned short mypacketid = getCurrentPacketId(client);
    unsigned char dup = 1;
    int rc = SUCCESS;

    OS_ASSERT(MQTT_QOS2 == pubInfo->message->qos);

    if (client->currentRetryTimes >= client->retryTimes)
    {
        MQTT_PRINT_ERROR("MQTT retryTimes:%d, exceed maxRetryTimes:%d, client:%p\r\n",
            client->currentRetryTimes, client->retryTimes, client);
        if (client->eventReport)
        {
            client->eventReport((MQTT_Handle)client, NULL, MQTT_EVENT_PUBCOMP_TIMEOUT);
        }

        rc = FAILURE;
        goto pub_comp_err_free;
    }

    if ((len = MQTTSerialize_ack(client->buf, client->buf_size,
        PUBREL, dup, mypacketid)) <= 0)
    {
        rc = FAILURE;
        goto exit; // there was a problem
    }
    else
    {
        Timer timer = {0, 0, NULL};
        TimerInit(&timer, NULL);
        TimerCountdownMS(&timer, client->command_timeout_ms);
        // send the PUBREL packet
        if ((rc = sendPacket(client, len, &timer)) != SUCCESS)
        {
            TimerRelease(&timer);
            rc = FAILURE; // there was a problem
            goto exit; // there was a problem
        }
        else
        {
            unsigned int timeout;
            TimerRelease(&timer);
            client->currentRetryTimes++;
            OS_ASSERT(client->currentRetryTimes >= 0 && client->currentRetryTimes <= 3);
            timeout = client->retransInterval * ((2 << (client->currentRetryTimes)) - 1);
            MQTT_PRINT_INFO("Start mqtt pubrelReq-retrans timer, timeout:%u, retryTimes:%d, client:%p!\r\n",
                timeout, client->currentRetryTimes, client);
            TimerCountdown(&client->retransTimer, timeout);

            OS_ASSERT(MQTT_EVENT_PUBREL == client->reqInfo.eventId);

            if (client->eventReport)
            {
                client->eventReport((MQTT_Handle)client, NULL, MQTT_EVENT_PUBCOMP_TIMEOUT_RETRANS);
            }
        }
    }
    return;

exit:
    if ((rc == FAILURE) && client->isconnected)
    {
        MQTTCloseSession(client, OS_TRUE);
    }

pub_comp_err_free:
    MQTTFreeProcessingReq(client);
}

void MQTTProcDiscEvent(MQTT_EventInfo *eventInfo)
{
    MQTTClient *client = eventInfo->client; // MQTTClientDeInit会清除processingReqEvent
    MQTTEventCallback eventCallback = client->eventReport; // MQTTClientDeInit会清除eventReport

    MQTT_PRINT_INFO("MQTTProcDiscEvent:proc discReq event, client:%p!\r\n", eventInfo->client);

    MQTTDisconnectFunc(client);
    if (0 == client->syncFlag)
    {
        // 同步模式接口此处不能调用, 需要用户在MQTT_Disconnect成功后调用
        MQTT_DeinitClient((MQTT_Handle)client);
    }
    else
    {
        MQTTNetworkDisconnect(client->ipstack);
    }

    if (eventCallback)
    {
        eventCallback((MQTT_Handle)client, NULL, MQTT_EVENT_DISC_OK);
    }
}

void MQTTProcRecvEvent(MQTT_EventInfo *eventInfo)
{
    MQTT_PRINT_INFO("MQTTProcRecvEvent:proc socket data event, client:%p!\r\n", eventInfo->client);

    Timer timer = {0, 0, NULL};
    MQTTClient *client = eventInfo->client;
    TimerInit(&timer, NULL);
    TimerCountdownMS(&timer, client->command_timeout_ms);

    while (1)
    {
        if (MQTTCycle(client, &timer) <= 0)
        {
            break;
        }
    }

    TimerRelease(&timer);
}

void MQTT_InitClientDefaultParam(MQTT_Handle handle)
{
    MQTTClient* client = (MQTTClient *)handle;
    OS_ASSERT(NULL != client);
    client->keepAliveInterval = MQTT_DEFAULT_KEEP_ALIVE_INTERVAL;
    client->pingInterval = MQTT_DEFAULT_PING_INTERVAL;
    client->cleansession = MQTT_DEFAULT_CLEAN_SESSION;
    client->retransInterval = MQTT_DEFAULT_RETRANS_INTERVAL;
    client->retryTimes = MQTT_DEFAULT_RETRANS_RETRY_TIMES;
    client->reconnTimes = MQTT_DEFAULT_RECONN_TIMES;
    client->reconnInterval = MQTT_DEFAULT_RECONN_INTERVAL;
    client->reconnMode = MQTT_DEFAULT_RECONN_MODE;
    client->cachedMode = MQTT_DEFAULT_CACHE_MODE;
    client->reconnEnable = OS_TRUE;
    client->defaultParamInit = OS_TRUE;
}

// 用户未配置的参数使用默认值
void MQTTClientCheckParam(MQTTClient* c)
{
    if (!(c->keepAliveInterval == 0 || (0 <= c->keepAliveInterval && c->keepAliveInterval <= 65535)))
    {
        c->keepAliveInterval = MQTT_DEFAULT_KEEP_ALIVE_INTERVAL;
    }

    if (!(5 <= c->pingInterval && c->pingInterval <= 86400))
    {
        c->pingInterval = MQTT_DEFAULT_PING_INTERVAL;
    }

    if (!(0 == c->cleansession || 1 == c->cleansession))
    {
        c->cleansession = MQTT_DEFAULT_CLEAN_SESSION;
    }

    if (!(20 <= c->retransInterval && c->cleansession <= 60))
    {
        c->retransInterval = MQTT_DEFAULT_RETRANS_INTERVAL;
    }

    if (!(0 <= c->retryTimes && c->retryTimes <= 3))
    {
        c->retryTimes = MQTT_DEFAULT_RETRANS_RETRY_TIMES;
    }

    if (!(0 <= c->reconnTimes && c->reconnTimes <= 3))
    {
        c->reconnTimes = MQTT_DEFAULT_RECONN_TIMES;
    }

    if (!(20 <= c->reconnInterval && c->reconnInterval <= 60))
    {
        c->reconnInterval = MQTT_DEFAULT_RECONN_INTERVAL;
    }

    if (!(0 == c->reconnMode || 1 == c->reconnMode))
    {
        c->reconnMode = MQTT_DEFAULT_RECONN_MODE;
    }

    if (!(0 == c->cachedMode || 1 == c->cachedMode))
    {
        c->cachedMode = MQTT_DEFAULT_CACHE_MODE;
    }
}

int MQTT_InitClientImpl(MQTT_Handle handle, unsigned int commandTimeoutMs,
        unsigned char* sendbuf, size_t sendbufSize, unsigned char* readbuf, size_t readbufSize, uint8_t syncFlag)
{
    int i;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    MQTTClient *c = (MQTTClient *)handle;

    MQTT_Init();
    MutexLock(&g_mqttWorkMutex);
    MQTTCleanSession(c);

    c->next_packetid = 1;
    c->command_timeout_ms = (commandTimeoutMs < MQTT_MIN_COMMAND_TIMEOUT) ? MQTT_MIN_COMMAND_TIMEOUT : commandTimeoutMs;
    c->buf = sendbuf;
    c->buf_size = sendbufSize;
    c->readbuf = readbuf;
    c->readbuf_size = readbufSize;
    c->ping_outstanding = 0;
    c->isconnected = 0;
    c->defaultMessageHandler = NULL;
    c->eventReport = NULL;
    c->currentRetryTimes = 0;
    c->currentReconnTimes = 0;
    c->cachedSize = 0;
    osMemcpy(&c->connectData, &default_options, sizeof(MQTTPacket_connectData));

    MQTTClientCheckParam(c);

    osSlistInit(&c->cachedMessageList);
    TimerInit(&c->keepAliveTimer, MQTTKeepAliveTimeout);
    TimerInit(&c->retransTimer, MQTTRetransTimeout);
    TimerInit(&c->reConnTimer, MQTTReconnTimeout);
    c->reqInfo.reqStartTick = osTickGet();
    c->reqInfo.reqEndTick = osTickGet();
    c->reqInfo.buf = NULL;
    TimerInit(&c->processingReqTimeout, MQTTProcessingReqTimeout);

    MutexInit(&c->mutex);

    c->recvSyncEventMb = osMbCreate(32, OS_IPC_FLAG_PRIO);
    OS_ASSERT(NULL != c->recvSyncEventMb);

    for (i = 0; i < MQTT_MAX_CLIENT_CNT; i++)
    {
        if (NULL == g_mqttClients[i])
        {
            g_mqttClients[i] = c;
            break;
        }
    }
    OS_ASSERT(i < MQTT_MAX_CLIENT_CNT);

    c->syncFlag = syncFlag;
    if (0 != syncFlag)
    {
        c->eventReport = MQTT_EventDefaultCallback;
        c->cachedMode = 0;
    }
    MutexUnlock(&g_mqttWorkMutex);
    return MQTT_OK;
}

int MQTT_CreateClient(MQTT_Handle *handle, unsigned int commandTimeoutMs,
        unsigned char* sendbuf, size_t sendbufSize, unsigned char* readbuf, size_t readbufSize, MQTT_ConnectionLost connLostHandler)
{
    MQTTClient *c = osMalloc(sizeof(MQTTClient));
    if (NULL == c)
    {
        return MQTT_ERR;
    }

    osMemset(c, 0, sizeof(MQTTClient));
    c->connLostHandlerInSyncMode = connLostHandler;
    *handle = (MQTT_Handle)c;
    MQTT_InitClientDefaultParam(*handle);

    return MQTT_InitClientImpl(*handle, commandTimeoutMs, sendbuf, sendbufSize, readbuf, readbufSize, OS_TRUE);
}

int MQTT_ClientGetConfigure(MQTT_Handle handle, MQTTClient_ConfigureParams *params)
{
    MQTTClient *c = (MQTTClient *)handle;
    if (NULL == c || params == NULL)
    {
        return MQTT_ERR_PARAMETER;
    }
    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_INVALID_HANDLE;
    }
    params ->reconnEnable = c->reconnEnable;
    MutexUnlock(&g_mqttWorkMutex);
    return MQTT_OK;
}

int MQTT_ClientSetConfigure(MQTT_Handle handle, MQTTClient_ConfigureParams params)
{
    MQTTClient *c = (MQTTClient *)handle;
    if (NULL == c)
    {
        return MQTT_ERR_PARAMETER;
    }
    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_INVALID_HANDLE;
    }
    c->reconnEnable = params.reconnEnable;
    MutexUnlock(&g_mqttWorkMutex);
    return MQTT_OK;
}

int MQTTAsync_InitClient(MQTT_Handle handle, unsigned int commandTimeoutMs,
        unsigned char* sendbuf, size_t sendbufSize, unsigned char* readbuf, size_t readbufSize)
{
    MQTTClient *c = (MQTTClient *)handle;
    OS_ASSERT(NULL != handle);
    c->connLostHandlerInSyncMode = NULL;
    c->eventNotifyHandlerInSyncMode = NULL;

    if (OS_TRUE != c->defaultParamInit)
    {
        MQTT_InitClientDefaultParam(handle);
    }

    return MQTT_InitClientImpl(handle, commandTimeoutMs, sendbuf, sendbufSize, readbuf, readbufSize, OS_FALSE);
}

int MQTT_SetNetwork(MQTT_Handle handle, Network *network)
{
    int rc = MQTT_OK;
    MQTTClient *c = (MQTTClient *)handle;

    if(NULL == handle)
    {
        MQTT_PRINT_ERROR("%s, handle is null\r\n", __FUNCTION__);
        return MQTT_ERR;
    }
    if(NULL == network)
    {
        MQTT_PRINT_ERROR("%s, network is null\r\n", __FUNCTION__);
        return MQTT_ERR_BUSY;
    }

    while (MQTT_IsBusy(handle))
    {
        osThreadMsSleep(100);
    }

    MutexLock(&g_mqttWorkMutex);
    if(!MQTT_ClientIsValid(handle))
    {
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_INVALID_HANDLE;
    }

    if(NULL != c->ipstack )
    {
        MQTT_PRINT_ERROR("%s, ipstack is used\r\n", __FUNCTION__);
        MutexUnlock(&g_mqttWorkMutex);
        return MQTT_ERR_TIMEOUT;
    }

    c->ipstack = network;
    lwip_socket_register_callback(network->fd, MQTT_SocketRecvEventCallback, NULL); /* 注册 LWIP 事件回调函数 */
    MutexUnlock(&g_mqttWorkMutex);
    return rc;
}

Network* MQTT_RemoveNetwork(MQTT_Handle handle)
{
    MQTTClient *c = (MQTTClient *)handle;
    Network* network = NULL;

    if(NULL == handle)
    {
        MQTT_PRINT_ERROR("%s, handle is null\r\n", __FUNCTION__);
        return NULL;
    }

    while (MQTT_IsBusy(handle))
    {
        osThreadMsSleep(100);
    }
    MutexLock(&g_mqttWorkMutex);
    if(MQTT_ClientIsValid(handle))
    {
        network = c->ipstack;
        c->ipstack = NULL;
    }
    MutexUnlock(&g_mqttWorkMutex);
    return network;
}


void MQTT_DeinitClient(MQTT_Handle handle)
{
    int i;
    MQTTClient *c = (MQTTClient *)handle;

    OS_ASSERT(NULL != handle);
    while (MQTT_IsBusy(handle))
    {
        osThreadMsSleep(100);
    }
    MutexLock(&g_mqttWorkMutex);
    MQTT_PRINT_INFO("MQTT_DeinitClient client:%p!\r\n", c);
    MQTTCleanEvent(c);
    //  ipstack 用户和MQTT都可以创建
    //  如果用户创建并通过MQTT_SetNetwork配置，在销毁Client前要通过MQTT_RemoveNetwork删除，以避免重复释放内存
    if(c->ipstack)
    {
        MQTT_PRINT_INFO("MQTT_DeinitClient close net client:%p!\r\n", c);
        MQTTNetworkDisconnect(c->ipstack);
        osFree(c->ipstack);
    }
    /* after do MQTTCloseSession */
    c->ipstack = NULL;
    c->command_timeout_ms = 0;
    c->buf = NULL;
    c->buf_size = 0;
    c->readbuf = NULL;
    c->readbuf_size = 0;
    c->cleansession = 0;
    c->defaultMessageHandler = NULL;
    c->eventReport = NULL;
    c->next_packetid = 0;
    c->currentReconnTimes = 0;
    c->currentRetryTimes = 0;

    TimerRelease(&c->keepAliveTimer);
    TimerRelease(&c->retransTimer);
    TimerRelease(&c->reConnTimer);
    TimerRelease(&c->processingReqTimeout);

    MQTTCleanSession(c);

    for (i = 0; i < MQTT_MAX_CLIENT_CNT; i++)
    {
        if (c == g_mqttClients[i])
        {
            g_mqttClients[i] = NULL;
            break;
        }
    }

    while (!osSlistIsEmpty(&c->cachedMessageList))
    {
        struct osSlistNode *pNode = osSlistFirst(&c->cachedMessageList);
        MQTT_CachedMessage *cacheMessage = (MQTT_CachedMessage *)pNode;
        osSlistRemove(&c->cachedMessageList, pNode);
        osFree(cacheMessage);
    }

    MQTTFreeProcessingReq(c);
    c->reqInfo.reqStartTick = osTickGet();
    c->reqInfo.reqEndTick = osTickGet();

    MutexDeInit(&c->mutex);
    osMbDestory(c->recvSyncEventMb);
    MutexUnlock(&g_mqttWorkMutex);
}

void MQTT_DestroyClient(MQTT_Handle* handle)
{
    if (*handle)
    {
        MutexLock(&g_mqttWorkMutex);
        if(!MQTT_ClientIsValid(*handle))
        {
            MutexUnlock(&g_mqttWorkMutex);
            return;
        }
        MQTT_DeinitClient(*handle);

        osFree(*handle);
        *handle = NULL;
        MutexUnlock(&g_mqttWorkMutex);
    }
}

int MQTT_Init(void)
{
    static int s_init = OS_FALSE;
    int rc;
    int i;

    if (s_init)
    {
        return MQTT_OK;
    }

    osInitCompletion(&g_mqttCmpl);
    osSlistInit(&g_mqttEventList);
    MQTT_WorkMutexInit();

    g_mqttThread = NULL;
    rc = ThreadStart(&g_mqttThread, &MQTTAsyncProc, NULL);
    OS_ASSERT(TRUE == rc);
    OS_ASSERT(NULL != g_mqttThread);

    for (i = 0; i < MQTT_MAX_CLIENT_CNT; i++)
    {
        g_mqttClients[i] = NULL;
    }

    s_init = OS_TRUE;

    return MQTT_OK;
}

void MQTT_EventDefaultCallback(MQTT_Handle handle, void *userData, MQTTEventType eventType)
{
    bool_t syncEventRpt = OS_FALSE;
    MQTTEventType syncEventType;
    MQTTClient *client = (MQTTClient *)handle;
    osTick_t tick = osTickGet();

    OS_ASSERT(NULL != client);
    MQTT_PRINT_INFO("Mqtt syncEvent callback, eventType:%d (%s), client:%p\r\n", eventType, MQTT_EventString(eventType), client);
    if (client->syncFlag && (MQTT_EVENT_PINGRESP == eventType || MQTT_EVENT_NET_CLOSE == eventType))
    {
        MQTT_PRINT_INFO("Mqtt syncEvent callback, drop eventType:%d (%s), client:%p in sync mode\r\n", eventType, MQTT_EventString(eventType), client);
        return;
    }

    if ((0 == client->syncFlag
        || tick < client->reqInfo.reqStartTick
        || tick > client->reqInfo.reqEndTick)
        && MQTT_EVENT_DISC_OK !=eventType) //MQTT_EVENT_DISC_OK不需要判断超时，因为不存在服务器ACK
    {
        uint32_t tickHigh = (tick >> 32) & 0xFFFFFFFF;
        uint32_t tickLow = tick & 0xFFFFFFFF;
        uint32_t reqStartTickHigh = (client->reqInfo.reqStartTick >> 32) & 0xFFFFFFFF;
        uint32_t reqStartTickLow = client->reqInfo.reqStartTick & 0xFFFFFFFF;
        uint32_t reqEndTickHigh = (client->reqInfo.reqEndTick >> 32) & 0xFFFFFFFF;
        uint32_t reqEndTickLow = client->reqInfo.reqEndTick & 0xFFFFFFFF;

        MQTT_PRINT_WARN("Mqtt invalid syncEvent, syncFlag:%u, tickHigh:%u, tickLow:%u, \
                        reqStartTickHigh:%u, reqStartTickLow:%u, reqEndTickHigh:%u, reqEndTickLow:%u, client:%p!\r\n",
            client->syncFlag,
            tickHigh,
            tickLow,
            reqStartTickHigh,
            reqStartTickLow,
            reqEndTickHigh,
            reqEndTickLow,
            client);

        return;
    }

    switch (eventType)
    {
    case MQTT_EVENT_CONN_TIMEOUT:
    case MQTT_EVENT_RECONN_TIMEOUT:
    case MQTT_EVENT_SUB_TIMEOUT:
    case MQTT_EVENT_UNSUB_TIMEOUT:
    case MQTT_EVENT_PUB_TIMEOUT:
    case MQTT_EVENT_PUBACK_TIMEOUT:
    case MQTT_EVENT_PUBREC_TIMEOUT:
    case MQTT_EVENT_PUBCOMP_TIMEOUT:
        {
            syncEventType = MQTT_EVENT_TIMEOUT;
            syncEventRpt = OS_TRUE;
        }
        break;

    case MQTT_EVENT_CONN_ERR:
    case MQTT_EVENT_PUB_ERR:
    case MQTT_EVENT_SUB_ERR:
    case MQTT_EVENT_UNSUB_ERR:
        {
            syncEventType = MQTT_EVENT_ERROR;
            syncEventRpt = OS_TRUE;
        }
        break;

    case MQTT_EVENT_CONNACK:
    case MQTT_EVENT_SUBACK:
    case MQTT_EVENT_UNSUBACK:
    case MQTT_EVENT_PUB_QOS0:
    case MQTT_EVENT_PUBACK:
    case MQTT_EVENT_PUBCOMP:
    case MQTT_EVENT_DISC_OK:
        {
            syncEventType = eventType;
            syncEventRpt = OS_TRUE;
        }
        break;

    case MQTT_EVENT_SESSION_CLOSED:
        {
            syncEventType = eventType;
            syncEventRpt = OS_TRUE;
        }

    case MQTT_EVENT_PINGRESP:
        {
            if (0 != client->ping_outstanding)
            {
                syncEventType = eventType;
                syncEventRpt = OS_TRUE;
            }
        }

    default:
        break;
    }

    if (syncEventRpt && client->reqInfo.eventId != MQTT_EVENT_RECONN) //  MQTT_EVENT_RECONN是内部事件，其结果不需要发送到Mailbox
    {
        MQTTEventType *pEventType = (MQTTEventType *)osMalloc(sizeof(MQTTEventType));
        if (pEventType)
        {
            *pEventType = syncEventType;
            MQTT_PRINT_INFO("Mqtt send syncEvent:%d (%s) to sync api, client:%p!\r\n", syncEventType, MQTT_EventString(syncEventType), client);
            osMbSend(client->recvSyncEventMb, (ubase_t)pEventType, osWaitForever);
        }
    }

    if (MQTT_EVENT_SESSION_CLOSED == eventType && NULL != client->connLostHandlerInSyncMode)
    {
        MQTT_PRINT_WARN("Mqtt recv session closed event, connection lost, client:%p!\r\n", client);
        client->connLostHandlerInSyncMode((MQTT_Handle)client, NULL);
    }

    if (MQTT_EVENT_SESSION_CLOSED == eventType && NULL != client->eventNotifyHandlerInSyncMode)
    {
        MQTT_PRINT_WARN("Mqtt notify session closed event, connection lost, client:%p!\r\n", client);
        client->eventNotifyHandlerInSyncMode((MQTT_Handle)client, MQTT_NOTIFY_EVENT_CLOSE_SESSION, NULL);
    }
}

int MQTT_ClientSetEventNotify(MQTT_Handle handle, MQTT_EventNotify_t notifyHandler)
{
    MQTTClient *c = (MQTTClient *)handle;
    if (NULL == c)
    {
        return MQTT_ERR;
    }
    MutexLock(&g_mqttWorkMutex);
    if(MQTT_ClientIsValid(handle))
    {
        c->eventNotifyHandlerInSyncMode = notifyHandler;
    }
    MutexUnlock(&g_mqttWorkMutex);
    return MQTT_OK;
}

int MQTT_ClientSetDefaultMessageHandler(MQTT_Handle handle, MQTT_MessageHandler defaultHandler)
{
    MQTTClient *c = (MQTTClient *)handle;
    if (NULL == c)
    {
        return MQTT_ERR;
    }
    MutexLock(&g_mqttWorkMutex);
    if(MQTT_ClientIsValid(handle))
    {
        c->defaultMessageHandler = defaultHandler;
    }
    MutexUnlock(&g_mqttWorkMutex);
    return MQTT_OK;
}


