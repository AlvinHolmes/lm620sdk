/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        ota_user.c
 *
 * @brief       OTA 用户接口实现.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-10-21   ict team        创建
 ************************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <os.h>
#include <drv_flash.h>
#include <drv_reset.h>
#include <drv_uart.h>
#include <vfs.h>
#include <md5.h>
#include <ota_user.h>
#include <verify.h>

extern UART_Handle* Board_GetAtUartHandle(void);
extern const char* VFS_GetFullPath(const char *absPath, char *fullPath, uint32_t len);
/************************************************************************************
 *                                 配置开关
 ************************************************************************************/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_IMG_MBL             ("mbl")
#define OTA_IMG_XBOOT           ("xboot")
#define OTA_IMG_NVD             ("nvd")
#define OTA_IMG_ZKNL            ("zknl")
#define OTA_IMG_XIP             ("xknl")
#define OTA_IMG_APP             ("app")
#define OTA_IMG_FSYS            ("/")

#define OTA_FLAGS_PARTITION     ("cmn")
#define OTA_PATCH_PARTITION     ("/usr/ota.cfg")

#define OTA_FLAG                (0xB2)
#define OTA_BLOCK_SIZE          (4096)

#define FILESYS_UPGRADE_RESV    (8)     // 文件系统升级保留的空间，单位: block

/**
 * @brief Symbol of OTA package's head
 */
#define OTA_PACKAGE_HEAD        "IoTa"
/**
 * @brief Symbol of OTA package's tail
 */
#define OTA_PACKAGE_TAIL        "aToI"

#define OTA_PACKIMG_MINSIZE     (sizeof(OTA_ImageHeadzone) + sizeof(OTA_PatchHeadzone))
#define OTA_PACKAGE_MINSIZE     (sizeof(OTA_PackageHeadzone) + sizeof(OTA_PackageTailzone) + OTA_PACKIMG_MINSIZE)

/************************************************************************************
 *                                 类型定义
************************************************************************************/
/**
 * @brief OTA package tail.
 */
typedef struct {
    uint8_t     md5[16];            ///< MD5 of OTA package's data exclude tail.
    int8_t      tail[4];            ///< Symbol of OTA package's tail.
}OTA_PackageTailzone;

/**
 * @brief OTA package image section head.
 */
typedef struct {
    uint32_t    size;               ///< Size of this image section in bytes including head.
    char        name[8];            ///< Name of image partition.
    uint16_t    split;              ///< Split size of new compressed image in 4K bytes.
    uint16_t    stoSize;            ///< Storage size of new image in 4K bytes.
    uint8_t     count;              ///< Count of patches in this image section.
    uint8_t     failFlag;           ///< Flag of exit upgrade, if fail.
    uint8_t     diffFlag;           ///< Flag of having diffrential patch.
    uint8_t     pathLen;            ///< Length of file path.
    uint32_t    oldSize;            ///< Size of old image file in bytes.
    uint8_t     oldMd5[16];         ///< MD5 of old image file.
    uint32_t    newSize;            ///< Size of new image file in bytes.
    uint8_t     newMd5[16];         ///< MD5 of new image file.
    //char        path[0];          ///< File path in FS.
}OTA_ImageHeadzone;

/**
 * @brief OTA package patch section head.
 */
typedef struct {
    uint32_t    size;               ///< Size of this patch section in bytes including head.
    uint8_t     index;              ///< Split index in new bin file.
    int8_t      offset;             ///< Offset for caculating split index in old bin file .
    uint8_t     flag;               ///< Flag of patch: original, compress, diffrential.
    uint8_t     resv;               ///< Reserved for alignment.
}OTA_PatchHeadzone;

#ifndef OTA_OFFSETOF
#define OTA_OFFSETOF(TYPE, MEMBER)  ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define OTA_UPGRADE_STATES_BASE     (0)
#define OTA_UGSTATE_STFLAG_ADDR     (OTA_UPGRADE_STATES_BASE + OTA_OFFSETOF(OTA_State, startFlag))
#define OTA_UGSTATE_UDFLAG_ADDR     (OTA_UPGRADE_STATES_BASE + OTA_OFFSETOF(OTA_State, updateFlag))
#define OTA_UGSTATE_RESULT_ADDR     (OTA_UPGRADE_STATES_BASE + OTA_OFFSETOF(OTA_State, result))
#define OTA_UGSTATE_EDFLAG_ADDR     (OTA_UPGRADE_STATES_BASE + OTA_OFFSETOF(OTA_State, endFlag))
#define OTA_UGSTATE_BAUDRATE_ADDR   (OTA_UPGRADE_STATES_BASE + OTA_OFFSETOF(OTA_State, baudrate))

