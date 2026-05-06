/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief:OneNET OTA upgrade by http
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-28     ict team          创建
 ************************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "lwip/sockets.h"
#include "onenet_ota_pub.h"
#include "onenet_ota_api.h"
#include "onenet_ota_rsp.h"
#include "onenet_ota_http.h"


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_HTTP_ON
 #ifdef OTA_HTTP_ON
#define OTA_HTTP_DBG_PRINTF                         OTA_PUB_PRINT_DEBUG
#else
#define OTA_HTTP_DBG_PRINTF(...)
#endif
#define OTA_HTTP_ERR_PRINTF                         OTA_PUB_PRINT_ERROR

/**
** OTA HTTP 升级的 URL 格式为 http(s)://{api domian}/{namespace}/{url and parameters}
** api domian: 请求的通用API域名地址, 目前为http(s)://iot-api.heclouds.com/fuse-ota/
** namespace: 放置在URL中第一段, 用以区分API类别, 如namespace=device, 那么则代表设备管理大类
** url and parameters:由该项大类API自定义，包括后续url以及query参数
*/
#define OTA_HTTP_URL_API_DOMAIN                     "http://iot-api.heclouds.com/fuse-ota/"
//#define OTA_HTTP_URL_API_DOMAIN                   "https://iot-api.heclouds.com/fuse-ota/"
#define OTA_HTTP_URL_VERSION                        "version"
#define OTA_HTTP_URL_CHECK                          "check"
#define OTA_HTTP_URL_DOWNLOAD                       "download"
#define OTA_HTTP_URL_STATUS                         "status"
#define OTA_HTTP_AUTHORIZATION                      "authorization: "
#define OTA_HTTP_CONNECT_KEEPALIVE                  "connection: keep-alive"
#define OTA_HTTP_CONTENT_TYPE                       "application/json"

#define OTA_HTTP_RESPONSE_TIMEOUT                   (30)    /* OTA HTTP(S) 响应的最大超时时间, 单位: 秒 */
#define OTA_HTTP_RESPONSE_HEAD_LEN                  (1024)  /* OTA HTTP(S) 响应头部长度 */
#define OTA_HTTP_RESPONSE_BODY_LEN                  (1024 * 2) /* OTA HTTP(S) 响应体长度 */
#define OTA_HTTP_SSL_ID_DEFAULT                     (0)  /* SSL ID 的默认值为0 */

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum
{
    OTA_HTTP_URL_IS_HTTPS      = 0,   /* HTTPS */
    OTA_HTTP_URL_IS_HTTP       = 1,   /* HTTP */
}OOTA_HttpUrlIsHttp;        /* HTTP 还是 HTTPS */

typedef enum
{
    OTA_HTTP_RSP_CODE_OK                             = 200,  /* 正常响应OK */
    OTA_HTTP_RSP_CODE_CREATED                        = 201,  /* Created */
    OTA_HTTP_RSP_CODE_ACCEPTED                       = 202,  /* Accepted */
    OTA_HTTP_RSP_CODE_PARTIAL_CONTENT                = 206,  /* 断点续传成功 */
    OTA_HTTP_RSP_CODE_BAD_REQUEST                    = 400,  /* Bad Request */
    OTA_HTTP_RSP_CODE_UNAUTHORIZED                   = 401,  /* Unauthorized */
    OTA_HTTP_RSP_CODE_PAYMENT_REQUIRED               = 402,  /* Payment Required */
    OTA_HTTP_RSP_CODE_FORBIDDEN                      = 403,  /* Forbidden */
    OTA_HTTP_RSP_CODE_NOT_FOUND                      = 404,  /* Not found */
    OTA_HTTP_RSP_CODE_METHOD_NOT_ALLOWED             = 405,  /* Method Not Allowed */
    OTA_HTTP_RSP_CODE_NOT_ACCEPTABLE                 = 406,  /* Not Acceptable */
    OTA_HTTP_RSP_CODE_PROXY_AUTHENTICATION           = 407,  /* Proxy Autuentication */
    OTA_HTTP_RSP_CODE_REQUEST_TIMEOUT                = 408,  /* Request Timeout */
    OTA_HTTP_RSP_CODE_CONFILCT                       = 409,  /* Conflict */
    OTA_HTTP_RSP_CODE_GONE                           = 410,  /* Gone */
    OTA_HTTP_RSP_CODE_LEN_REQUIRED                   = 411,  /* Length required */
    OTA_HTTP_RSP_CODE_PRECONDITION_FAILED            = 412,  /* Precondition Failed */
    OTA_HTTP_RSP_CODE_PAYLOAD_TOO_LARGE              = 413,  /* Payload Too Large */
    OTA_HTTP_RSP_CODE_URL_TOO_LONG                   = 414,  /* URI Too Long */
    OTA_HTTP_RSP_CODE_UNSUPPORTED_MEDIA_TYPE         = 415,  /* Unsupported Media Type */
    OTA_HTTP_RSP_CODE_RANGE_NOT_SATISFIABLE          = 416,  /* Range Not Satisfiable */
    OTA_HTTP_RSP_CODE_EXPECTATION_FAILED             = 417,  /* Expectation Failed */
    OTA_HTTP_RSP_CODE_INTERNAL_SERVER_ERR            = 500,  /* Internal server error */
    OTA_HTTP_RSP_CODE_NOT_IMPLEMENTED                = 501,  /* Not Implemented */
}OTA_HttpRspCode;    /* <httprspcode>响应代码 */

typedef struct
{
    struct osTimer *rspTimer; /* HTTP(S) 响应定时器指针 */
}OTA_HttpTimeout;     /* 定时器超时时间 */


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static int g_OtaHttpFd = -1;
static ONENET_OTA_Cb *g_OtaHttpCb = NULL;
static OTA_HttpClientInfo *g_ClientInfo = NULL;
static OTA_HttpTimeout *g_Timeout = NULL;  /* ONENET OTA 超时时间 */


