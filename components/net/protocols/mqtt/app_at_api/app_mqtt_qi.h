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

#ifndef __APP_MQTT_QI_H__
#define __APP_MQTT_QI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os.h"
#include "mqtt_api.h"
#include "app_at_ssl.h"
#include "app_at_pdp.h"

#define APP_MQTT_QI_COMMAND_TIMEOUT                30000   ///< 单位ms
#define APP_MQTT_QI_SEND_BUF_SIZE                  2048
#define APP_MQTT_QI_RECV_BUF_SIZE                  2048
#define APP_MQTT_QI_CLIENT_CNT                     6

#define APP_MQTT_QI_CLIENTIDX_MIN                  0
#define APP_MQTT_QI_CLIENTIDX_MAX                  5

#define APP_MQTT_QI_V3_1                           3
#define APP_MQTT_QI_V3_1_1                         4
#define APP_MQTT_QI_PDP_CID_MIN                    APP_AT_PDP_CONTEXT_MIN
#define APP_MQTT_QI_PDP_CID_MAX                    APP_AT_PDP_CONTEXT_MAX

#define APP_MQTT_QI_SSL_DISABLE                    0
#define APP_MQTT_QI_SSL_ENABLE                     1
#define APP_MQTT_QI_SSL_CTX_IDX_MIN                0
#define APP_MQTT_QI_SSL_CTX_IDX_MAX                (AT_SSL_MAX_SSL_CTX_NUM - 1)

#define APP_MQTT_QI_KEEP_ALIV_TIME_MIN             0
#define APP_MQTT_QI_KEEP_ALIV_TIME_MAX             3600

#define APP_MQTT_QI_CLEAN_SESSION_0                0
#define APP_MQTT_QI_CLEAN_SESSION_1                1

#define APP_MQTT_QI_PKT_TIMEOUT_MIN                20
#define APP_MQTT_QI_PKT_TIMEOUT_MAX                60
#define APP_MQTT_QI_PKT_TIMEOUT_DEFAULT            20
#define APP_MQTT_QI_RETRY_TIMES_MIN                0
#define APP_MQTT_QI_RETRY_TIMES_MAX                3
#define APP_MQTT_QI_RETRY_TIMES_DEFAULT            0
#define APP_MQTT_QI_TIMEOUT_NOTICE_DISABLE         0
#define APP_MQTT_QI_TIMEOUT_NOTICE_ENABLE          1

#define APP_MQTT_QI_WILL_FG_UNSET                  0
#define APP_MQTT_QI_WILL_FG_SET                    1
#define APP_MQTT_QI_WILL_QOS_MIN                   0
#define APP_MQTT_QI_WILL_QOS_MAX                   2
#define APP_MQTT_QI_WILL_RETAIN_0                  0
#define APP_MQTT_QI_WILL_RETAIN_1                  1
#define APP_MQTT_QI_WILL_LEN_MIN                   0
#define APP_MQTT_QI_WILL_LEN_MAX                   256

#define APP_MQTT_QI_RECV_NO_CACHE                  0
#define APP_MQTT_QI_RECV_CACHE                     1
#define APP_MQTT_QI_RECV_LEN_DISABLE               0
#define APP_MQTT_QI_RECV_LEN_ENABLE                1

#define APP_MQTT_QI_SEND_URC_INDICATED            0
#define APP_MQTT_QI_SEND_URC_NO_INDICATED         1

#define APP_MQTT_QI_SEND_MODE_STRING_FORMAT        0
#define APP_MQTT_QI_SEND_MODE_HEX_FORMAT           1

#define APP_MQTT_QI_RECV_MODE_STRING_FORMAT        0
#define APP_MQTT_QI_RECV_MODE_HEX_FORMAT           1

#define APP_MQTT_QI_IMTPING_INTERVAL_MIN           5
#define APP_MQTT_QI_IMTPING_INTERVAL_MAX           60

#define APP_MQTT_PROTOCAL_CHECK_DISABLE            0
#define APP_MQTT_PROTOCAL_CHECK_ENABLE             1

