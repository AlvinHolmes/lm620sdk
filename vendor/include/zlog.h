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


#ifndef __ZLOG_H__
#define __ZLOG_H__

#ifdef ZLOG_TO_SLOG
#include "slog_print.h"
#else
#include <os.h>
#endif

#ifndef ZLOG_TARGET
#define ZLOG_TARGET   "zlog"
#endif

// ZLOGN    直接打印输出
// ZLOGD    打印调试信息
// ZLOGE    打印错误信息
// ZLOGI    打印提示信息
// ZLOGW    打印警告信息
#ifdef ZLOG_DEBUG
    #ifdef ZLOG_TO_SLOG
        #define ZLOGN(fmt, args...) slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_APP, fmt, ## args)
    #else
        #define ZLOGN(fmt, args...) osPrintf(fmt, ## args)
    #endif
#else
    ZLOGN(fmt, args...)
#endif
#ifdef ZLOG_DEBUG
    #ifdef ZLOG_COLOR
        #ifdef ZLOG_TO_SLOG
            #define ZLOGD(fmt, args...) slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_APP, "[%s] " fmt, ZLOG_TARGET, ## args)
        #else
            #define ZLOGD(fmt, args...) osPrintf("\033[;33m[D][%s] \033[0m" fmt, ZLOG_TARGET, ## args)
        #endif
    #else
        #ifdef ZLOG_TO_SLOG
            #define ZLOGD(fmt, args...) slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_APP, "[%s] " fmt, ZLOG_TARGET, ## args)
        #else
            #define ZLOGD(fmt, args...) osPrintf("[D][%s] " fmt, ZLOG_TARGET, ## args)
        #endif
    #endif
#else
    #define ZLOGD(fmt, args...)
#endif

/*
 * The color for terminal (foreground)
 * BLACK    30
 * RED      31
 * GREEN    32
 * YELLOW   33
 * BLUE     34
 * PURPLE   35
 * CYAN     36
 * WHITE    37
 */

#ifdef ZLOG_COLOR
    #ifdef ZLOG_TO_SLOG
        #define ZLOGE(fmt, args...) slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_APP, "[%s] " fmt, ZLOG_TARGET, ## args)
        #define ZLOGI(fmt, args...) slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_APP, "[%s] " fmt, ZLOG_TARGET, ## args)
        #define ZLOGW(fmt, args...) slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_APP, "[%s] " fmt, ZLOG_TARGET, ## args)
    #else
        #define ZLOGE(fmt, args...) osPrintf("\033[;33m[E][%s] \033[;31mLINE: %d FUNC: %s " fmt "\033[0m", ZLOG_TARGET, __LINE__, __func__,  ## args)
        #define ZLOGI(fmt, args...) osPrintf("\033[;33m[I][%s] \033[;32m" fmt "\033[0m", ZLOG_TARGET, ## args)
        #define ZLOGW(fmt, args...) osPrintf("\033[;33m[W][%s] \033[;35m" fmt "\033[0m", ZLOG_TARGET, ## args)
    #endif
#else
    #ifdef ZLOG_TO_SLOG
        #define ZLOGE(fmt, args...) slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_APP, "[%s] " fmt, ZLOG_TARGET, ## args)
        #define ZLOGI(fmt, args...) slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_APP, "[%s] " fmt, ZLOG_TARGET, ## args)
        #define ZLOGW(fmt, args...) slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_APP, "[%s] " fmt, ZLOG_TARGET, ## args)
    #else
        #define ZLOGE(fmt, args...) osPrintf("[E][%s] LINE: %d FUNC: %s " fmt, ZLOG_TARGET, __LINE__, __func__, ## args)
        #define ZLOGI(fmt, args...) osPrintf("[I][%s] " fmt, ZLOG_TARGET, ## args)
        #define ZLOGW(fmt, args...) osPrintf("[W][%s] " fmt, ZLOG_TARGET, ## args)
    #endif
#endif



#endif  // __ZLOG_H__
