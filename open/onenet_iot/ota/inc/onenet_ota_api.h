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

#ifndef __ONENET_OTA_API_H__
#define __ONENET_OTA_API_H__

#include "http_application_api.h"
#include "app_at_ssl.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#ifdef CONFIG_HTTP_SECURE
#define ONENET_OTA_HTTP_SECURE_CONTROL
#else
#endif

#define ONENET_OTA_DOWNLOAD_PRE_PKG_LEN               (1024)  /* ONENET OTA 升级时每次下载一包的包长度 */
#define ONENET_OTA_TOKEN_AUTH_LEN_MAX                 (255)
#define ONENET_OTA_RSP_MSG_SUCC                       "succ"


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum
{
    ONENET_OTA_PARAM_TASK_ID, /* tid */
    ONENET_OTA_PARAM_FIRMWARE_SIZE, /* 固件升级包大小 */
    ONENET_OTA_PARAM_TASK_TYPE, /* 升级任务类型 */
    ONENET_OTA_PARAM_USER_ID, /* 用户 ID */
    ONENET_OTA_PARAM_PROJECT_ID, /* 项目 ID */
    ONENET_OTA_PARAM_PRODUCT_ID, /* 产品 ID */
    ONENET_OTA_PARAM_DEV_NAME, /* 设备名 */
    ONENET_OTA_PARAM_AUTHORIZATION, /* 生成的安全秘钥鉴权 */
    ONENET_OTA_PARAM_F_VERSION, /* 模组版本号 */
    ONENET_OTA_PARAM_S_VERSION, /* 应用服务版本号 */
    ONENET_OTA_PARAM_DEV_VERSION, /* 升级之前的旧版本号 */
    ONENET_OTA_PARAM_FILEPATH, /* 升级包路径(如: /usr/ 路径)*/
    ONENET_OTA_PARAM_FILENAME, /* 升级包名字(1-32 字节) */
    ONENET_OTA_PARAM_USERNAME, /* 用户名 */
    ONENET_OTA_PARAM_PASSWORD, /* 密码 */
    ONENET_OTA_PARAM_RANGE_START, /* range 开始值 */
    ONENET_OTA_PARAM_RANGE_LEN, /* range 长度 */
    ONENET_OTA_PARAM_UPGRADE_STATUS, /* 升级状态 */
    ONENET_OTA_PARAM_MORE,
}ONENET_OTA_ParamEvent; /* ONENET OTA 参数获取的回调事件类型 */

typedef enum
{
    ONENET_OTA_EVENT_UPGRADE_INIT                   = 0, /* 初始状态 */
    ONENET_OTA_EVENT_UPGRADE_START                  = 1, /* 开始升级 */
    ONENET_OTA_EVENT_REPORT_VERSION                 = 2, /* 上报设备当前版本 */
    ONENET_OTA_EVENT_INQUIRE_VERSION                = 3, /* 查看设备版本号(可省略) */
    ONENET_OTA_EVENT_CHECK_UPGRADE_TASK             = 4, /* 检测升级任务 */
    ONENET_OTA_EVENT_INQUIRE_TASK_STATUS            = 5, /* 查询任务状态 */
    ONENET_OTA_EVENT_DOWNLOAD_FIRMWARE              = 6, /* 下载升级包 */
    ONENET_OTA_EVENT_REPORT_DOWNLOAD_RATE           = 7, /* 上报下载进度 */
    ONENET_OTA_EVENT_REPORT_UPGRADE_STATUS          = 8, /* 上报升级状态 */
    ONENET_OTA_EVENT_UPGRADE_ERR                    = 9, /* 升级发生错误 */
    ONENET_OTA_EVENT_UPGRADE_STOP                   = 10, /* 结束升级 */
    ONENET_OTA_EVENT_MORE,
}ONENET_OTA_UpgradeEvent; /* ONENET OTA 升级任务事件 */

typedef enum
{
    ONENET_OTA_TASK_STATUS_NONE                      = 0,
    ONENET_OTA_TASK_STATUS_WAIT                      = 1,   /* 待升级 */
    ONENET_OTA_TASK_STATUS_DOWNLOADING               = 2,   /* 下载中 */
    ONENET_OTA_TASK_STATUS_UPGRADING                 = 3,   /* 升级中 */
    ONENET_OTA_TASK_STATUS_UPGRADE_SUCC              = 4,   /* 升级成功 */
    ONENET_OTA_TASK_STATUS_UPGRADE_FAIL              = 5,   /* 升级失败 */
    ONENET_OTA_TASK_STATUS_UPGRADE_CANCLE            = 6,   /* 升级取消 */
} ONENET_OTA_TaskStatus; /* ONENET OTA 升级任务状态 */