/**
 * @brief OTA upgrade state.
 */
typedef struct {
    uint8_t  resv1[2];      ///< Resvered;
    uint8_t  startFlag;     ///< Flag of need to upgrade, 0xB2: need to upgrade, others: invalid;
    uint8_t  resv2[2];      ///< Resvered;
    uint8_t  updateFlag;    ///< Flag of ugrading, 0xB2: in upgrading, others: not;
    uint8_t  resv3[2];      ///< Resvered;
    uint8_t  result;        ///< Upgraded result, 0: successfully, others: error number;
    uint8_t  endFlag;       ///< End flag of ugrading, 0xB2: End upgrading, others: not;
    uint8_t  resv4[2];      ///< Resvered;
    uint32_t baudrate;      ///< Baudrate of OTA
}OTA_State;



/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief       OTA 模块初始化.
 *
 * @return      错误码
 * @retval      OTA_ERROR_OK    成功;
 *              其他              失败;
 ************************************************************************************
 */
int32_t OTA_Init(void)
{
    return OTA_ERROR_OK;
}

/**
 ************************************************************************************
 * @brief       获取升级包大小的最小值.
 *
 * @return      升级包大小的最小值.
 ************************************************************************************
 */
uint32_t OTA_GetPackageMinSize(void)
{
    return OTA_PACKAGE_MINSIZE + 1;
}

/**
 ***********************************************************************************************************************
 * @brief       检测升级包是否有效.
 *
 * @param[in]   path    升级包绝对路径, 不可为NULL.
 * @param[out]  pHead   存放读取的头信息, 可为NULL.
 *
 * @return      错误码
 * @retval      OTA_ERROR_OK    成功;
 *              <0              失败;
 ***********************************************************************************************************************
 */
int32_t OTA_CheckPackage(const char *path, OTA_PackageHeadzone *pHead)
{
    VFS_File* fp;
    int32_t ret = -OTA_ERROR_BAD_PATCH;

    if (path[0] != '/')
        return ret;

    fp = VFS_OpenFile(path, "rb");
    if (fp != OS_NULL) {
        OTA_PackageHeadzone head;
        OTA_PackageTailzone tail;

        if (VFS_ReadFile(&head, 1, sizeof(head), fp) == sizeof(head)) {
            /* check head valid */
            if (memcmp(head.head, OTA_PACKAGE_HEAD, sizeof(head.head)))
                goto __exit;
            if (head.size <= OTA_PACKAGE_MINSIZE || head.count == 0)
                goto __exit;
            if (head.dbufLen < head.nbufLen) {
                goto __exit;
            }
            if (VFS_GetFileSize(fp) < (long)head.size)
                goto __exit;

            /* read package file tail */
            VFS_SeekFile(fp, head.size - sizeof(tail), VFS_SEEK_SET);
            if (VFS_ReadFile(&tail, 1, sizeof(tail), fp) != sizeof(tail))
                goto __exit;
            if (memcmp(tail.tail, OTA_PACKAGE_TAIL, sizeof(tail.tail)) == 0) {
                MD5_CTX     md5ctx;
                uint8_t     md5[16];
                uint32_t    readlen, pos = 0;
                uint8_t     *buf = (uint8_t*)osMalloc(256);
                if (buf != OS_NULL) {
                    MD5Init(&md5ctx);
                    head.size -= sizeof(OTA_PackageTailzone);
                    VFS_SeekFile(fp, 0, VFS_SEEK_SET);
                    while (pos < head.size) {
                        readlen = head.size - pos;
                        if (readlen > 256)
                            readlen = 256;
                        readlen = VFS_ReadFile(buf, 1, readlen, fp);
                        if (readlen > 0) {
                            MD5Update(&md5ctx, buf, readlen);
                            pos += readlen;
                        } else {
                           break;
                        }
                    }
                    osFree(buf);
                    MD5Final(md5, &md5ctx);
                    head.size += sizeof(OTA_PackageTailzone);
                    if (memcmp(md5, tail.md5, sizeof(tail.md5)) == 0) {
                        if (pHead)
                            memcpy(pHead, &head, sizeof(head));
                        ret = OTA_ERROR_OK;
                    }
                }else {
                    ret = -OTA_ERROR_NOMEM;
                }
            }
        }
__exit:
        VFS_CloseFile(fp);
    }
    return ret;
}


