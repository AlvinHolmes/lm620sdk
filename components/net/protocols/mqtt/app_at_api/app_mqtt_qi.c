#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "at_api.h"
#include "net_pub.h"
#include "app_at_name_space.h"
#include "app_mqtt_qi.h"
#include "MQTTClient.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"

// #define MQTT_QI_DEBUG_PRINTF
#ifdef  MQTT_QI_DEBUG_PRINTF
    #define APP_MQTT_QI_PRINTF_INFO        osPrintf
    #define APP_MQTT_QI_PRINTF_ERROR       osPrintf
#else
    #define APP_MQTT_QI_PRINTF_INFO        MQTT_PRINT_INFO
    #define APP_MQTT_QI_PRINTF_ERROR       MQTT_PRINT_ERROR
#endif

extern void MQTT_SocketRecvEventCallback(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param);

typedef struct
{
    MQTTClient           client;
    APP_MqttQiContext   *mqttContext;
} APP_MqttQiClient;

APP_MqttQiContext g_appMqttQiContext[APP_MQTT_QI_CLIENT_CNT];
APP_MqttQiClient  g_appMqttQiClient[APP_MQTT_QI_CLIENT_CNT];

void APP_MqttQiEventReport(MQTT_Handle handle, void *userData, MQTTEventType eventType);
void APP_MqttQiMessageHandler(MQTT_Handle handle, MQTT_MessageData *data);

static int APP_MqttQiNetRead(Network *pNetwork, unsigned char *buffer, int len, int timeoutMs)
{
    int ret = -1;

    if (NULL == pNetwork || NULL == buffer || 0 == len)
    {
        APP_MQTT_QI_PRINTF_ERROR("MqttNetRead:network/buffer/len is null");
        return -1;
    }

    if ((uintptr_t)(-1) != pNetwork->handle)
    {
        AT_SslContextEx *sslCtxEx = (AT_SslContextEx *)pNetwork->handle;
        //在兼容非SSL时timeoutMS为0表示的非阻塞，但在SSL里timeoutMS为0表示一直wait
        //这里避免传0，兼容两种场景。timeoutMs设置成比较小的等待，比省电的sleep门限要小
        if (timeoutMs == 0) timeoutMs = 5;
        ret = AT_SslRead(sslCtxEx->sslContex.ssl, (char *)buffer, len, timeoutMs);
    }

    return ret;
}

static int APP_MqttQiNetWrite(Network *pNetwork, unsigned char *buffer, int len, int timeoutMs)
{
    int ret = -1;

    if (NULL == pNetwork || NULL == buffer || 0 == len)
    {
        APP_MQTT_QI_PRINTF_ERROR("MqttNetWrite:network/buffer/len is null");
        return -1;
    }

    if ((uintptr_t)(-1) != pNetwork->handle)
    {
        AT_SslContextEx *sslCtxEx = (AT_SslContextEx *)pNetwork->handle;
        ret = AT_SslWrite(sslCtxEx->sslContex.ssl, (char *)buffer, len);
    }

    return ret;
}

static int APP_MqttQiNetConnect(Network *pNetwork, uint8_t cid, uint8_t sslCtxId)
{
    int ret = -1;
    if (NULL == pNetwork)
    {
        APP_MQTT_QI_PRINTF_ERROR("MqttNetConn:network is null");
        return -1;
    }

    if ((uintptr_t)(-1) == pNetwork->handle)
    {
        AT_SslContextEx *sslCtxEx = NULL;

        sslCtxEx = AT_SslMbedtlsEstablish(pNetwork->pHostAddress, pNetwork->port, cid, sslCtxId, OS_TRUE);
        if (NULL != sslCtxEx)
        {
            pNetwork->handle = (intptr_t)sslCtxEx;

            /* 注册 LWIP 事件回调函数 */
            pNetwork->fd = sslCtxEx->sslContex.serverFd->fd;
            lwip_socket_register_callback(sslCtxEx->sslContex.serverFd->fd, MQTT_SocketRecvEventCallback, NULL);
            ret = 0;
        }
    }

    return ret;
}

static int APP_MqttQiNetDisconnect(Network *pNetwork)
{
    int ret = -1;

    if (NULL == pNetwork)
    {
        APP_MQTT_QI_PRINTF_ERROR("MqttNetDisc:network is null");
        return -1;
    }

    if ((uintptr_t)(-1) != pNetwork->handle)
    {
        AT_SslContextEx *sslCtxEx = (AT_SslContextEx *)pNetwork->handle;
        AT_SslMbedtlsDestroy(sslCtxEx);
        pNetwork->handle = (uintptr_t)(-1);
        ret = 0;
    }

    return ret;
}

void APP_MqttQiFreeConnResource(uint8_t clientIdx)
{
    int32_t i;
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (NULL != Context->transTimer.timeOut)
    {
        MqttQiTimerStop(&Context->transTimer);
        MqttQiTimerRelease(&Context->transTimer);
    }

    if (NULL != Context->pubExReadyTimer.timeOut)
    {
        MqttQiTimerStop(&Context->pubExReadyTimer);
        MqttQiTimerRelease(&Context->pubExReadyTimer);
    }

    if (NULL != Context->retransTimer.timeOut)
    {
        MqttQiTimerStop(&Context->retransTimer);
        MqttQiTimerRelease(&Context->retransTimer);
    }

    if (NULL != Context->connectData.clientID.cstring)
    {
        osFree(Context->connectData.clientID.cstring);
        Context->connectData.clientID.cstring = NULL;
    }

    if (NULL != Context->connectData.username.cstring)
    {
        osFree(Context->connectData.username.cstring);
        Context->connectData.username.cstring = NULL;
    }

    if (NULL != Context->connectData.password.cstring)
    {
        osFree(Context->connectData.password.cstring);
        Context->connectData.password.cstring = NULL;
    }

    if (NULL != Context->pHost)
    {
        osFree(Context->pHost);
        Context->pHost = NULL;
    }

    Context->port = APP_MQTT_QI_PORT_MIN;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
    {
        uint32_t cleanSession = (uint32_t)Context->connectData.cleansession;
        if ((APP_MQTT_QI_CLEAN_SESSION_1 == cleanSession) && (NULL != Context->topics[i]))
        {
            osFree(Context->topics[i]);
            Context->topics[i] = NULL;
        }

        if (NULL != Context->addTopics[i])
        {
            osFree(Context->addTopics[i]);
            Context->addTopics[i] = NULL;
        }

        if (NULL != Context->unsubTopics[i])
        {
            osFree(Context->unsubTopics[i]);
            Context->unsubTopics[i] = NULL;
        }
    }

    if (Context->pubMessage)
    {
        osFree(Context->pubMessage);
        Context->pubMessage = NULL;
    }

    if (Context->pubTopic)
    {
        osFree(Context->pubTopic);
        Context->pubTopic = NULL;
    }
}

