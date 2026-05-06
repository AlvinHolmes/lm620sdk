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
 * 2023-08-01     ict team          创建
 ************************************************************************************
 */
#ifndef __RTMSYM_H__
#define __RTMSYM_H__


/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <os.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define RTM_IMPORT_FUNC(type, symbol, args, args_name)                         \
type symbol args                                                               \
{                                                                              \
    static void * __rtmsym_##symbol##_function__ = NULL;                       \
                                                                               \
    if (__rtmsym_##symbol##_function__ == NULL) {                              \
        __rtmsym_##symbol##_function__ = rtmsym_find(#symbol);                 \
    }                                                                          \
                                                                               \
    return ((type (*)args)__rtmsym_##symbol##_function__) args_name;           \
}

#define RTM_IMPORT_VAR(type, function, symbol)                                 \
type function(void)                                                            \
{                                                                              \
    static void * __rtmsym_##symbol##_ptr__ = NULL;                            \
                                                                               \
    if (__rtmsym_##symbol##_ptr__ == NULL) {                                   \
        __rtmsym_##symbol##_ptr__ = rtmsym_find(#symbol);                      \
    }                                                                          \
                                                                               \
    return ((type)__rtmsym_##symbol##_ptr__);                                  \
}

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
 typedef struct {
  struct osModuleSymtab * start;
  struct osModuleSymtab * end;
 }T_sRtmSymInfo;

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
extern int rtmsym_initialize(const T_sRtmSymInfo * symtab);
extern void * rtmsym_find(const char * symbol);

// extern void * g_osPrintf_symbol;
// #define osPrintf(fmt, args...) ((void (*)(const char *, ...))g_osPrintf_symbol)(fmt, ## args)


#ifdef __cplusplus
}
#endif
#endif//__RTMSYM_H__


