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
#if defined(_CPU_AP)
#include "chip_pin_list.h"
#include "drv_pin.h"
#include "pinmap.h"
#include <os_def.h>


const PM_PINFUNC_T g_PinFunc[] = {

/********************************AON*************************************** */
/*{ pinNum 引脚名称,      pinmux   功能复用,         flags      功能标志位,
                                                    pinDrv     驱动能力,
                                                    pinSen     施密特触发,
                                                    pinPull    上下拉,
                                                    pinDir     输入输出方向,
                                                    pinRate     速率},*/
{LPUART0_RX,             PIN_5_MUX_LPUART0_RX,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_UP,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{LPUART0_TX,             PIN_6_MUX_LPUART0_TX,      PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{LPUART0_CTS,            PIN_7_MUX_GPIO,            PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{LPUART0_RTS,            PIN_8_MUX_GPIO,            PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{AON_GPIO_0,             PIN_9_MUX_GPIO,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{AON_GPIO_1,             PIN_10_MUX_AON_INT_1,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_DISABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{CLK_EXT_OUT,            PIN_11_MUX_GPIO,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{CLK_AU_MCLK,            PIN_14_MUX_GPIO,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},
{AON_GPIO_7,             PIN_48_MUX_UART3_RX,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_UP,
                                                    .pinDir =   PIN_INPUT_ENABLE},

{AON_GPIO_8,             PIN_49_MUX_UART3_TX,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_DIR,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinDir =   PIN_INPUT_ENABLE},
/*********************************PD******************************************* */
// PD域IO没有方向寄存器，不能配置 PIN_CFG_DIR(pinDir)属性，不然会死机
{PD_GPIO_0,              PIN_21_MUX_SSP0_CLK,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_1,              PIN_22_MUX_SSP0_RXD,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_2,              PIN_23_MUX_SSP0_TXD,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_3,              PIN_24_MUX_SSP0_CS,        PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_4,              PIN_25_MUX_GPIO,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_5,              PIN_26_MUX_GPIO,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_6,              PIN_27_MUX_UART2_RX,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_UP,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_7,              PIN_28_MUX_UART2_TX,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_8,              PIN_29_MUX_GPIO,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_9,              PIN_30_MUX_GPIO,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_10,             PIN_31_MUX_GPIO,      PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_11,             PIN_32_MUX_GPIO,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_12,             PIN_33_MUX_UART1_RX,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_UP,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_13,             PIN_34_MUX_UART1_TX,       PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_14,             PIN_35_MUX_UART1_CTS,      PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_15,             PIN_36_MUX_UART1_RTS,      PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_16,             PIN_50_MUX_GPIO,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_NONE,
                                                    .pinRate =  SLEW_RATE_FAST},

{PD_GPIO_17,             PIN_51_MUX_GPIO,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_UP,
                                                    .pinRate =  SLEW_RATE_FAST},

{PD_GPIO_18,             PIN_52_MUX_GPIO,            PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{PD_GPIO_19,             PIN_53_MUX_GPIO,           PIN_CFG_DRV | PIN_CFG_SEN | PIN_CFG_PULL | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_1,
                                                    .pinSen =   SEN_ENABLE,
                                                    .pinPull =  PULL_DOWN,
                                                    .pinRate =  SLEW_RATE_LOW},

{SWD0_SWCLK,             PIN_12_MUX_SWD0_SWCLK,     PIN_CFG_DRV | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinRate =  SLEW_RATE_LOW},

{SWD0_SWDIO,             PIN_13_MUX_SWD0_SWDIO,     PIN_CFG_DRV | PIN_CFG_RATE,
                                                    .pinDrv =   PIN_DRV_2,
                                                    .pinRate =  SLEW_RATE_LOW},
};

const int PINFUNC_SIZE = sizeof(g_PinFunc)/(sizeof(g_PinFunc[0]));


/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

void prv_PIN_Config(const PM_PINFUNC_T* pin)
{
    PIN_SetMux(pin->pinNum, pin->pinMux);
    if(pin->flags & PIN_CFG_DRV)
    {
        PIN_SetDrCap(pin->pinNum, (pin->pinDrv));
    }

    if(pin->flags & PIN_CFG_SEN)
    {
        PIN_SetIS(pin->pinNum, (pin->pinSen));
    }

    if(pin->flags & PIN_CFG_PULL)
    {
        PIN_SetPull(pin->pinNum, (pin->pinPull));
    }

    if(pin->flags & PIN_CFG_RATE)
    {
        PIN_SetSlewRate(pin->pinNum, (pin->pinRate));
    }

    if(pin->flags & PIN_CFG_DIR)
    {
        PIN_SetIE(pin->pinNum, (pin->pinDir));
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

int Board_PinInit(void)
{

    for(uint8_t i = 0; i < PINFUNC_SIZE; i++)
    {
        prv_PIN_Config(&g_PinFunc[i]);
    }

    return 0;
}
INIT_BOARD_EXPORT(Board_PinInit, OS_INIT_SUBLEVEL_HIGH);
#endif