void APP_MqttQiFreeWillResource(uint8_t clientIdx)
{
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (NULL != Context->connectData.will.topicName.cstring)
    {
        osFree(Context->connectData.will.topicName.cstring);
        Context->connectData.will.topicName.cstring = NULL;
    }
    if (NULL != Context->connectData.will.message.cstring)
    {
        osFree(Context->connectData.will.message.cstring);
        Context->connectData.will.message.cstring = NULL;
    }
}

void APP_MqttQiFreeBufResource(uint8_t clientIdx)
{
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (Context->readbuf)
    {
        osFree(Context->readbuf);
        Context->readbuf = NULL;
    }

    if (Context->sendbuf)
    {
        osFree(Context->sendbuf);
        Context->sendbuf = NULL;
    }
}

void APP_MqttQiSetString(char **destbuf, char *srcBuf)
{
    size_t strLen;

    OS_ASSERT((NULL != destbuf) && (NULL == *destbuf) && (NULL != srcBuf));

    strLen = osStrlen(srcBuf);
    *destbuf = (char *)osMalloc(strLen + 1);
    OS_ASSERT(NULL != *destbuf);
    osStrncpy(*destbuf, srcBuf, strLen);

    (*destbuf)[strLen]= '\0';
}

bool_t APP_MqttQiIsConnected(uint8_t clientIdx)
{
    return MQTT_IsConnected(g_appMqttQiContext[clientIdx].clientHandle);
}

int32_t APP_MqttQiConnect(uint8_t channelId, uint8_t clientIdx, char *clientID, char *username, char *password)
{
    int32_t rc = 0;
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if(Context->isOpen == OS_FALSE)
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt conn not open, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_PARAMETER;
    }

    if (APP_MqttQiIsConnected(clientIdx))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt conn exist, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_CONN_EXIST;
    }

    if (MQTT_IsBusy(Context->clientHandle))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt busy, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_BUSY;
    }

    if (NULL == clientID || NULL == username || NULL == password)
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt param error, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_PARAMETER;
    }

    Context->channelId = channelId;
    Context->clientIdx = clientIdx;

    APP_MqttQiSetString(&Context->connectData.clientID.cstring, clientID);
    APP_MqttQiSetString(&Context->connectData.username.cstring, username);
    APP_MqttQiSetString(&Context->connectData.password.cstring, password);

    Context->msgHandler = APP_MqttQiMessageHandler;
    MQTT_SetEventCallback(Context->clientHandle, APP_MqttQiEventReport);

    if ((rc = MQTT_Connect(Context->clientHandle, &Context->connectData)) != 0)
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt connect, fail %u clientIdx:%u!\r\n", rc, (uint32_t)clientIdx);
        rc = MQTT_ERR;
        goto APP_MQTT_QI_CONN_ERR_FREE;
    }

    return 0;

APP_MQTT_QI_CONN_ERR_FREE:
    APP_MqttQiFreeConnResource(clientIdx);
    APP_MqttQiFreeBufResource(clientIdx);


    return rc;
}

/* 判断 netif 是否存在并且 up */
static int8_t APP_MqttNetifIsAct(uint8_t pdpId)
{
    struct netif *netif = NULL;

    APP_MQTT_QI_PRINTF_INFO("APP_MqttNetifIsAct pdpId:%u\r\n", (uint32_t)pdpId);
    netif = netif_get_by_cid(pdpId);
    if(NULL == netif) /* netif 未注册, PDP 未激活 */
    {
        APP_MQTT_QI_PRINTF_ERROR("act netif is null\r\n");
        return OS_FALSE;
    }

    if(!netif_is_up(netif))
    {
       APP_MQTT_QI_PRINTF_ERROR("act netif not up\r\n");
       return OS_FALSE;
    }

    return OS_TRUE;
}

static void APP_MqttQiContextClientParamInit(APP_MqttQiClientParam *mqttQiClientParam, MQTT_Handle handle)
{
    MQTTClient *client = (MQTTClient *)handle;
    mqttQiClientParam->pingInterval = client->pingInterval;
    mqttQiClientParam->retransInterval = client->retransInterval;
    mqttQiClientParam->retryTimes = client->retryTimes;
    mqttQiClientParam->reconnTimes = client->reconnTimes;
    mqttQiClientParam->reconnInterval = client->reconnInterval;
    mqttQiClientParam->reconnMode = client->reconnMode;
    mqttQiClientParam->cachedMode = client->cachedMode;
}

static void APP_MqttHandleParamUpdate(MQTT_Handle handle, APP_MqttQiClientParam *mqttQiClientParam)
{
    MQTTClient *client = (MQTTClient *)handle;
    client->pingInterval = mqttQiClientParam->pingInterval;
    client->retransInterval = mqttQiClientParam->retransInterval;
    client->retryTimes = mqttQiClientParam->retryTimes;
    client->reconnTimes = mqttQiClientParam->reconnTimes;
    client->reconnInterval = mqttQiClientParam->reconnInterval;
    client->reconnMode = mqttQiClientParam->reconnMode;
    client->cachedMode = mqttQiClientParam->cachedMode;
}