typedef enum
{
    ONENET_OTA_TASK_TYPE_NONE                        = 0,
    ONENET_OTA_TASK_TYPE_FOTA                        = 1,    /* fota 任务(默认值) */
    ONENET_OTA_TASK_TYPE_SOTA                        = 2,    /* sota 任务 */
} ONENET_OTA_TaskType; /* ONENET OTA 升级任务类型 */

typedef enum
{
    ONENET_OTA_STATUS_DOWNLOAD_SUCC                   = 101,  /* 升级包下载成功(设备状态变成：升级中) */
    ONENET_OTA_STATUS_DOWNLOAD_NO_MEMORY              = 102,  /* 下载失败, 空间不足(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_DOWNLOAD_OVER_MEMORY            = 103,  /* 下载失败, 内存溢出（设备状态变成：升级失败） */
    ONENET_OTA_STATUS_DOWNLOAD_REQ_TIMEOUT            = 104,  /* 下载失败, 下载请求超时(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_DOWNLOAD_NO_POWER               = 105,  /* 下载失败, 电量不足(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_DOWNLOAD_NO_SIGNAL              = 106,  /* 下载失败, 信号不良(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_DOWNLOAD_UNKNOWN_ERR            = 107,  /* 下载失败, 未知异常(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_UPGRADE_SUCC                    = 201,  /* 升级成功, 此时会把设备的版本号修改为任务的目标版本(设备状态变成：升级完成) */
    ONENET_OTA_STATUS_UPGRADE_NO_POWER                = 202,  /* 升级失败, 电量不足(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_UPGRADE_OVER_MEMORY             = 203,  /* 升级失败, 内存溢出(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_UPGRADE_VERSION_ERR             = 204,  /* 升级失败, 升级包与当前任务目标版本不一致(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_UPGRADE_MD5_CHECK_FAIL          = 205,  /* 升级失败, MD5校验失败(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_UPGRADE_UNKNOWN_ERR             = 206,  /* 升级失败, 未知异常(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_MAX_RETRY_FAIL                  = 207,  /* 达到最大重试次数(设备状态变成：升级失败) */
    ONENET_OTA_STATUS_MORE,
}ONENET_OTA_StatusCode;        /* ONENET OTA 升级状态码 */

typedef enum
{
    ONENET_OTA_RSP_COMMON_CODE_SUCC                    = 0,    /* 调用成功 */
    ONENET_OTA_RSP_COMMON_CODE_OTHER_FAIL              = 500,  /* 其他原因调用失败(需再结合msg参数辅助判断)*/
    ONENET_OTA_RSP_COMMON_CODE_PARAM_ERR               = 10001, /* 参数错误 */
    ONENET_OTA_RSP_COMMON_CODE_AUTH_ERR                = 10403, /* 鉴权错误 */
    ONENET_OTA_RSP_COMMON_CODE_OPT_ERR                 = 10407, /* 用户业务操作错误 */
    ONENET_OTA_RSP_COMMON_CODE_IN_SERVER_ERR           = 10500, /* 内部服务错误 */
    ONENET_OTA_RSP_COMMON_CODE_MORE,
}ONENET_OTA_RspCommonCode;        /* ONENET OTA 响应的综合错误码 */

typedef enum
{
    ONENET_OTA_RSP_PRODUCT_CODE_NO_PRODUCT              = 10408, /* 产品不存在 */
    ONENET_OTA_RSP_PRODUCT_CODE_NOT_CREAT_DEV           = 10409, /* 产品未创建设备 */
    ONENET_OTA_RSP_PRODUCT_CODE_MORE,
}ONENET_OTA_RspProductCode;        /* OTA HTTP(S) 响应的产品相关错误码 */