static int32_t checkFileMd5(const char *name, uint32_t len, uint8_t md5[16])
{
    int32_t         ret;
    VFS_FileStat    stat;
    uint8_t         *buf;
    uint8_t         calcMd5[16];
    VFS_File        *fp;

    if (VFS_Stat(name, &stat) != VFS_EOK) {
        return -OTA_ERROR_IMAGE_NTEXIST;
    }
    if (stat.st_size != len) {
        return -OTA_ERROR_IMAGE_MD5;
    }

    buf = osMalloc(256);
    if (buf == NULL) {
        return -OTA_ERROR_NOMEM;
    }

    ret = OTA_ERROR_OK;
    fp = VFS_OpenFile(name, "rb");
    if (fp != NULL) {
        MD5_CTX     md5ctx;
        uint32_t    readlen, pos = 0;

        MD5Init(&md5ctx);
        VFS_SeekFile(fp, 0, VFS_SEEK_SET);
        while (pos < len) {
            readlen = len - pos;
            if (readlen > 256)
                readlen = 256;
            readlen = VFS_ReadFile(buf, 1, readlen, fp);
            if (readlen > 0) {
                MD5Update(&md5ctx, buf, readlen);
                pos += readlen;
            } else {
               break;
            }
        }
        MD5Final(calcMd5, &md5ctx);
        if (memcmp(md5, calcMd5, sizeof(calcMd5))) {
            ret = -OTA_ERROR_IMAGE_MD5;
        }
        VFS_CloseFile(fp);
    }

    osFree(buf);
    return ret;
}