int32_t APP_MqttQiOpen(uint8_t channelId, uint8_t clientIdx, char *host, uint16_t port)
{
    bool_t clientInitFlag = OS_FALSE;
    int32_t rc = 0;
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];
    Network *pNetwork = NULL;

    if (APP_MqttQiIsConnected(clientIdx))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt conn exist, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_CONN_EXIST;
    }

    if (MQTT_IsBusy(Context->clientHandle))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt busy, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_BUSY;
    }

    if (NULL == host)
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt param error, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_PARAMETER;
    }

    if(OS_TRUE != APP_MqttNetifIsAct(g_appMqttQiContext[clientIdx].pdpCid))
    {
        APP_MQTT_QI_PRINTF_ERROR("clientID:%u, netif not act\r\n", (uint32_t)clientIdx);
        return MQTT_ERR;
    }

    APP_MqttQiFreeConnResource(clientIdx);
    APP_MqttQiFreeBufResource(clientIdx);
    // APP_MqttQiFreeWillResource(clientIdx); // CFG中配置了遗嘱消息，此处不能释放

    Context->pHost = NULL;
    Context->port = port;
    Context->sendbuf = (uint8_t *)osMalloc(APP_MQTT_QI_SEND_BUF_SIZE);
    Context->readbuf = (uint8_t *)osMalloc(APP_MQTT_QI_RECV_BUF_SIZE);
    Context->channelId = channelId;
    Context->clientIdx = clientIdx;

    APP_MqttQiSetString(&Context->pHost, host);

    APP_MqttHandleParamUpdate(Context->clientHandle, &Context->clientParam);
    rc = MQTTAsync_InitClient(Context->clientHandle,
            APP_MQTT_QI_COMMAND_TIMEOUT,
            Context->sendbuf,
            APP_MQTT_QI_SEND_BUF_SIZE,
            Context->readbuf,
            APP_MQTT_QI_RECV_BUF_SIZE);
    if (0 != rc)
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt create failed, check param!clientIdx:%u!\r\n", (uint32_t)clientIdx);
        rc = MQTT_ERR_PARAMETER;
        goto APP_MQTT_QI_CONN_ERR_FREE;
    }

    clientInitFlag = OS_TRUE;

    ((MQTTClient *)Context->clientHandle)->ipstack = osMalloc(sizeof(Network));
    if (NULL == ((MQTTClient *)Context->clientHandle)->ipstack)
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt malloc network failed!clientIdx:%u!\r\n", (uint32_t)clientIdx);
        rc = MQTT_ERR;
        goto APP_MQTT_QI_CONN_ERR_FREE;
    }
    pNetwork = ((MQTTClient *)Context->clientHandle)->ipstack;

    rc = MQTTNetworkInit(pNetwork, Context->pHost, port, NULL);
    if (0 != rc)
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt init network failed, check param!clientIdx:%u!\r\n", (uint32_t)clientIdx);
        rc = MQTT_ERR_PARAMETER;
        goto APP_MQTT_QI_CONN_ERR_FREE;
    }

    if (Context->sslEnable)
    {
        // 开启ssl, network对应的read、write、disconnect、connect设置对应支持ssl的函数
        pNetwork->mqttread = APP_MqttQiNetRead;
        pNetwork->mqttwrite = APP_MqttQiNetWrite;
        pNetwork->disconnect = APP_MqttQiNetDisconnect;
        pNetwork->connect = NULL; // ssl connect此处设置为NULL

        rc = APP_MqttQiNetConnect(pNetwork, Context->pdpCid, Context->sslCtxIdx);
        if (0 != rc)
        {
            rc = MQTT_ERR;
            goto APP_MQTT_QI_CONN_ERR_FREE;
        }
    }
    else
    {
        pNetwork->pdpCid = Context->pdpCid;
        rc = MQTTNetworkConnect(pNetwork);
        if (0 != rc)
        {
            rc = MQTT_ERR;
            goto APP_MQTT_QI_CONN_ERR_FREE;
        }
    }
    Context->isOpen = OS_TRUE;
    return 0;

APP_MQTT_QI_CONN_ERR_FREE:
    if (clientInitFlag)
    {
        MQTT_DeinitClient(Context->clientHandle);
    }
    if (NULL != pNetwork)
    {
        if (Context->sslEnable)
        {
            APP_MqttQiNetDisconnect(pNetwork);
        }
        else
        {
            MQTTNetworkDisconnect(pNetwork);
        }
    }
    APP_MqttQiFreeConnResource(clientIdx);
    APP_MqttQiFreeBufResource(clientIdx);

    return rc;

}

uint32_t APP_MqttQiGetTopicCount(uint8_t clientIdx)
{
    int32_t i = 0;
    uint32_t count = 0;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
    {
        if (NULL != g_appMqttQiContext[clientIdx].topics[i])
        {
            count++;
        }
    }

    return count;
}

int32_t APP_MqttQiSubscribe(uint8_t clientIdx, uint16_t id, uint32_t topicCnt, char* topics[], int32_t QoSs[])
{
    int32_t rc = 0;
    uint32_t i, j;
    uint32_t totalTopicCount;
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (!APP_MqttQiIsConnected(clientIdx))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt unconn, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_UNCONN;
    }

    if (MQTT_IsBusy(Context->clientHandle))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt busy, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_BUSY;
    }

    uint32_t findTopicCnt = 0;

    totalTopicCount = APP_MqttQiGetTopicCount(clientIdx);
    for (i = 0; i < topicCnt; i++)
    {
        bool_t find = OS_FALSE;
        for (j = 0; j < MAX_MESSAGE_HANDLERS; j++)
        {
            if (Context->topics[j] != NULL
                && strcmp(Context->topics[j], topics[i]) == 0)
            {
                find = OS_TRUE;
                findTopicCnt ++;
                break;
            }
        }

        if (!find)
        {
            totalTopicCount++;
        }
        Context->addTopics[i] = NULL;
        APP_MqttQiSetString(&Context->addTopics[i], topics[i]);
        Context->addQoSs[i] = QoSs[i];
    }

    if(findTopicCnt == topicCnt)
    {
        rc = MQTT_OK;
        goto APP_MQTT_QI_SUB_ERR_FREE;
    }

    if (totalTopicCount > MAX_MESSAGE_HANDLERS)
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt total subscribe count:%u, exceed max subscribe count:%u, clientIdx:%u!\r\n",
            (uint32_t)totalTopicCount,
            MAX_MESSAGE_HANDLERS,
            (uint32_t)clientIdx);
        rc = MQTT_ERR_FULL;
        goto APP_MQTT_QI_SUB_ERR_FREE;
    }

    rc = MQTT_SubscribeWithId(Context->clientHandle, id,
            topicCnt,
            (const char**)Context->addTopics,
            (int *)Context->addQoSs,
            Context->msgHandler);
    if (MQTT_OK == rc)
    {
        return MQTT_OK;
    }

APP_MQTT_QI_SUB_ERR_FREE:
    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
    {
        if (NULL != Context->addTopics[i])
        {
            osFree(Context->addTopics[i]);
            Context->addTopics[i] = NULL;
        }
    }

    return rc;
}

int32_t APP_MqttQiUnsubscribe(uint8_t clientIdx, uint16_t id, uint32_t topicCnt, char *topics[])
{
    int32_t i;
    int32_t rc = 0;
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (!APP_MqttQiIsConnected(clientIdx))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt unconn, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_UNCONN;
    }

    if (MQTT_IsBusy(Context->clientHandle))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt busy, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_BUSY;
    }

    for (i = 0; i < topicCnt; i ++)
    {
        APP_MqttQiSetString(&Context->unsubTopics[i], topics[i]);
    }

    rc = MQTT_UnsubscribeWithId(Context->clientHandle, id, (int)topicCnt, (const char **)Context->unsubTopics);
    if (MQTT_OK == rc)
    {
        return MQTT_OK;
    }

    for (i = 0; i < topicCnt; i ++)
    {
        if (Context->unsubTopics[i])
        {
            osFree(Context->unsubTopics[i]);
            Context->unsubTopics[i] = NULL;
        }
    }

    return rc;
}

