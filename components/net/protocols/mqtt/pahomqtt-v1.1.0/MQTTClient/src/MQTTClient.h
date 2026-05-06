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
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *    Ian Craggs - documentation and platform specific header
 *    Ian Craggs - add setMessageHandler function
 *******************************************************************************/

#if !defined(MQTT_CLIENT_H)
#define MQTT_CLIENT_H

#include "stdbool.h"
#include "MQTTPacket.h"
#include "MQTTPort.h"

#define MAX_PACKET_ID 65535 /* according to the MQTT specification - do not change! */

#if !defined(MAX_MESSAGE_HANDLERS)
#define MAX_MESSAGE_HANDLERS 5 /* redefinable - how many subscriptions do you want? */
#endif

#define MQTT_MAX_CLIENT_CNT         6
#define MQTT_MAX_CACHED_SIZE        1024
#define MQTT_MAX_TOPICS_EACH_TIME   3    // 单次订阅/取消订阅最大主题数

/* all failure return codes must be negative */
enum returnCode { BUFFER_OVERFLOW = -2, FAILURE = -1, SUCCESS = 0 };

typedef enum
{
    MQTT_EVENT_ERROR = -2,
    MQTT_EVENT_TIMEOUT = -1,
    MQTT_EVENT_UNKNOWN = 0,
    MQTT_EVENT_CONN,
    MQTT_EVENT_RECONN,
    MQTT_EVENT_CONN_ERR,
    MQTT_EVENT_CONNACK,
    MQTT_EVENT_CONN_TIMEOUT,
    MQTT_EVENT_RECONN_TIMEOUT,
    MQTT_EVENT_PUB,
    MQTT_EVENT_PUB_QOS0,
    MQTT_EVENT_PUB_ERR,
    MQTT_EVENT_PUBACK,
    MQTT_EVENT_PUBACK_TIMEOUT,
    MQTT_EVENT_PUBACK_TIMEOUT_RETRANS,
    MQTT_EVENT_PUBREC,
    MQTT_EVENT_PUBREC_TIMEOUT,
    MQTT_EVENT_PUBREC_TIMEOUT_RETRANS,
    MQTT_EVENT_PUBREL,
    MQTT_EVENT_PUBCOMP,
    MQTT_EVENT_PUBCOMP_TIMEOUT,
    MQTT_EVENT_PUBCOMP_TIMEOUT_RETRANS,
    MQTT_EVENT_PUB_TIMEOUT,
    MQTT_EVENT_SUB,
    MQTT_EVENT_SUB_ERR,
    MQTT_EVENT_SUBACK,
    MQTT_EVENT_SUB_TIMEOUT,
    MQTT_EVENT_UNSUB,
    MQTT_EVENT_UNSUB_ERR,
    MQTT_EVENT_UNSUBACK,
    MQTT_EVENT_UNSUB_TIMEOUT,
    MQTT_EVENT_PINGREQ,
    MQTT_EVENT_PINGRESP,
    MQTT_EVENT_DISC,
    MQTT_EVENT_DISC_OK,
    MQTT_EVENT_NET_RECV,
    MQTT_EVENT_SESSION_CLOSED,
    MQTT_EVENT_TIMER_TIMEOUT,   // 定时器超时
    MQTT_EVENT_DROP,
    MQTT_EVENT_NEW_MSG_RECV,
    MQTT_EVENT_NET_CLOSE,
} MQTTEventType;

typedef struct MQTTConnackData
{
    unsigned char rc;
    unsigned char sessionPresent;
} MQTTConnackData;

typedef struct MQTTSubackData
{
    int grantedQoS[MAX_MESSAGE_HANDLERS];
} MQTTSubackData;

typedef void (*MQTTEventCallback)(MQTT_Handle handle, void *data, MQTTEventType eventType);  /* (void*, void*, MQTTEventType) */