/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
#if 1
static void dbg_hex_data(char *name, char *pdata, uint16_t len)
{
    uint32_t i;

    OTA_HTTP_DBG_PRINTF("[%s],len[%d]\r\n", name, len);
    for(i = 0; i < len; i++)
    {
        if (i % 16 == 0)
        {
            OTA_HTTP_DBG_PRINTF("%04x:  ", i);
        }
        OTA_HTTP_DBG_PRINTF("%02x ", pdata[i]);

        if (((i + 1) % 8 == 0) && (((i + 1) % 16) != 0))
        {
            OTA_HTTP_DBG_PRINTF(" ");
        }
        if(((i + 1) % 16) == 0)
        {
            OTA_HTTP_DBG_PRINTF("\r\n");
        }
    }
    OTA_HTTP_DBG_PRINTF("\r\n");

    return;
}

static void dbg_string_data(char *name, char *pdata, uint16_t len)
{
    uint32_t i;

    OTA_HTTP_DBG_PRINTF("[%s],len[%d]\r\n", name, len);
    for(i = 0; i < len; i++)
    {
        OTA_HTTP_DBG_PRINTF("%c", pdata[i]);
    }
    OTA_HTTP_DBG_PRINTF("\r\n");

    return;
}
#endif

static int32_t OTA_HttpParamReadFromApp(uint16_t event, const void **buf, uint32_t *len)
{
    if(NULL == g_OtaHttpCb)
    {
        OTA_HTTP_ERR_PRINTF("OtaHttpCb is null\r\n");
        return osError;
    }
    if(NULL == g_OtaHttpCb->readFunc)
    {
        OTA_HTTP_ERR_PRINTF("OtaHttpCb readFunc is null\r\n");
        return osErrorTimeout;
    }

    return g_OtaHttpCb->readFunc(event, buf, len);
}

static int32_t OTA_HttpResponseWriteToApp(uint16_t event, const void *buf, uint32_t len)
{
    if(NULL == g_OtaHttpCb)
    {
        OTA_HTTP_ERR_PRINTF("OtaHttpCb is null\r\n");
        return osError;
    }
    if(NULL == g_OtaHttpCb->writeFunc)
    {
        OTA_HTTP_ERR_PRINTF("OtaHttpCb method is null\r\n");
        return osErrorTimeout;
    }

    return g_OtaHttpCb->writeFunc(event, buf, len);
}

/* 用途: 1. LWIP 有数据到来时通知用户 2. 感知 TCP 连接状态的变化情况. */
static void OTA_HttplwipEventCallback(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param)
{
    if(SOCKET_TCP_EVENT_POLL != event)
    {
        //OTA_HTTP_DBG_PRINTF("OTA lwipEventCallback:fd [%d],event [%d],len [%d]\r\n", fd, event, len);
    }
    switch(event)
    {
        case SOCKET_TCP_EVENT_ACCEPT:  /* 作为服务器, 已接收到客户端的连接 */
        case SOCKET_TCP_EVENT_SENT:  /* TCP 时, 已发送数据 */
        case SOCKET_TCP_EVENT_CONNECTED:  /* 作为客户端, 连接成功 */
        case SOCKET_TCP_EVENT_POLL:
            break;

        case SOCKET_TCP_EVENT_RECV:  /* 接收到的数据 */
            if(0 == len) /* 作为 TCP 服务器, 收到客户端的关闭请求 */
            {
                OTA_HttpResponseWriteToApp(ONENET_OTA_EVENT_UPGRADE_ERR, NULL, 0);
            }
            break;

        case SOCKET_TCP_EVENT_ERR:  /* TCP 错误提示, 如: 收到对端 RST */
        case SOCKET_NETIF_EVENT_DOWN:
            OTA_HttpResponseWriteToApp(ONENET_OTA_EVENT_UPGRADE_ERR, NULL, 0);
            break;

        default:
            OTA_HTTP_ERR_PRINTF("OTA http lwipEventCallback:event out of range [%d]\r\n", event);
            break;
    }

    return;
}

static void OTA_HttpClientInfoBufClean(OTA_HttpClientInfo *clientInfo)
{
    OTA_PubBufFree((char*)clientInfo->client->header);
    OTA_PubBufFree((char*)clientInfo->clientData->post_buf);
    OTA_PubBufFree((char*)clientInfo->clientData->header_buf);
    OTA_PubBufFree((char*)clientInfo->clientData->response_buf);
    OTA_PubBufFree((char*)clientInfo->rsp.msg);
    OTA_PubBufFree((char*)clientInfo->rsp.firmwareVersion);
    OTA_PubBufFree((char*)clientInfo->rsp.serverVersion);
    OTA_PubBufFree((char*)clientInfo->rsp.target);
    OTA_PubBufFree((char*)clientInfo->rsp.md5);
    OTA_PubBufFree((char*)clientInfo->rsp.reqId);
    OTA_PubBufFree((char*)clientInfo->rsp.body);
    clientInfo->client->header = NULL;
    clientInfo->clientData->post_buf = NULL;
    clientInfo->clientData->header_buf = NULL;
    clientInfo->clientData->response_buf = NULL;
    clientInfo->rsp.msg = NULL;
    clientInfo->rsp.firmwareVersion = NULL;
    clientInfo->rsp.serverVersion = NULL;
    clientInfo->rsp.target = NULL;
    clientInfo->rsp.md5 = NULL;
    clientInfo->rsp.reqId = NULL;
    clientInfo->rsp.body = NULL;
    clientInfo->rsp.code = 0xFFFF;
    clientInfo->rsp.tid = 0;
    clientInfo->rsp.size = 0;
    clientInfo->rsp.status = 0;
}

