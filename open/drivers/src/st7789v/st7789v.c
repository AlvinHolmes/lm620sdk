#include <st7789v.h>
#include <drv_pin.h>
#include <drv_pcu.h>
#include "os.h"
#include <res_tree.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define CMD_RDDID                           (0x04)
#define CMD_CASET                           (0x2A)
#define CMD_RASET                           (0x2B)
#define CMD_RAMWR                           (0x2C)
#define CMD_MADCTL                          (0x36)

#define ST7789V_PORCH_MAX                   (0x0c)
#define ST7789V_PORCH_MIN                   (0x0c)
#define ST7789V_PORCH_NORMAL                (ST7789V_PORCH_MIN)

// 值越大，fps反而越小
#define ST7789V_FPS_MAX                     (0x0f)
#define ST7789V_FPS_MIN                     (0x1f)
#define ST7789V_FPS_NORMAL                  (ST7789V_FPS_MAX)

#define ST7789V_PIN_RES_BACKLIGHT(config)   ((config)->backLight->res)
#define ST7789V_PIN_RES_RESET(config)       ((config)->reset->res)
#define ST7789V_PIN_RES_CSX(config)         ((config)->csx->res)
#define ST7789V_PIN_RES_SCL(config)         ((config)->scl->res)
#define ST7789V_PIN_RES_SDO(config)         ((config)->sdo->res)
#define ST7789V_PIN_RES_SDI(config)         ((config)->sdi->res)
#define ST7789V_PIN_RES_DCX(config)         ((config)->dcx->res)

#define ST7789V_PIN_MUX_BACKLIGHT(config)   ((config)->backLight->gpioMux)
#define ST7789V_PIN_MUX_RESET(config)       ((config)->reset->gpioMux)
#define ST7789V_PIN_MUX_CSX(config)         ((config)->csx->specMux)
#define ST7789V_PIN_MUX_SCL(config)         ((config)->scl->specMux)
#define ST7789V_PIN_MUX_SDO(config)         ((config)->sdo->specMux)
#define ST7789V_PIN_MUX_SDI(config)         ((config)->sdi->specMux)
#define ST7789V_PIN_MUX_DCX(config)         ((config)->dcx->specMux)

#define ST7789V_RGB_MODE                    (1)  // 1:RGB, 0:BGR
#define ST7789V_MSB_MODE                    (1)  // 1:MSB, 0:LSB

#if ST7789V_RGB_MODE
#define ST7789V_MADCTL_DEF                  (0x00)
#else
#define ST7789V_MADCTL_DEF                  (0x08)
#endif

#define ST7789V_BACKLIGHT_SUPPORT_PWM       (1)
#define ST7789V_BACKLIGHT_USE_SOFT_PWM      (1)
#define ST7789V_TE_ENABLE                   (1)

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
static ST7789V_PinConfig g_st7782vPinConfig = {0};
static ST7789V_PinConfig *g_st7782vPinCfg = &g_st7782vPinConfig;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
static void ST7789V_PorchConfig(void *userData, uint8_t porch)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)userData;
    LCD_Handle *hdl = &handle->lcd;
    ALIGN(OS_CACHE_LINE_SZ) uint8_t regVal[32 * 4];

    regVal[0]=porch;
    regVal[1]=porch;
    regVal[2]=0x0;
    regVal[3]=0x33;
    regVal[4]=0x33;
    LCD_WriteParam(hdl,0xb2, regVal, 5);
}

static void ST7789V_FpsRateConfig(void *userData, uint8_t fps)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)userData;
    LCD_Handle *hdl = &handle->lcd;
    ALIGN(OS_CACHE_LINE_SZ) uint8_t regVal[32 * 4];

    regVal[0]=fps;
    LCD_WriteParam(hdl,0xc6, regVal, 1);
}

