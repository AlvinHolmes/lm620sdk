#ifndef __ST7789V_H__
#define __ST7789V_H__

#include <os.h>
#include <drv_common.h>
#include <drv_lcd.h>
#include <drv_lcd_te.h>
#include <drv_pin.h>

/**
 * @addtogroup St7789v
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
    ST7789V_SCREEN_PORTRAIT,    // 竖屏
    ST7789V_SCREEN_LANDSCAPE,   // 横屏
} ST7789V_SCREEN_MODE;

typedef struct {
    PIN_MultiMux_t *backLight;
    PIN_MultiMux_t *reset;
    PIN_MultiMux_t *te;
    PIN_MultiMux_t *csx;
    PIN_MultiMux_t *scl;
    PIN_MultiMux_t *sdo;
    PIN_MultiMux_t *sdi;
    PIN_MultiMux_t *dcx;
}ST7789V_PinConfig;

typedef struct {
    uint8_t     freq;
    uint8_t     diff;
    uint8_t     high;
    uint8_t     low;
    uint8_t     level;
    void        *timer;
} ST7789V_PwmHandle;

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

    ST7789V_PwmHandle pwm;

    ST7789V_PinConfig   *pin;
}ST7789V_Handle;

typedef struct
{
    void (*lcdInit)(ST7789V_Handle *hdl);
    void (*lcdDeInit)(ST7789V_Handle *hdl);
    void (*lcdPowerControl)(ST7789V_Handle *hdl, bool en);
    void (*lcdSetFlip)(ST7789V_Handle *hdl, bool x, bool y);
    void (*lcdSetWidth)(ST7789V_Handle *hdl, uint16_t width, uint16_t height);
    void (*lcdSetScreenMode)(ST7789V_Handle *hdl, ST7789V_SCREEN_MODE mode);
    void (*lcdFlush)(ST7789V_Handle *hdl, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint8_t *pixel);
    void (*lcdSleep)(ST7789V_Handle *hdl);
    void (*lcdWakeup)(ST7789V_Handle *hdl);
    void (*lcdEndianConvert)(ST7789V_Handle *hdl, bool enable);
} LcdOps_t;

void ST7789V_Init(ST7789V_Handle *hdl);
void ST7789V_DeInit(ST7789V_Handle *hdl);
void ST7789V_SetWidth(ST7789V_Handle *hdl, uint16_t width, uint16_t height);
void ST7789V_Flush(ST7789V_Handle *hdl, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint8_t *pixel);
void ST7789V_PinInit(PIN_MultiMux_t *bk, PIN_MultiMux_t *reset, PIN_MultiMux_t *csx, PIN_MultiMux_t *scl, PIN_MultiMux_t *sdi, PIN_MultiMux_t *sdo, PIN_MultiMux_t *dcx);
int32_t ST7789V_BackLight(ST7789V_Handle *hdl, uint8_t level);
void ST7789V_PowerControl(ST7789V_Handle *hdl, bool en);
void ST7789V_Sleep(ST7789V_Handle *hdl);
void ST7789V_Wakeup(ST7789V_Handle *hdl);
void ST7789V_SetScreenMode(ST7789V_Handle *hdl, ST7789V_SCREEN_MODE mode);
void ST7789V_SetFlip(ST7789V_Handle *hdl, bool x, bool y);
uint32_t ST7789V_GetID(ST7789V_Handle *hdl);
void ST7789V_EndianConvert(ST7789V_Handle *hdl, bool enable);
#endif
/** @} */