#define APP_MQTT_VIEW_MODE_NO_ECHO                 0
#define APP_MQTT_VIEW_MODE_ECHO                    1
#define APP_MQTT_VIEW_MODE_DEFAULT                 0

#define APP_MQTT_EDIT_MODE_EXIT_DISABLE            0
#define APP_MQTT_EDIT_MODE_EXIT_ENABLE             1
#define APP_MQTT_EDIT_MODE_EXIT_DEFAULT            0

#define APP_MQTT_EDIT_TIME_MIN                     1
#define APP_MQTT_EDIT_TIME_MAX                     120
#define APP_MQTT_EDIT_TIME_DEFAULT                 120

#define APP_MQTT_QI_PORT_MIN                       1
#define APP_MQTT_QI_PORT_MAX                       65535

#define APP_MQTT_MSG_ID_INVALID                    0   // Mqtt 协议规定采用Qos为0发布消息时，msgId必须为0，即MQTT报文中不携带msgId字段，服务器无需对Qos为0的消息进行确认。
#define APP_MQTT_MSG_ID_MIN                        1
#define APP_MQTT_MSG_ID_MAX                        65535


#define APP_MQTT_IMTPING_INTERVAL_MIN              5
#define APP_MQTT_IMTPING_INTERVAL_MAX              60
#define APP_MQTT_IMTPING_INTERVAL_DEFAULT          5

#define APP_MQTT_CACHE_RECV_MSG_MAX                5

#define AT_MQTT_QI_CFG_PARAM_GET(clientIdx, member)        g_appMqttQiContext[clientIdx].member
#define AT_MQTT_QI_CFG_PARAM_SET(clientIdx, member, value) g_appMqttQiContext[clientIdx].member = (value)
#define AT_MQTT_QI_CFG_PARAM_GET_ADDR(clientIdx, member)   (&g_appMqttQiContext[clientIdx].member)

#if !defined(MAX_MESSAGE_HANDLERS)
#define MAX_MESSAGE_HANDLERS 5 /* redefinable - how many subscriptions do you want? */
#endif

#if !defined(MQTT_MAX_TOPICS_EACH_TIME)
#define MQTT_MAX_TOPICS_EACH_TIME   3    // 单次订阅/取消订阅最大主题数
#endif

typedef struct osMutex Mutex;

typedef enum
{
    MQTT_QI_UNKNOW_ERR = 600,
    MQTT_QI_UNVALID_PARA_ERR,
    MQTT_QI_CONNECT_FAILED_ERR,
    MQTT_QI_CONNECTING,
    MQTT_QI_CONNECTED,
    MQTT_QI_NET_ERR,
    MQTT_QI_STORAGE_ERR,
    MQTT_QI_STATE_ERR,
    MQTT_QI_DNS_ERR,
    MQTT_QI_TIMEOUT_ERR
}MqttQiErrCode;


typedef enum
{
    MQTT_QI_PKG_TRAN_SUCCESS,  
    MQTT_QI_PKG_RETRNS, 
    MQTT_QI_PKG_TRAN_FAIL,   
}MqttQiPkgSendResult;


typedef struct
{
    unsigned short msgid;
    char *topic;
    unsigned char *payload;
    unsigned int payloadlen;
	uint8_t storeStatus;
} APP_MqttQiCacheMessage;

typedef enum
{
    MQTT_QI_CONN,
    MQTT_QI_DISK,
    MQTT_QI_PUBNMI,
    MQTT_QI_DROP,
    MQTT_QI_PUBLISH,
    MQTT_QI_PINGRESP,
    MQTT_QI_TIMEOUT,
    MQTT_QI_SUBACK,
    MQTT_QI_UNSUBACK,
    MQTT_QI_PUBACK,
    MQTT_QI_PUBREC,
    MQTT_QI_PUBCOMP,
    MQTT_QI_STAT,
}MqttQiUrcType;

