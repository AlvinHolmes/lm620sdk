/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief:OneNET OTA token
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
#include "onenet_ota_pub.h"
#include "onenet_ota_token.h"
#include "onenet_ota_http.h"
#include "onenet_ota_api.h"


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define ONENET_OTA_API_ON
 #ifdef ONENET_OTA_API_ON
#define ONENET_OTA_API_DBG_PRINTF                         OTA_PUB_PRINT_DEBUG
#else
#define ONENET_OTA_API_DBG_PRINTF(...)
#endif
#define ONENET_OTA_API_ERR_PRINTF                         OTA_PUB_PRINT_ERROR


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/


/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/


/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
/**
 ***********************************************************************************************
 * @brief           1. Token 的签名生成算法为:
                    sign = base64(hmac_<method>(base64decode(accessKey), utf-8(StringForSignature)))
 *                  2. StringForSignature组成:
 *                  StringForSignature = et + '\n' + method + '\n' + res+ '\n' + version
 *
 * @param[in]       res            格式为: userid/xxx 或 projectid/xxx 或 products/xxx/devices/xxx 或 products/xxx
 * @param[in]       et             UNIX时间戳, 从1970年1月1日开始所经过的秒数. 转换成北京时间必须比当前
 *                                 系统时间更大. 1836152085代表北京时间2028-03-09 02:14:45
 * @param[in]       accessKey      对应的秘钥(用户秘钥或产品秘钥或设备秘钥). 秘钥是唯一的
 * @param[in]       method         签名方法. 有 md5, sha1, sha256
 *
 * @param[out]      authorization  生成的安全秘钥鉴权
 *
 * @return          osOK: Token 计算生成成功. 其它: Token 计算生成失败
 ***********************************************************************************************
*/
int32_t ONENET_OTA_TokenGenerate(const char *res, uint32_t et, const char *accessKey, uint8_t method, char *authorization)
{
    int32_t ret = osError;
    OTA_PubMutexLock();
    ret = OTA_TokenGenerate(res, et, accessKey, method, authorization);
    OTA_PubMutexUnlock();
    return ret;
}

/**
************************************************************************************
* @brief           ONENET OTA 获取 event 对应的 URL
*
* @param[in]       cbEvent  OTA 流程的事件
* @param[in]       force   是否强制重新下载升级包(将上次下载的偏移量置为0). true: 是; false: 否
*
* @return          >=: 偏移位置. 其它: 失败
************************************************************************************
*/
char* ONENET_OTA_UrlGet(uint16_t event)
{
    char *ret = NULL;
    OTA_PubMutexLock();
    ret = OTA_HttpClientUrlGet(event);
    OTA_PubMutexUnlock();
    return ret;
}

/**
************************************************************************************
* @brief           ONENET OTA 上报设备当前版本
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_ReportVersion(void)
{
    int32_t ret = osError;
    OTA_PubMutexLock();
    ret = OTA_HttpClientSend(ONENET_OTA_EVENT_REPORT_VERSION, OTA_HTTP_REQ_METHOD_POST);
    OTA_PubMutexUnlock();
    return ret;
}

/**
************************************************************************************
* @brief           ONENET OTA 查看设备版本号(可省略)
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_InquireVersion(void)
{
    int32_t ret = osError;
    OTA_PubMutexLock();
    ret = OTA_HttpClientSend(ONENET_OTA_EVENT_INQUIRE_VERSION, OTA_HTTP_REQ_METHOD_GET);
    OTA_PubMutexUnlock();
    return ret;
}

/**
************************************************************************************
* @brief           ONENET OTA  检测升级任务
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_CheckUpgradeTask(void)
{
    int32_t ret = osError;
    OTA_PubMutexLock();
    ret = OTA_HttpClientSend(ONENET_OTA_EVENT_CHECK_UPGRADE_TASK, OTA_HTTP_REQ_METHOD_GET);
    OTA_PubMutexUnlock();
    return ret;
}

/**
************************************************************************************
* @brief           ONENET OTA  查询任务状态
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_InquireTaskStatus(void)
{
    int32_t ret = osError;
    OTA_PubMutexLock();
    ret = OTA_HttpClientSend(ONENET_OTA_EVENT_INQUIRE_TASK_STATUS, OTA_HTTP_REQ_METHOD_GET);
    OTA_PubMutexUnlock();
    return ret;
}

/**
************************************************************************************
* @brief           ONENET OTA  下载升级包
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_DownloadFirmware(void)
{
    int32_t ret = osError;
    OTA_PubMutexLock();
    ret = OTA_HttpClientSend(ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE, OTA_HTTP_REQ_METHOD_GET);
    OTA_PubMutexUnlock();
    return ret;
}

/**
************************************************************************************
* @brief           ONENET OTA  上报下载进度
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_ReportDownloadStatus(void)
{
    int32_t ret = osError;
    OTA_PubMutexLock();
    ret = OTA_HttpClientSend(ONENET_OTA_EVENT_REPORT_DOWNLOAD_RATE, OTA_HTTP_REQ_METHOD_POST);
    OTA_PubMutexUnlock();
    return ret;
}

/**
************************************************************************************
* @brief           ONENET OTA  上报升级状态
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_ReportUpgradeStatus(void)
{
    int32_t ret = osError;
    OTA_PubMutexLock();
    ret = OTA_HttpClientSend(ONENET_OTA_EVENT_REPORT_UPGRADE_STATUS, OTA_HTTP_REQ_METHOD_POST);
    OTA_PubMutexUnlock();
    return ret;
}

/**
 ************************************************************************************
 * @brief           ONENET OTA 升级开始
 *
 * @param[in]       void
 *
 * @return          osOK: 升级开始成功. 其它: 升级开始失败
 ************************************************************************************
*/
int32_t ONENET_OTA_Start(ONENET_OTA_Cb *callback)
{
    if(osOK != OTA_PubMutexCreate())
    {
        ONENET_OTA_API_ERR_PRINTF("OneNET OTA mutex create fail\r\n");
        return osError;
    }

    OTA_PubMutexLock();
    if(osOK != OTA_HttpCallbackRegister(callback))
    {
        ONENET_OTA_API_ERR_PRINTF("OneNET OTA cb register fail\r\n");
        return osErrorTimeout;
    }

    if(osOK != OTA_HttpClientInfoInit())
    {
        ONENET_OTA_API_ERR_PRINTF("OneNET OTA client init fail\r\n");
        return osErrorResource;
    }

    if(osOK != OTA_HttpTimeoutInit())
    {
        ONENET_OTA_API_ERR_PRINTF("OneNET OTA timeout init fail\r\n");
        return osErrorParameter;
    }

    if(osOK != OTA_HttpClientCreate()) /* 在升级开始的时候建立 TCP 连接, 一次升级任务只建立一次 TCP 连接 */
    {
        ONENET_OTA_API_ERR_PRINTF("OneNET OTA connect fail\r\n");
        return osErrorNoMemory;
    }
    OTA_PubMutexUnlock();

    return osOK;
}

/**
 ************************************************************************************
 * @brief           ONENET OTA 升级结束
 *
 * @param[in]       void
 *
 * @return          void
 ************************************************************************************
*/
void ONENET_OTA_Stop(void)
{
    OTA_HttpCallbackUnregister();
    OTA_PubMutexLock();
    OTA_HttpClientClose();
    OTA_PubMutexUnlock();
    OTA_PubMutexDelete();
    return;
}


#ifdef __cplusplus
}
#endif

