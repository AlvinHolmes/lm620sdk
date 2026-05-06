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
#include "cJSON.h"
#include "onenet_ota_pub.h"
//#include "onenet_ota_api.h"
#include "onenet_ota_http.h"
#include "onenet_ota_rsp.h"


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_RSP_ON
 #ifdef OTA_RSP_ON
#define OTA_RSP_DBG_PRINTF                  OTA_PUB_PRINT_DEBUG
#else
#define OTA_RSP_DBG_PRINTF(...)
#endif
#define OTA_RSP_ERR_PRINTF                  OTA_PUB_PRINT_ERROR


#define OTA_RSP_CODE                        "code"
#define OTA_RSP_MSG                         "msg"
#define OTA_RSP_REQUEST_ID                  "request_id"
#define OTA_RSP_DATA                        "data"
#define OTA_RSP_FVERSION                    "f_version"
#define OTA_RSP_SVERSION                    "s_version"
#define OTA_RSP_TARGET                      "target"
#define OTA_RSP_TID                         "tid"
#define OTA_RSP_SIZE                        "size"
#define OTA_RSP_MD5                         "md5"
#define OTA_RSP_STATUS                      "status"
#define OTA_RSP_TYPE                        "type"

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/


/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static int32_t OTA_RspParseCommon(OTA_HttpClientInfo *clientInfo)
{
    cJSON *root = NULL;
    cJSON *item = NULL;
    const char *buf = clientInfo->clientData->response_buf;

    root = cJSON_Parse(buf);
    if(NULL == root)
    {
        OTA_RSP_ERR_PRINTF("RspCommonParse, root is null\r\n");
        return osErrorTimeout;
    }

    /* code */
    item = cJSON_GetObjectItem(root, OTA_RSP_CODE);
    if(NULL == item)
    {
        OTA_RSP_ERR_PRINTF("RspCommonParse, code is null\r\n");
        return osErrorResource;
    }
    if (item->valueint >= 0)
    {
        clientInfo->rsp.code = (uint16_t)item->valueint;
    }
    else
    {
        clientInfo->rsp.code = 0;
    }

    /* msg */
    item = cJSON_GetObjectItem(root, OTA_RSP_MSG);
    if(NULL == item)
    {
        OTA_RSP_ERR_PRINTF("RspCommonParse, msg is null\r\n");
        return osErrorParameter;
    }
    if(NULL == item->valuestring)
    {
        OTA_RSP_ERR_PRINTF("RspCommonParse, valuestring is null\r\n");
        return osErrorNoMemory;
    }
    uint16_t len = strlen(item->valuestring) + 1; /* +1 for store '\0' */
    clientInfo->rsp.msg = osMalloc(len);
    if(NULL == clientInfo->rsp.msg)
    {
        OTA_RSP_ERR_PRINTF("RspCommonParse, msg alloc fail\r\n");
        return osErrorNoMemory;
    }
    memset(clientInfo->rsp.msg, 0, len);
    memcpy(clientInfo->rsp.msg, item->valuestring, strlen(item->valuestring));

    cJSON_Delete(root);
    OTA_RSP_DBG_PRINTF("RspCommonParse, code[%d], msg[%s]\r\n", clientInfo->rsp.code, clientInfo->rsp.msg);

    return osOK;
}

