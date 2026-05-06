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

#ifndef _DRV_DISPLAY_H_
#define _DRV_DISPLAY_H_

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <os.h>
#include <drv_lcd.h>

/**
 * @addtogroup Display
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

typedef enum
{
    DISPLAY_DIR_ROTATE_0 = 0,
    DISPLAY_DIR_ROTATE_90,
    DISPLAY_DIR_ROTATE_180,
    DISPLAY_DIR_ROTATE_270,
} DisplayDir_t;

typedef struct DisplayDevice DispDevice_t;

typedef struct DispOps
{
    int (*init)(struct DisplayDevice *pDev, void (*callback)(void *data), void *data);
    int (*deinit)(struct DisplayDevice *pDev);
    void (*flush)(struct DisplayDevice *pDev, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint8_t *pixel);
    int (*flip)(struct DisplayDevice *pDev, bool x, bool y);
    int (*backlight)(struct DisplayDevice *pDev, uint8_t level);
    int (*sleep)(struct DisplayDevice *pDev);
    int (*wakeup)(struct DisplayDevice *pDev);
    int (*direction)(struct DisplayDevice *pDev, DisplayDir_t dir);
    void (*endianConvert)(struct DisplayDevice *pDev, bool enable);
    uint32_t (*id)(struct DisplayDevice *pDev);
} DispOps_t;

/**
 * @brief 显示设备
 */
typedef struct DisplayDevice
{
    LCD_Handle *interface;

    uint16_t horRes;         /**< Horizontal resolution.*/
    uint16_t verRes;         /**< Vertical resolution.*/

    void  *userData;
    void (*cbEvent)(void *userData);

    osCompletion cmpl;

    DispOps_t   *ops;
} DispDevice_t;

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief           获取模拟器的LCD设备
 *
 * @return          获取LCD操作设备.
 * @retval          DispDevice_t                      
 ************************************************************************************
 */
DispDevice_t * DispEmu_GetDevice(void);

/**
 ************************************************************************************
 * @brief           获取某厂家的的LCD设备
 *
 * @return          获取LCD操作设备.
 * @retval          DispDevice_t                      
 ************************************************************************************
 */
DispDevice_t * DispXxx_GetDevice(void);
DispDevice_t * DispST7789V_GetDevice(void);
DispOps_t * DispST7789V_GetOps(void);

DispDevice_t * DispNV3041A_GetDevice(void);
DispOps_t * DispNV3041A_GetOps(void);

#endif /* End of _DRV_DISP_H_*/
/** @} */