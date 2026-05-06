/**
 * @file        cm_iomux.h
 * @brief       IOMUX接口
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By zyf
 * @date        2021/07/30
 *
 * @defgroup iomux
 * @ingroup PI
 * @{
 */
/**********************************************************************************
 ***********************IOMUX使用注意事项*************************************
 * 1、设备上电之后需要将各个引脚功能进行IOMUX配置,以唯一功能进行调试，中途不可变更;
 ***************************************************************************/

#ifndef __CM_IOMUX_H__
#define __CM_IOMUX_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
/****************************************************************************
 * Public Types
 ****************************************************************************/
typedef enum{
    /* 模组引脚号 - 芯片引脚号 */
    CM_IOMUX_PIN_16 = 21,    // PD_GPIO_0
    CM_IOMUX_PIN_17 = 5,     // LPUART0_RX
    CM_IOMUX_PIN_18 = 6,     // LPUART0_TX
    CM_IOMUX_PIN_20 = 25,    // PD_GPIO_4
    CM_IOMUX_PIN_21 = 51,    // PD_GPIO_17
    CM_IOMUX_PIN_22 = 7,     // LPUART0_CTS
    CM_IOMUX_PIN_23 = 8,     // LPUART0_RTS
    CM_IOMUX_PIN_25 = 48,    // AON_GPIO_7
    CM_IOMUX_PIN_28 = 27,    // PD_GPIO_6
    CM_IOMUX_PIN_29 = 28,    // PD_GPIO_7
    CM_IOMUX_PIN_38 = 12,    // SWD0_SWCLK
    CM_IOMUX_PIN_39 = 13,    // SWD0_SWDIO
    CM_IOMUX_PIN_49 = 49,    // AON_GPIO_8
    CM_IOMUX_PIN_54 = 24,    // PD_GPIO_3
    CM_IOMUX_PIN_55 = 23,    // PD_GPIO_2
    CM_IOMUX_PIN_56 = 22,    // PD_GPIO_1
    CM_IOMUX_PIN_57 = 14,    // CLK_AU_MCLK
    CM_IOMUX_PIN_58 = 50,    // PD_GPIO_16
    CM_IOMUX_PIN_62 = 30,    // PD_GPIO_9
    CM_IOMUX_PIN_63 = 29,    // PD_GPIO_8
    CM_IOMUX_PIN_64 = 31,    // PD_GPIO_10
    CM_IOMUX_PIN_74 = 9,     // AON_GPIO_0
    CM_IOMUX_PIN_75 = 26,    // PD_GPIO_5
    CM_IOMUX_PIN_76 = 34,    // PD_GPIO_13
    CM_IOMUX_PIN_77 = 36,    // PD_GPIO_15
    CM_IOMUX_PIN_79 = 10,    // AON_GPIO_1
    CM_IOMUX_PIN_80 = 11,    // CLK_EXT_OUT
    CM_IOMUX_PIN_81 = 32,    // PD_GPIO_11
    CM_IOMUX_PIN_86 = 35,    // PD_GPIO_14
    CM_IOMUX_PIN_87 = 33,    // PD_GPIO_12
    CM_IOMUX_PIN_MAX = 52,
} cm_iomux_pin_e;

/*IOMUX FUNC  definition */
typedef enum{
    CM_IOMUX_FUNC_FUNCTION0 ,         /*!<功能0*/
    CM_IOMUX_FUNC_FUNCTION1,          /*!<功能1*/
    CM_IOMUX_FUNC_FUNCTION2,          /*!<功能2*/
    CM_IOMUX_FUNC_FUNCTION3,          /*!<功能3*/
    CM_IOMUX_FUNC_FUNCTION4,          /*!<功能4*/
    CM_IOMUX_FUNC_FUNCTION5,          /*!<功能5*/
    CM_IOMUX_FUNC_FUNCTION6,          /*!<功能6*/
    CM_IOMUX_FUNC_FUNCTION7,          /*!<功能7*/
    CM_IOMUX_FUNC_FUNCTIONNUM_END,
} cm_iomux_func_e;

