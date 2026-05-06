/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        vfs_cmd.c
 *
 * @brief       VFS shell 命令实现.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-04-21     ict team          创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <os.h>
#if defined(OS_USING_VFS) && defined(VFS_SHELL_CMD) && defined(OS_USING_SHELL)
#include <stdlib.h>
#include <drv_flash.h>
#include <vfs_port.h>
#include <vfs_path.h>
#include <nr_micro_shell.h>
#include <string.h>
#include <ctype.h>
/************************************************************************************
 *                                 配置开关
 ************************************************************************************/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
************************************************************************************/


/************************************************************************************
 *                                 局部函数声明
 ************************************************************************************/
#if VFS_MOUNT_COUNT > 1
extern int32_t VFS_TraverseMountPoint(int32_t (*cb)(const VFS_MountPoint*, void*), void *arg);

static int32_t prvMountPointList(const VFS_MountPoint *mnt, void *arg);
static int32_t prvMountPointPrint(const VFS_MountPoint *mnt, void *arg);
static int32_t prvMountPointDiskFree(const VFS_MountPoint *mnt, void *arg);
#endif

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/


/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
static void _ls(const char *path)
{
    VFS_Dir         *dp;
    VFS_Dirent      *dent;

    dp = VFS_OpenDir(path);
    if (dp) {
        shell_printf("Directory %s:\r\n", path);
        do {
            dent = VFS_ReadDir(dp);
            if (!dent) {
                break;
            }
            shell_printf("%-20s    ", dent->d_name);
            if (dent->d_type != VFS_DT_REG) {
                shell_printf("%-25s\r\n", "<DIR>");
            }
            else {
                shell_printf("%-25lu\r\n", dent->d_size);
            }
        }while (dent);
        VFS_CloseDir(dp);
#if VFS_MOUNT_COUNT > 1
        const char *absPath = VFS_GetAbsPath(path);
        if (absPath == OS_NULL) {
            shell_printf("Get abs path, err = %d\r\n", osGetErrno());
            return;
        }
        VFS_TraverseMountPoint(prvMountPointList, (void*)absPath);
        if (absPath != path) {
            VFS_FREE(absPath);
        }
#endif
    }
    else {
        shell_printf("No such directory, err = %d\r\n", osGetErrno());
    }
}

static void SHELL_ls(char argc, char **argv)
{
    if (1 == argc) {
        _ls(".");
    }
    else if (2 == argc) {
        _ls(argv[1]);
    }
    else {
        shell_printf("Usage: ls [DIRNAME]\r\n");
    }
}
NR_SHELL_CMD_EXPORT(ls, SHELL_ls);

static void SHELL_cd(char argc, char **argv)
{
    int ret;

    if (1 == argc) {
        ret = VFS_ChangeDir("/");
        if (VFS_EOK != ret) {
            shell_printf("No such directory, err = %d\r\n", ret);
        }
    }
    else if (2 == argc) {
        ret = VFS_ChangeDir(argv[1]);
        if (VFS_EOK != ret) {
            shell_printf("No such directory, err = %d\r\n", ret);
        }
    }
    else {
        shell_printf("Usage: cd [PATH]\r\n");
    }
}
NR_SHELL_CMD_EXPORT(cd, SHELL_cd);

static void SHELL_pwd(char argc, char **argv)
{
    char cwd[VFS_PATH_MAX+1];

    OS_UNREFERENCE(argc && argv);

    shell_printf("%s\r\n", VFS_GetCwd(cwd, VFS_PATH_MAX));
}
NR_SHELL_CMD_EXPORT(pwd, SHELL_pwd);

static void SHELL_mkdir(char argc, char **argv)
{
    if (2 == argc) {
        int ret = VFS_MakeDir(argv[1], 0);
        if (VFS_EOK != ret) {
            shell_printf("mkdir failed, err = %d\r\n", ret);
        }
    }
    else {
        shell_printf("Usage: mkdir <DIRNAME>\r\n");
    }
}
NR_SHELL_CMD_EXPORT(mkdir, SHELL_mkdir);

static void SHELL_rd(char argc, char **argv)
{
    if (argc == 2) {
        int ret = VFS_RmDir(argv[1]);
        if (VFS_EOK != ret) {
            shell_printf("rd failed, err = %d\r\n", ret);
        }
    }
    else {
        shell_printf("Usage: rd <DIRNAME>\r\n");
    }
}
NR_SHELL_CMD_EXPORT(rd, SHELL_rd);


static void SHELL_rm(char argc, char **argv)
{
    if (argc > 1) {
        int ret, i;
        for (i = 1; i < argc; i ++) {
            ret = VFS_Unlink(argv[i]);
            if (VFS_EOK != ret) {
                shell_printf("rm %s failed, err = %d\r\n", argv[i], ret);
            }
        }
    }
    else {
        shell_printf("Usage: rm <FILE>...\r\n");
    }
}
NR_SHELL_CMD_EXPORT(rm, SHELL_rm);

#define CPBUF_LEN   (512)