typedef enum
{
    MQTT_QI_CONN_SUCCESS,
    MQTT_QI_CONN_REFUSE_PROTO_ERR,
    MQTT_QI_CONN_REFUSE_ID_ERR,
    MQTT_QI_CONN_REFUSE_SERVER_UNUSED,
    MQTT_QI_CONN_REFUSE_USER_PASSWORD_ERR,
    MQTT_QI_CONN_REFUSE_UNAUTHORIZED,
    MQTT_QI_CONN_REFUSE_UNKNOWN_ERR = 0xFF,
}MqttQiRetCode;

typedef enum
{
    MQTT_QI_CONN_STATE_INIT = 1,
    MQTT_QI_CONN_STATE_CONNING,
    MQTT_QI_CONN_STATE_CONNECTED,
    MQTT_QI_CONN_STATE_DISC,
}MqttQiConnState;

typedef enum
{
    MQTT_QI_PING_RET_SUCCEED,
    MQTT_QI_PING_RET_TIMEOUT
}MqttQiPingRet;

typedef enum
{
    MQTT_QI_SUB_SUCCEED_QOS0,
    MQTT_QI_SUB_SUCCEED_QOS1,
    MQTT_QI_SUB_SUCCEED_QOS2,
    MQTT_QI_SUB_FAILED = 128,
}MqttQiSubCode;

typedef enum
{
    MQTT_QI_STAT_CLOSE_BY_SERVER = 1
}MqttQiStatRet;

typedef enum
{
    MQTT_QI_DUP_NORAML_DATA,
    MQTT_QI_DUP_RETRY_DATA
}MqttQiDup;

typedef enum
{
    ASCII_STR = 0,
    HEX_STR,
    CONVERT_STR
}MqttQiMsgFormat;

typedef struct
{
    uint8_t clientIdx;
    uint8_t closeMode;
    MqttQiRetCode retcode;
	MqttQiPkgSendResult result;
}MqttQiConnUrcInfo;


typedef struct
{
    uint8_t clientIdx;
    uint32_t dropped_length;
}MqttQiDropUrcInfo;

typedef struct
{
    uint8_t clientIdx;
    uint16_t msgid;
    uint8_t *topic;
    //uint32_t total_len;
    uint32_t payload_len;
    uint8_t *payload; 
	uint8_t  dup;
    uint8_t  isPub;
}MqttQiPubUrcInfo;


typedef struct
{
    uint8_t clientIdx;
    MqttQiPingRet ping_ret;
}MqttQiPingrespUrcInfo;

typedef struct
{
    uint8_t clientIdx;
    uint16_t msgid;
}MqttQiTimeoutUrcInfo;

typedef struct
{
    uint8_t clientIdx;
    uint16_t msgid;
    uint8_t code_number;
    MqttQiSubCode code[MQTT_MAX_TOPICS_EACH_TIME];
}MqttQiSubackUrcInfo;

typedef struct
{
    uint8_t clientIdx;
    uint16_t msgid;
}MqttQiUnsubackUrcInfo;

typedef struct
{
    uint8_t clientIdx;
    MqttQiStatRet stat_ret;
}MqttQiStatUrcInfo;

typedef struct
{
    struct osTimer *timeOut;
	uint32_t        ms;
} MqttQiTimer;
typedef void (*MqttQiTimerCallbackFunc)             (void *argument);

typedef struct
{
    unsigned int pingInterval;
    int retransInterval;
    int retryTimes;
    int reconnTimes;
    int reconnInterval;
    int reconnMode;
    int cachedMode;
} APP_MqttQiClientParam;

