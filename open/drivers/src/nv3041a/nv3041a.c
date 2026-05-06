#include <nv3041a.h>
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

#define NV3041A_PORCH_MAX                   (0x0c)
#define NV3041A_PORCH_MIN                   (0x0c)
#define NV3041A_PORCH_NORMAL                (NV3041A_PORCH_MIN)

// 值越大，fps反而越小
#define NV3041A_FPS_MAX                     (0x0f)
#define NV3041A_FPS_MIN                     (0x1f)
#define NV3041A_FPS_NORMAL                  (NV3041A_FPS_MAX)

#define NV3041A_PIN_RES_BACKLIGHT(config)   ((config)->backLight->res)
#define NV3041A_PIN_RES_RESET(config)       ((config)->reset->res)
#define NV3041A_PIN_RES_CSX(config)         ((config)->csx->res)
#define NV3041A_PIN_RES_SCL(config)         ((config)->scl->res)
#define NV3041A_PIN_RES_SDO(config)         ((config)->sdo->res)
#define NV3041A_PIN_RES_SDI(config)         ((config)->sdi->res)
#define NV3041A_PIN_RES_DCX(config)         ((config)->dcx->res)

#define NV3041A_PIN_MUX_BACKLIGHT(config)   ((config)->backLight->gpioMux)
#define NV3041A_PIN_MUX_RESET(config)       ((config)->reset->gpioMux)
#define NV3041A_PIN_MUX_CSX(config)         ((config)->csx->specMux)
#define NV3041A_PIN_MUX_SCL(config)         ((config)->scl->specMux)
#define NV3041A_PIN_MUX_SDO(config)         ((config)->sdo->specMux)
#define NV3041A_PIN_MUX_SDI(config)         ((config)->sdi->specMux)
#define NV3041A_PIN_MUX_DCX(config)         ((config)->dcx->specMux)

#define NV3041A_RGB_MODE                    (1)  // 1:RGB, 0:BGR
#define NV3041A_MSB_MODE                    (1)  // 1:MSB, 0:LSB

#if NV3041A_RGB_MODE
#define NV3041A_MADCTL_DEF                  (0x00)
#else
#define NV3041A_MADCTL_DEF                  (0x08)
#endif

#define NV3041A_BACKLIGHT_SUPPORT_PWM       (1)
#define NV3041A_BACKLIGHT_USE_SOFT_PWM      (0)
#define NV3041A_TE_ENABLE                   (0)

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
static NV3041A_PinConfig g_st7782vPinConfig = {0};
static NV3041A_PinConfig *g_st7782vPinCfg = &g_st7782vPinConfig;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
static void NV3041A_PorchConfig(void *userData, uint8_t porch)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)userData;
    LCD_Handle *hdl = &handle->lcd;
    ALIGN(OS_CACHE_LINE_SZ) uint8_t regVal[32 * 4];

    regVal[0]=porch;
    regVal[1]=porch;
    regVal[2]=0x0;
    regVal[3]=0x33;
    regVal[4]=0x33;
    LCD_WriteParam(hdl,0xb2, regVal, 5);
}

static void NV3041A_FpsRateConfig(void *userData, uint8_t fps)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)userData;
    LCD_Handle *hdl = &handle->lcd;
    ALIGN(OS_CACHE_LINE_SZ) uint8_t regVal[32 * 4];

    regVal[0]=fps;
    LCD_WriteParam(hdl,0xc6, regVal, 1);
}

static void NV3041A_TE_Enable(NV3041A_Handle *handle, bool en)
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

static void NV3041A_SendData(void *userData, uint8_t *pixel, uint32_t len)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)userData;
    int ret = LCD_SendData(&handle->lcd, CMD_RAMWR, pixel, len);
    OS_ASSERT(ret == 0);
}

#if NV3041A_BACKLIGHT_USE_SOFT_PWM