static void _copyfile(const char *src, const char *dst)
{
    size_t      readSize;
    size_t      writeSize;
    VFS_File    *fSrc;
    VFS_File    *fDst;

    char *buf = osMalloc(CPBUF_LEN);

    fSrc = VFS_OpenFile(src, "r");
    if (fSrc != NULL) {
        fDst = VFS_OpenFile(dst, "w");
        if (fDst != NULL) {
            do
            {
                readSize = VFS_ReadFile(buf, 1, CPBUF_LEN, fSrc);
                if (readSize == 0) {
                    break;
                }
                writeSize = VFS_WriteFile(buf, 1, readSize, fDst);
                if (writeSize != readSize) {
                    shell_printf("Write file data failed, not copy all the data\r\n");
                    break;
                }
            } while (readSize > 0);
            VFS_CloseFile(fDst);
            VFS_CloseFile(fSrc);
        } else {
            VFS_CloseFile(fSrc);
            shell_printf("open %s failed\r\n", dst);
        }
    } else {
        shell_printf("open %s failed\r\n", src);
    }
    osFree(buf);
}

static void _copy(const char *src, const char *dst)
{
    VFS_FileStat    srcStat;
    VFS_FileStat    dstStat;
    bool_t          exist;
    int             ret;

    exist = OS_FALSE;
    VFS_MEMSET(&srcStat, 0, sizeof(srcStat));
    VFS_MEMSET(&dstStat, 0, sizeof(dstStat));

    ret = VFS_Stat(src, &srcStat);
    if (ret == VFS_EOK) {
        ret = VFS_Stat(dst, &dstStat);
        if (ret == VFS_EOK) {
            exist = OS_TRUE;
        }
        if (VFS_S_ISREG(srcStat.st_mode)) {
            if (VFS_S_ISREG(dstStat.st_mode) || OS_FALSE == exist) {
                /* If dst is file, truncate it and copy. */
                _copyfile(src, dst);
            }
            else if (VFS_S_ISDIR(dstStat.st_mode)) {
                /* If dst is directory, copy src file under dst directory. */
                uint32_t    len;
                char        *path;
                const char  *p = strrchr(src, '/');

                p = p ? p + 1 : src;
                len = strlen(dst)+1+strlen(p)+1;
                path = (char *)osMalloc(len);
                VFS_SNPRINTF(path, len, "%s/%s", dst, p);
                _copyfile(src, path);
                osFree(path);
            }
        }
        else {
            /* Src type is invaild, do nothing. */
            shell_printf("cp faild, %s is not regular file.\r\n", src);
        }
    }
    else {
        shell_printf("copy failed, bad %s\r\n", src);
    }
}

static void SHELL_cp(char argc, char **argv)
{
    if (3 == argc) {
        const char *src = argv[1];
        const char *dst = argv[2];
        if (VFS_STRCMP(src, dst))
            _copy(src, dst);
    }
    else {
        shell_printf("Usage: cp <SOURCE> <DEST> \r\n");
    }
}
NR_SHELL_CMD_EXPORT(cp, SHELL_cp);

static void _move(const char *src, const char *dst)
{
    VFS_FileStat    srcStat;
    VFS_FileStat    dstStat;
    bool_t          exist;
    int             ret;

    exist = OS_FALSE;
    VFS_MEMSET(&srcStat, 0, sizeof(srcStat));
    VFS_MEMSET(&dstStat, 0, sizeof(dstStat));

    ret = VFS_Stat(src, &srcStat);
    if (ret == VFS_EOK) {
        shell_printf("%s => %s\r\n", src, dst);

        ret = VFS_Stat(dst, &dstStat);
        if (ret == VFS_EOK) {
            exist = OS_TRUE;
        }
        if (VFS_S_ISREG(srcStat.st_mode)) {
            if (VFS_S_ISREG(dstStat.st_mode) || OS_FALSE == exist) {
                /* If dst is exist, delete it first. */
                if (VFS_S_ISREG(dstStat.st_mode)) {
                    VFS_Unlink(dst);
                }
                /* rename it. */
                ret = VFS_Rename(src, dst);
            }
            else if (VFS_S_ISDIR(dstStat.st_mode)) {
                /* If dst is directory, move src file under dst directory. */
                uint32_t    len;
                char        *path;
                const char  *p = strrchr(src, '/');

                p = p ? p + 1 : src;
                len = strlen(dst)+1+strlen(p)+1;
                path = (char *)osMalloc(len);
                VFS_SNPRINTF(path, len, "%s/%s", dst, p);
                ret = VFS_Rename(src, path);
                osFree(path);
            }
            if (ret != VFS_EOK) {
                shell_printf("mv failed, can not rename: %d.\r\n", ret);
            }
        }
        else {
            /* Src type is invaild, do nothing. */
            shell_printf("mv faild, %s is not regular file.\r\n", src);
        }
    }
    else {
        shell_printf("move failed, bad %s\r\n", src);
    }
}

static void SHELL_mv(char argc, char **argv)
{
    if (3 == argc) {
        const char *src = argv[1];
        const char *dst = argv[2];
        if (VFS_STRCMP(src, dst))
            _move(src, dst);
    }
    else {
        shell_printf("Usage: mv <SOURCE> <DEST> \r\n");
    }
}
NR_SHELL_CMD_EXPORT(mv, SHELL_mv);


