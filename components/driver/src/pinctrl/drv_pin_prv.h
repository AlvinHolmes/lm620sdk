/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_pin_private.h
 *
 * @brief       pin&GPIO驱动接口.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-04-21     ict team          创建
 ************************************************************************************
 */


#ifndef _DRV_PIN_PRIVATE_H
#define _DRV_PIN_PRIVATE_H
/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

typedef enum
{
/*** Bit definition for [regIoSelCfg0] register(0x00000000) ****/
  RBIT_AON_GPIO_0_SEL_REG              = (0x00000003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_AON_GPIO_1_SEL_REG              = (0x00000303),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_AON_GPIO_7_SEL_REG              = (0x00000603),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_AON_GPIO_8_SEL_REG              = (0x00000903),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_CLK_EXT_OUT_SEL_REG             = (0x00000c03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_LPUART0_RX_SEL_REG              = (0x00000f03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_LPUART0_TX_SEL_REG              = (0x00001203),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_LPUART0_CTS_SEL_REG             = (0x00001503),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_LPUART0_RTS_SEL_REG             = (0x00001803),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */


/*** Bit definition for [regIoIsCfg0] register(0x00000014) ****/
  RBIT_AON_GPIO_0_IS_REG               = (0x00140001),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_AON_GPIO_1_IS_REG               = (0x00140101),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_AON_GPIO_7_IS_REG               = (0x00140201),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_AON_GPIO_8_IS_REG               = (0x00140301),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_CLK_EXT_OUT_IS_REG              = (0x00140401),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_LPUART0_RX_IS_REG               = (0x00140501),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_LPUART0_TX_IS_REG               = (0x00140601),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_LPUART0_CTS_IS_REG              = (0x00140701),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_LPUART0_RTS_IS_REG              = (0x00140801),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */


/*** Bit definition for [regIoPsCfg0] register(0x0000001c) ****/
  RBIT_AON_GPIO_0_PS_REG               = (0x001c0001),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_AON_GPIO_1_PS_REG               = (0x001c0101),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_AON_GPIO_7_PS_REG               = (0x001c0201),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_AON_GPIO_8_PS_REG               = (0x001c0301),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_CLK_EXT_OUT_PS_REG              = (0x001c0401),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_LPUART0_RX_PS_REG               = (0x001c0501),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_LPUART0_TX_PS_REG               = (0x001c0601),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_LPUART0_CTS_PS_REG              = (0x001c0701),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_LPUART0_RTS_PS_REG              = (0x001c0801),  /*!<pad上下拉功能，1：上拉，0：下拉 */


/*** Bit definition for [regIoPeCfg0] register(0x00000024) ****/
  RBIT_AON_GPIO_0_PE_REG               = (0x00240001),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_AON_GPIO_1_PE_REG               = (0x00240101),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_AON_GPIO_7_PE_REG               = (0x00240201),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_AON_GPIO_8_PE_REG               = (0x00240301),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_CLK_EXT_OUT_PE_REG              = (0x00240401),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_LPUART0_RX_PE_REG               = (0x00240501),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_LPUART0_TX_PE_REG               = (0x00240601),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_LPUART0_CTS_PE_REG              = (0x00240701),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_LPUART0_RTS_PE_REG              = (0x00240801),  /*!<pad上下拉使能，1：使能，0：不使能 */


/*** Bit definition for [regIoDsCfg0] register(0x0000002c) ****/
  RBIT_AON_GPIO_0_DS_REG               = (0x002c0002),  /*!<pad驱动能力调整 */
  RBIT_AON_GPIO_1_DS_REG               = (0x002c0202),  /*!<pad驱动能力调整 */
  RBIT_AON_GPIO_7_DS_REG               = (0x002c0402),  /*!<pad驱动能力调整 */
  RBIT_AON_GPIO_8_DS_REG               = (0x002c0602),  /*!<pad驱动能力调整 */
  RBIT_CLK_EXT_OUT_DS_REG              = (0x002c0802),  /*!<pad驱动能力调整 */
  RBIT_LPUART0_RX_DS_REG               = (0x002c0a02),  /*!<pad驱动能力调整 */
  RBIT_LPUART0_TX_DS_REG               = (0x002c0c02),  /*!<pad驱动能力调整 */
  RBIT_LPUART0_CTS_DS_REG              = (0x002c0e02),  /*!<pad驱动能力调整 */
  RBIT_LPUART0_RTS_DS_REG              = (0x002c1002),  /*!<pad驱动能力调整 */


/*** Bit definition for [regIoIeCfg0] register(0x0000003c) ****/
  RBIT_AON_GPIO_0_IE_REG               = (0x003c0001),  /*!<input enable:  0, disable   1,enable */
  RBIT_AON_GPIO_1_IE_REG               = (0x003c0101),  /*!<input enable:  0, disable   1,enable */
  RBIT_AON_GPIO_7_IE_REG               = (0x003c0201),  /*!<input enable:  0, disable   1,enable */
  RBIT_AON_GPIO_8_IE_REG               = (0x003c0301),  /*!<input enable:  0, disable   1,enable */
  RBIT_CLK_EXT_OUT_IE_REG              = (0x003c0401),  /*!<input enable:  0, disable   1,enable */
  RBIT_LPUART0_RX_IE_REG               = (0x003c0501),  /*!<input enable:  0, disable   1,enable */
  RBIT_LPUART0_TX_IE_REG               = (0x003c0601),  /*!<input enable:  0, disable   1,enable */
  RBIT_LPUART0_CTS_IE_REG              = (0x003c0701),  /*!<input enable:  0, disable   1,enable */
  RBIT_LPUART0_RTS_IE_REG              = (0x003c0801),  /*!<input enable:  0, disable   1,enable */


} RBIT_AonPadCtrl;

typedef enum
{
/*** Bit definition for [regIoSelCfg0] register(0x00000000) ****/
  RBIT_SWD0_SWCLK_SEL_REG              = (0x00000003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_SWD0_SWDIO_SEL_REG              = (0x00000403),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_CLK_AU_MCLK_SEL_REG             = (0x00000803),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */


/*** Bit definition for [regIoSelCfg1] register(0x00000004) ****/
  RBIT_PD_GPIO_0_SEL_REG               = (0x00040c03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_1_SEL_REG               = (0x00041003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_2_SEL_REG               = (0x00041403),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_3_SEL_REG               = (0x00041803),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_4_SEL_REG               = (0x00041c03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */


/*** Bit definition for [regIoSelCfg2] register(0x00000008) ****/
  RBIT_PD_GPIO_5_SEL_REG               = (0x00080003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_6_SEL_REG               = (0x00080403),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_7_SEL_REG               = (0x00080803),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_8_SEL_REG               = (0x00080c03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_9_SEL_REG               = (0x00081003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_10_SEL_REG              = (0x00081403),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_11_SEL_REG              = (0x00081803),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_12_SEL_REG              = (0x00081c03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */


/*** Bit definition for [regIoSelCfg3] register(0x0000000c) ****/
  RBIT_PD_GPIO_13_SEL_REG              = (0x000c0003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_14_SEL_REG              = (0x000c0403),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_15_SEL_REG              = (0x000c0803),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_SIM0_RST_SEL_REG                = (0x000c0c03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_SIM0_CLK_SEL_REG                = (0x000c1003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_SIM0_DATA_SEL_REG               = (0x000c1403),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_RF_CONTROL_0_SEL_REG            = (0x000c1803),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_RF_CONTROL_1_SEL_REG            = (0x000c1c03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */


/*** Bit definition for [regIoSelCfg4] register(0x00000010) ****/
  RBIT_RF_CONTROL_2_SEL_REG            = (0x00100003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_RF_CONTROL_3_SEL_REG            = (0x00100403),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_RF_CONTROL_4_SEL_REG            = (0x00100803),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_RF_CONTROL_5_SEL_REG            = (0x00100c03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_RF_CONTROL_6_SEL_REG            = (0x00101003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_RF_CONTROL_7_SEL_REG            = (0x00101403),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_16_SEL_REG              = (0x00101803),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_17_SEL_REG              = (0x00101c03),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */


/*** Bit definition for [regIoSelCfg5] register(0x00000040) ****/
  RBIT_PD_GPIO_18_SEL_REG              = (0x00400003),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */
  RBIT_PD_GPIO_19_SEL_REG              = (0x00400403),  /*!<pad功能选择，0：function0，1：function1，2：function2，3：function3，4：function4，5：function5，6：function6，7：function7 */


/*** Bit definition for [regIoIsCfg0] register(0x00000014) ****/
  RBIT_SWD0_SWCLK_IS_REG               = (0x00140001),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SWD0_SWDIO_IS_REG               = (0x00140101),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_CLK_AU_MCLK_IS_REG              = (0x00140201),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SPI0_CS_IS_REG                  = (0x00140501),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SPI0_CLK_IS_REG                 = (0x00140601),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SPI0_D_0_IS_REG                 = (0x00140701),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SPI0_D_1_IS_REG                 = (0x00140801),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SPI0_D_2_IS_REG                 = (0x00140901),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SPI0_D_3_IS_REG                 = (0x00140a01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_0_IS_REG                = (0x00140b01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_1_IS_REG                = (0x00140c01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_2_IS_REG                = (0x00140d01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_3_IS_REG                = (0x00140e01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_4_IS_REG                = (0x00140f01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_5_IS_REG                = (0x00141001),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_6_IS_REG                = (0x00141101),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_7_IS_REG                = (0x00141201),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_8_IS_REG                = (0x00141301),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_9_IS_REG                = (0x00141401),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_10_IS_REG               = (0x00141501),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_11_IS_REG               = (0x00141601),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_12_IS_REG               = (0x00141701),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_13_IS_REG               = (0x00141801),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_14_IS_REG               = (0x00141901),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_15_IS_REG               = (0x00141a01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SIM0_RST_IS_REG                 = (0x00141b01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SIM0_CLK_IS_REG                 = (0x00141c01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_SIM0_DATA_IS_REG                = (0x00141d01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */


/*** Bit definition for [regIoIsCfg1] register(0x00000018) ****/
  RBIT_RF_CONTROL_0_IS_REG             = (0x00180001),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_RF_CONTROL_1_IS_REG             = (0x00180101),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_RF_CONTROL_2_IS_REG             = (0x00180201),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_RF_CONTROL_3_IS_REG             = (0x00180301),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_RF_CONTROL_4_IS_REG             = (0x00180401),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_RF_CONTROL_5_IS_REG             = (0x00180501),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_RF_CONTROL_6_IS_REG             = (0x00180601),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_RF_CONTROL_7_IS_REG             = (0x00180701),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_16_IS_REG               = (0x00180801),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_17_IS_REG               = (0x00180901),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_18_IS_REG               = (0x00180a01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */
  RBIT_PD_GPIO_19_IS_REG               = (0x00180b01),  /*!<Input Select，1：CMOS Schmitt input，0：CMOS input */


/*** Bit definition for [regIoPsCfg0] register(0x0000001c) ****/
  RBIT_SWD0_SWCLK_PS_REG               = (0x001c0001),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SWD0_SWDIO_PS_REG               = (0x001c0101),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_CLK_AU_MCLK_PS_REG              = (0x001c0201),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SPI0_CS_PS_REG                  = (0x001c0501),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SPI0_CLK_PS_REG                 = (0x001c0601),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SPI0_D_0_PS_REG                 = (0x001c0701),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SPI0_D_1_PS_REG                 = (0x001c0801),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SPI0_D_2_PS_REG                 = (0x001c0901),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SPI0_D_3_PS_REG                 = (0x001c0a01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_0_PS_REG                = (0x001c0b01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_1_PS_REG                = (0x001c0c01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_2_PS_REG                = (0x001c0d01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_3_PS_REG                = (0x001c0e01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_4_PS_REG                = (0x001c0f01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_5_PS_REG                = (0x001c1001),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_6_PS_REG                = (0x001c1101),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_7_PS_REG                = (0x001c1201),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_8_PS_REG                = (0x001c1301),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_9_PS_REG                = (0x001c1401),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_10_PS_REG               = (0x001c1501),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_11_PS_REG               = (0x001c1601),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_12_PS_REG               = (0x001c1701),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_13_PS_REG               = (0x001c1801),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_14_PS_REG               = (0x001c1901),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_15_PS_REG               = (0x001c1a01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SIM0_RST_PS_REG                 = (0x001c1b01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SIM0_CLK_PS_REG                 = (0x001c1c01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_SIM0_DATA_PS_REG                = (0x001c1d01),  /*!<pad上下拉功能，1：上拉，0：下拉 */


/*** Bit definition for [regIoPsCfg1] register(0x00000020) ****/
  RBIT_RF_CONTROL_0_PS_REG             = (0x00200001),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_RF_CONTROL_1_PS_REG             = (0x00200101),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_RF_CONTROL_2_PS_REG             = (0x00200201),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_RF_CONTROL_3_PS_REG             = (0x00200301),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_RF_CONTROL_4_PS_REG             = (0x00200401),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_RF_CONTROL_5_PS_REG             = (0x00200501),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_RF_CONTROL_6_PS_REG             = (0x00200601),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_RF_CONTROL_7_PS_REG             = (0x00200701),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_16_PS_REG               = (0x00200801),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_17_PS_REG               = (0x00200901),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_18_PS_REG               = (0x00200a01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
  RBIT_PD_GPIO_19_PS_REG               = (0x00200b01),  /*!<pad上下拉功能，1：上拉，0：下拉 */


/*** Bit definition for [regIoPeCfg0] register(0x00000024) ****/
  RBIT_SWD0_SWCLK_PE_REG               = (0x00240001),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SWD0_SWDIO_PE_REG               = (0x00240101),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_CLK_AU_MCLK_PE_REG              = (0x00240201),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SPI0_CS_PE_REG                  = (0x00240501),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SPI0_CLK_PE_REG                 = (0x00240601),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SPI0_D_0_PE_REG                 = (0x00240701),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SPI0_D_1_PE_REG                 = (0x00240801),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SPI0_D_2_PE_REG                 = (0x00240901),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SPI0_D_3_PE_REG                 = (0x00240a01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_0_PE_REG                = (0x00240b01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_1_PE_REG                = (0x00240c01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_2_PE_REG                = (0x00240d01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_3_PE_REG                = (0x00240e01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_4_PE_REG                = (0x00240f01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_5_PE_REG                = (0x00241001),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_6_PE_REG                = (0x00241101),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_7_PE_REG                = (0x00241201),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_8_PE_REG                = (0x00241301),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_9_PE_REG                = (0x00241401),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_10_PE_REG               = (0x00241501),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_11_PE_REG               = (0x00241601),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_12_PE_REG               = (0x00241701),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_13_PE_REG               = (0x00241801),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_14_PE_REG               = (0x00241901),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_15_PE_REG               = (0x00241a01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SIM0_RST_PE_REG                 = (0x00241b01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SIM0_CLK_PE_REG                 = (0x00241c01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_SIM0_DATA_PE_REG                = (0x00241d01),  /*!<pad上下拉使能，1：使能，0：不使能 */


/*** Bit definition for [regIoPeCfg1] register(0x00000028) ****/
  RBIT_RF_CONTROL_0_PE_REG             = (0x00280001),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_RF_CONTROL_1_PE_REG             = (0x00280101),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_RF_CONTROL_2_PE_REG             = (0x00280201),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_RF_CONTROL_3_PE_REG             = (0x00280301),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_RF_CONTROL_4_PE_REG             = (0x00280401),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_RF_CONTROL_5_PE_REG             = (0x00280501),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_RF_CONTROL_6_PE_REG             = (0x00280601),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_RF_CONTROL_7_PE_REG             = (0x00280701),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_16_PE_REG               = (0x00280801),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_17_PE_REG               = (0x00280901),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_18_PE_REG               = (0x00280a01),  /*!<pad上下拉使能，1：使能，0：不使能 */
  RBIT_PD_GPIO_19_PE_REG               = (0x00280b01),  /*!<pad上下拉使能，1：使能，0：不使能 */


/*** Bit definition for [regIoDsCfg0] register(0x0000002c) ****/
  RBIT_SWD0_SWCLK_DS_REG               = (0x002c0002),  /*!<pad驱动能力调整 */
  RBIT_SWD0_SWDIO_DS_REG               = (0x002c0202),  /*!<pad驱动能力调整 */
  RBIT_CLK_AU_MCLK_DS_REG              = (0x002c0402),  /*!<pad驱动能力调整 */
  RBIT_SPI0_CS_DS_REG                  = (0x002c0a02),  /*!<pad驱动能力调整 */
  RBIT_SPI0_CLK_DS_REG                 = (0x002c0c02),  /*!<pad驱动能力调整 */
  RBIT_SPI0_D_0_DS_REG                 = (0x002c0e02),  /*!<pad驱动能力调整 */
  RBIT_SPI0_D_1_DS_REG                 = (0x002c1002),  /*!<pad驱动能力调整 */
  RBIT_SPI0_D_2_DS_REG                 = (0x002c1202),  /*!<pad驱动能力调整 */
  RBIT_SPI0_D_3_DS_REG                 = (0x002c1402),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_0_DS_REG                = (0x002c1602),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_1_DS_REG                = (0x002c1802),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_2_DS_REG                = (0x002c1a02),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_3_DS_REG                = (0x002c1c02),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_4_DS_REG                = (0x002c1e02),  /*!<pad驱动能力调整 */


/*** Bit definition for [regIoDsCfg1] register(0x00000030) ****/
  RBIT_PD_GPIO_5_DS_REG                = (0x00300002),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_6_DS_REG                = (0x00300202),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_7_DS_REG                = (0x00300402),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_8_DS_REG                = (0x00300602),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_9_DS_REG                = (0x00300802),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_10_DS_REG               = (0x00300a02),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_11_DS_REG               = (0x00300c02),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_12_DS_REG               = (0x00300e02),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_13_DS_REG               = (0x00301002),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_14_DS_REG               = (0x00301202),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_15_DS_REG               = (0x00301402),  /*!<pad驱动能力调整 */
  RBIT_SIM0_RST_DS_REG                 = (0x00301602),  /*!<pad驱动能力调整 */
  RBIT_SIM0_CLK_DS_REG                 = (0x00301802),  /*!<pad驱动能力调整 */
  RBIT_SIM0_DATA_DS_REG                = (0x00301a02),  /*!<pad驱动能力调整 */


/*** Bit definition for [regIoDsCfg2] register(0x00000034) ****/
  RBIT_RF_CONTROL_0_DS_REG             = (0x00340002),  /*!<pad驱动能力调整 */
  RBIT_RF_CONTROL_1_DS_REG             = (0x00340202),  /*!<pad驱动能力调整 */
  RBIT_RF_CONTROL_2_DS_REG             = (0x00340402),  /*!<pad驱动能力调整 */
  RBIT_RF_CONTROL_3_DS_REG             = (0x00340602),  /*!<pad驱动能力调整 */
  RBIT_RF_CONTROL_4_DS_REG             = (0x00340802),  /*!<pad驱动能力调整 */
  RBIT_RF_CONTROL_5_DS_REG             = (0x00340a02),  /*!<pad驱动能力调整 */
  RBIT_RF_CONTROL_6_DS_REG             = (0x00340c02),  /*!<pad驱动能力调整 */
  RBIT_RF_CONTROL_7_DS_REG             = (0x00340e02),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_16_DS_REG               = (0x00341002),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_17_DS_REG               = (0x00341202),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_18_DS_REG               = (0x00341402),  /*!<pad驱动能力调整 */
  RBIT_PD_GPIO_19_DS_REG               = (0x00341602),  /*!<pad驱动能力调整 */


/*** Bit definition for [regIoSrCfg0] register(0x00000038) ****/
  RBIT_SWD0_SWCLK_SR_REG               = (0x00380001),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SWD0_SWDIO_SR_REG               = (0x00380101),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_CLK_AU_MCLK_SR_REG              = (0x00380201),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SPI0_CS_SR_REG                  = (0x00380501),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SPI0_CLK_SR_REG                 = (0x00380601),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SPI0_D_0_SR_REG                 = (0x00380701),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SPI0_D_1_SR_REG                 = (0x00380801),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SPI0_D_2_SR_REG                 = (0x00380901),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SPI0_D_3_SR_REG                 = (0x00380a01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_0_SR_REG                = (0x00380b01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_1_SR_REG                = (0x00380c01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_2_SR_REG                = (0x00380d01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_3_SR_REG                = (0x00380e01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_4_SR_REG                = (0x00380f01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_5_SR_REG                = (0x00381001),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_6_SR_REG                = (0x00381101),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_7_SR_REG                = (0x00381201),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_8_SR_REG                = (0x00381301),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_9_SR_REG                = (0x00381401),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_10_SR_REG               = (0x00381501),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_11_SR_REG               = (0x00381601),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_12_SR_REG               = (0x00381701),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_13_SR_REG               = (0x00381801),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_14_SR_REG               = (0x00381901),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_15_SR_REG               = (0x00381a01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SIM0_RST_SR_REG                 = (0x00381b01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SIM0_CLK_SR_REG                 = (0x00381c01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_SIM0_DATA_SR_REG                = (0x00381d01),  /*!<pad slew rate:  0, fast   1,slow */


/*** Bit definition for [regIoSrCfg1] register(0x0000003c) ****/
  RBIT_RF_CONTROL_0_SR_REG             = (0x003c0001),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_RF_CONTROL_1_SR_REG             = (0x003c0101),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_RF_CONTROL_2_SR_REG             = (0x003c0201),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_RF_CONTROL_3_SR_REG             = (0x003c0301),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_RF_CONTROL_4_SR_REG             = (0x003c0401),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_RF_CONTROL_5_SR_REG             = (0x003c0501),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_RF_CONTROL_6_SR_REG             = (0x003c0601),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_RF_CONTROL_7_SR_REG             = (0x003c0701),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_16_SR_REG               = (0x003c0801),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_17_SR_REG               = (0x003c0901),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_18_SR_REG               = (0x003c0a01),  /*!<pad slew rate:  0, fast   1,slow */
  RBIT_PD_GPIO_19_SR_REG               = (0x003c0b01),  /*!<pad slew rate:  0, fast   1,slow */


} RBIT_PdPadCtrl;

#endif