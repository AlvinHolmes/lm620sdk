/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file
*
* @brief  MQTTs 订阅和断开重连示例
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/

#ifndef MQTT_USING_TLS // 这个DEMO展示外部创建TLS连接，跟MQTT自带的TLS连接不可共存
#include "lwip/sockets.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mqtt_api.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

/************************************************************************************
*                                 宏定义
************************************************************************************/
#define DEMO_MQTTS_USE_SLOG                    (0) // 0:使用osPrintf; 1:使用slogPrintf
#if DEMO_MQTTS_USE_SLOG
#define DEMO_MQTTS_PRINTF_INFO(format, ...)    slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_TEST, format, ##__VA_ARGS__)
#define DEMO_MQTTS_PRINTF_ERROR(format, ...)  slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_TEST, format, ##__VA_ARGS__)
#else
#define DEMO_MQTTS_PRINTF_INFO(format, ...)    osPrintf(format, ##__VA_ARGS__)
#define DEMO_MQTTS_PRINTF_ERROR(format, ...)  osPrintf(format, ##__VA_ARGS__)
#endif

#define DEMO_MQTTS_TASK_STACK_SIZE               (1024 * 4)
#define DEMO_MQTTS_TASK_PRIO                     (osPriorityLow3)
#define DEMO_MQTTS_COMMAND_TIMEOUT               (3000)
#define MAX_EVENT_COUNT 10

#define DEMO_MQTTS_SERVER_ADDR                   "broker.emqx.io"
#define DEMO_MQTTS_SERVER_PORT                   (8883)
#define BROKER_EMQX_IO_SERVER_CA                                       \
"-----BEGIN CERTIFICATE-----\r\n"                                      \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\r\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\r\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\r\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\r\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\r\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\r\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\r\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\r\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\r\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\r\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\r\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\r\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\r\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\r\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\r\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\r\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\r\n"                 \
"-----END CERTIFICATE-----\r\n"

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct
{
    mbedtls_ssl_context ssl_ctx;        /**< mbedtls ssl context */
    mbedtls_net_context net_ctx;        /**< fill in socket id   */
    mbedtls_ssl_config ssl_conf;        /**< entropy context     */
    mbedtls_entropy_context entropy;    /**< ssl configuration   */
    mbedtls_ctr_drbg_context ctr_drbg;  /**< ctr drbg context    */
    mbedtls_x509_crt_profile profile;   /**< x509 cacert profile */
    mbedtls_x509_crt cacert;            /**< x509 cacert         */
    mbedtls_x509_crt clicert;           /**< x509 client cacert  */
    mbedtls_pk_context pkey;            /**< pkey context        */
} DEMO_MQTTsClientSSL;

typedef struct
{
    uint32_t eventID;
    uint32_t dataLen;
    void* data;
} MQTT_DEMO_Event;

typedef enum
{
    MQTT_DEMO_EVENT_START_CONNECT = 0,
    MQTT_DEMO_EVENT_CONNECTED,
    MQTT_DEMO_EVENT_RECV,
    MQTT_DEMO_EVENT_KEEPALIVE_LOST,
    MQTT_DEMO_EVENT_SESSION_CLOSED,
} MQTT_DEMO_EventId;


typedef struct
{
    char *topic;
    uint32_t topicLen;
    uint8_t *data;
    uint32_t dataLen;
} MQTT_DEMO_TopicData;

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/

//线程句柄
static struct osThread *g_MQTTs_Thread = NULL;
//消息队列名柄
static osMessageQueueId_t g_MQTTs_QueueId = NULL;

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
//  创建队列
static osMessageQueueId_t DEMO_MQTT_MessageQueue_Create(uint32_t messageCount)
{
    osMessageQueueAttr_t msgQueueAttr = {"MQTTAPP", 0U, NULL, 0U, NULL, 0U};

    return osMessageQueueNew(messageCount, sizeof(MQTT_DEMO_Event), &msgQueueAttr);
}

//  删除队列
static osStatus_t DEMO_MQTT_MessageQueue_Delete(osMessageQueueId_t queue)
{
    return osMessageQueueDelete(queue);
}

//  从队列收消息
static osStatus_t DEMO_MQTT_MessageQueue_Recv(osMessageQueueId_t queue, MQTT_DEMO_Event *event)
{
    return osMessageQueueGet(queue, event, 0, osWaitForever);
}

//  向队列发送消息
static osStatus_t DEMO_MQTT_MessageQueue_Send(osMessageQueueId_t queue, MQTT_DEMO_Event event)
{
    return osMessageQueuePut(queue, &event, 0, 0);
}