static int32_t OTA_RspParseData(OTA_HttpClientInfo *clientInfo)
{
    cJSON *root = NULL;
    cJSON *item = NULL;
    cJSON *rootData = NULL;
    const char *buf = clientInfo->clientData->response_buf;

    root = cJSON_Parse(buf);
    if(NULL == root)
    {
        OTA_RSP_ERR_PRINTF("RspDataParse, root is null\r\n");
        return osErrorTimeout;
    }

    /* data */
    rootData = cJSON_GetObjectItem(root, OTA_RSP_DATA);
    if(NULL == rootData)
    {
        OTA_RSP_ERR_PRINTF("RspDataParse, data is null\r\n");
        return osErrorResource;
    }

    /* data.tid */
    item = cJSON_GetObjectItem(rootData, OTA_RSP_TID);
    if(item != NULL)
    {
        if (item->valueint >= 0)
        {
            clientInfo->rsp.tid = (uint32_t)item->valueint;
        }
        else
        {
            clientInfo->rsp.tid = 0;
        }
        OTA_RSP_DBG_PRINTF("RspDataParse, tid[%d]\r\n", clientInfo->rsp.tid);
    }

    /* data.size */
    item = cJSON_GetObjectItem(rootData, OTA_RSP_SIZE);
    if(item != NULL)
    {
        if (item->valueint >= 0)
        {
            clientInfo->rsp.size = (uint32_t)item->valueint;
        }
        else
        {
            clientInfo->rsp.size = 0;
        }
        OTA_RSP_DBG_PRINTF("RspDataParse, size[%d]\r\n", clientInfo->rsp.size);
    }

    /* data.status */
    item = cJSON_GetObjectItem(rootData, OTA_RSP_STATUS);
    if(item != NULL)
    {
        if (item->valueint >= 0)
        {
            clientInfo->rsp.status = (uint32_t)item->valueint;
        }
        else
        {
            clientInfo->rsp.status = ONENET_OTA_TASK_STATUS_NONE;
        }
        OTA_RSP_DBG_PRINTF("RspDataParse, status[%d]\r\n", clientInfo->rsp.status);
    }

    /* data.type */
    item = cJSON_GetObjectItem(rootData, OTA_RSP_TYPE);
    if(item != NULL)
    {
        if (item->valueint >= 0)
        {
            clientInfo->rsp.taskType = (uint8_t)item->valueint;
        }
        else
        {
            clientInfo->rsp.taskType = ONENET_OTA_TASK_TYPE_NONE;
        }
        OTA_RSP_DBG_PRINTF("RspDataParse, taskType[%d]\r\n", clientInfo->rsp.taskType);
    }

    /* data.f_version */
    item = cJSON_GetObjectItem(rootData, OTA_RSP_FVERSION);
    if ((item != NULL) && (item->valuestring != NULL))
    {
        uint16_t len = strlen(item->valuestring) + 1; /* +1 for store '\0' */
        clientInfo->rsp.firmwareVersion = osMalloc(len);
        if(NULL == clientInfo->rsp.firmwareVersion)
        {
            OTA_RSP_ERR_PRINTF("RspDataParse, f_version malloc fail\r\n");
            return osErrorParameter;
        }
        memset(clientInfo->rsp.firmwareVersion, 0, len);
        memcpy(clientInfo->rsp.firmwareVersion, item->valuestring, strlen(item->valuestring));
        OTA_RSP_DBG_PRINTF("RspDataParse, f_version[%s]\r\n", clientInfo->rsp.firmwareVersion);
    }

    /* data.s_version */
    item = cJSON_GetObjectItem(rootData, OTA_RSP_SVERSION);
    if ((item != NULL) && (item->valuestring != NULL))
    {
        uint16_t len = strlen(item->valuestring) + 1; /* +1 for store '\0' */
        clientInfo->rsp.serverVersion = osMalloc(len);
        if(NULL == clientInfo->rsp.serverVersion)
        {
            OTA_RSP_ERR_PRINTF("RspDataParse, s_version alloc fail\r\n");
            OTA_PubBufFree(clientInfo->rsp.firmwareVersion);
            clientInfo->rsp.firmwareVersion = NULL;
            return osErrorNoMemory;
        }
        memset(clientInfo->rsp.serverVersion, 0, len);
        memcpy(clientInfo->rsp.serverVersion, item->valuestring, strlen(item->valuestring));
        OTA_RSP_DBG_PRINTF("RspDataParse, s_version[%s]\r\n", clientInfo->rsp.serverVersion);
    }

    /* data.target */
    item = cJSON_GetObjectItem(rootData, OTA_RSP_TARGET);
    if ((item != NULL) && (item->valuestring != NULL))
    {
        uint16_t len = strlen(item->valuestring) + 1; /* +1 for store '\0' */
        clientInfo->rsp.target = osMalloc(len);
        if(NULL == clientInfo->rsp.target)
        {
            OTA_RSP_ERR_PRINTF("RspDataParse, target alloc fail\r\n");
            OTA_PubBufFree(clientInfo->rsp.firmwareVersion);
            OTA_PubBufFree(clientInfo->rsp.serverVersion);
            clientInfo->rsp.firmwareVersion = NULL;
            clientInfo->rsp.serverVersion = NULL;
            return osErrorISR;
        }
        memset(clientInfo->rsp.target, 0, len);
        memcpy(clientInfo->rsp.target, item->valuestring, strlen(item->valuestring));
        OTA_RSP_DBG_PRINTF("RspDataParse, target[%s]\r\n", clientInfo->rsp.target);
    }

    /* data.md5 */
    item = cJSON_GetObjectItem(rootData, OTA_RSP_MD5);
    if ((item != NULL) && (item->valuestring != NULL))
    {
        uint16_t len = strlen(item->valuestring) + 1; /* +1 for store '\0' */
        clientInfo->rsp.md5 = osMalloc(len);
        if(NULL == clientInfo->rsp.md5)
        {
            OTA_RSP_ERR_PRINTF("RspDataParse, md5 alloc fail\r\n");
            OTA_PubBufFree(clientInfo->rsp.firmwareVersion);
            OTA_PubBufFree(clientInfo->rsp.serverVersion);
            OTA_PubBufFree(clientInfo->rsp.target);
            clientInfo->rsp.firmwareVersion = NULL;
            clientInfo->rsp.serverVersion = NULL;
            clientInfo->rsp.target = NULL;
            return osErrorResourceFull;
        }
        memset(clientInfo->rsp.md5, 0, len);
        memcpy(clientInfo->rsp.md5, item->valuestring, strlen(item->valuestring));
        OTA_RSP_DBG_PRINTF("RspDataParse, md5[%s]\r\n", clientInfo->rsp.md5);
    }

    cJSON_Delete(root);

    return osOK;
}

