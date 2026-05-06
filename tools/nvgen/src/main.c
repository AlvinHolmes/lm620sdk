/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        main.c
 *
 * @brief       程序入口.
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
#include <errno.h>
#include "dirfile.h"
#define EXPORT_GLOBAL
#include "nv_init_defparam.h"

static NVGEN_Func g_nvInitHead[NVGEN_INIT_ORDER_MAX+1];

static int _init_nv_defparams(const char *dest_path, NV_Default *nvDefault)
{
    unsigned int    i;
    NVGEN_Func     *item;
    char            bin_file[260] = {0};
    unsigned char   *buf = (unsigned char *)nvDefault;

    NVGENLOG("---------------------------------------------------------------------");
    for (i = 0; i < sizeof(g_nvInitHead)/sizeof(g_nvInitHead[0]); i++) {
        item = g_nvInitHead[i].next;
        while (item) {
            /* 获取默认参数值 */
            printf("nvgen: Get %16s default paramters", item->modname);
            if (item->definit && item->definit(nvDefault)) {
                printf("\r\n");
                NVGENLOG("call %s(T_NV_DEVCONFIG*) init function fail", item->modname);
                return -1;
            }
            printf("\r\n");
            /* 保存默认参数到文件 */
            sprintf(bin_file, NVGEN_BIN_FILE_NAME_FORMAT, dest_path,
                                        item->partition, item->modname, item->offset);
            if (NVGEN_WriteNewFile(bin_file, buf+item->offset, item->length)) {
                NVGENLOG("generate nvbin file( %s )fail: %s", bin_file, strerror(errno));
                return -1;
            }
            item = item->next;
        }
        if (g_nvInitHead[i].next)
            NVGENLOG("---------------------------------------------------------------------");
    }
    return 0;
}

int main(int argc, char *argv[])
{
    NV_Default      *nvDefault;
    char            *outPath;
    char            *extPath = NULL;
    long            address = -1;
    int             i, ret = 0;

    printf("\r\n");

#ifndef BUILD_IN_SDK
    /* 检查输入参数 */
    if (argc < 2) {
        goto usage;
    }
#else
    if (argc < 3) {
        goto usage;
    } else {
        sdknv_path = argv[2];
        NVGENLOG("sdknv bin path: %s", sdknv_path);
    }
#endif

    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            if (i+1 >= argc)
                goto usage;
            extPath = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "-a") == 0) {
            char *endptr;
            if (i+1 >= argc)
                goto usage;
            address = strtol(argv[i+1], &endptr, 0);
            if (*endptr != '\0' || address < 0) {
                NVGENLOG("address invliad: %s", argv[i+1]);
                goto usage;
            }
            if ((size_t)address >= sizeof(NV_Default)) {
                NVGENLOG("address too big: %u >= %u", address, sizeof(NV_Default));
                goto usage;
            }
            i++;
        }
    }
    if (extPath || address >= 0) {
        if (!(extPath && address >= 0))
            goto usage;
    }

    outPath = argv[1];
    if (outPath[strlen(outPath)-1] == '\\' || outPath[strlen(outPath)-1] == '/')
        outPath[strlen(outPath)-1] = '\0';
    if (strlen(outPath) == 0) {
        NVGENLOG("out_dir is error");
        goto usage;
    }

    /* 创建输出文件夹 */
    if (NVGEN_CreateDirs(outPath)) {
        NVGENLOG("create directory( %s )fail", outPath);
        return -1;
    }
    nvDefault = (NV_Default *)malloc(sizeof(NV_Default));
    if (nvDefault == NULL) {
        NVGENLOG("alloc T_Nv_Default fail: %lu", sizeof(NV_Default));
        return -1;
    }

    /* 所有参数默认ff */
    memset(nvDefault, 0xff, sizeof(NV_Default));

    /* 调用模块接口初始化默认参数 */
    if (ret == 0)
        ret = _init_nv_defparams(outPath, nvDefault);
    if (ret == 0) {
        char bin_file[260] = {0};

        /* 合并额外参数到文件 */
        if ((extPath && address >= 0)) {
            ret = NVGEN_ReadFile(extPath, (uint8_t*)nvDefault + address, sizeof(NV_Default) - address);
            if (ret) {
                NVGENLOG("read %s fail: %d", extPath, ret);
                return -1;
            }
        }

        /* 保存默认参数到文件 */
        sprintf(bin_file, NVGEN_BIN_FILE_NAME_FORMAT, outPath, NV_DEFAULT_PART_NAME, NVGEN_FULL_BIN_FILE, 0);
        if (NVGEN_WriteNewFile(bin_file, nvDefault, sizeof(NV_Default))) {
            NVGENLOG("generate nvfull file( %s )fail: %s", bin_file, strerror(errno));
            ret = -1;
        }
    }
    if (nvDefault)
        free(nvDefault);

    printf("\r\n");
    return ret;

usage:
#ifndef BUILD_IN_SDK
    NVGENLOG("usage: %s out_dir [-f path -a address]", argv[0]);
#else
    NVGENLOG("usage: %s out_dir sdknv_path [-f path -a address]", argv[0]);
#endif
    return -1;
}

void NVGEN_RegisterNvinit(unsigned int order, NVGEN_Func *func)
{
    if (order < sizeof(g_nvInitHead)/sizeof(g_nvInitHead[0]) && func->definit) {
        NVGEN_Func *pre = &g_nvInitHead[order];
        NVGEN_Func *item = pre->next;
        while (item) {
            if (item->offset > func->offset)
                break;
            pre = item;
            item = item->next;
        }
        pre->next = func;
        func->next = item;
    }
}