static void SHELL_df(char argc, char **argv)
{

    OS_UNREFERENCE(argc && argv);

#if VFS_MOUNT_COUNT > 1
    VFS_TraverseMountPoint(prvMountPointDiskFree, OS_NULL);
#else
    int         ret;
    VFS_FSStat  stat;
    ret = VFS_StatFSUser("/", &stat);
    if (VFS_EOK == ret) {
        shell_printf("disk free: %d KB [ %d block, %d bytes per block ]\r\n",
                   (stat.blkSize) * (stat.blkFree) / 1024,
                   stat.blkFree,
                   stat.blkSize);
    }
    else {
        shell_printf("df failed: %d\r\n", ret);
    }
#endif
}

NR_SHELL_CMD_EXPORT(df, SHELL_df);

#include <stdlib.h>
static void SHELL_echo(char argc, char **argv)
{
    if (3 <= argc) {
        const char *path = argv[1];
        uint32_t len = (uint32_t)strtoul(argv[2], OS_NULL, 0);
        VFS_File* fp = VFS_OpenFile(path, "rb+");
        if (fp == NULL)
            fp = VFS_OpenFile(path, "wb");
        if (fp) {
            int32_t pos = 0;
            if (4 == argc) {
                pos = (int32_t)strtol(argv[3], OS_NULL, 0);
            }
            VFS_SeekFile(fp, pos, VFS_SEEK_END);
            if (len && VFS_WriteFile(argv[1], 1, len, fp) != len) {
                shell_printf("write %s fail, errno = %d\r\n", path, errno);
            }
            VFS_CloseFile(fp);
        } else {
            shell_printf("Open %s fail, errno = %d\r\n", path, errno);
        }
    }
}

NR_SHELL_CMD_EXPORT(echo, SHELL_echo);


#if VFS_MOUNT_COUNT > 1

static void SHELL_mkfs(char argc, char **argv)
{
    if (argc == 2) {
        int ret = VFS_MakeFS(argv[1], FSYS_NAME);
        if (VFS_EOK != ret)
            shell_printf("mkfs failed: %d\r\n", ret);
    }
    else {
        shell_printf("Usage: mkfs <partition>\r\n");
    }
}

NR_SHELL_CMD_EXPORT(mkfs, SHELL_mkfs);


static void SHELL_mount(char argc, char **argv)
{
    if (argc == 1) {
        VFS_TraverseMountPoint(prvMountPointPrint, OS_NULL);
    } else if (argc == 3 || argc == 4) {
        int flags = 0;
        const char *path = argv[2];
        const char *devname = argv[1];
        PART_Handle *part = PART_Find(devname);
        if (part != OS_NULL) {
            devname = PART_Name(part);
        }
        if (argc == 4 && strcmp(argv[3], "ro") == 0)
            flags = MS_RDONLY;
        int ret = VFS_Mount(devname, FSYS_NAME, path, flags);
        if (VFS_EOK != ret)
            shell_printf("mount failed: %d\r\n", ret);
    } else {
        shell_printf("Usage: mount [partition path [ro]]\r\n");
    }
}

NR_SHELL_CMD_EXPORT(mount, SHELL_mount);

static void SHELL_umount(char argc, char **argv)
{
    if (argc == 2) {
        int ret = VFS_Unmount(argv[1]);
        if (VFS_EOK != ret)
            shell_printf("unmount failed: %d\r\n", ret);
    } else {
        shell_printf("Usage: umount path\r\n");
    }
}

NR_SHELL_CMD_EXPORT(umount, SHELL_umount);


static int32_t prvMountPointList(const VFS_MountPoint *mnt, void *arg)
{
    const char *parent = (const char*)arg;
    if (strcmp(mnt->mntPath, parent) &&
        strstr(mnt->mntPath, parent) == mnt->mntPath) {
        shell_printf("%-20s    ", mnt->mntPath+1);
        shell_printf("%-25s\r\n", "<MNT>");
    }
    return 0;
}

static int32_t prvMountPointPrint(const VFS_MountPoint *mnt, void *arg)
{
    osPrintf("%-10s dev: %-10s ref: %d, flag: %04X\r\n",
            mnt->mntPath, mnt->mntDev, mnt->refCount, mnt->mntFlags);
    return 0;
}

static int32_t prvMountPointDiskFree(const VFS_MountPoint *mnt, void *arg)
{
    int         ret;
    VFS_FSStat  stat;

    ret = VFS_StatFSUser(mnt->mntPath, &stat);
    if (VFS_EOK == ret) {
        shell_printf("%-10s: disk free: %-4d KB [%-3d block, %d bytes per block]\r\n",
                    mnt->mntPath,
                   (stat.blkSize) * (stat.blkFree) / 1024,
                   stat.blkFree,
                   stat.blkSize);
    }
    else {
        shell_printf("%-10s: failed: %d\r\n", mnt->mntPath, ret);
    }

    return 0;
}

#endif


#endif // defined(OS_USING_VFS) && defined(VFS_SHELL_CMD) && defined(OS_USING_SHELL)