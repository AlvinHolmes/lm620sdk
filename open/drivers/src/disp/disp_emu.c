/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        disp_emu.c
 *
 * @brief       模拟器LCD.
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
#include "drv_lcd.h"
#include "drv_display.h"

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define CMD_RDDID    (0x04)
#define CMD_CASET    (0x2A)
#define CMD_RASET    (0x2B)
#define CMD_RAMWR    (0x2C)

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

static int DispEmu_Init(DispDevice_t *pDev, void (*callback)(void *data), void *data)
{
    LCD_Handle *handle = pDev->interface;

    handle->userData = data;
    handle->SendCallback = callback;
    
    LCD_Initialize(handle);
    
    LCD_SetEmu(handle, RGB_5_6_5, pDev->horRes, pDev->verRes);

    return 0;
}

static int DispEmu_Deinit(DispDevice_t *pDev)
{
    LCD_Handle *handle = pDev->interface;
    
    LCD_Uninitialize(handle);
    
    return 0;
}

static void DispEmu_Flush(DispDevice_t *pDev, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint8_t *pixel)
{
    LCD_Handle *handle = pDev->interface;
    uint16_t x[2];
    uint16_t y[2];
        
    x[0] = (x1 >> 8) | (x1 << 8);
    x[1] = (x2 >> 8) | (x2 << 8);
    
    LCD_WriteParam(handle, CMD_CASET, (uint8_t*)x, sizeof(x));

    y[0] = (y1 >> 8) | (y1 << 8);
    y[1] = (y2 >> 8) | (y2 << 8);
    LCD_WriteParam(handle, CMD_RASET, (uint8_t*)y, sizeof(y));

    LCD_SendData(handle, CMD_RAMWR, pixel, (x2 - x1 + 1) * (y2 - y1 + 1) * 2);
#if 0
    {
        int i,j;
        
        for(i = y1; i <= y2; i++)
        {
            for(j = x1; j <= x2; j++)
            {
                uint16_t rgb = ((uint16_t*)pixel)[(i - y1) * (x2 - x1 + 1) + j - x1];
                
                osPrintf("pixel(%d-(%d,%d)) set rgb: %d,%d,%d\r\n", i*240 + j, j, i, (rgb >> 11) << 3, ((rgb >> 5) & 0x3f) << 2, (rgb & 0x1f)<<3);
            }
        }
    }
#endif
    return;
}

/**
 ************************************************************************************
 * @brief           获取模拟器的LCD设备
 *
 * @return          获取LCD操作设备.
 * @retval          DispDevice_t                      
 ************************************************************************************
 */
/*
 * 设备接口
 */
static DispOps_t g_dispEmuOps ={
    .init = DispEmu_Init,
    .flush = DispEmu_Flush,
    .deinit = DispEmu_Deinit,
};
/*
 * 操作设备
 */
static DispDevice_t g_dispEmuDev ={
    .ops = &g_dispEmuOps,
};

DispDevice_t * DispEmu_GetDevice(void)
{
    return &g_dispEmuDev;
}