static int32_t checkImageMd5(const char *name, uint32_t skip, uint32_t len, uint8_t md5[16])
{
    int32_t         ret;
    uint8_t         *buf;
    uint8_t         calcMd5[16];
    PART_Handle     *part;

    buf = osMalloc(256);
    if (buf == NULL) {
        return -OTA_ERROR_NOMEM;
    }

    ret = OTA_ERROR_OK;
    part = PART_Find(name);
    if (part != NULL) {
        MD5_CTX     md5ctx;
        uint32_t    readlen, pos = 0;

        MD5Init(&md5ctx);
        while (pos < len) {
            readlen = len - pos;
            if (readlen > 256)
                readlen = 256;
            ret = PART_Read(part, pos + skip, buf, readlen);
            if (ret > 0) {
                MD5Update(&md5ctx, buf, ret);
                pos += ret;
            } else {
               break;
            }
        }
        MD5Final(calcMd5, &md5ctx);
        ret = OTA_ERROR_OK;
        if (memcmp(md5, calcMd5, sizeof(calcMd5))) {
            ret = -OTA_ERROR_IMAGE_MD5;
        }
    }

    osFree(buf);
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief       检测升级版本是否匹配.
 *
 * @param[in]   path    升级包绝对路径, 不可为NULL.
 *
 * @return      错误码
 * @retval      OTA_ERROR_OK    成功;
 *              <0              失败;
 ***********************************************************************************************************************
 */
int32_t OTA_CheckVersion(const char *path)
{
    OTA_PackageHeadzone packHead;
    OTA_ImageHeadzone   imgHead;

    if (OTA_CheckPackage(path, &packHead) != OTA_ERROR_OK) {
        return -OTA_ERROR_BAD_PATCH;
    }

    VFS_File *fp = VFS_OpenFile(path, "rb");
    if (fp == OS_NULL) {
        return -OTA_ERROR_BAD_PATCH;
    }

    uint32_t pos = sizeof(OTA_PackageHeadzone);
    for (uint32_t i = 0; i < packHead.count; i++) {
        int32_t ret;
        size_t  read;

        VFS_SeekFile(fp, pos, VFS_SEEK_SET);
        read = VFS_ReadFile(&imgHead, 1, sizeof(OTA_ImageHeadzone), fp);
        if (read != sizeof(OTA_ImageHeadzone)) {
            VFS_CloseFile(fp);
            return -OTA_ERROR_GENERAL;
        }

        pos += imgHead.size;

        if (imgHead.diffFlag == 0)
            continue;

        ret = OTA_ERROR_OK;
        if (strcmp(imgHead.name, OTA_IMG_FSYS) == 0) {
            char path[VFS_PATH_MAX+1] = {0};

            // 读取文件路径
            read = VFS_ReadFile(path, 1, imgHead.pathLen, fp);
            if (read != imgHead.pathLen) {
                ret = -OTA_ERROR_GENERAL;
            } else {
                ret = checkFileMd5(path, imgHead.oldSize, imgHead.oldMd5);
            }
        } else if (strcmp(imgHead.name, OTA_IMG_NVD) == 0) {
            ret = checkImageMd5(OTA_IMG_NVD, 0, imgHead.oldSize, imgHead.oldMd5);
        } else if (strcmp(imgHead.name, OTA_IMG_XIP) == 0) {
            ret = checkImageMd5(OTA_IMG_XIP, 0, imgHead.oldSize, imgHead.oldMd5);
        } else if (strcmp(imgHead.name, OTA_IMG_APP) == 0) {
            ret = checkImageMd5(OTA_IMG_APP, 0, imgHead.oldSize, imgHead.oldMd5);
        }

        if (ret != OTA_ERROR_OK) {
            VFS_CloseFile(fp);
            return ret;
        }
    }

    VFS_CloseFile(fp);
    return OTA_ERROR_OK;
}

static uint32_t OTA_GetBaudratesOfATPort(void)
{
    uint32_t    baud = 115200;
    UART_Handle *atPort = Board_GetAtUartHandle();

    if (atPort) {
        baud = UART_GetCurrentBaudRate(atPort);
    }

    return baud;
}
/**
 ***********************************************************************************************************************
 * @brief       开始进行OTA升级.
 *
 * @param[in]   path    升级包绝对路径, 不可为NULL.
 *
 * @return      操作结果.
 * @retval      N/A     成功;
 *              <0      失败;
 ***********************************************************************************************************************
 */
int32_t OTA_StartUpgrade(const char *path)
{
    int         ret;
    uint8_t     flag;
    PART_Handle *part;
    VFS_File    *fp;
    const char  *realPath;
    char        *fullPath;

    if (path[0] != '/')
        goto fail_ret;

    part = PART_Find(OTA_FLAGS_PARTITION);
    if (part == OS_NULL)
        goto fail_ret;

    ret = PART_Erase(part, 0, PART_Size(part));
    if (ret < 0)
        goto fail_ret;

    fullPath = (char*)osMalloc(VFS_PATH_MAX+1);
    if (fullPath == OS_NULL)
        goto fail_ret;
    realPath = VFS_GetFullPath(path, fullPath, VFS_PATH_MAX+1);

    fp = VFS_OpenFile(OTA_PATCH_PARTITION, "wb");
    if (fp == OS_NULL) {
        osFree(fullPath);
        goto fail_ret;
    }
    if (VFS_WriteFile(realPath, 1, strlen(realPath)+1, fp) != strlen(realPath)+1) {
        osFree(fullPath);
        VFS_CloseFile(fp);
        goto fail_ret;
    }

    VFS_CloseFile(fp);
    osFree(fullPath);

    //Set baudrates of OTA serial port
    uint32_t baud = OTA_GetBaudratesOfATPort();
    ret = PART_Write(part, OTA_UGSTATE_BAUDRATE_ADDR, (uint8_t*)&baud, sizeof(baud));
    if (ret < 0)
        goto fail_ret;

    flag = OTA_FLAG;
    ret = PART_Write(part, OTA_UGSTATE_STFLAG_ADDR, &flag, sizeof(flag));
    if (ret < 0)
        goto fail_ret;

    return OTA_ERROR_OK;

fail_ret:
    VFS_Unlink(OTA_PATCH_PARTITION);
    return -OTA_ERROR_GENERAL;
}

/**
 ***********************************************************************************************************************
 * @brief       获取上次 OTA 升级的结果.
 *
 * @return      升级结果.
 * @retval      >=0     上次升级结果;
 *              <0      无有效结果;
 ***********************************************************************************************************************
 */
int32_t OTA_GetResult(void)
{
    PART_Handle *part = PART_Find(OTA_FLAGS_PARTITION);
    if (part != NULL) {
        int ret;
        OTA_State state = {0};
        ret = PART_Read(part, 0, (uint8_t*)&state, sizeof(state));
        if (ret > 0 && (uint32_t)ret >= sizeof(state)) {
            if (state.startFlag == OTA_FLAG) {
                if (state.endFlag == OTA_FLAG || state.result < OTA_ERROR_MAX) {
                    return state.result;
                }
                if(state.updateFlag != OTA_FLAG){
                    return -OTA_ERROR_READY;
                }
            }
        }
    }
    return -OTA_ERROR_GENERAL;
}

/**
 ***********************************************************************************************************************
 * @brief       清除上次升级结果.
 *
 * @return      无.
 ***********************************************************************************************************************
 */
void OTA_ClearResult(void)
{
    PART_Handle *part = PART_Find(OTA_FLAGS_PARTITION);
    if (part != NULL) {
        PART_Erase(part, 0, PART_Size(part));
    }
}

/**
 ***********************************************************************************************************************
 * @brief       获取升级包内的版本号.
 *
 * @param[in]   path    升级包绝对路径, 不可为NULL.
 * @param[out]  oldVer  存放老版本号, 可为NULL.
 * @param[out]  newVer  存放新版本号, 可为NULL.
 *
 * @retval      OTA_ERROR_OK    成功;
 *              <0              失败;
 ***********************************************************************************************************************
 */
int32_t OTA_GetUpdateVersion(const char *path, char oldVer[VERLEN], char newVer[VERLEN])
{
    int32_t ret;

    OTA_PackageHeadzone head = {0};
    ret = OTA_CheckPackage(path, &head);
    if (ret == OTA_ERROR_OK) {
        if (oldVer) {
            memset(oldVer, 0, VERLEN);
            strncpy(oldVer, head.oldVersion, VERLEN-1);
        }
        if (newVer) {
            memset(newVer, 0, VERLEN);
            strncpy(newVer, head.newVersion, VERLEN-1);
        }
    }
    return ret;
}


static uint32_t evaluateNewFileBlocks(char *path, uint32_t size)
{
    uint32_t count = 0;

    // 计算新建文件夹所需空间
    if (path != NULL) {
        do {
            char *p = strrchr(path, '/');
            if (p)
                *p = 0;
            if (p == path || VFS_Access(path, 0) == VFS_EOK) {
                break;
            }
            count += VFS_EvaluateBlock(OS_TRUE, 0);
        } while(1);
    }
    // 计算新建文件所需空间
    count += VFS_EvaluateBlock(OS_FALSE, size);

    return count;
}

static uint32_t evaluateDirBlocks(char *path)
{
    uint32_t    count;
    char        *p;
    VFS_Dir     *dp;
    VFS_Dirent  *dent;

    count = 0;
    dp = VFS_OpenDir(path);
    if (dp != NULL) {
        p = path + strlen(path);
        while ((dent = VFS_ReadDir(dp)) != NULL) {
            if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) {
                continue;
            }
            if (dent->d_type != VFS_DT_REG) {
                if (*(p-1) != '/') {
                    *p = '/';
                    strncpy(p+1, dent->d_name, VFS_PATH_MAX);
                    p[1+strlen(dent->d_name)] = 0;
                } else {
                    strncpy(p, dent->d_name, VFS_PATH_MAX);
                    p[strlen(dent->d_name)] = 0;
                }
                count += evaluateDirBlocks(path) + VFS_EvaluateBlock(OS_TRUE, 0);
                *p = 0;
            }
            else {
                count += VFS_EvaluateBlock(OS_FALSE, dent->d_size);
            }
        }
        VFS_CloseDir(dp);
    }
    return count;
}