typedef enum
{
    ONENET_OTA_RSP_DEV_CODEEXIST                        = 10406,  /* 新增设备失败: 设备已存在 */
    ONENET_OTA_RSP_DEV_CODE_NO_DEV                      = 10410,  /* 设备不存在 */
    ONENET_OTA_RSP_DEV_CODE_ATTR_SET_FAIL               = 10411,  /* 设备属性设置失败 */
    ONENET_OTA_RSP_DEV_CODE_ATTR_EXPECT_SET_FAIL        = 10412,  /* 设备属性期望设置失败 */
    ONENET_OTA_RSP_DEV_CODE_ATTR_EXPECT_INQ_FAIL        = 10413,  /* 设备属性期望查询失败 */
    ONENET_OTA_RSP_DEV_CODE_ATTR_INQ_FAIL               = 10414,  /* 设备属性获取失败 */
    ONENET_OTA_RSP_DEV_CODE_SERVER_CALL_FAIL            = 10415,  /* 设备服务调用失败 */
    ONENET_OTA_RSP_DEV_CODE_ATTR_EXPECT_DEL_FAIL        = 10416,  /* 设备属性期望删除失败 */
    ONENET_OTA_RSP_DEV_CODE_NEW_INQ_FAIL                = 10417,  /* 设备最新数据查询失败 */
    ONENET_OTA_RSP_DEV_CODE_ATTR_OLD_INQ_FAIL           = 10418,  /* 设备属性历史数据查询失败 */
    ONENET_OTA_RSP_DEV_CODE_EVENT_OLD_INQ_FAIL          = 10419,  /* 设备事件历史数据查询失败 */
    ONENET_OTA_RSP_DEV_CODE_OPT_INQ_FAIL                = 10420,  /* 设备操作记录查询失败 */
    ONENET_OTA_RSP_DEV_CODE_OFFLINE                     = 10421,  /* 设备不在线 */
    ONENET_OTA_RSP_DEV_CODE_MORE,
}ONENET_OTA_RspDevCode;        /* ONENET OTA 响应的设备相关错误码 */

typedef enum
{
    ONENET_OTA_RSP_FILE_CODE_EXIST                      = 10422,  /* 文件已存在 */
    ONENET_OTA_RSP_FILE_CODE_NOT_EXIST                  = 10423,  /* 文件不存在 */
    ONENET_OTA_RSP_FILE_CODE_LIMIT                      = 10424,  /* 文件限制 */
    ONENET_OTA_RSP_FILE_CODE_SIZE_DISAGREE              = 10425,  /* 文件大小不一致 */
    ONENET_OTA_RSP_FILE_CODE_MD5_DISAGREE               = 10426,  /* 文件MD5不一致*/
    ONENET_OTA_RSP_FILE_CODE_MORE,
}OTA_HttpRspErrorCode;        /* ONENET OTA 响应的文件错误码 */

typedef enum
{
    ONENET_OTA_RSP_LBS_CODE_INVALID_REQ                 = 13027,  /* invalid request 非法请求 */
    ONENET_OTA_RSP_LBS_CODE_NOT_FOUND                   = 13028,  /* not found 未找到对应信息 */
    ONENET_OTA_RSP_LBS_CODE_FORMAT_ERR                  = 13029,  /* format error 转换异常 */
    ONENET_OTA_RSP_LBS_CODE_SERVICE_INTERNAL_ERR        = 13030,  /* service internal error 服务内部错误 */
    ONENET_OTA_RSP_LBS_CODE_INVALID_JSON                = 13031,  /* nvalid JSON 转换JSON格式数据异常 */
    ONENET_OTA_RSP_LBS_CODE_INVALID_PARAM               = 13032,  /* invalid parameter 非法参数 */
    ONENET_OTA_RSP_LBS_CODE_PARAM_REQUIRED              = 13033,  /* parameter required 参数缺失 */
    ONENET_OTA_RSP_LBS_CODE_MORE,
}ONENET_OTA_RspLbsCode;        /* ONENET OTA 响应LBS位置相关错误码 */