static void ST7789V_TE_Enable(ST7789V_Handle *handle, bool en)
{
    LCD_Handle *hdl = &handle->lcd;
    ALIGN(OS_CACHE_LINE_SZ) uint8_t regVal[32 * 4];

    if (en) {
        // regVal[ 0 ] = 0x00;
        // regVal[ 1 ] = 0x08;
        // LCD_WriteParam(hdl, 0x44,regVal, 2);

        regVal[0] = 0x00;
        LCD_WriteParam(hdl, 0x35, regVal, 1);
    }
    else {
        LCD_WriteParam(hdl, 0x34, NULL, 0);
    }
}

static void ST7789V_SendData(void *userData, uint8_t *pixel, uint32_t len)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)userData;
    int ret = LCD_SendData(&handle->lcd, CMD_RAMWR, pixel, len);
    OS_ASSERT(ret == 0);
}

#if ST7789V_BACKLIGHT_USE_SOFT_PWM

static void ST7789V_SoftPwmTimer(void* parameter)
{
    ST7789V_Handle *handle = (ST7789V_Handle *)parameter;
    ST7789V_PinConfig *config = handle->pin;

    if(handle->pwm.level) {
        GPIO_Write(ST7789V_PIN_RES_BACKLIGHT(config), 1);
        handle->pwm.level = 0;
        osTimerStart(handle->pwm.timer, osTickFromMs(handle->pwm.high));
    }
    else {
        GPIO_Write(ST7789V_PIN_RES_BACKLIGHT(config), 0);
        handle->pwm.level = 1;
        osTimerStart(handle->pwm.timer, osTickFromMs(handle->pwm.low));
    }
}

#endif

static int32_t ST7789V_BK_En(ST7789V_Handle *handle, uint8_t level)
{
    ST7789V_PinConfig *config = handle->pin;

    //打开背光
    if(ST7789V_PIN_RES_BACKLIGHT(config) == NULL) {
        return -1;
    }

#if ST7789V_BACKLIGHT_USE_SOFT_PWM
    osTimerStop(handle->pwm.timer);
#endif

    if(level == 0) {
        GPIO_Write(ST7789V_PIN_RES_BACKLIGHT(config), 0);
    }
    else if(level == 100) {
        GPIO_Write(ST7789V_PIN_RES_BACKLIGHT(config), 1);
    }
    else {
        GPIO_Write(ST7789V_PIN_RES_BACKLIGHT(config), 1);
#if ST7789V_BACKLIGHT_USE_SOFT_PWM
        uint8_t val = level / 10;
        if(val == 0) {
            val = 1;
        }
        else if (val == 10) {
            val = 9;
        }
        handle->pwm.level = 0;
        handle->pwm.high = val;
        handle->pwm.low = handle->pwm.diff - handle->pwm.high;
        osTimerStart(handle->pwm.timer, osTickFromMs(handle->pwm.high));
#endif
    }

    return 0;
}

static void ST7789V_Reset(ST7789V_PinConfig *config)
{
    if(ST7789V_PIN_RES_RESET(config) == NULL) {
        return;
    }

    PIN_SetMux(ST7789V_PIN_RES_RESET(config), ST7789V_PIN_MUX_RESET(config));
    GPIO_SetDir(ST7789V_PIN_RES_RESET(config), GPIO_OUTPUT);
    GPIO_Write(ST7789V_PIN_RES_RESET(config), GPIO_HIGH);
    osThreadMsSleep(50);
    GPIO_Write(ST7789V_PIN_RES_RESET(config), GPIO_LOW);
    osThreadMsSleep(100);
    GPIO_Write(ST7789V_PIN_RES_RESET(config), GPIO_HIGH);
    osThreadMsSleep(150);
}