/* 关闭 SSL 连接 */
static int DEMO_MQTTs_SSLClose(DEMO_MQTTsClientSSL *ssl)
{
    if(NULL == ssl)
    {
        DEMO_MQTTS_PRINTF_ERROR("%s, ssl is null\r\n", __FUNCTION__);
        return osError;
    }

    mbedtls_ssl_close_notify(&ssl->ssl_ctx);
    mbedtls_net_free(&ssl->net_ctx);
#ifdef MBEDTLS_X509_CRT_PARSE_C
    mbedtls_x509_crt_free(&ssl->cacert);
    mbedtls_x509_crt_free(&ssl->clicert);
#endif
    mbedtls_pk_free(&ssl->pkey);
    mbedtls_ssl_free(&ssl->ssl_ctx);
    mbedtls_ssl_config_free(&ssl->ssl_conf);
#ifdef MBEDTLS_ENTROPY_C
    mbedtls_ctr_drbg_free(&ssl->ctr_drbg);
    mbedtls_entropy_free(&ssl->entropy);
#endif
    DEMO_MQTTS_PRINTF_INFO("demo mqtts ssl close success\r\n");

    return osOK;
}

//SSL连接
static int DEMO_MQTTs_SSLConnect(DEMO_MQTTsClientSSL *ssl, const char *hostname, const uint16_t serverport)
{
#ifdef MBEDTLS_ENTROPY_C
    const char *pers = "mqtts_demo";
#endif
    #if 0
    /*
     * 无认证,双方都不认证证书，仅协商密钥
     * 使用时 authmode 配置成 MBEDTLS_SSL_VERIFY_NONE
     * server_cert, client_cert, client_pk 配置成NULL
     * server_cert_len, client_cert_len, client_pk_len 配置成 0
    */
    int authmode = MBEDTLS_SSL_VERIFY_NONE;
    //  服务器端证书，单向或双向认证时使用
    const char *server_cert = NULL;        /* 服务器端证书 */
    int server_cert_len = 0;               /* 服务器端证书buffer大小，字符串需要算上字符串结束符'\0' */
    //  客户端证书，双向认证时使用
    const char *client_cert = NULL;        /* 客户端证书 */
    int client_cert_len = 0;               /* 客户端证书buffer大小，字符串需要算上字符串结束符'\0' */
    //  客户端KEY，双向认证时使用
    const char *client_pk = NULL;          /* 客户端key */
    int client_pk_len = 0;                 /* 客户端key buffer大小 */
    #endif
    #if 1
    /*
     * 单向认证，客户端认证服务器的证书，并协商密钥
     * 使用时 authmode 配置成 MBEDTLS_SSL_VERIFY_OPTIONAL
     * server_cert 赋值正确的服务器端证书， server_cert_len 赋值证书长度
     * client_cert, client_pk 配置成NULL
     * client_cert_len, client_pk_len 配置成 0
    */
    int authmode = MBEDTLS_SSL_VERIFY_OPTIONAL;
    //  服务器端证书，单向或双向认证时使用
    const char *server_cert = BROKER_EMQX_IO_SERVER_CA;        /* 服务器端证书 */
    int server_cert_len = strlen(BROKER_EMQX_IO_SERVER_CA)+1;  /* 服务器端证书buffer大小，字符串需要算上字符串结束符'\0' */
    //  客户端证书，双向认证时使用
    const char *client_cert = NULL;        /* 客户端证书 */
    int client_cert_len = 0;               /* 客户端证书buffer大小，字符串需要算上字符串结束符'\0' */
    //  客户端KEY，双向认证时使用
    const char *client_pk = NULL;          /* 客户端key */
    int client_pk_len = 0;                 /* 客户端key buffer大小 */
    #endif

    #if 0
    /*
     * 双向认证，客户端和服务器端双方都认证对端证书，并协商密钥
     * 使用时 authmode 配置成 MBEDTLS_SSL_VERIFY_REQUIRED
     * server_cert 赋值正确的服务器端证书， server_cert_len 赋值证书长度
     * client_cert 赋值正确的客户端证书， client_cert_len 赋值证书长度
     * client_pk 赋值正确的客户端密钥， client_pk_len 赋值密钥长度
    */
    int authmode = MBEDTLS_SSL_VERIFY_REQUIRED;
    //  服务器端证书，单向或双向认证时使用
    const char *server_cert = NULL;        /* 服务器端证书 */
    int server_cert_len = 0;  /* 服务器端证书buffer大小，字符串需要算上字符串结束符'\0' */
    //  客户端证书，双向认证时使用
    const char *client_cert = NULL;        /* 客户端证书 */
    int client_cert_len = 0;               /* 客户端证书buffer大小，字符串需要算上字符串结束符'\0' */
    //  客户端KEY，双向认证时使用
    const char *client_pk = NULL;          /* 客户端key */
    int client_pk_len = 0;                 /* 客户端key buffer大小 */
    #error "No Example"
    #endif

    int value = 0;
    int ret = 0;
    uint32_t flags;
    char strport[10] = {0};

    DEMO_MQTTS_PRINTF_INFO("authmode %u \r\n", authmode);

    mbedtls_net_init(&ssl->net_ctx);
    mbedtls_ssl_init(&ssl->ssl_ctx);
    mbedtls_ssl_config_init(&ssl->ssl_conf);
#ifdef MBEDTLS_X509_CRT_PARSE_C
    mbedtls_x509_crt_init(&ssl->cacert);
    mbedtls_x509_crt_init(&ssl->clicert);
#endif
    mbedtls_pk_init(&ssl->pkey);
#ifdef MBEDTLS_ENTROPY_C
    mbedtls_ctr_drbg_init(&ssl->ctr_drbg);
    mbedtls_entropy_init(&ssl->entropy);
    if ((value = mbedtls_ctr_drbg_seed(&ssl->ctr_drbg,
                               mbedtls_entropy_func,
                               &ssl->entropy,
                               (const unsigned char*)pers,
                               strlen(pers))) != 0) {
        DEMO_MQTTS_PRINTF_ERROR("mbedtls_ctr_drbg_seed() failed, value:-0x%x\r\n", -value);
        ret = -1;
        goto exit;
    }
#endif
    /*
    * Load the Client certificate
    */
    if (client_cert && client_pk) {
#ifdef MBEDTLS_X509_CRT_PARSE_C
        ret = mbedtls_x509_crt_parse(&ssl->clicert, (const unsigned char *)client_cert, client_cert_len);
        if (ret < 0) {
            DEMO_MQTTS_PRINTF_ERROR("Loading cli_cert failed! mbedtls_x509_crt_parse returned -0x%x\r\n", -ret);
            ret = -1;
            goto exit;
        }
#endif
        ret = mbedtls_pk_parse_key(&ssl->pkey, (const unsigned char *)client_pk, client_pk_len, NULL, 0);
        if (ret != 0) {
            DEMO_MQTTS_PRINTF_ERROR("failed! mbedtls_pk_parse_key returned -0x%x\r\n", -ret);
            ret = -1;
            goto exit;
        }
    }

    /*
    * Load the trusted CA
    */
    /* cert_len passed in is gotten from sizeof not strlen */
#ifdef MBEDTLS_X509_CRT_PARSE_C
    if (server_cert && ((value = mbedtls_x509_crt_parse(&ssl->cacert,
                                        (const unsigned char *)server_cert,
                                        server_cert_len)) < 0)) {
        /* when the type of certificate is PEM, the server_cert need end with \0', so server_cert_len need +1  */
        DEMO_MQTTS_PRINTF_ERROR("mbedtls_x509_crt_parse() server failed, value:-0x%x\r\n", -value);
        ret = -1;
        goto exit;
    }
#endif

    /*
     * Start the connection
     */
    itoa(serverport, strport, 10);
    if ((ret = mbedtls_net_connect(&ssl->net_ctx, hostname, strport, MBEDTLS_NET_PROTO_TCP)) != 0) {
        DEMO_MQTTS_PRINTF_ERROR("failed! mbedtls_net_connect returned %d, port:%s\r\n", ret, strport);
        ret = -1;
        goto exit;
    }

    /*
     * Setup stuff
     */
    if ((value = mbedtls_ssl_config_defaults(&ssl->ssl_conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        DEMO_MQTTS_PRINTF_ERROR("mbedtls_ssl_config_defaults() failed, value:-0x%x\r\n", -value);
        ret = -1;
        goto exit;
    }

#ifdef MBEDTLS_X509_CRT_PARSE_C
    /*
    memcpy(&ssl->profile, ssl->ssl_conf.cert_profile, sizeof(mbedtls_x509_crt_profile));
    ssl->profile.allowed_mds = ssl->profile.allowed_mds | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_MD5);
    mbedtls_ssl_conf_cert_profile(&ssl->ssl_conf, &ssl->profile);
    */

    mbedtls_ssl_conf_authmode(&ssl->ssl_conf, authmode);
    mbedtls_ssl_conf_ca_chain(&ssl->ssl_conf, &ssl->cacert, NULL);

    if (client_cert && (ret = mbedtls_ssl_conf_own_cert(&ssl->ssl_conf, &ssl->clicert, &ssl->pkey)) != 0) {
        DEMO_MQTTS_PRINTF_ERROR(" failed! mbedtls_ssl_conf_own_cert returned %d\r\n", ret );
        ret = -1;
        goto exit;
    }
#endif

    mbedtls_ssl_conf_rng(&ssl->ssl_conf, mbedtls_ctr_drbg_random, &ssl->ctr_drbg);
    mbedtls_ssl_conf_dbg(&ssl->ssl_conf, NULL, NULL);

    if ((value = mbedtls_ssl_setup(&ssl->ssl_ctx, &ssl->ssl_conf)) != 0) {
        DEMO_MQTTS_PRINTF_ERROR("mbedtls_ssl_setup() failed, value:-0x%x\r\n", -value);
        ret = -1;
        goto exit;
    }

    if ((value = mbedtls_ssl_set_hostname(&ssl->ssl_ctx, hostname)) != 0) {
        DEMO_MQTTS_PRINTF_ERROR("mbedtls_ssl_set_hostname() failed, value:-0x%x\r\n", -value);
        ret = -1;
        goto exit;
    }

    mbedtls_ssl_set_bio(&ssl->ssl_ctx, &ssl->net_ctx, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
    mbedtls_ssl_conf_read_timeout(&ssl->ssl_conf, 10000);

    /*
    * Handshake
    */
    while ((ret = mbedtls_ssl_handshake(&ssl->ssl_ctx)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            DEMO_MQTTS_PRINTF_ERROR("mbedtls_ssl_handshake() failed, ret:-0x%x\r\n", -ret);
            ret = -1;
            goto exit;
        }
    }

    /*
     * Verify the server certificate
     */
    if ((flags = mbedtls_ssl_get_verify_result(&ssl->ssl_ctx)) != 0) {
        char vrfy_buf[100];
        DEMO_MQTTS_PRINTF_ERROR("svr_cert varification failed.\r\n");
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
        DEMO_MQTTS_PRINTF_INFO("svr_cert varification failed, [%s]\r\n", vrfy_buf);
        ret = -1;
    } else {
        DEMO_MQTTS_PRINTF_INFO("svr_cert varification ok\r\n");
    }

exit:
    if (ret != 0) {
        DEMO_MQTTS_PRINTF_INFO("%s failed, ret=%d\r\n", __FUNCTION__, ret);
        DEMO_MQTTs_SSLClose(ssl);
    }

    return ret;
}

static void DEMO_MQTTs_SubscribeCallback(MQTT_Handle handle, MQTT_MessageData *data)
{
    osStatus_t status = osStatusReserved;
    //打印消息内容
    DEMO_MQTTS_PRINTF_INFO("MQTTs arrived topic [%u][%.*s]\r\n",
                            data->topicName->lenstring.len, data->topicName->lenstring.len, data->topicName->lenstring.data);
    DEMO_MQTTS_PRINTF_INFO("MQTTs arrived data [%u][%.*s]\r\n",
                            data->message->payloadlen, data->message->payloadlen, data->message->payload);

    MQTT_DEMO_Event sendevent = {0};
    // memory layout |struct TopicData| topic memory | topic data memory |
    uint32_t event_dataLen = sizeof(MQTT_DEMO_TopicData) +  (data->topicName->lenstring.len + 1) + (data->message->payloadlen);
    MQTT_DEMO_TopicData *topicdata = NULL;

    sendevent.eventID = MQTT_DEMO_EVENT_RECV;

    sendevent.data = osCalloc(1, event_dataLen);
    if (sendevent.data != NULL)
    {
        topicdata = sendevent.data;
        // copy topic
        topicdata->topic = (char *)(topicdata+1);
        memcpy(topicdata->topic, data->topicName->lenstring.data, data->topicName->lenstring.len);
        topicdata->topicLen = data->topicName->lenstring.len;
        // copy topic data
        topicdata->data = (uint8_t *)topicdata->topic + topicdata->topicLen + 1;
        topicdata->dataLen = data->message->payloadlen;
        osMemcpy(topicdata->data, data->message->payload, data->message->payloadlen);
    }

    status = DEMO_MQTT_MessageQueue_Send(g_MQTTs_QueueId, sendevent);
    DEMO_MQTTS_PRINTF_INFO("Message arrived Event Send[%d]\r\n", status);
    if (status != osOK && sendevent.data != NULL)
    {
        osFree(sendevent.data);
    }

    return;
}

//默认主题订阅回调函数
static void defaultMessageArrived(MQTT_Handle handle, MQTT_MessageData *data)
{
    DEMO_MQTTS_PRINTF_INFO("MQTTs default arrived topic [%u][%.*s]\r\n",
                            data->topicName->lenstring.len, data->topicName->lenstring.len, data->topicName->lenstring.data);
    DEMO_MQTTS_PRINTF_INFO("MQTTs default arrived data [%u][%.*s]\r\n",
                            data->message->payloadlen, data->message->payloadlen, data->message->payload);
}

static int DEMO_MQTTs_SSLRead(Network *pNetwork, unsigned char *buffer, int len, int timeoutMs)
{
    int32_t ret = 0;
    int32_t recvLen = 0;

    if (NULL == pNetwork || NULL == buffer || 0 == len)
    {
        DEMO_MQTTS_PRINTF_ERROR("%s,  network/buffer/len is null\r\n", __FUNCTION__);
        return 0;
    }

    if ((uintptr_t)(-1) == pNetwork->handle)
    {
        DEMO_MQTTS_PRINTF_ERROR("%s, handle is null\r\n", __FUNCTION__);
        return 0;
    }

    DEMO_MQTTsClientSSL *ssl = (DEMO_MQTTsClientSSL *)pNetwork->handle;
    mbedtls_ssl_context *ssl_ctx = &ssl->ssl_ctx;
    if (NULL ==ssl_ctx)
    {
        DEMO_MQTTS_PRINTF_ERROR("%s, ssl_ctx is null\r\n", __FUNCTION__);
        return 0;
    }

    if(0 == timeoutMs) /* timeoutMs 为0表示非阻塞 */
    {
        timeoutMs = 10; /* 由于0会导致select接口一直等，所以这里置超时时间为 10 毫秒 */
    }

    do
    {
        mbedtls_ssl_conf_read_timeout((mbedtls_ssl_config *)ssl_ctx->conf, timeoutMs);
        ret = mbedtls_ssl_read(ssl_ctx, (unsigned char *)(buffer + recvLen), len - recvLen);

        if (ret >= 0)
        {
            recvLen = ret;
            break;
        }
        else
        {
            DEMO_MQTTS_PRINTF_INFO("mbedtls_ssl_read returned -0x%x\r\n", (unsigned int) -ret);
            if (MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == ret)
            {
                recvLen = 0;
                DEMO_MQTTS_PRINTF_INFO("ssl connection was closed gracefully\r\n");
                break;
            }
            else if ((MBEDTLS_ERR_SSL_TIMEOUT == ret) || (MBEDTLS_ERR_SSL_CONN_EOF == ret)
                || (MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED == ret) || (MBEDTLS_ERR_SSL_NON_FATAL == ret))
            {
                /* read already complete */
                /* if call mbedtls_ssl_read again, it will return 0 (means EOF) */
                recvLen = 0;
                break;
            }
            else if (MBEDTLS_ERR_SSL_WANT_READ == ret || MBEDTLS_ERR_SSL_WANT_WRITE == ret)
            {
                continue;
            }
            else
            {
                recvLen = 0;
                break;
            }
        }
    } while (1);

    return recvLen;
}

static int DEMO_MQTTs_SSLWrite(Network *pNetwork, unsigned char *buffer, int len, int timeoutMs)
{
    int32_t sendLen = 0;
    int32_t ret = -1;

    if (NULL == pNetwork || NULL == buffer || 0 == len)
    {
        DEMO_MQTTS_PRINTF_ERROR("%s,  network/buffer/len is null\r\n", __FUNCTION__);
        return osError;
    }

    if ((uintptr_t)(-1) == pNetwork->handle)
    {
        DEMO_MQTTS_PRINTF_ERROR("%s, handle is null\r\n", __FUNCTION__);
        return osErrorTimeout;
    }

    DEMO_MQTTsClientSSL *ssl = (DEMO_MQTTsClientSSL *)pNetwork->handle;
    mbedtls_ssl_context *ssl_ctx = &ssl->ssl_ctx;
    if (NULL ==ssl_ctx)
    {
        DEMO_MQTTS_PRINTF_ERROR("%s, ssl_ctx is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    do
    {
        DEMO_MQTTS_PRINTF_ERROR("%s,%d\r\n", __FUNCTION__, __LINE__);
        ret = mbedtls_ssl_write(ssl_ctx, (unsigned char *)(buffer + sendLen), len - sendLen);

        if (ret > 0)
        {
            sendLen += ret;
        }
        else if (ret == 0)
        {
            break;
        }
        else if (ret < 0)
        {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                DEMO_MQTTS_PRINTF_ERROR("failed!mbedtls_ssl_write returned -0x%x\r\n", -ret);
                sendLen = ret;
                break;
            }
        }
    } while (sendLen < len);

    if (sendLen >= 0)
    {
        DEMO_MQTTS_PRINTF_INFO("ssl_write %d bytes written\r\n", sendLen);
    }

    return sendLen;
}

static int DEMO_MQTTs_Disconnect(Network *pNetwork)
{
    if (NULL == pNetwork)
    {
        DEMO_MQTTS_PRINTF_ERROR("%s,  network is null\r\n", __FUNCTION__);
        return -1;
    }

    if ((uintptr_t)(-1) == pNetwork->handle)
    {
        DEMO_MQTTS_PRINTF_ERROR("%s, handle is null,maybe disconnect already\r\n", __FUNCTION__);
        return -1;
    }

    DEMO_MQTTsClientSSL *ssl = (DEMO_MQTTsClientSSL *)pNetwork->handle;
    lwip_socket_unregister_callback(pNetwork->fd); /* 注销 LWIP 事件回调 */
    DEMO_MQTTs_SSLClose(ssl);
    //g_MqttsClientInfo->ssl = NULL;
    pNetwork->handle = (uintptr_t)(-1); /* must have */
    pNetwork->fd = -1;
    DEMO_MQTTS_PRINTF_INFO("demo mqtts disconnect success\r\n");

    return 0;
}

static Network* DEMO_MQTTs_NetworkInit(DEMO_MQTTsClientSSL *clientSsl)
{
    Network *pNetwork = osMalloc(sizeof(Network));
    if (NULL == pNetwork)
    {
        DEMO_MQTTS_PRINTF_INFO("demo mqtts network malloc failed\r\n");
        return NULL;
    }
    memset(pNetwork, 0x00, sizeof(Network));

    pNetwork->fd = clientSsl->net_ctx.fd;
    pNetwork->pdpCid = 0; /* 不指定 CID */
    pNetwork->handle = (uintptr_t)clientSsl;

    pNetwork->mqttread = DEMO_MQTTs_SSLRead;
    pNetwork->mqttwrite = DEMO_MQTTs_SSLWrite;
    pNetwork->disconnect = DEMO_MQTTs_Disconnect;
    pNetwork->connect = NULL;

    return pNetwork;
}

static void DEMO_MQTTs_EventNotifyFunc(MQTT_Handle handle, uint32_t eventId, void* eventParam)
{
    MQTT_DEMO_Event sendevent = {0};
    osStatus_t status = osStatusReserved;

    if (eventId == MQTT_NOTIFY_EVENT_CONNECT)
    {
        sendevent.eventID = MQTT_DEMO_EVENT_CONNECTED;

        status = DEMO_MQTT_MessageQueue_Send(g_MQTTs_QueueId, sendevent);
    }

    if (eventId == MQTT_NOTIFY_EVENT_LOST_KEEPALIVE)
    {
        sendevent.eventID = MQTT_DEMO_EVENT_KEEPALIVE_LOST;

        status = DEMO_MQTT_MessageQueue_Send(g_MQTTs_QueueId, sendevent);
    }

    if (eventId == MQTT_NOTIFY_EVENT_CLOSE_SESSION)
    {
        sendevent.eventID = MQTT_DEMO_EVENT_SESSION_CLOSED;

        status = DEMO_MQTT_MessageQueue_Send(g_MQTTs_QueueId, sendevent);
    }

    DEMO_MQTTS_PRINTF_INFO("MQTTs Notify Event[%u] Send[%d]\r\n", eventId, status);
}

static void DEMO_MQTTs_TaskEntry(void *param)
{
    int32_t ret = -1;
    int quit = 0;
    const char *topics[1] = { "/MQTT_TEST" };
    int QoSs[1] = { MQTT_QOS1 };
    uint32_t topicCnt = 1; /* 订阅的主题的个数 */
    Network *pNetwork = NULL;
    DEMO_MQTTsClientSSL *ssl = NULL;

    MQTT_Handle MQTTs_handle = NULL;
    MQTTClient_ConfigureParams configparam = {0};
    MQTT_ConnectOption connectData = MQTT_CONNECTDATA_INITIALIZER;
    connectData.MQTTVersion = 4; /*3 = 3.1 4 = 3.1.1*/
    connectData.keepAliveInterval = 60;
    connectData.cleansession = 0;
    connectData.clientID.cstring = "DEMO_MQTTS";
    connectData.username.cstring = "username";
    connectData.password.cstring = "password";
    connectData.willFlag = 1; //支持遗嘱消息
    connectData.will.qos = 1;
    connectData.will.retained = 0;
    connectData.will.topicName.cstring = "last/will";
    connectData.will.message.cstring = "Clientl disconnected unexpectedly.";
    connectData.host = DEMO_MQTTS_SERVER_ADDR;
    connectData.port = DEMO_MQTTS_SERVER_PORT;
    connectData.ca_crt = NULL;

    uint32_t sendbufLen = 512;
    uint32_t readbufLen = 512;
    int             rc = 0;
    unsigned char *sendbuf = osMalloc(sendbufLen);
    unsigned char *readbuf = osMalloc(readbufLen);
    osStatus_t sendstatus = osStatusReserved;
    MQTT_DEMO_Event sendevent = {0};

    OS_ASSERT(sendbuf != NULL && readbuf != NULL);
    if (sendbuf == NULL || readbuf == NULL)
    {
        if (sendbuf)
        {
            osFree(sendbuf);
        }
        if (readbuf)
        {
            osFree(readbuf);
        }

        return;
    }


    osMemset(sendbuf, 0, sendbufLen);
    osMemset(readbuf, 0, readbufLen);

    //创建MQTT客户端
    rc = MQTT_CreateClient(&MQTTs_handle,
                    DEMO_MQTTS_COMMAND_TIMEOUT,
                    sendbuf,
                    sendbufLen,
                    readbuf,
                    readbufLen,
                    NULL);
    if (0 != rc)
    {
         DEMO_MQTTS_PRINTF_ERROR("MQTTs create client failed, check param\r\n");
        goto exit;
    }
    DEMO_MQTTS_PRINTF_INFO("MQTTs create client sucess\r\n");
    MQTT_ClientSetEventNotify(MQTTs_handle, DEMO_MQTTs_EventNotifyFunc);
    MQTT_ClientSetDefaultMessageHandler(MQTTs_handle, defaultMessageArrived);
    MQTT_ClientGetConfigure(MQTTs_handle, &configparam);
    configparam.reconnEnable = 0;
    MQTT_ClientSetConfigure(MQTTs_handle, configparam);


    g_MQTTs_QueueId = DEMO_MQTT_MessageQueue_Create(MAX_EVENT_COUNT);

    if(g_MQTTs_QueueId == NULL)
    {
        goto exit;
    }
    else
    {
        DEMO_MQTTS_PRINTF_INFO("MQTTs create message queue sucess\r\n");
    }

    memset(&sendevent,0, sizeof(sendevent));
    do
    {
        sendevent.eventID = MQTT_DEMO_EVENT_START_CONNECT;

        sendstatus = DEMO_MQTT_MessageQueue_Send(g_MQTTs_QueueId, sendevent);
        DEMO_MQTTS_PRINTF_INFO("MQTTs send sendstatus %d\r\n", sendstatus);
    } while ( sendstatus != osOK);

    while(quit == 0)
    {
        MQTT_DEMO_Event event = {0};
        if(DEMO_MQTT_MessageQueue_Recv(g_MQTTs_QueueId, &event) == osOK )
        {
            //   从消息队列获取消息
            switch (event.eventID)
            {
                case MQTT_DEMO_EVENT_START_CONNECT :
                    if (ssl == NULL)
                    {
                        ssl = (DEMO_MQTTsClientSSL *)osMalloc(sizeof(DEMO_MQTTsClientSSL));
                        if (!ssl) {
                            DEMO_MQTTS_PRINTF_ERROR("MQTTs ssl memory osMalloc error\r\n");
                            goto exit;
                        }
                    }

                    ret = osOK;
                    do
                    {
                        if(ret != osOK)
                            osDelay(5000);
                        ret = DEMO_MQTTs_SSLConnect(ssl, DEMO_MQTTS_SERVER_ADDR, DEMO_MQTTS_SERVER_PORT);
                        DEMO_MQTTS_PRINTF_INFO("MQTTs connect is %d\r\n", ret);
                    } while(ret != osOK);
                    DEMO_MQTTS_PRINTF_INFO("MQTTs ssl connect ok, fd[%d]\r\n", ssl->net_ctx.fd);

                    pNetwork = DEMO_MQTTs_NetworkInit(ssl);
                    if (NULL == pNetwork)
                    {
                        DEMO_MQTTS_PRINTF_ERROR("MQTTs network init failed\r\n");
                        goto exit;
                    }
                    DEMO_MQTTS_PRINTF_INFO("MQTTs network init ok\r\n");

                    ret = MQTT_SetNetwork(MQTTs_handle, pNetwork);
                    if (MQTT_OK != ret)
                    {
                        DEMO_MQTTS_PRINTF_ERROR("MQTTs set network failed, ret[%d]\r\n", ret);
                        goto exit;
                    }
                    DEMO_MQTTS_PRINTF_ERROR("MQTTs set network ok\r\n");

                    rc = 0;
                    do
                    {
                        if(rc != 0)
                            osDelay(5000);
                        rc = MQTT_Connect(MQTTs_handle, &connectData);
                    } while(rc != 0);
                    DEMO_MQTTS_PRINTF_INFO("MQTTs Connected\r\n");
                    break;

                case MQTT_DEMO_EVENT_CONNECTED :
                    //SUBSCRIBE 订阅主题
                    if ((rc = MQTT_Subscribe(MQTTs_handle,  topicCnt, topics, QoSs, DEMO_MQTTs_SubscribeCallback)) != 0)
                    {
                        DEMO_MQTTS_PRINTF_INFO("MQTTs subscribe failed, return code from MQTT subscribe is %d\r\n", rc);
                        goto exit;
                    }
                    DEMO_MQTTS_PRINTF_INFO("MQTTs Subscribed\r\n");
                    break;

                case MQTT_DEMO_EVENT_RECV :
                    if (event.data != NULL)
                    {
                        MQTT_DEMO_TopicData *topicdata = event.data;
                        DEMO_MQTTS_PRINTF_INFO("MQTTs Event recv topic [%u][%.*s]\r\n", topicdata->topicLen, topicdata->topicLen, topicdata->topic);
                        DEMO_MQTTS_PRINTF_INFO("MQTTs Event recv data [%u][%.*s]\r\n", topicdata->dataLen, topicdata->dataLen, topicdata->data);
                        if(topicdata->dataLen >= 4 && memcmp(topicdata->data, "QUIT", 4) == 0) // 订阅到QUIT内容退出线程
                        {
                            quit = 1;
                            DEMO_MQTTS_PRINTF_INFO("MQTT Event Quit\r\n");
                        }
                        osFree(event.data);
                    }
                    else
                    {
                        DEMO_MQTTS_PRINTF_INFO("MQTTs Event recv data[NULL]\r\n");
                    }
                    break;

                case MQTT_DEMO_EVENT_KEEPALIVE_LOST :
                case MQTT_DEMO_EVENT_SESSION_CLOSED :
                    if (MQTT_IsConnected(MQTTs_handle) == OS_TRUE)
                    {
                        DEMO_MQTTS_PRINTF_ERROR("MQTTs disconnect proc already connected\r\n");
                        //MQTT_Disconnect(MQTTs_handle);
                    }
                    else
                    {
                        DEMO_MQTTS_PRINTF_ERROR("MQTTs disconnect proc\r\n");
                        MQTT_RemoveNetwork(MQTTs_handle);
                        if(NULL != pNetwork)
                        {
                            osFree(pNetwork);
                            pNetwork = NULL;
                        }
                        memset(&sendevent,0, sizeof(sendevent));
                        sendevent.eventID = MQTT_DEMO_EVENT_START_CONNECT;
                        DEMO_MQTT_MessageQueue_Send(g_MQTTs_QueueId, sendevent);
                    }
                    break;

                default:
                    break;

            }
        }
        else
        {
            DEMO_MQTTS_PRINTF_ERROR("MQTTs Queue Get Msg Err\r\n");
        }
    }


    ret = MQTT_Unsubscribe(MQTTs_handle, topicCnt, topics); /* 退订主题 */
    if (ret != 0)
    {
        DEMO_MQTTS_PRINTF_ERROR("MQTTs unsubscribe failed, ret[%d]\r\n", ret);
        goto exit;
    }
    DEMO_MQTTS_PRINTF_INFO("MQTTs unsubscribe ok\r\n");

exit:
    if(NULL != MQTTs_handle)
    {
        if (MQTT_IsConnected(MQTTs_handle) == OS_TRUE)
        {
            MQTT_Disconnect(MQTTs_handle); /* 断开 MQTT 连接 */
        }
        MQTT_RemoveNetwork(MQTTs_handle);
        MQTT_DestroyClient(&MQTTs_handle);
        MQTTs_handle = NULL;
    }

    if(NULL != sendbuf)
    {
        osFree(sendbuf);
        sendbuf = NULL;
    }
    if(NULL != readbuf)
    {
        osFree(readbuf);
        readbuf = NULL;
    }
    if(NULL != pNetwork)
    {
        osFree(pNetwork);
        pNetwork = NULL;
    }
    if(NULL != ssl)
    {
        osFree(ssl);
        ssl = NULL;
    }

    if (g_MQTTs_QueueId != NULL)
    {
        DEMO_MQTT_MessageQueue_Delete(g_MQTTs_QueueId);
        g_MQTTs_QueueId = NULL;
    }

    DEMO_MQTTS_PRINTF_INFO("MQTTs sample Reconn terminate.\r\n");
    g_MQTTs_Thread = NULL;
    osThreadTerminate(osThreadSelf());
    return;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
static void DEMO_MQTTs_Entry(char argc, char **argv)
{
    DEMO_MQTTS_PRINTF_INFO("%s start\r\n", __FUNCTION__);
    osThreadAttr_t attr = {"MQTTs_recon_demo", osThreadDetached, NULL, 0U, NULL, DEMO_MQTTS_TASK_STACK_SIZE, DEMO_MQTTS_TASK_PRIO, 0U, 0U};
    if(NULL != g_MQTTs_Thread)
    {
        DEMO_MQTTS_PRINTF_ERROR("MQTTs task is running\r\n");
        return;
    }
    g_MQTTs_Thread = osThreadNew(DEMO_MQTTs_TaskEntry, NULL, &attr);
    if(NULL == g_MQTTs_Thread)
    {
        DEMO_MQTTS_PRINTF_ERROR("MQTTs task create fail\r\n");
        return;
    }
    return;
}

#if 0
int main(void)
{
    DEMO_MQTTs_Entry(0, NULL);
}
#endif

//SHELL命令
#ifdef OS_USING_SHELL
#include "nr_micro_shell.h"
NR_SHELL_CMD_EXPORT(demo_mqtts_recon, DEMO_MQTTs_Entry); // "start mqtts demo"
#endif

#endif // #ifndef MQTT_USING_TLS

