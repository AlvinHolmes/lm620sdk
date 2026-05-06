/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        disp_st7789v.c
 *
 * @brief       ST7789V LCD.
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
#include <st7789v.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

static void DispST7789V_Callback(void *param)
{
    DispDevice_t *pDev = (DispDevice_t *)param;

    if(pDev->cbEvent) {
        pDev->cbEvent(pDev->userData);
    }
}

static int DispST7789V_Init(DispDevice_t *pDev, void (*callback)(void *data), void *data)
{
    ST7789V_Handle *handle = osMalloc(sizeof(ST7789V_Handle));
    if(!handle) {
        return -1;
    }

    osMemset(handle, 0, sizeof(ST7789V_Handle));
    handle->lcd.pRes = DRV_RES(LCD, 0);
    
    pDev->interface = (LCD_Handle *)handle;
    pDev->userData = data;
    pDev->cbEvent = callback,

    handle->userData = pDev;
    handle->cbEvent = DispST7789V_Callback;

    ST7789V_PowerControl(handle, true);
    
    ST7789V_Init(handle);

    if(pDev->horRes < pDev->verRes) {
        ST7789V_SetFlip(handle, 1, 0);
        ST7789V_SetScreenMode(handle, ST7789V_SCREEN_PORTRAIT);
    }
    else {
        ST7789V_SetFlip(handle, 0, 0);
        ST7789V_SetScreenMode(handle, ST7789V_SCREEN_LANDSCAPE);
    }

    ST7789V_SetWidth(handle, pDev->horRes, pDev->verRes);

    return 0;
}

static int DispST7789V_Deinit(DispDevice_t *pDev)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)pDev->interface;

    ST7789V_DeInit(handle);

    ST7789V_PowerControl(handle, false);

    osFree(pDev->interface);
    
    return 0;
}

static void DispST7789V_Flush(DispDevice_t *pDev, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint8_t *pixel)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)pDev->interface;

    ST7789V_Flush(handle, x1, x2, y1, y2, pixel);

    return;
}

static int DispST7789V_Flip(DispDevice_t *pDev, bool x, bool y)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)pDev->interface;

    ST7789V_SetFlip(handle, x, y);

    return 0;
}

static int DispST7789V_SetDirection(struct DisplayDevice *pDev, DisplayDir_t dir)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)pDev->interface;

    switch (dir) {
        case DISPLAY_DIR_ROTATE_0:
            ST7789V_SetFlip(handle, 1, 0);
            ST7789V_SetScreenMode(handle, ST7789V_SCREEN_PORTRAIT);
            break;

        case DISPLAY_DIR_ROTATE_90:
            ST7789V_SetFlip(handle, 1, 1);
            ST7789V_SetScreenMode(handle, ST7789V_SCREEN_LANDSCAPE);
            break;

        case DISPLAY_DIR_ROTATE_180:
            ST7789V_SetFlip(handle, 0, 1);
            ST7789V_SetScreenMode(handle, ST7789V_SCREEN_PORTRAIT);
            break;

        case DISPLAY_DIR_ROTATE_270:
            ST7789V_SetFlip(handle, 0, 0);
            ST7789V_SetScreenMode(handle, ST7789V_SCREEN_LANDSCAPE);
            break;
    
    default:
        break;
    }

    return 0;
}

static int DispST7789V_SetBackLight(struct DisplayDevice *pDev, uint8_t level)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)pDev->interface;

    return ST7789V_BackLight(handle, level);
}

static int DispST7789V_Sleep(struct DisplayDevice *pDev)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)pDev->interface;

    ST7789V_Sleep(handle);

    return 0;
}

static int DispST7789V_Wakeup(struct DisplayDevice *pDev)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)pDev->interface;

    ST7789V_Wakeup(handle);

    return 0;
}

static uint32_t DispSt7789v_GetID(struct DisplayDevice *pDev)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)pDev->interface;

    return ST7789V_GetID(handle);
}

static void DispSt7789v_EndianConvert(struct DisplayDevice *pDev, bool enable)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)pDev->interface;

    ST7789V_EndianConvert(handle, enable);
}

/**
 ************************************************************************************
 * @brief           获取ST7789V的LCD设备
 *
 * @return          获取LCD操作设备.
 * @retval          DispDevice_t                      
 ************************************************************************************
 */
/*
 * 设备接口
 */
static DispOps_t g_dispST7789V_Ops = {
    .init = DispST7789V_Init,
    .deinit = DispST7789V_Deinit,
    .flush = DispST7789V_Flush,
    .flip = DispST7789V_Flip,
    .backlight = DispST7789V_SetBackLight,
    .sleep = DispST7789V_Sleep,
    .wakeup = DispST7789V_Wakeup,
    .direction = DispST7789V_SetDirection,
    .endianConvert = DispSt7789v_EndianConvert,
    .id = DispSt7789v_GetID
};
/*
 * 操作设备
 */
static DispDevice_t g_dispST7789VDev = {
    .interface = NULL,
    .ops = &g_dispST7789V_Ops,
    .cbEvent = NULL,
    .userData = NULL,
};

DispDevice_t * DispST7789V_GetDevice(void)
{
    return &g_dispST7789VDev;
}

DispOps_t * DispST7789V_GetOps(void)
{
    return &g_dispST7789V_Ops;
}