static void ST7789V_PinMuxInit(ST7789V_PinConfig *config)
{
    if(ST7789V_PIN_RES_CSX(config)) {
        PIN_SetMux(ST7789V_PIN_RES_CSX(config), ST7789V_PIN_MUX_CSX(config));
    }

    if(ST7789V_PIN_RES_SCL(config)) {
        PIN_SetMux(ST7789V_PIN_RES_SCL(config), ST7789V_PIN_MUX_SCL(config));
    }
    
    if(ST7789V_PIN_RES_SDO(config)) {
        PIN_SetMux(ST7789V_PIN_RES_SDO(config), ST7789V_PIN_MUX_SDO(config));
    }

    if(ST7789V_PIN_RES_SDI(config)) {
        PIN_SetMux(ST7789V_PIN_RES_SDI(config), ST7789V_PIN_MUX_SDI(config));
    }

    if(ST7789V_PIN_RES_DCX(config)) {
        PIN_SetMux(ST7789V_PIN_RES_DCX(config), ST7789V_PIN_MUX_DCX(config));
    }

    if(ST7789V_PIN_RES_BACKLIGHT(config)) {
        GPIO_Write(ST7789V_PIN_RES_BACKLIGHT(config), 0);
        PIN_SetMux(ST7789V_PIN_RES_BACKLIGHT(config), ST7789V_PIN_MUX_BACKLIGHT(config));
        GPIO_SetDir(ST7789V_PIN_RES_BACKLIGHT(config), GPIO_OUTPUT);
    }
}

static void ST7789V_RegInit(ST7789V_Handle *handle)
{
    LCD_Handle *hdl = &handle->lcd;

    ALIGN(OS_CACHE_LINE_SZ) uint8_t regVal[32 * 4];

    LCD_WriteParam(hdl, 0x11,NULL, 0);
    osThreadMsSleep(120);

    regVal[ 0 ] = handle->madctl;
    LCD_WriteParam(hdl, CMD_MADCTL, regVal, 1);

    regVal[0]=0x05;
    LCD_WriteParam(hdl,0x3a, regVal, 1); 
    
    regVal[0]=0x00;

#if ST7789V_MSB_MODE
    regVal[1]=0xe0;
#else
    regVal[1]=0xe8;
#endif
    LCD_WriteParam(hdl,0xb0, regVal, 2);

    regVal[0]=ST7789V_PORCH_NORMAL;
    regVal[1]=ST7789V_PORCH_NORMAL;
    regVal[2]=0x0;
    regVal[3]=0x33;
    regVal[4]=0x33;
    LCD_WriteParam(hdl,0xb2, regVal, 5);

    regVal[0]=0x75;
    LCD_WriteParam(hdl,0xb7, regVal, 1);

    regVal[0]=0x13;
    LCD_WriteParam(hdl,0xbb, regVal, 1);

    regVal[0]=0x2c;
    LCD_WriteParam(hdl,0xc0, regVal, 1);

    regVal[0]=0x01;
    LCD_WriteParam(hdl,0xc2, regVal, 1);

    regVal[0]=0x11;
    LCD_WriteParam(hdl,0xc3, regVal, 1);

    regVal[0]=0x2c;
    LCD_WriteParam(hdl,0xc5, regVal, 1);

    regVal[0]=0x20;
    LCD_WriteParam(hdl,0xc4, regVal, 1);

    regVal[0]=ST7789V_FPS_NORMAL;
    LCD_WriteParam(hdl,0xc6, regVal, 1);

    regVal[0]=0xa4;
    regVal[1]=0xa1;
    LCD_WriteParam(hdl,0xd0, regVal, 2);

    regVal[0]=0xD0;
    regVal[1]=0x1A;
    regVal[2]=0x1E;
    regVal[3]=0x0A;
    regVal[4]=0x0A;
    regVal[5]=0x27;
    regVal[6]=0x3B;
    regVal[7]=0x44;
    regVal[8]=0x4A;
    regVal[9]=0x2B;
    regVal[10]=0x16;
    regVal[11]=0x15;
    regVal[12]=0x1A;
    regVal[13]=0x1E;
    //增加两个无效字节构成4字节对齐，否则颜色显示异常
    regVal[14]=0xFF;
    regVal[15]=0xFF;
    LCD_WriteParam(hdl,0xe0, regVal, 16);


    regVal[0]=0xD0;
    regVal[1]=0x1A;
    regVal[2]=0x1E;
    regVal[3]=0x0A;
    regVal[4]=0x0A;
    regVal[5]=0x27;
    regVal[6]=0x3A;
    regVal[7]=0x43;
    regVal[8]=0x49;
    regVal[9]=0x2B;
    regVal[10]=0x16;
    regVal[11]=0x15;
    regVal[12]=0x1A;
    regVal[13]=0x1D;
    //增加两个无效字节构成4字节对齐，否则颜色显示异常
    regVal[14]=0xFF;
    regVal[15]=0xFF;
    LCD_WriteParam(hdl,0xe1, regVal, 16);

    LCD_WriteParam(hdl,0x29, NULL, 0);
    osThreadMsSleep(200);
}