/* 请求响应的超时时间定时器, 超时时间到达时, 将释放内存 */
static void OTA_HttpResponseTimeout(void *parameter)
{
    OTA_HttpTimeout *timeout = g_Timeout;
    OTA_HttpClientInfo *clientInfo = g_ClientInfo;

    if((NULL == timeout) || (NULL == clientInfo))
    {
        OTA_HTTP_ERR_PRINTF("%s, clientInfo is null\r\n", __FUNCTION__);
        return;
    }

    if((NULL == clientInfo->client) || (NULL == clientInfo->clientData))
    {
        OTA_HTTP_ERR_PRINTF("%s, clientInfo client is null\r\n", __FUNCTION__);
        return;
    }

    OTA_HttpResponseWriteToApp(ONENET_OTA_EVENT_UPGRADE_ERR, (const void*)&clientInfo->rsp, 0);
    OTA_HTTP_DBG_PRINTF("OTA http rsp timeout callback write ok\r\n");
    OTA_PubTimerStop(&timeout->rspTimer);

    return;
}

#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
/* 销毁 SSL 上下文参数信息 */
static void OTA_HttpSslContexExDestory(OTA_HttpClientInfo *clientInfo)
{
    if(OTA_HTTP_URL_IS_HTTPS != clientInfo->cfg.isHttp)
    {
        return;
    }

    if(OS_NULL != clientInfo->cfg.sslCtxEx)
    {
        AT_SslMbedtlsDestroy(clientInfo->cfg.sslCtxEx);
        clientInfo->cfg.sslCtxEx = NULL;
    }
    return;
}
#endif

/* 判断HTTP(S) 服务器 URL 是 HTTP 还是 HTTPS */
static int8_t OTA_HttpUrlisHttp(char *url, OTA_HttpClientInfo *clientInfo)
{
    if(NULL == url)
    {
        OTA_HTTP_ERR_PRINTF("ota http url is null\r\n");
        return osError;
    }

    if(!strncmp(url, "http://", strlen("http://")))
    {
        clientInfo->cfg.isHttp = OTA_HTTP_URL_IS_HTTP;
    }
    else if(!strncmp(url, "https://", strlen("https://")))
    {
        clientInfo->cfg.isHttp = OTA_HTTP_URL_IS_HTTPS;
    }
    else
    {
        return osErrorTimeout;
    }
    return osOK;
}

/* 创建生成 URL */
static int32_t OTA_HttpUrlGenerate(uint16_t event, OTA_HttpClientInfo *clientInfo)
{
    int32_t ret = -1;
    uint32_t paramLen = 0;
    char *url = clientInfo->cfg.url;
    uint16_t urlLen = OTA_HTTP_URL_LEN_MAX + 1;
    char *productId = NULL;
    char *devName = NULL;
    char *devVersion = NULL;
    uint8_t *taskType = 0;
    uint32_t *taskId = 0;

    ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_PRODUCT_ID, (const void**)&productId, &paramLen);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http productId get fail\r\n");
        return osError;
    }
    if(NULL == productId)
    {
        OTA_HTTP_ERR_PRINTF("ota http productId is NULL\r\n");
        return osErrorTimeout;
    }

    ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_DEV_NAME, (const void**)&devName, &paramLen);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http devName get fail\r\n");
        return osErrorResource;
    }
    if(NULL == devName)
    {
        OTA_HTTP_ERR_PRINTF("ota http devName is NULL\r\n");
        return osErrorParameter;
    }

    ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_DEV_VERSION, (const void**)&devVersion, &paramLen);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http devVersion get fail\r\n");
        return osErrorNoMemory;
    }
    if(NULL == devVersion)
    {
        OTA_HTTP_ERR_PRINTF("ota http devVersion is NULL\r\n");
        return osErrorISR;
    }

    ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_TASK_TYPE, (const void**)&taskType, &paramLen);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http taskType get fail\r\n");
        return osErrorResourceFull;
    }

    ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_TASK_TYPE, (const void**)&taskType, &paramLen);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http taskType get fail\r\n");
        return osErrorBusy;
    }

    ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_TASK_ID, (const void**)&taskId, &paramLen);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http taskId get fail\r\n");
        return osErrorNoSys;
    }

    if(NULL == url)
    {
        url = osMalloc(urlLen);
        if(NULL == url)
        {
            OTA_HTTP_ERR_PRINTF("ota http url malloc fail\r\n");
            return osErrorIO;
        }
        clientInfo->cfg.url = url;
    }
    memset(url, 0, urlLen);

    switch(event)
    {
        case ONENET_OTA_EVENT_UPGRADE_START: /* 开始升级 */
            break;

        case ONENET_OTA_EVENT_REPORT_VERSION: /* 上报设备当前版本 */
        case ONENET_OTA_EVENT_INQUIRE_VERSION: /* 查看设备版本号(可省略) */
            osSnprintf(url, urlLen, "%s%s/%s/%s", OTA_HTTP_URL_API_DOMAIN, productId, devName, OTA_HTTP_URL_VERSION);
            break;

        case ONENET_OTA_EVENT_CHECK_UPGRADE_TASK:  /* 检测升级任务 */
            osSnprintf(url, urlLen, "%s%s/%s/%s?type=%d&version=%s", OTA_HTTP_URL_API_DOMAIN, \
                    productId, devName, OTA_HTTP_URL_CHECK, *taskType, devVersion);
            break;

        case ONENET_OTA_EVENT_INQUIRE_TASK_STATUS: /* 查询任务状态 */
            osSnprintf(url, urlLen, "%s%s/%s/%d/%s", OTA_HTTP_URL_API_DOMAIN, productId, devName, \
                *taskId, OTA_HTTP_URL_CHECK);
            break;

        case ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE: /* 下载升级包 */
            osSnprintf(url, urlLen, "%s%s/%s/%d/%s", OTA_HTTP_URL_API_DOMAIN, productId, devName, \
                    *taskId, OTA_HTTP_URL_DOWNLOAD);
            break;

        case ONENET_OTA_EVENT_REPORT_DOWNLOAD_RATE: /* 上报下载进度 */
        case ONENET_OTA_EVENT_REPORT_UPGRADE_STATUS: /* 上报升级状态 */
            osSnprintf(url, urlLen, "%s%s/%s/%d/%s", OTA_HTTP_URL_API_DOMAIN, productId, devName, \
                    *taskId, OTA_HTTP_URL_STATUS);
            break;

        case ONENET_OTA_EVENT_UPGRADE_ERR: /* 升级发生错误 */
            break;

        case ONENET_OTA_EVENT_UPGRADE_STOP: /* 结束升级 */
            break;

        default:
            OTA_HTTP_ERR_PRINTF("ota http event err[%d]\r\n", event);
            OTA_PubBufFree(clientInfo->cfg.url);
            clientInfo->cfg.url = NULL;
            return osErrorIntr;
    }

    return osOK;
}