typedef enum
{
    ONENET_OTA_RSP_OTA_CODE_PARAM_REQUIRED               = 12001,  /* parameter required 必填参数未填 */
    ONENET_OTA_RSP_OTA_CODE_NOT_FOUND                    = 12002,  /* not found 对象未找到 */
    ONENET_OTA_RSP_OTA_CODE_OBJECT_EXIST                 = 12003,  /* object already exists 对象已存在 */
    ONENET_OTA_RSP_OTA_CODE_FORMATE_ERR                  = 12004,  /* format error 格式错误 */
    ONENET_OTA_RSP_OTA_CODE_TASK_TYPE_ERR                = 12010,  /* task type error 任务类型错误 */
    ONENET_OTA_RSP_OTA_CODE_START_VERSION_ERR            = 12011,  /* start version error 任务起始版本错误 */
    ONENET_OTA_RSP_OTA_CODE_TASK_SUCC                    = 12013,  /* task succ 任务成功 */
    ONENET_OTA_RSP_OTA_CODE_TASK_EXPIRE                  = 12014,  /* task expire 任务已过期 */
    ONENET_OTA_RSP_OTA_CODE_INVALID_PARAM                = 12015,  /* invalid parameter 非法参数 */
    ONENET_OTA_RSP_OTA_CODE_INVALID_STEP                 = 12016,  /* invalid step 上报步骤非法 */
    ONENET_OTA_RSP_OTA_CODE_FILE_TOO_LARGE               = 12020,  /* file is too large. 文件过大 */
    ONENET_OTA_RSP_OTA_CODE_MORE,
}ONENET_OTA_RspOtaCode;        /* ONENET OTA 响应的 OTA 相关错误码 */

typedef struct
{
    int32_t (*readFunc)(uint16_t event, const void **buf, uint32_t *len);
    int32_t (*writeFunc)(uint16_t event, const void *buf, uint32_t len);
}ONENET_OTA_Cb;

typedef struct
{
    uint16_t code;          /* 响应错误码 */
    uint32_t tid;           /* 任务 ID */
    uint32_t size;          /* 升级版本的文件大小 */
    uint32_t status;        /* 状态 */
    uint8_t taskType;       /* 任务类型 */
    int bodyLen;            /* HTTP(S) 响应体内容长度 */
    char *msg;              /* 响应信息 */
    char *firmwareVersion;  /* 模组版本号 */
    char *serverVersion;    /* 应用服务版本号 */
    char *target;           /* 目标版本 */
    char *md5;              /* md5 */
    char *reqId;            /* 请求 ID */
    char *body;             /* HTTP(S) 响应体内容 */
}ONENT_OTA_RspInfo; /* ONENET OTA 响应信息 */

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
/**
 ***********************************************************************************************
 * @brief           1. Token 的签名生成算法为:
                    sign = base64(hmac_<method>(base64decode(accessKey), utf-8(StringForSignature)))
 *                  2. StringForSignature组成:
 *                  StringForSignature = et + '\n' + method + '\n' + res+ '\n' + version
 *
 * @param[in]       res            格式为: userid/xxx 或 userid/xxx 或 products/xxx/devices/xxx
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
int32_t ONENET_OTA_TokenGenerate(const char *res, uint32_t et, const char *accessKey, uint8_t method, char *authorization);

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
char* ONENET_OTA_UrlGet(uint16_t event);


/**
************************************************************************************
* @brief           ONENET OTA 上报设备当前版本
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_ReportVersion(void);

/**
************************************************************************************
* @brief           ONENET OTA 查看设备版本号
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_InquireVersion(void);

/**
************************************************************************************
* @brief           ONENET OTA 检测升级任务
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_CheckUpgradeTask(void);

/**
************************************************************************************
* @brief           ONENET OTA 查询任务状态
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_InquireTaskStatus(void);

/**
************************************************************************************
* @brief           ONENET OTA 下载升级包
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_DownloadFirmware(void);

/**
************************************************************************************
* @brief           ONENET OTA 上报下载进度
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_ReportDownloadStatus(void);

/**
************************************************************************************
* @brief           ONENET OTA 上报升级状态
*
* @param[in]       void
*
* @return          osOK: 成功. 其它: 失败
************************************************************************************
*/
int32_t ONENET_OTA_ReportUpgradeStatus(void);

/**
 ************************************************************************************
 * @brief           ONENET OTA 升级开始
 *
 * @param[in]       void
 *
 * @return          osOK: 升级开始成功. 其它: 升级开始失败
 ************************************************************************************
*/
int32_t ONENET_OTA_Start(ONENET_OTA_Cb *callback);

/**
 ************************************************************************************
 * @brief           ONENET OTA 升级结束
 *
 * @param[in]       void
 *
 * @return          void
 ************************************************************************************
*/
void ONENET_OTA_Stop(void);


#ifdef __cplusplus
}
#endif
#endif