typedef struct
{
    struct osSlistNode  node;
    unsigned int        size;
    MQTT_MessageData    data;
} MQTT_CachedMessage;

typedef struct
{
    osTick_t reqStartTick;
    osTick_t reqEndTick;
    int eventId;
    uint8_t *buf;
} MQTT_ProcessingReqInfo;

typedef struct MQTTClient
{
    unsigned int defaultParamInit;
    unsigned int next_packetid;
    unsigned int command_timeout_ms;
    unsigned int syncFlag;
    size_t buf_size;
    size_t readbuf_size;
    unsigned char *buf;
    unsigned char *readbuf;
    unsigned int keepAliveInterval; // 保活时间。范围：0-65535。单位s，默认值120
    unsigned int pingInterval; // 心跳间隔时间。范围5-86400。单位s，默认值120
    char ping_outstanding;
    int isconnected;
    int cleansession;
    int retransInterval;    // 数据包重传初始间隔时间。范围：20-60, 单位：s。默认值20。仅用于PUB QoS1/QoS2的重传。
    int retryTimes;         // 数据包传输超时后最大重发次数。范围：0-3。默认值0。仅用于PUB QoS1/QoS2的重传。
    int currentRetryTimes;  // 数据包已重传次数, 仅用于PUB QoS1/QoS2的重传。
    int reconnTimes;        // MQTT重连次数。范围：0-3, 单位：次。默认值3。
    int currentReconnTimes; // MQTT已重连次数
    int reconnInterval;     // 重连间隔。范围：20-60, 单位：s。默认值20。
    int reconnMode;         // 重连策略。0:以固定间隔重连;1:以n倍间隔重连
    int cachedMode;         // 接收MQTT消息缓存模式, 立即生效。0：无缓存 1：缓存到本地
    int cachedSize;         // 已缓存的消息长度
    int reconnEnable;       // 重连功能开关。0：关闭；1：打开。如果不使用MQTTNetworkConnect函数创建的连接, 需要关闭重连功能

    osSlist_t cachedMessageList;

    struct MessageHandlers
    {
        const char* topicFilter;
        void (*fp) (MQTT_Handle, MQTT_MessageData*);         /* (MQTTClient*, MQTT_MessageData*) */
    } messageHandlers[MAX_MESSAGE_HANDLERS];      /* Message handlers are indexed by subscription topic */

    void (*defaultMessageHandler) (MQTT_Handle, MQTT_MessageData*);  /* (MQTTClient*, MQTT_MessageData*) */

    Network* ipstack;
    Timer keepAliveTimer;   // 心跳定时器, 间隔pingInterval
    Timer retransTimer;     // Qos1、Qos2超时重传定时器
    Timer reConnTimer;      // 重连定时器
    Mutex mutex;

    void (*eventReport) (MQTT_Handle, void*, MQTTEventType eventType); /* (MQTTClient*, void*, MQTTEventType eventType) */

    Timer processingReqTimeout;
    MQTT_ProcessingReqInfo reqInfo;

    MQTTPacket_connectData  connectData;
    osMb_t  recvSyncEventMb; // 仅用于同步模式下, 接收到服务端事件队列
    MQTT_ConnectionLost connLostHandlerInSyncMode;    // 连接断开回调，仅用于同步模式下
    MQTT_EventNotify_t eventNotifyHandlerInSyncMode;    // 事件通知回调，仅用于同步模式下
} MQTTClient;

typedef struct
{
    struct osSlistNode node;
    MQTTClient *client;
    int eventId;
    uint8_t buf[0];
} MQTT_EventInfo;

typedef struct
{
    MQTTPacket_connectData  *connectData;
} MQTT_ConnInfo;

typedef struct
{
    uint16_t                 id;        // 0:无效值
    uint32_t                 topicCnt;
    const char              *topics[MQTT_MAX_TOPICS_EACH_TIME];
    int                      QoSs[MQTT_MAX_TOPICS_EACH_TIME];
    MQTT_MessageHandler      messageHandler;
} MQTT_SubInfo;