static void NV3041A_SoftPwmTimer(void* parameter)
{
    NV3041A_Handle *handle = (NV3041A_Handle *)parameter;
    NV3041A_PinConfig *config = handle->pin;

    if(handle->pwm.level) {
        GPIO_Write(NV3041A_PIN_RES_BACKLIGHT(config), 1);
        handle->pwm.level = 0;
        osTimerStart(handle->pwm.timer, osTickFromMs(handle->pwm.high));
    }
    else {
        GPIO_Write(NV3041A_PIN_RES_BACKLIGHT(config), 0);
        handle->pwm.level = 1;
        osTimerStart(handle->pwm.timer, osTickFromMs(handle->pwm.low));
    }
}

#endif

static int32_t NV3041A_BK_En(NV3041A_Handle *handle, uint8_t level)
{
    NV3041A_PinConfig *config = handle->pin;

    //打开背光
    if(NV3041A_PIN_RES_BACKLIGHT(config) == NULL) {
        return -1;
    }

#if NV3041A_BACKLIGHT_USE_SOFT_PWM
    osTimerStop(handle->pwm.timer);
#endif

    if(level == 0) {
        GPIO_Write(NV3041A_PIN_RES_BACKLIGHT(config), 0);
    }
    else if(level == 100) {
        GPIO_Write(NV3041A_PIN_RES_BACKLIGHT(config), 1);
    }
    else {
        GPIO_Write(NV3041A_PIN_RES_BACKLIGHT(config), 1);
#if NV3041A_BACKLIGHT_USE_SOFT_PWM
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

static void NV3041A_Reset(NV3041A_PinConfig *config)
{
    if(NV3041A_PIN_RES_RESET(config) == NULL) {
        return;
    }

    PIN_SetMux(NV3041A_PIN_RES_RESET(config), NV3041A_PIN_MUX_RESET(config));
    GPIO_SetDir(NV3041A_PIN_RES_RESET(config), GPIO_OUTPUT);
    GPIO_Write(NV3041A_PIN_RES_RESET(config), GPIO_HIGH);
    osThreadMsSleep(50);
    GPIO_Write(NV3041A_PIN_RES_RESET(config), GPIO_LOW);
    osThreadMsSleep(100);
    GPIO_Write(NV3041A_PIN_RES_RESET(config), GPIO_HIGH);
    osThreadMsSleep(150);
}

static void NV3041A_PinMuxInit(NV3041A_PinConfig *config)
{
    if(NV3041A_PIN_RES_CSX(config)) {
        PIN_SetMux(NV3041A_PIN_RES_CSX(config), NV3041A_PIN_MUX_CSX(config));
    }

    if(NV3041A_PIN_RES_SCL(config)) {
        PIN_SetMux(NV3041A_PIN_RES_SCL(config), NV3041A_PIN_MUX_SCL(config));
    }
    
    if(NV3041A_PIN_RES_SDO(config)) {
        PIN_SetMux(NV3041A_PIN_RES_SDO(config), NV3041A_PIN_MUX_SDO(config));
    }

    if(NV3041A_PIN_RES_SDI(config)) {
        PIN_SetMux(NV3041A_PIN_RES_SDI(config), NV3041A_PIN_MUX_SDI(config));
    }

    if(NV3041A_PIN_RES_DCX(config)) {
        PIN_SetMux(NV3041A_PIN_RES_DCX(config), NV3041A_PIN_MUX_DCX(config));
    }

    if(NV3041A_PIN_RES_BACKLIGHT(config)) {
        GPIO_Write(NV3041A_PIN_RES_BACKLIGHT(config), 0);
        PIN_SetMux(NV3041A_PIN_RES_BACKLIGHT(config), NV3041A_PIN_MUX_BACKLIGHT(config));
        GPIO_SetDir(NV3041A_PIN_RES_BACKLIGHT(config), GPIO_OUTPUT);
    }
}

static void NV3041A_RegInit(NV3041A_Handle *handle)
{
    LCD_Handle *hdl = &handle->lcd;
    
    //---------Start Initial Code ------//
    uint8_t data;
    
    data = 0xa5;
    LCD_WriteParam(hdl, 0xff, &data, 1);
    
    data = 0x10;
    LCD_WriteParam(hdl, 0xE7, &data, 1);  //TE_output_en
    
    data = 0x00;
    LCD_WriteParam(hdl, 0x35, &data, 1);  //TE_ interface_en
    
    data = 0x01; //00---666//01--565
    LCD_WriteParam(hdl, 0x3A, &data, 1);
    
    data = 0x01; //01:IPS/00:TN
    LCD_WriteParam(hdl, 0x40, &data, 1); 
    
    data = 0x01; //01--8bit//03--16bit
    LCD_WriteParam(hdl, 0x41, &data, 1); 
    
    data = 0x01;
    LCD_WriteParam(hdl, 0x55, &data, 1); 
    
    data = 0x15; //21
    LCD_WriteParam(hdl, 0x44, &data, 1);  //VBP
    
    data = 0x15; //21
    LCD_WriteParam(hdl, 0x45, &data, 1);  //VFP
    
    data = 0x03;
    LCD_WriteParam(hdl, 0x7d, &data, 1);//vdds_trim[2:0]
    
    data = 0xbb;//0xbb 88 a2
    LCD_WriteParam(hdl, 0xc1, &data, 1);//avdd_clp_en avdd_clp[1:0] avcl_clp_en avcl_clp[1:0]
    
    data = 0x13;//05
    LCD_WriteParam(hdl, 0xc2, &data, 1);//vgl_clp_en vgl_clp[2:0] 
    
    data = 0x10;//10
    LCD_WriteParam(hdl, 0xc3, &data, 1);//vgl_clp_en vgl_clp[2:0] 
    
    data = 0x3e; // 35
    LCD_WriteParam(hdl, 0xc6, &data, 1);//avdd_ratio_sel avcl_ratio_sel vgh_ratio_sel[1:0] vgl_ratio_sel[1:0] 
    
    data = 0x25; //2e
    LCD_WriteParam(hdl, 0xc7, &data, 1);//mv_clk_sel[1:0] avdd_clk_sel[1:0] avcl_clk_sel[1:0] 
    
    data = 0x11;
    LCD_WriteParam(hdl, 0xc8, &data, 1);// VGL_CLK_sel
    
    data = 0x66; //61
    LCD_WriteParam(hdl, 0x7a, &data, 1);// user_vgsp
    
    data = 0x49;  //3F
    LCD_WriteParam(hdl, 0x6f, &data, 1);// user_gvdd
    
    data = 0x57; //60
    LCD_WriteParam(hdl, 0x78, &data, 1);// user_gvcl
    
    data = 0x08;
    LCD_WriteParam(hdl, 0x73, &data, 1);//osc
    
    data = 0x13;
    LCD_WriteParam(hdl, 0x74, &data, 1);
    
    data = 0x00;
    LCD_WriteParam(hdl, 0xc9, &data, 1);
    
    data = 0x33;
    LCD_WriteParam(hdl, 0x67, &data, 1);  //
    
    //gate_ed
    data = 0x4b; //0a
    LCD_WriteParam(hdl, 0x51, &data, 1);//gate_st_o[7:0]
    
    data = 0x7c; //76
    LCD_WriteParam(hdl, 0x52, &data, 1);//gate_ed_o[7:0]
    
    data = 0x45;  //0a
    LCD_WriteParam(hdl, 0x53, &data, 1);//gate_st_e[7:0]
    
    data = 0x77;  //76
    LCD_WriteParam(hdl, 0x54, &data, 1);//gate_ed_e[7:0]
    
    ////source
    data = 0x0a;
    LCD_WriteParam(hdl, 0x46, &data, 1);//fsm_hbp_o[5:0]
    
    data = 0x2a;
    LCD_WriteParam(hdl, 0x47, &data, 1);//fsm_hfp_o[5:0]
    
    data = 0x0a;
    LCD_WriteParam(hdl, 0x48, &data, 1);//fsm_hbp_e[5:0]
    
    data = 0x1a;
    LCD_WriteParam(hdl, 0x49, &data, 1);//fsm_hfp_e[5:0]
    
    data = 0x43;
    LCD_WriteParam(hdl, 0x56, &data, 1);//src_ld_wd[1:0] src_ld_st[5:0]
    
    data = 0x42;
    LCD_WriteParam(hdl, 0x57, &data, 1);//pn_cs_en src_cs_st[5:0]
    
    data = 0x3c;
    LCD_WriteParam(hdl, 0x58, &data, 1);//src_cs_p_wd[6:0]
    
    data = 0x64;
    LCD_WriteParam(hdl, 0x59, &data, 1);//src_cs_n_wd[6:0]
    
    data = 0x41;
    LCD_WriteParam(hdl, 0x5a, &data, 1);//src_pchg_st_o[6:0]
    
    data = 0x3c;
    LCD_WriteParam(hdl, 0x5b, &data, 1);//src_pchg_wd_o[6:0]
    
    data = 0x02; 
    LCD_WriteParam(hdl, 0x5c, &data, 1);//src_pchg_st_e[6:0]
    
    data = 0x3c; 
    LCD_WriteParam(hdl, 0x5d, &data, 1);//src_pchg_wd_e[6:0]
    
    data = 0x1f;
    LCD_WriteParam(hdl, 0x5e, &data, 1);//src_pol_sw[7:0]
    
    data = 0x80;
    LCD_WriteParam(hdl, 0x60, &data, 1);//src_op_st_o[7:0]
    
    data = 0x3f;
    LCD_WriteParam(hdl, 0x61, &data, 1);//src_op_st_e[7:0]
    
    data = 0x21;
    LCD_WriteParam(hdl, 0x62, &data, 1);//src_op_ed_o[9:8] src_op_ed_e[9:8]
    
    data = 0x07;
    LCD_WriteParam(hdl, 0x63, &data, 1);//src_op_ed_o[7:0]
    
    data = 0xe0;
    LCD_WriteParam(hdl, 0x64, &data, 1);//src_op_ed_e[7:0]
    
    data = 0x01;//02--A1,01--A2
    LCD_WriteParam(hdl, 0x65, &data, 1);//chopper
    
    data = 0x14;
    LCD_WriteParam(hdl, 0x6e, &data, 1);//lvd
    
    data = 0x20;
    LCD_WriteParam(hdl, 0xca, &data, 1);  //avdd_mux_st_o[7:0]
    
    data = 0x52; 
    LCD_WriteParam(hdl, 0xcb, &data, 1);  //avdd_mux_ed_o[7:0]
    
    data = 0x10;
    LCD_WriteParam(hdl, 0xcc, &data, 1);  //avdd_mux_st_e[7:0]
    
    data = 0x42;
    LCD_WriteParam(hdl, 0xcD, &data, 1); //avdd_mux_ed_e[7:0]
    
    data = 0x20;
    LCD_WriteParam(hdl, 0xD0, &data, 1); //avcl_mux_st_o[7:0]
    
    data = 0x52;
    LCD_WriteParam(hdl, 0xD1, &data, 1); //avcl_mux_ed_o[7:0]
    
    data = 0x10;
    LCD_WriteParam(hdl, 0xD2, &data, 1); //avcl_mux_st_e[7:0]
    
    data = 0x42;
    LCD_WriteParam(hdl, 0xD3, &data, 1); //avcl_mux_ed_e[7:0]
    
    data = 0x0a;
    LCD_WriteParam(hdl, 0xD4, &data, 1); //vgh_mux_st[7:0]
    
    data = 0x32;
    LCD_WriteParam(hdl, 0xD5, &data, 1); //vgh_mux_ed[7:0]
    
    data = 0x06;
    LCD_WriteParam(hdl, 0xe5, &data, 1);  //DVDD_TRIM
    
    data = 0x00;
    LCD_WriteParam(hdl, 0xe6, &data, 1);  //ESD_CTRL
    
    ///test mode
    data = 0x06;  //
    LCD_WriteParam(hdl, 0xf8, &data, 1);  //
    
    data = 0x00;
    LCD_WriteParam(hdl, 0xf9, &data, 1);  // 
    
    //BOE IPS GAMMA 0508
    data = 0x00;
    LCD_WriteParam(hdl, 0x80, &data, 1);  //gam_vrp0 
    
    data = 0x00;
    LCD_WriteParam(hdl, 0xA0, &data, 1); //gam_VRN0 
    
    data = 0x05;
    LCD_WriteParam(hdl, 0x81, &data, 1);  //gam_vrp1 
    
    data = 0x03;
    LCD_WriteParam(hdl, 0xA1, &data, 1); //gam_VRN1 
    
    data = 0x02;
    LCD_WriteParam(hdl, 0x82, &data, 1);  //gam_vrp2 
    
    data = 0x02;
    LCD_WriteParam(hdl, 0xA2, &data, 1); //gam_VRN2 
    
    data = 0x2d;  //
    LCD_WriteParam(hdl, 0x86, &data, 1);  //gam_prp0 
    
    data = 0x1a;  //
    LCD_WriteParam(hdl, 0xA6, &data, 1); //gam_PRN0 
    
    data = 0x40; //
    LCD_WriteParam(hdl, 0x87, &data, 1);  //gam_prp1 
    
    data = 0x3f; //
    LCD_WriteParam(hdl, 0xA7, &data, 1); //gam_PRN1 
    
    data = 0x38;
    LCD_WriteParam(hdl, 0x83, &data, 1);  //gam_vrp3 
    
    data = 0x37;
    LCD_WriteParam(hdl, 0xA3, &data, 1); //gam_VRN3 
    
    data = 0x37;
    LCD_WriteParam(hdl, 0x84, &data, 1);  //gam_vrp4 
    
    data = 0x36;
    LCD_WriteParam(hdl, 0xA4, &data, 1); //gam_VRN4 
    
    data = 0x28;
    LCD_WriteParam(hdl, 0x85, &data, 1);  //gam_vrp5 
    
    data = 0x28;
    LCD_WriteParam(hdl, 0xA5, &data, 1); //gam_VRN5 
    
    data = 0x09; 
    LCD_WriteParam(hdl, 0x88, &data, 1);  //gam_pkp0 
    
    data = 0x05; 
    LCD_WriteParam(hdl, 0xA8, &data, 1); //gam_PKN0 
    
    data = 0x0f; //
    LCD_WriteParam(hdl, 0x89, &data, 1);  //gam_pkp1 
    
    data = 0x0c; //
    LCD_WriteParam(hdl, 0xA9, &data, 1); //gam_PKN1 
    
    data = 0x18;//
    LCD_WriteParam(hdl, 0x8a, &data, 1);  //gam_pkp2 
    
    data = 0x14;//
    LCD_WriteParam(hdl, 0xAa, &data, 1); //gam_PKN2 
    
    data = 0x12;
    LCD_WriteParam(hdl, 0x8b, &data, 1);  //gam_PKP3 
    
    data = 0x0e;
    LCD_WriteParam(hdl, 0xAb, &data, 1); //gam_PKN3 
    
    data = 0x15;
    LCD_WriteParam(hdl, 0x8c, &data, 1);  //gam_PKP4 
    
    data = 0x15;
    LCD_WriteParam(hdl, 0xAc, &data, 1); //gam_PKN4 
    
    data = 0x11;//
    LCD_WriteParam(hdl, 0x8d, &data, 1);  //gam_PKP5 
    
    data = 0x15;
    LCD_WriteParam(hdl, 0xAd, &data, 1); //gam_PKN5 
    
    data = 0x12;//
    LCD_WriteParam(hdl, 0x8e, &data, 1);  //gam_PKP6 
    
    data = 0x11;//
    LCD_WriteParam(hdl, 0xAe, &data, 1); //gam_PKN6 
    
    data = 0x19;//
    LCD_WriteParam(hdl, 0x8f, &data, 1);  //gam_PKP7 
    
    data = 0x0f;//
    LCD_WriteParam(hdl, 0xAf, &data, 1);  //gam_PKN7 
    
    data = 0x0a;
    LCD_WriteParam(hdl, 0x90, &data, 1);  //gam_PKP8 
    
    data = 0x01;
    LCD_WriteParam(hdl, 0xB0, &data, 1); //gam_PKN8 
    
    data = 0x11;
    LCD_WriteParam(hdl, 0x91, &data, 1);  //gam_PKP9 
    
    data = 0x0d;
    LCD_WriteParam(hdl, 0xB1, &data, 1); //gam_PKN9 
    
    data = 0x19;
    LCD_WriteParam(hdl, 0x92, &data, 1);  //gam_PKP10
    
    data = 0x12;
    LCD_WriteParam(hdl, 0xB2, &data, 1); //gam_PKN10
    
    data = 0x00;
    LCD_WriteParam(hdl, 0xff, &data, 1);
    
    LCD_WriteParam(hdl, 0x11, NULL, 0);
    osThreadMsSleep(120);
    
    LCD_WriteParam(hdl, 0x29, NULL, 0);
    osThreadMsSleep(20);
}

static void NV3041A_SetArea(LCD_Handle *hdl, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2) 
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

static void NV3041A_TeInterruptServer(void *param)
{
    NV3041A_Handle *hdl = (NV3041A_Handle *)param;

    if(hdl->cbEvent) {
        hdl->cbEvent(hdl->userData);
    }
}

static void NV3041A_InterruptServer(void *param)
{
    NV3041A_Handle *hdl = (NV3041A_Handle *)param;

    if(hdl->fmark_enable) {
        LCD_TeRelease(&hdl->te);
    }
    else {
        if(hdl->cbEvent) {
            hdl->cbEvent(hdl->userData);
        }
    }
}

static void NV3041A_DrvInit(NV3041A_Handle *handle)
{
    LCD_Handle *hdl = &handle->lcd;
    
    NV3041A_Reset(g_st7782vPinCfg);
    
    LCD_Initialize(hdl);

    NV3041A_PinMuxInit(g_st7782vPinCfg);

    NV3041A_BackLight(handle, 100);

    LCD_PowerControl(hdl, DRV_POWER_FULL);

    if(NV3041A_PIN_RES_DCX(g_st7782vPinCfg) == NULL) {
        LCD_Control(hdl, LCD_SPI_DCX, 0);
        LCD_Control(hdl, LCD_READ_DMY_CYC, 9);
    }
    else {
        LCD_Control(hdl, LCD_SPI_DCX, 1);
        LCD_Control(hdl, LCD_READ_DMY_CYC, 8);
    }

    if(NV3041A_PIN_RES_SDO(g_st7782vPinCfg) && NV3041A_PIN_RES_SDI(g_st7782vPinCfg)) {
        LCD_Control(hdl, LCD_SPI_BIDIR, 0);
    }
    else {
        LCD_Control(hdl, LCD_SPI_BIDIR, 1);
    }
    LCD_Control(hdl, LCD_SAMP_SEL, 1);
    // LCD_Control(hdl, LCD_RGB565_SWAP, 1);

    NV3041A_RegInit(handle);

    if(handle->fmark_enable) {
    #ifdef LCD_TE_AUTO_RATE
        handle->te.fpsMinVal = NV3041A_FPS_MIN;
        handle->te.fpsMaxVal = NV3041A_FPS_MAX;
        handle->te.porchMinVal = NV3041A_PORCH_MIN;
        handle->te.porchMaxVal = NV3041A_PORCH_MAX;
        handle->te.fpsFunc = NV3041A_FpsRateConfig;
        handle->te.porchFunc = NV3041A_PorchConfig;
        // handle->te.teEnable = NV3041A_TE_Enable;
    #endif
        NV3041A_TE_Enable(handle, true);
        LCD_TeInit(&handle->te, NV3041A_TeInterruptServer, NV3041A_SendData, handle);
    }
}

void NV3041A_PinInit(PIN_MultiMux_t *bk, PIN_MultiMux_t *reset, PIN_MultiMux_t *csx, PIN_MultiMux_t *scl, PIN_MultiMux_t *sdi, PIN_MultiMux_t *sdo, PIN_MultiMux_t *dcx)
{
    g_st7782vPinCfg->backLight = bk;
    g_st7782vPinCfg->reset = reset;
    g_st7782vPinCfg->csx = csx;
    g_st7782vPinCfg->scl = scl;
    g_st7782vPinCfg->sdi = sdi;
    g_st7782vPinCfg->sdo = sdo;
    g_st7782vPinCfg->dcx = dcx;
}

void NV3041A_PowerControl(NV3041A_Handle *hdl, bool en)
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

int32_t NV3041A_BackLight(NV3041A_Handle *hdl, uint8_t level)
{
#if NV3041A_BACKLIGHT_SUPPORT_PWM
    return NV3041A_BK_En(hdl, level);
#else
    if(level) {
        return NV3041A_BK_En(hdl, 100);
    }
    else {
        return NV3041A_BK_En(hdl, 0);
    }
#endif
}

void NV3041A_Init(NV3041A_Handle *hdl)
{
    OS_ASSERT(g_st7782vPinCfg);

    hdl->lcd.SendCallback = NV3041A_InterruptServer;
    hdl->lcd.userData = hdl;

    hdl->madctl = NV3041A_MADCTL_DEF;

    hdl->fmark_enable = NV3041A_TE_ENABLE;

    hdl->pin = g_st7782vPinCfg;

#if NV3041A_BACKLIGHT_USE_SOFT_PWM
    hdl->pwm.freq = 100;
    hdl->pwm.diff = OS_TICK_PER_SECOND / hdl->pwm.freq;
    hdl->pwm.high = hdl->pwm.diff;
    hdl->pwm.low = hdl->pwm.diff - hdl->pwm.high;
    hdl->pwm.timer = osTimerNew(NV3041A_SoftPwmTimer, osTimerOnce, hdl, NULL);
#endif

    NV3041A_DrvInit(hdl);
}

void NV3041A_DeInit(NV3041A_Handle *hdl)
{
    hdl->madctl_last = hdl->madctl;

    if(hdl->fmark_enable) {
        LCD_TeDeInit(&hdl->te);
    }

    NV3041A_BackLight(hdl, 0);

#if NV3041A_BACKLIGHT_USE_SOFT_PWM
    if(hdl->pwm.timer) {
        osTimerDelete(hdl->pwm.timer);
    }
#endif

    LCD_PowerControl(&hdl->lcd, DRV_POWER_OFF);

    LCD_Uninitialize(&hdl->lcd);
}

void NV3041A_Flush(NV3041A_Handle *hdl, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint8_t *pixel)
{
    NV3041A_SetArea(&hdl->lcd, x1, x2, y1, y2);
    
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

void NV3041A_SetWidth(NV3041A_Handle *hdl, uint16_t width, uint16_t height)
{
    NV3041A_SetArea(&hdl->lcd, 0, width - 1, 0, height - 1);
}

void NV3041A_Sleep(NV3041A_Handle *hdl)
{
    // LCD_WriteParam(&hdl->lcd, 0x10,NULL, 0);

    NV3041A_DeInit(hdl);

    NV3041A_PowerControl(hdl, false);

    hdl->madctl_last = hdl->madctl;

#ifdef LCD_TE_AUTO_RATE
    hdl->porch_last = hdl->te.porchMaxVal;
#endif
}

void NV3041A_Wakeup(NV3041A_Handle *hdl)
{
    // LCD_WriteParam(&hdl->lcd, 0x11,NULL, 0);
    // osThreadMsSleep(120);

    NV3041A_PowerControl(hdl, true);
    
    NV3041A_Init(hdl);

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

void NV3041A_SetScreenMode(NV3041A_Handle *hdl, NV3041A_SCREEN_MODE mode)
{    
    switch (mode) {
        case NV3041A_SCREEN_PORTRAIT:
            hdl->madctl &= ~(0x1 << 5);
            break;

        case NV3041A_SCREEN_LANDSCAPE:
            hdl->madctl |= (0x1 << 5);
            break;
    }

    LCD_WriteParam(&hdl->lcd, CMD_MADCTL, &hdl->madctl, 1);
}

void NV3041A_SetFlip(NV3041A_Handle *hdl, bool x, bool y)
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

uint32_t NV3041A_GetID(NV3041A_Handle *hdl)
{
    uint32_t id = 0;
    LCD_ReadParam(&hdl->lcd, CMD_RDDID, (uint8_t *)&id, 3);
    return id;
}

void NV3041A_EndianConvert(NV3041A_Handle *hdl, bool enable)
{
    hdl->endian_enable = enable ? 1 : 0;
    LCD_Control(&hdl->lcd, LCD_RGB565_SWAP, hdl->endian_enable);
}
