/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_sysctrl.h
 *
 * @brief       实现系统控制配置接口
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team          创建
 ************************************************************************************
 */

#ifndef __DRV_SYSCTRL__
#define __DRV_SYSCTRL__

/************************************************************************************
 *                                 头文件定义
 ************************************************************************************/
#include "drv_common.h"

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define PD_SYSCTRL_ADDRBASE     BASE_PD_SYS_CTRL
#define PD_SYSCTRL_PSRAM_CFG    (BASE_PD_SYS_CTRL + 0x84)
#define AON_SYSCTRL_ADDRBASE    BASE_AON_SYS_CTRL
//use for psm
#define AON_SLOCK0_ADDR         (AON_SYSCTRL_ADDRBASE + 0x94)
#define AON_SLOCK1_ADDR         (AON_SYSCTRL_ADDRBASE + 0x98)

/******************************************************************************************************************
 *
 ******************************************************************************************************************/

typedef enum
{
    /*** Bit definition for [bootRamRomRemapCfg] register(0x00000000) ****/
    SYSCTRL_IRAM_IROM_REMAP_RAM0            = (0x00000001),  /*!<iram irom remap寄存器，1：ram，0：rom */
    SYSCTRL_BOOT_MODE                       = (0x00000403),  /*!<boot mode可读寄存器 */


    /*** Bit definition for [ramCfg0] register(0x00000008) ****/
    SYSCTRL_GLRF2PHS_RET1N                  = (0x00080001),  /*!<ram低功耗控制 */
    SYSCTRL_GLRF2PHS_EMAB                   = (0x00080403),  /*!<ram时序margin控制 */
    SYSCTRL_GLRF2PHS_EMASA                  = (0x00080801),  /*!<ram时序margin控制 */
    SYSCTRL_GLRF2PHS_EMAA                   = (0x00080c03),  /*!<ram时序margin控制 */
    SYSCTRL_GLRF2PHS_STOV                   = (0x00081001),  /*!<ram时序margin控制 */


    /*** Bit definition for [ramCfg1] register(0x0000000c) ****/
    SYSCTRL_GLRFSPHD_EMA                    = (0x000c0003),  /*!<ram时序margin控制 */
    SYSCTRL_GLRFSPHD_EMAW                   = (0x000c0402),  /*!<ram时序margin控制 */
    SYSCTRL_GLRFSPHD_EMAS                   = (0x000c0801),  /*!<ram时序margin控制 */
    SYSCTRL_GLRFSPHD_RET1N                  = (0x000c0c01),  /*!<ram时序margin控制 */
    SYSCTRL_GLRFSPHD_WABL                   = (0x000c1001),  /*!<ram时序margin控制 */
    SYSCTRL_GLRFSPHD_WABLM                  = (0x000c1402),  /*!<ram时序margin控制 */
    SYSCTRL_GLRFSPHD_RAWL                   = (0x000c1801),  /*!<ram时序margin控制 */
    SYSCTRL_GLRFSPHD_RAWLM                  = (0x000c1c02),  /*!<ram时序margin控制 */


    /*** Bit definition for [ramCfg2] register(0x00000010) ****/
    SYSCTRL_GLSRSPUHD_STOV                  = (0x00100001),  /*!<ram时序margin控制 */
    SYSCTRL_GLSRSPUHD_EMA                   = (0x00100803),  /*!<ram时序margin控制 */
    SYSCTRL_GLSRSPUHD_EMAW                  = (0x00100c02),  /*!<ram时序margin控制 */
    SYSCTRL_GLSRSPUHD_EMAS                  = (0x00101001),  /*!<ram时序margin控制 */
    SYSCTRL_GLSRSPUHD_RAWL                  = (0x00101401),  /*!<ram时序margin控制 */
    SYSCTRL_GLSRSPUHD_RAWLM                 = (0x00101802),  /*!<ram时序margin控制 */
    SYSCTRL_GLSRSPUHD_WABL                  = (0x00101c01),  /*!<ram时序margin控制 */
    SYSCTRL_GLSRSPUHD_WABLM                 = (0x00101e02),  /*!<ram时序margin控制 */


    /*** Bit definition for [ramCfg3] register(0x00000018) ****/
    SYSCTRL_IROM_SLP                        = (0x00180001),  /*!<irom低功耗控制，1：低功耗状态，0：正常状态 */
    SYSCTRL_IROM_RTSEL                      = (0x00180402),  /*!<ram时序margin控制 */
    SYSCTRL_IROM_PTSEL                      = (0x00180802),  /*!<ram时序margin控制 */
    SYSCTRL_IROM_TRB                        = (0x00180c02),  /*!<ram时序margin控制 */


    /*** Bit definition for [qosCfg0] register(0x00000020) ****/
    SYSCTRL_AWQOS_CP_MEM                    = (0x00200004),  /*!<axi qos配置 */
    SYSCTRL_ARQOS_CP_MEM                    = (0x00200404),  /*!<axi qos配置 */
    SYSCTRL_AWQOS_AP_MEM                    = (0x00200804),  /*!<axi qos配置 */
    SYSCTRL_ARQOS_AP_MEM                    = (0x00200c04),  /*!<axi qos配置 */
    SYSCTRL_AWQOS_EDCP                      = (0x00201004),  /*!<axi qos配置 */
    SYSCTRL_ARQOS_EDCP                      = (0x00201404),  /*!<axi qos配置 */
    SYSCTRL_AWQOS_EDMA                      = (0x00201804),  /*!<axi qos配置 */
    SYSCTRL_ARQOS_EDMA                      = (0x00201c04),  /*!<axi qos配置 */


    /*** Bit definition for [qosCfg1] register(0x00000024) ****/
    SYSCTRL_AWQOS_AP_SYS                    = (0x00240004),  /*!<axi qos配置 */
    SYSCTRL_ARQOS_AP_SYS                    = (0x00240404),  /*!<axi qos配置 */
    SYSCTRL_AWQOS_CP_SYS                    = (0x00240804),  /*!<axi qos配置 */
    SYSCTRL_ARQOS_CP_SYS                    = (0x00240c04),  /*!<axi qos配置 */
    SYSCTRL_AWQOS_DMA                       = (0x00241004),  /*!<axi qos配置 */
    SYSCTRL_ARQOS_DMA                       = (0x00241404),  /*!<axi qos配置 */
    SYSCTRL_AWQOS_HDLC                      = (0x00241804),  /*!<axi qos配置 */
    SYSCTRL_ARQOS_HDLC                      = (0x00241c04),  /*!<axi qos配置 */


    /*** Bit definition for [debugCfg] register(0x00000040) ****/
    SYSCTRL_DEBUG_TEST                      = (0x00400003),  /*!<debug test选择 */
    SYSCTRL_LB_DEBUG_TEST_PIN_OUT           = (0x0040100e),  /*!<debug test pin read */

    /*** Bit definition for [spiLcdCfg] register(0x00000048) ****/
    SYSCTRL_SPI_LCD_MODE_SEL                = (0x00480001),  /*!<0: 3 wire, 1: 4 wire */


    /*** Bit definition for [sysCfg0] register(0x0000004c) ****/
    SYSCTRL_USIM_CARD_IN                    = (0x004c0001),  /*!<usim卡插入，1：插入，0：不插入 */
    SYSCTRL_LB_LTE_RST_B_DIV                = (0x004c0401),  /*!<lte reset，0：reset，1：release reset */
    SYSCTRL_LB_LTE_RST_B_STANDBY            = (0x004c0501),  /*!<lte reset，0：reset，1：release reset */
    SYSCTRL_LB_LTE_RST_B_PD0                = (0x004c0601),  /*!<lte reset，0：reset，1：release reset */
    SYSCTRL_LB_LTE_RST_B_PD1                = (0x004c0701),  /*!<lte reset，0：reset，1：release reset */


    /*** Bit definition for [sysCfg1] register(0x00000050) ****/
    SYSCTRL_LB_EDCP_UL_STATE                = (0x00501001),  /*!<edcp ul state */
    SYSCTRL_LB_EDCP_EMAC_STATE              = (0x00501101),  /*!<edcp ema state */
    SYSCTRL_LB_EDCP_DL_STATE                = (0x00501201),  /*!<edcp dl state */


    /*** Bit definition for [xipCfg0] register(0x00000060) ****/
    SYSCTRL_XIP_BOOT                        = (0x00600001),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_QPI_EN              = (0x00600101),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_EXT_SPI             = (0x00600202),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_CMD_BYP             = (0x00600401),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_ADR_BYP             = (0x00600501),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_DMY_BYP             = (0x00600601),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_DAT_BYP             = (0x00600701),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_ADR_MIO             = (0x00600801),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_DAT_MIO             = (0x00600901),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_ADR_LEN             = (0x00600c02),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_WRAP_EN             = (0x00600e01),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_WRAP_LEN            = (0x00600f01),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_DMY_CYC             = (0x00601005),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_DMY_MOD             = (0x00601808),  /*!<XIP 配置寄存器 */


    /*** Bit definition for [xipCfg1] register(0x00000070) ****/
    SYSCTRL_XIP_FC_BOOT_MOD_BYP             = (0x00700001),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_CMD_CODE            = (0x00700808),  /*!<XIP 配置寄存器 */
    SYSCTRL_XIP_FC_BOOT_T_SHSL              = (0x00701003),  /*!<XIP 配置寄存器 */


    /*** Bit definition for [psramCfg] register(0x00000084) ****/
    SYSCTRL_CLK_PAD_CFG                     = (0x00840006),  /*!<psram pad控制 */
    SYSCTRL_AC_PAD_CFG                      = (0x00840806),  /*!<psram pad控制 */
    SYSCTRL_DX_PAD_CFG                      = (0x00841006),  /*!<psram pad控制 */
    SYSCTRL_PSRAM_ACG_BYP                   = (0x00841803),  /*!<psram acg bypass，1：bypass，0：是能自动门控功能 */
    SYSCTRL_PSRAM_IDDQ_EN                   = (0x00841c01),  /*!<psram iddq enable，1：iddq，0：normal */
    SYSCTRL_PSRAM_PHY_MODE                  = (0x00841d01),  /*!<psram phy mode，1：3208 mode,  0: 6408 mode */


    /*** Bit definition for [dmaCfg] register(0x00000090) ****/
    SYSCTRL_HDLC_LOG_SEL                    = (0x00900002),  /*!<hdlc吐log选择用的uart，0: lpuart, 1: uart0，2:uart2，3:gp spi。 */
    SYSCTRL_UART0_USE_HDLC                  = (0x00900201),  /*!<uart0是否用hdlc搬运，0：用dma搬运，1：用hdlc搬运 */
    SYSCTRL_LPUART_USE_HDLC                 = (0x00900301),  /*!<lp uart是否用hdlc搬运，0：用dma搬运，1：用hdlc搬运 */
    SYSCTRL_UART2_USE_HDLC                  = (0x00900401),  /*!<uart2是否用hdlc搬运，0：用dma搬运，1：用hdlc搬运 */
    SYSCTRL_GP_SSP_USE_HDLC                 = (0x00900501),  /*!<gp spi是否用hdlc搬运，0：用dma搬运，1：用hdlc搬运 */
    SYSCTRL_EN_BK2_N                        = (0x00900601),  /*!<ext pmu控制 */
} SYSCTRL_PdRegGen;