static uint32_t evaluateDeleteFileBlocks(char *path)
{
    uint32_t count = 0;

    VFS_FileStat stat;
    // 计算删除文件夹所需空间
    if (VFS_Stat(path, &stat) == VFS_EOK) {
        if (VFS_S_ISDIR(stat.st_mode)) {
            count = evaluateDirBlocks(path) + VFS_EvaluateBlock(OS_TRUE, 0);
        } else { // 计算删除文件所需空间
            count = VFS_EvaluateBlock(OS_FALSE, stat.st_size);
        }
    }
    return count;
}

/**
 ***********************************************************************************************************************
 * @brief       计算升级时，所需的临时空间大小(文件系统).
 *
 * @param[in]   path    升级包绝对路径, 不可为NULL.
 *
 * @retval      > 0     临时空间大小空间大小，单位: Byte;
 *              < 0     错误码;
 ***********************************************************************************************************************
 */
int32_t OTA_GetUpdateRequiredSpace(const char *path)
{
    int32_t bakBlk, fsDelBlk, fsUpBlk, lfsBakBlk, fsNewBlk, fsResvBlk;

    OTA_PackageHeadzone packHead;
    OTA_ImageHeadzone   imgHead;

    if (OTA_CheckPackage(path, &packHead) != OTA_ERROR_OK) {
        return -OTA_ERROR_BAD_PATCH;
    }

    VFS_File *fp = VFS_OpenFile(path, "rb");
    if (fp == OS_NULL) {
        return -OTA_ERROR_BAD_PATCH;
    }

    uint32_t pos = sizeof(OTA_PackageHeadzone);
    bakBlk = fsDelBlk = fsUpBlk = lfsBakBlk = fsNewBlk = fsResvBlk = 0;
    for (uint32_t i = 0; i < packHead.count; i++) {
        size_t ret;
        char   path[VFS_PATH_MAX+1];

        VFS_SeekFile(fp, pos, VFS_SEEK_SET);
        ret = VFS_ReadFile(&imgHead, 1, sizeof(OTA_ImageHeadzone), fp);
        if (ret != sizeof(OTA_ImageHeadzone)) {
            VFS_CloseFile(fp);
            return -OTA_ERROR_GENERAL;
        }

        pos += imgHead.size;

        if (imgHead.diffFlag && (int32_t)imgHead.split > bakBlk)
            bakBlk = imgHead.split;

        if (strcmp(imgHead.name, OTA_IMG_FSYS))
            continue;

        fsResvBlk = FILESYS_UPGRADE_RESV;

        // 读取文件路径
        memset(path, 0, sizeof(path));
        ret = VFS_ReadFile(path, 1, imgHead.pathLen, fp);
        if (ret != imgHead.pathLen) {
            VFS_CloseFile(fp);
            return -OTA_ERROR_GENERAL;
        }

        // 计算文件更新空间变化
        if (imgHead.newSize == 0) {
            fsDelBlk += evaluateDeleteFileBlocks(path);
        } else if (imgHead.diffFlag) {
            VFS_FileStat stat;
            if (VFS_Stat(path, &stat) != VFS_EOK) {
                VFS_CloseFile(fp);
                osPrintf("OTA_GetUpdateRequiredSpace: %s not exist\r\n", path);
                return -OTA_ERROR_IMAGE_NTEXIST;
            }

            int32_t oldBlk = evaluateNewFileBlocks(NULL, stat.st_size);
            int32_t newBlk = evaluateNewFileBlocks(NULL, imgHead.newSize);
            fsUpBlk += newBlk - oldBlk;
            // lfs 写文件时，需额外消耗1倍的(备份)空间
            newBlk = (newBlk > oldBlk ? newBlk : oldBlk);
            if (newBlk > lfsBakBlk)
                lfsBakBlk = newBlk;
        } else {
            fsNewBlk += evaluateNewFileBlocks(path, imgHead.newSize);
        }
    }
    VFS_CloseFile(fp);

    osPrintf("OTA_GetUpdateRequiredSpace: bak=%d, lfsbak=%d, del=%d, difup=%d, new=%d\r\n",
                bakBlk, lfsBakBlk, fsDelBlk, fsUpBlk, fsNewBlk);

    // 计算最终空间需求
    fsDelBlk = 0 - fsDelBlk;
    if (bakBlk) {
        bakBlk = (bakBlk + 2) + lfsBakBlk;
        // 有差分升级: 先删除，再更新，最后新建
        fsUpBlk    = fsUpBlk + fsDelBlk;    // 删除 & 更新后的空间消耗
        fsNewBlk   = fsNewBlk + fsUpBlk;    // 删除 & 更新 & 新建后的空间消耗
        if (fsNewBlk > bakBlk)
            bakBlk = fsNewBlk;
    } else {
        // 无差分升级
        bakBlk = fsNewBlk + fsDelBlk;
    }
    if (bakBlk < 0)
        bakBlk = 0;
    return (bakBlk + fsResvBlk) * OTA_BLOCK_SIZE;
}

