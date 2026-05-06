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
 * 2023-08-28     ict team          创建
 ************************************************************************************
 */

#ifndef __APP_MQTT_H__
#define __APP_MQTT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os.h"
#include "mqtt_api.h"

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define MQTT_MAX_TOPIC_NUM                      3

#define APP_MQTT_COMMAND_TIMEOUT                30000   ///< 单位ms
#define APP_MQTT_SEND_BUF_SIZE                  2048
#define APP_MQTT_RECV_BUF_SIZE                  2048
#define APP_MQTT_CONNECT_CNT                    6

#define AT_MQTT_CFG_PARAM_GET(connectId, member)        g_appMqttContext[connectId].member
#define AT_MQTT_CFG_PARAM_SET(connectId, member, value) g_appMqttContext[connectId].member = (value)
#define AT_MQTT_CFG_PARAM_GET_ADDR(connectId, member)   (&g_appMqttContext[connectId].member)

#if !defined(MAX_MESSAGE_HANDLERS)
#define MAX_MESSAGE_HANDLERS 5 /* redefinable - how many subscriptions do you want? */
#endif

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum
{
    MQTT_CONN,
    MQTT_PUBNMI,
    MQTT_DROP,
    MQTT_PUBLISH,
    MQTT_PINGRESP,
    MQTT_TIMEOUT,
    MQTT_SUBACK,
    MQTT_UNSUBACK,
    MQTT_PUBACK,
    MQTT_PUBREC,
    MQTT_PUBCOMP
}MqttUrcType;

typedef enum
{
    MQTT_UNKNOW_ERR = 600,
    MQTT_UNVALID_PARA_ERR,
    MQTT_CONNECT_FAILED_ERR,
    MQTT_CONNECTING,
    MQTT_CONNECTED,
    MQTT_NET_ERR,
    MQTT_STORAGE_ERR,
    MQTT_STATE_ERR,
    MQTT_DNS_ERR
}MqttErrCode;

typedef enum
{
    MQTT_CONN_STATE_SUCCEED,
    MQTT_CONN_STATE_ONGOING,
    MQTT_CONN_STATE_CLIENT_DSICONNECTED,
    MQTT_CONN_STATE_SERVER_REFUSED,
    MQTT_CONN_STATE_SERVER_DISCONNECTED,
    MQTT_CONN_STATE_PING_TIMEOUT,
    MQTT_CONN_STATE_NET_ERR,
    MQTT_CONN_STATE_UNKNOW_ERR = 255
}MqttConnState;

typedef enum
{
    MQTT_PING_RET_SUCCEED,
    MQTT_PING_RET_TIMEOUT
}MqttPingRet;

typedef enum
{
    SUB_SUCCEED_QOS0,
    SUB_SUCCEED_QOS1,
    SUB_SUCCEED_QOS2,
    SUB_FAILED = 128,
}MqttCode;

typedef enum
{
    MQTT_DUP_NORAML_DATA,
    MQTT_DUP_RETRY_DATA
}MqttDup;

typedef enum
{
    ASCII_STR = 0,
    HEX_STR,
    CONVERT_STR
}MqttMsgFormat;

typedef struct
{
    uint8_t connect_id;
    MqttConnState state;
}MqttConnUrcInfo;

typedef struct
{
    uint8_t connect_id;
    uint16_t mid;
    uint32_t data_len;
}MqttPubNmiUrcInfo;

typedef struct
{
    uint8_t connect_id;
    uint32_t dropped_length;
}MqttDropUrcInfo;

typedef struct
{
    uint8_t connect_id;
    uint16_t mid;
    uint8_t *topic;
    uint32_t total_len;
    uint32_t payload_len;
    uint8_t *payload;
}MqttPublishUrcInfo;

typedef struct
{
    uint8_t connect_id;
    MqttPingRet ping_ret;
}MqttPingrespUrcInfo;

typedef struct
{
    uint8_t connect_id;
    uint16_t mid;
}MqttTimeoutUrcInfo;

typedef struct
{
    uint8_t connect_id;
    uint16_t mid;
    uint8_t code_number;
    MqttCode code[MQTT_MAX_TOPIC_NUM];
}MqttSubackUrcInfo;

typedef struct
{
    uint8_t connect_id;
    uint16_t mid;
}MqttUnsubackUrcInfo;

typedef struct
{
    uint8_t connect_id;
    uint16_t mid;
    MqttDup dup;
}MqttPubackUrcInfo;

typedef struct
{
    uint8_t connect_id;
    uint16_t mid;
    MqttDup dup;
}MqttPubrecUrcInfo;

typedef struct
{
    uint8_t connect_id;
    uint16_t mid;
    MqttDup dup;
}MqttPubcompUrcInfo;

typedef struct
{
    unsigned int pingInterval;
    int retransInterval;
    int retryTimes;
    int reconnTimes;
    int reconnInterval;
    int reconnMode;
    int cachedMode;
} APP_MqttClientParam;

typedef struct
{
    uint8_t                channelId;
    uint8_t                connectId;
    uint8_t                isInit;
    uint8_t                pingRespRpt;
    uint8_t               *sendbuf;
    uint8_t               *readbuf;
    char                  *pHost;
    char                  *topics[MAX_MESSAGE_HANDLERS];
    int                    QoSs[MAX_MESSAGE_HANDLERS];
    APP_MqttClientParam    clientParam;
    MQTT_MessageHandler    msgHandler;
    MQTT_Handle            clientHandle;
    MQTT_ConnectOption     connectData;
    char                  *addTopics[MAX_MESSAGE_HANDLERS];
    int                    addQoSs[MAX_MESSAGE_HANDLERS];
    char                  *unsubTopics[MAX_MESSAGE_HANDLERS];
    MQTT_Message          *pubMessage;
    char                  *pubTopic;
} APP_MqttContext;

typedef struct
{
    unsigned short mid;
    char *topic;
    unsigned char *payload;
    unsigned int payloadlen;
} APP_MqttCacheMessage;

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
extern APP_MqttContext g_appMqttContext[APP_MQTT_CONNECT_CNT];

void APP_MqttFreeWillResource(uint8_t connectId);
void APP_MqttSetString(char **destbuf, char *srcBuf);
bool_t APP_MqttIsConnected(uint8_t connectId);
int32_t APP_MqttConnect(uint8_t channelId, uint8_t connectId, char *host,
        uint16_t port, char *clientID, char *username, char *password);
int32_t APP_MqttSubscribe(uint8_t connectId, uint32_t topicCnt, char* topics[], int32_t QoSs[]);
int32_t APP_MqttUnsubscribe(uint8_t connectId, uint32_t topicCnt, char *topics[]);
int32_t APP_MqttPublish(uint8_t connectId, char *topic, int32_t qos,
        uint8_t retain, uint8_t dup, void *payload, size_t payloadlen);
void APP_MqttQueryCacheMessageCount(uint8_t connectId, uint32_t *storeMsgCount, uint32_t *totalMsgLen);
uint32_t APP_MqttReadCacheMessage(uint8_t connectId, uint32_t count, APP_MqttCacheMessage *mqttCacheMessages[]);
void APP_MqttFreeCacheMessage(uint8_t connectId, uint32_t count);
int32_t APP_MqttDisconnect(uint8_t connectId);

#ifdef __cplusplus
}
#endif
#endif