typedef struct
{
    uint8_t                channelId;
    uint8_t                clientIdx;
    uint8_t                pdpCid;
    uint8_t                sslEnable;
    uint8_t                sslCtxIdx;
    uint8_t                timeOutNotice;
    uint8_t                isInit;
    uint8_t                isOpen;
    uint8_t               *sendbuf;
    uint8_t               *readbuf;
    char                  *pHost;
    uint16_t               port;
    char                  *topics[MAX_MESSAGE_HANDLERS];
    int                    QoSs[MAX_MESSAGE_HANDLERS];
    APP_MqttQiClientParam  clientParam;
    MQTT_MessageHandler    msgHandler;
    MQTT_Handle            clientHandle;
    MQTT_ConnectOption     connectData;
    char                  *addTopics[MAX_MESSAGE_HANDLERS];
    int                    addQoSs[MAX_MESSAGE_HANDLERS];
    char                  *unsubTopics[MAX_MESSAGE_HANDLERS];
    MQTT_Message          *pubMessage;
    char                  *pubTopic;
    uint8_t               *pubMsgPayloadRecv;       /* 透传下已接收的pub payload数据 */
    size_t                 pubMsgPayloadRecvLen;    /* 透传下已接收的pub payload数据长度 */
    uint8_t               *willMsgRecv;             /* 透传下已接收的will 数据 */
    uint16_t               willMsgRecvLen;          /* 透传下已接收的will 数据 */
    uint16_t               willexWillLen;
    uint8_t                recvTransparent;
    Mutex                  mqttQiMutex;
    uint8_t                msgLenEnable;
    uint8_t                msgSendMode;
    uint8_t                msgRecvMode;
    uint8_t                sendMode;
    uint8_t                recvMode;
    uint8_t                viewMode;
    uint8_t                editMode;
    uint8_t                editTime;
    uint8_t                isPub;       /* 指示采用的是哪种命令发布消息 1：QMTPUB，0：QMTPUBEX */
    uint8_t                closeMode;   /* 指示在 mqttconnected 状态下是采用哪种命令关闭 mqtt client 链接, 1: QMTCLOSE，0：QMTDISC */
    APP_MqttQiCacheMessage cacheMessages[APP_MQTT_CACHE_RECV_MSG_MAX];
    MqttQiTimer            transTimer;          /* 透传超时定时器指针 */
    MqttQiTimer            pubExReadyTimer;     /* pubExReady超时定时器指针 */
    MqttQiTimer            retransTimer;        /* 数据包超时重传定时器 */
 
    int8_t (*AT_MqttUrcUnsolicited)(MqttQiUrcType type, uint8_t channelId, void *urcInfo);
} APP_MqttQiContext;

extern APP_MqttQiContext g_appMqttQiContext[APP_MQTT_QI_CLIENT_CNT];

void APP_MqttQiFreeWillResource(uint8_t clientIdx);
void APP_MqttQiFreeBufResource(uint8_t clientIdx);
void APP_MqttQiFreeConnResource(uint8_t clientIdx);
void APP_MqttQiPublishFree(uint8_t clientIdx);

void APP_MqttQiSetString(char **destbuf, char *srcBuf);
bool_t APP_MqttQiIsConnected(uint8_t clientIdx);
int32_t APP_MqttQiConnect(uint8_t channelId, uint8_t clientIdx, char *clientID, char *username, char *password);
int32_t APP_MqttQiOpen(uint8_t channelId, uint8_t clientIdx, char *hostName, uint16_t port);
int32_t APP_MqttQiClose(uint8_t clientIdx);
int32_t APP_MqttQiSubscribe(uint8_t clientIdx, uint16_t id, uint32_t topicCnt, char* topics[], int32_t QoSs[]);
int32_t APP_MqttQiUnsubscribe(uint8_t clientIdx, uint16_t id, uint32_t topicCnt, char *topics[]);
int32_t APP_MqttQiPublish(uint8_t clientIdx, uint16_t id, char *topic, int32_t qos, uint8_t retain, 
            uint8_t dup, void *payload, size_t payloadlen, uint8_t pubex);
void APP_MqttQiQueryCacheMessageCount(uint8_t connectId, uint32_t *storeMsgCount, uint32_t *totalMsgLen);
int32_t APP_MqttQiDisconnect(uint8_t clientIdx, uint8_t closeMode);

void APP_MqttSetString(char **destbuf, char *srcBuf);

int8_t MqttQiTimerNew(MqttQiTimer *timer, uint32_t ms, void *parameter, MqttQiTimerCallbackFunc timerCb);
void MqttQiTimerStart(MqttQiTimer *timer);
void MqttQiTimerStop(MqttQiTimer *timer);
void MqttQiTimerRelease(MqttQiTimer *timer);

#ifdef __cplusplus
}
#endif
#endif

