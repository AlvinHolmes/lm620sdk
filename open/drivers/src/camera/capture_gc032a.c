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
#include <os.h>
#include "drv_capture.h"
#include <gc032a.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

static int CamGC032A_Init(CaptureDevice_t *pDev, void (*callback)(void *data), void *data)
{
    GC032A_Handle *handle = osMalloc(sizeof(GC032A_Handle));
    if(!handle) {
        return -1;
    }
    
    osMemset(handle, 0, sizeof(GC032A_Handle));
    handle->spiCam.res = DRV_RES(CAM, 0);

    pDev->interface = (void *)handle;

    if(GC032A_Init(handle, callback, data, pDev->horRes, pDev->verRes)) {
        return -1;
    }

    pDev->imageSize = pDev->horRes * pDev->verRes * 2;

    return 0;
}

static int CamGC032A_Deinit(CaptureDevice_t *pDev)
{
    GC032A_Handle *handle = (GC032A_Handle *)pDev->interface;
    
    GC032A_DeInit(handle);

    osFree(pDev->interface);

    return 0;
}

static void CamGC032A_Start(struct CaptureDevice *pDev)
{
    GC032A_Handle *handle = (GC032A_Handle *)pDev->interface;

    GC032A_Start(handle);

    return;
}

static int CamGC032A_Stop(struct CaptureDevice *pDev)
{
    GC032A_Handle *handle = (GC032A_Handle *)pDev->interface;

    GC032A_Stop(handle);

    return 0;
}

static uint8_t* CamGC032A_Capture(struct CaptureDevice *pDev)
{
    GC032A_Handle *handle = (GC032A_Handle *)pDev->interface;

    uint8_t *addr = GC032A_Capture(handle);

    return addr;
}

static int CamGC032A_Release(struct CaptureDevice *pDev, uint8_t *image)
{
    GC032A_Handle *handle = (GC032A_Handle *)pDev->interface;

    GC032A_ImageRelease(handle, image);

    return 0;
}

static uint32_t CamGC032A_GetChipID(struct CaptureDevice *pDev)
{
    GC032A_Handle *handle = (GC032A_Handle *)pDev->interface;

    return GC032A_IdGet(handle);
}

static void CamGC032A_Control(struct CaptureDevice *pDev, uint32_t cmd, uint32_t arg)
{
    GC032A_Handle *handle = (GC032A_Handle *)pDev->interface;

    switch (cmd) {
        case CAPTURE_CMD_SET_AWB:
            GC032A_Control(handle, GC032A_SET_AWB, arg);
            break;

        case CAPTURE_CMD_SET_BRIGHTNESS:
            GC032A_Control(handle, GC032A_SET_BRIGHTNESS, arg);
            break;

        case CAPTURE_CMD_SET_CONTRAST:
            GC032A_Control(handle, GC032A_SET_CONTRAST, arg);
            break;

        case CAPTURE_CMD_SET_EV:
            GC032A_Control(handle, GC032A_SET_EV, arg);
            break;
        
        default:
            break;
    }
}

/**
 ************************************************************************************
 * @brief           获取GC032A的CAMERA设备
 *
 * @return          获取LCD操作设备.
 * @retval          CaptureDevice_t                      
 ************************************************************************************
 */
/*
 * 操作设备
 */
static CaptureDevice_t g_camGC032ADev ={
    .init = CamGC032A_Init,
    .deinit = CamGC032A_Deinit,
    .start = CamGC032A_Start,
    .stop = CamGC032A_Stop,
    .capture = CamGC032A_Capture,
    .release = CamGC032A_Release,
    .getChipID = CamGC032A_GetChipID,
    .control = CamGC032A_Control,
};

CaptureDevice_t * CamGC032A_GetDevice(void)
{
    return &g_camGC032ADev;
}