static int32_t OTA_RspParseReportVersion(OTA_HttpClientInfo *clientInfo)
{
    int32_t ret = -1;

    ret = OTA_RspParseCommon(clientInfo);
    if(osOK != ret)
    {
        OTA_RSP_ERR_PRINTF("ReportVersionRspParse, common fail\r\n");
        return osError;
    }

    return osOK;
}

static int32_t OTA_RspParseInquireVersion(OTA_HttpClientInfo *clientInfo)
{
    int32_t ret = -1;

    ret = OTA_RspParseCommon(clientInfo);
    if(osOK != ret)
    {
        OTA_RSP_ERR_PRINTF("InquireVersionRspParse, common fail\r\n");
        return osError;
    }

    ret = OTA_RspParseData(clientInfo);
    if(osOK != ret)
    {
        OTA_RSP_ERR_PRINTF("InquireVersionRspParse, data fail\r\n");
        return osErrorTimeout;
    }

    return osOK;
}

static int32_t OTA_RspParseCheckUpgradeTask(OTA_HttpClientInfo *clientInfo)
{
    int32_t ret = -1;

    ret = OTA_RspParseCommon(clientInfo);
    if(osOK != ret)
    {
        OTA_RSP_ERR_PRINTF("CheckUpgradeTaskRspParse, common fail\r\n");
        return osError;
    }

    ret = OTA_RspParseData(clientInfo);
    if(osOK != ret)
    {
        OTA_RSP_ERR_PRINTF("CheckUpgradeTaskRspParse, data fail\r\n");
        return osErrorTimeout;
    }

    return osOK;
}

