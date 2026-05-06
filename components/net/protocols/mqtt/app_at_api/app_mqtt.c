#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTPort.h"
#include "MQTTClient.h"
#include "net_pub.h"
#include "app_mqtt.h"


#define APP_MQTT_PRINTF_INFO        MQTT_PRINT_INFO
#define APP_MQTT_PRINTF_ERROR       MQTT_PRINT_ERROR

typedef struct
{
    MQTTClient           client;
    APP_MqttContext     *mqttContext;
} APP_MqttClient;

extern int8_t AT_MqttSendUnsolicited(MqttUrcType type, uint8_t channelId, void *urcInfo);
APP_MqttContext g_appMqttContext[APP_MQTT_CONNECT_CNT];
APP_MqttClient  g_appMqttClient[APP_MQTT_CONNECT_CNT];

void APP_MqttEventReport(MQTT_Handle handle, void *userData, MQTTEventType eventType);
void APP_MqttMessageHandler(MQTT_Handle handle, MQTT_MessageData *data);

void APP_MqttFreeConnResource(uint8_t connectId)
{
    int32_t i;

    if (NULL != g_appMqttContext[connectId].connectData.clientID.cstring)
    {
        osFree(g_appMqttContext[connectId].connectData.clientID.cstring);
        g_appMqttContext[connectId].connectData.clientID.cstring = NULL;
    }

    if (NULL != g_appMqttContext[connectId].connectData.username.cstring)
    {
        osFree(g_appMqttContext[connectId].connectData.username.cstring);
        g_appMqttContext[connectId].connectData.username.cstring = NULL;
    }

    if (NULL != g_appMqttContext[connectId].connectData.password.cstring)
    {
        osFree(g_appMqttContext[connectId].connectData.password.cstring);
        g_appMqttContext[connectId].connectData.password.cstring = NULL;
    }

    if (NULL != g_appMqttContext[connectId].pHost)
    {
        osFree(g_appMqttContext[connectId].pHost);
        g_appMqttContext[connectId].pHost = NULL;
    }

    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
    {
        if (NULL != g_appMqttContext[connectId].topics[i])
        {
            osFree(g_appMqttContext[connectId].topics[i]);
            g_appMqttContext[connectId].topics[i] = NULL;
        }

        if (NULL != g_appMqttContext[connectId].addTopics[i])
        {
            osFree(g_appMqttContext[connectId].addTopics[i]);
            g_appMqttContext[connectId].addTopics[i] = NULL;
        }

        if (NULL != g_appMqttContext[connectId].unsubTopics[i])
        {
            osFree(g_appMqttContext[connectId].unsubTopics[i]);
            g_appMqttContext[connectId].unsubTopics[i] = NULL;
        }
    }

    if (g_appMqttContext[connectId].pubMessage)
    {
        osFree(g_appMqttContext[connectId].pubMessage);
        g_appMqttContext[connectId].pubMessage = NULL;
    }

    if (g_appMqttContext[connectId].pubTopic)
    {
        osFree(g_appMqttContext[connectId].pubTopic);
        g_appMqttContext[connectId].pubTopic = NULL;
    }
}

void APP_MqttFreeWillResource(uint8_t connectId)
{
    if (NULL != g_appMqttContext[connectId].connectData.will.topicName.cstring)
    {
        osFree(g_appMqttContext[connectId].connectData.will.topicName.cstring);
        g_appMqttContext[connectId].connectData.will.topicName.cstring = NULL;
    }
    if (NULL != g_appMqttContext[connectId].connectData.will.message.cstring)
    {
        osFree(g_appMqttContext[connectId].connectData.will.message.cstring);
        g_appMqttContext[connectId].connectData.will.message.cstring = NULL;
    }
}

void APP_MqttFreeBufResource(uint8_t connectId)
{
    if (g_appMqttContext[connectId].readbuf)
    {
        osFree(g_appMqttContext[connectId].readbuf);
        g_appMqttContext[connectId].readbuf = NULL;
    }

    if (g_appMqttContext[connectId].sendbuf)
    {
        osFree(g_appMqttContext[connectId].sendbuf);
        g_appMqttContext[connectId].sendbuf = NULL;
    }
}

void APP_MqttSetString(char **destbuf, char *srcBuf)
{
    size_t strLen;

    OS_ASSERT((NULL != destbuf) && (NULL == *destbuf) && (NULL != srcBuf));

    strLen = osStrlen(srcBuf);
    *destbuf = (char *)osMalloc(strLen + 1);
    OS_ASSERT(NULL != *destbuf);
    osStrncpy(*destbuf, srcBuf, strLen);

    (*destbuf)[strLen]= '\0';
}

bool_t APP_MqttIsConnected(uint8_t connectId)
{
    return MQTT_IsConnected(g_appMqttContext[connectId].clientHandle);
}