void APP_MqttQiPublishFree(uint8_t clientIdx)
{
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    Context->isPub = 0; // 将指示采用的是哪种命令发布消息标志位清除

    if (Context->pubTopic)
    {
        osFree(Context->pubTopic);
        Context->pubTopic = NULL;
    }

    if (Context->pubMessage)
    {
        osFree(Context->pubMessage);
        Context->pubMessage = NULL;
    }
}

/* 获取当前发布消息采用的MsgId*/
uint16_t APP_MqttQiGetCurPublishMsgId(uint8_t clientIdx)
{
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (Context->pubMessage)
    {
        return Context->pubMessage->id;
    }

    return 0;
}

/* 指示采用的是哪种命令发布消息 1：QMTPUB，0：QMTPUBEX*/
uint8_t APP_MqttQiGetCurPublishCommType(uint8_t clientIdx)
{
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (Context->pubMessage)
    {
        return Context->isPub;
    }

    return 0;
}

int32_t APP_MqttQiPublish(uint8_t clientIdx, uint16_t id, char *topic, int32_t qos, uint8_t retain,
            uint8_t dup, void *payload, size_t payloadlen, uint8_t pubex)
{
    int32_t rc = 0;
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (!APP_MqttQiIsConnected(clientIdx))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt unconn, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_UNCONN;
    }

    if (MQTT_IsBusy(Context->clientHandle))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt busy, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_BUSY;
    }

    if (pubex == 0)
    {
        Context->isPub = 1;
        APP_MqttQiSetString(&Context->pubTopic, topic); // MQTTPublishAsync成功后, event回调中释放
        Context->pubMessage = osMalloc(sizeof(MQTT_Message) + payloadlen); // MQTTPublishAsync成功后, event回调中释放
        Context->pubMessage->qos = qos;
        Context->pubMessage->retained = retain;
        Context->pubMessage->dup = dup;
        Context->pubMessage->id = id;
    }
    Context->pubMessage->payloadlen = payloadlen;
    if (0 == payloadlen)
    {
        Context->pubMessage->payload = NULL;
    }
    else
    {
        Context->pubMessage->payload = Context->pubMessage + 1;
        osMemcpy(Context->pubMessage->payload, payload, payloadlen);
    }

    rc = MQTT_Publish(Context->clientHandle, Context->pubTopic, Context->pubMessage);
    if (MQTT_OK == rc)
    {
        return MQTT_OK;
    }

    APP_MqttQiPublishFree(clientIdx);

    return rc;
}


void APP_MqttQiQueryCacheMessageCount(uint8_t clientIdx, uint32_t *storeMsgCount, uint32_t *totalMsgLen)
{
    MQTTClient *c = (MQTTClient *)g_appMqttQiContext[clientIdx].clientHandle;
    int i;
    struct osSlistNode *pNode = osSlistFirst(&c->cachedMessageList);
    struct osSlistNode *pNextNode = NULL;

    *storeMsgCount = osSlistLen(&c->cachedMessageList);
    *totalMsgLen = 0;
    for (i = 0; i < *storeMsgCount; i++)
    {
        if (NULL != pNode)
        {
            MQTT_CachedMessage *cacheMsg = (MQTT_CachedMessage *)pNode;
            *totalMsgLen += cacheMsg->data.topicName->lenstring.len;
            *totalMsgLen += cacheMsg->data.message->payloadlen;
            pNextNode = osSlistNext(pNode);
            pNode = pNextNode;
        }
        else
        {
            break;
        }
    }
}

int32_t APP_MqttQiDisconnect(uint8_t clientIdx, uint8_t closeMode)
{
    int rc;
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (!Context->isInit || !APP_MqttQiIsConnected(clientIdx))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt Qi unconn, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_UNCONN;
    }

    if (MQTT_IsBusy(Context->clientHandle))
    {
        APP_MQTT_QI_PRINTF_ERROR("Mqtt busy, clientIdx:%u!\r\n", (uint32_t)clientIdx);
        return MQTT_ERR_BUSY;
    }

    Context->closeMode = closeMode;

    rc = MQTT_Disconnect(Context->clientHandle);

    return rc;
}

int32_t APP_MqttQiClose(uint8_t clientIdx)
{
    APP_MqttQiContext *Context = &g_appMqttQiContext[clientIdx];

    if (Context->isOpen)
    {
        Network *pNetwork = (Network *)(((MQTTClient *)Context->clientHandle)->ipstack);
        if (NULL == pNetwork)
        {
            return 0;
        }

        if (Context->sslEnable)
        {
            if ((uintptr_t)(-1) != pNetwork->handle)
            {
                AT_SslContextEx *sslCtxEx = (AT_SslContextEx *)pNetwork->handle;
                AT_SslMbedtlsDestroy(sslCtxEx);
                pNetwork->handle = (uintptr_t)(-1);
                APP_MQTT_QI_PRINTF_INFO("Mqtt Qi close ok, clientIdx:%u!\r\n", (uint32_t)clientIdx);
            }
        }
        else
        {
            MQTTNetworkDisconnect(pNetwork);
        }
        Context->isOpen = OS_FALSE;
    }

    return 0;
}