static int32_t OTA_RspParseInquireTaskStatus(OTA_HttpClientInfo *clientInfo)
{
    int32_t ret = -1;

    ret = OTA_RspParseCommon(clientInfo);
    if(osOK != ret)
    {
        OTA_RSP_ERR_PRINTF("InquireTaskStatusRspParse, common fail\r\n");
        return osError;
    }

    ret = OTA_RspParseData(clientInfo);
    if(osOK != ret)
    {
        OTA_RSP_ERR_PRINTF("InquireTaskStatusRspParse, data fail\r\n");
        return osErrorTimeout;
    }

    return osOK;
}

static int32_t OTA_RspParseDownloadPkg(OTA_HttpClientInfo *clientInfo)
{
    int32_t len = 0;
    const char *buf = clientInfo->clientData->response_buf;

    len = clientInfo->clientData->response_content_len;
    if(len > ONENET_OTA_DOWNLOAD_PRE_PKG_LEN)
    {
        OTA_RSP_ERR_PRINTF("dl pkg RspParse, len too large[%d]\r\n", len);
        return osErrorTimeout;
    }
    clientInfo->rsp.body = osMalloc(len);
    if(NULL == clientInfo->rsp.body)
    {
        OTA_RSP_ERR_PRINTF("DlRspParse, body malloc fail\r\n");
        return osError;
    }
    memset(clientInfo->rsp.body, 0, len);
    clientInfo->rsp.bodyLen = len;
    memcpy(clientInfo->rsp.body, buf, len);

    return osOK;
}

static int32_t OTA_RspParseReportUpgradeStatus(OTA_HttpClientInfo *clientInfo)
{
    int32_t ret = -1;

    ret = OTA_RspParseCommon(clientInfo);
    if(osOK != ret)
    {
        OTA_RSP_ERR_PRINTF("ReportUpgradeStatusRspParse, common fail\r\n");
        return osError;
    }

    return osOK;
}


/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
/* 解析 OTA HTTP(S) GET/POST/PUT 的响应 */
int32_t OTA_RspParse(uint16_t event, const char *buf)
{
    int32_t ret = osOK;
    OTA_HttpClientInfo *clientInfo = (OTA_HttpClientInfo *)buf;

    if(NULL == clientInfo->clientData->response_buf)
    {
        OTA_RSP_ERR_PRINTF("%s, response_buf is null\r\n", __FUNCTION__);
        return osError;
    }

    switch(event)
    {
        case ONENET_OTA_EVENT_UPGRADE_START: /* 开始升级 */
            break;

        case ONENET_OTA_EVENT_REPORT_VERSION: /* 上报设备当前版本 */
            ret = OTA_RspParseReportVersion(clientInfo);
            break;

        case ONENET_OTA_EVENT_INQUIRE_VERSION: /* 查看设备版本号(可省略) */
            ret = OTA_RspParseInquireVersion(clientInfo);
            break;

        case ONENET_OTA_EVENT_CHECK_UPGRADE_TASK: /* 检测升级任务 */
            ret = OTA_RspParseCheckUpgradeTask(clientInfo);
            break;

        case ONENET_OTA_EVENT_INQUIRE_TASK_STATUS:/* 查询任务状态 */
            ret = OTA_RspParseInquireTaskStatus(clientInfo);
            break;

        case ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE: /* 下载升级包 */
            OTA_RspParseDownloadPkg(clientInfo);
            break;

        case ONENET_OTA_EVENT_REPORT_DOWNLOAD_RATE: /* 上报下载进度 */
        case ONENET_OTA_EVENT_REPORT_UPGRADE_STATUS: /* 上报升级状态 */
            ret = OTA_RspParseReportUpgradeStatus(clientInfo);
            break;

        case ONENET_OTA_EVENT_UPGRADE_ERR: /* 升级发生错误 */
            break;

        case ONENET_OTA_EVENT_UPGRADE_STOP: /* 结束升级 */
            break;

        default:
            OTA_RSP_ERR_PRINTF("onenet ota cbEvent err[%d]\r\n", event);
            break;
    }

    return ret;
}


#ifdef __cplusplus
}
#endif

