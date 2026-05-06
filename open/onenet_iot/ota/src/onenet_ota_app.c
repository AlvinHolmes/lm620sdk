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
#include "onenet_ota_api.h"
#include "onenet_ota_upgrade.h"
#include "onenet_ota_app.h"


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_APP_ON
 #ifdef OTA_APP_ON
#define OTA_APP_DBG_PRINTF                       osPrintf
#else
#define OTA_APP_DBG_PRINTF(...)
#endif
#define OTA_APP_ERR_PRINTF                       osPrintf

#define OTA_APP_TASK_STACK_SIZE                  (4096)
#define OTA_APP_MAX_QUEUE_NUM                    (32)
#define OTA_APP_TASK_PRIO                        (osPriorityNormal)
#define OTA_APP_RETRY_COUNT_MAX                  (3)     /* OTA HTTP(S) 升级下载重传最大次数 */

#define OTA_APP_RES_USERID                       "userid"
#define OTA_APP_RES_PROJECTID                    "projectid"
#define OTA_APP_RES_PRODUCTS                     "products"
#define OTA_APP_RES_DEVICES                      "devices"

#define OTA_APP_FIRMWARE_PATH                    "/usr/"

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
 typedef enum
{
    OTA_APP_TASK_TYPE_FOTA = 1,            /* fota 任务(默认值) */
    OTA_APP_TASK_TYPE_SOTA = 2,            /* sota 任务 */
} OTA_AppTaskType; /* OTA HTTP 升级任务类型 */

typedef enum
{
    OTA_APP_RES_TYPE_USER = 0,
    OTA_APP_RES_TYPE_PROJECT,
    OTA_APP_RES_TYPE_PRODUCT,
    OTA_APP_RES_TYPE_DEVICE,
    OTA_RES_TYPE_MORE,
} OTA_AppResType; /* res 类型 */

typedef struct
{
    uint8_t resType;         /* res 类型(用户或项目或产品) */
    uint8_t keyMethod;       /* 签名方法. 有 md5, sha1, sha256 */
    uint8_t taskType;        /* 升级任务类型 */
    uint8_t retryCnt;        /* 升级下载重传次数 */
    uint16_t upgradeStatus;  /* 升级状态 */
    uint32_t tid;            /* 任务 ID */
    uint32_t offset;         /* 升级版本的文件下载偏移量 */
    uint32_t totalSize;      /* 升级版本的文件大小 */
    int rangeStart;          /**< range start position in request header. */
    int rangeLen;            /**< range length in request header. */
    uint32_t et;             /*UNIX时间戳, 从1970年1月1日开始所经过的秒数. 转换成北京时间必须比当前
                               系统时间更大. 1836152085代表北京时间2028-03-09 02:14:45 */
    char *accessKey;         /* 对应的秘钥(用户秘钥或产品秘钥或设备秘钥). 秘钥是唯一的 */
    char *userId;            /* 用户 ID */
    char *projectId;         /* 项目 ID */
    char *productId;         /* 产品 ID */
    char *devName;           /* 设备名 */
    char *authorization;     /* 生成的安全秘钥鉴权 */
    char *filePath;          /* 差分包存放路径 */
    char *fileName;          /* 差分包名字 */
    char *firmwareVersion;   /* 模组版本号 */
    char *serverVersion;     /* 应用服务版本号 */
    char *devVersion;        /* 设备当前版本号 */
    char *userName;          /* HTTP(S) 登录的用户名 */
    char *password;          /* HTTP(S) 登录的密码 */
#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
    AT_SslContextEx *sslCtxEx; /* SSL 上下文参数 */
#endif
}OTA_AppParam; /* ONENET OTA 升级参数信息 */

typedef struct
{
    uint16_t msgId;     //   消息ID
}OTA_AppTaskMessage;

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static ONENET_OTA_Cb *g_OtaAppCb = NULL;
static OTA_AppParam *g_OtaAppParam = NULL;
static osThreadId_t g_OtaAppThreadId = NULL;
static osMessageQueueId_t g_OtaAppTaskMQ = NULL;

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
/* 释放内存 */
static void OTA_AppBufFree(char *buf)
{
     if(NULL != buf)
     {
         osFree(buf);
     }
     return;
}

static int32_t OTA_AppResponseCbMsgCheck(ONENT_OTA_RspInfo* rsp)
{
    if(NULL == rsp->msg)
    {
        OTA_APP_ERR_PRINTF("ota app ResponseCallback msg is null\r\n");
        return osError;
    }

    if((ONENET_OTA_RSP_COMMON_CODE_SUCC == rsp->code) && (!strcmp(rsp->msg, ONENET_OTA_RSP_MSG_SUCC)))
    {
        return osOK;
    }
    else
    {
        return osErrorTimeout;
    }
}

static int32_t OTA_AppResponseCbVersionCheck(ONENT_OTA_RspInfo* rsp, OTA_AppParam *param)
{
    if((NULL == param->firmwareVersion) || (NULL == param->serverVersion))
    {
        OTA_APP_ERR_PRINTF("ota app param version is null\r\n");
        return osErrorTimeout;
    }
    if((NULL == rsp->firmwareVersion) || (NULL == rsp->serverVersion))
    {
        OTA_APP_ERR_PRINTF("ota app ResponseCallback version is null\r\n");
        return osErrorResource;
    }

    if((!strcmp(param->firmwareVersion, rsp->firmwareVersion)) && (!strcmp(param->serverVersion, rsp->serverVersion)))
    {
        return osOK;
    }
    else
    {
        return osErrorParameter;
    }
}

