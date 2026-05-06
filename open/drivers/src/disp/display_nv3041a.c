/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        disp_nv3041a.c
 *
 * @brief       NV3041A LCD.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team         创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "drv_display.h"
#include <nv3041a.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

static void DispNV3041A_Callback(void *param)
{
    DispDevice_t *pDev = (DispDevice_t *)param;

    if(pDev->cbEvent) {
        pDev->cbEvent(pDev->userData);
    }
}

static int DispNV3041A_Init(DispDevice_t *pDev, void (*callback)(void *data), void *data)
{
    NV3041A_Handle *handle = osMalloc(sizeof(NV3041A_Handle));
    if(!handle) {
        return -1;
    }

    osMemset(handle, 0, sizeof(NV3041A_Handle));
    handle->lcd.pRes = DRV_RES(LCD, 0);
    
    pDev->interface = (LCD_Handle *)handle;
    pDev->userData = data;
    pDev->cbEvent = callback,

    handle->userData = pDev;
    handle->cbEvent = DispNV3041A_Callback;

    NV3041A_PowerControl(handle, true);
    
    NV3041A_Init(handle);

    if(pDev->horRes < pDev->verRes) {
        NV3041A_SetFlip(handle, 1, 0);
        NV3041A_SetScreenMode(handle, NV3041A_SCREEN_PORTRAIT);
    }
    else {
        NV3041A_SetFlip(handle, 0, 0);
        NV3041A_SetScreenMode(handle, NV3041A_SCREEN_LANDSCAPE);
    }

    NV3041A_SetWidth(handle, pDev->horRes, pDev->verRes);

    return 0;
}

static int DispNV3041A_Deinit(DispDevice_t *pDev)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)pDev->interface;

    NV3041A_DeInit(handle);

    NV3041A_PowerControl(handle, false);

    osFree(pDev->interface);
    
    return 0;
}

static void DispNV3041A_Flush(DispDevice_t *pDev, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint8_t *pixel)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)pDev->interface;

    NV3041A_Flush(handle, x1, x2, y1, y2, pixel);

    return;
}

static int DispNV3041A_Flip(DispDevice_t *pDev, bool x, bool y)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)pDev->interface;

    NV3041A_SetFlip(handle, x, y);

    return 0;
}

static int DispNV3041A_SetDirection(struct DisplayDevice *pDev, DisplayDir_t dir)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)pDev->interface;

    switch (dir) {
        case DISPLAY_DIR_ROTATE_0:
            NV3041A_SetFlip(handle, 1, 0);
            NV3041A_SetScreenMode(handle, NV3041A_SCREEN_PORTRAIT);
            break;

        case DISPLAY_DIR_ROTATE_90:
            NV3041A_SetFlip(handle, 1, 1);
            NV3041A_SetScreenMode(handle, NV3041A_SCREEN_LANDSCAPE);
            break;

        case DISPLAY_DIR_ROTATE_180:
            NV3041A_SetFlip(handle, 0, 1);
            NV3041A_SetScreenMode(handle, NV3041A_SCREEN_PORTRAIT);
            break;

        case DISPLAY_DIR_ROTATE_270:
            NV3041A_SetFlip(handle, 0, 0);
            NV3041A_SetScreenMode(handle, NV3041A_SCREEN_LANDSCAPE);
            break;
    
    default:
        break;
    }

    return 0;
}

static int DispNV3041A_SetBackLight(struct DisplayDevice *pDev, uint8_t level)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)pDev->interface;

    return NV3041A_BackLight(handle, level);
}

static int DispNV3041A_Sleep(struct DisplayDevice *pDev)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)pDev->interface;

    NV3041A_Sleep(handle);

    return 0;
}

static int DispNV3041A_Wakeup(struct DisplayDevice *pDev)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)pDev->interface;

    NV3041A_Wakeup(handle);

    return 0;
}

static uint32_t DispSt7789v_GetID(struct DisplayDevice *pDev)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)pDev->interface;

    return NV3041A_GetID(handle);
}

static void DispSt7789v_EndianConvert(struct DisplayDevice *pDev, bool enable)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)pDev->interface;

    NV3041A_EndianConvert(handle, enable);
}

/**
 ************************************************************************************
 * @brief           获取NV3041A的LCD设备
 *
 * @return          获取LCD操作设备.
 * @retval          DispDevice_t                      
 ************************************************************************************
 */
/*
 * 设备接口
 */
static DispOps_t g_dispNV3041A_Ops = {
    .init = DispNV3041A_Init,
    .deinit = DispNV3041A_Deinit,
    .flush = DispNV3041A_Flush,
    .flip = DispNV3041A_Flip,
    .backlight = DispNV3041A_SetBackLight,
    .sleep = DispNV3041A_Sleep,
    .wakeup = DispNV3041A_Wakeup,
    .direction = DispNV3041A_SetDirection,
    .endianConvert = DispSt7789v_EndianConvert,
    .id = DispSt7789v_GetID
};
/*
 * 操作设备
 */
static DispDevice_t g_dispNV3041ADev = {
    .interface = NULL,
    .ops = &g_dispNV3041A_Ops,
    .cbEvent = NULL,
    .userData = NULL,
};

DispDevice_t * DispNV3041A_GetDevice(void)
{
    return &g_dispNV3041ADev;
}

DispOps_t * DispNV3041A_GetOps(void)
{
    return &g_dispNV3041A_Ops;
}