void APP_MqttQiMessageHandler(MQTT_Handle handle, MQTT_MessageData *data)
{
    uint32_t i = 0;
    APP_MqttQiContext *mqttContext;
    char *topic = NULL;

    OS_ASSERT(NULL != handle && NULL != data);
    mqttContext = ((APP_MqttQiClient *)osContainerOf(handle, APP_MqttQiClient, client))->mqttContext;
    topic = (char *)osMalloc(data->topicName->lenstring.len + 1);
    if (topic != NULL)
    {
        osStrncpy(topic, data->topicName->lenstring.data, data->topicName->lenstring.len);
        topic[data->topicName->lenstring.len] = '\0';
    }

    if (APP_MQTT_QI_RECV_CACHE == g_appMqttQiContext[mqttContext->clientIdx].msgRecvMode)
    {
        char urcBuf[30] = {0};

        for (i = 0; i < APP_MQTT_CACHE_RECV_MSG_MAX; i++)
        {
            APP_MqttQiCacheMessage *pCacheMessages = &g_appMqttQiContext[mqttContext->clientIdx].cacheMessages[i];
            if (0 == pCacheMessages->storeStatus)
            {
                OS_ASSERT((pCacheMessages->topic == NULL) && (pCacheMessages->payload == NULL));

                pCacheMessages->msgid = data->message->id;
                pCacheMessages->topic = topic;
                pCacheMessages->payloadlen = data->message->payloadlen;
                pCacheMessages->payload = osMalloc(data->message->payloadlen);
                osMemcpy(pCacheMessages->payload, data->message->payload, data->message->payloadlen);
                pCacheMessages->storeStatus = 1;

                break;
            }
        }

        if (i < APP_MQTT_CACHE_RECV_MSG_MAX)
        {
            if (APP_MQTT_QI_RECV_LEN_ENABLE == g_appMqttQiContext[mqttContext->clientIdx].msgLenEnable)
            {
                osSnprintf(urcBuf, sizeof(urcBuf), "\r\n%s: %u,%u,%u\r\n", AT_MQTT_RECV, mqttContext->clientIdx, i, data->message->payloadlen);
                AT_SendUnsolicited(g_appMqttQiContext[mqttContext->clientIdx].channelId, (char *)urcBuf, strlen((char *)urcBuf));
            }
            else
            {
                osSnprintf(urcBuf, sizeof(urcBuf), "\r\n%s: %u,%u\r\n", AT_MQTT_RECV, mqttContext->clientIdx, i);
                AT_SendUnsolicited(g_appMqttQiContext[mqttContext->clientIdx].channelId, (char *)urcBuf, strlen((char *)urcBuf));
            }
        }
        else
        {
            // IMTDROP
            osSnprintf(urcBuf, sizeof(urcBuf), "\r\n%s: %u,%u\r\n", AT_MQTT_DROP, mqttContext->clientIdx, data->message->payloadlen);
            AT_SendUnsolicited(g_appMqttQiContext[mqttContext->clientIdx].channelId, (char *)urcBuf, strlen((char *)urcBuf));
        }
    }
    else
    {
        MqttQiPubUrcInfo publishUrcInfo;
        publishUrcInfo.clientIdx = mqttContext->clientIdx;
        publishUrcInfo.msgid = data->message->id;
        publishUrcInfo.topic = (uint8_t *)topic;
        publishUrcInfo.payload_len = data->message->payloadlen;
        publishUrcInfo.payload = data->message->payload;

        mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_PUBLISH, g_appMqttQiContext[publishUrcInfo.clientIdx].channelId, &publishUrcInfo);

        // 非cache模式下,直接释放内存
        if (topic)
        {
            osFree(topic);
            topic = NULL;
        }
    }

    APP_MQTT_QI_PRINTF_INFO("Message arrived on topic %s: %s\r\n",
               (char *)data->topicName->lenstring.data,
               (char *)data->message->payload);
}