static void OTA_AppSysReboot(void)
{
    osThreadMsSleep(500);
    /* 重启系统, 如果重启成功, 只执行到 osShutdown 这一步, 后面的不会执行*/
    OTA_APP_ERR_PRINTF("OTA app system reboot\r\n");
    osShutdown(OS_REBOOT, OS_REBOOT_FOTA);
    osThreadSuspend(osThreadSelf());
    OS_ASSERT(0);
    return;
}

/* 获取升级包路径 */
char* OTA_AppPathNameGet(char *filePath, char *fileName)
{
    char *pathName = OS_NULL;
    uint16_t pathNameLen = 0;

    if(OS_NULL == fileName)
    {
        OTA_APP_ERR_PRINTF("filename is null\r\n");
        return NULL;
    }

   if(OS_NULL == filePath)
   {
       filePath = OTA_APP_FIRMWARE_PATH;
   }

    /* 获取 /usr/ 路径下的文件名 fileName */
    pathNameLen = strlen(filePath) + strlen(fileName) + 1;
    pathName = osMalloc(pathNameLen);
    if(OS_NULL == pathName)
    {
        OTA_APP_ERR_PRINTF("Pathame malloc fail\r\n");
        return NULL;
    }
    memset(pathName, 0, pathNameLen);

    strcat(pathName, OTA_APP_FIRMWARE_PATH);
    strcat(pathName, fileName);

    return pathName;
}

static int32_t OTA_AppFirmwareOffsetGet(uint16_t event, OTA_AppParam *param)
{
    char *url = NULL;
    char *pathName = NULL;

    pathName = OTA_AppPathNameGet(param->filePath, param->fileName);
    if(NULL == pathName)
    {
        OTA_APP_ERR_PRINTF("ota app offset pathName is null\r\n");
        return osError;
    }

    url = ONENET_OTA_UrlGet(event);
    if(NULL == url)
    {
        OTA_APP_ERR_PRINTF("oat app url get fail\r\n", url);
        return osErrorTimeout;
    }
    OTA_APP_ERR_PRINTF("ota app get url[%s]\r\n", url);

    param->offset = OTA_UpgradeFirmwareOffsetGet(url, 0, pathName, param->totalSize);
    if(param->offset < 0)
    {
        OTA_APP_ERR_PRINTF("ota app dl pkg offset err[%d]\r\n", param->offset);
        return osErrorResource;
    }
    OTA_APP_ERR_PRINTF("ota app dl pkg offset\r\n", param->offset);

    osFree(pathName);
    pathName = NULL;

    return osOK;
}

static int32_t OTA_AppSaveFirmware(ONENT_OTA_RspInfo* rsp, OTA_AppParam *param)
{
    int32_t ret = -1;
    int32_t dlLen = rsp->bodyLen;
    char *pathName = OS_NULL;

    if((NULL == rsp->body) || (dlLen <= 0))
    {
        OTA_APP_ERR_PRINTF("ota app MethodCallback dl pkg is null\r\n");
        return osError;
    }

    pathName = OTA_AppPathNameGet(param->filePath, param->fileName);
    if(NULL == pathName)
    {
        OTA_APP_ERR_PRINTF("ota app save pathName is null\r\n");
        return osErrorTimeout;
    }

    /* 保存文件数据 */
    ret = OTA_UpgradeSaveFirmware(rsp->body, dlLen, pathName);
    if (osOK == ret)
    {
        /* 保存下载进度 */
        param->offset += dlLen;
    }
    else
    {
        OTA_APP_ERR_PRINTF("Save package data fail[%d]\r\n", ret);
    }

    osFree(pathName);
    pathName = NULL;

    return ret;
}

static int32_t OTA_AppUpgradeFirmware(OTA_AppParam *param)
{
    int32_t ret = osOK;
    char *pathName = OS_NULL;

    pathName = OTA_AppPathNameGet(param->filePath, param->fileName);
    if(NULL == pathName)
    {
        OTA_APP_ERR_PRINTF("ota app upgrade pathName is null\r\n");
        return osErrorTimeout;
    }

    ret = OTA_UpgradeFirmwareSizeCheck(param->totalSize, pathName);
    if(osOK != ret)
    {
        OTA_APP_ERR_PRINTF("ota app dl pkg check err[%d]\r\n", ret);
        osFree(pathName);
        pathName = NULL;
        return osErrorTimeout;
    }
    ret = OTA_UpgradeFirmware(pathName);

    osFree(pathName);
    pathName = NULL;

    return ret;
}