#define CM_IOMUX_AON_0_MUX_EN_BK1_N                    CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_AON_0_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_AON_0_MUX_AON_INT_0                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_AON_0_MUX_UART1_TX                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_AON_0_MUX_KBR_0                       CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_AON_0_MUX_AON_DEBUG_TEST_PIN0         CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_AON_0_MUX_AON_PWM_0                   CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_AON_0_MUX_CLK_EXT_OUT                 CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_AON_1_MUX_FPWM_BK1                    CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_AON_1_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_AON_1_MUX_AON_INT_1                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_AON_1_MUX_UART1_RX                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_AON_1_MUX_KBR_1                       CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_AON_1_MUX_AON_DEBUG_TEST_PIN1         CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_AON_1_MUX_AON_PWM_1                   CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_AON_1_MUX_CLK_EXT_REQ                 CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_AON_2_MUX_LPUART0_RX                  CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_AON_2_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_AON_2_MUX_AON_INT_2                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_AON_2_MUX_UART1_CTS                   CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_AON_2_MUX_SBY_DEBUG_TEST_PIN0         CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_AON_2_MUX_CLK_EXT_OUT                 CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_AON_2_MUX_KBR_0                       CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_AON_3_MUX_LPUART0_TX                  CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_AON_3_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_AON_3_MUX_AON_INT_3                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_AON_3_MUX_UART1_RTS                   CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_AON_3_MUX_SBY_DEBUG_TEST_PIN1         CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_AON_3_MUX_CLK_EXT_REQ                 CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_AON_3_MUX_KBR_1                       CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_AON_4_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_AON_4_MUX_CLK_32K_EXTIN               CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_AON_4_MUX_AON_INT_4                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_AON_4_MUX_LPUART0_CTS                 CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_AON_4_MUX_UART1_TX                    CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_AON_4_MUX_SBY_DEBUG_TEST_PIN2         CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_AON_4_MUX_AON_PWM_0                   CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_AON_4_MUX_KBR_2                       CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_AON_5_MUX_CLK_32K_EXTOUT              CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_AON_5_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_AON_5_MUX_AON_INT_5                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_AON_5_MUX_LPUART0_RTS                 CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_AON_5_MUX_UART1_RX                    CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_AON_5_MUX_SBY_DEBUG_TEST_PIN3         CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_AON_5_MUX_AON_PWM_1                   CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_AON_5_MUX_KBR_3                       CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_AON_6_MUX_CLK_EXT_OUT                 CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_AON_6_MUX_KBR_4                       CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_AON_6_MUX_AON_INT_6                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_AON_6_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_AON_6_MUX_CLK_26M_EXTIN               CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_AON_6_MUX_PDCORE_RSTN_0               CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_AON_6_MUX_CAM_MCLK                    CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_AON_6_MUX_CLK_AU_MCLK                 CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_AON_7_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_AON_7_MUX_KBC_4                       CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_AON_7_MUX_AON_INT_7                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_AON_7_MUX_UART3_RX                    CM_IOMUX_FUNC_FUNCTION3

#define CM_IOMUX_AON_8_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_AON_8_MUX_KBR_4                       CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_AON_8_MUX_AON_INT_8                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_AON_8_MUX_UART3_TX                    CM_IOMUX_FUNC_FUNCTION3

