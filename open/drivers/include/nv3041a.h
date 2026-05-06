#ifndef __NV3041A_H__
#define __NV3041A_H__

#include <os.h>
#include <drv_common.h>
#include <drv_lcd.h>
#include <drv_lcd_te.h>
#include <drv_pin.h>

/**
 * @addtogroup NV3041A
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
/**
 * @brief 输入设备类型
 */
typedef enum {
    NV3041A_SCREEN_PORTRAIT,    // 竖屏
    NV3041A_SCREEN_LANDSCAPE,   // 横屏
} NV3041A_SCREEN_MODE;

typedef struct {
    PIN_MultiMux_t *backLight;
    PIN_MultiMux_t *reset;
    PIN_MultiMux_t *te;
    PIN_MultiMux_t *csx;
    PIN_MultiMux_t *scl;
    PIN_MultiMux_t *sdo;
    PIN_MultiMux_t *sdi;
    PIN_MultiMux_t *dcx;
}NV3041A_PinConfig;

typedef struct {
    uint8_t     freq;
    uint8_t     diff;
    uint8_t     high;
    uint8_t     low;
    uint8_t     level;
    void        *timer;
} NV3041A_PwmHandle;

typedef struct {
    LCD_Handle lcd;

    void  *userData;
    void (*cbEvent)(void *userData);

    uint8_t madctl;
    uint8_t madctl_last;
    uint8_t porch_last;

    uint8_t fmark_enable : 1;
    uint8_t endian_enable : 1;

    LCD_TeHandle    te;

    NV3041A_PwmHandle pwm;

    NV3041A_PinConfig   *pin;
}NV3041A_Handle;

void NV3041A_Init(NV3041A_Handle *hdl);
void NV3041A_DeInit(NV3041A_Handle *hdl);
void NV3041A_SetWidth(NV3041A_Handle *hdl, uint16_t width, uint16_t height);
void NV3041A_Flush(NV3041A_Handle *hdl, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint8_t *pixel);
void NV3041A_PinInit(PIN_MultiMux_t *bk, PIN_MultiMux_t *reset, PIN_MultiMux_t *csx, PIN_MultiMux_t *scl, PIN_MultiMux_t *sdi, PIN_MultiMux_t *sdo, PIN_MultiMux_t *dcx);
int32_t NV3041A_BackLight(NV3041A_Handle *hdl, uint8_t level);
void NV3041A_PowerControl(NV3041A_Handle *hdl, bool en);
void NV3041A_Sleep(NV3041A_Handle *hdl);
void NV3041A_Wakeup(NV3041A_Handle *hdl);
void NV3041A_SetScreenMode(NV3041A_Handle *hdl, NV3041A_SCREEN_MODE mode);
void NV3041A_SetFlip(NV3041A_Handle *hdl, bool x, bool y);
uint32_t NV3041A_GetID(NV3041A_Handle *hdl);
void NV3041A_EndianConvert(NV3041A_Handle *hdl, bool enable);
#endif
/** @} */