static int32_t OTA_AppDownloadFirmware(void)
{
    int32_t ret = -1;
    int32_t dlLen = -1;
    OTA_AppParam *param = g_OtaAppParam;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("ota   param dl pkg is null\r\n");
        return osError;
    }

    OTA_APP_DBG_PRINTF("ota download offset[%d], totalSize[%d]\r\n", param->offset, param->totalSize);
    if(param->offset < param->totalSize)
    {
        param->retryCnt = 0;
        dlLen = param->totalSize - param->offset;
        dlLen = dlLen < ONENET_OTA_DOWNLOAD_PRE_PKG_LEN ? dlLen : ONENET_OTA_DOWNLOAD_PRE_PKG_LEN;
        param->rangeStart = param->offset;
        param->rangeLen = dlLen;
        OTA_APP_DBG_PRINTF("has download byte[%u]\r\n", param->offset);
        while(param->retryCnt <= OTA_APP_RETRY_COUNT_MAX)
        {
            ret = ONENET_OTA_DownloadFirmware();
            if(osOK != ret)
            {
                OTA_APP_ERR_PRINTF("ota donwload retry count [%d]\r\n", param->retryCnt);
                if (param->retryCnt > OTA_APP_RETRY_COUNT_MAX)
                {
                    OTA_APP_ERR_PRINTF("ota donwload reach retry count [%d]\r\n", param->retryCnt);
                    param->retryCnt = 0;
                    ret = osErrorTimeout;
                    break;
                }
                else
                {
                    param->retryCnt += 1;
                }
            }
            else
            {
                ret = osOK;
                break;
            }
        }
    }
    else if(param->offset == param->totalSize)
    {
        OTA_APP_DBG_PRINTF("ota app upgrade pkg already exist, upgrade it\r\n");
        ret = OTA_AppUpgradeFirmware(param);
        if(osOK == ret)
        {
            OTA_AppTaskMessage msg;
            param->upgradeStatus = ONENET_OTA_STATUS_UPGRADE_SUCC;
            msg.msgId = ONENET_OTA_EVENT_REPORT_UPGRADE_STATUS;
            ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
        }
        //ret = osOK;
    }
    else
    {
        OTA_APP_ERR_PRINTF("ota donwload offset[%d] greater than totalSize[%d]\r\n", param->offset, param->totalSize);
        ret = osErrorResource;
    }

    return ret;
}