#define CM_IOMUX_PD_0_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_0_MUX_UART1_RX                     CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_0_MUX_SSP0_CLK                     CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_0_MUX_PD_INT_0                     CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_0_MUX_I2C0_SCL                     CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_0_MUX_CLK_AU_MCLK                  CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_0_MUX_KBC_0                        CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_0_MUX_LB_DEBUG_TEST_PIN_0          CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_1_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_1_MUX_UART1_TX                     CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_1_MUX_SSP0_RXD                     CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_1_MUX_PD_INT_1                     CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_1_MUX_I2C0_SDA                     CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_1_MUX_KBC_1                        CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_1_MUX_LB_DEBUG_TEST_PIN_1          CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_2_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_2_MUX_SWD1_SWCLK                   CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_2_MUX_SSP0_TXD                     CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_2_MUX_PD_INT_2                     CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_2_MUX_I2C1_SCL                     CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_2_MUX_SPI_CAM_DI2                  CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_2_MUX_KBC_2                        CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_2_MUX_LB_DEBUG_TEST_PIN_2          CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_3_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_3_MUX_SWD1_SWDIO                   CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_3_MUX_SSP0_CS                      CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_3_MUX_PD_INT_3                     CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_3_MUX_I2C1_SDA                     CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_3_MUX_SPI_CAM_DI3                  CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_3_MUX_KBC_3                        CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_3_MUX_CAM_MCLK                     CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_4_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_4_MUX_I2C1_SCL                     CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_4_MUX_PD_PWM                       CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_4_MUX_SPI_CAM_DI3                  CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_4_MUX_I2S0_WS                      CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_4_MUX_KBC_4                        CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_4_MUX_I2C0_SDA                     CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_5_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_5_MUX_I2C1_SDA                     CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_5_MUX_PD_PWM                       CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_5_MUX_SPI_CAM_TXD                  CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_5_MUX_I2S0_CLK                     CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_5_MUX_I2C0_SCL                     CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_6_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_6_MUX_I2C0_SCL                     CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_6_MUX_UART2_RX                     CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_6_MUX_I2S0_DIN                     CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_6_MUX_SPI_LCD_DCX                  CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_7_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_7_MUX_I2C0_SDA                     CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_7_MUX_UART2_TX                     CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_7_MUX_I2S0_DOUT                    CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_7_MUX_SPI_LCD_SDA                  CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_8_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_8_MUX_SPI_CAM_CS                   CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_8_MUX_I2S0_WS                      CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_8_MUX_PD_INT_4                     CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_8_MUX_PD_PWM                       CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_8_MUX_I2C0_SCL                     CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_8_MUX_SIM0_RST                     CM_IOMUX_FUNC_FUNCTION6

#define CM_IOMUX_PD_9_MUX_GPIO                         CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_9_MUX_SPI_CAM_CLK                  CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_9_MUX_I2S0_CLK                     CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_9_MUX_PD_INT_5                     CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_9_MUX_I2C0_SDA                     CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_9_MUX_SIM0_CLK                     CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_9_MUX_LB_DEBUG_TEST_PIN_3          CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_10_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_10_MUX_SPI_CAM_DI0                 CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_10_MUX_I2S0_DIN                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_10_MUX_UART1_RX                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_10_MUX_KBC_3                       CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_10_MUX_I2C1_SCL                    CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_10_MUX_SIM0_DATA                   CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_10_MUX_LB_DEBUG_TEST_PIN_4         CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_11_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_11_MUX_SPI_CAM_DI1                 CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_11_MUX_I2S0_DOUT                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_11_MUX_UART1_TX                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_11_MUX_I2C0_SDA                    CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_11_MUX_I2C1_SDA                    CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_11_MUX_KBC_4                       CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_11_MUX_SPI_LCD_DCX                 CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_12_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_12_MUX_SPI_LCD_CSX                 CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_12_MUX_I2C1_SCL                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_12_MUX_I2C0_SCL                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_12_MUX_PD_PWM                      CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_12_MUX_UART1_RX                    CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_12_MUX_I2S0_WS                     CM_IOMUX_FUNC_FUNCTION6

