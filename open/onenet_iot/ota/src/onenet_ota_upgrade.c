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
#include <vfs.h>
#include <ota_user.h>
#include "os.h"
#include "onenet_ota_pub.h"
#include "onenet_ota_http.h"
#include "onenet_ota_upgrade.h"


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_UPGRADE_ON
 #ifdef OTA_UPGRADE_ON
#define OTA_UPGRADE_DBG_PRINTF                         OTA_PUB_PRINT_DEBUG
#else
#define OTA_UPGRADE_DBG_PRINTF(...)
#endif
#define OTA_UPGRADE_ERR_PRINTF                         OTA_PUB_PRINT_ERROR


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct
 {
    uint32_t size; /* < 下载文件大小 */
    char url[OTA_HTTP_URL_LEN_MAX]; /* < 下载地址: HTTP 或 HTTP(S) */
}OTA_UpgradeDownloadInfo;


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/


/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void OTA_UpgradeDownloadInfoGet(const char *path, OTA_UpgradeDownloadInfo *info)
{
    VFS_File *fp = VFS_OpenFile(path, "rb");
    if(OS_NULL != fp)
    {
        VFS_ReadFile(info, 1, sizeof(OTA_UpgradeDownloadInfo), fp);
        VFS_CloseFile(fp);
    }
    return;
}

static void OTA_UpgradeDownloadInfoSave(const char *path, OTA_UpgradeDownloadInfo *info)
{
    VFS_File *fp = VFS_OpenFile(path, "wb");
    if(OS_NULL != fp)
    {
        uint32_t count = sizeof(info->size) + strlen(info->url) + 1;
        VFS_WriteFile(info, 1, count, fp);
        VFS_CloseFile(fp);
    }
    return;
}

