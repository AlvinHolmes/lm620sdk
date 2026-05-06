/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file main.c
*
* @brief  main函数入口文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/

#include "drv_display.h"
#include "drv_capture.h"
#include "drv_input.h"
#include "lvgl_port.h"
#include "nr_micro_shell.h"

#define DISP_WIDTH      240
#define DISP_HEIGHT     320

int main(void)
{
    return 0;
}


/**
 ************************************************************************************
 * @brief           LVGL初始化
 *
 * @param[in]       void
 *
 * @return          int32_t
 * @retval          0          成功
 ************************************************************************************
*/
int LVGL_Init(void)
{
    DispDevice_t * pDispDev = DispST7789V_GetDevice();

    // pDispDev->interface = &g_boardLcd;
    // pDispDev->interface->pRes = DRV_RES(LCD, 0);

    pDispDev->horRes = DISP_WIDTH;
    pDispDev->verRes = DISP_HEIGHT;

    InputTouch_t *touch = (InputTouch_t*)InputEmu_GetTouchDevice();
    touch->touchWidth = DISP_WIDTH;
    touch->touchHeight = DISP_HEIGHT;
    
    // return LVGL_PortInit(pDispDev, InputEmu_GetTouchDevice(), InputEmu_GetKeypadDevice(), InputEmu_GetButtonDevice());
    return LVGL_PortInit(pDispDev, NULL, NULL, NULL);
}
INIT_APP_EXPORT(LVGL_Init, OS_INIT_SUBLEVEL_HIGH);