#define APP_MQTT_CA_CRT_RSA_SHA256_PEM                                     \
    "-----BEGIN CERTIFICATE-----\r\n"                                      \
    "MIIDUTCCAjmgAwIBAgIJAPPYCjTmxdt/MA0GCSqGSIb3DQEBCwUAMD8xCzAJBgNV\r\n" \
    "BAYTAkNOMREwDwYDVQQIDAhoYW5nemhvdTEMMAoGA1UECgwDRU1RMQ8wDQYDVQQD\r\n" \
    "DAZSb290Q0EwHhcNMjAwNTA4MDgwNjUyWhcNMzAwNTA2MDgwNjUyWjA/MQswCQYD\r\n" \
    "VQQGEwJDTjERMA8GA1UECAwIaGFuZ3pob3UxDDAKBgNVBAoMA0VNUTEPMA0GA1UE\r\n" \
    "AwwGUm9vdENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzcgVLex1\r\n" \
    "EZ9ON64EX8v+wcSjzOZpiEOsAOuSXOEN3wb8FKUxCdsGrsJYB7a5VM/Jot25Mod2\r\n" \
    "juS3OBMg6r85k2TWjdxUoUs+HiUB/pP/ARaaW6VntpAEokpij/przWMPgJnBF3Ur\r\n" \
    "MjtbLayH9hGmpQrI5c2vmHQ2reRZnSFbY+2b8SXZ+3lZZgz9+BaQYWdQWfaUWEHZ\r\n" \
    "uDaNiViVO0OT8DRjCuiDp3yYDj3iLWbTA/gDL6Tf5XuHuEwcOQUrd+h0hyIphO8D\r\n" \
    "tsrsHZ14j4AWYLk1CPA6pq1HIUvEl2rANx2lVUNv+nt64K/Mr3RnVQd9s8bK+TXQ\r\n" \
    "KGHd2Lv/PALYuwIDAQABo1AwTjAdBgNVHQ4EFgQUGBmW+iDzxctWAWxmhgdlE8Pj\r\n" \
    "EbQwHwYDVR0jBBgwFoAUGBmW+iDzxctWAWxmhgdlE8PjEbQwDAYDVR0TBAUwAwEB\r\n" \
    "/zANBgkqhkiG9w0BAQsFAAOCAQEAGbhRUjpIred4cFAFJ7bbYD9hKu/yzWPWkMRa\r\n" \
    "ErlCKHmuYsYk+5d16JQhJaFy6MGXfLgo3KV2itl0d+OWNH0U9ULXcglTxy6+njo5\r\n" \
    "CFqdUBPwN1jxhzo9yteDMKF4+AHIxbvCAJa17qcwUKR5MKNvv09C6pvQDJLzid7y\r\n" \
    "E2dkgSuggik3oa0427KvctFf8uhOV94RvEDyqvT5+pgNYZ2Yfga9pD/jjpoHEUlo\r\n" \
    "88IGU8/wJCx3Ds2yc8+oBg/ynxG8f/HmCC1ET6EHHoe2jlo8FpU/SgGtghS1YL30\r\n" \
    "IWxNsPrUP+XsZpBJy/mvOhE5QXo6Y35zDqqj8tI7AGmAWu22jg==\r\n"             \
    "-----END CERTIFICATE-----\r\n"
/* END FILE */

const char app_mqtt_mbedtls_cas_pem[] =
    APP_MQTT_CA_CRT_RSA_SHA256_PEM
    "";

static void APP_MqttContextClientParamInit(APP_MqttClientParam *mqttClientParam, MQTT_Handle handle)
{
    MQTTClient *client = (MQTTClient *)handle;
    mqttClientParam->pingInterval = client->pingInterval;
    mqttClientParam->retransInterval = client->retransInterval;
    mqttClientParam->retryTimes = client->retryTimes;
    mqttClientParam->reconnTimes = client->reconnTimes;
    mqttClientParam->reconnInterval = client->reconnInterval;
    mqttClientParam->reconnMode = client->reconnMode;
    mqttClientParam->cachedMode = client->cachedMode;
}

static void APP_MqttHandleParamUpdate(MQTT_Handle handle, APP_MqttClientParam *mqttQiClientParam)
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

