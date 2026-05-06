/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        dlimg.h
 *
 * @brief       动态加载镜像.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-10-21   ict team        创建
 ************************************************************************************
 */

#ifndef  _DLOAD_IMAGE_H_
#define  _DLOAD_IMAGE_H_
#include <stdint.h>

/**
 * @brief Symbol of image's head
 */
#define DLMO_IMAGE_HEAD        "APPS"

/**
 * @brief Image head.
 */
typedef struct {
    int8_t      symb[4];            ///< Symbol of image's head.
    uint32_t    addr_store;               ///store addr of app[flash].
    uint32_t    addr_load;               //run addr of app[psram].
    uint32_t    size;               ///< Size of image in bytes including head.
    uint32_t    resv;               ///< Resv of image.
}DLMO_ImageHead;
/**
 * @brief Image tail.
 */
typedef struct {
    uint8_t     md5[16];            ///< MD5 of image data excluding tail.
}DLMO_ImageTail;

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif

