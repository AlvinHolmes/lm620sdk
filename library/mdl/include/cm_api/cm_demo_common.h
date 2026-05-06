/**
 * @file        cm_demo_common.h
 * @brief       opencpu 通用头文件
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By tw
 * @date        2021/03/18
 */

#ifndef __CM_COMMON_H__
#define __CM_COMMON_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/
 
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef FALSE
#define FALSE (0U)
#endif

#ifndef TRUE
#define TRUE (1U)
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/
 

/****************************************************************************
 * Public Data
 ****************************************************************************/


/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* 因为底层默认配置，printf 接口最多支持打印 128 字节 */
extern int cm_vprintf(const char *fmt, va_list args);
extern int cm_printf(const char *fmt, ...);


#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#define cm_demo_printf cm_printf

#undef EXTERN
#ifdef __cplusplus
}
#endif

/** @}*/
/** @}*/

#endif /* __CM_COMMON_H__ */