#define CM_IOMUX_PD_13_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_13_MUX_SPI_LCD_SCL                 CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_13_MUX_UART1_RX                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_13_MUX_I2C0_SDA                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_13_MUX_UART1_TX                    CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_13_MUX_I2S0_CLK                    CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_13_MUX_LB_DEBUG_TEST_PIN_5         CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_14_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_14_MUX_SPI_LCD_SDI                 CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_14_MUX_I2C1_SDA                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_14_MUX_PD_INT_6                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_14_MUX_SPI_CAM_TXD                 CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_14_MUX_UART1_CTS                   CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_14_MUX_I2S0_DIN                    CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_14_MUX_SPI_LCD_DCX                 CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_15_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_15_MUX_SPI_LCD_SDO                 CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_15_MUX_UART1_TX                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_15_MUX_PD_INT_7                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_15_MUX_KBC_0                       CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_15_MUX_UART1_RTS                   CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_15_MUX_I2S0_DOUT                   CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_15_MUX_SPI_LCD_SDA                 CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_16_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_16_MUX_SSP1_CS                     CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_16_MUX_UART3_TX                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_16_MUX_I2C2_SCL                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_16_MUX_SPI_CAM_CS                  CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_16_MUX_I2S0_WS                     CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_16_MUX_PD_INT_8                    CM_IOMUX_FUNC_FUNCTION6

#define CM_IOMUX_PD_17_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_17_MUX_SSP1_CLK                    CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_17_MUX_UART3_RX                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_17_MUX_I2C2_SDA                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_17_MUX_SPI_CAM_CLK                 CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_17_MUX_I2S0_CLK                    CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_17_MUX_PD_INT_9                    CM_IOMUX_FUNC_FUNCTION6

#define CM_IOMUX_PD_18_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_18_MUX_SSP1_RXD                    CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_18_MUX_UART3_CTS                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_18_MUX_I2C3_SCL                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_18_MUX_SPI_CAM_DI0                 CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_18_MUX_I2S0_DIN                    CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_18_MUX_PD_INT_10                   CM_IOMUX_FUNC_FUNCTION6

#define CM_IOMUX_PD_19_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_19_MUX_SSP1_TXD                    CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_19_MUX_UART3_RTS                   CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_19_MUX_I2C3_SDA                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_19_MUX_SPI_CAM_DI1                 CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_19_MUX_I2S0_DOUT                   CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_19_MUX_PD_INT_11                   CM_IOMUX_FUNC_FUNCTION6

#define CM_IOMUX_PD_20_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_20_MUX_RFC_GPIO_0                  CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_20_MUX_RF_SPI0_CS                  CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_20_MUX_KBC_2                       CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_20_MUX_LB_DEBUG_TEST_PIN_6         CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_21_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_21_MUX_RFC_GPIO_1                  CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_21_MUX_RF_SPI0_CLK                 CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_21_MUX_KBC_3                       CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_21_MUX_LB_DEBUG_TEST_PIN_7         CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_22_MUX_BOOT_MODE_0                 CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_22_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_22_MUX_RF_SPI0_DIN                 CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_22_MUX_KBC_0                       CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_22_MUX_SWD1_SWCLK                  CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_22_MUX_PD_GPIO_22                  CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_22_MUX_LB_DEBUG_TEST_PIN_12        CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_23_MUX_BOOT_MODE_1                 CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_23_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_23_MUX_RF_SPI0_DOUT                CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_23_MUX_KBC_1                       CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_23_MUX_SWD1_SWDIO                  CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_23_MUX_PD_GPIO_23                  CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_23_MUX_LB_DEBUG_TEST_PIN_13        CM_IOMUX_FUNC_FUNCTION7

#define CM_IOMUX_PD_24_MUX_SWD0_SWCLK                  CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_24_MUX_KBR_4                       CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_24_MUX_KBC_3                       CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_24_MUX_UART1_RX                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_24_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_24_MUX_SWD1_SWCLK                  CM_IOMUX_FUNC_FUNCTION5