/* 封装 HTTP client 头部信息 */
static int32_t OTA_HttpHeaderNosecureFill(OTA_HttpClientInfo *clientInfo)
{
    int32_t ret = -1;
    uint32_t paramLen = 0;
    char *authorization = NULL;
    char *authUser = NULL;
    char *authPassword = NULL;
    http_client_t *client = clientInfo->client;

    ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_AUTHORIZATION, (const void**)&authorization, &paramLen);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http authorization get fail\r\n");
        return osError;
    }
    if(NULL == authorization)
    {
        OTA_HTTP_ERR_PRINTF("ota http get authorization is null\r\n");
        return osErrorTimeout;
    }

    ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_USERNAME, (const void**)&authUser, &paramLen);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http authUser get fail\r\n");
        return osErrorResource;
    }

    ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_PASSWORD, (const void**)&authPassword, &paramLen);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http authPassword get fail\r\n");
        return osErrorParameter;
    }

    uint16_t len = strlen(authorization) + strlen(OTA_HTTP_AUTHORIZATION) + 3; /* 1个 \r\n + 1个 \0 */
    len += strlen(OTA_HTTP_CONNECT_KEEPALIVE) + 3; /* 1个 \r\n + 1个 \0 */
    if(NULL == client->header)
    {
        client->header = osMalloc(len); /* +1 for store '\0' in HTTP API */
        if(NULL == client->header)
        {
            OTA_HTTP_ERR_PRINTF("ota header malloc fail\r\n");
            return osErrorNoMemory;
        }
    }
    memset(client->header, 0, len);

    osSnprintf(client->header, len, "authorization: %s\r\n%s\r\n", authorization, OTA_HTTP_CONNECT_KEEPALIVE);
    client->auth_user = authUser;
    client->auth_password = authPassword;
    client->is_http = clientInfo->cfg.isHttp;

    return osOK;
}

#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
/* 封装 HTTP client secure 头部信息 */
static int32_t OTA_HttpHeaderSecureFill(OTA_HttpClientInfo *clientInfo)
{
    http_client_t *client = clientInfo->client;
    AT_SslContextEx *sslCtxEx = NULL;
    uint8_t cid = clientInfo->cfg.cid;
    uint8_t sslId = clientInfo->cfg.sslId;

    OTA_HTTP_DBG_PRINTF("ssl param get cid[%d], sslId[%d]\r\n", cid, sslId);
    clientInfo->cfg.sslCtxEx = OTA_PubSslMbedtlsEstablish(NULL, 0, cid, sslId, 0);
    if(OS_NULL == clientInfo->cfg.sslCtxEx)
    {
        OTA_HTTP_ERR_PRINTF("ssl param get fail\r\n");
        return osError;
    }
    sslCtxEx = clientInfo->cfg.sslCtxEx;

    client->server_cert = sslCtxEx->sslCtxParam->certInfo.caCert;
    client->client_cert = sslCtxEx->sslCtxParam->certInfo.clientCert;
    client->client_pk = sslCtxEx->sslCtxParam->certInfo.clientPk;
    client->server_cert_len = sslCtxEx->sslCtxParam->certInfo.caCertLen;
    client->client_cert_len = sslCtxEx->sslCtxParam->certInfo.clientCertLen;
    client->client_pk_len = sslCtxEx->sslCtxParam->certInfo.clientPkLen;
    client->ssl = NULL;

    return osOK;
}
#endif

