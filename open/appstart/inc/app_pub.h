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

#ifndef __APP_PUB_H__
#define __APP_PUB_H__

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "slog_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#if 1
#define APP_PRINT_INFO(format, ...)   slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_APP, format, ##__VA_ARGS__)
#define APP_PRINT_ERROR(format, ...)  slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_APP, format, ##__VA_ARGS__)
#else
#define APP_PRINT_INFO(format, ...)   osPrintf(format, ##__VA_ARGS__)
#define APP_PRINT_ERROR(format, ...)  osPrintf(format, ##__VA_ARGS__)
#endif

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/


#ifdef __cplusplus
}
#endif
#endif//__APP_PUB_H__


