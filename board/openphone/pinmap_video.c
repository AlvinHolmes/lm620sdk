/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file pinmap.c
*
* @brief  pin配置文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-11      ICT Team             创建
************************************************************************************/

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#if defined(_CPU_AP) && defined(USE_TOP_VIDEO) && defined(USE_TOP_VOLTE)
#include "chip_pin_list.h"
#include "drv_pin.h"
#include "pinmap.h"
#include "drv_pcu.h"
#include <os_def.h>
#include "nr_micro_shell.h"



const PM_PINFUNC_T g_PinFunc_video[] = {

/********************************AON*************************************** */
/*{ pinNum 引脚名称,      pinmux   功能复用,         flags      功能标志位,
                                                    pinDrv     驱动能力,
                                                    pinSen     施密特触发,
                                                    pinPull    上下拉,
                                                    pinDir     输入输出方向,
                                                    pinRate     速率},*/
{LPUART0_RX,             PIN_5_MUX_LPUART0_RX,       PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_UP,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{LPUART0_TX,             PIN_6_MUX_LPUART0_TX,       PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{LPUART0_CTS,            PIN_7_MUX_GPIO,             PIN_CFG_DIR | GPIO_OUT,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE,
                                                    .gpioOut =  GPIO_LOW},

{LPUART0_RTS,            PIN_8_MUX_GPIO,             PIN_CFG_DIR | GPIO_OUT,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE,
                                                    .gpioOut =  GPIO_LOW},

{AON_GPIO_0,             PIN_9_MUX_AON_INT_0,         PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{AON_GPIO_1,             PIN_10_MUX_FPWM_BK1,        PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_DISABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_DISABLE},

{CLK_EXT_OUT,            PIN_11_MUX_CAM_MCLK,        PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{SWD0_SWCLK,             PIN_12_MUX_SWD0_SWCLK,      PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinRate =  SLEW_RATE_LOW},

{SWD0_SWDIO,             PIN_13_MUX_SWD0_SWDIO,      PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinRate =  SLEW_RATE_LOW},

{CLK_AU_MCLK,            PIN_14_MUX_GPIO,            PIN_CFG_RATE | GPIO_OUT,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW,
                                                    .gpioOut =  GPIO_LOW},

/*********************************PD****************************************** */
{PD_GPIO_0,              PIN_21_MUX_SSP0_CLK,        PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_1,              PIN_22_MUX_SSP0_RXD,        PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_2,              PIN_23_MUX_SSP0_TXD,        PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_3,              PIN_24_MUX_SSP0_CS,         PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_4,              PIN_25_MUX_I2S0_WS,          PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},  //CodecPin  25-28

{PD_GPIO_5,              PIN_26_MUX_I2S0_CLK,         PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_6,              PIN_27_MUX_I2S0_DIN,         PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_7,              PIN_28_MUX_I2S0_DOUT,        PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_8,              PIN_29_MUX_GPIO,            PIN_CFG_RATE | GPIO_OUT,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW,
                                                    .gpioOut =  GPIO_LOW},

{PD_GPIO_9,              PIN_30_MUX_SPI_CAM_CLK,     PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_10,             PIN_31_MUX_SPI_CAM_DI0,     PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_11,             PIN_32_MUX_SPI_CAM_DI1,     PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_12,             PIN_33_MUX_SPI_LCD_CSX,     PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_13,             PIN_34_MUX_SPI_LCD_SCL,     PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_14,             PIN_35_MUX_SPI_LCD_SDI,     PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_15,             PIN_36_MUX_SPI_LCD_SDO,     PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{SIM0_RST,               PIN_37_MUX_SIM0_RST,        PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{SIM0_CLK,               PIN_38_MUX_SIM0_CLK,        PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{SIM0_DATA,              PIN_39_MUX_SIM0_DATA,       PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{RF_CONTROL_0,           PIN_40_MUX_RFC_GPIO_0,      PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{RF_CONTROL_1,           PIN_41_MUX_RFC_GPIO_1,      PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{RF_CONTROL_2,           PIN_42_MUX_RFC_GPIO_2,      PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{RF_CONTROL_3,           PIN_43_MUX_GPIO,            PIN_CFG_RATE | GPIO_OUT,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW,
                                                    .gpioOut =  GPIO_LOW},

{RF_CONTROL_4,           PIN_44_MUX_RFFE_SDATA,      PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{RF_CONTROL_5,           PIN_45_MUX_RFFE_SCLK,       PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{RF_CONTROL_6,           PIN_46_MUX_BOOT_MODE_0,     PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{RF_CONTROL_7,           PIN_47_MUX_BOOT_MODE_1,     PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{AON_GPIO_7,             PIN_48_MUX_UART3_RX,         PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_UP,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{AON_GPIO_8,             PIN_49_MUX_UART3_TX,        PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{PD_GPIO_16,             PIN_50_MUX_GPIO,            PIN_CFG_RATE | GPIO_OUT,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_FAST,
                                                    .gpioOut =  GPIO_LOW},

{PD_GPIO_17,             PIN_51_MUX_GPIO,            PIN_CFG_RATE | GPIO_OUT,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_FAST,
                                                    .gpioOut =  GPIO_LOW},

{PD_GPIO_18,             PIN_52_MUX_GPIO,            PIN_CFG_RATE | GPIO_OUT,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_FAST,
                                                    .gpioOut =  GPIO_LOW},

{PD_GPIO_19,             PIN_53_MUX_GPIO,            PIN_CFG_RATE | GPIO_OUT,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_FAST,
                                                    .gpioOut =  GPIO_LOW},
};

const int PINFUNC_SIZE_VIDEO = sizeof(g_PinFunc_video)/(sizeof(g_PinFunc_video[0]));


/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

void prv_PIN_ConfigVi(const PM_PINFUNC_T* pin)
{
    PIN_SetMux(pin->pinNum, pin->pinMux);

    PIN_SetDrCap(pin->pinNum, (pin->pinDrv));

    PIN_SetIS(pin->pinNum, (pin->pinSen));

    PIN_SetPull(pin->pinNum, (pin->pinPull));


    if(pin->flags & PIN_CFG_RATE)
    {
        PIN_SetSlewRate(pin->pinNum, (pin->pinRate));
    }

    if(pin->flags & PIN_CFG_DIR)
    {
        PIN_SetIE(pin->pinNum, (pin->pinDir));
    }

    if(pin->flags & GPIO_OUT)
    {
        GPIO_SetDir(pin->pinNum, GPIO_OUTPUT);
        GPIO_Write(pin->pinNum, (pin->gpioOut));
    }
    else
    {
        GPIO_SetDir(pin->pinNum, GPIO_INPUT);
    }
}


/**
 ************************************************************************************
 * @brief           PIN配置
 *
 * @param[in]       void
 *
 * @return          int32_t
 * @retval          0          成功
 ************************************************************************************
*/

int Board_VideoPinInit(void)
{

    for(uint8_t i = 0; i < PINFUNC_SIZE_VIDEO; i++)
    {
        prv_PIN_ConfigVi(&g_PinFunc_video[i]);
    }

    return 0;
}
// INIT_BOARD_EXPORT(Board_VideoPinInit, OS_INIT_SUBLEVEL_HIGH);
#endif