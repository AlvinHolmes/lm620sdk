/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        nv_init_defparam.h
 *
 * @brief       初始化宏定义.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-23     ict team          创建
 ************************************************************************************
 */

#ifndef _NVGEN_INIT_DEFPARAM_H_
#define _NVGEN_INIT_DEFPARAM_H_
#include "nvparam_top.h"

#define NVGEN_INIT_ORDER_MAX            (4)

#define NVGEN_BIN_FILE_NAME_FORMAT      ("%s/%s_%s_0x%08X.bin")
#define NVGEN_FULL_BIN_FILE             ("full")

typedef struct _NVGEN_Func {
    struct _NVGEN_Func  *next;
    const char          *partition;
    const char          *modname;
    unsigned int        offset;
    unsigned int        length;
    int (*definit)(NV_Default *nvDefault);
}NVGEN_Func;

#ifdef _MSC_VER
typedef int(*before_main)(void);

#define NVGEN_NV_DEFINIT(mod, funname, order)       \
static int funname(NV_Default *nvDefault);          \
static NVGEN_Func _##mod##_init = {0};              \
static int nvinit_##mod##funname##order(void) {     \
    _##mod##_init.next = 0;                         \
    _##mod##_init.partition = NV_DEFAULT_PART_NAME; \
    _##mod##_init.modname = #mod;                   \
    _##mod##_init.offset = NV_ITEM_ADDR(mod);       \
    _##mod##_init.length = NV_ITEM_SIZE(mod##Area); \
    _##mod##_init.definit = funname;                \
    NVGEN_RegisterNvinit(order, &_##mod##_init);    \
    return 0;                                       \
}                                                   \
__pragma( data_seg(".CRT$XIU") )                    \
static before_main bm_##mod##funname##order = nvinit_##mod##funname##order; \
__pragma( data_seg())

#else

#define NVGEN_NV_DEFINIT(mod, funname, order)       \
static int funname(NV_Default *nvDefault);          \
static NVGEN_Func _##mod##_init;                    \
static void __attribute__((constructor)) nvinit_##mod##funname##order(void) { \
    _##mod##_init.next = 0;                         \
    _##mod##_init.partition = NV_DEFAULT_PART_NAME; \
    _##mod##_init.modname = #mod;                   \
    _##mod##_init.offset = NV_ITEM_ADDR(mod);       \
    _##mod##_init.length = NV_ITEM_SIZE(mod##Area); \
    _##mod##_init.definit = funname;                \
    NVGEN_RegisterNvinit(order, &_##mod##_init);    \
}

#endif

#define NVGENLOG(fmt, ...)      printf("nvgen: " fmt "\r\n", ##__VA_ARGS__)

#define PRINTNVINFO(nvitem)     NVGENLOG("%16s offset: 0x%08X, size: %d", #nvitem, \
                                (unsigned int)NV_ITEM_ADDR(nvitem), (unsigned int)NV_ITEM_SIZE(nvitem))

#ifdef EXPORT_GLOBAL
#define GLOBAL_VAR
#else
#define GLOBAL_VAR extern
#endif

GLOBAL_VAR const char *sdknv_path;

#ifdef __cplusplus
extern "C" {
#endif

void NVGEN_RegisterNvinit(unsigned int order, NVGEN_Func *func);

#ifdef __cplusplus
}
#endif

#endif /* _NVGEN_INIT_DEFPARAM_H_ */