typedef struct
{
    uint16_t                 id;        // 0:无效值
    uint32_t                 topicCnt;
    const char              *topics[MQTT_MAX_TOPICS_EACH_TIME];
} MQTT_UnsubInfo;

typedef struct
{
    const char              *topic;
    MQTT_Message            *message;
} MQTT_PubInfo;

typedef struct
{
    uint32_t                 len;
} MQTT_RecvInfo;

typedef void (*MQTTTimerOutProcFunc)(MQTTClient *);
typedef struct
{
    MQTTTimerOutProcFunc     fp;
} MQTT_TimerOutInfo;

//   检测用户的Client是否有效，true表示有效句柄，适用AT命令
typedef bool (*MQTTCustomerClientIsValid)(MQTT_Handle handle);
void MQTT_CustomerClientCheckSet(MQTTCustomerClientIsValid func);

/**
 * Init an MQTT client default param.
 * @param handle - the client handle.
 * @return void
 */
void MQTT_InitClientDefaultParam(MQTT_Handle handle);

/**
 * Init an MQTT async client handle. Only use MQTT_DeinitClient to deinit
 * an MQTT async client handle ininted by MQTTAsync_InitClient.
 * @param handle - the client handle.
 * @param commandTimeoutMs
 * @param sendbuf - sendbuf pointer.
 * @param sendbufSize - sendbuf size.
 * @param readbuf - readbuf pointer.
 * @param readbufSize - readbuf size.
 * @return MQTT return code
 */
int MQTTAsync_InitClient(MQTT_Handle handle, unsigned int commandTimeoutMs,
    unsigned char* sendbuf, size_t sendbufSize, unsigned char* readbuf, size_t readbufSize);

/**
 * Deinit an MQTT client handle - This function frees the resources allocated to an MQTT client.
 * @param handle - the client handle.
 */
void MQTT_DeinitClient(MQTT_Handle handle);

/** MQTT SetEventCallback - set event callback in async mode
 *  @param handle - the client handle
 *  @param eventCallback - event callback func
 *  @return
 */
void MQTT_SetEventCallback(MQTT_Handle handle, MQTTEventCallback eventCallback);

/** MQTT Subscribe - send an MQTT subscribe packet and wait for suback before returning.
 *  @param handle - the client handle
 *  @param id - packetid
 *  @param count - number of members in the topicFilters and QoSs arrays
 *  @param topicFilters - array of topic filter names
 *  @param QoSs - array of MQTT_QOS
 *  @param messageHandler - messageArrived callback function
 *  @return MQTT return code
 */
int MQTT_SubscribeWithId(MQTT_Handle handle, unsigned short id, int count, const char* topicFilters[], int QoSs[], MQTT_MessageHandler messageHandler);

/** MQTT Unsubscribe - send an MQTT unsubscribe packet and wait for unsuback before returning.
 *  @param handle - the client handle
 *  @param id - packetid
 *  @param count - number of members in the topicFilters array
 *  @param topicFilters - array of topic filter names
 *  @return MQTT return code
 */
int MQTT_UnsubscribeWithId(MQTT_Handle handle, unsigned short id, int count, const char* topicFilters[]);

// 异步模式，并且MQTT配置为消息缓存模式下使用
void MQTT_QueryCacheMessage(MQTT_Handle handle, uint32_t *count, uint32_t *totalLen);
// 异步模式，并且MQTT配置为消息缓存模式下使用
int MQTT_ReadCacheMessage(MQTT_Handle handle, uint32_t count, MQTT_MessageData* data[]);
// 异步模式，并且MQTT配置为消息缓存模式下使用
void MQTT_FreeCacheMessage(MQTT_Handle handle, uint32_t count);
//   MQTT  全局互斥锁初始化，在使用MQTT功能前初始化，只需要初始化一次
void MQTT_WorkMutexInit(void);

#endif