static void freePackContentlist(OTA_UpdatePackContent *list, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++) {
        if (list[i].name)
            osFree(list[i].name);
    }
    osFree(list);
}

/**
 ***********************************************************************************************************************
 * @brief       获取升级包中升级内容列表.
 *
 * @param[in]   path    升级包绝对路径, 不可为NULL.
 * @param[out]  list    内容列表, 不可为NULL.
 *
 * @retval      > 0     列表个数;
 *              < 0     错误码;
 ***********************************************************************************************************************
 */
int32_t OTA_GetUpdatePackContent(const char *path, OTA_UpdatePackContent **list)
{
    OTA_PackageHeadzone packHead;

    if (OTA_CheckPackage(path, &packHead) != OTA_ERROR_OK) {
        return -OTA_ERROR_BAD_PATCH;
    }

    VFS_File *fp = VFS_OpenFile(path, "rb");
    if (fp != OS_NULL) {
        OTA_UpdatePackContent   *content;
        OTA_ImageHeadzone       imgHead;

        content = (OTA_UpdatePackContent *)osMalloc(packHead.count * sizeof(OTA_UpdatePackContent));
        memset(content, 0, packHead.count * sizeof(OTA_UpdatePackContent));

        uint32_t pos = sizeof(OTA_PackageHeadzone);
        for (uint32_t i = 0; i < packHead.count; i++) {
            VFS_SeekFile(fp, pos, VFS_SEEK_SET);
            size_t ret = VFS_ReadFile(&imgHead, 1, sizeof(OTA_ImageHeadzone), fp);
            if (ret != sizeof(OTA_ImageHeadzone)) {
                freePackContentlist(content, packHead.count);
                VFS_CloseFile(fp);
                return -OTA_ERROR_GENERAL;
            }
            content[i].size = (int32_t)imgHead.newSize - (int32_t)imgHead.oldSize;
            if (strcmp(imgHead.name, OTA_IMG_FSYS)) {
                content[i].type = OTA_CONTENT_FLASH;
                content[i].nameSize = strlen(imgHead.name);
                content[i].name = osMalloc(content[i].nameSize + 1);
                memset(content[i].name, 0, content[i].nameSize + 1);
                strncpy(content[i].name, imgHead.name, content[i].nameSize);
            } else {
                if (imgHead.diffFlag) {
                    content[i].type = OTA_CONTENT_FILE_UPDATE;
                } else if (imgHead.newSize) {
                    content[i].type = OTA_CONTENT_FILE_INSERT;
                } else {
                    content[i].type = OTA_CONTENT_FILE_REMOVE;
                }
                content[i].nameSize = imgHead.pathLen;
                content[i].name = osMalloc(content[i].nameSize + 1);
                memset(content[i].name, 0, content[i].nameSize + 1);
                ret = VFS_ReadFile(content[i].name, 1, content[i].nameSize, fp);
                if (ret != content[i].nameSize) {
                    freePackContentlist(content, packHead.count);
                    VFS_CloseFile(fp);
                    return -OTA_ERROR_GENERAL;
                }
            }
            pos += imgHead.size;
        }
        VFS_CloseFile(fp);

        *list = content;
        return packHead.count;
    }
    return -OTA_ERROR_BAD_PATCH;
}