static void OTA_UpgradeFirmwareDelete(const char *pathName)
{
    char downInfoPath[strlen(pathName)+4];
    VFS_Unlink(pathName);
    osSprintf(downInfoPath, "%s.dl", pathName);
    VFS_Unlink(downInfoPath);
    return;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
/* 获取上次下载的位置, 接着上次的下载位置继续下载 */
int32_t OTA_UpgradeFirmwareOffsetGet(char *url, uint8_t force, char *pathName, uint32_t firmwareSize)
{
    int32_t err = 0;
    uint16_t pathLen = 0;
    uint32_t offset = 0;
    char *downInfoPath = NULL;
    VFS_File *fpPkg = NULL;
    OTA_UpgradeDownloadInfo downInfo;

    OTA_UPGRADE_DBG_PRINTF("ota upgrade offset pathName is [%s]\r\n", pathName);
    pathLen = strlen(pathName) + 4;
    downInfoPath = osMalloc(pathLen);
    if(downInfoPath == OS_NULL)
    {
        OTA_UPGRADE_ERR_PRINTF("offset downInfoPath malloc fail\r\n");
        return osErrorTimeout;
    }
    memset(downInfoPath, 0, pathLen);

    /* 1. 获取历史下载记录 */
    osSprintf(downInfoPath, "%s.dl", pathName);
    memset(&downInfo, 0, sizeof(downInfo));
    OTA_UpgradeDownloadInfoGet(downInfoPath, &downInfo);
    fpPkg = VFS_OpenFile(pathName, "rb+");
    if(OS_NULL == fpPkg)
    {
        OTA_UPGRADE_ERR_PRINTF("ota upgrade offset open path[%s]\r\n", pathName);
        fpPkg = VFS_OpenFile(pathName, "wb");
    }

    if(OS_NULL == fpPkg)
    {
        OTA_UPGRADE_ERR_PRINTF("ota upgrade offset open file fail\r\n");
        osFree(downInfoPath);
        downInfoPath = NULL;
        return osErrorResource;
    }
    err = VFS_GetFileSize(fpPkg);
    offset = err < 0 ? 0 : (uint32_t)err;

    /* 2. 获取文件大小 */
    if(firmwareSize < 0 || firmwareSize < (int32_t)OTA_GetPackageMinSize())
    {
        OTA_UPGRADE_ERR_PRINTF("ota upgrade offset get firmwareSize [%d]\r\n", firmwareSize);
        VFS_CloseFile(fpPkg);
        osFree(downInfoPath);
        downInfoPath = NULL;
        return osErrorParameter;
    }

    /* 3. 判断是否需要重新下载: force 为1 或地址变化或大小变化, 则需要重新下载 */
    if(force || strncmp(url, downInfo.url, OTA_HTTP_URL_LEN_MAX) || firmwareSize != (int32_t)downInfo.size)
    {
        memset(&downInfo, 0, sizeof(downInfo));
        OTA_UPGRADE_ERR_PRINTF("ota upgrade offset download reset\r\n");
    }

    /* 4. 记录下载信息 */
    if(downInfo.size == 0)
    {
        VFS_CloseFile(fpPkg);
        VFS_Unlink(pathName);
        VFS_Unlink(downInfoPath);
        fpPkg = VFS_OpenFile(pathName, "wb");
        if(OS_NULL == fpPkg)
        {
            OTA_UPGRADE_ERR_PRINTF("ota upgrade offset open file fail2\r\n");
            VFS_CloseFile(fpPkg);
            osFree(downInfoPath);
            downInfoPath = NULL;
            return osErrorNoMemory;
        }
        offset = 0;
        downInfo.size = firmwareSize;
        strncpy(downInfo.url, url, OTA_HTTP_URL_LEN_MAX);
        OTA_UpgradeDownloadInfoSave(downInfoPath, &downInfo);
    }

    /* 5. 偏移下载数据 */
    err = VFS_SeekFile(fpPkg, offset, VFS_SEEK_SET);
    if(err < 0)
    {
        OTA_UPGRADE_ERR_PRINTF("Seek c to %d: %d\r\n", offset, err);
        VFS_CloseFile(fpPkg);
        osFree(downInfoPath);
        downInfoPath = NULL;
        return osErrorISR;
    }

    VFS_CloseFile(fpPkg);
    osFree(downInfoPath);
    fpPkg = NULL;
    downInfoPath = NULL;

    return offset;
}

/* 保存下载到的升级包*/
int32_t OTA_UpgradeSaveFirmware(const void *buf, uint32_t len, char *pathName)
{
    VFS_File *fpPkg = NULL;
    size_t ret = 0;

    OTA_UPGRADE_DBG_PRINTF("ota upgrade save pathName is [%s]\r\n", pathName);
    fpPkg = VFS_OpenFile(pathName, "ab");
    if(OS_NULL == fpPkg)
    {
        OTA_UPGRADE_ERR_PRINTF("ota upgrade save open file fail\r\n");
        return osError;
    }

    ret = VFS_WriteFile(buf, 1, len, fpPkg);
    if(ret != len)
    {
        OTA_UPGRADE_ERR_PRINTF("ota upgrade write file fail\r\n");
        return osErrorTimeout;
    }
    VFS_SyncFile(fpPkg);

    VFS_CloseFile(fpPkg);
    fpPkg = NULL;

    return osOK;
}

/* 将下载到的的升级包大小与检测升级任务中响应的升级包大小进行校验比对 */
int32_t OTA_UpgradeFirmwareSizeCheck(int32_t totalSize, char *pathName)
{
    int32_t len = 0;
    VFS_File *fpPkg = NULL;

    OTA_UPGRADE_DBG_PRINTF("ota upgrade check pathName is [%s]\r\n", pathName);
    fpPkg = VFS_OpenFile(pathName, "rb");
    if(OS_NULL == fpPkg)
    {
        OTA_UPGRADE_ERR_PRINTF("ota upgrade check open file fail\r\n");
        return osErrorTimeout;
    }
    len = VFS_GetFileSize(fpPkg);
    VFS_CloseFile(fpPkg);
    fpPkg = NULL;

    if(len != totalSize)
    {
        OTA_UPGRADE_ERR_PRINTF("ota upgrade check file size err[%d][%d]\r\n", len, totalSize);
        return osErrorResource;
    }

    return osOK;
}

/* 升级下载的升级包 */
int32_t OTA_UpgradeFirmware(char *pathName)
{
    int32_t ret = -1;
    int32_t err = -1;

    ret = OTA_CheckPackage(pathName, NULL);
    if(ret == OTA_ERROR_OK)
    {
        err = osOK;
    }
    else if(ret == -OTA_ERROR_BAD_PATCH)
    {
        err = osError;
        OTA_UpgradeFirmwareDelete(pathName);
    }
    else
    {
        err = osErrorTimeout;
    }

    if(osOK == err)
    {
        osThreadMsSleep(500);
        OTA_UPGRADE_ERR_PRINTF("ota upgrade start\r\n");
        ret = OTA_StartUpgrade(pathName);
        if(OTA_ERROR_OK == ret)
        {
            OTA_UPGRADE_DBG_PRINTF("ota upgrade succ\r\n");
            ret = osOK;
        }
        else
        {
            OTA_UPGRADE_ERR_PRINTF("ota upgrade fail[%d], stop\r\n", ret);
            ret = osErrorTimeout;
        }
    }
    else
    {
        OTA_UPGRADE_ERR_PRINTF("ota upgrade check fail[%d], stop\r\n", err);
        ret = osErrorResource;
    }

    return ret;
}

#ifdef __cplusplus
}
#endif

