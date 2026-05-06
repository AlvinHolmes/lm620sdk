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

#ifndef __ONENET_OTA_HTTP_H__
#define __ONENET_OTA_HTTP_H__

#include "http_application_api.h"
#include "app_at_ssl.h"
#include "onenet_ota_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_HTTP_URL_LEN_MAX                        (255)   /* OTA HTTP(S) URL 最大长度 */

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum
{
    OTA_HTTP_REQ_METHOD_URL = 0,          /* 设置服务器 URL */
    OTA_HTTP_REQ_METHOD_GET,              /* 发送 GET 请求到 HTTP(S) 服务器 */
    OTA_HTTP_REQ_METHOD_POST,             /* 发送 POST 请求到 HTTP(S) 服务器 */
    OTA_HTTP_REQ_METHOD_POST_FILE,        /* 通过文件发送 POST 请求到 HTTP(S) 服务器 */
    OTA_HTTP_REQ_METHOD_PUT,              /* 发送 PUT 请求到 HTTP(S)服务器 */
    OTA_HTTP_REQ_METHOD_PUT_FILE,         /* 通过文件发送 PUT 请求到 HTTP(S)服务器 */
    OTA_HTTP_REQ_METHOD_MORE,
}OTA_HttpReqMethod;        /* HTTP(S) 请求方法 */

typedef struct
{
    uint8_t cid;             /* PDP 上下文 id, 指定当前实例使用的 PDP 上下文. 默认值1 */
    uint8_t sslId;           /* HTTP(S) 的 SSL 上下文 ID. 范围:0-5, 默认值1 */
    uint8_t isHttp;          /* 1: HTTP; 0: HTTPS */
    uint8_t upgradeStatus;   /* 升级状态码. 100以内为升级包下载进度，100以上为状态码 */
    char *url;               /* URL 字符串, GET/POST/PUT 时使用, 必须以 http:// 或   https:// 开头. 仅本次连接有效, GET/POST/PUT 完后需要释放内存 */
#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
    AT_SslContextEx *sslCtxEx; /* SSL 上下文参数 */
#endif
}OTA_HttpClientCfg; /* ONENET OTA client 参数信息 */

typedef struct
{
    OTA_HttpClientCfg cfg;
    ONENT_OTA_RspInfo rsp;
    http_client_t *client;
    http_client_data_t *clientData;
}OTA_HttpClientInfo; /* ONENET OTA client 信息 */

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
char* OTA_HttpClientUrlGet(uint16_t event);
int32_t OTA_HttpClientCreate(void);
void OTA_HttpClientClose(void);
int32_t OTA_HttpClientSend(uint16_t event, uint8_t reqMethod);
int32_t OTA_HttpCallbackRegister(ONENET_OTA_Cb *callback);
int32_t OTA_HttpClientInfoInit(void);
int32_t OTA_HttpTimeoutInit(void);
void OTA_HttpCallbackUnregister(void);
void OTA_HttpClientInfoDeinit(void);
void OTA_HttpTimeoutDeinit(void);

#ifdef __cplusplus
}
#endif
#endif

