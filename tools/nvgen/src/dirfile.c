/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        dir_file.c
 *
 * @brief       目录文件操作接口实现.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-25     ict team          创建
 ************************************************************************************
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "dirfile.h"

#ifdef WIN32
#include <io.h>
#include <direct.h>
int NVGEN_CreateDirs(const char *path)
{
    size_t i, len = strlen(path);
    char dir_path[260] = { 0 };

    strcpy(dir_path, path);
    if (dir_path[len - 1] != '\\' || dir_path[len - 1] == '/') {
        dir_path[len++] = '/';
    }
    dir_path[len] = '\0';
    for (i = 0; i < len; i++) {
        if (dir_path[i] == '\\' || dir_path[i] == '/') {
            dir_path[i] = '\0';
            if (_access(dir_path, 0) < 0) {
                if (_mkdir(dir_path) < 0) {
                    return -1;
                }
            }
            dir_path[i] = '/';
        }
    }
    return 0;
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
int NVGEN_CreateDirs(const char *path)
{
    size_t i, len = strlen(path);
    char dir_path[260] = {0};

    strcpy(dir_path, path);
    if (dir_path[len-1] != '/') {
        dir_path[len++] = '/';
    }
    dir_path[len] = '\0';
    for (i = 1; i < len; i++) {
        if (dir_path[i] == '/') {
            dir_path[i] = '\0';
            if (access(dir_path, F_OK) < 0) {
                mode_t  pre = umask(0002);
                if (mkdir(dir_path, 0755) < 0) {
                    umask(pre);
                    return -1;
                }
                umask(pre);
            }
            dir_path[i] = '/';
        }
    }
    return 0;
}

#endif

int NVGEN_ReadFile(const char* file, void *buf, unsigned int len)
{
    int ret  = -1;
    FILE *fp = fopen(file, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_SET);
        fread(buf, 1, len, fp);
        ret = ferror(fp);
        fclose(fp);
    }
    return ret;
}

int NVGEN_WriteNewFile(const char* file, const void *buf, unsigned int len)
{
    int ret = -1;
    FILE *fp = fopen(file, "wb+");
    if (fp) {
        if (fwrite(buf, 1, len, fp) == len) {
            ret = 0;
        }
        fclose(fp);
    }
    return ret;
}