void APP_MqttQiEventReport(MQTT_Handle handle, void *userData, MQTTEventType eventType)
{
    APP_MqttQiContext *mqttContext;
    uint8_t clientIdx;
    int32_t i;

    OS_ASSERT(NULL != handle);
    mqttContext = ((APP_MqttQiClient *)osContainerOf(handle, APP_MqttQiClient, client))->mqttContext;
    clientIdx = mqttContext->clientIdx;
    APP_MQTT_QI_PRINTF_INFO("Mqtt event callback:%d, clientIdx:%u!\r\n", eventType, (uint32_t)clientIdx);

    switch (eventType)
    {
    case MQTT_EVENT_CONN_TIMEOUT:
    case MQTT_EVENT_RECONN_TIMEOUT:
        {
            char urcBuf[30] = {0};
            APP_MQTT_QI_PRINTF_INFO("Mqtt conn timeout, clientIdx:%u!\r\n", (uint32_t)clientIdx);
            APP_MqttQiFreeConnResource(clientIdx);
            APP_MqttQiFreeBufResource(clientIdx);
            // APP_MqttQiReportTimeout(clientIdx);

            osSnprintf((char *)urcBuf,sizeof(urcBuf), "\r\n%s: %d,%d,%d\r\n", AT_MQTT_CONN, clientIdx,
                MQTT_QI_PKG_TRAN_FAIL, MQTT_QI_CONN_REFUSE_SERVER_UNUSED);
            AT_SendUnsolicited(mqttContext->channelId, (char *)urcBuf, strlen((char *)urcBuf));
        }
        break;

    case MQTT_EVENT_PUB_TIMEOUT:
    case MQTT_EVENT_PUBACK_TIMEOUT:
    case MQTT_EVENT_PUBREC_TIMEOUT:
    case MQTT_EVENT_PUBCOMP_TIMEOUT:
        {
            char rsp[40];

            uint8_t isPub = APP_MqttQiGetCurPublishCommType(clientIdx);
            uint16_t msgId = APP_MqttQiGetCurPublishMsgId(clientIdx);

            APP_MQTT_QI_PRINTF_INFO("Mqtt pub timeout, clientIdx:%u!\r\n", (uint32_t)clientIdx);
            APP_MqttQiPublishFree(clientIdx);
            // APP_MqttQiReportTimeout(clientIdx);

            osSnprintf(rsp, sizeof(rsp), "\r\n%s: %d,%d,%d\r\n", (isPub ? AT_MQTT_PUB : AT_MQTT_PUBEX), clientIdx, msgId, MQTT_QI_PKG_TRAN_FAIL);
            AT_SendUnsolicited(mqttContext->channelId, rsp, strlen(rsp));
        }
        break;

    case MQTT_EVENT_PUBACK_TIMEOUT_RETRANS:
    case MQTT_EVENT_PUBREC_TIMEOUT_RETRANS:
    case MQTT_EVENT_PUBCOMP_TIMEOUT_RETRANS:
        {
            APP_MQTT_QI_PRINTF_INFO("Mqtt pub eventType:%d timeout, retrans, clientIdx:%u!\r\n", (int32_t)eventType, (uint32_t)clientIdx);
            if (APP_MQTT_QI_TIMEOUT_NOTICE_ENABLE == mqttContext->timeOutNotice)
            {
                char rsp[40];

                uint8_t isPub = APP_MqttQiGetCurPublishCommType(clientIdx);
                uint16_t msgId = APP_MqttQiGetCurPublishMsgId(clientIdx);

                MQTTClient *client = (MQTTClient *)handle;
                osSnprintf(rsp, sizeof(rsp), "\r\n%s: %d,%d,%d,%d\r\n", (isPub ? AT_MQTT_PUB : AT_MQTT_PUBEX), clientIdx, msgId, MQTT_QI_PKG_RETRNS, client->currentRetryTimes);
                AT_SendUnsolicited(mqttContext->channelId, rsp, strlen(rsp));
            }
        }
        break;

    case MQTT_EVENT_CONN_ERR:
        {
            MqttQiConnUrcInfo connUrcInfo;
            int32_t *retCode = (int32_t *)userData;

            connUrcInfo.clientIdx = clientIdx;

            if (retCode != NULL)
            {
                switch (*retCode)
                {
                    case MQTT_QI_CONN_REFUSE_PROTO_ERR:
                    case MQTT_QI_CONN_REFUSE_ID_ERR:
                    case MQTT_QI_CONN_REFUSE_SERVER_UNUSED:
                    case MQTT_QI_CONN_REFUSE_USER_PASSWORD_ERR:
                    case MQTT_QI_CONN_REFUSE_UNAUTHORIZED:
                        {
                            connUrcInfo.retcode = (MqttQiRetCode)(*retCode);
                        }
                        break;

                    default:
                        {
                            connUrcInfo.retcode = MQTT_QI_CONN_REFUSE_UNKNOWN_ERR;
                        }
                        break;
                }
            }
            else
            {
                connUrcInfo.retcode = MQTT_QI_CONN_REFUSE_UNKNOWN_ERR;
            }
            connUrcInfo.result = MQTT_QI_PKG_TRAN_FAIL;

            APP_MQTT_QI_PRINTF_INFO("Mqtt conn err, clientIdx:%u!\r\n", (uint32_t)clientIdx);

            APP_MqttQiFreeConnResource(clientIdx);
            APP_MqttQiFreeBufResource(clientIdx);
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_CONN, mqttContext->channelId, &connUrcInfo);
        }
        break;

    case MQTT_EVENT_CONNACK:
        {
            MqttQiConnUrcInfo connUrcInfo;
            connUrcInfo.clientIdx = clientIdx;
            connUrcInfo.retcode = MQTT_QI_CONN_SUCCESS;
            connUrcInfo.result = MQTT_QI_PKG_TRAN_SUCCESS;
            mqttContext->isInit = OS_TRUE;

            APP_MQTT_QI_PRINTF_INFO("Mqtt conn ack, clientIdx:%u!\r\n", (uint32_t)clientIdx);
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_CONN, mqttContext->channelId, &connUrcInfo);
        }
        break;

    case MQTT_EVENT_PUB_ERR:
        {
            char rsp[40] = {0};

            uint8_t isPub = APP_MqttQiGetCurPublishCommType(clientIdx);
            uint16_t msgId = APP_MqttQiGetCurPublishMsgId(clientIdx);

            APP_MqttQiPublishFree(clientIdx);
            APP_MQTT_QI_PRINTF_INFO("Mqtt pub error, clientIdx:%u\r\n", (uint32_t)clientIdx);
            osSnprintf(rsp, sizeof(rsp), "\r\n%s: %d,%d,%d\r\n", (isPub ? AT_MQTT_PUB : AT_MQTT_PUBEX), clientIdx, msgId, MQTT_QI_PKG_TRAN_FAIL);
            AT_SendUnsolicited(mqttContext->channelId, (char *)rsp, strlen((char *)rsp));
        }
        break;

    case MQTT_EVENT_PUB_QOS0:
        {
            char rsp[40] = {0};

            uint8_t isPub = APP_MqttQiGetCurPublishCommType(clientIdx);
            uint16_t msgId = APP_MqttQiGetCurPublishMsgId(clientIdx);

            APP_MqttQiPublishFree(clientIdx);
            APP_MQTT_QI_PRINTF_INFO("Mqtt pub Qos0 free, clientIdx:%u\r\n", (uint32_t)clientIdx);
            osSnprintf(rsp, sizeof(rsp), "\r\n%s: %d,%d,%d\r\n", (isPub ? AT_MQTT_PUB : AT_MQTT_PUBEX), clientIdx, msgId, MQTT_QI_PKG_TRAN_SUCCESS);
            AT_SendUnsolicited(mqttContext->channelId, (char *)rsp, strlen((char *)rsp));
        }
        break;

    case MQTT_EVENT_PUBACK:
        {
            MqttQiPubUrcInfo pubAckUrcInfo;
            MQTT_MessageData *data = (MQTT_MessageData *)userData;
            pubAckUrcInfo.isPub = APP_MqttQiGetCurPublishCommType(clientIdx);
            APP_MqttQiPublishFree(clientIdx);
            OS_ASSERT(NULL != data);
            pubAckUrcInfo.clientIdx = clientIdx;
            pubAckUrcInfo.msgid = data->message->id;
            pubAckUrcInfo.dup = data->message->dup;
            APP_MQTT_QI_PRINTF_INFO("Mqtt clientIdx:%u, recv puback!\r\n", clientIdx);
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_PUBACK, mqttContext->channelId, &pubAckUrcInfo);
        }
        break;

    case MQTT_EVENT_PUBREC:
        {
            // MqttQiPubUrcInfo pubRecUrcInfo;
            // MQTT_MessageData *data = (MQTT_MessageData *)userData;
            // pubRecUrcInfo.clientIdx = clientIdx;
            // pubRecUrcInfo.dup = data->message->dup;
            // pubRecUrcInfo.msgid = data->message->id;
            APP_MQTT_QI_PRINTF_INFO("Mqtt clientIdx:%u, recv pubrec!\r\n", clientIdx);
            // mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_PUBREC, mqttContext->channelId, &pubRecUrcInfo); 根据AT手册,REC无需上报
        }
        break;

    case MQTT_EVENT_PUBCOMP:
        {
            MqttQiPubUrcInfo pubCompUrcInfo;
            pubCompUrcInfo.isPub = APP_MqttQiGetCurPublishCommType(clientIdx);
            MQTT_MessageData *data = (MQTT_MessageData *)userData;
            APP_MqttQiPublishFree(clientIdx);
            OS_ASSERT(NULL != data);
            pubCompUrcInfo.clientIdx = clientIdx;
            pubCompUrcInfo.dup = data->message->dup;
            pubCompUrcInfo.msgid = data->message->id;
            APP_MQTT_QI_PRINTF_INFO("Mqtt clientIdx:%u, recv pubcomp!\r\n", clientIdx);
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_PUBCOMP, mqttContext->channelId, &pubCompUrcInfo);
        }
        break;

    case MQTT_EVENT_SUB_TIMEOUT:
    case MQTT_EVENT_SUB_ERR:
        {
            uint16_t msgid = 0;
            if (NULL != userData)
            {
                msgid = *((uint16_t *)userData);
            }
            char urcBuf[30] = {0};
            int32_t i;
            for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
            {
                if (NULL != mqttContext->addTopics[i])
                {
                    osFree(mqttContext->addTopics[i]);
                    mqttContext->addTopics[i] = NULL;
                }
            }

            APP_MQTT_QI_PRINTF_INFO("Mqtt sub error/timeout, clientIdx:%u\r\n", (uint32_t)clientIdx);
            osSnprintf((char *)urcBuf, sizeof(urcBuf),"\r\n%s: %d,%d,%d\r\n", AT_MQTT_SUB, clientIdx, msgid, MQTT_QI_PKG_TRAN_FAIL);
            AT_SendUnsolicited(mqttContext->channelId, (char *)urcBuf, strlen((char *)urcBuf));
        }
        break;

    case MQTT_EVENT_SUBACK:
        {
            MqttQiSubackUrcInfo subAckUrcInfo;
            uint16_t msgid = 0;
            if (NULL != userData)
            {
                msgid = *((uint16_t *)userData);
            }

            int32_t i, j;
            subAckUrcInfo.code_number = 0;

            for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
            {
                bool_t find = OS_FALSE;
                if (NULL == mqttContext->addTopics[i])
                {
                    break;
                }

                for (j = 0; j < MAX_MESSAGE_HANDLERS; j++)
                {
                    if (mqttContext->topics[j] != NULL
                        && strcmp(mqttContext->addTopics[i], mqttContext->topics[j]) == 0)
                    {
                        find = OS_TRUE;
                        break;
                    }
                }

                if (find)
                {
                    osFree(mqttContext->addTopics[i]);
                    mqttContext->addTopics[i] = NULL;
                    mqttContext->QoSs[j] = mqttContext->addQoSs[i];
                }
                else
                {
                    for (j = 0; j < MAX_MESSAGE_HANDLERS; j++)
                    {
                        if (NULL == mqttContext->topics[j])
                        {
                            mqttContext->topics[j] = mqttContext->addTopics[i];
                            mqttContext->QoSs[j] = mqttContext->addQoSs[i];
                            mqttContext->addTopics[i] = NULL;
                            break;
                        }
                    }
                    OS_ASSERT(j < MAX_MESSAGE_HANDLERS);
                }

                subAckUrcInfo.code_number++;
                switch (mqttContext->addQoSs[i])
                {
                case MQTT_QOS0:
                    subAckUrcInfo.code[i] = MQTT_QI_SUB_SUCCEED_QOS0;
                    break;
                case MQTT_QOS1:
                    subAckUrcInfo.code[i] = MQTT_QI_SUB_SUCCEED_QOS1;
                    break;
                case MQTT_QOS2:
                    subAckUrcInfo.code[i] = MQTT_QI_SUB_SUCCEED_QOS2;
                    break;
                default:
                    OS_ASSERT(0);
                    break;
                }
            }

            subAckUrcInfo.clientIdx = clientIdx;
            subAckUrcInfo.msgid = msgid;
            APP_MQTT_QI_PRINTF_INFO("Mqtt suback, clientIdx:%u %u\r\n", (uint32_t)clientIdx, subAckUrcInfo.msgid);
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_SUBACK, mqttContext->channelId, &subAckUrcInfo);
        }
        break;

    case MQTT_EVENT_UNSUB_TIMEOUT:
    case MQTT_EVENT_UNSUB_ERR:
        {
            uint16_t msgid = 0;
            if (NULL != userData)
            {
                msgid = *((uint16_t *)userData);
            }
            char urcBuf[30] = {0};
            for (i = 0; i < MAX_MESSAGE_HANDLERS; i ++)
            {
                if (mqttContext->unsubTopics[i])
                {
                    osFree(mqttContext->unsubTopics[i]);
                    mqttContext->unsubTopics[i] = NULL;
                }
            }

            APP_MQTT_QI_PRINTF_INFO("Mqtt unsub error/timeout, clientIdx:%u\r\n", (uint32_t)clientIdx);
            osSnprintf((char *)urcBuf, sizeof(urcBuf),"\r\n%s: %d,%d,%d\r\n", AT_MQTT_UNS, clientIdx, msgid, MQTT_QI_PKG_TRAN_FAIL);
            AT_SendUnsolicited(mqttContext->channelId, (char *)urcBuf, strlen((char *)urcBuf));
        }
        break;

    case MQTT_EVENT_UNSUBACK:
        {
            MqttQiUnsubackUrcInfo unSubAckUrcInfo;
            uint16_t msgid = 0;
            if (NULL != userData)
            {
                msgid = *((uint16_t *)userData);
            }

            APP_MQTT_QI_PRINTF_INFO("Mqtt unsub ack, clientIdx:%u\r\n", (uint32_t)clientIdx);

            for (i = 0; i < MAX_MESSAGE_HANDLERS; i ++)
            {
                if (mqttContext->unsubTopics[i])
                {
                    osFree(mqttContext->unsubTopics[i]);
                    mqttContext->unsubTopics[i] = NULL;
                }
            }

            unSubAckUrcInfo.clientIdx = clientIdx;
            unSubAckUrcInfo.msgid = msgid;
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_UNSUBACK, mqttContext->channelId, &unSubAckUrcInfo);
        }
        break;

    case MQTT_EVENT_PINGRESP:
        {
            MQTTClient *client = (MQTTClient *)handle;
            MqttQiPingrespUrcInfo pingRespUrcInfo;

            APP_MQTT_QI_PRINTF_INFO("Mqtt ping resp:%d, clientIdx:%u\r\n", (int)client->ping_outstanding, (uint32_t)clientIdx);
            pingRespUrcInfo.clientIdx = clientIdx;
            pingRespUrcInfo.ping_ret = (client->ping_outstanding == 0) ? MQTT_QI_PING_RET_SUCCEED : MQTT_QI_PING_RET_TIMEOUT;
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_PINGRESP, mqttContext->channelId, &pingRespUrcInfo);
        }
        break;

    case MQTT_EVENT_DISC_OK:
        {
            MqttQiConnUrcInfo connUrcInfo;
            APP_MqttQiFreeConnResource(clientIdx);
            APP_MqttQiFreeBufResource(clientIdx);

            APP_MQTT_QI_PRINTF_INFO("Mqtt disc ok, clientIdx:%u\r\n", (uint32_t)clientIdx);

            mqttContext->isInit = OS_FALSE;
            connUrcInfo.result = 0;
            connUrcInfo.retcode = 0;
            connUrcInfo.clientIdx = clientIdx;
            connUrcInfo.closeMode = mqttContext->closeMode;
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_DISK, mqttContext->channelId, &connUrcInfo);
        }
        break;

    case MQTT_EVENT_SESSION_CLOSED:
        {
            int rc = MQTT_Disconnect(mqttContext->clientHandle);
            APP_MQTT_QI_PRINTF_INFO("Mqtt recv session closed, clientIdx:%u\r\n", (uint32_t)clientIdx);
            if (MQTT_OK != rc)
            {
                APP_MQTT_QI_PRINTF_ERROR("Mqtt recv session closed, but disc async error, clientIdx:%u\r\n", (uint32_t)clientIdx);
            }
        }
        break;

    case MQTT_EVENT_NET_CLOSE:
        {
            MqttQiStatUrcInfo statRespUrcInfo;

            MQTT_DeinitClient(mqttContext->clientHandle);
            APP_MqttQiFreeConnResource(clientIdx);
            APP_MqttQiFreeBufResource(clientIdx);

            mqttContext->isInit = OS_FALSE;

            APP_MQTT_QI_PRINTF_INFO("Mqtt session closed, clientIdx:%u\r\n", (uint32_t)clientIdx);
            statRespUrcInfo.clientIdx = clientIdx;
            statRespUrcInfo.stat_ret = MQTT_QI_STAT_CLOSE_BY_SERVER;
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_STAT, mqttContext->channelId, &statRespUrcInfo);
        }
        break;