static void ST7789V_SetArea(LCD_Handle *hdl, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2) 
{
    int ret;
    uint16_t x[2];
    uint16_t y[2];

    x[0] = (x1 >> 8) | (x1 << 8);
    x[1] = (x2 >> 8) | (x2 << 8);
    
    ret = LCD_WriteParam(hdl, CMD_CASET, (uint8_t*)x, sizeof(x));
    OS_ASSERT(ret == 0);

    y[0] = (y1 >> 8) | (y1 << 8);
    y[1] = (y2 >> 8) | (y2 << 8);
    ret = LCD_WriteParam(hdl, CMD_RASET, (uint8_t*)y, sizeof(y));
    OS_ASSERT(ret == 0);
}

static void ST7789V_TeInterruptServer(void *param)
{
    ST7789V_Handle *hdl = (ST7789V_Handle *)param;

    if(hdl->cbEvent) {
        hdl->cbEvent(hdl->userData);
    }
}

static void ST7789V_InterruptServer(void *param)
{
    ST7789V_Handle *hdl = (ST7789V_Handle *)param;

    if(hdl->fmark_enable) {
        LCD_TeRelease(&hdl->te);
    }
    else {
        if(hdl->cbEvent) {
            hdl->cbEvent(hdl->userData);
        }
    }
}

static void ST7789V_DrvInit(ST7789V_Handle *handle)
{
    LCD_Handle *hdl = &handle->lcd;
    
    ST7789V_Reset(g_st7782vPinCfg);
    
    LCD_Initialize(hdl);

    ST7789V_PinMuxInit(g_st7782vPinCfg);

    ST7789V_BackLight(handle, 100);

    LCD_PowerControl(hdl, DRV_POWER_FULL);

    if(ST7789V_PIN_RES_DCX(g_st7782vPinCfg) == NULL) {
        LCD_Control(hdl, LCD_SPI_DCX, 0);
        LCD_Control(hdl, LCD_READ_DMY_CYC, 9);
    }
    else {
        LCD_Control(hdl, LCD_SPI_DCX, 1);
        LCD_Control(hdl, LCD_READ_DMY_CYC, 8);
    }

    if(ST7789V_PIN_RES_SDO(g_st7782vPinCfg) && ST7789V_PIN_RES_SDI(g_st7782vPinCfg)) {
        LCD_Control(hdl, LCD_SPI_BIDIR, 0);
    }
    else {
        LCD_Control(hdl, LCD_SPI_BIDIR, 1);
    }
    LCD_Control(hdl, LCD_SAMP_SEL, 1);
    // LCD_Control(hdl, LCD_RGB565_SWAP, 1);

    ST7789V_RegInit(handle);

    if(handle->fmark_enable) {
    #ifdef LCD_TE_AUTO_RATE
        handle->te.fpsMinVal = ST7789V_FPS_MIN;
        handle->te.fpsMaxVal = ST7789V_FPS_MAX;
        handle->te.porchMinVal = ST7789V_PORCH_MIN;
        handle->te.porchMaxVal = ST7789V_PORCH_MAX;
        handle->te.fpsFunc = ST7789V_FpsRateConfig;
        handle->te.porchFunc = ST7789V_PorchConfig;
        // handle->te.teEnable = ST7789V_TE_Enable;
    #endif
        ST7789V_TE_Enable(handle, true);
        LCD_TeInit(&handle->te, ST7789V_TeInterruptServer, ST7789V_SendData, handle);
    }
}