/* 封装 post buf */
static int32_t OTA_HttpPostBufFill(uint16_t event, http_client_data_t *clientData)
{
    int32_t ret = -1;
    uint32_t paramLen = 0;
    uint16_t len = 0;

    if(ONENET_OTA_EVENT_REPORT_VERSION == event)
    {
        char *serverVersion = NULL;
        char *firmwareVersion = NULL;
        ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_S_VERSION, (const void**)&serverVersion, &paramLen);
        if(osOK != ret)
        {
            OTA_HTTP_ERR_PRINTF("ota http serverVersion get fail\r\n");
            return osError;
        }
        if(NULL == serverVersion)
        {
            OTA_HTTP_ERR_PRINTF("ota http serverVersion is null\r\n");
            return osErrorTimeout;
        }

        ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_F_VERSION, (const void**)&firmwareVersion, &paramLen);
        if(osOK != ret)
        {
            OTA_HTTP_ERR_PRINTF("ota http firmwareVersion get fail\r\n");
            return osErrorResource;
        }
        if(NULL == firmwareVersion)
        {
            OTA_HTTP_ERR_PRINTF("ota http firmwareVersion is null\r\n");
            return osErrorParameter;
        }

        len = strlen("{\"s_version\":\"\",\"f_version\":\"\"}") + strlen(serverVersion) + strlen(firmwareVersion);
        len += 4; /* 多2个 \r\n */
        clientData->post_buf = osMalloc(len);
        if(OS_NULL == clientData->post_buf)
        {
            OTA_HTTP_ERR_PRINTF("ota http post buf malloc fail\r\n");
            return osErrorNoMemory;
        }
        memset(clientData->post_buf, 0, len);
        osSnprintf(clientData->post_buf, len, "{\"s_version\": \"%s\", \"f_version\":\"%s\"}", serverVersion, firmwareVersion);
    }
    else if((ONENET_OTA_EVENT_REPORT_DOWNLOAD_RATE == event) || (ONENET_OTA_EVENT_REPORT_UPGRADE_STATUS == event))
    {
        uint8_t *upgradeStatus = NULL;
        ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_UPGRADE_STATUS, (const void**)&upgradeStatus, &paramLen);
        if(osOK != ret)
        {
            OTA_HTTP_ERR_PRINTF("ota http upgradeStatus get fail\r\n");
            return osErrorParameter;
        }
        len = strlen("{\"step\":}") + 3; /* 状态码占 3 个字节 */
        len += 4; /* 多2个 \r\n */
        clientData->post_buf = osMalloc(len);
        if(OS_NULL == clientData->post_buf)
        {
            OTA_HTTP_ERR_PRINTF("ota post buf malloc fail\r\n");
            return osErrorNoMemory;
        }
        memset(clientData->post_buf, 0, len);
        osSnprintf(clientData->post_buf, len, "{\"step\":%d}", *upgradeStatus);
    }

    if(NULL != clientData->post_buf)
    {
        OTA_HTTP_DBG_PRINTF("ota http post_buf[%s]\r\n", clientData->post_buf);
    }

    return osOK;
}

/* 封装 HTTP client 头部信息 */
static int32_t OTA_HttpHeaderFill(OTA_HttpClientInfo *clientInfo)
{
    int32_t ret = -1;

    ret = OTA_HttpHeaderNosecureFill(clientInfo);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http HeaderNosecureFill fail\r\n");
        return osError;
    }

#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
    if(OTA_HTTP_URL_IS_HTTPS == clientInfo->cfg.isHttp)
    {
        ret = OTA_HttpHeaderSecureFill(clientInfo);
    }
#endif

    return ret;
}

/* 封装 HTTP client 正文体信息 */
static int32_t OTA_HttpBodyFill(uint16_t event, http_client_data_t *clientData)
{
    int responseHeadLen = OTA_HTTP_RESPONSE_HEAD_LEN;
    int responseBodyLen = OTA_HTTP_RESPONSE_BODY_LEN;
    int *rangeStart = 0;
    int *rangeLen = 0;

    if(ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE == event)
    {
        int32_t ret = -1;
        uint32_t paramLen = 0;
        ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_RANGE_START, (const void**)&rangeStart, &paramLen);
        if(osOK != ret)
        {
            OTA_HTTP_ERR_PRINTF("ota http rangeStart get fail\r\n");
            return osError;
        }

        ret = OTA_HttpParamReadFromApp(ONENET_OTA_PARAM_RANGE_LEN, (const void**)&rangeLen, &paramLen);
        if(osOK != ret)
        {
            OTA_HTTP_ERR_PRINTF("ota http rangeLen get fail\r\n");
            return osErrorTimeout;
        }
    }

    if(osOK != OTA_HttpPostBufFill(event, clientData))
    {
        OTA_HTTP_ERR_PRINTF("ota post buf fill fail\r\n");
        return osErrorResource;
    }

    /* clientData 结构体赋值 */
    if(NULL != clientData->post_buf)
    {
        clientData->post_buf_len = strlen(clientData->post_buf); /* post_buf 为 NULL 时, 使用 strlen 将死机 */
    }
    clientData->post_content_type = OTA_HTTP_CONTENT_TYPE;
    clientData->response_buf_len = responseBodyLen;
    clientData->header_buf_len = responseHeadLen;

    /* 断点续传, 获取服务器的指定开始位置的指定长度的数据. 将 Range 信息封装在 HTTP(S) 头部. 有以下几种格式可供选择:
    ** 1. Range: bytes=startPostion-endPostion. 表示第startPostion-endPostion字节范围的内容.
    ** 2. Range: bytes=endPostion. 表示最后endPostion字节范围的内容.
    ** 3. Range: bytes=startPostion-. 表示从第startPostion字节开始到文件结束部分的内容.
    ** 4. Range: bytes=0-0,-1. 表示第一个和最后一个字节的内容.
    ** 5. Range: bytes=startPostion1-endPostion1,startPostion2-endPostion2. 同时指定几个字节范围的内容.
    */
    if(ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE == event)
    {
        clientData->range_start = *rangeStart;
        clientData->range_len = *rangeLen;
    }
    else
    {
        clientData->range_start = 0;
        clientData->range_len = 0;
    }

    return osOK;
}