int32_t APP_MqttConnect(uint8_t channelId, uint8_t connectId, char *host,
    uint16_t port, char *clientID, char *username, char *password)
{
    int32_t rc = 0;

    if (APP_MqttIsConnected(connectId))
    {
        APP_MQTT_PRINTF_ERROR("Mqtt conn exist, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_CONN_EXIST;
    }

    if (MQTT_IsBusy(g_appMqttContext[connectId].clientHandle))
    {
        APP_MQTT_PRINTF_ERROR("Mqtt busy, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_BUSY;
    }

    if (NULL == host || NULL == clientID)
    {
        APP_MQTT_PRINTF_ERROR("Mqtt param error, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_PARAMETER;
    }

    APP_MqttFreeConnResource(connectId);
    APP_MqttFreeBufResource(connectId);
    APP_MqttFreeWillResource(connectId);

    g_appMqttContext[connectId].pHost = NULL;
    g_appMqttContext[connectId].sendbuf = (uint8_t *)osMalloc(APP_MQTT_SEND_BUF_SIZE);
    g_appMqttContext[connectId].readbuf = (uint8_t *)osMalloc(APP_MQTT_RECV_BUF_SIZE);
    g_appMqttContext[connectId].channelId = channelId;
    g_appMqttContext[connectId].connectId = connectId;

    APP_MqttSetString(&g_appMqttContext[connectId].connectData.clientID.cstring, clientID);
    APP_MqttSetString(&g_appMqttContext[connectId].pHost, host);
    if (NULL != username && NULL != password)
    {
        APP_MqttSetString(&g_appMqttContext[connectId].connectData.username.cstring, username);
        APP_MqttSetString(&g_appMqttContext[connectId].connectData.password.cstring, password);
    }

    g_appMqttContext[connectId].connectData.host = g_appMqttContext[connectId].pHost;
    g_appMqttContext[connectId].connectData.port = port;
    g_appMqttContext[connectId].connectData.ca_crt = NULL;
#if defined(MQTT_USING_TLS)
    g_appMqttContext[connectId].connectData.ca_crt = app_mqtt_mbedtls_cas_pem;
#endif

    APP_MqttHandleParamUpdate(g_appMqttContext[connectId].clientHandle, &g_appMqttContext[connectId].clientParam);
    rc = MQTTAsync_InitClient(g_appMqttContext[connectId].clientHandle,
                   APP_MQTT_COMMAND_TIMEOUT,
                   g_appMqttContext[connectId].sendbuf,
                   APP_MQTT_SEND_BUF_SIZE,
                   g_appMqttContext[connectId].readbuf,
                   APP_MQTT_RECV_BUF_SIZE);
    if (0 != rc)
    {
        APP_MQTT_PRINTF_ERROR("Mqtt create failed, check param!connectId:%u!\r\n", (uint32_t)connectId);
        rc = MQTT_ERR_PARAMETER;
        goto APP_MQTT_CONN_ERR_FREE;
    }

    g_appMqttContext[connectId].msgHandler = APP_MqttMessageHandler;
    MQTT_SetEventCallback(g_appMqttContext[connectId].clientHandle, APP_MqttEventReport);

    rc = MQTT_Connect(g_appMqttContext[connectId].clientHandle, &g_appMqttContext[connectId].connectData);
    if (MQTT_OK == rc)
    {
        return MQTT_OK;
    }
    else
    {
        APP_MQTT_PRINTF_ERROR("Mqtt conn async error:%d, connectId:%u!\r\n", rc, (uint32_t)connectId);
        rc = MQTT_ERR;
        goto APP_MQTT_CONN_ERR_FREE;
    }

APP_MQTT_CONN_ERR_FREE:
    APP_MqttFreeConnResource(connectId);
    APP_MqttFreeBufResource(connectId);
    APP_MqttFreeWillResource(connectId);

    return rc;
}

uint32_t APP_MqttGetTopicCount(uint8_t connectId)
{
    int32_t i = 0;
    uint32_t count = 0;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
    {
        if (NULL != g_appMqttContext[connectId].topics[i])
        {
            count++;
        }
    }

    return count;
}

int32_t APP_MqttSubscribe(uint8_t connectId, uint32_t topicCnt, char* topics[], int32_t QoSs[])
{
    int32_t rc = 0;
    uint32_t i, j;
    uint32_t totalTopicCount;

    if (!APP_MqttIsConnected(connectId))
    {
        APP_MQTT_PRINTF_ERROR("Mqtt unconn, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_UNCONN;
    }

    if (MQTT_IsBusy(g_appMqttContext[connectId].clientHandle))
    {
        APP_MQTT_PRINTF_ERROR("Mqtt busy, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_BUSY;
    }

    totalTopicCount = APP_MqttGetTopicCount(connectId);
    for (i = 0; i < topicCnt; i++)
    {
        bool_t find = OS_FALSE;
        for (j = 0; j < MAX_MESSAGE_HANDLERS; j++)
        {
            if (g_appMqttContext[connectId].topics[j] != NULL
                && strcmp(g_appMqttContext[connectId].topics[j], topics[i]) == 0)
            {
                find = OS_TRUE;
                break;
            }
        }

        if (!find)
        {
            totalTopicCount++;
        }
        g_appMqttContext[connectId].addTopics[i] = NULL;
        APP_MqttSetString(&g_appMqttContext[connectId].addTopics[i], topics[i]);
        g_appMqttContext[connectId].addQoSs[i] = QoSs[i];
    }

    if (totalTopicCount > MAX_MESSAGE_HANDLERS)
    {
        APP_MQTT_PRINTF_ERROR("Mqtt total subscribe count:%u, exceed max subscribe count:%u, connectId:%u!\r\n",
            (uint32_t)totalTopicCount,
            MAX_MESSAGE_HANDLERS,
            (uint32_t)connectId);
        rc = MQTT_ERR_FULL;
        goto APP_MQTT_SUB_ERR_FREE;
    }

    rc = MQTT_Subscribe(g_appMqttContext[connectId].clientHandle, topicCnt,
        (const char**)g_appMqttContext[connectId].addTopics,
        (int *)g_appMqttContext[connectId].addQoSs,
        g_appMqttContext[connectId].msgHandler);
    if (MQTT_OK == rc)
    {
        return MQTT_OK;
    }

APP_MQTT_SUB_ERR_FREE:
    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
    {
        if (NULL != g_appMqttContext[connectId].addTopics[i])
        {
            osFree(g_appMqttContext[connectId].addTopics[i]);
            g_appMqttContext[connectId].addTopics[i] = NULL;
        }
    }

    return rc;
}

int32_t APP_MqttUnsubscribe(uint8_t connectId, uint32_t topicCnt, char *topics[])
{
    int32_t i;
    int32_t rc = 0;

    if (!APP_MqttIsConnected(connectId))
    {
        APP_MQTT_PRINTF_ERROR("Mqtt unconn, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_UNCONN;
    }

    if (MQTT_IsBusy(g_appMqttContext[connectId].clientHandle))
    {
        APP_MQTT_PRINTF_ERROR("Mqtt busy, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_BUSY;
    }

    for (i = 0; i < topicCnt; i ++)
    {
        APP_MqttSetString(&g_appMqttContext[connectId].unsubTopics[i], topics[i]);
    }

    rc = MQTT_Unsubscribe(g_appMqttContext[connectId].clientHandle, (int)topicCnt, (const char **)g_appMqttContext[connectId].unsubTopics);
    if (MQTT_OK == rc)
    {
        return MQTT_OK;
    }

    for (i = 0; i < topicCnt; i ++)
    {
        if (g_appMqttContext[connectId].unsubTopics[i])
        {
            osFree(g_appMqttContext[connectId].unsubTopics[i]);
            g_appMqttContext[connectId].unsubTopics[i] = NULL;
        }
    }

    return rc;
}

void APP_MqttPublishFree(uint8_t connectId)
{
    if (g_appMqttContext[connectId].pubTopic)
    {
        osFree(g_appMqttContext[connectId].pubTopic);
        g_appMqttContext[connectId].pubTopic = NULL;
    }

    if (g_appMqttContext[connectId].pubMessage)
    {
        osFree(g_appMqttContext[connectId].pubMessage);
        g_appMqttContext[connectId].pubMessage = NULL;
    }
}

int32_t APP_MqttPublish(uint8_t connectId, char *topic, int32_t qos,
        uint8_t retain, uint8_t dup, void *payload, size_t payloadlen)
{
    int32_t rc = 0;

    if (!APP_MqttIsConnected(connectId))
    {
        APP_MQTT_PRINTF_ERROR("Mqtt unconn, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_UNCONN;
    }

    if (MQTT_IsBusy(g_appMqttContext[connectId].clientHandle))
    {
        APP_MQTT_PRINTF_ERROR("Mqtt busy, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_BUSY;
    }

    APP_MqttSetString(&g_appMqttContext[connectId].pubTopic, topic); // MQTTPublishAsync成功后, event回调中释放
    g_appMqttContext[connectId].pubMessage = osMalloc(sizeof(MQTT_Message) + payloadlen); // MQTTPublishAsync成功后, event回调中释放
    g_appMqttContext[connectId].pubMessage->qos = qos;
    g_appMqttContext[connectId].pubMessage->retained = retain;
    g_appMqttContext[connectId].pubMessage->dup = dup;
    g_appMqttContext[connectId].pubMessage->id = 0;
    g_appMqttContext[connectId].pubMessage->payloadlen = payloadlen;
    g_appMqttContext[connectId].pubMessage->payload = g_appMqttContext[connectId].pubMessage + 1;
    osMemcpy(g_appMqttContext[connectId].pubMessage->payload, payload, payloadlen);

    rc = MQTT_Publish(g_appMqttContext[connectId].clientHandle, g_appMqttContext[connectId].pubTopic, g_appMqttContext[connectId].pubMessage);
    if (MQTT_OK == rc)
    {
        return MQTT_OK;
    }

    APP_MqttPublishFree(connectId);

    return rc;
}

void APP_MqttQueryCacheMessageCount(uint8_t connectId, uint32_t *storeMsgCount, uint32_t *totalMsgLen)
{
    MQTT_Handle clientHandle = g_appMqttContext[connectId].clientHandle;
    MQTT_QueryCacheMessage(clientHandle, storeMsgCount, totalMsgLen);
}

uint32_t APP_MqttReadCacheMessage(uint8_t connectId, uint32_t count, APP_MqttCacheMessage *mqttCacheMessages[])
{
    MQTT_Handle clientHandle = g_appMqttContext[connectId].clientHandle;
    int i;
    uint32_t readCount = 0;
    MQTT_MessageData* data[count];

    readCount = MQTT_ReadCacheMessage(clientHandle, count, data);
    for (i = 0; i < readCount; i++)
    {
        mqttCacheMessages[i]->mid = data[i]->message->id;
        mqttCacheMessages[i]->topic = data[i]->topicName->lenstring.data;
        mqttCacheMessages[i]->payloadlen = data[i]->message->payloadlen;
        mqttCacheMessages[i]->payload = (unsigned char *)data[i]->message->payload;
    }

    return readCount;
}

void APP_MqttFreeCacheMessage(uint8_t connectId, uint32_t count)
{
    MQTT_FreeCacheMessage(g_appMqttContext[connectId].clientHandle, count);
}

int32_t APP_MqttDisconnect(uint8_t connectId)
{
    int rc;
    if (!g_appMqttContext[connectId].isInit || !APP_MqttIsConnected(connectId))
    {
        APP_MQTT_PRINTF_ERROR("Mqtt unconn, connectId:%u!\r\n", (uint32_t)connectId);
        return MQTT_ERR_UNCONN;
    }

    rc = MQTT_Disconnect(g_appMqttContext[connectId].clientHandle);
    return rc;
}

void APP_MqttMessageHandler(MQTT_Handle handle, MQTT_MessageData *data)
{
    MqttPublishUrcInfo publishUrcInfo;
    APP_MqttContext *mqttContext;
    char *topic = NULL;

    OS_ASSERT(NULL != handle && NULL != data);
    mqttContext = ((APP_MqttClient *)osContainerOf(handle, APP_MqttClient, client))->mqttContext;
    publishUrcInfo.connect_id = mqttContext->connectId;
    publishUrcInfo.mid = data->message->id;
    topic = (char *)osMalloc(data->topicName->lenstring.len + 1);
    if (topic != NULL)
    {
        osStrncpy(topic, data->topicName->lenstring.data, data->topicName->lenstring.len);
        topic[data->topicName->lenstring.len] = '\0';
    }
    publishUrcInfo.topic = (uint8_t *)topic;
    publishUrcInfo.total_len = data->message->payloadlen;
    publishUrcInfo.payload_len = data->message->payloadlen;
    publishUrcInfo.payload = data->message->payload;

    AT_MqttSendUnsolicited(MQTT_PUBLISH, g_appMqttContext[0].channelId, &publishUrcInfo);
    if (topic)
    {
        osFree(topic);
        topic = NULL;
    }

    APP_MQTT_PRINTF_INFO("Message arrived on topic %s: %s\r\n",
               (char *)data->topicName->lenstring.data,
               (char *)data->message->payload);
}

void APP_MqttReportTimeout(uint8_t connectId)
{
    MqttTimeoutUrcInfo timeOutUrcInfo;
    timeOutUrcInfo.connect_id = connectId;
    timeOutUrcInfo.mid = ((MQTTClient *)AT_MQTT_CFG_PARAM_GET(connectId, clientHandle))->next_packetid;
    APP_MQTT_PRINTF_INFO("Mqtt wait req response timeout, processing event:%d, connectId:%u, mid:%u\r\n",
        ((MQTTClient *)AT_MQTT_CFG_PARAM_GET(connectId, clientHandle))->reqInfo.eventId,
        (uint32_t)connectId,
        (uint32_t)timeOutUrcInfo.mid);
    AT_MqttSendUnsolicited(MQTT_TIMEOUT, g_appMqttContext[connectId].channelId, &timeOutUrcInfo);
}

void APP_MqttEventReport(MQTT_Handle handle, void *userData, MQTTEventType eventType)
{
    APP_MqttContext *mqttContext;
    uint8_t connectId;
    int32_t i;

    OS_ASSERT(NULL != handle);
    mqttContext = ((APP_MqttClient *)osContainerOf(handle, APP_MqttClient, client))->mqttContext;
    connectId = mqttContext->connectId;
    APP_MQTT_PRINTF_INFO("Mqtt event callback:%d, connectId:%u!\r\n", eventType, (uint32_t)connectId);

    switch (eventType)
    {
    case MQTT_EVENT_CONN_TIMEOUT:
    case MQTT_EVENT_RECONN_TIMEOUT:
        {
            APP_MQTT_PRINTF_INFO("Mqtt conn timeout, connectId:%u!\r\n", (uint32_t)connectId);
            APP_MqttFreeConnResource(connectId);
            APP_MqttFreeBufResource(connectId);
            APP_MqttFreeWillResource(connectId);
            APP_MqttReportTimeout(connectId);
        }
        break;

    case MQTT_EVENT_SUB_TIMEOUT:
        {
            APP_MQTT_PRINTF_INFO("Mqtt sub timeout, connectId:%u!\r\n", (uint32_t)connectId);
            for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
            {
                if (NULL != g_appMqttContext[connectId].addTopics[i])
                {
                    osFree(g_appMqttContext[connectId].addTopics[i]);
                    g_appMqttContext[connectId].addTopics[i] = NULL;
                }
            }
            APP_MqttReportTimeout(connectId);
        }
        break;

    case MQTT_EVENT_UNSUB_TIMEOUT:
        {
            APP_MQTT_PRINTF_INFO("Mqtt unsub timeout, connectId:%u!\r\n", (uint32_t)connectId);
            for (i = 0; i < MAX_MESSAGE_HANDLERS; i ++)
            {
                if (g_appMqttContext[connectId].unsubTopics[i])
                {
                    osFree(g_appMqttContext[connectId].unsubTopics[i]);
                    g_appMqttContext[connectId].unsubTopics[i] = NULL;
                }
            }
            APP_MqttReportTimeout(connectId);
        }
        break;

    case MQTT_EVENT_PUB_TIMEOUT:
    case MQTT_EVENT_PUBACK_TIMEOUT:
    case MQTT_EVENT_PUBREC_TIMEOUT:
    case MQTT_EVENT_PUBCOMP_TIMEOUT:
        {
            APP_MQTT_PRINTF_INFO("Mqtt pub timeout, connectId:%u!\r\n", (uint32_t)connectId);
            APP_MqttPublishFree(connectId);
            APP_MqttReportTimeout(connectId);
        }
        break;

    case MQTT_EVENT_CONN_ERR:
        {
            MqttConnUrcInfo connUrcInfo;
            connUrcInfo.connect_id = connectId;
            connUrcInfo.state = MQTT_CONN_STATE_NET_ERR;

            APP_MQTT_PRINTF_INFO("Mqtt conn err, connectId:%u!\r\n", (uint32_t)connectId);

            APP_MqttFreeConnResource(connectId);
            APP_MqttFreeBufResource(connectId);
            APP_MqttFreeWillResource(connectId);

            AT_MqttSendUnsolicited(MQTT_CONN, g_appMqttContext[connectId].channelId, &connUrcInfo);
        }
        break;

    case MQTT_EVENT_CONNACK:
        {
            MqttConnUrcInfo connUrcInfo;
            connUrcInfo.connect_id = connectId;
            connUrcInfo.state = MQTT_CONN_STATE_SUCCEED;
            g_appMqttContext[connectId].isInit = OS_TRUE;
            APP_MQTT_PRINTF_INFO("Mqtt conn ack, connectId:%u!\r\n", (uint32_t)connectId);

            AT_MqttSendUnsolicited(MQTT_CONN, g_appMqttContext[connectId].channelId, &connUrcInfo);
        }
        break;

    case MQTT_EVENT_PUB_ERR:
        {
            APP_MqttPublishFree(connectId);
            APP_MQTT_PRINTF_INFO("Mqtt pub error, connectId:%u\r\n", (uint32_t)connectId);
        }
        break;

    case MQTT_EVENT_PUB_QOS0:
        {
            APP_MqttPublishFree(connectId);
            APP_MQTT_PRINTF_INFO("Mqtt pub Qos0 free, connectId:%u\r\n", (uint32_t)connectId);
        }
        break;

    case MQTT_EVENT_PUBACK:
        {
            MqttPubackUrcInfo pubAckUrcInfo;
            MQTT_MessageData *data = (MQTT_MessageData *)userData;

            APP_MqttPublishFree(connectId);
            OS_ASSERT(NULL != data);
            pubAckUrcInfo.connect_id = connectId;
            pubAckUrcInfo.mid = data->message->id;
            pubAckUrcInfo.dup = data->message->dup;
            APP_MQTT_PRINTF_INFO("Mqtt connectId:%u, recv puback!\r\n", connectId);
            AT_MqttSendUnsolicited(MQTT_PUBACK, g_appMqttContext[connectId].channelId, &pubAckUrcInfo);
        }
        break;

    case MQTT_EVENT_PUBREC:
        {
            MqttPubrecUrcInfo pubRecUrcInfo;
            MQTT_MessageData *data = (MQTT_MessageData *)userData;
            pubRecUrcInfo.connect_id = connectId;
            pubRecUrcInfo.dup = data->message->dup;
            pubRecUrcInfo.mid = data->message->id;
            APP_MQTT_PRINTF_INFO("Mqtt connectId:%u, recv pubrec!\r\n", connectId);
            AT_MqttSendUnsolicited(MQTT_PUBREC, g_appMqttContext[connectId].channelId, &pubRecUrcInfo);
        }
        break;

	case MQTT_EVENT_PUBCOMP:
        {
            MqttPubcompUrcInfo pubCompUrcInfo;
            MQTT_MessageData *data = (MQTT_MessageData *)userData;
            APP_MqttPublishFree(connectId);
            pubCompUrcInfo.connect_id = connectId;
            pubCompUrcInfo.dup = data->message->dup;
            pubCompUrcInfo.mid = data->message->id;
            APP_MQTT_PRINTF_INFO("Mqtt connectId:%u, recv pubcomp!\r\n", connectId);
            AT_MqttSendUnsolicited(MQTT_PUBCOMP, g_appMqttContext[connectId].channelId, &pubCompUrcInfo);
        }
        break;

    case MQTT_EVENT_SUB_ERR:
        {
            int32_t i;
            for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
            {
                if (NULL != g_appMqttContext[connectId].addTopics[i])
                {
                    osFree(g_appMqttContext[connectId].addTopics[i]);
                    g_appMqttContext[connectId].addTopics[i] = NULL;
                }
            }

            APP_MQTT_PRINTF_INFO("Mqtt sub error, connectId:%u\r\n", (uint32_t)connectId);
        }
        break;

    case MQTT_EVENT_SUBACK:
        {
            MqttSubackUrcInfo subAckUrcInfo;
            int32_t i, j;
            subAckUrcInfo.code_number = 0;

            APP_MQTT_PRINTF_INFO("Mqtt suback, connectId:%u\r\n", (uint32_t)connectId);
            for (i = 0; i < MAX_MESSAGE_HANDLERS; i++)
            {
                bool_t find = OS_FALSE;
                if (NULL == g_appMqttContext[connectId].addTopics[i])
                {
                    break;
                }

                for (j = 0; j < MAX_MESSAGE_HANDLERS; j++)
                {
                    if (g_appMqttContext[connectId].topics[j] != NULL
                        && strcmp(g_appMqttContext[connectId].addTopics[i], g_appMqttContext[connectId].topics[j]) == 0)
                    {
                        find = OS_TRUE;
                        break;
                    }
                }

                if (find)
                {
                    osFree(g_appMqttContext[connectId].addTopics[i]);
                    g_appMqttContext[connectId].addTopics[i] = NULL;
                    g_appMqttContext[connectId].QoSs[j] = g_appMqttContext[connectId].addQoSs[i];
                }
                else
                {
                    for (j = 0; j < MAX_MESSAGE_HANDLERS; j++)
                    {
                        if (NULL == g_appMqttContext[connectId].topics[j])
                        {
                            g_appMqttContext[connectId].topics[j] = g_appMqttContext[connectId].addTopics[i];
                            g_appMqttContext[connectId].QoSs[j] = g_appMqttContext[connectId].addQoSs[i];
                            g_appMqttContext[connectId].addTopics[i] = NULL;
                            break;
                        }
                    }
                    OS_ASSERT(j < MAX_MESSAGE_HANDLERS);
                }

                subAckUrcInfo.code_number++;
                switch (g_appMqttContext[connectId].addQoSs[i])
                {
                case MQTT_QOS0:
                    subAckUrcInfo.code[i] = SUB_SUCCEED_QOS0;
                    break;
                case MQTT_QOS1:
                    subAckUrcInfo.code[i] = SUB_SUCCEED_QOS1;
                    break;
                case MQTT_QOS2:
                    subAckUrcInfo.code[i] = SUB_SUCCEED_QOS2;
                    break;
                default:
                    OS_ASSERT(0);
                    break;
                }
            }

            subAckUrcInfo.connect_id = connectId;
            subAckUrcInfo.mid = ((MQTTClient *)AT_MQTT_CFG_PARAM_GET(connectId, clientHandle))->next_packetid;
            AT_MqttSendUnsolicited(MQTT_SUBACK, g_appMqttContext[connectId].channelId, &subAckUrcInfo);
        }
        break;

    case MQTT_EVENT_UNSUB_ERR:
        {
            for (i = 0; i < MAX_MESSAGE_HANDLERS; i ++)
            {
                if (g_appMqttContext[connectId].unsubTopics[i])
                {
                    osFree(g_appMqttContext[connectId].unsubTopics[i]);
                    g_appMqttContext[connectId].unsubTopics[i] = NULL;
                }
            }

            APP_MQTT_PRINTF_INFO("Mqtt unsub error, connectId:%u\r\n", (uint32_t)connectId);
        }
        break;

    case MQTT_EVENT_UNSUBACK:
        {
            MqttUnsubackUrcInfo unSubAckUrcInfo;

            APP_MQTT_PRINTF_INFO("Mqtt unsub ack, connectId:%u\r\n", (uint32_t)connectId);

            for (i = 0; i < MAX_MESSAGE_HANDLERS; i ++)
            {
                if (g_appMqttContext[connectId].unsubTopics[i])
                {
                    osFree(g_appMqttContext[connectId].unsubTopics[i]);
                    g_appMqttContext[connectId].unsubTopics[i] = NULL;
                }
            }

            unSubAckUrcInfo.connect_id = connectId;
            unSubAckUrcInfo.mid = ((MQTTClient *)AT_MQTT_CFG_PARAM_GET(connectId, clientHandle))->next_packetid;
            AT_MqttSendUnsolicited(MQTT_UNSUBACK, g_appMqttContext[connectId].channelId, &unSubAckUrcInfo);
        }
        break;

    case MQTT_EVENT_PINGRESP:
        {
            if (mqttContext->pingRespRpt)
            {
                MQTTClient *client = (MQTTClient *)handle;
                MqttPingrespUrcInfo pingRespUrcInfo;

                APP_MQTT_PRINTF_INFO("Mqtt ping resp:%d, connectId:%u\r\n", (int)client->ping_outstanding, (uint32_t)connectId);
                pingRespUrcInfo.connect_id = connectId;
                pingRespUrcInfo.ping_ret = (client->ping_outstanding == 0) ? MQTT_PING_RET_SUCCEED : MQTT_PING_RET_TIMEOUT;
                AT_MqttSendUnsolicited(MQTT_PINGRESP, g_appMqttContext[connectId].channelId, &pingRespUrcInfo);
            }
        }
        break;

    case MQTT_EVENT_DISC_OK:
        {
            MqttConnUrcInfo connUrcInfo;
            APP_MqttFreeConnResource(connectId);
            APP_MqttFreeWillResource(connectId);
            APP_MqttFreeBufResource(connectId);

            APP_MQTT_PRINTF_INFO("Mqtt disc ok, connectId:%u\r\n", (uint32_t)connectId);

            g_appMqttContext[connectId].isInit = OS_FALSE;
            connUrcInfo.state = MQTT_CONN_STATE_CLIENT_DSICONNECTED;
            AT_MqttSendUnsolicited(MQTT_CONN, g_appMqttContext[connectId].channelId, &connUrcInfo);
        }
        break;

    case MQTT_EVENT_SESSION_CLOSED:
        {
            int rc = MQTT_Disconnect(g_appMqttContext[connectId].clientHandle);
            APP_MQTT_PRINTF_INFO("Mqtt recv session closed, connectId:%u\r\n", (uint32_t)connectId);
            if (MQTT_OK != rc)
            {
                APP_MQTT_PRINTF_INFO("Mqtt recv session closed, but disc async error, connectId:%u\r\n", (uint32_t)connectId);
            }
        }
        break;

    case MQTT_EVENT_DROP:
        {
            MqttDropUrcInfo dropUrcInfo;
            MQTT_MessageData *data = (MQTT_MessageData *)userData;
            OS_ASSERT(NULL != data && NULL != data->message && NULL != data->topicName);
            if (NULL != data && NULL != data->message && NULL != data->topicName)
            {
                dropUrcInfo.connect_id = connectId;
                dropUrcInfo.dropped_length = data->message->payloadlen + data->topicName->lenstring.len + 1; // +1是填充'\0'

                APP_MQTT_PRINTF_INFO("Mqtt drop cached message, dropLen:%u, connectId:%u\r\n", dropUrcInfo.dropped_length, (uint32_t)connectId);
                AT_MqttSendUnsolicited(MQTT_DROP, g_appMqttContext[connectId].channelId, &dropUrcInfo);
            }
            else
            {
                APP_MQTT_PRINTF_INFO("Mqtt drop cached message, data is null\r\n");
            }
        }
        break;

    case MQTT_EVENT_NEW_MSG_RECV:
        {
            MqttPubNmiUrcInfo pubNmiUrcInfo;
            MQTT_MessageData *data = (MQTT_MessageData *)userData;
            OS_ASSERT(NULL != data && NULL != data->message && NULL != data->topicName);
            if (NULL != data && NULL != data->message && NULL != data->topicName)
            {
                pubNmiUrcInfo.connect_id = connectId;
                pubNmiUrcInfo.mid = data->message->id;
                pubNmiUrcInfo.data_len = data->message->payloadlen + data->topicName->lenstring.len + 1; // +1是填充'\0'

                APP_MQTT_PRINTF_INFO("Mqtt pubNmi, dataLen:%u, connectId:%u\r\n", pubNmiUrcInfo.data_len, (uint32_t)connectId);
                AT_MqttSendUnsolicited(MQTT_PUBNMI, g_appMqttContext[connectId].channelId, &pubNmiUrcInfo);
            }
            else
            {
                APP_MQTT_PRINTF_INFO("Mqtt pubNmi, data is null\r\n");
            }
        }
        break;

    default:
        break;
    }
}

static bool APP_MqttClientIsValid(MQTT_Handle handle)
{
    bool ret = false;
    int i;

    if(handle == NULL)
        return false;

    for(i = 0; i < APP_MQTT_CONNECT_CNT; i++)
    {
        if(handle == (MQTT_Handle)&g_appMqttClient[i].client)
        {
            ret = true;
            break;
        }
    }

    return ret;
}

static void APP_MqttProtocolInit(void)
{
    MQTT_CustomerClientCheckSet(APP_MqttClientIsValid);
    MQTT_WorkMutexInit();
}

int APP_MqttInit(void)
{
    int i;

    //  MQTT 协议初始化
    APP_MqttProtocolInit();

    osMemset(g_appMqttContext, 0x0, APP_MQTT_CONNECT_CNT * sizeof(APP_MqttContext));
    for (i = 0; i < APP_MQTT_CONNECT_CNT; i++)
    {
        MQTT_ConnectOption connectData = MQTT_CONNECTDATA_INITIALIZER;
        g_appMqttContext[i].connectData = connectData;
        g_appMqttContext[i].clientHandle = (MQTT_Handle)&g_appMqttClient[i].client;
        g_appMqttClient[i].mqttContext = &g_appMqttContext[i];
        MQTT_InitClientDefaultParam(g_appMqttContext[i].clientHandle);
        APP_MqttContextClientParamInit(&g_appMqttClient[i].mqttContext->clientParam, g_appMqttContext[i].clientHandle);
        g_appMqttContext[i].isInit = OS_FALSE;
    }

    return MQTT_OK;
}