/******************************************************************************************************************
 *
 ******************************************************************************************************************/

typedef enum
{
    /*** Bit definition for [apBootAddrCfg] register(0x00000000) ****/
    SYSCTRL_AP_CPU_RESET_VECTOR             = (0x00000020),  /*!<ap cpu启动地址 */


    /*** Bit definition for [cpBootAddrCfg] register(0x00000004) ****/
    SYSCTRL_CP_CPU_RESET_VECTOR             = (0x00040020),  /*!<cp cpu启动地址 */


    /*** Bit definition for [bootRamRomRemapCfg] register(0x00000008) ****/
    SYSCTRL_IRAM_IROM_REMAP                 = (0x00080001),  /*!<iram irom remap寄存器，1：ram，0：rom */


    /*** Bit definition for [regSharedDevice0] register(0x00000094) ****/
    SYSCTRL_REG_SHARED_DEVICE0              = (0x00940020),  /*!<互斥寄存器1 */


    /*** Bit definition for [regSharedDevice1] register(0x00000098) ****/
    SYSCTRL_REG_SHARED_DEVICE1              = (0x00980020),  /*!<互斥寄存器2 */


    /*** Bit definition for [regPdcoreCfg] register(0x0000009c) ****/
    SYSCTRL_REG_CP_RESET_N                  = (0x009c0001),  /*!<CP 软件复位 */
    SYSCTRL_BUFFER_CORE_EN                  = (0x009c0101),  /*!<PSRAM buffer core 使能  */


    /*** Bit definition for [bgConfig] register(0x00000134) ****/
    SYSCTRL_DIG_BANDGAP_ITRIM               = (0x01340005),  /*!<bandgap PTAT current bias trim,step:0.3125uA
                                                                10000:14.6875uA
                                                                ...
                                                                11111:10.3125uA
                                                                00000:10uA(default)
                                                                ...
                                                                01111:5uA */
    SYSCTRL_DIG_BANDGAP_LPF_FASTCHARGE_EN   = (0x01340501),  /*!<bandgap low pass filter fast charge enable, active high */
    SYSCTRL_DIG_BANDGAP_RTRIM               = (0x01340805),  /*!<bandgap resitance trim for ZTC current bias trim，Iztc=0.8/((31+rtim)*2600)
                                                                10000:7.889uA
                                                                ...
                                                                11111:9.615uA
                                                                00000:9.925uA(default)
                                                                ...
                                                                01111:13.378uA */
    SYSCTRL_DIG_BANDGAP_TUNE_R              = (0x01340e03),  /*!<temperature drift tuner
                                                                100:-32mV
                                                                101:-24mV
                                                                110:-16mV
                                                                100:-8mV
                                                                000:0V(default)
                                                                001:+8mV
                                                                010:+16mV
                                                                011:+24mV */
    SYSCTRL_DIG_BANDGAP_VBG_0P8_TRIM        = (0x01341104),  /*!<bandgap output 0.8V trim
                                                                1000:0.72V
                                                                1001:0.73V
                                                                1010:0.74V
                                                                1011:0.75V
                                                                1100:0.76V
                                                                1101:0.77V
                                                                1110:0.78V
                                                                1111:0.79V
                                                                0000:0.8V(default)
                                                                0001:0.81V
                                                                0010:0.82V
                                                                0011:0.83V
                                                                0100:0.84V
                                                                0101:0.85V
                                                                0110:0.86V
                                                                0111:0.87V */
    SYSCTRL_DIG_BANDGAP_VBG_1P2_TRIM        = (0x01341504),  /*!<bandgap output 1.2V trim
                                                                1000:1.12V
                                                                1001:1.13V
                                                                1010:1.14V
                                                                1011:1.15V
                                                                1100:1.16V
                                                                1101:1.17V
                                                                1110:1.18V
                                                                1111:1.19V
                                                                0000:1.2V(default)
                                                                0001:1.21V
                                                                0010:1.22V
                                                                0011:1.23V
                                                                0100:1.24V
                                                                0101:1.25V
                                                                0110:1.26V
                                                                0111:1.27V */
    SYSCTRL_DIG_LP_BANDGAP_LDO_OK           = (0x01341a01),  /*!<low power bandgap ldo ok */
    SYSCTRL_DIG_LP_BANDGAP_VTRIM            = (0x01341b03),  /*!<low power bandgap voltage trim
                                                                100:0.72V
                                                                101:0.74V
                                                                110:0.76V
                                                                111:0.78V
                                                                000:0.8V(default)
                                                                001:0.82V
                                                                010:0.84V
                                                                011:0.86V */
    SYSCTRL_DIG_LP_BANDGAP_ITRIM_FOR_OSC    = (0x01341e01),  /*!<low power bandgap current trim for osc
    0:1uA(default)
    1:1.25uA */


    /*** Bit definition for [bandgapEnable] register(0x00000138) ****/
    SYSCTRL_DIG_SLEEP_MODE_EN               = (0x01380201),  /*!<sleep mode enable, active high */
    SYSCTRL_SW_SLEEP_MODE_EN_SEL            = (0x01380301),  /*!<sleep mode enable select, 1 sw ,0 hw */
    SYSCTRL_DIG_BANDGAP_PD                  = (0x01380401),  /*!<rf bandgap power down, 1: power down; 0: power on */


    /*** Bit definition for [bandgapAtst] register(0x0000013c) ****/
    SYSCTRL_DIG_BANDGAP_ATST_EN             = (0x013c0001),  /*!<bandgap atst enable, active high */


    /*** Bit definition for [dcxoCoreConfig0] register(0x00000140) ****/
    SYSCTRL_DIG_DCXO_CT_GM_TUNE_EN          = (0x01400001),  /*!<dcxo gm tunning */
    SYSCTRL_DIG_DCXO_FBRES_EN               = (0x01400101),  /*!<dcxo feedback resistor enable. 1: enable; 0 disable */
    SYSCTRL_DIG_DCXO_FASTSET                = (0x01400201),  /*!<dcxo feedback resistor bypass. 1: bypass; */
    SYSCTRL_DIG_DCXO_VCOMP_LOW_SEL          = (0x01400803),  /*!<low detect voltage select */
    SYSCTRL_DIG_DCXO_CORE_GM                = (0x01401005),  /*!<dcxo gm manual tune value;00000:min.;11111:max */
    SYSCTRL_DIG_DCXO_GMCAL_CLK_SEL          = (0x01401803),  /*!<dcxo gm calibration clock cycle time select;000:9.84us;001:19.68us;010:39.36us;011:78.72us;100:157.44us;101:314.88us;110:629.76us;111:1259us; */


    /*** Bit definition for [dcxoBufferEnable] register(0x00000144) ****/
    SYSCTRL_DIG_DCXO_DIG_CLK_EN             = (0x01440101),  /*!<dcxo to digital clock buffer enable. 1: enable; 0 disable */
    SYSCTRL_DIG_DCXO_AUXADC_CLK_EN          = (0x01440201),  /*!<dcxo to auxadc buffer enable. 1: enable; 0 disable */
    SYSCTRL_DIG_DCXO_GMCAL_EN               = (0x01440401),  /*!<dcxo gm calibration station enable;1:enable;0:disable */
    SYSCTRL_DIG_DCXO_GM_MANUAL_EN           = (0x01440501),  /*!<dcxo gm manual tune enable;1:enable;0:disable */


    /*** Bit definition for [dcxoBufferConfig] register(0x00000148) ****/
    SYSCTRL_DIG_DCXO_BUF_STR                = (0x01480003),  /*!<digital clock buffer strength control;001:min.;111:max. */
    SYSCTRL_DIG_DCXO_AUXADC_CLK_STR         = (0x01480403),  /*!<dcxo to auxadc clock buffer strength control;001:min.;111:max. */
    SYSCTRL_DIG_DCXO_32KLESS_CLK_STR        = (0x01480803),  /*!<dcxo 32kless clock buffer strength control;001:min.;111:max. */
    SYSCTRL_DIG_DCXO_AUXADC_CLK_SEL         = (0x01480c02),  /*!<aux adc clock divider:00:26M;01:13M;10:6.5M;11:3.25M */


    /*** Bit definition for [dcxoGmcalBack] register(0x0000014c) ****/
    SYSCTRL_LOGIC_DCXO_GMCAL_OVERFLOW       = (0x014c0001),  /*!<dcxo gm calibration overflow indication;1:overflow */
    SYSCTRL_LOGIC_DCXO_GMCAL_DONE           = (0x014c0101),  /*!<dcxo gm calibration done;1:cal. done */
    SYSCTRL_LOGIC_DCXO_GMCAL_BACK           = (0x014c0405),  /*!<dcxo gm calibration value return;00000:min.;11111:max. */


    /*** Bit definition for [dcxoRegulConfig] register(0x00000150) ****/
    SYSCTRL_DIG_TX_PLL_LOOP_REGUL_IBIAS_CTRL1= (0x01500002),  /*!<dcxo&tx pll loop regulator ibias control1;no used */
    SYSCTRL_DIG_TX_PLL_LOOP_REGUL_IBIAS_CTRL2= (0x01500202),  /*!<dcxo&tx pll loop regulator ibias control2;no used */
    SYSCTRL_DIG_TX_PLL_LOOP_REGUL_VOUT_SEL  = (0x01500803),  /*!<dcxo&tx pll loop regulator voltage output control;step:0.025V;000:0.8V;001:0.825V;010:0.85V;011:0.875V;100:0.9V;101:0.925V;110:0.95V;112:0.975V */
    SYSCTRL_DIG_TX_PLL_LOOP_REGUL_BYPASS    = (0x01500c01),  /*!<dcxo&tx pll loop regulator bypass. 1: enable; 0 disable */
    SYSCTRL_DIG_TX_PLL_LOOP_REGUL_ATST_EN   = (0x01501001),  /*!<dcxo&tx pll regulator test enable: 1: enable; 0 disable */


    /*** Bit definition for [atstSel] register(0x00000158) ****/
    SYSCTRL_DIG_ATST_SEL                    = (0x01580002),  /*!<test point select */


    /*** Bit definition for [atstModeEnable] register(0x0000015c) ****/
    SYSCTRL_DIG_ATST_EN                     = (0x015c0001),  /*!<all test function is disabled */
    SYSCTRL_DIG_ATST_THROUGH_MODE_EN        = (0x015c0101),  /*!<pass mode enable */
    SYSCTRL_DIG_ATST_DIG_MODE_EN            = (0x015c0201),  /*!<digital test mode enable */
    SYSCTRL_DIG_ATST_BUF_MODE_EN            = (0x015c0301),  /*!<buffer mode enable */
    SYSCTRL_DIG_ATST_1M_LOAD_EN             = (0x015c0401),  /*!<the circuit is used to driver 1M ohm//10pF，otherwise,the load is 10k ohm//10pF */


    /*** Bit definition for [dcxoRefbufferEnable] register(0x00000160) ****/
    SYSCTRL_DIG_DCXO_RXPLL_REFCLK_EN        = (0x01600001),  /*!<dcxo to rx pll buffer enable. 1: enable; 0 disable */
    SYSCTRL_DIG_DCXO_TXPLL_REFCLK_EN        = (0x01600101),  /*!<dcxo to tx pll buffer enable. 1: enable; 0 disable */
    SYSCTRL_DIG_DCXO_AUXDAC_CLK_EN          = (0x01600201),  /*!<dcxo to auxdac buffer enable. 1: enable; 0 disable */
    SYSCTRL_DIG_DCXO_ABBPLL_CLK_EN          = (0x01600301),  /*!<dcxo to abbpll buffer enable. 1: enable; 0 disable */


    /*** Bit definition for [dcxoRefbufferConfig] register(0x00000164) ****/
    SYSCTRL_DIG_DCXO_RXPLL_REFCLK_STR       = (0x01640003),  /*!<dcxo to rx pll clock buffer strength control;001:min.;111:max. */
    SYSCTRL_DIG_DCXO_TXPLL_REFCLK_STR       = (0x01640403),  /*!<dcxo to tx pll clock buffer strength control;001:min.;111:max. */
    SYSCTRL_DIG_DCXO_AUXDAC_CLK_STR         = (0x01640803),  /*!<dcxo to auxdac clock buffer strength control;001:min.;111:max. */
    SYSCTRL_DIG_DCXO_ABBPLL_CLK_STR         = (0x01640c03),  /*!<dcxo to abbpll clock buffer strength control;001:min.;111:max. */


    /*** Bit definition for [dcxoSpareBits] register(0x00000168) ****/
    SYSCTRL_DIG_DCXO_SPAREBITS0             = (0x01680010),  /*!<dcxo spare bits register */
    SYSCTRL_DIG_DCXO_SPAREBITS1             = (0x01681010),  /*!<dcxo spare bits register */


    /*** Bit definition for [dcxoFrequencyCalibrtion0] register(0x0000016c) ****/
    SYSCTRL_DIG_DCXO_COARSE_TUNE            = (0x016c0007),  /*!<dcxo frequency coarse tune;0000000:min.;1111111:max. */


    /*** Bit definition for [regIoPeTestCfg0] register(0x00000240) ****/
    SYSCTRL_AON_GPIO_0_PE_TEST_REG          = (0x02400001),  /*!<pad上下拉使能，1：使能，0：不使能 */
    SYSCTRL_AON_GPIO_1_PE_TEST_REG          = (0x02400101),  /*!<pad上下拉使能，1：使能，0：不使能 */
    SYSCTRL_CLK_EXT_OUT_PE_TEST_REG         = (0x02400401),  /*!<pad上下拉使能，1：使能，0：不使能 */
    SYSCTRL_LPUART0_RX_PE_TEST_REG          = (0x02400501),  /*!<pad上下拉使能，1：使能，0：不使能 */
    SYSCTRL_LPUART0_TX_PE_TEST_REG          = (0x02400601),  /*!<pad上下拉使能，1：使能，0：不使能 */
    SYSCTRL_LPUART0_CTS_PE_TEST_REG         = (0x02400701),  /*!<pad上下拉使能，1：使能，0：不使能 */
    SYSCTRL_LPUART0_RTS_PE_TEST_REG         = (0x02400801),  /*!<pad上下拉使能，1：使能，0：不使能 */
    SYSCTRL_LPUART0_CTS_TEST_MODE_IE        = (0x02400a01),  /*!< */
    SYSCTRL_AON_GPIO_7_PE_TEST_REG          = (0x02400b01),  /*!<pad上下拉使能，1：使能，0：不使能 */
    SYSCTRL_AON_GPIO_8_PE_TEST_REG          = (0x02400c01),  /*!<pad上下拉使能，1：使能，0：不使能 */


    /*** Bit definition for [regIoPsTestCfg0] register(0x00000248) ****/
    SYSCTRL_AON_GPIO_0_PS_TEST_REG          = (0x02480001),  /*!<pad上下拉功能，1：上拉，0：下拉 */
    SYSCTRL_AON_GPIO_1_PS_TEST_REG          = (0x02480101),  /*!<pad上下拉功能，1：上拉，0：下拉 */
    SYSCTRL_CLK_EXT_OUT_PS_TEST_REG         = (0x02480401),  /*!<pad上下拉功能，1：上拉，0：下拉 */
    SYSCTRL_LPUART0_RX_PS_TEST_REG          = (0x02480501),  /*!<pad上下拉功能，1：上拉，0：下拉 */
    SYSCTRL_LPUART0_TX_PS_TEST_REG          = (0x02480601),  /*!<pad上下拉功能，1：上拉，0：下拉 */
    SYSCTRL_LPUART0_CTS_PS_TEST_REG         = (0x02480701),  /*!<pad上下拉功能，1：上拉，0：下拉 */
    SYSCTRL_LPUART0_RTS_PS_TEST_REG         = (0x02480801),  /*!<pad上下拉功能，1：上拉，0：下拉 */
    SYSCTRL_AON_GPIO_7_PS_TEST_REG          = (0x02480b01),  /*!<pad上下拉功能，1：上拉，0：下拉 */
    SYSCTRL_AON_GPIO_8_PS_TEST_REG          = (0x02480c01),  /*!<pad上下拉功能，1：上拉，0：下拉 */


    /*** Bit definition for [regIoDsTestCfg0] register(0x00000250) ****/
    SYSCTRL_AON_GPIO_0_DS_TEST_REG          = (0x02500002),  /*!<pad驱动能力调整 */
    SYSCTRL_AON_GPIO_1_DS_TEST_REG          = (0x02500202),  /*!<pad驱动能力调整 */
    SYSCTRL_CLK_EXT_OUT_DS_TEST_REG         = (0x02500802),  /*!<pad驱动能力调整 */
    SYSCTRL_LPUART0_RX_DS_TEST_REG          = (0x02500a02),  /*!<pad驱动能力调整 */
    SYSCTRL_LPUART0_TX_DS_TEST_REG          = (0x02500c02),  /*!<pad驱动能力调整 */
    SYSCTRL_LPUART0_CTS_DS_TEST_REG         = (0x02500e02),  /*!<pad驱动能力调整 */
    SYSCTRL_LPUART0_RTS_DS_TEST_REG         = (0x02501002),  /*!<pad驱动能力调整 */
    SYSCTRL_AON_GPIO_7_DS_TEST_REG          = (0x02501402),  /*!<pad驱动能力调整，0:4ma，1:8ma，2:12ma，3:16ma */
    SYSCTRL_AON_GPIO_8_DS_TEST_REG          = (0x02501602),  /*!<pad驱动能力调整，0:4ma，1:8ma，2:12ma，3:17ma */


    /*** Bit definition for [regAonCfg] register(0x00000260) ****/
    SYSCTRL_EN_BK1_N_TEST                   = (0x02600001),  /*!<dft测试信号 */
    SYSCTRL_FPWM_BK1_TEST                   = (0x02600101),  /*!<dft测试信号 */
    SYSCTRL_LPUART0_CTS_OEN_TEST            = (0x02600201),  /*!<dft测试信号 */


    /*** Bit definition for [regRfControl] register(0x00000308) ****/
    SYSCTRL_REG_DCXO_LDO_EN_LV              = (0x03080001),  /*!<软件控制dcxo_ldo_en */
    SYSCTRL_REG_RF_LPBGP_EN_LV              = (0x03080101),  /*!<软件控制rf_lpbgp_en */
    SYSCTRL_REG_RF_BGP_EN_LV                = (0x03080201),  /*!<软件控制rf_bgp_en */
    SYSCTRL_REG_ABBPLL_LDO_EN_LV            = (0x03080301),  /*!<软件控制abbpll_ldo_en */
    SYSCTRL_REG_DCXO_BUFFER_EN              = (0x03080401),  /*!<软件控制dcxo_buffer_en */
    SYSCTRL_REG_TX_PLL_LOOP_REGUL_ECO       = (0x03080b01),  /*!<软件控制tx_pll_loop_regul_eco */
    SYSCTRL_REG_DCXO_VCOMP_HIGH_SEL         = (0x03080d03),  /*!<软件控制dcxo_vcomp_high_sel */
    SYSCTRL_REG_DCXO_GMCAL_HOLDON_PULSE     = (0x03081101),  /*!<软件控制dcxo_gmcal_holdon_pulse */


    /*** Bit definition for [dcdc2Enable] register(0x00000310) ****/
    SYSCTRL_DCDC2_EN                        = (0x03100001),  /*!<dcdc2 enable, active high */
    SYSCTRL_DCDC2_SLEEP_EN                  = (0x03100101),  /*!<dcdc2 sleep enable,active high */
    SYSCTRL_DCDC2_SLEEP_MODE                = (0x03100201),  /*!<dcdc2 sleep mode select,active high, 1:保持电压，降低负载能力； 0:关闭输出 */
    SYSCTRL_DCDC2_DISCHARGE_EN              = (0x03100301),  /*!<dcdc2 discharge enable,active high, 1:discharge enable */
    SYSCTRL_DCDC2_INTEST                    = (0x03100401),  /*!<dcdc2 test mode enable,active high, 1:test mode enable */
    SYSCTRL_DCDC2_MODE                      = (0x03100501),  /*!<dcdc2 mode select, 1:PWM/PFM自动切换；0:强制PWM */


    /*** Bit definition for [dcdc2Trim] register(0x00000314) ****/
    SYSCTRL_DCDC2_TRIM_OCP                  = (0x03141003),  /*!<dcdc2 over current protect trim */


    /*** Bit definition for [dcdc2Test] register(0x00000318) ****/
    SYSCTRL_DCDC2_TESTE                     = (0x03180003),  /*!<dcdc2 test select */
    SYSCTRL_DCDC2_SPAREBITS                 = (0x03180410),  /*!<sparebit */


    /*** Bit definition for [lppmuOscLdo2Config] register(0x00000328) ****/
    SYSCTRL_DIG_LPPMU_LDO2_ECO_MODE         = (0x03280001),  /*!<ldo2 eco_mode: 1: keep output ; 0 : close ldo */
    SYSCTRL_DIG_LPPMU_LDO2_EN               = (0x03280101),  /*!<ldo2 enable : 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO2_ECO              = (0x03280301),  /*!<ldo2 eco mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO2_DISCHARGE        = (0x03280401),  /*!<ldo2 discharge mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO2_TRIM             = (0x03280604),  /*!<ldo2 voltage output trim;step:40mV;0000:1.48V;0001:1.52V;0010:1.56V;0011:1.6V;0100:1.64V;0101:1.68V;0110:1.72V;
    0111:1.76V;1000:1.8V;1001:1.84V;1010:1.88V;1011:1.92V;1100:1.96V;1101:2V;1110:2.04V;1111:2.08V */


    /*** Bit definition for [lppmuOscLdo3Config] register(0x0000032c) ****/
    SYSCTRL_DIG_LPPMU_LDO3_ECO_MODE         = (0x032c0001),  /*!<ldo3 eco_mode: 1: keep output ; 0 : close ldo */
    SYSCTRL_DIG_LPPMU_LDO3_EN               = (0x032c0101),  /*!<ldo3 enable : 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO3_ECO              = (0x032c0301),  /*!<ldo3 eco mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO3_DISCHARGE        = (0x032c0401),  /*!<ldo3 discharge mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO3_TRIM             = (0x032c0604),  /*!<ldo3 voltage output trim;step:40mV;0000:1.48V;0001:1.52V;0010:1.56V;0011:1.6V;0100:1.64V;0101:1.68V;0110:1.72V;
    0111:1.76V;1000:1.8V;1001:1.84V;1010:1.88V;1011:1.92V;1100:1.96V;1101:2V;1110:2.04V;1111:2.08V */


    /*** Bit definition for [lppmuOscLdo4Config] register(0x00000330) ****/
    SYSCTRL_DIG_LPPMU_LDO4_ECO_MODE         = (0x03300001),  /*!<ldo4 eco_mode: 1: keep output ; 0 : close ldo */
    SYSCTRL_DIG_LPPMU_LDO4_EN               = (0x03300101),  /*!<ldo4 enable : 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO4_ECO              = (0x03300301),  /*!<ldo4 eco mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO4_DISCHARGE        = (0x03300401),  /*!<ldo4 discharge mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO4_TRIM             = (0x03300604),  /*!<ldo4 voltage output trim;step:25mV;0000:0.7V;0001:0.725V;0010:0.75V;0011:0.775V;0100:0.8V;0101:0.825V;0110:0.85V;0111:0.875V;1000:0.9V;1001:0.925V;1010:0.95V;1011:0.975V;1100:1V;1101:1.025V;1110:1.05V;1111:1.075V */


    /*** Bit definition for [lppmuOscLdo5Config] register(0x00000334) ****/
    SYSCTRL_DIG_LPPMU_LDO5_ECO_MODE         = (0x03340001),  /*!<ldo5 eco_mode: 1: keep output ; 0 : close ldo */
    SYSCTRL_DIG_LPPMU_LDO5_EN               = (0x03340101),  /*!<ldo5 enable : 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO5_ECO              = (0x03340301),  /*!<ldo5 eco mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO5_DISCHARGE        = (0x03340401),  /*!<ldo5 discharge mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO5_TRIM             = (0x03340604),  /*!<ldo5 voltage output trim;step:50mV;0000:2.1V;0001:2.15V;0010:2.2V;0011:2.25V;0100:2.3V;0101:2.35V;0110:2.4V;0111:2.45V;1000:2.5V;1001:2.55V;1010:2.6V;1011:2.65V;1100:2.7V;1101:2.75V;1110:2.8V;1111:2.85V */


    /*** Bit definition for [lppmuOscLdo6Config] register(0x00000338) ****/
    SYSCTRL_DIG_LPPMU_LDO6_ECO_MODE         = (0x03380001),  /*!<ldo6 eco_mode: 1: keep output ; 0 : close ldo */
    SYSCTRL_DIG_LPPMU_LDO6_EN               = (0x03380101),  /*!<ldo6 enable : 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO6_ECO              = (0x03380301),  /*!<ldo6 eco mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO6_DISCHARGE        = (0x03380401),  /*!<ldo6 discharge mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO6_TRIM             = (0x03380604),  /*!<ldo6 voltage output trim;step:25mV;0000:0.7V;0001:0.725V;0010:0.75V;0011:0.775V;0100:0.8V;0101:0.825V;0110:0.85V;0111:0.875V;1000:0.9V;1001:0.925V;1010:0.95V;1011:0.975V;1100:1V;1101:1.025V;1110:1.05V;1111:1.075V */


    /*** Bit definition for [lppmuOscLdo7Config] register(0x0000033c) ****/
    SYSCTRL_DIG_LPPMU_LDO7_ECO_MODE         = (0x033c0001),  /*!<ldo7 eco_mode: 1: keep output ; 0 : close ldo */
    SYSCTRL_DIG_LPPMU_LDO7_EN               = (0x033c0101),  /*!<ldo7 enable : 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO7_ECO              = (0x033c0301),  /*!<ldo7 eco mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO7_DISCHARGE        = (0x033c0401),  /*!<ldo7 discharge mode: 1: enable; 0 disable */


    /*** Bit definition for [lppmuOscLdo8Config] register(0x00000340) ****/
    SYSCTRL_DIG_LPPMU_LDO8_ECO_MODE         = (0x03400001),  /*!<ldo8 eco_mode: 1: keep output ; 0 : close ldo */
    SYSCTRL_DIG_LPPMU_LDO8_EN               = (0x03400101),  /*!<ldo8 enable : 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO8_ECO              = (0x03400301),  /*!<ldo8 eco mode: 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_LDO8_DISCHARGE        = (0x03400401),  /*!<ldo8 discharge mode: 1: enable; 0 disable */


    /*** Bit definition for [lppmuOscTestConfig] register(0x0000034c) ****/
    SYSCTRL_DIG_LPPMU_TEST_EN               = (0x034c0001),  /*!<test enable : 1: enable; 0 disable */
    SYSCTRL_DIG_LPPMU_TEST_SEL              = (0x034c0202),  /*!<test analog sign sel;00:bandgap vref0p6;01:opm v26_on;10:bandgap ib_125na;11:TBD */


    /*** Bit definition for [lppmuOscOk] register(0x00000350) ****/
    SYSCTRL_LDO1_OK                         = (0x03500001),  /*!<LDO1 上电完成标志 */
    SYSCTRL_LDO2_OK                         = (0x03500101),  /*!<LDO2 上电完成标志 */
    SYSCTRL_LDO3_OK                         = (0x03500201),  /*!<LDO3 上电完成标志 */
    SYSCTRL_LDO4_OK                         = (0x03500301),  /*!<LDO4 上电完成标志 */
    SYSCTRL_LDO5_OK                         = (0x03500401),  /*!<LDO5 上电完成标志 */
    SYSCTRL_LDO6_OK                         = (0x03500501),  /*!<LDO6 上电完成标志 */
    SYSCTRL_LDO7_OK                         = (0x03500601),  /*!<LDO7 上电完成标志 */
    SYSCTRL_LDO8_OK                         = (0x03500701),  /*!<LDO8 上电完成标志 */
    SYSCTRL_LDO7_BYPASS_OK_LV               = (0x03500801),  /*!<ldo7 bypass模式标志 */
    SYSCTRL_LDO8_BYPASS_OK_LV               = (0x03500901),  /*!<ldo8 bypass模式标志 */


    /*** Bit definition for [rc32kTrimCfg1] register(0x00000474) ****/
    SYSCTRL_REG_RC_32K_TRIM_MODE            = (0x04740001),  /*!<32K RC校准寄存器 */
    SYSCTRL_REG_RC_32K_AUTO_CTRL_EN         = (0x04740101),  /*!<32K RC校准寄存器 */
    SYSCTRL_REG_RC_32K_TRIM_RSTN            = (0x04741201),  /*!<32K RC校准寄存器 */


    /*** Bit definition for [rc32kTrimCfg2] register(0x00000478) ****/
    SYSCTRL_REG_RC_32K_FREQ_FTUNE_LV        = (0x04780004),  /*!<32K RC校准寄存器 */
    SYSCTRL_REG_RC_32K_FREQ_CTUNE_LV        = (0x04780406),  /*!<32K RC校准寄存器 */


    /*** Bit definition for [rc32kTrimCfg3] register(0x0000047c) ****/
    SYSCTRL_REG_SOFT_COUNTER_EN             = (0x047c1401),  /*!<RC 32K 校准寄存器 */
    SYSCTRL_REG_COUNTER_NUM                 = (0x047c1508),  /*!<RC 32K 校准寄存器 */
    SYSCTRL_REG_SOFT_COUNTER_FINISH         = (0x047c1d01),  /*!<RC 32K 校准寄存器 */
    SYSCTRL_RC_32K_TRIM_STATE               = (0x047c1e01),  /*!<RC 32K 校准寄存器 */


    /*** Bit definition for [rc32kTrimCfg4] register(0x00000480) ****/
    SYSCTRL_REG_SOFT_COUNTER                = (0x04800020),  /*!<RC 32K 校准寄存器 */


    /*** Bit definition for [otwReg] register(0x00000490) ****/
    SYSCTRL_OTW_JTAG_EN                     = (0x04900001),  /*!<RC 32K 校准寄存器 */
    SYSCTRL_OTW_RSVED                       = (0x0490011f),  /*!<RC 32K 校准寄存器 */
} SYSCTRL_AonRegGen;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief           设置 sysctrl配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
void SYSCTRL_SetPdRegGen(SYSCTRL_PdRegGen regBit, uint32_t bitVal);

/**
 ************************************************************************************
 * @brief           获取 sysctrl配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
uint32_t SYSCTRL_GetPdRegGen(SYSCTRL_PdRegGen regBit);

/**
 ************************************************************************************
 * @brief           设置 aon sysctrl配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
void SYSCTRL_SetAonRegGen(SYSCTRL_AonRegGen regBit, uint32_t bitVal);

/**
 ************************************************************************************
 * @brief           获取 aon sysctrl配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
uint32_t SYSCTRL_GetAonRegGen(SYSCTRL_AonRegGen regBit);

#endif