static int32_t OTA_AppParamReadCbProcess(uint16_t event, const void **buf, uint32_t *len)
{
    OTA_AppParam *param = g_OtaAppParam;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("ota app param is null\r\n");
        return osError;
    }

    *len = 0;

    switch(event)
    {
        case ONENET_OTA_PARAM_TASK_ID:
        {
            uint32_t **tmpBuf = (uint32_t**)buf;
            *tmpBuf = &param->tid;
            *len = sizeof(param->tid);
            break;
        }

        case ONENET_OTA_PARAM_FIRMWARE_SIZE:
        {
            uint32_t **tmpBuf = (uint32_t**)buf;
            *tmpBuf = &param->totalSize;
            *len = sizeof(param->totalSize);
            break;
        }

        case ONENET_OTA_PARAM_TASK_TYPE:
        {
            uint8_t **tmpBuf = (uint8_t**)buf;
            *tmpBuf = &param->taskType;
            *len = sizeof(param->taskType);
            break;
        }

        case ONENET_OTA_PARAM_USER_ID:
        {
            *buf = (void**)param->userId;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_PROJECT_ID:
        {
            *buf = (void*)param->projectId;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_PRODUCT_ID:
        {
            *buf = (void*)param->productId;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_DEV_NAME:
        {
            *buf = (void*)param->devName;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_AUTHORIZATION:
        {
            *buf = (void*)param->authorization;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_F_VERSION:
        {
            *buf = (void*)param->firmwareVersion;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_S_VERSION:
        {
            *buf = (void*)param->serverVersion;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_DEV_VERSION:
        {
            *buf = (void*)param->devVersion;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_FILEPATH:
        {
            *buf = (void*)param->filePath;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_FILENAME:
        {
            *buf = (void*)param->fileName;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_USERNAME:
        {
            *buf = (void*)param->userName;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_PASSWORD:
        {
            *buf = (void*)param->password;
            if(NULL != *buf)
            {
                *len = strlen(*buf);
            }
            break;
        }

        case ONENET_OTA_PARAM_RANGE_START:
        {
            int **tmpBuf = (int**)buf;
            *tmpBuf = &param->rangeStart;
            *len = sizeof(param->rangeStart);
            break;
        }

        case ONENET_OTA_PARAM_RANGE_LEN:
        {
            int **tmpBuf = (int**)buf;
            *tmpBuf = &param->rangeLen;
            *len = sizeof(param->rangeLen);
            break;
        }

        case ONENET_OTA_PARAM_UPGRADE_STATUS:
        {
            uint16_t **tmpBuf = (uint16_t**)buf;
            *tmpBuf = &param->upgradeStatus;
            *len = sizeof(param->upgradeStatus);
            break;
        }

        default:
            OTA_APP_ERR_PRINTF("ota   param cb event[%d]\r\n", event);
            return osErrorTimeout;
    }

    return osOK;
}

static int32_t OTA_AppResponseWriteCbProcess(uint16_t event, const void *buf, uint32_t len)
{
    int32_t ret = -1;
    OTA_AppParam *param = g_OtaAppParam;
    OTA_AppTaskMessage msg;
    ONENT_OTA_RspInfo* rsp = (ONENT_OTA_RspInfo*)buf;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("ota   param cb is null\r\n");
        return osError;
    }

    if(NULL == g_OtaAppTaskMQ)
    {
        OTA_APP_ERR_PRINTF("ota   param task MQ is null\r\n");
        return osError;
    }

    if(ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE != event)
    {
        if(NULL == rsp)
        {
            OTA_APP_ERR_PRINTF("ota app ResponseCallback rsp is null [%d]\r\n", event);
            return osErrorTimeout;
        }
    }

    OTA_APP_DBG_PRINTF("ota app ResponseCallback event[%d]\r\n", event);
    switch(event)
    {
        case ONENET_OTA_EVENT_UPGRADE_INIT:
        case ONENET_OTA_EVENT_UPGRADE_START:
            break;

        case ONENET_OTA_EVENT_REPORT_VERSION:
            ret = OTA_AppResponseCbMsgCheck(rsp);
            if(osOK == ret)
            {
                msg.msgId = ONENET_OTA_EVENT_INQUIRE_VERSION;
                ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
            }
            break;

        case ONENET_OTA_EVENT_INQUIRE_VERSION:
            ret = OTA_AppResponseCbMsgCheck(rsp);
            if(osOK == ret)
            {
                ret = OTA_AppResponseCbVersionCheck(rsp, param);
                if(osOK == ret)
                {
                    msg.msgId = ONENET_OTA_EVENT_CHECK_UPGRADE_TASK;
                    ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
                }
            }
            break;

        case ONENET_OTA_EVENT_CHECK_UPGRADE_TASK:
            ret = OTA_AppResponseCbMsgCheck(rsp);
            if(osOK == ret)
            {
                param->tid = rsp->tid;
                param->totalSize = rsp->size;
                msg.msgId = ONENET_OTA_EVENT_INQUIRE_TASK_STATUS;
                ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
            }
            break;

        case ONENET_OTA_EVENT_INQUIRE_TASK_STATUS:
            ret = OTA_AppResponseCbMsgCheck(rsp);
            if(osOK == ret)
            {
                ret = OTA_AppFirmwareOffsetGet(ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE, param);
                if(osOK == ret)
                {
                    msg.msgId = ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE;
                    ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
                }
            }
            break;

        case ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE:
            ret = OTA_AppSaveFirmware(rsp, param);
            if(osOK == ret)
            {
                if(param->offset == param->totalSize)
                {
                    param->rangeStart = 0;
                    param->rangeLen = 0;
                    param->upgradeStatus = ONENET_OTA_STATUS_DOWNLOAD_SUCC;
                    msg.msgId = ONENET_OTA_EVENT_REPORT_DOWNLOAD_RATE;
                }
                else
                {
                    msg.msgId = ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE;
                }
                ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
            }
            break;

        case ONENET_OTA_EVENT_REPORT_DOWNLOAD_RATE:
            ret = OTA_AppResponseCbMsgCheck(rsp);
            if(osOK == ret)
            {
                OTA_APP_DBG_PRINTF("ota app start upgrade pkg\r\n");
                ret = OTA_AppUpgradeFirmware(param);
                if(osOK == ret)
                {
                    param->upgradeStatus = ONENET_OTA_STATUS_UPGRADE_SUCC;
                    msg.msgId = ONENET_OTA_EVENT_REPORT_UPGRADE_STATUS;
                    ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
                }
            }
            break;

        case ONENET_OTA_EVENT_REPORT_UPGRADE_STATUS:
            ret = OTA_AppResponseCbMsgCheck(rsp);
            if(osOK == ret)
            {
                OTA_APP_DBG_PRINTF("OTA bin download succ\r\n");
                msg.msgId = ONENET_OTA_EVENT_UPGRADE_STOP;
                ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
                OTA_AppSysReboot();
            }
            break;

        case ONENET_OTA_EVENT_UPGRADE_ERR:
        case ONENET_OTA_EVENT_UPGRADE_STOP:
            msg.msgId = event;
            osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
            ret = osOK;
            break;

        default:
            OTA_APP_ERR_PRINTF("OTA app callback event err\r\n", event);
            break;
    }

    if(osOK != ret)
    {
        OTA_APP_ERR_PRINTF("OTA app send fail, stop\r\n");
        msg.msgId = ONENET_OTA_EVENT_UPGRADE_ERR;
        ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
    }

    return ret;
}

static void OTA_AppEventProcess(uint16_t event)
{
    int32_t ret = osOK;
    OTA_AppTaskMessage msg;

    OTA_APP_DBG_PRINTF("ota app process event[%d]\r\n", event);
    switch(event)
    {
        case ONENET_OTA_EVENT_UPGRADE_INIT:
        case ONENET_OTA_EVENT_UPGRADE_START: /* 开始升级 */
            msg.msgId = ONENET_OTA_EVENT_REPORT_VERSION;
            ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
            break;

        case ONENET_OTA_EVENT_REPORT_VERSION: /* 上报设备当前版本 */
            ret = ONENET_OTA_ReportVersion();
            break;

        case ONENET_OTA_EVENT_INQUIRE_VERSION: /* 查看设备版本号(可省略) */
            ret = ONENET_OTA_InquireVersion();
            break;

        case ONENET_OTA_EVENT_CHECK_UPGRADE_TASK: /* 检测升级任务 */
            ret = ONENET_OTA_CheckUpgradeTask();
            break;

        case ONENET_OTA_EVENT_INQUIRE_TASK_STATUS:/* 查询任务状态 */
            ret = ONENET_OTA_InquireTaskStatus();
            break;

        case ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE: /* 下载升级包 */
            ret = OTA_AppDownloadFirmware();
            break;

        case ONENET_OTA_EVENT_REPORT_DOWNLOAD_RATE: /* 上报下载进度 */
            ret = ONENET_OTA_ReportDownloadStatus();
            break;

        case ONENET_OTA_EVENT_REPORT_UPGRADE_STATUS: /* 上报升级状态 */
            ret = ONENET_OTA_ReportUpgradeStatus();
            break;

        case ONENET_OTA_EVENT_UPGRADE_ERR: /* 升级发生错误 */
            OTA_AppStop();
            ret = osOK;
            break;

        case ONENET_OTA_EVENT_UPGRADE_STOP: /* 结束升级 */
            OTA_AppStop();
            OTA_AppSysReboot();
            ret = osOK;
            break;

        default:
            OTA_APP_ERR_PRINTF("onenet ota cbEvent err[%d]\r\n", event);
            break;
    }

    if(osOK != ret)
    {
        OTA_APP_ERR_PRINTF("ota app event[%d] fail, stop task\r\n", event);
        msg.msgId = ONENET_OTA_EVENT_UPGRADE_ERR;
        ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);
    }

    return;
}

static int32_t OTA_AppEventStart(void )
{
    int32_t ret = -1;
    OTA_AppTaskMessage msg;

    ret = ONENET_OTA_Start(g_OtaAppCb);
    if(osOK == ret)
    {
        msg.msgId = ONENET_OTA_EVENT_UPGRADE_START;
    }
    else
    {
        msg.msgId = ONENET_OTA_EVENT_UPGRADE_ERR;
    }
    ret = osMessageQueuePut(g_OtaAppTaskMQ, &msg, 0, 0);

    return ret;
}

static void OTA_AppThreadProcess(void *param)
{
    while(1)
    {
        OTA_AppTaskMessage msg;
        if(osMessageQueueGet(g_OtaAppTaskMQ, &msg, 0, osWaitForever) == osOK)
        {
            OTA_AppEventProcess(msg.msgId);
        }
        else
        {
            OTA_APP_ERR_PRINTF("ota app get msg fail\r\n");
            break;
        }
    }
    return;
}

static int32_t OTA_AppThreadInit(void)
{
    osThreadAttr_t attr = {"OTA_TASK", osThreadDetached, NULL, 0U, NULL, OTA_APP_TASK_STACK_SIZE,
                            OTA_APP_TASK_PRIO, 0U, 0U};
    osMessageQueueAttr_t msgQueueAttr = {"OTA_MSG", 0U, NULL, 0U, NULL, 0U};

    if(NULL == g_OtaAppTaskMQ)
    {
        g_OtaAppTaskMQ = osMessageQueueNew(OTA_APP_MAX_QUEUE_NUM, sizeof(OTA_AppTaskMessage), &msgQueueAttr);
        if(NULL == g_OtaAppTaskMQ)
        {
            OTA_APP_ERR_PRINTF("ota app queue create fail\r\n");
            return osError;
        }
    }

    if(NULL == g_OtaAppThreadId)
    {
        g_OtaAppThreadId = osThreadNew(OTA_AppThreadProcess, NULL, &attr);
        if(NULL == g_OtaAppThreadId)
        {
            OTA_APP_ERR_PRINTF("ota app thread create fail\r\n");
            return osErrorTimeout;
        }
    }

    return osOK;
}

static void OTA_AppThreadDeInit(void)
{
    if(g_OtaAppTaskMQ != NULL)
    {
        osMessageQueueReset(g_OtaAppTaskMQ);
        OS_ASSERT(osMessageQueueGetCount(g_OtaAppTaskMQ) == 0); //   需要等消息处理完,否则消息体的内存会泄漏
        (void)osMessageQueueDelete(g_OtaAppTaskMQ);
        g_OtaAppTaskMQ = NULL;
    }

    if(g_OtaAppThreadId != NULL)
    {
        g_OtaAppThreadId = NULL;
    }

    return;
}

static int32_t OTA_AppCallbackInit(void)
{
    if(NULL != g_OtaAppCb)
    {
        OTA_APP_ERR_PRINTF("%s, ota app already start\r\n", __FUNCTION__);
        return osError;
    }

    g_OtaAppCb = (ONENET_OTA_Cb*)osMalloc(sizeof(ONENET_OTA_Cb));
    if(NULL == g_OtaAppCb)
    {
        OTA_APP_ERR_PRINTF("%s,   malloc fail\r\n", __FUNCTION__);
        return osErrorTimeout;
    }

    g_OtaAppCb->readFunc = OTA_AppParamReadCbProcess;
    g_OtaAppCb->writeFunc = OTA_AppResponseWriteCbProcess;

    return osOK;
}

static void OTA_AppCallbackDeinit(void)
{
    OTA_AppBufFree((char*)g_OtaAppCb);
    g_OtaAppCb = NULL;

    return;
}

static char* OTA_AppResGet(OTA_AppParam *param)
{
    uint16_t len = 0;
    char *res = NULL;

    switch(param->resType)
    {
        case OTA_APP_RES_TYPE_USER:
            if(NULL != param->userId)
            {
                len = strlen(OTA_APP_RES_USERID"/") + strlen(param->userId) + 1;/* +1 for store '\0' */
                res = osMalloc(len);
                if(NULL == res)
                {
                    OTA_APP_ERR_PRINTF("%s,   malloc fail\r\n", __FUNCTION__);
                    return NULL;
                }
                memset(res, 0, len);
                osSnprintf(res, len, "%s/%s", OTA_APP_RES_USERID, param->userId);
            }
            break;

        case OTA_APP_RES_TYPE_PROJECT:
            if(NULL != param->projectId)
            {
                len = strlen(OTA_APP_RES_PROJECTID"/") + strlen(param->projectId) + 1;/* +1 for store '\0' */
                res = osMalloc(len);
                if(NULL == res)
                {
                    OTA_APP_ERR_PRINTF("%s,   malloc fail\r\n", __FUNCTION__);
                    return NULL;
                }
                memset(res, 0, len);
                osSnprintf(res, len, "%s/%s", OTA_APP_RES_PROJECTID, param->projectId);
            }
            break;

        case OTA_APP_RES_TYPE_PRODUCT:
            if(NULL != param->productId)
            {
                len = strlen(OTA_APP_RES_PRODUCTS"/") + strlen(param->productId) + 1;/* +1 for store '\0' */
                res = osMalloc(len);
                if(NULL == res)
                {
                    OTA_APP_ERR_PRINTF("%s,   malloc fail\r\n", __FUNCTION__);
                    return NULL;
                }
                memset(res, 0, len);
                osSnprintf(res, len, "%s/%s", OTA_APP_RES_PRODUCTS, param->productId);
            }
            break;

        case OTA_APP_RES_TYPE_DEVICE:
            if((NULL != param->productId) && (NULL != param->devName))
            {
                len = strlen(OTA_APP_RES_PRODUCTS"/") + strlen(param->productId) + strlen("/") + \
                    strlen(OTA_APP_RES_DEVICES"/") + strlen(param->devName) + 1;/* +1 for store '\0' */
                res = osMalloc(len);
                if(NULL == res)
                {
                    OTA_APP_ERR_PRINTF("%s,   malloc fail\r\n", __FUNCTION__);
                    return NULL;
                }
                memset(res, 0, len);
                osSnprintf(res, len, "%s/%s/%s/%s", OTA_APP_RES_PRODUCTS, param->productId, OTA_APP_RES_DEVICES, param->devName);
                OTA_APP_DBG_PRINTF("%s, res[%s][%s][%s]\r\n", __FUNCTION__, res, param->productId, param->devName);
            }
            break;

        default:
            OTA_APP_ERR_PRINTF("%s, resType err[%d]\r\n", __FUNCTION__, param->resType);
            break;
    }

    return res;
}

static int32_t OTA_AppTokenGet(OTA_AppParam *param)
{
    int32_t ret = -1;
    char *res = NULL;
    uint16_t len = ONENET_OTA_TOKEN_AUTH_LEN_MAX + 1;

    res = OTA_AppResGet(param);
    if(NULL == res)
    {
        OTA_APP_ERR_PRINTF("%s, res is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL == param->authorization)
    {
        param->authorization = osMalloc(len);
        if(NULL == param->authorization)
        {
            OTA_APP_ERR_PRINTF("%s, authorization malloc fail", __FUNCTION__);
            return osErrorTimeout;
        }
    }
    memset(param->authorization, 0, len);
    ret = ONENET_OTA_TokenGenerate(res, param->et, param->accessKey, param->keyMethod, param->authorization);
    OTA_AppBufFree(res);

    return ret;
}



/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
/**
************************************************************************************
* @brief           ONENET OTA 配置UNIX时间戳
*
* @param[in]       uint32_t            UNIX时间戳
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppEtSet(uint32_t et)
{
    OTA_AppParam *param = g_OtaAppParam;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }
    param->et = et;

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置签名方法
*
* @param[in]       uint32_t            签名方法
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppKeyMethodSet(uint32_t keyMethod)
{
    OTA_AppParam *param = g_OtaAppParam;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }
    param->keyMethod = keyMethod;

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置秘钥
*
* @param[in]       accessKey          秘钥
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppAccessKeySet(char *accessKey)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != param->accessKey)
    {
        osFree(param->accessKey); /* 考虑到每次的字符串长度可能不一样, 当不为 NULL 时, 要先释放掉, 再申请新 buf */
        param->accessKey = NULL;
    }

    if(NULL != accessKey)
    {
        len = strlen(accessKey) + 1; /* +1 for store '\0' */
        param->accessKey = osMalloc(len);
        if(NULL == param->accessKey)
        {
            OTA_APP_ERR_PRINTF("%s, userId malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->accessKey, 0, len);
        memcpy(param->accessKey, accessKey, strlen(accessKey));
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置用户 ID
*
* @param[in]       userId            用户 ID
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppUserIdSet(char *userId)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != param->userId)
    {
        osFree(param->userId); /* 考虑到每次的字符串长度可能不一样, 当不为 NULL 时, 要先释放掉, 再申请新 buf */
        param->userId = NULL;
    }

    if(NULL != userId)
    {
        len = strlen(userId) + 1; /* +1 for store '\0' */
        param->userId = osMalloc(len);
        if(NULL == param->userId)
        {
            OTA_APP_ERR_PRINTF("%s, userId malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->userId, 0, len);
        memcpy(param->userId, userId, strlen(userId));
        param->resType = OTA_APP_RES_TYPE_USER;
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置项目 ID
*
* @param[in]       projectId            项目 ID
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppProjectIdSet(char *projectId)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != param->projectId)
    {
        osFree(param->projectId); /* 考虑到每次的字符串长度可能不一样, 当不为 NULL 时, 要先释放掉, 再申请新 buf */
        param->projectId = NULL;
    }

    if(NULL != projectId)
    {
        len = strlen(projectId) + 1; /* +1 for store '\0' */
        param->projectId = osMalloc(len);
        if(NULL == param->projectId)
        {
            OTA_APP_ERR_PRINTF("%s, projectId malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->projectId, 0, len);
        memcpy(param->projectId, projectId, strlen(projectId));
        param->resType = OTA_APP_RES_TYPE_PROJECT;
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置产品 ID
*
* @param[in]       productId            产品 ID
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppProductIdSet(char *productId)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != param->productId)
    {
        osFree(param->productId); /* 考虑到每次的字符串长度可能不一样, 当不为 NULL 时, 要先释放掉, 再申请新 buf */
        param->productId = NULL;
    }

    if(NULL != productId)
    {
        len = strlen(productId) + 1; /* +1 for store '\0' */
        param->productId = osMalloc(len);
        if(NULL == param->productId)
        {
            OTA_APP_ERR_PRINTF("%s, productId malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->productId, 0, len);
        memcpy(param->productId, productId, strlen(productId));
        param->resType = OTA_APP_RES_TYPE_PRODUCT;
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置设备名
*
* @param[in]       devName            设备名
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppDevNameSet(char *devName)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != param->devName)
    {
        osFree(param->devName); /* 考虑到每次的字符串长度可能不一样, 当不为 NULL 时, 要先释放掉, 再申请新 buf */
        param->devName = NULL;
    }

    if(NULL != devName)
    {
        len = strlen(devName) + 1; /* +1 for store '\0' */
        param->devName = osMalloc(len);
        if(NULL == param->devName)
        {
            OTA_APP_ERR_PRINTF("%s, devName malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->devName, 0, len);
        memcpy(param->devName, devName, strlen(devName));
        param->resType = OTA_APP_RES_TYPE_DEVICE;
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置差分包名字
*
* @param[in]       filePath            差分包存放路径
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppFilePathSet(char *filePath)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != param->filePath)
    {
        osFree(param->filePath); /* 考虑到每次的字符串长度可能不一样, 当已有版本号不为 NULL 时, 要先释放掉, 再申请新 buf */
        param->filePath = NULL;
    }

    if(NULL != filePath)
    {
        len = strlen(filePath) + 1; /* +1 for store '\0' */
        param->filePath = osMalloc(len);
        if(NULL == param->filePath)
        {
            OTA_APP_ERR_PRINTF("%s, filePath malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->filePath, 0, len);
        memcpy(param->filePath, filePath, strlen(filePath));
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置差分包名字
*
* @param[in]       fileName            需要下载升级的差分包名字
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppFileNameSet(char *fileName)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != param->fileName)
    {
        osFree(param->fileName); /* 考虑到每次的字符串长度可能不一样, 当已有版本号不为 NULL 时, 要先释放掉, 再申请新 buf */
        param->fileName = NULL;
    }

    if(NULL != fileName)
    {
        len = strlen(fileName) + 1; /* +1 for store '\0' */
        param->fileName = osMalloc(len);
        if(NULL == param->fileName)
        {
            OTA_APP_ERR_PRINTF("%s, fileName malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->fileName, 0, len);
        memcpy(param->fileName, fileName, strlen(fileName));
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置模组版本号
*
* @param[in]       firmwareVersion        模组版本号
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppFVersionSet(char *firmwareVersion)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != param->firmwareVersion)
    {
        osFree(param->firmwareVersion); /* 考虑到每次的字符串长度可能不一样, 当已有版本号不为 NULL 时, 要先释放掉, 再申请新 buf */
        param->firmwareVersion = NULL;
    }

    if(NULL != firmwareVersion)
    {
        len = strlen(firmwareVersion) + 1; /* +1 for store '\0' */
        param->firmwareVersion = osMalloc(len);
        if(NULL == param->firmwareVersion)
        {
            OTA_APP_ERR_PRINTF("%s, firmwareVersion malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->firmwareVersion, 0, len);
        memcpy(param->firmwareVersion, firmwareVersion, strlen(firmwareVersion));
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置应用服务版本号
*
* @param[in]       serverVersion        应用服务版本号
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppSVersionSet(char *serverVersion)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != param->serverVersion)
    {
        osFree(param->serverVersion); /* 考虑到每次的字符串长度可能不一样, 当已有版本号不为 NULL 时, 要先释放掉, 再申请新 buf */
        param->serverVersion = NULL;
    }

    if(NULL != serverVersion)
    {
        len = strlen(serverVersion) + 1; /* +1 for store '\0' */
        param->serverVersion = osMalloc(len);
        if(NULL == param->serverVersion)
        {
            OTA_APP_ERR_PRINTF("%s, serverVersion malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->serverVersion, 0, len);
        memcpy(param->serverVersion, serverVersion, strlen(serverVersion));
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 配置设备当前版本号
*
* @param[in]       devVersion        设备当前版本号
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppDevVersionSet(char *devVersion)
{
    OTA_AppParam *param = g_OtaAppParam;
    uint16_t len = 0;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(NULL != devVersion)
    {
        if(NULL != param->devVersion)
        {
            osFree(param->devVersion); /* 考虑到每次的字符串长度可能不一样, 当已有版本号不为 NULL 时, 要先释放掉, 再申请新 buf */
            param->devVersion = NULL;
        }
        len = strlen(devVersion) + 1; /* +1 for store '\0' */
        param->devVersion = osMalloc(len);
        if(NULL == param->devVersion)
        {
            OTA_APP_ERR_PRINTF("%s, devVersion malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
        memset(param->devVersion, 0, len);
        memcpy(param->devVersion, devVersion, strlen(devVersion));
    }
    else
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osErrorResource;
    }

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 升级参数初始化
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t OTA_AppParamInit(void)
{
    OTA_AppParam *param = g_OtaAppParam;

    if(NULL == param)
    {
        param = (OTA_AppParam*)osMalloc(sizeof(OTA_AppParam));
        if(NULL == param)
        {
            OTA_APP_ERR_PRINTF("%s, param malloc fail\r\n", __FUNCTION__);
            return osError;
        }
        g_OtaAppParam = param;
    }
    memset(param, 0, sizeof(OTA_AppParam));
    param->taskType = OTA_APP_TASK_TYPE_FOTA;
    param->accessKey = NULL;
    param->userId = NULL;
    param->projectId = NULL;
    param->productId = NULL;
    param->devName = NULL;
    param->authorization = NULL;
    param->filePath = NULL;
    param->fileName = NULL;
    param->firmwareVersion = NULL;
    param->serverVersion = NULL;
    param->devVersion = NULL;
    param->userName = NULL;
    param->password = NULL;
#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
    param->sslCtxEx = NULL;
#endif

    return osOK;
}

/**
************************************************************************************
* @brief           ONENET OTA 升级参数去初始化
*
* @param[in]       void
*
* @return          void
************************************************************************************
*/
void OTA_AppParamDeinit(void)
{
    OTA_AppParam *param = g_OtaAppParam;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, cfg is null, no need deinit", __FUNCTION__);
        return;
    }

    OTA_AppBufFree(param->accessKey);
    OTA_AppBufFree(param->userId);
    OTA_AppBufFree(param->projectId);
    OTA_AppBufFree(param->productId);
    OTA_AppBufFree(param->devName);
    OTA_AppBufFree(param->authorization);
    OTA_AppBufFree(param->filePath);
    OTA_AppBufFree(param->fileName);
    OTA_AppBufFree(param->firmwareVersion);
    OTA_AppBufFree(param->serverVersion);
    OTA_AppBufFree(param->devVersion);
    OTA_AppBufFree(param->userName);
    OTA_AppBufFree(param->password);
#ifdef ONENET_OTA_HTTP_SECURE_CONTROL
    OTA_AppBufFree((char*)param->sslCtxEx);
    param->sslCtxEx = NULL;
#endif
    param->accessKey = NULL;
    param->userId = NULL;
    param->projectId = NULL;
    param->productId = NULL;
    param->devName = NULL;
    param->authorization = NULL;
    param->filePath = NULL;
    param->fileName = NULL;
    param->firmwareVersion = NULL;
    param->serverVersion = NULL;
    param->devVersion = NULL;
    param->userName = NULL;
    param->password = NULL;

    OTA_AppBufFree((char*)g_OtaAppParam);
    g_OtaAppParam = NULL;
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
int32_t OTA_AppStart(void)
{
    OTA_AppParam *param = g_OtaAppParam;

    if(NULL == param)
    {
        OTA_APP_ERR_PRINTF("%s, param is null\r\n", __FUNCTION__);
        return osError;
    }

    if(osOK != OTA_AppTokenGet(param))
    {
        OTA_APP_ERR_PRINTF("%s, token get fail\r\n", __FUNCTION__);
        return osError;
    }

    if(osOK != OTA_AppCallbackInit())
    {
        OTA_APP_ERR_PRINTF("%s, cb fail\r\n", __FUNCTION__);
        return osErrorTimeout;
    }

    if(osOK != OTA_AppThreadInit())
    {
        OTA_AppStop();
        OTA_APP_ERR_PRINTF("%s, task fail\r\n", __FUNCTION__);
        return osErrorResource;
    }

    if(osOK != OTA_AppEventStart())
    {
        OTA_AppStop();
        OTA_APP_ERR_PRINTF("%s, event start fail\r\n", __FUNCTION__);
        return osErrorParameter;
    }

    return osOK;
}

/**
 ************************************************************************************
 * @brief           ONENET OTA 升级停止
 *
 * @param[in]       void
 *
 * @return          void
 ************************************************************************************
*/
void OTA_AppStop(void)
{
    OTA_AppCallbackDeinit();
    ONENET_OTA_Stop();
    OTA_AppThreadDeInit();

    return;
}


#ifdef __cplusplus
}
#endif