#if 0 // 不使用MQTT客户端带的缓存功能，本模块自己实现缓存功能
    case MQTT_EVENT_DROP:
        {
            MqttQiDropUrcInfo dropUrcInfo;
            MQTT_MessageData *data = (MQTT_MessageData *)userData;

            OS_ASSERT(NULL != data && NULL != data->message && NULL != data->topicName);
            dropUrcInfo.clientIdx = clientIdx;
            dropUrcInfo.dropped_length = data->message->payloadlen + data->topicName->lenstring.len + 1; // +1是填充'\0'

            APP_MQTT_QI_PRINTF_INFO("Mqtt drop cached message, dropLen:%u, clientIdx:%u\r\n", dropUrcInfo.dropped_length, (uint32_t)clientIdx);
        }
        break;

    case MQTT_EVENT_NEW_MSG_RECV:
        {
            MqttQiPubUrcInfo pubNmiUrcInfo;
            MQTT_MessageData *data = (MQTT_MessageData *)userData;

            OS_ASSERT(NULL != data && NULL != data->message && NULL != data->topicName);
            pubNmiUrcInfo.clientIdx = clientIdx;
            pubNmiUrcInfo.msgid = data->message->id;
            pubNmiUrcInfo.payload = data->message->payload;
            pubNmiUrcInfo.payload_len = data->message->payloadlen;
            //pubNmiUrcInfo.data_len = data->message->payloadlen + data->topicName->lenstring.len + 1; // +1是填充'\0'

            //APP_MQTT_PRINTF_INFO("Mqtt pubNmi, dataLen:%u, connectId:%u\r\n", pubNmiUrcInfo.data_len, (uint32_t)connectId);
            mqttContext->AT_MqttUrcUnsolicited(MQTT_QI_PUBNMI, mqttContext->channelId, &pubNmiUrcInfo);
        }
        break;
