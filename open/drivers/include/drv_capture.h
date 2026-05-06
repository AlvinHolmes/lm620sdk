/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_disp.h
 *
 * @brief       显示驱动接口.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team         创建
 ************************************************************************************
 */

#ifndef _DRV_CAPTURE_H_
#define _DRV_CAPTURE_H_

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <os.h>
#include <stdint.h>

/**
 * @addtogroup Capture
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum {
    CAPTURE_CMD_SET_AWB,
    CAPTURE_CMD_SET_BRIGHTNESS,
    CAPTURE_CMD_SET_CONTRAST,
    CAPTURE_CMD_SET_EV,
}CaptureCmd_t;

/**
 * @brief CAMERA设备
 */
typedef struct CaptureDevice
{
    void *interface;

    uint16_t horRes;         /**< Horizontal resolution.*/
    uint16_t verRes;         /**< Vertical resolution.*/
    uint32_t imageSize;
    
    int (*init)(struct CaptureDevice *pDev, void (*callback)(void *data), void *data);
    int (*deinit)(struct CaptureDevice *pDev);
    void (*start)(struct CaptureDevice *pDev);
    int (*stop)(struct CaptureDevice *pDev);
    uint8_t* (*capture)(struct CaptureDevice *pDev);
    int (*release)(struct CaptureDevice *pDev, uint8_t *image);
    uint32_t (*getChipID)(struct CaptureDevice *pDev);
    void (*control)(struct CaptureDevice *pDev, uint32_t cmd, uint32_t arg);
} CaptureDevice_t;

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief           获取模拟器的CAMERA设备
 *
 * @return          获取CAMERA操作设备.
 * @retval          CaptureDevice_t                      
 ************************************************************************************
 */
// CaptureDevice_t * CameraEmu_GetDevice(void);

/**
 ************************************************************************************
 * @brief           获取某厂家的的CAMERA设备
 *
 * @return          获取CAMERA操作设备.
 * @retval          CaptureDevice_t                      
 ************************************************************************************
 */
// CaptureDevice_t * DispXxx_GetDevice(void);
CaptureDevice_t * CamGC032A_GetDevice(void);

#endif /* End of _DRV_CAMERA_H_*/
/** @} */