/* 发送 HTTP 请求到服务器 */
static int8_t OTA_HttpClientProcess(uint16_t event, uint8_t reqMethod, OTA_HttpClientInfo *clientInfo)
{
    int ret = -1;
    char *url = clientInfo->cfg.url;
    http_client_t *client = clientInfo->client;
    http_client_data_t *clientData = clientInfo->clientData;

    if(NULL == clientData->header_buf)
    {
        clientData->header_buf = osMalloc(OTA_HTTP_RESPONSE_HEAD_LEN + 1); /* +1 for store '\0' in HTTP API */
        if(NULL == clientData->header_buf)
        {
            OTA_HTTP_ERR_PRINTF("ota header_buf malloc fail\r\n");
            return osError;
        }
    }
    memset(clientData->header_buf, 0, OTA_HTTP_RESPONSE_HEAD_LEN + 1);

    if(NULL == clientData->response_buf)
    {
        clientData->response_buf = osMalloc(OTA_HTTP_RESPONSE_BODY_LEN + 1); /* +1 for store '\0' in HTTP API */
        if(NULL == clientData->response_buf)
        {
            OTA_HTTP_ERR_PRINTF("ota response_buf malloc fail\r\n");
            return osErrorTimeout;
        }
    }
    memset(clientData->response_buf, 0, OTA_HTTP_RESPONSE_BODY_LEN + 1);

    ret = OTA_HttpHeaderFill(clientInfo);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http header fill fail\r\n");
        return HTTP_ECONN;
    }

    ret = OTA_HttpBodyFill(event, clientData);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http body fill fail\r\n");
        return HTTP_ESEND;
    }

    switch(reqMethod)
    {
        case OTA_HTTP_REQ_METHOD_GET:
            ret = http_client_get(client, url, clientData); /* 从服务器上取数据 */
            break;

        case OTA_HTTP_REQ_METHOD_POST:
            ret = http_client_post(client, url, clientData); /* 向服务器上传送数据 */
            break;

        case OTA_HTTP_REQ_METHOD_URL:
        case OTA_HTTP_REQ_METHOD_POST_FILE:
        case OTA_HTTP_REQ_METHOD_PUT:
        case OTA_HTTP_REQ_METHOD_PUT_FILE:
            break;

        default:
            OTA_HTTP_ERR_PRINTF("sendType err [%d]\r\n", reqMethod);
            break;
    }

    if(HTTP_SUCCESS != ret)
    {
        OTA_HTTP_ERR_PRINTF("OTA_HttpClientSend fail, method[%d], ret[%d]\r\n", reqMethod, ret);
#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
        OTA_HttpSslContexExDestory(clientInfo);
#endif
        return ret;
    }

    if(NULL != client)
    {
        if((client->response_code != OTA_HTTP_RSP_CODE_OK) && (client->response_code != OTA_HTTP_RSP_CODE_CREATED) \
            && (client->response_code != OTA_HTTP_RSP_CODE_ACCEPTED) && (client->response_code != OTA_HTTP_RSP_CODE_PARTIAL_CONTENT))
        {
            OTA_HTTP_ERR_PRINTF("ota_http_client_send, rsp code err[%d]\r\n", client->response_code);
            ret = HTTP_EUNKOWN;
        }
    }

    if((NULL != client) && (NULL != clientData) && (NULL != clientData->response_buf))
    {
        OTA_HTTP_DBG_PRINTF("ota_http_client_send: rsp_code[%d], tot_len[%d][%d][%d][%d][%d]\r\n", client->response_code, clientData->content_range_len, \
                clientData->retrieve_len, clientData->response_content_len, clientData->content_block_len, strlen(clientData->response_buf));
        //dbg_string_data("header_buf", clientData->header_buf, strlen(clientData->header_buf));
        //dbg_string_data("response_buf", clientData->response_buf, clientData->response_content_len);
        //dbg_hex_data("response_buf", clientData->response_buf, clientData->response_content_len);
    }

    return ret;
}