/**
 ***********************************************************************************************************************
 * @brief       检测升级包签名是否有效.
 *
 * @param[in]   path    升级包绝对路径, 不可为NULL.
 *
 * @retval      OTA_ERROR_OK    成功;
 *              <0              失败;
 ***********************************************************************************************************************
 */
int32_t OTA_CheckSignedPackage(const char *path)
{
    VFS_File* fp;
    int32_t ret = -OTA_ERROR_BAD_PATCH;

    if (path[0] != '/')
        return ret;

    fp = VFS_OpenFile(path, "rb");
    if (fp != OS_NULL) {
        ret = FileCertVerify(fp) ? OTA_ERROR_SIGNATURE : OTA_ERROR_OK;
        VFS_CloseFile(fp);
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief       获取升级包安全启动版本.
 *
 * @param[in]   path    升级包绝对路径, 不可为NULL.
 *
 * @retval      > 0     安全启动版本;
 *              < 0     失败;
 ***********************************************************************************************************************
 */
int32_t OTA_GetSecureVersion(const char *path)
{
    int32_t ret;

    OTA_PackageHeadzone head = {0};
    ret = OTA_CheckPackage(path, &head);
    if (ret == OTA_ERROR_OK) {
        return head.sver;
    }
    return ret;
}

#if defined(USE_TOP_DEBUG) && defined(OS_USING_SHELL)
#include <nr_micro_shell.h>

static void SHELL_ota(char argc, char **argv)
{
    if (argc > 1) {
        if (!strcmp("check", argv[1]) && argc > 2) {
            OTA_PackageHeadzone head;
            int32_t ret = OTA_CheckPackage(argv[2], &head);
            if (ret == OTA_ERROR_OK) {
                shell_printf("OTA patch: %s -> %s\r\n", head.oldVersion, head.newVersion);
            } else {
                shell_printf("OTA check %s fail: %d\r\n", argv[2], ret);
            }
            return;
        }  else if (!strcmp("chkver", argv[1])) {
            int32_t ret = OTA_CheckVersion(argv[2]);
            if (ret != OTA_ERROR_OK) {
                shell_printf("OTA check version %s fail: %d\r\n", argv[2], ret);
            }
            return;
        }else if (!strcmp("upgrade", argv[1]) && argc > 2) {
            if (OTA_StartUpgrade(argv[2])) {
                shell_printf("OTA upgrade %s fail\r\n", argv[2]);
            } else {
                //do reboot, and should never return
                osShutdown(OS_REBOOT, OS_REBOOT_FOTA);
                osThreadSuspend(osThreadSelf());
            }
            return;
        } else if (!strcmp("result", argv[1])) {
            uint32_t clear = 0;
            if (argc == 3)
                clear = (uint32_t)strtoul(argv[2], OS_NULL, 0);

            if (clear) {
                OTA_ClearResult();
            } else {
                int32_t result = OTA_GetResult();
                if (result < 0)
                    shell_printf("OTA query result fail: %d\r\n", result);
                else
                    shell_printf("OTA query result: %d\r\n", result);
            }
            return;
        } else if (!strcmp("upspace", argv[1]) && argc > 2) {
            int32_t ret = OTA_GetUpdateRequiredSpace(argv[2]);
            if (ret < 0) {
                shell_printf("OTA upspace %s fail\r\n", argv[2]);
            } else {
                shell_printf("OTA upspace: %d bytes\r\n", ret);
            }
            return;
        } else if (!strcmp("upcont", argv[1])) {
            OTA_UpdatePackContent *list = NULL;
            int32_t ret = OTA_GetUpdatePackContent(argv[2], &list);
            if (ret <= 0) {
                shell_printf("OTA upcont %s fail\r\n", argv[2]);
            } else {
                for (int32_t i = 0; i < ret; i++) {
                    shell_printf("\ttype=%d, size=%d, name=%s\r\n",
                        list[i].type, list[i].size, list[i].name);
                    if (list[i].name)
                        osFree(list[i].name);
                }
                osFree(list);
            }
            return;
        } else if (!strcmp("verify", argv[1])) {
            if (argc < 3)
            {
                shell_printf("Please input patch file\r\n");
                return;
            }

            if (OTA_CheckSignedPackage(argv[2])) {
                shell_printf("OTA verify %s fail\r\n", argv[2]);
            }
            return;
        }
    }

    shell_printf("ota <check/upgrade> <path>\r\n");
}

NR_SHELL_CMD_EXPORT(ota, SHELL_ota);

#endif