#define CM_IOMUX_PD_25_MUX_SWD0_SWDIO                  CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_25_MUX_KBR_3                       CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_25_MUX_KBC_4                       CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_25_MUX_UART1_TX                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_25_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_25_MUX_SWD1_SWDIO                  CM_IOMUX_FUNC_FUNCTION5

#define CM_IOMUX_PD_26_MUX_EN_BK2_N                    CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_26_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_26_MUX_CLK_AU_MCLK                 CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_26_MUX_CAM_MCLK                    CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_26_MUX_PD_PWM                      CM_IOMUX_FUNC_FUNCTION4

#define CM_IOMUX_PD_29_MUX_SIM0_RST                    CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_29_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_29_MUX_I2C0_SCL                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_29_MUX_KBC_2                       CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_29_MUX_UART1_RX                    CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_29_MUX_I2S0_CLK                    CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_29_MUX_SPI_CAM_CS                  CM_IOMUX_FUNC_FUNCTION6

#define CM_IOMUX_PD_30_MUX_SIM0_CLK                    CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_30_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_30_MUX_I2C0_SDA                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_30_MUX_KBC_3                       CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_30_MUX_UART1_TX                    CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_30_MUX_I2S0_WS                     CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_30_MUX_SPI_CAM_CLK                 CM_IOMUX_FUNC_FUNCTION6

#define CM_IOMUX_PD_31_MUX_SIM0_DATA                   CM_IOMUX_FUNC_FUNCTION0
#define CM_IOMUX_PD_31_MUX_GPIO                        CM_IOMUX_FUNC_FUNCTION1
#define CM_IOMUX_PD_31_MUX_PD_INT_4                    CM_IOMUX_FUNC_FUNCTION2
#define CM_IOMUX_PD_31_MUX_KBC_4                       CM_IOMUX_FUNC_FUNCTION3
#define CM_IOMUX_PD_31_MUX_KBC_3                       CM_IOMUX_FUNC_FUNCTION4
#define CM_IOMUX_PD_31_MUX_I2S0_DIN                    CM_IOMUX_FUNC_FUNCTION5
#define CM_IOMUX_PD_31_MUX_SPI_CAM_DI1                 CM_IOMUX_FUNC_FUNCTION6
#define CM_IOMUX_PD_31_MUX_PD_PWM                      CM_IOMUX_FUNC_FUNCTION7

/*IOMUX PIN  cmd*/
typedef enum{
    CM_IOMUX_PINCMD0_SLEEP ,          /*!<休眠状态*/
    CM_IOMUX_PINCMD1_LPMEDEG,         /*!<边沿检测*/
    CM_IOMUX_PINCMD2_DRIVEABILITY,    /*!<驱动能力*/
    CM_IOMUX_PINCMD3_PULL,            /*!<上下拉*/
    CM_IOMUX_PINCMDNUM_END
} cm_iomux_pincmd_e;

/*IOMUX PIN  cmdnum enumeration*/
typedef enum{
    CM_IOMUX_PINCMD0_FUNC0_SLEEP_NONE,             /*!<不使能 pad 的 sleep 功能*/
    CM_IOMUX_PINCMD0_FUNC1_SLEEP_DIR,              /*!<不使能 sleep，只是设置为 sleep 时候，输入状态*/
    CM_IOMUX_PINCMD0_FUNC2_SLEEP_DATA,             /*!<不使能 sleep，只是设置为 sleep 时候，输出状态*/
    CM_IOMUX_PINCMD0_FUNC3_SLEEP_FLOAT,            /*!<使能 sleep 功能，设置为 sleep 时候，输入状态*/
    CM_IOMUX_PINCMD0_FUNC4_SLEEP_OUTPUT_HIGH,      /*!<使能 sleep 功能，设置为 sleep 时候，输出高状态*/
    CM_IOMUX_PINCMD0_FUNC5_SLEEP_OUTPUT_LOW,       /*!<使能 sleep 功能，设置为 sleep 时候，输出低状态*/
    CM_IOMUX_PINCMD0_FUNCNUM_END
} cm_iomux_pincmd0_e;