static int32_t OTA_HttpClientInit(OTA_HttpClientInfo *clientInfo)
{
    if(NULL == clientInfo->client)
    {
        clientInfo->client = (http_client_t*)osMalloc(sizeof(http_client_t));
        if(NULL == clientInfo->client)
        {
            OTA_HTTP_ERR_PRINTF("%s, client alloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
    }
    memset(clientInfo->client, 0, sizeof(http_client_t));
    clientInfo->client->header = NULL;
    clientInfo->client->auth_user = NULL;
    clientInfo->client->auth_password = NULL;
#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
    clientInfo->client->server_cert = NULL;
    clientInfo->client->client_cert = NULL;
    clientInfo->client->client_pk = NULL;
    clientInfo->client->ssl = NULL;
#endif

    return osOK;
}

/* 申请 HTTP client_data 内存 */
static int32_t OTA_HttpClientDataInit(OTA_HttpClientInfo *clientInfo)
{
    if(NULL == clientInfo->clientData)
    {
        clientInfo->clientData = (http_client_data_t*)osMalloc(sizeof(http_client_data_t));
        if(NULL == clientInfo->clientData)
        {
            OTA_HTTP_ERR_PRINTF("%s, clientData alloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
    }
    memset(clientInfo->clientData, 0, sizeof(http_client_data_t));
    clientInfo->clientData->post_content_type = NULL;
    clientInfo->clientData->post_buf = NULL;
    clientInfo->clientData->response_buf = NULL;
    clientInfo->clientData->header_buf = NULL;
    clientInfo->clientData->redirect_url = NULL;

    return osOK;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
 /* 获取 HTTP 连接时创建的套接字 FD */
char* OTA_HttpClientUrlGet(uint16_t event)
{
    OTA_HttpClientInfo *clientInfo = g_ClientInfo;
    if(NULL == clientInfo)
    {
        OTA_HTTP_ERR_PRINTF("%s, clientInfo is null\r\n", __FUNCTION__);
        return NULL;
    }
    if(osOK != OTA_HttpUrlGenerate(event, clientInfo))
    {
        OTA_HTTP_ERR_PRINTF("ota url get fail Event[%d]\r\n", event);
        return NULL;
    }
    return clientInfo->cfg.url;
 }

/* 建立 HTTP 连接 */
int32_t OTA_HttpClientCreate(void)
{
    int8_t ret = -1;
    char *url = OTA_HTTP_URL_API_DOMAIN;
    http_client_t *client = NULL;
    OTA_HttpTimeout *timeout = g_Timeout;
    OTA_HttpClientInfo *clientInfo = g_ClientInfo;

    if((NULL == timeout) || (NULL == clientInfo))
    {
        OTA_HTTP_ERR_PRINTF("%s, clientInfo is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL == clientInfo->client)
    {
        OTA_HTTP_ERR_PRINTF("%s, clientInfo client is null\r\n", __FUNCTION__);
        return osErrorTimeout;
    }

    if(osOK != OTA_HttpUrlisHttp(url, clientInfo))
    {
        OTA_HTTP_ERR_PRINTF("%s, url not http err\r\n", __FUNCTION__);
        return osErrorResource;
    }

    ret = OTA_HttpHeaderFill(clientInfo);
    if(osOK != ret)
    {
        OTA_HTTP_ERR_PRINTF("ota http msg fill fail\r\n");
        return osErrorParameter;
    }

    client = clientInfo->client;
    uint8_t cid = clientInfo->cfg.cid;
    OTA_HTTP_DBG_PRINTF("OTA http create, cid[%d], url[%s]\r\n", cid, url);
    ret = http_client_create_extend(client, url, cid);
    if(0 == ret) /* http 连接创建成功 */
    {
        g_OtaHttpFd = client->socket;
        OTA_PubSetSockoptLinger(client->socket, OTA_PUB_SOCKET_CLOSE_NOW);
        if(0 != OTA_PubSocketCallbackRegister(client->socket, OTA_HttplwipEventCallback, NULL))
        {
            http_client_close(client);
            g_OtaHttpFd = client->socket;
            OTA_HTTP_ERR_PRINTF("ota http register callback fail\r\n");
            return osErrorNoMemory;
        }
    }

    return osOK;
}

/* 关闭 HTTP 连接 */
void OTA_HttpClientClose(void)
{
    OTA_HttpClientInfo *clientInfo = g_ClientInfo;

    OTA_HttpTimeoutDeinit();

    if(NULL == clientInfo)
    {
        OTA_HTTP_ERR_PRINTF("%s, clientInfo is null\r\n", __FUNCTION__);
        return;
    }
    if(NULL == clientInfo->client)
    {
        OTA_HTTP_ERR_PRINTF("%s, clientInfo client is null\r\n", __FUNCTION__);
        return;
    }

    if(g_OtaHttpFd >= 0) /* 为了避免重复关闭 */
    {
        OTA_PubSocketCallbackUnregister(g_OtaHttpFd); /* 注销 LWIP 事件回调 */
        OTA_HTTP_ERR_PRINTF("ota httpClient close\r\n");
        http_client_close(clientInfo->client);
        g_OtaHttpFd = clientInfo->client->socket;
#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
        OTA_HttpSslContexExDestory(clientInfo);
#endif
    }
    OTA_HttpClientInfoDeinit();

    return;
}

/* 发送 HTTP 数据(GET/POST) */
int32_t OTA_HttpClientSend(uint16_t event, uint8_t reqMethod)
{
    int32_t ret = -1;
    uint16_t rspTimeout = OTA_HTTP_RESPONSE_TIMEOUT;
    OTA_HttpTimeout *timeout = g_Timeout;
    OTA_HttpClientInfo *clientInfo = g_ClientInfo;

    if((NULL == timeout) || (NULL == clientInfo))
    {
        OTA_HTTP_ERR_PRINTF("%s, clientInfo is null\r\n", __FUNCTION__);
        return osError;
    }

    if((NULL == clientInfo->client) || (NULL == clientInfo->clientData))
    {
        OTA_HTTP_ERR_PRINTF("%s, clientInfo client is null\r\n", __FUNCTION__);
        return osErrorTimeout;
    }

    if(osOK != OTA_HttpUrlGenerate(event, clientInfo))
    {
        OTA_HTTP_ERR_PRINTF("ota event[%d], url generate fail\r\n", event);
        return osErrorTimeout;
    }

    if(ERR_OK != OTA_PubTimerStart(&timeout->rspTimer, OTA_HttpResponseTimeout, \
                                rspTimeout * 1000, NULL, OS_TIMER_FLAG_ONE_SHOT))
    {
        OTA_HTTP_ERR_PRINTF("OTA http req rsp timer start fail\r\n");
        return osErrorResource;
    }

    OTA_HTTP_DBG_PRINTF("ota http send start, event[%d], method[%d], url[%s]\r\n", event, reqMethod, clientInfo->cfg.url);
    ret = OTA_HttpClientProcess(event, reqMethod, clientInfo);
    if(HTTP_SUCCESS == ret) /* 成功 */
    {
        OTA_RspParse(event, (char*)clientInfo);
    }
    OTA_HttpResponseWriteToApp(event, (const void*)&clientInfo->rsp, 0);
    OTA_HTTP_DBG_PRINTF("OTA http send callback write ok\r\n");
    OTA_PubTimerStop(&timeout->rspTimer);
    OTA_HttpClientInfoBufClean(clientInfo);

    return osOK;
}

int32_t OTA_HttpCallbackRegister(ONENET_OTA_Cb *callback)
{
    g_OtaHttpCb = callback;
    return osOK;
}

int32_t OTA_HttpClientInfoInit(void)
{
    OTA_HttpClientInfo *clientInfo = g_ClientInfo;

    if(NULL == clientInfo)
    {
        clientInfo = (OTA_HttpClientInfo*)osMalloc(sizeof(OTA_HttpClientInfo));
        if(NULL == clientInfo)
        {
            OTA_HTTP_ERR_PRINTF("%s, clientInfo alloc fail\r\n", __FUNCTION__);
            return osError;
        }
        memset(clientInfo, 0, sizeof(OTA_HttpClientInfo));
        g_ClientInfo = clientInfo;
        clientInfo->cfg.cid = CID_DEFAULT_NETIF;
        clientInfo->cfg.sslId = OTA_HTTP_SSL_ID_DEFAULT;
        clientInfo->cfg.url = NULL;
#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
        clientInfo->cfg.sslCtxEx = NULL;
#endif
        clientInfo->rsp.code = 0xFFFF;
        clientInfo->rsp.taskType = ONENET_OTA_TASK_TYPE_FOTA;
        clientInfo->rsp.msg = NULL;
        clientInfo->rsp.firmwareVersion = NULL;
        clientInfo->rsp.serverVersion = NULL;
        clientInfo->rsp.target = NULL;
        clientInfo->rsp.md5 = NULL;
        clientInfo->rsp.reqId = NULL;
        clientInfo->client = NULL;
        clientInfo->clientData = NULL;
    }

    if(osOK != OTA_HttpClientInit(clientInfo))
    {
        OTA_HTTP_ERR_PRINTF("%s, client init fail\r\n", __FUNCTION__);
        OTA_PubBufFree((char*)g_ClientInfo);
        g_ClientInfo = NULL;
        return osErrorTimeout;
    }

    if(osOK != OTA_HttpClientDataInit(clientInfo))
    {
        OTA_HTTP_ERR_PRINTF("%s, clientData init fail\r\n", __FUNCTION__);
        OTA_PubBufFree((char*)clientInfo->client);
        clientInfo->client = NULL;
        OTA_PubBufFree((char*)g_ClientInfo);
        g_ClientInfo = NULL;
        return osErrorResource;
    }

    return osOK;
}

int32_t OTA_HttpTimeoutInit(void)
{
    OTA_HttpTimeout *timeout = g_Timeout;

    if(NULL == timeout)
    {
        timeout = (OTA_HttpTimeout*)osMalloc(sizeof(OTA_HttpTimeout));
        if(NULL == timeout)
        {
            OTA_HTTP_ERR_PRINTF("%s, timeout alloc fail\r\n", __FUNCTION__);
            return osError;
        }
        memset(timeout, 0, sizeof(OTA_HttpTimeout));
        g_Timeout = timeout;
    }
    timeout->rspTimer = NULL;

    return osOK;
}

void OTA_HttpCallbackUnregister(void)
{
    g_OtaHttpCb = NULL;
    return;
}

void OTA_HttpClientInfoDeinit(void)
{
    OTA_HttpClientInfo *clientInfo = g_ClientInfo;

    if(NULL == clientInfo)
    {
        OTA_HTTP_ERR_PRINTF("clientInfo is null, no need deinit\r\n");
        return;
    }

    OTA_PubBufFree((char*)clientInfo->cfg.url);
    OTA_PubBufFree((char*)clientInfo->rsp.msg);
    OTA_PubBufFree((char*)clientInfo->rsp.firmwareVersion);
    OTA_PubBufFree((char*)clientInfo->rsp.serverVersion);
    OTA_PubBufFree((char*)clientInfo->rsp.target);
    OTA_PubBufFree((char*)clientInfo->rsp.md5);
    OTA_PubBufFree((char*)clientInfo->rsp.reqId);
    clientInfo->cfg.url = NULL;
    clientInfo->rsp.msg = NULL;
    clientInfo->rsp.firmwareVersion = NULL;
    clientInfo->rsp.serverVersion = NULL;
    clientInfo->rsp.target = NULL;
    clientInfo->rsp.md5 = NULL;
    clientInfo->rsp.reqId = NULL;

    if(NULL != clientInfo->client)
    {
        OTA_PubBufFree((char*)clientInfo->client->header);
        OTA_PubBufFree((char*)clientInfo->client->auth_user);
        OTA_PubBufFree((char*)clientInfo->client->auth_password);
        clientInfo->client->header = NULL;
        clientInfo->client->auth_user = NULL;
        clientInfo->client->auth_password = NULL;
#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
        OTA_PubBufFree((char*)clientInfo->client->server_cert);
        OTA_PubBufFree((char*)clientInfo->client->client_cert);
        OTA_PubBufFree((char*)clientInfo->client->client_pk);
        OTA_PubBufFree((char*)clientInfo->client->ssl);
        clientInfo->client->server_cert = NULL;
        clientInfo->client->client_cert = NULL;
        clientInfo->client->client_pk = NULL;
        clientInfo->client->ssl = NULL;
#endif
    }

    if(NULL != clientInfo->clientData)
    {
        OTA_PubBufFree((char*)clientInfo->clientData->post_buf);
        OTA_PubBufFree((char*)clientInfo->clientData->header_buf);
        OTA_PubBufFree((char*)clientInfo->clientData->response_buf);
        //OTA_PubBufFree((char*)clientInfo->clientData->post_content_type);
        OTA_PubBufFree((char*)clientInfo->clientData->redirect_url);
        clientInfo->clientData->post_buf = NULL;
        clientInfo->clientData->header_buf = NULL;
        clientInfo->clientData->response_buf = NULL;
        clientInfo->clientData->post_content_type = NULL;
        clientInfo->clientData->redirect_url = NULL;
    }

    OTA_PubBufFree((char*)clientInfo->client);
    OTA_PubBufFree((char*)clientInfo->clientData);
    clientInfo->client = NULL;
    clientInfo->clientData = NULL;

    OTA_PubBufFree((char*)g_ClientInfo);
    g_ClientInfo = NULL;

    return;
}

void OTA_HttpTimeoutDeinit(void)
{
    if(NULL == g_Timeout)
    {
        OTA_HTTP_ERR_PRINTF("%s, timeout is null, no need deinit\r\n", __FUNCTION__);
        return;
    }

    OTA_PubTimerStop(&g_Timeout->rspTimer);
    OTA_PubBufFree((char*)g_Timeout);
    g_Timeout = NULL;

    return;
}

#ifdef __cplusplus
}
#endif

