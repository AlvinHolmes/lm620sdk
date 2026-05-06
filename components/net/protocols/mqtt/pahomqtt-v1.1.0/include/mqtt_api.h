/**
 *************************************************************************************
 * 版权所有 (C) 2025, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        mqtt_api.h
 *
 * @brief       MQTT接口.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2025-01-20     ICT Team         创建
 ************************************************************************************
 */

#if !defined(MQTT_API_H)
#define MQTT_API_H

#include <stdint.h>
#include <os.h>

/**
 * MQTT return code
 */
#define MQTT_OK                             0    // 操作成功
#define MQTT_ERR                           -1    // 未知错误
#define MQTT_ERR_BUSY                      -2    // 设备忙
#define MQTT_ERR_TIMEOUT                   -3    // 操作超时
#define MQTT_ERR_UNSUPPORTED               -4    // 不支持的操作
#define MQTT_ERR_PARAMETER                 -5    // 参数错误
#define MQTT_ERR_CONN_EXIST                -6    // 连接已存在
#define MQTT_ERR_UNCONN                    -7    // 未连接
#define MQTT_ERR_FULL                      -8    // 满状态
#define MQTT_ERR_EMPTY                     -9    // 空状态
#define MQTT_ERR_RESOURCE                  -10   // 未申请到资源
#define MQTT_ERR_NET_CONNECT               -11   // 网络连接错误
#define MQTT_ERR_INVALID_HANDLE            -12   // 句柄无效

#define MQTT_WILLOPTIONS_INITIALIZER        { {'M', 'Q', 'T', 'W'}, 0, {NULL, {0, NULL}}, {NULL, {0, NULL}}, 0, 0 }

#define MQTT_CONNECTDATA_INITIALIZER        { {'M', 'Q', 'T', 'C'}, 0, 4, {NULL, {0, NULL}}, 120, 1, 0, \
            MQTT_WILLOPTIONS_INITIALIZER, {NULL, {0, NULL}}, {NULL, {0, NULL}}, NULL, 0, NULL }

typedef enum
{
    MQTT_QOS0,
    MQTT_QOS1,
    MQTT_QOS2,
    MQTT_SUBFAIL = 0x80,
} MQTT_QOS;

typedef struct
{
    MQTT_QOS qos;
    unsigned char retained;
    unsigned char dup;
    unsigned short id;
    void *payload;
    size_t payloadlen;
} MQTT_Message;

typedef struct
{
    int len;
    char* data;
} MQTT_LenString;

typedef struct
{
    char* cstring;
    MQTT_LenString lenstring;
} MQTT_String;

typedef struct
{
    MQTT_Message *message;
    MQTT_String *topicName;
} MQTT_MessageData;

/**
 * A handle representing an MQTT client. A valid client handle is available
 * following a successful call to MQTT_CreateClient().
 */
typedef int* MQTT_Handle; // 定义成void* 会导致指针类型不匹配时编译器不报错，所以改成int*

typedef struct
{
    int reconnEnable;       // 重连功能开关。0：关闭；1：打开。如果不使用MQTTNetworkConnect函数创建的连接, 需要关闭重连功能
} MQTTClient_ConfigureParams;

typedef void (*MQTT_MessageHandler)(MQTT_Handle, MQTT_MessageData *);
typedef void (*MQTT_ConnectionLost)(MQTT_Handle, void *);

typedef enum
{
    MQTT_NOTIFY_EVENT_CONNECT = 0, /* MQTT协议连接成功 */
    MQTT_NOTIFY_EVENT_LOST_KEEPALIVE, /* MQTT协议心跳丢失 */
    MQTT_NOTIFY_EVENT_CLOSE_SESSION, /* MQTT协议会话关闭 */
} MQTT_NotifyEventId;
//  eventId定义，参考MQTT_NotifyEventId
typedef void (*MQTT_EventNotify_t)(MQTT_Handle handle, uint32_t eventId, void* eventParam);

/**
 * Defines the MQTT "Last Will and Testament" (LWT) settings for
 * the connect packet.
 */
typedef struct
{
    /** The eyecatcher for this structure.  must be MQTW. */
    char struct_id[4];
    /** The version number of this structure.  Must be 0 */
    int struct_version;
    /** The LWT topic to which the LWT message will be published. */
    MQTT_String topicName;
    /** The LWT payload. */
    MQTT_String message;
    /**
      * The retained flag for the LWT message.
      */
    unsigned char retained;
    /**
      * The quality of service setting for the LWT message.
      */
    char qos;
} MQTT_WillOptions;

typedef struct
{
    /** The eyecatcher for this structure.  must be MQTC. */
    char struct_id[4];
    /** The version number of this structure.  Must be 0 */
    int struct_version;
    /** Version of MQTT to be used.  3 = 3.1 4 = 3.1.1
      */
    unsigned char MQTTVersion;
    MQTT_String clientID;
    unsigned short keepAliveInterval;
    unsigned char cleansession;
    unsigned char willFlag;
    MQTT_WillOptions will;
    MQTT_String username;
    MQTT_String password;
    const char *host;
    uint16_t port;
    const char *ca_crt;
} MQTT_ConnectOption;

typedef struct Network Network;

struct Network
{
    /* if only use tcp */
	/* int my_socket;
	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
    int (*connect) (Network*); */

    const char *pHostAddress;
    uint16_t port;
    uint16_t ca_crt_len;
    /* NULL: TCP connection, not NULL: SSL connection */
    const char *ca_crt;
    /* Using PSK */
    const uint8_t *psk_identity;
    uint16_t psk_identity_len;
    uint16_t psk_key_len;
    const uint8_t *psk_key;
    int fd;
    uint32_t pdpCid;
    /* connection handle:(uintptr_t)(-1) not connection */
    uintptr_t handle;
    /* function pointer of recv mqtt data */
    int (*mqttread)(Network *, unsigned char *, int, int);
    /* function pointer of send mqtt data */
    int (*mqttwrite)(Network *, unsigned char *, int, int);
    /* function pointer of disconnect mqtt network */
    int (*disconnect)(Network *);
    /* function pointer of establish mqtt network */
    int (*connect)(Network *);
};