void ST7789V_PinInit(PIN_MultiMux_t *bk, PIN_MultiMux_t *reset, PIN_MultiMux_t *csx, PIN_MultiMux_t *scl, PIN_MultiMux_t *sdi, PIN_MultiMux_t *sdo, PIN_MultiMux_t *dcx)
{
    g_st7782vPinCfg->backLight = bk;
    g_st7782vPinCfg->reset = reset;
    g_st7782vPinCfg->csx = csx;
    g_st7782vPinCfg->scl = scl;
    g_st7782vPinCfg->sdi = sdi;
    g_st7782vPinCfg->sdo = sdo;
    g_st7782vPinCfg->dcx = dcx;
}

void ST7789V_PowerControl(ST7789V_Handle *hdl, bool en)
{
    if(en) {
        EXTLDO_Acquire(EXT_LDO_1V8);
        EXTLDO_Acquire(EXT_LDO_2V8);
    }
    else {
        EXTLDO_Release(EXT_LDO_1V8);
        EXTLDO_Release(EXT_LDO_2V8);
    }
}

int32_t ST7789V_BackLight(ST7789V_Handle *hdl, uint8_t level)
{
#if ST7789V_BACKLIGHT_SUPPORT_PWM
    return ST7789V_BK_En(hdl, level);
#else
    if(level) {
        return ST7789V_BK_En(hdl, 100);
    }
    else {
        return ST7789V_BK_En(hdl, 0);
    }
#endif
}

void ST7789V_Init(ST7789V_Handle *hdl)
{
    OS_ASSERT(g_st7782vPinCfg);

    hdl->lcd.SendCallback = ST7789V_InterruptServer;
    hdl->lcd.userData = hdl;

    hdl->madctl = ST7789V_MADCTL_DEF;

    hdl->fmark_enable = ST7789V_TE_ENABLE;

    hdl->pin = g_st7782vPinCfg;

#if ST7789V_BACKLIGHT_USE_SOFT_PWM
    hdl->pwm.freq = 100;
    hdl->pwm.diff = OS_TICK_PER_SECOND / hdl->pwm.freq;
    hdl->pwm.high = hdl->pwm.diff;
    hdl->pwm.low = hdl->pwm.diff - hdl->pwm.high;
    hdl->pwm.timer = osTimerNew(ST7789V_SoftPwmTimer, osTimerOnce, hdl, NULL);
#endif

    ST7789V_DrvInit(hdl);
}

void ST7789V_DeInit(ST7789V_Handle *hdl)
{
    hdl->madctl_last = hdl->madctl;

    if(hdl->fmark_enable) {
        LCD_TeDeInit(&hdl->te);
    }

    ST7789V_BackLight(hdl, 0);

#if ST7789V_BACKLIGHT_USE_SOFT_PWM
    if(hdl->pwm.timer) {
        osTimerDelete(hdl->pwm.timer);
    }
#endif

    LCD_PowerControl(&hdl->lcd, DRV_POWER_OFF);

    LCD_Uninitialize(&hdl->lcd);
}

void ST7789V_Flush(ST7789V_Handle *hdl, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint8_t *pixel)
{
    ST7789V_SetArea(&hdl->lcd, x1, x2, y1, y2);
    
    if(hdl->fmark_enable) {
    #ifdef LCD_TE_ASYNC
        LCD_TeAcquireAsync(&hdl->te, pixel, (x2 - x1 + 1) * (y2 - y1 + 1) * 2);
        return;
    #else
        LCD_TeAcquire(&hdl->te);
    #endif
    }
    
    // LCD_WriteParam(&hdl->lcd, CMD_MADCTL, &hdl->madctl, 1);
    int ret = LCD_SendData(&hdl->lcd, CMD_RAMWR, pixel, (x2 - x1 + 1) * (y2 - y1 + 1) * 2);
    OS_ASSERT(ret == 0);
}