typedef enum{
    CM_IOMUX_PINCMD1_FUNC0_LPM_EDGE_NONE,        /*!<不使能 pad 的边沿检测功能,需配合休眠解锁cm_pm_work_unlock函数使用*/
    CM_IOMUX_PINCMD1_FUNC1_LPM_EDGE_RISE ,       /*!<使能 pad 的边沿检测功能，且是上升沿。并提供唤醒事件,需配合休眠锁cm_pm_work_lock函数使用*/
    CM_IOMUX_PINCMD1_FUNC2_LPM_EDGE_FALL,        /*!<使能 pad 的边沿检测功能，且是下降沿。并提供唤醒事件.需配合休眠锁cm_pm_work_lock函数使用*/
    CM_IOMUX_PINCMD1_FUNC3_EDGE_BOTH,            /*!<使能 pad 的边沿检测功能，且是双沿。并提供唤醒事件.需配合休眠锁cm_pm_work_lock函数使用*/
    CM_IOMUX_PINCMD1_FUNCNUM_END
} cm_iomux_pincmd1_e;

typedef enum{
    CM_IOMUX_PINCMD2_FUNC0_DRIVE_VERY_SLOW,
    CM_IOMUX_PINCMD2_FUNC1_DRIVE_SLOW,
    CM_IOMUX_PINCMD2_FUNC2_DRIVE_MEDIUM,
    CM_IOMUX_PINCMD2_FUNC3_DRIVE_FAST,
    CM_IOMUX_PINCMD2_FUNC4_MASK,
    CM_IOMUX_PINCMD2_FUNCNUM_END
} cm_iomux_pincmd2_e;

typedef enum{
    CM_IOMUX_PINCMD3_FUNC0_PULL_NONE,              /*!<不使能内部的上拉下拉功能*/
    CM_IOMUX_PINCMD3_FUNC1_PULL_LOW,               /*!<使能下拉*/
    CM_IOMUX_PINCMD3_FUNC2_PULL_HIGH,              /*!<使能上拉*/
    CM_IOMUX_PINCMD3_FUNC3_PULL_BOTH,              /*!<使能上下拉*/
    CM_IOMUX_PINCMD3_FUNC4_PULL_FLOAT,             /*!<浮空功能*/
    CM_IOMUX_PINCMD3_FUNCNUM_END
} cm_iomux_pincmd3_e;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*IOMUX PIN FUNCTION  definition*/


/****************************************************************************
 * Public Data
 ****************************************************************************/


/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/**
 * @brief IOMUX 设置引脚复用功能
 *
 * @param [in] pin PIN 定义号
 * @param [in] fun FUN 定义号
 *
 *  @return
 *    = 0  - 成功 \n
 *    = -1 - 失败
 */
int32_t cm_iomux_set_pin_func(cm_iomux_pin_e pin, cm_iomux_func_e fun);

/**
 * @brief IOMUX 获取引脚功能
 *
 * @param [in] pin PIN 定义号
 * @param [out] fun FUN 定义号
 *
 * @return
 *    = 0  - 成功\n
 *    < 0  - 失败
 */
int32_t cm_iomux_get_pin_func(cm_iomux_pin_e pin, cm_iomux_func_e *fun);

/**
 * @brief IOMUX PIN脚控制
 *
 * @param [in] pin PIN 定义号
 * @param [in] cmd 功能定义
 * @param [in] cmd_arg 具体功能值
 *
 * @return
 *    = 0  - 成功\n
 *    < 0  - 失败
 */
int32_t cm_iomux_set_pin_cmd(cm_iomux_pin_e pin, cm_iomux_pincmd_e cmd, uint8_t cmd_arg);


#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __CM_IOMUX_H__ */

/** @}*/