/**
 * Create an MQTT client handle. Only use MQTT_DestroyClient to destroy
 * an MQTT client handle created by MQTT_CreateClient.
 * @param handle - the client handle. The handle is a valid client reference
 * following a successful return from this function.
 * @param commandTimeoutMs
 * @param sendbuf - sendbuf pointer.
 * @param sendbufSize - sendbuf size.
 * @param readbuf - readbuf pointer.
 * @param readbufSize - readbuf size.
 * @param connLostHandler - connection lost callback function.You can set this to NULL if your application doesn't handle disconnections.
 * @return MQTT return code
 */
int MQTT_CreateClient(MQTT_Handle* handle, unsigned int commandTimeoutMs,
    unsigned char* sendbuf, size_t sendbufSize, unsigned char* readbuf, size_t readbufSize, MQTT_ConnectionLost connLostHandler);
/**
 * 获取client的配置参数，需要在创建client后才能调用
 * @param[in] handle - the client handle. The handle is a valid client reference
 * @param[out] params - 当前client配置的参数
 */
int MQTT_ClientGetConfigure(MQTT_Handle handle, MQTTClient_ConfigureParams *params);
/**
 * 配置client的参数，需要在创建client后才能调用
 * @param[in] handle - the client handle. The handle is a valid client reference
 * @param[in] params - 需要配置的参数
 * 先用 MQTT_ClientGetConfigure获取当前配置，再修改对应的值后，用MQTT_ClientSetConfigure配置新的参数
 */
int MQTT_ClientSetConfigure(MQTT_Handle handle, MQTTClient_ConfigureParams params);

/** MQTT DestroyClient - This function frees the memory allocated to an MQTT client(see MQTT_CreateClient).
 *  It should be called when the client is no longer required.
 *  @param handle - the client handle. The handle is invalid reference following
 *  a successfulreturn from this function.
 *  @return void
 */
void MQTT_DestroyClient(MQTT_Handle* handle);

/** MQTT Connect - send an MQTT connect packet down the network and wait for a Connack
 *  @param handle - the client handle
 *  @param options - connect options (default options see MQTT_CONNECTDATA_INITIALIZER)
 *  @return MQTT return code
 */
int MQTT_Connect(MQTT_Handle handle, MQTT_ConnectOption* options);

/** MQTT Disconnect - send an MQTT disconnect packet and close the connection
 *  @param handle - the client handle
 *  @return MQTT return code
 */
int MQTT_Disconnect(MQTT_Handle handle);

/** MQTT isConnected
 *  @param handle - the client handle
 *  @return OS_TRUE if the client is connected to the server, otherwise OS_FALSE
 */
int MQTT_IsConnected(MQTT_Handle handle);

/** MQTT Subscribe - send an MQTT subscribe packet and wait for suback before returning.
 *  @param handle - the client handle
 *  @param count - number of members in the topicFilters and QoSs arrays
 *  @param topicFilters - array of topic filter names
 *  @param QoSs - array of MQTT_QOS
 *  @param messageHandler - messageArrived callback function
 *  @return MQTT return code
 */
int MQTT_Subscribe(MQTT_Handle handle, int count, const char* topicFilters[], int QoSs[], MQTT_MessageHandler messageHandler);

/** MQTT Unsubscribe - send an MQTT unsubscribe packet and wait for unsuback before returning.
 *  @param handle - the client handle
 *  @param count - number of members in the topicFilters array
 *  @param topicFilters - array of topic filter names
 *  @return MQTT return code
 */
int MQTT_Unsubscribe(MQTT_Handle handle, int count, const char* topicFilters[]);

/** MQTT Publish - send an MQTT publish packet and wait for all acks to complete for all QoSs
 *  @param handle - the client handle
 *  @param topic - the topic to publish to
 *  @param message - the message to send
 *  @return MQTT return code
 */
int MQTT_Publish(MQTT_Handle handle, const char* topicName, MQTT_Message* message);

/** MQTT isBusy
 *  @param handle - the client handle
 *  @return OS_TRUE if the client is busy to proc mqtt req, otherwise OS_FALSE
 */
int MQTT_IsBusy(MQTT_Handle handle);

/** MQTT InitNetwork - initialize network
 *  @param handle - the client handle
 *  @param Network - the network
 *  @return MQTT_OK if init success, otherwise init fail
 */
int MQTT_SetNetwork(MQTT_Handle handle, Network *network);

/** MQTT Remove client Network - delete network
 *  @param handle - the client handle
 *  @return Network* pointer to current using network
 */
Network* MQTT_RemoveNetwork(MQTT_Handle handle);

/**
 * 配置client的事件通知回调函数，需要在创建client后才能调用
 * @param[in] handle - the client handle. The handle is a valid client reference
 * @param[in] notifyHandler - 事件通知回调函数
 *
 */
int MQTT_ClientSetEventNotify(MQTT_Handle handle, MQTT_EventNotify_t notifyHandler);

/**
 * 配置client的主题默认处理函数，需要在创建client后才能调用
 * 在匹配不到主题处理函数时调用该函数
 * @param[in] handle - the client handle. The handle is a valid client reference
 * @param[in] defaultHandler - 主题处理默认回调函数
 *
 */
int MQTT_ClientSetDefaultMessageHandler(MQTT_Handle handle, MQTT_MessageHandler defaultHandler);

#endif /* End of MQTT_API_H */