#endif

    default:
        break;
    }
}

static bool APP_MqttQiClientIsValid(MQTT_Handle handle)
{
    bool ret = false;
    int i;

    if(handle == NULL)
        return false;

    for(i = 0; i < APP_MQTT_QI_CLIENT_CNT; i++)
    {
        if(handle == (MQTT_Handle)&g_appMqttQiClient[i].client)
        {
            ret = true;
            break;
        }
    }

    return ret;
}

static void APP_MqttQiProtocolInit(void)
{
    MQTT_CustomerClientCheckSet(APP_MqttQiClientIsValid);
    MQTT_WorkMutexInit();
}

int APP_MqttQiInit(void)
{
    int i;

    //  MQTT 协议初始化
    APP_MqttQiProtocolInit();

    osMemset(g_appMqttQiContext, 0x0, APP_MQTT_QI_CLIENT_CNT * sizeof(APP_MqttQiContext));
    for (i = 0; i < APP_MQTT_QI_CLIENT_CNT; i++)
    {
        MQTT_ConnectOption connectData = MQTT_CONNECTDATA_INITIALIZER;
        g_appMqttQiContext[i].connectData = connectData;
        g_appMqttQiContext[i].connectData.MQTTVersion = APP_MQTT_QI_V3_1;
        g_appMqttQiContext[i].clientHandle = (MQTT_Handle)&g_appMqttQiClient[i].client;
        g_appMqttQiClient[i].mqttContext = &g_appMqttQiContext[i];
        MQTT_InitClientDefaultParam(g_appMqttQiContext[i].clientHandle);
        ((MQTTClient *)g_appMqttQiContext[i].clientHandle)->reconnEnable = OS_FALSE;
        ((MQTTClient *)g_appMqttQiContext[i].clientHandle)->pingInterval = APP_MQTT_IMTPING_INTERVAL_DEFAULT; // 移远AT默认值与模组AT不一样
        APP_MqttQiContextClientParamInit(&g_appMqttQiClient[i].mqttContext->clientParam, g_appMqttQiContext[i].clientHandle);
        g_appMqttQiContext[i].timeOutNotice = APP_MQTT_QI_TIMEOUT_NOTICE_DISABLE;
        g_appMqttQiContext[i].isInit = OS_FALSE;
        g_appMqttQiContext[i].isOpen = OS_FALSE;
        g_appMqttQiContext[i].viewMode = APP_MQTT_VIEW_MODE_DEFAULT;
        g_appMqttQiContext[i].editMode = APP_MQTT_EDIT_MODE_EXIT_DEFAULT;
        g_appMqttQiContext[i].editTime = APP_MQTT_EDIT_TIME_DEFAULT;
        g_appMqttQiContext[i].clientIdx = i;
        g_appMqttQiContext[i].pdpCid = APP_MQTT_QI_PDP_CID_MIN;
        g_appMqttQiContext[i].port = APP_MQTT_QI_PORT_MIN;
        MutexInit(&g_appMqttQiContext[i].mqttQiMutex);
    }

    return MQTT_OK;
}