void ST7789V_SetWidth(ST7789V_Handle *hdl, uint16_t width, uint16_t height)
{
    ST7789V_SetArea(&hdl->lcd, 0, width - 1, 0, height - 1);
}

void ST7789V_Sleep(ST7789V_Handle *hdl)
{
    // LCD_WriteParam(&hdl->lcd, 0x10,NULL, 0);

    ST7789V_DeInit(hdl);

    ST7789V_PowerControl(hdl, false);

    hdl->madctl_last = hdl->madctl;

#ifdef LCD_TE_AUTO_RATE
    hdl->porch_last = hdl->te.porchMaxVal;
#endif
}

void ST7789V_Wakeup(ST7789V_Handle *hdl)
{
    // LCD_WriteParam(&hdl->lcd, 0x11,NULL, 0);
    // osThreadMsSleep(120);

    ST7789V_PowerControl(hdl, true);
    
    ST7789V_Init(hdl);

    if(hdl->madctl != hdl->madctl_last) {
        hdl->madctl = hdl->madctl_last;
        LCD_WriteParam(&hdl->lcd, CMD_MADCTL, &hdl->madctl_last, 1);
    }

    if(hdl->fmark_enable) {
    #ifdef LCD_TE_AUTO_RATE
        hdl->te.porchMaxVal = hdl->porch_last;
    #endif
    }

    LCD_Control(&hdl->lcd, LCD_RGB565_SWAP, hdl->endian_enable);
}

void ST7789V_SetScreenMode(ST7789V_Handle *hdl, ST7789V_SCREEN_MODE mode)
{    
    switch (mode) {
        case ST7789V_SCREEN_PORTRAIT:
            hdl->madctl &= ~(0x1 << 5);
            break;

        case ST7789V_SCREEN_LANDSCAPE:
            hdl->madctl |= (0x1 << 5);
            break;
    }

    LCD_WriteParam(&hdl->lcd, CMD_MADCTL, &hdl->madctl, 1);
}

void ST7789V_SetFlip(ST7789V_Handle *hdl, bool x, bool y)
{
    if(x) {
        hdl->madctl |= (0x1 << 6);
    }
    else {
        hdl->madctl &= ~(0x1 << 6);
    }

    if(y) {
        hdl->madctl |= (0x1 << 7);
    }
    else {
        hdl->madctl &= ~(0x1 << 7);
    }

    LCD_WriteParam(&hdl->lcd, CMD_MADCTL, &hdl->madctl, 1);
}

uint32_t ST7789V_GetID(ST7789V_Handle *hdl)
{
    uint32_t id = 0;
    LCD_ReadParam(&hdl->lcd, CMD_RDDID, (uint8_t *)&id, 3);
    return id;
}

void ST7789V_EndianConvert(ST7789V_Handle *hdl, bool enable)
{
    hdl->endian_enable = enable ? 1 : 0;
    LCD_Control(&hdl->lcd, LCD_RGB565_SWAP, hdl->endian_enable);
}

bool lcdSt7789Reg(LcdOps_t *pLcdOpsCB)
{
       //add check id after,if id ok,mount function
    pLcdOpsCB->lcdInit = ST7789V_Init;
    pLcdOpsCB->lcdDeInit = ST7789V_DeInit;
    pLcdOpsCB->lcdSetFlip = ST7789V_SetFlip;
    pLcdOpsCB->lcdSetWidth = ST7789V_SetWidth;
    pLcdOpsCB->lcdSetScreenMode = ST7789V_SetScreenMode;
    pLcdOpsCB->lcdFlush = ST7789V_Flush;
    pLcdOpsCB->lcdSleep = ST7789V_Sleep;
    pLcdOpsCB->lcdWakeup = ST7789V_Wakeup;
    pLcdOpsCB->lcdEndianConvert = ST7789V_EndianConvert;
    return true;
}

