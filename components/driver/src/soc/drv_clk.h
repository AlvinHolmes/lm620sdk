/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_clk.h
 *
 * @brief       实现时钟配置接口
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team          创建
 ************************************************************************************
 */

#ifndef __DRV_CLK__
#define __DRV_CLK__
/************************************************************************************
 *                                 头文件定义
 ************************************************************************************/
#include "drv_common.h"
#ifdef OS_USING_PM
#include <psm_common.h>
#endif
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define TOP_CRM_ADDRBASE        BASE_TOP_CRM
#define SPLL0_LOCK_CSR          (TOP_CRM_ADDRBASE + 0x0)
#define PLL_FOUT_SWAP_DONE      (TOP_CRM_ADDRBASE + 0x4)
#define SPLL0_CTRL0             (TOP_CRM_ADDRBASE + 0x8)
#define SPLL0_CTRL1             (TOP_CRM_ADDRBASE + 0xC)
#define SW_CLK_EN               (TOP_CRM_ADDRBASE + 0x10)
#define SW_CLK_SEL              (TOP_CRM_ADDRBASE + 0x14)
#define CLK_OUT_CTRL            (TOP_CRM_ADDRBASE + 0x1C)
#define CLK_REQEN               (TOP_CRM_ADDRBASE + 0x24)
#define SW_RESET                (TOP_CRM_ADDRBASE + 0x28)
#define DEBOUNCE_COUNT_CFG      (TOP_CRM_ADDRBASE + 0x2C)
#define DIV_CLK_OUT_CTRL        (TOP_CRM_ADDRBASE + 0x30)
#define AIX_CLK_ICG_BYPASS      (TOP_CRM_ADDRBASE + 0x34)
#define ACG_ACS_CFG0            (TOP_CRM_ADDRBASE + 0x38)
#define ACG_ACS_CFG1            (TOP_CRM_ADDRBASE + 0x3C)
#define LTE_HIGH_LOCK_CSR       (TOP_CRM_ADDRBASE + 0x44)

#define SBY_CRM_ADDRBASE        BASE_SBY_LSP

#define AON_CRM_ADDRBASE        BASE_AON_CRM
#define PS_PWM_TIMER_PARA       (AON_CRM_ADDRBASE + 0x0)
#define CP_PWM_TIMER_PARA       (AON_CRM_ADDRBASE + 0x4)
#define CLK_RST_PARA            (AON_CRM_ADDRBASE + 0x8)

#define LSP_CRM_ADDRBASE        BASE_LSP_CRM
#define PD_LSP_CTL              (LSP_CRM_ADDRBASE + 0x0)
#define PD_SYSCTL_CTL           (LSP_CRM_ADDRBASE + 0x4)
#define PD_GPIO_CTL             (LSP_CRM_ADDRBASE + 0x8)
#define PD_IOPAD_CTL            (LSP_CRM_ADDRBASE + 0xC)
#define PAD_CTL                 (LSP_CRM_ADDRBASE + 0x10)
#define I2C0_CTL                (LSP_CRM_ADDRBASE + 0x14)
#define I2C1_CTL                (LSP_CRM_ADDRBASE + 0x18)
#define RFFE_SSC_CTL            (LSP_CRM_ADDRBASE + 0x20)
#define RF_CTL                  (LSP_CRM_ADDRBASE + 0x24)
#define UART0_CTL               (LSP_CRM_ADDRBASE + 0x28)
#define SPI_LCD_CTL             (LSP_CRM_ADDRBASE + 0x2C)
#define SPI_CAM_CTL             (LSP_CRM_ADDRBASE + 0x30)
#define SSP0_CTL                (LSP_CRM_ADDRBASE + 0x34)
#define I2S0_CTL                (LSP_CRM_ADDRBASE + 0x38)
#define I2S1_CTL                (LSP_CRM_ADDRBASE + 0x3C)
#define I2S2_CTL                (LSP_CRM_ADDRBASE + 0x40)
#define I2S3_CTL                (LSP_CRM_ADDRBASE + 0x44)
#define USIM_CTL                (LSP_CRM_ADDRBASE + 0x48)
#define PSRAM_CTL_RST           (LSP_CRM_ADDRBASE + 0x4C)
#define PSRAM_PHY_RST           (LSP_CRM_ADDRBASE + 0x50)
#define XIP_CTL_RST             (LSP_CRM_ADDRBASE + 0x54)
#define AUDIO_PWM_CTL           (LSP_CRM_ADDRBASE + 0x58)
#define UART2_CTL               (LSP_CRM_ADDRBASE + 0x5C)
#define AUDIO_PWM_DIV1          (LSP_CRM_ADDRBASE + 0x60)
#define AUDIO_PWM_DIV2          (LSP_CRM_ADDRBASE + 0x64)
#define UART3_CTL               (LSP_CRM_ADDRBASE + 0x68)
#define I2C2_CTL                (LSP_CRM_ADDRBASE + 0x6c)
#define I2C3_CTL                (LSP_CRM_ADDRBASE + 0x70)
#define SSP1_CTL                (LSP_CRM_ADDRBASE + 0x74)

#define SUBSYS_CRM_ADDRBASE     BASE_CPU_MEM_CRM
#define CPM_MEM_CLK_EN          (SUBSYS_CRM_ADDRBASE + 0x0)
#define CPM_MEM_RST_SW          (SUBSYS_CRM_ADDRBASE + 0x4)
#define CPM_MEM_ACG_CFG         (SUBSYS_CRM_ADDRBASE + 0x8)

#if defined(_CPU_AP)
#define CORE_CSR_ADDRBASE       BASE_APCPU_CSR
#else
#define CORE_CSR_ADDRBASE       BASE_CPCPU_CSR
#endif

#define AP_CSR_DBG              (BASE_APCPU_CSR)
#define AP_STATUS               (BASE_APCPU_CSR + 0x8)
#define AP_CLK_REQ              (BASE_APCPU_CSR + 0x100)
#define AP_TIMER0_CFG           (BASE_APCPU_CSR + 0x104)
#define AP_TIMER1_CFG           (BASE_APCPU_CSR + 0x108)
#define AP_WDT_CFG              (BASE_APCPU_CSR + 0x10C)
#define AP_BUS_GATE             (BASE_APCPU_CSR + 0x114)
#define AP_ICP_GFG              (BASE_APCPU_CSR + 0x118)
#define AP_SRSPHD_RAM_CFG       (BASE_APCPU_CSR + 0x11C)
#define AP_RFSPHD_RAM_CFG       (BASE_APCPU_CSR + 0x120)
#define AP_AXCACHE_OVERRIDE     (BASE_APCPU_CSR + 0x124)
#define AP_TRACE0_CAUSE         (BASE_APCPU_CSR + 0x128)
#define AP_TRACE0_TVAL          (BASE_APCPU_CSR + 0x12C)
#define AP_TRACE0_IADDR         (BASE_APCPU_CSR + 0x130)
#define AP_TRACE0_INSTR         (BASE_APCPU_CSR + 0x134)
#define AP_TRACE0_MISC          (BASE_APCPU_CSR + 0x138)
#define AP_TRACE1_CAUSE         (BASE_APCPU_CSR + 0x13c)
#define AP_TRACE1_TVAL          (BASE_APCPU_CSR + 0x140)
#define AP_TRACE1_IADDR         (BASE_APCPU_CSR + 0x144)
#define AP_TRACE1_INSTR         (BASE_APCPU_CSR + 0x148)
#define AP_TRACE1_MISC          (BASE_APCPU_CSR + 0x150)

#define CP_CSR_DBG              (BASE_CPCPU_CSR)
#define CP_STATUS               (BASE_CPCPU_CSR + 0x8)
#define CP_CLK_REQ              (BASE_CPCPU_CSR + 0x100)
#define CP_TIMER0_CFG           (BASE_CPCPU_CSR + 0x104)
#define CP_TIMER1_CFG           (BASE_CPCPU_CSR + 0x108)
#define CP_WDT_CFG              (BASE_CPCPU_CSR + 0x10C)
#define CP_BUS_GATE             (BASE_CPCPU_CSR + 0x114)
#define CP_ICP_GFG              (BASE_CPCPU_CSR + 0x118)
#define CP_SRSPHD_RAM_CFG       (BASE_CPCPU_CSR + 0x11C)
#define CP_RFSPHD_RAM_CFG       (BASE_CPCPU_CSR + 0x120)
#define CP_AXCACHE_OVERRIDE     (BASE_CPCPU_CSR + 0x124)
#define CP_TRACE0_CAUSE         (BASE_CPCPU_CSR + 0x128)
#define CP_TRACE0_TVAL          (BASE_CPCPU_CSR + 0x12C)
#define CP_TRACE0_IADDR         (BASE_CPCPU_CSR + 0x130)
#define CP_TRACE0_INSTR         (BASE_CPCPU_CSR + 0x134)
#define CP_TRACE0_MISC          (BASE_CPCPU_CSR + 0x138)
#define CP_TRACE1_CAUSE         (BASE_CPCPU_CSR + 0x13c)
#define CP_TRACE1_TVAL          (BASE_CPCPU_CSR + 0x140)
#define CP_TRACE1_IADDR         (BASE_CPCPU_CSR + 0x144)
#define CP_TRACE1_INSTR         (BASE_CPCPU_CSR + 0x148)
#define CP_TRACE1_MISC          (BASE_CPCPU_CSR + 0x150)

// TOP CRM CFG
#define CLK_EN_WIDTH            (1)
#define CLK_REQEN_WIDTH         (1)
#define CLK_RESET_WIDTH         (1)
#define CLK_ACS_EN_WIDTH        (1)
#define CLK_ACG_EN_WIDTH        (1)

//SW_CLK_EN
#define APCORE_CLK_EN_OFF       (0)
#define CPCORE_CLK_EN_OFF       (2)
#define MODEM_CLK_EN_OFF        (3)
#define PSRAM_PHYCTL_EN_OFF     (5)
#define XIP_SFC_CLK_EN_OFF      (7)
#define EDCP_ULDL_ACLK_EN_OFF   (10)
#define PSRAM_CACHE_CLK_EN_OFF  (12)
#define EDCP_HCLK_EN_OFF        (13)
#define EDCP_312M_CLK_EN_OFF    (19)
#define RFFE_SSC_WCLK_EN_OFF    (21)
#define RFFE_SSC_PCLK_EN_OFF    (22)
#define AU_MCLK_EN_OFF          (23)
#define CAM_MCLK_EN_OFF         (24)

//SW_CLK_SEL
#define APCORE_CLK_SEL_OFF      (0)
#define APCORE_CLK_SEL_WID      (2)
#define CPCORE_CLK_SEL_OFF      (8)
#define CPCORE_CLK_SEL_WID      (2)
#define PSRAM_PHYCTL_SEL_OFF    (12)
#define PSRAM_PHYCTL_SEL_WID    (3)
#define XIP_SFC_CLK_SEL_OFF     (16)
#define XIP_SFC_CLK_SEL_WID     (2)
#define APSS_AXI_SEL_OFF        (20)
#define APSS_AXI_SEL_WID        (2)
#define PSRAM_CACHE_CLK_SEL_OFF (24)
#define PSRAM_CACHE_CLK_SEL_WID (3)

//APDMA_CRM_CTRL
#define APDMA_PCLK_EN_OFF       (0)
#define APDMA_ACLK_EN_OFF       (1)
#define APDMA_PCLK_REQEN_OFF    (4)
#define APDMA_ACLK_REQEN_OFF    (5)
#define APDMA_PCLK_RESET_OFF    (8)
#define APDMA_ACLK_RESET_OFF    (9)

//CLK_OUT_CTRL
#define CAM_MCLK_SEL_OFF        (0)
#define CAM_MCLK_SEL_WID        (3)
#define AU_MCLK_SEL_OFF         (4)
#define AU_MCLK_SEL_WID         (3)
#define CLK26M_OUT_SEL_OFF      (8)
#define CLK26M_OUT_SEL_WID      (2)

//SW_RESET
#define APSS_LSP_SW_RESET_OFF   (8)

//DEBOUNCE_COUNT_CFG --- acg debounce计数器，当idle后经过该寄存器delay后关闭时钟
#define DEBOUNCE_COUNT_OFF      (0)
#define DEBOUNCE_COUNT_WID      (16)

//DIV_CLK_OUT_CTRL
#define CAM_MCLK_DIV_OFF        (0)
#define CAM_MCLK_DIV_WID        (2)
#define AU_MCLK_DIV_OFF         (4)
#define AU_MCLK_DIV_WID         (2)

//AIX_CLK_ICG_BYPASS --- axi bus自动门控bupass使能
#define AIX_CLK_ICG_BYPASS_OFF  (2)

//ACG_ACS_CFG0  ACS ---  自动调频       ACG ---  自动门控
#define APCORE_CLK_ACS_EN_OFF   (0)
#define CPCORE_CLK_ACS_EN_OFF   (1)
#define PSRAM_CCLK_ACS_EN_OFF   (2)
#define XIP_SFC_CLK_ACS_EN_OFF  (3)
#define APSS_AXI_CLK_ACS_EN_OFF (4)
#define PSRAM_PHYCTL_ACS_EN_OFF (5)
#define APCORE_CLK_ACG_EN_OFF   (6)
#define CPCORE_CLK_ACG_EN_OFF   (7)
#define APSS_AXI_CLK_ACG_EN_OFF (9)
#define PSRAM_CCLK_ACG_EN_OFF   (10)
#define PSRAM_PHYCTL_ACG_EN_OFF (11)
#define XIP_SFC_CLK_ACG_EN_OFF  (12)

//ACG_ACS_CFG1
#define APCORE_CLK_ACS_SEL_OFF  (0)
#define APCORE_CLK_ACS_SEL_WID  (3)
#define CPCORE_CLK_ACS_SEL_OFF  (4)
#define CPCORE_CLK_ACS_SEL_WID  (3)
#define PSRAM_CCLK_ACS_SEL_OFF  (8)
#define PSRAM_CCLK_ACS_SEL_WID  (3)
#define APSS_AXI_CLK_ACS_SEL_OFF (16)
#define APSS_AXI_CLK_ACS_SEL_WID (2)
#define PSRAM_PHYCTL_ACS_SEL_OFF (20)
#define PSRAM_PHYCTL_ACS_SEL_WID (3)
#define XIP_SFC_CLK_ACS_SEL_OFF (24)
#define XIP_SFC_CLK_ACS_SEL_WID (2)

//SBY_CLK_RST_PARA
#define PMU_PCLK_EN_OFF         (0)
#define PMU_WCLK_EN_OFF         (1)
#define PCU_SBY_PCLK_EN_OFF     (2)
#define PCU_SBY_WCLK_EN_OFF     (3)
#define SBY_SYSCTL_PCLK_EN_OFF  (4)
#define SBY_PDCTL_PCLK_EN_OFF   (5)
#define LTE_LPM_PCLK_EN_OFF     (6)
#define LTE_LPM_WCLK_EN_OFF     (7)
#define RC32K_TRIM_CLK_EN_OFF   (8)
#define RC26M_TRIM_CLK_EN_OFF   (9)
#define RC32K_TRIM_RESET_OFF    (10)
#define RTC_CLK_REQEN_OFF       (11)
#define CLK32K_EXTOUT_EN_OFF    (12)
#define PCU_SBY_PRESET_OFF      (13)
#define PCU_SBY_WRESET_OFF      (14)
#define SBY_SYSCTL_PRESET_OFF   (15)
#define SBY_PDCTL_PRESET_OFF    (16)
#define LTE_LPM_PRESET_OFF      (17)
#define LTE_LPM_WRESET_OFF      (18)

//SBY_CLKSEL_PARA
#define SBY_32K_CLK_SEL_OFF     (0)
#define SBY_32K_CLK_SEL_WID     (2)
#define CLK32K_EXTOUT_SEL_OFF   (2)
#define CLK32K_EXTOUT_SEL_WID   (1)

//LP_UART_REG
#define LPUART_PCLK_EN_OFF      (0)
#define LPUART_WCLK_EN_OFF      (1)
#define LPUART_CLK_SEL_OFF      (2)
#define LPUART_CLK_SEL_WID      (1)
#define LPUART_PRESET_OFF       (3)
#define LPUART_WRESET_OFF       (4)
#define LPUART_PCLK_REQEN_OFF   (5)
#define LPUART_WCLK_REQEN_OFF   (6)

//PS_PWM_TIMER_PARA
//CP_PWM_TIMER_PARA
#define PWM_PCLK_EN_OFF         (0)
#define PWM_WCLK_EN_OFF         (1)
#define PWM_CLK_SEL_OFF         (2)
#define PWM_CLK_SEL_WID         (1)
#define PWM_PRESET_OFF          (3)
#define PWM_WRESET_OFF          (4)
#define PWM_PCLK_REQEN_OFF      (5)
#define PWM_CLK_DIV_OFF         (6)
#define PWM_CLK_DIV_WID         (4)

//CLK_RST_PARA
#define AON26M_CLK_SEL_OFF      (0)
#define AON26M_CLK_SEL_WID      (1)
#define DCXO26M_DIV794_EN_OFF   (1)
#define AON_PCU_PCLK_EN_OFF     (2)
#define AON_PCU_PRESET_OFF      (3)
#define AON_PCU_WCLK_EN_OFF     (4)
#define AON_PCU_WRESET_OFF      (5)
#define AON_SYSCTL_PCLK_EN_OFF  (6)
#define AON_SYSCTL_PRESET_OFF   (7)
#define AON_PDCTL_PCLK_EN_OFF   (8)
#define AON_PDCTL_PRESET_OFF    (9)
#define AON_GPIO_PCLK_EN_OFF    (10)
#define AON_GPIO_PCLK_REQEN_OFF (11)
#define AON_GPIO_PRESET_OFF     (12)
#define APB_KEY_PCLK_EN_OFF     (13)
#define APB_KEY_PCLK_REQEN_OFF  (14)
#define APB_KEY_PCLK_RESET_OFF  (15)
#define APB_KEY_WCLK_EN_OFF     (16)
#define APB_KEY_WCLK_REQEN_OFF  (17)
#define APB_KEY_WCLK_RESET_OFF  (18)
#define RC26M_T_REFCLK_EN_OFF   (29)
#define RC26M_T_RCCLK_EN_OFF    (30)
#define CLK_EXT_OUT_SEL_OFF     (31)
#define CLK_EXT_OUT_SEL_WID     (1)

//PD_LSP_CTL
#define PD_LSP_RESET_OFF        (0)
//PD_SYSCTL_CTL
#define PD_SYSCTL_PRESET_OFF    (0)
//PD_GPIO_CTL
#define PD_GPIO_PRESET_OFF      (0)
#define PD_GPIO_PCLK_REQEN_OFF  (8)
//PD_IOPAD_CTL
#define PD_IOPAD_RESET_OFF      (0)
//PAD_CTL
#define PD_PAD_CTL_PRESET_OFF   (0)
//I2C0_CTL
//I2C1_CTL
#define I2C_PCLK_EN_OFF         (0)
#define I2C_WCLK_EN_OFF         (4)
#define I2C_CLK_SEL_OFF         (8)
#define I2C_CLK_SEL_WID         (1)
#define I2C_PRESET_OFF          (12)
#define I2C_WRESET_OFF          (16)
#define I2C_WCLK_REQEN_OFF      (20)
#define I2C_PCLK_REQEN_OFF      (24)
//RFFE_SSC_CTL
#define RFFE_SSC_PRESET_OFF     (0)
//RF_CTL
#define RF_CTRL_PRESET_OFF      (0)
//UART0_CTL
#define UART0_WCLK_SEL_OFF      (0)
#define UART0_WCLK_SEL_WID      (1)
#define UART0_PCLK_EN_OFF       (4)
#define UART0_WCLK_EN_OFF       (8)
#define UART0_PRESET_OFF        (12)
#define UART0_WRESET_OFF        (16)
#define UART0_PCLK_REQEN_OFF    (20)
#define UART0_WCLK_REQEN_OFF    (24)
//SPI_LCD_CTL
//SPI_CAM_CTL
//SSP0_CTL
#define SPI_PCLK_EN_OFF         (0)
#define SPI_WCLK_EN_OFF         (4)
#define SPI_PRESET_OFF          (8)
#define SPI_WRESET_OFF          (12)
#define SPI_WCLK_REQEN_OFF      (16)
#define SPI_PCLK_REQEN_OFF      (20)
#define SPI_WCLK_SEL_OFF        (24)
#define SPI_WCLK_SEL_WID        (2)
#define SSP0_DIV_OFF            (28)
#define SSP0_DIV_WID            (4)
//I2S0_CTL
#define I2S_WCLK_SEL_OFF        (0)
#define I2S_WCLK_SEL_WID        (2)
#define I2S_PCLK_EN_OFF         (4)
#define I2S_WCLK_EN_OFF         (8)
#define I2S_PRESET_OFF          (12)
#define I2S_WRESET_OFF          (16)
#define I2S_WCLK_REQEN_OFF      (20)
#define I2S_PCLK_REQEN_OFF      (24)
#define I2S_DIV_OFF1            (28)
#define I2S_DIV_WID1            (4)
//I2S1_CTL
#define I2S_DIV_OFF2            (0)
#define I2S_DIV_WID2            (6)
//I2S2_CTL
#define I2S_DIV1_OFF            (0)
#define I2S_DIV1_WID            (32)
//I2S3_CTL
#define I2S_DIV2_OFF            (0)
#define I2S_DIV2_WID            (17)

#define CPUMEMSYS_MASK          0x00C00FE0
#define SWCLK_MASK              0xBE7FFE7F
#define SWSEL_MASK              0xFFCFFFFF
#define AXISEL_26M              0x00300000
#define CPUMEMACG_MASK          0xFFEBFFFF

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum
{
    /*** Bit definition for [pdLspModuleCtrl] register(0x00000000) ****/
    CLK_PD_LSP_SW_RESETN                = (0x00000001),  /*!<pdcore lsp global software resetn 0-active */


    /*** Bit definition for [pdSysCtrlModuleCtrl] register(0x00000004) ****/
    CLK_PD_SYS_CTRL_SW_PRESETN          = (0x00040001),  /*!<pd_sys_ctrl apb software resetn 0-active */
    CLK_PD_SYS_CTRL_PCLK_SW_EN          = (0x00040401),  /*!<pd_sys_ctrl apb bus clk gate (0-close clk ,1-open clk)  */
    CLK_PD_SYS_CTRL_PCLK_REQEN          = (0x00040801),  /*!<pd_sys_ctrl apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */


    /*** Bit definition for [pdGpioModuleCtrl] register(0x00000008) ****/
    CLK_PD_GPIO_SW_PRESETN              = (0x00080001),  /*!<pd_gpio apb software resetn 0-active */
    CLK_PD_GPIO_PCLK_SW_EN              = (0x00080401),  /*!<pd_gpio apb clk clk gate (0-close clk ,1-open clk)  */
    CLK_PD_GPIO_PCLK_REQEN              = (0x00080801),  /*!<pd_gpio apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */


    /*** Bit definition for [padCtrlModuleCtrl] register(0x00000010) ****/
    CLK_PD_CTRL_SW_PRESETN              = (0x00100001),  /*!<pad_ctrl apb software resetn 0-active */
    CLK_PD_CTRL_PCLK_SW_EN              = (0x00100401),  /*!<pad_ctrl apb bus clk gate (0-close clk ,1-open clk)  */
    CLK_PD_CTRL_PCLK_REQEN              = (0x00100801),  /*!<pad_ctrl apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */


    /*** Bit definition for [i2c0ModuleCtrl] register(0x00000014) ****/
    CLK_I2C0_PCLK_EN                    = (0x00140001),  /*!<i2c0 apb bus clk gate (0-close clk ,1-open clk)  */
    CLK_I2C0_WCLK_EN                    = (0x00140401),  /*!<i2c0 work clk gate       (0-close clk ,1-open clk)  */
    CLK_I2C0_CLK_SEL                    = (0x00140801),  /*!<i2c0 work clk mux select (0-26Mhz,1-56Mhz) */
    CLK_I2C0_SW_PRST_N                  = (0x00140c01),  /*!<i2c0 apb software resetn (0-active) */
    CLK_I2C0_SW_WRST_N                  = (0x00141001),  /*!<i2c0 work clk domain software resetn(0-active) */
    CLK_I2C0_WCLK_REQEN                 = (0x00141401),  /*!<i2c0 work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_I2C0_PCLK_REQEN                 = (0x00141801),  /*!<i2c0 apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */


    /*** Bit definition for [i2c1ModuleCtrl] register(0x00000018) ****/
    CLK_I2C1_PCLK_EN                    = (0x00180001),  /*!<i2c1 apb bus clk gate (0-close clk ,1-open clk)  */
    CLK_I2C1_WCLK_EN                    = (0x00180401),  /*!<i2c1 work clk gate       (0-close clk ,1-open clk)  */
    CLK_I2C1_CLK_SEL                    = (0x00180801),  /*!<i2c1 work clk mux select (0-26Mhz,1-56Mhz) */
    CLK_I2C1_SW_PRST_N                  = (0x00180c01),  /*!<i2c1 apb software resetn (0-active) */
    CLK_I2C1_SW_WRST_N                  = (0x00181001),  /*!<i2c1 work clk domain software resetn(0-active) */
    CLK_I2C1_WCLK_REQEN                 = (0x00181401),  /*!<i2c1 work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_I2C1_PCLK_REQEN                 = (0x00181801),  /*!<i2c1 apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */


    /*** Bit definition for [rffeSscModuleCtrl] register(0x00000020) ****/
    CLK_RFFE_SSC_SW_PRESETN             = (0x00200001),  /*!<rffe_ssc  apb software resetn (0-active) */


    /*** Bit definition for [rfCtrlModuleCtrl] register(0x00000024) ****/
    CLK_RF_CTRL_SW_PRESETN              = (0x00240001),  /*!<rf_ctrl  apb software resetn (0-active) */


    /*** Bit definition for [uart0ModuleCtrl] register(0x00000028) ****/
    CLK_UART0_WCLK_SEL                  = (0x00280002),  /*!<uart0 work clk select (0-26Mhz,1-78Mhz,2-104Mhz,3-156Mhz) */
    CLK_UART0_SW_PCLK_EN                = (0x00280401),  /*!<uart0 apb bus clk sw gate (0-close clk ,1-open clk)  */
    CLK_UART0_SW_WCLK_EN                = (0x00280801),  /*!<uart0 work clk sw gate (0-close clk ,1-open clk)  */
    CLK_UART0_SW_PRESET_B               = (0x00280c01),  /*!<uart0 apb software resetn (0-active) */
    CLK_UART0_SW_WRESET_B               = (0x00281001),  /*!<uart0 work clk domain software resetn(0-active) */
    CLK_UART0_PCLK_REQEN                = (0x00281401),  /*!<uart0 apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_UART0_UCLK_REQEN                = (0x00281801),  /*!<uart0 work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_UART0_BUSY                      = (0x00281c01),  /*!<uart0 busy state 1-busy */


    /*** Bit definition for [spiLcdModuleCtrl] register(0x0000002c) ****/
    CLK_SPI_LCD_SW_PCLK_EN              = (0x002c0001),  /*!<spi lcd apb bus clk sw gate (0-close clk ,1-open clk)  */
    CLK_SPI_LCD_SW_WCLK_EN              = (0x002c0401),  /*!<spi lcd work clk sw gate (0-close clk ,1-open clk)  */
    CLK_SPI_LCD_SW_PRESET_B             = (0x002c0801),  /*!<spi lcd apb software resetn (0-active) */
    CLK_SPI_LCD_SW_WRESET_B             = (0x002c0c01),  /*!<spi lcd work clk domain software resetn(0-active) */
    CLK_SPI_LCD_WCLK_SEL                = (0x002c1802),  /*!<spi lcd work clk select (0-26Mhz,1-52Mhz,2-78Mhz,3-104Mhz) */


    /*** Bit definition for [spiCamModuleCtrl] register(0x00000030) ****/
    CLK_SPI_CAM_SW_PCLK_EN              = (0x00300001),  /*!<spi_cam apb bus clk sw gate (0-close clk ,1-open clk) */
    CLK_SPI_CAM_SW_WCLK_EN              = (0x00300401),  /*!<spi_cam work clk sw gate (0-close clk ,1-open clk)  */
    CLK_SPI_CAM_SW_PRESET_B             = (0x00300801),  /*!<spi_cam apb software resetn (0-active) */
    CLK_SPI_CAM_SW_WRESET_B             = (0x00300c01),  /*!<spi_cam work clk domain software resetn(0-active) */
    CLK_SPI_CAM_PCLK_REQEN              = (0x00301001),  /*!<spi_cam apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_SPI_CAM_WCLK_REQEN              = (0x00301401),  /*!<spi_cam work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_SPI_CAM_WCLK_SEL                = (0x00301802),  /*!<spi_cam work clk select (0-26Mhz,1-52Mhz,2-78Mhz,3-156Mhz) */


    /*** Bit definition for [ssp0ModuleCtrl] register(0x00000034) ****/
    CLK_GEN_SSP_SW_PCLK_EN              = (0x00340001),  /*!<gen_ssp apb bus clk sw gate (0-close clk ,1-open clk) */
    CLK_GEN_SSP_SW_WCLK_EN              = (0x00340401),  /*!<gen_ssp work clk sw gate (0-close clk ,1-open clk)  */
    CLK_GEN_SSP_SW_PRESET_B             = (0x00340801),  /*!<gen_ssp apb software resetn (0-active) */
    CLK_GEN_SSP_SW_WRESET_B             = (0x00340c01),  /*!<gen_ssp work clk domain software resetn(0-active) */
    CLK_GEN_SSP_PCLK_REQEN              = (0x00341001),  /*!<gen_ssp apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_GEN_SSP_WCLK_SEL                = (0x00341802),  /*!<gen_ssp work clk select(0-26Mhz,1-104Mhz,2-156Mhz,3-104Mhz) */
    CLK_GEN_SSP_DIV_PARAM               = (0x00341c04),  /*!<gen_ssp work clk  div param support (1-16) */


    /*** Bit definition for [i2sModuleCtrl0] register(0x00000038) ****/
    CLK_I2S_WCLK_SEL_HL                 = (0x00380002),  /*!<i2s work clk select(0-26Mhz,1-26Mhz,2-245.76Mhz,3-104Mhz) */
    CLK_I2S_SW_PCLK_EN                  = (0x00380401),  /*!<i2s apb bus clk sw gate (0-close clk ,1-open clk) */
    CLK_I2S_SW_WCLK_EN                  = (0x00380801),  /*!<i2s work clk sw gate (0-close clk ,1-open clk)  */
    CLK_I2S_SW_PRESET_B                 = (0x00380c01),  /*!<i2s apb software resetn (0-active) */
    CLK_I2S_SW_WRESET_B                 = (0x00381001),  /*!<i2s work clk domain software resetn(0-active) */
    CLK_I2S_WCLK_REQEN                  = (0x00381401),  /*!<i2s apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_I2S_PCLK_REQEN                  = (0x00381801),  /*!<i2s work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */


    /*** Bit definition for [i2sModuleCtrl1] register(0x0000003c) ****/
    CLK_I2S_WCLK_DIV                    = (0x003c0008),  /*!<work clk  div param support (2-256) */


    /*** Bit definition for [i2sModuleCtrl2] register(0x00000040) ****/
    CLK_I2S_FRAC_DIV1                   = (0x00400020),  /*!<[15:0]numerator of fraction part (fen zi),[31:16]  denominator of fraction part (fen mu)
    */


    /*** Bit definition for [i2sModuleCtrl3] register(0x00000044) ****/
    CLK_I2S_FRAC_DIV2                   = (0x00440011),  /*!<[15:0]   integer part (zheng shu),divide parameter update control
    i2s_frac_divclko = clk_in/(i2s_frac_div2[15:0] + i2s_frac_div1[15:0]/i2s_frac_div1[31:16]) */


    /*** Bit definition for [usimModuleCtrl] register(0x00000048) ****/
    CLK_USIM_SW_PCLK_EN                 = (0x00480001),  /*!<usim apb bus clk sw gate (0-close clk ,1-open clk) */
    CLK_USIM_SW_WCLK_EN                 = (0x00480401),  /*!<usim work clk sw gate (0-close clk ,1-open clk)  */
    CLK_USIM_SW_PRESET_B                = (0x00480801),  /*!<usim apb software resetn (0-active) */
    CLK_USIM_SW_WRESET_B                = (0x00480c01),  /*!<usim work clk domain software resetn(0-active) */
    CLK_USIM_PCLK_REQEN                 = (0x00481001),  /*!<usim apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_USIM_WCLK_REQEN                 = (0x00481401),  /*!<usim  work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_USIM_WCLK_DIV                   = (0x00481804),  /*!<usim work clk  div param support (1-16) */
    CLK_USIM_POWER_ON_N                 = (0x00481c01),  /*!<usim power_on read back 0-power_on */


    /*** Bit definition for [psramCtrlSwReset] register(0x0000004c) ****/
    CLK_PSRAM_CTRL_SW_PRESETN           = (0x004c0001),  /*!<psram_ctrl apb bus  software resetn (0-active) */


    /*** Bit definition for [psramPhySwReset] register(0x00000050) ****/
    CLK_PSRAM_PHY_SW_PRESETN            = (0x00500001),  /*!<psram_phy apb bus  software resetn (0-active) */


    /*** Bit definition for [xipCtrlSwReset] register(0x00000054) ****/
    CLK_XIP_CTRL_SW_PRESETN             = (0x00540001),  /*!<xip_ctrlapb bus  software resetn (0-active) */


    /*** Bit definition for [pwmCtrlSwReset] register(0x00000058) ****/
    CLK_PWM_SW_WRESET_B                 = (0x00580001),  /*!<audio pwm work clk domain software resetn(0-active) */
    CLK_PWM_SW_PRESET_B                 = (0x00580401),  /*!<audio pwm apb bus  software resetn (0-active) */
    CLK_PWM_SW_WCLK_EN                  = (0x00580801),  /*!<audio pwm work clk sw gate (0-close clk ,1-open clk)  */
    CLK_PWM_SW_PCLK_EN                  = (0x00580c01),  /*!<audio pwm apb bus clk sw gate (0-close clk ,1-open clk) */


    /*** Bit definition for [uart2ModuleCtrl] register(0x0000005c) ****/
    CLK_UART2_WCLK_SEL                  = (0x005c0002),  /*!<uart2 work clk select (0-26Mhz,1-78Mhz,2-104Mhz,3-156Mhz) */
    CLK_UART2_SW_PCLK_EN                = (0x005c0401),  /*!<uart2 apb bus clk sw gate (0-close clk ,1-open clk)  */
    CLK_UART2_SW_WCLK_EN                = (0x005c0801),  /*!<uart2 work clk sw gate (0-close clk ,1-open clk)  */
    CLK_UART2_SW_PRESET_B               = (0x005c0c01),  /*!<uart2 apb software resetn (0-active) */
    CLK_UART2_SW_WRESET_B               = (0x005c1001),  /*!<uart2 work clk domain software resetn(0-active) */
    CLK_UART2_PCLK_REQEN                = (0x005c1401),  /*!<uart2 apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_UART2_UCLK_REQEN                = (0x005c1801),  /*!<uart2 work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_UART2_BUSY                      = (0x005c1c01),  /*!<usrt2 busy state 1-busy */


    /*** Bit definition for [pdPpwmFracDiv1] register(0x00000060) ****/
    CLK_PD_PWM_FRAC_DIV1                = (0x00600020),  /*!<[15:0]numerator of fraction part (fen zi),[31:16]  denominator of fraction part (fen mu) */


    /*** Bit definition for [pdPpwmFracDiv2] register(0x00000064) ****/
    CLK_PD_PWM_FRAC_DIV2                = (0x00640011),  /*!<[15:0]   integer part (zheng shu),divide parameter update control
    pd_pwm_frac_div_clko = clk_in/(pd_pwm_frac_div2[15:0] + pd_pwm_frac_div1[15:0]/pd_pwm_frac_div1[31:16]) */

    /*** Bit definition for [uart3ModuleCtrl] register(0x00000068) ****/
    CLK_UART3_WCLK_SEL                  = (0x00680002),  /*!<uart3 work clk select (0-26Mhz,1-78Mhz,2-104Mhz,3-156Mhz) */
    CLK_UART3_SW_PCLK_EN                = (0x00680401),  /*!<uart3 apb bus clk sw gate (0-close clk ,1-open clk)  */
    CLK_UART3_SW_WCLK_EN                = (0x00680801),  /*!<uart3 work clk sw gate (0-close clk ,1-open clk)  */
    CLK_UART3_SW_PRESET_B               = (0x00680c01),  /*!<uart3 apb software resetn (0-active) */
    CLK_UART3_SW_WRESET_B               = (0x00681001),  /*!<uart3 work clk domain software resetn(0-active) */
    CLK_UART3_PCLK_REQEN                = (0x00681401),  /*!<uart3 apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_UART3_UCLK_REQEN                = (0x00681801),  /*!<uart3 work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_UART3_BUSY                      = (0x00681c01),  /*!<usrt3 busy state 1-busy */


    /*** Bit definition for [i2c2ModuleCtrl] register(0x0000006c) ****/
    CLK_I2C2_PCLK_EN                    = (0x006c0001),  /*!<i2c2 apb bus clk gate (0-close clk ,1-open clk)  */
    CLK_I2C2_WCLK_EN                    = (0x006c0401),  /*!<i2c2 work clk gate       (0-close clk ,1-open clk)  */
    CLK_I2C2_CLK_SEL                    = (0x006c0801),  /*!<i2c2 work clk mux select (0-26Mhz,1-56Mhz) */
    CLK_I2C2_SW_PRST_N                  = (0x006c0c01),  /*!<i2c2 apb software resetn (0-active) */
    CLK_I2C2_SW_WRST_N                  = (0x006c1001),  /*!<i2c2 work clk domain software resetn(0-active) */
    CLK_I2C2_WCLK_REQEN                 = (0x006c1401),  /*!<i2c2 work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_I2C2_PCLK_REQEN                 = (0x006c1801),  /*!<i2c2 apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */


    /*** Bit definition for [i2c3ModuleCtrl] register(0x00000070) ****/
    CLK_I2C3_PCLK_EN                    = (0x00700001),  /*!<i2c3 apb bus clk gate (0-close clk ,1-open clk)  */
    CLK_I2C3_WCLK_EN                    = (0x00700401),  /*!<i2c3 work clk gate       (0-close clk ,1-open clk)  */
    CLK_I2C3_CLK_SEL                    = (0x00700801),  /*!<i2c3 work clk mux select (0-26Mhz,1-56Mhz) */
    CLK_I2C3_SW_PRST_N                  = (0x00700c01),  /*!<i2c3 apb software resetn (0-active) */
    CLK_I2C3_SW_WRST_N                  = (0x00701001),  /*!<i2c3 work clk domain software resetn(0-active) */
    CLK_I2C3_WCLK_REQEN                 = (0x00701401),  /*!<i2c3 work clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_I2C3_PCLK_REQEN                 = (0x00701801),  /*!<i2c3 apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */


    /*** Bit definition for [ssp1ModuleCtrl] register(0x00000074) ****/
    CLK_GEN_SSP1_SW_PCLK_EN             = (0x00740001),  /*!<gen_ssp1 apb bus clk sw gate (0-close clk ,1-open clk) */
    CLK_GEN_SSP1_SW_WCLK_EN             = (0x00740401),  /*!<gen_ssp1 work clk sw gate (0-close clk ,1-open clk)  */
    CLK_GEN_SSP1_SW_PRESET_B            = (0x00740801),  /*!<gen_ssp apb software resetn (0-active) */
    CLK_GEN_SSP1_SW_WRESET_B            = (0x00740c01),  /*!<gen_ssp1 work clk domain software resetn(0-active) */
    CLK_GEN_SSP1_PCLK_REQEN             = (0x00741001),  /*!<gen_ssp1 apb bus clk auto gate ，use by lowpower（1-open auto gate，0-clk always open） */
    CLK_GEN_SSP1_WCLK_SEL               = (0x00741802),  /*!<gen_ssp1 work clk select(0-26Mhz,1-104Mhz,2-156Mhz,3-104Mhz) */
    CLK_GEN_SSP1_DIV_PARAM              = (0x00741c04),  /*!<gen_ssp1 work clk  div param support (1-16) */

} CLK_PdcoreLspCrmRegs;


typedef enum
{
    /*** Bit definition for [spll0LockCsr] register(0x00000000) ****/
    CLK_SPLL0_LOCK_MODE                 = (0x00000001),  /*!<0:use pll generated lock;1:use timer counter based lock; */
    CLK_SPLL0_LOCK                      = (0x00000101),  /*!<pll generated lock for spll0. */
    CLK_SPLL0_DBG_LOCK                  = (0x00000201),  /*!<pll_lock_sel output for spll0. */
    CLK_SPLL0_LOCK_DELAY                = (0x00001010),  /*!<lock timer count value for spll0. */


    /*** Bit definition for [spll0FoutSwapDone] register(0x00000004) ****/
    CLK_SPLL0_FOUTVCO_SWAP_DONE         = (0x00040001),  /*!<switch completed. */
    CLK_SPLL0_FOUT1PH0_PSRAM_PHY_SWAP_DONE= (0x00040d01),  /*!<switch completed. */
    CLK_SPLL0_FOUTPOSTDIV_PSRAM_PHY_SWAP_DONE= (0x00040e01),  /*!<switch completed. */


    /*** Bit definition for [spll0Ctrl0] register(0x00000008) ****/
    CLK_SPLL0_PD                        = (0x00080001),  /*!<spll0 power down. */
    CLK_SPLL0_FOUTVCO2XPD               = (0x00080101),  /*!<spll0 foutvco2x power down. */
    CLK_SPLL0_FOUTVCOPD                 = (0x00080201),  /*!<spll0 foutvco power down. */
    CLK_SPLL0_FOUTPOSTDIVPD             = (0x00080301),  /*!<spll0 foutpostdiv power down. */
    CLK_SPLL0_FOUT4PHASEPD              = (0x00080401),  /*!<spll0 fout4phase power down. */
    CLK_SPLL0_BYPASS                    = (0x00080501),  /*!<spll0 foutpostdiv bypassed to fref. */
    CLK_SPLL0_DACPD                     = (0x00080601),  /*!<spll0 noise cancelling dac power down. default 0,1 is only for test. */
    CLK_SPLL0_DSMPD                     = (0x00080701),  /*!<0:int mode;1:frac mode; */
    CLK_SPLL0_REFDIV                    = (0x00080806),  /*!<spll0 reference clock divider for PFD. */
    CLK_SPLL0_FBDIV                     = (0x0008100c),  /*!<spll0 feedback divider. */


    /*** Bit definition for [spll0Ctrl1] register(0x0000000c) ****/
    CLK_SPLL0_FRAC                      = (0x000c0018),  /*!<spll0 feedback divider fractional part. */
    CLK_SPLL0_POSTDIV1                  = (0x000c1803),  /*!<spll0 post divider1. */
    CLK_SPLL0_POSTDIV2                  = (0x000c1c03),  /*!<spll0 post divider2. */


    /*** Bit definition for [swClkEn] register(0x00000010) ****/
    CLK_AP_CORE_CLK_EN                  = (0x00100001),  /*!<ap core clock software enable. */
    CLK_PSRAM_CLK_EN                    = (0x00100101),  /*!< */
    CLK_CP_CORE_CLK_EN                  = (0x00100201),  /*!<cp core clock software enable. */
    CLK_MODEM_CLK_EN                    = (0x00100301),  /*!<modem clock software enable. */
    CLK_PSRAM_PHY_CLK_EN                = (0x00100401),  /*!<psram phy clock software enable. */
    CLK_PSRAM_CTL_CLK_EN                = (0x00100501),  /*!<psram control clock software enable. */
    CLK_RFC_PCLK_EN                     = (0x00100601),  /*!< */
    CLK_XIP_SFC_CLK_EN                  = (0x00100701),  /*!<xip sfc work clock software enable. */
    CLK_XIP_SFC_PCLK_EN                 = (0x00100801),  /*!<xip sfc apb clock software enable. */
    CLK_PSRAM_CTRL_PCLK_EN              = (0x00100901),  /*!< */
    CLK_PSRAM_PHY_PCLK_EN               = (0x00100a01),  /*!< */
    CLK_PSRAM_CACHE_CLK_EN              = (0x00100c01),  /*!< */
    CLK_RFFE_SSC_WCLK_52M_EN            = (0x00101501),  /*!<rffe ssc wclk 52m时钟门控，1：时钟打开，0：时钟关闭 */
    CLK_RFFE_SSC_APB_PCLK_EN            = (0x00101601),  /*!<rffe ssc apb clk时钟门控，1：时钟打开，0：时钟关闭 */
    CLK_AU_MCLK_EN                      = (0x00101701),  /*!<au mclk时钟门控，1：时钟打开，0：时钟关闭 */
    CLK_CAM_MCLK_EN                     = (0x00101801),  /*!<cam mclk时钟门控，1：时钟打开，0：时钟关闭 */
    CLK_RF_CTL_PCLK_REQ_EN              = (0x00101901),  /*!< */
    CLK_PSRAM_CTL_PCLK_REQEN            = (0x00101a01),  /*!< */
    CLK_PSRAM_PHY_PCLK_REQEN            = (0x00101b01),  /*!< */
    CLK_MODEM_ADB400_CLK_EN             = (0x00101c01),  /*!< */
    CLK_PSRAM_ADB400_CLK_EN             = (0x00101d01),  /*!< */
    CLK_XIP_SFC_ADB400_CLK_EN           = (0x00101e01),  /*!< */


    /*** Bit definition for [swClkSel] register(0x00000014) ****/
    CLK_AP_CORE_CLK_SEL                 = (0x00140002),  /*!<ap core clock frequecy selection. */
    CLK_CP_CORE_CLK_SEL                 = (0x00140802),  /*!<cp core clock frequecy selection. */
    CLK_PSRAM_PHY_CTRL_CLK_SEL          = (0x00140c02),  /*!<psram phy control clock frequecy selection. */
    CLK_XIP_SFC_CLK_SEL                 = (0x00141002),  /*!<xip sfc main clock frequecy selection. */
    CLK_APSS_MATRIX_CLK_SEL             = (0x00141402),  /*!<ap subsystem matrix main clock frequecy selection. */

    /*** Bit definition for [clkOutCtrl] register(0x0000001c) ****/
    CLK_CAM_MCLK_SEL                    = (0x001c0002),  /*!<camera module main clock selection */
    CLK_AU_MCLK_SEL                     = (0x001c0403),  /*!<audio codec main clock selection */


    /*** Bit definition for [clkReqen] register(0x00000024) ****/
    CLK_IRAM_ACLK_REQEN_RAM0            = (0x00240001),  /*!< */
    CLK_IRAM_ACLK_REQEN_RAM1            = (0x00240101),  /*!< */
    CLK_IRAM_ACLK_REQEN_RAM2            = (0x00240201),  /*!< */


    /*** Bit definition for [swReset] register(0x00000028) ****/
    CLK_XIP_SFC_RESETN_SW               = (0x00280001),  /*!<软件复位，1：释放复位，0：复位 */
    CLK_XIP_SFC_PRESETN_SW              = (0x00280101),  /*!<软件复位，1：释放复位，0：复位 */
    CLK_PSRAM_PHY_RESETN_SW             = (0x00280201),  /*!<软件复位，1：释放复位，0：复位 */
    CLK_PSRAM_PHY_PRESETN_SW            = (0x00280301),  /*!<软件复位，1：释放复位，0：复位 */
    CLK_PSRAM_CTRL_RESETN_SW            = (0x00280401),  /*!<软件复位，1：释放复位，0：复位 */
    CLK_PSRAM_CTRL_PRESETN_SW           = (0x00280501),  /*!<软件复位，1：释放复位，0：复位 */


    /*** Bit definition for [divClkOutCtrl] register(0x00000030) ****/
    CLK_CAM_MCLK_DIV_SEL                = (0x00300004),  /*!<时钟分频控制，0：1分频，1：2分频，2：3分频，3：4分频 */
    CLK_AU_MCLK_DIV_SEL                 = (0x00300404),  /*!<时钟分频控制，0：1分频，1：2分频，2：3分频，3：4分频 */


    /*** Bit definition for [acgAcsCfg0] register(0x00000038) ****/
    CLK_AP_CORE_CLK_ACS_EN              = (0x00380001),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */
    CLK_CP_CORE_CLK_ACS_EN              = (0x00380101),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */
    CLK_PSRAM_CACHE_CLK_ACS_EN          = (0x00380201),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */
    CLK_XIP_SFC_CLK_ACS_EN              = (0x00380301),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */
    CLK_APSS_MATRIX_CLK_ACS_EN          = (0x00380401),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */
    CLK_PSRAM_PHY_CTRL_CLK_ACS_EN       = (0x00380501),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */
    CLK_APSS_MATRIX_CLK_ACG_EN          = (0x00380901),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */
    CLK_PSRAM_CACHE_CLK_ACG_EN          = (0x00380a01),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */
    CLK_PSRAM_PHY_CTRL_CLK_ACG_EN       = (0x00380b01),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */
    CLK_XIP_SFC_CLK_ACG_EN              = (0x00380c01),  /*!<acs自动时钟选择使能，1：使能自动频率选择，0：不使能 */


    /*** Bit definition for [acgAcsCfg1] register(0x0000003c) ****/
    CLK_AP_CORE_CLK_SEL_ACS             = (0x003c0002),  /*!<acs自动时候后进入低功耗状态选择的时钟 */
    CLK_CP_CORE_CLK_SEL_ACS             = (0x003c0402),  /*!<acs自动时候后进入低功耗状态选择的时钟 */
    CLK_PSRAM_CACHE_CLK_SEL_ACS         = (0x003c0802),  /*!<acs自动时候后进入低功耗状态选择的时钟 */
    CLK_APSS_MATRIX_CLK_SEL_ACS         = (0x003c1002),  /*!<acs自动时候后进入低功耗状态选择的时钟 */
    CLK_PSRAM_PHY_CTRL_CLK_SEL_ACS      = (0x003c1402),  /*!<acs自动时候后进入低功耗状态选择的时钟 */
    CLK_XIP_SFC_CLK_SEL_ACS             = (0x003c1802),  /*!<acs自动时候后进入低功耗状态选择的时钟 */


    /*** Bit definition for [psramCfg] register(0x00000040) ****/
    CLK_XSPI_DFI_DQS_UNDERRUN           = (0x00400001),  /*!< */
    CLK_XSPI_DFI_DQS_OVERFLOW           = (0x00400101),  /*!< */
    CLK_MODEM_ADB400_MST_CACTIVEM       = (0x00400201),  /*!< */
    CLK_XIP_SFC_ADB400_MST_CACTIVEM     = (0x00400301),  /*!< */
    CLK_PSRAM_ADB400_MST_CACTIVEM       = (0x00400401),  /*!< */
    CLK_MODEM_ADB400_SLV_CACTIVEM       = (0x00400501),  /*!< */
    CLK_XIP_SFC_ADB400_SLV_CACTIVEM     = (0x00400601),  /*!< */
    CLK_PSRAM_ADB400_SLV_CACTIVEM       = (0x00400701),  /*!< */


    /*** Bit definition for [lteHighLockCsr] register(0x00000044) ****/
    CLK_LTE_HIGH_CLK_LOCK_MODE          = (0x00440001),  /*!<lte high clk寄存器配置 */
    CLK_LTE_HIGH_CLK_LOCK               = (0x00440101),  /*!<lte high clk寄存器配置 */
    CLK_LTE_HIGH_CLK_DBG_LOCK           = (0x00440201),  /*!<lte high clk寄存器配置 */
    CLK_LTE_HIGH_CLK_SWAP_DONE          = (0x00440301),  /*!<lte high clk寄存器配置 */
    CLK_LTE_HIGH_CLK_PD                 = (0x00440401),  /*!<lte high clk寄存器配置 */
    CLK_LTE_HIGH_CLK_LOCK_DELAY         = (0x00441010),  /*!<lte high clk寄存器配置 */


    /*** Bit definition for [pllSscCfg] register(0x00000048) ****/
    CLK_DIVVAL                          = (0x00480006),  /*!<pll1 ssc寄存器配置 */
    CLK_SPREAD                          = (0x00480705),  /*!<pll1 ssc寄存器配置 */
    CLK_EXT_MAXADDR                     = (0x00481008),  /*!<pll1 ssc寄存器配置 */
    CLK_DIGABLE_SSCG                    = (0x00481801),  /*!<pll1 ssc寄存器配置 */
    CLK_DOWNSPREAD                      = (0x00481901),  /*!<pll1 ssc寄存器配置 */
    CLK_SEL_EXTWAVE                     = (0x00481a01),  /*!<pll1 ssc寄存器配置 */
    CLK_RESETPTR                        = (0x00481b01),  /*!<pll1 ssc寄存器配置 */
    CLK_PLL_SSC_CLKSSCG_EN              = (0x00481d01),  /*!<pll1 ssc时钟软件门控，1：时钟打开，0：时钟关闭 */

} CLK_TopCrmRegs;

typedef enum
{
    /*** Bit definition for [psPwmTimerPara] register(0x00000000) ****/
    CLK_FB_PS_PWM_TIMER_PCLK_EN         = (0x00000001),  /*!<pclk门控使能，1：使能，0：不使能 */
    CLK_FB_PS_PWM_TIMER_WCLK_EN         = (0x00000101),  /*!<wclk门控使能，1：使能，0：不使能 */
    CLK_FB_PS_PWM_TIMER_CLK_SEL         = (0x00000201),  /*!<时钟选择，1：26m，0：32k */
    CLK_FB_PS_PWM_TIMER_SW_PRST_N       = (0x00000301),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_FB_PS_PWM_TIMER_SW_WRST_N       = (0x00000401),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_FB_PS_PWM_TIMER_PCLK_REQEN      = (0x00000501),  /*!<自动门控使能寄存器，1：使能，0：不使能 */
    CLK_FB_PS_PWM_TIMER_CLK_DIV         = (0x0000060a),  /*!<时钟分频寄存器 */


    /*** Bit definition for [cpPwmTimerPara] register(0x00000004) ****/
    CLK_FB_CP_PWM_TIMER_PCLK_EN         = (0x00040001),  /*!<pclk门控使能，1：使能，0：不使能 */
    CLK_FB_CP_PWM_TIMER_WCLK_EN         = (0x00040101),  /*!<wclk门控使能，1：使能，0：不使能 */
    CLK_FB_CP_PWM_TIMER_CLK_SEL         = (0x00040201),  /*!<时钟选择，1：26m，0：32k */
    CLK_FB_CP_PWM_TIMER_SW_PRST_N       = (0x00040301),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_FB_CP_PWM_TIMER_SW_WRST_N       = (0x00040401),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_FB_CP_PWM_TIMER_PCLK_REQEN      = (0x00040501),  /*!<自动门控使能寄存器，1：使能，0：不使能 */
    CLK_FB_CP_PWM_TIMER_CLK_DIV         = (0x0004060a),  /*!<时钟分频寄存器 */


    /*** Bit definition for [clkRstPara] register(0x00000008) ****/
    CLK_AON_26M_CLK_SEL                 = (0x00080001),  /*!<aon 26m 时钟选择，0：dcxo 26m，1：rc 26m */
    CLK_DCXO_26M_DIV794_CLK_EN          = (0x00080101),  /*!<26m 794分频时钟门控使能，1：使能，0：不使能 */
    CLK_AON_PCU_PCLK_EN                 = (0x00080201),  /*!<时钟门控使能，1：使能，0：不使能 */
    CLK_AON_PCU_PRESET_N_SW             = (0x00080301),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_AON_PCU_WCLK_EN                 = (0x00080401),  /*!<时钟门控使能，1：使能，0：不使能 */
    CLK_AON_PCU_WRESET_N_SW             = (0x00080501),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_AON_SYS_CTRL_PCLK_EN            = (0x00080601),  /*!<时钟门控使能，1：使能，0：不使能 */
    CLK_AON_SYS_CTRL_PRESET_N_SW        = (0x00080701),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_AON_PAD_CTRL_PCLK_EN            = (0x00080801),  /*!<时钟门控使能，1：使能，0：不使能 */
    CLK_AON_PAD_CTRL_PRESET_N_SW        = (0x00080901),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_AON_GPIO_PCLK_EN                = (0x00080a01),  /*!<时钟门控使能，1：使能，0：不使能 */
    CLK_AON_GPIO_PCLK_REQEN             = (0x00080b01),  /*!<自动时钟门控使能，1：使能，0：不使能 */
    CLK_AON_GPIO_PRESET_N_SW            = (0x00080c01),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_APB_KEY_PCLK_EN                 = (0x00080d01),  /*!<时钟门控使能，1：使能，0：不使能 */
    CLK_APB_KEY_PCLK_REQEN              = (0x00080e01),  /*!<自动时钟门控使能，1：使能，0：不使能 */
    CLK_APB_KEY_PRESET_N_SW             = (0x00080f01),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_APB_KEY_WCLK_EN                 = (0x00081001),  /*!<时钟门控使能，1：使能，0：不使能 */
    CLK_APB_KEY_WCLK_REQEN              = (0x00081101),  /*!<自动时钟门控使能，1：使能，0：不使能 */
    CLK_APB_KEY_WRESET_N_SW             = (0x00081201),  /*!<软复位寄存器，0：复位，1：不复位 */
    CLK_AON_32K_CLK_SEL0                = (0x00081301),  /*!<AON域32k时钟选择信号 */
    CLK_AON_32K_CLK_SEL1                = (0x00081401),  /*!<AON域32k时钟选择信号 */
    CLK_CLK_26M_OUT_EN                  = (0x00081501),  /*!<26m时钟输出门控使能 */
    CLK_CLK_26M_OUT_DIV_PARAM           = (0x00081602),  /*!<26m时钟输出分频系数 */
    CLK_CLK_26M_OUT_SEL                 = (0x00081801),  /*!<26m时钟输出选择信号,  0: dcxo_26m_clk, 1: aon_26m_clk */
    CLK_CLK_32K_OUT_SEL                 = (0x00081901),  /*!<32k时钟输出选择信号，0: rc_32k_clk,       1: aon_32k_clk */
    CLK_AON_PCU_PCLK_REQEN              = (0x00081a01),  /*!<APB 时钟自动门控请求使能，1：使能，0：不使能 */
    CLK_AON_SYS_CTRL_PCLK_REQEN         = (0x00081b01),  /*!<APB 时钟自动门控请求使能，1：使能，0：不使能 */
    CLK_AON_PAD_CTRL_PCLK_REQEN         = (0x00081c01),  /*!<APB 时钟自动门控请求使能，1：使能，0：不使能 */
    CLK_REG_26M_32KLESSCLK_SEL          = (0x00081d01),  /*!<26m时钟输出选择信号,  0: dcxo_26m_clk, 1: dcxo_26m_32kless_clk */
    CLK_DIG_DCXO_32KLESS_CLK_EN         = (0x00081e01),  /*!< dcxo_26m_32kless_clk时钟使能 */


    /*** Bit definition for [lpuartLpmCrmPara] register(0x0000000c) ****/
    CLK_LTE_LPM_PCLK_EN                 = (0x000c0001),  /*!<lte_lpm pclk enable，1：enable，0：disable */
    CLK_LTE_LPM_WCLK_EN                 = (0x000c0101),  /*!<lte_lpm wclk enable，1：enable，0：disable */
    CLK_LTE_LPM_PRESET_N_SW             = (0x000c0201),  /*!<lte_lpm  preset，1：no reset，0：reset */
    CLK_LTE_LPM_WRESET_N_SW             = (0x000c0301),  /*!<lte_lpm  wreset，1：no reset，0：reset */
    CLK_LPUART_PCLK_EN                  = (0x000c0401),  /*!<lpuart pclk enable，1：enable，0：disable */
    CLK_LPUART_WCLK_EN                  = (0x000c0501),  /*!<lpuart wclk enable，1：enable，0：disable */
    CLK_LPUART_SW_PRST_N                = (0x000c0701),  /*!<lpuart  soft preset，1：no reset，0：reset */
    CLK_LPUART_SW_WRST_N                = (0x000c0801),  /*!<lpuart  soft wreset，1：no reset，0：reset */
    CLK_LPUART_PCLK_REQEN               = (0x000c0901),  /*!<lpuart  pclk request enable，1：enable，0：disable */
    CLK_LPUART_WCLK_REQEN               = (0x000c0a01),  /*!<lpuart  wclk request enable，1：enable，0：disable */
    CLK_LPUART_CLK_SEL                  = (0x000c0b02),  /*!<lpuart clk sel */
    CLK_LTE_LPM_PCLK_REQEN              = (0x000c0d01),  /*!<APB 时钟自动门控请求使能，1：使能，0：不使能 */


    /*** Bit definition for [sbyApbClkRstPara] register(0x00000020) ****/
    CLK_SBY_LSP_PCLK_EN                 = (0x00200001),  /*!<SBY LSP APB 时钟门控使能，1：使能，0：不使能 */
    CLK_PMU_CTRL_AON_PCLK_EN            = (0x00200201),  /*!<时钟门控使能，1：使能，0：不使能 */
    CLK_PMU_CTRL_AON_WCLK_EN            = (0x00200401),  /*!<时钟门控使能，1：使能，0：不使能 */
    CLK_RC32K_TRIM_CLK_EN               = (0x00200501),  /*!<RC32K trim工作时钟门控使能，1：使能，0：不使能 */
    CLK_RC32K_TRIM_REF_CLK_EN           = (0x00200601),  /*!<RC32K trim参考时钟门控使能，1：使能，0：不使能 */


    /*** Bit definition for [clkDivPara1] register(0x00000030) ****/
    CLK_REG_DIV32K_PARAM_CFG1           = (0x00300020),  /*!<分频器，分数部分：[31:16]为分母，[15:0]为分子 */

    /*** Bit definition for [clkDivPara2] register(0x00000034) ****/
    CLK_REG_DIV32K_PARAM_CFG2           = (0x00340011),  /*!<分频器，整数部分：[15:0]，  [16]为参数更新控制 */

} CLK_regGenLbAonCrmRegs;

typedef enum
{
/*** Bit definition for [sbyClkRstPara] register(0x00000300) ****/
    CLK_PMU_CTRL_WCLK_EN                = (0x03000001),  /*!<pmu_ctrl wclk enable，1：enable，0：disable */
    CLK_LP_RTC_CLK_32K_SW_EN            = (0x03000101),  /*!< */
    CLK_LP_RTC_SW_RESETN                = (0x03000201),  /*!< */
    CLK_RTC_CLK_REQEN                   = (0x03000301),  /*!<rtc clk request enable，1：enable，0：disable */
    CLK_SBY_32K_CLK_SEL                 = (0x03000401),  /*!<sby_32k_clk mux sel，bit0 ：0,rc 32k;1,aon_32k_clk */

} CLK_regGenLbSbyCrmRegs;

typedef enum
    {
    /*** Bit definition for [clkEn] register(0x00000000) ****/
    CLK_APM0_PCLK_EN                    = (0x00000001),  /*!< */
    CLK_APM1_PCLK_EN                    = (0x00000101),  /*!< */
    CLK_APM2_PCLK_EN                    = (0x00000201),  /*!< */
    CLK_APM3_PCLK_EN                    = (0x00000301),  /*!< */
    CLK_EDMA_PCLK_EN                    = (0x00000401),  /*!< */
    CLK_IMEM_6M5S_ACLK_EN               = (0x00000501),  /*!< */
    CLK_IMEM_AS_ACLK_EN                 = (0x00000601),  /*!< */
    CLK_APMTX_AS_ACLK_EN                = (0x00000701),  /*!< */
    CLK_SHRAM0_ACLK_EN                  = (0x00000901),  /*!< */
    CLK_SHRAM1_ACLK_EN                  = (0x00000a01),  /*!< */
    CLK_SHRAM2_ACLK_EN                  = (0x00000b01),  /*!< */
    CLK_APM0_ACLK_EN                    = (0x00000c01),  /*!< */
    CLK_APM1_ACLK_EN                    = (0x00000d01),  /*!< */
    CLK_APM2_ACLK_EN                    = (0x00000e01),  /*!< */
    CLK_APM3_ACLK_EN                    = (0x00000f01),  /*!< */
    CLK_EDMA_ACLK_EN                    = (0x00001001),  /*!< */
    CLK_EDMA_CLK_REQEN                  = (0x00001101),  /*!< */
    CLK_APDMA_PCLK_EN                   = (0x00001201),  /*!< */
    CLK_APDMA_ACLK_EN                   = (0x00001301),  /*!< */
    CLK_APDMA_PCLK_REQEN                = (0x00001401),  /*!< */
    CLK_APDMA_ACLK_REQEN                = (0x00001501),  /*!< */
    CLK_EDCP_ULDL_ACLK_EN               = (0x00001601),  /*!< */
    CLK_EDCP_HCLK_EN                    = (0x00001701),  /*!< */
    CLK_HDLC_SW_PCLK_EN                 = (0x00001801),  /*!< */
    CLK_HDLC_SW_ACLK_EN                 = (0x00001901),  /*!< */
    CLK_LB_SECURITY_EFUSE_HCLK_EN       = (0x00001a01),  /*!< */
    CLK_APB_MON0_PCLK_EN                = (0x00001b01),  /*!< */
    CLK_APB_MON1_PCLK_EN                = (0x00001c01),  /*!< */


    /*** Bit definition for [rstSw] register(0x00000004) ****/
    CLK_APM0_PRESET_B_SW                = (0x00040001),  /*!< */
    CLK_APM1_PRESET_B_SW                = (0x00040101),  /*!< */
    CLK_APM2_PRESET_B_SW                = (0x00040201),  /*!< */
    CLK_APM3_PRESET_B_SW                = (0x00040301),  /*!< */
    CLK_EDMA_PRESET_B_SW                = (0x00040401),  /*!< */
    CLK_IMEM_6M5S_ACLK_RESET_B_SW       = (0x00040501),  /*!< */
    CLK_IMEM_AS_ACLK_RESET_B_SW         = (0x00040601),  /*!< */
    CLK_APMTX_AS_ACLK_RESET_B_SW        = (0x00040701),  /*!< */
    CLK_SHRAM0_ACLK_RESET_B_SW          = (0x00040901),  /*!< */
    CLK_SHRAM1_ACLK_RESET_B_SW          = (0x00040a01),  /*!< */
    CLK_SHRAM2_ACLK_RESET_B_SW          = (0x00040b01),  /*!< */
    CLK_APM0_ARESET_B_SW                = (0x00040c01),  /*!< */
    CLK_APM1_ARESET_B_SW                = (0x00040d01),  /*!< */
    CLK_APM2_ARESET_B_SW                = (0x00040e01),  /*!< */
    CLK_APM3_ARESET_B_SW                = (0x00040f01),  /*!< */
    CLK_EDMA_ACLK_RESET_B_SW            = (0x00041001),  /*!< */
    CLK_APDMA_SW_PRESET_B               = (0x00041101),  /*!< */
    CLK_APDMA_SW_ARESET_B               = (0x00041201),  /*!< */
    CLK_HDLC_SW_PRESET_B                = (0x00041301),  /*!< */
    CLK_HDLC_SW_ARESET_B                = (0x00041401),  /*!< */
    CLK_LB_SECURITY_TOP_SOFT_RESET_N    = (0x00041501),  /*!< */
    CLK_LB_SECURITY_EFUSE_SW_HRST_N     = (0x00041601),  /*!< */
    CLK_APB_MON0_PRESET_B_SW            = (0x00041701),  /*!< */
    CLK_APB_MON1_PRESET_B_SW            = (0x00041801),  /*!< */
    CLK_EDCP_SW_ULDL_ARESETN            = (0x00041901),  /*!< */
    CLK_EDCP_SW_HRESETN                 = (0x00041a01),  /*!< */
    CLK_IMEM_6M5S_ACG_BYPASS            = (0x00081101),  /*!< */
    CLK_APSS_MATRIX_CLK_BYPASS          = (0x00081401),  /*!< */
} CLK_regGenSubsysCrmRegs;

typedef enum
{
    /*** Bit definition for [clkReq] register(0x00000100) ****/
    CLK_CPU_CSR_PCLK_REQEN              = (0x01000001),  /*!<cpu_csr模块总线自动门控请求 */
    CLK_TIMER0_PCLK_REQEN               = (0x01000301),  /*!<cpu timer0模块总线自动门控请求 */
    CLK_TIMER1_PCLK_REQEN               = (0x01000401),  /*!<cpu timer1模块总线自动门控请求 */
    CLK_ICP_PCLK_REQEN                  = (0x01000501),  /*!<cpu icp模块总线自动门控请求 */
    CLK_WDT_PCLK_REQEN                  = (0x01000601),  /*!<cpu wdt模块总线自动门控请求 */
    CLK_AXI2ICB_CPUCLK_REQEN            = (0x01000701),  /*!<cpu axi2icb模块总线自动门控请求 */
    CLK_ICB1TON_CPUCLK_REQEN            = (0x01000801),  /*!<cpu icb1to4模块总线自动门控请求 */


    /*** Bit definition for [timer0Cfg] register(0x00000104) ****/
    CLK_TIMER0_PCLK_EN                  = (0x01040001),  /*!<timer0 plck enable */
    CLK_TIMER0_WCLK_EN                  = (0x01040101),  /*!<timer0 wlck enable */
    CLK_TIMER0_CLK_SEL                  = (0x01040201),  /*!<timer0 wlck select */
    CLK_TIMER0_SW_PRESET_B              = (0x01040401),  /*!<timer0 pclk soft reset */
    CLK_TIMER0_SW_WRESET_B              = (0x01040501),  /*!<timer0 wclk soft reset */
    CLK_TIMER0_CLK_DIV                  = (0x0104060a),  /*!<timer0 wlck division */


    /*** Bit definition for [timer1Cfg] register(0x00000108) ****/
    CLK_TIMER1_PCLK_EN                  = (0x01080001),  /*!<timer1 plck enable */
    CLK_TIMER1_WCLK_EN                  = (0x01080101),  /*!<timer1 wlck enable */
    CLK_TIMER1_CLK_SEL                  = (0x01080201),  /*!<timer1 wlck select */
    CLK_TIMER1_SW_PRESET_B              = (0x01080401),  /*!<timer1 pclk soft reset */
    CLK_TIMER1_SW_WRESET_B              = (0x01080501),  /*!<timer1 wclk soft reset */
    CLK_TIMER1_CLK_DIV                  = (0x0108060a),  /*!<timer1 wlck division */


    /*** Bit definition for [wdtCfg] register(0x0000010c) ****/
    CLK_WDT_PCLK_EN                     = (0x010c0001),  /*!<wdt plck enable */
    CLK_WDT_WCLK_EN                     = (0x010c0101),  /*!<wdt wlck enable */
    CLK_WDT_CLK_SEL                     = (0x010c0201),  /*!<wdt wlck select */
    CLK_WDT_SW_PRESET_B                 = (0x010c0401),  /*!<wdt pclk soft reset */
    CLK_WDT_SW_WRESET_B                 = (0x010c0501),  /*!<wdt wclk soft reset */
    CLK_WDT_CLK_DIV                     = (0x010c0604),  /*!<wdt wlck division */


    /*** Bit definition for [axiBusGate] register(0x00000114) ****/
    CLK_DEBOUNCE_COUNT                  = (0x01140010),  /*!<Wait counter to disable the bus clock, default 8’hff */
    CLK_CLK_BYPASS_SYS156               = (0x01141001),  /*!<axi2axi1system 156M clock domain clock gate bypss */
    CLK_CLK_BYPASS_AS0_CPU              = (0x01141101),  /*!<axi2axi peri clock domain clock gate bypss */
    CLK_CLK_BYPASS_2M1S_MEM312          = (0x01141201),  /*!<axi2axi_2x1 mem312 clock domain clock gate bypss */
    CLK_CLK_BYPASS_2M1S_MAIN            = (0x01141301),  /*!<axi2axi_2x1 main clock domain clock gate bypss */


    /*** Bit definition for [icpCfg] register(0x00000118) ****/
    CLK_ICP_SW_RESET_B                  = (0x01180001),  /*!<icp wclk soft reset */

} CLK_CoreCsrReg;

/*
*  bits domain definition  for clock frequency
*  clock selection
*  7...0
*/
enum clk_sel {
    APCORE_CLK_491M             = 0,
    APCORE_CLK_312M             = 1,
    APCORE_CLK_208M             = 2,
    APCORE_CLK_26M              = 3,

    CPCORE_CLK_416M             = 0,
    CPCORE_CLK_312M             = 1,
    CPCORE_CLK_208M             = 2,
    CPCORE_CLK_26M              = 3,

    PSRAM_PHYCTL_CLK_245M       = 0,
    PSRAM_PHYCTL_CLK_208M       = 1,
    PSRAM_PHYCTL_CLK_104M       = 2,
    PSRAM_PHYCTL_CLK_26M        = 3,

    PSRAM_CACHE_CLK_312M        = 0,
    PSRAM_CACHE_CLK_208M        = 1,
    PSRAM_CACHE_CLK_104M        = 2,
    PSRAM_CACHE_CLK_26M         = 3,

    APSS_AXI_CLK_156M           = 0,
    APSS_AXI_CLK_104M           = 1,
    APSS_AXI_CLK_78M            = 2,
    APSS_AXI_CLK_26M            = 3,

    XIP_SFC_CLK_245M            = 0,
    XIP_SFC_CLK_208M            = 1,
    XIP_SFC_CLK_156M            = 2,
    XIP_SFC_CLK_26M             = 3,

    CAM_MCLK_CLK_24M            = 0,
    CAM_MCLK_CLK_26M            = 1,
    CAM_MCLK_CLK_52M            = 2,
    CAM_MCLK_CLK_15M            = 3,

    AU_MCLK_CLK_26M             = 0,
    AU_MCLK_CLK_24M             = 1,
    AU_MCLK_CLK_52M             = 2,
    AU_MCLK_CLK_12M             = 3,
    AU_MCLK_CLK_15M             = 4,

    //CLK26M_OUT_SEL

    SBY_32K_CLK_RC              = 0,
    SBY_32K_CLK_AON             = 1,
    SBY_32K_CLK_DCXO            = 2,

    //CLK32K_EXTOUT_SEL
    CLK32K_EXTOUT_RC            = 0,
    CLK32K_EXTOUT_AON           = 1,

    //LPUART_CLK_SEL
    LPUART_26M                  = 0,
    LPUART_32K                  = 1,
    LPUART_156M                 = 3,

    PWM_TIMER_CLK_26M           = 1,
    PWM_TIMER_CLK_32K           = 0,

    AON26M_CLK_EXT_IN           = 1,
    AON26M_CLK_DCXO             = 0,

    CLK_EXT_OUT_AON26M          = 0,
    CLK_EXT_OUT_DCXO26M         = 1,

    I2C_CLK_26M                 = 0,
    I2C_CLK_52M                 = 1,

    UART0_WCLK_26M              = 0,
    UART0_WCLK_78M              = 1,
    UART0_WCLK_104M             = 2,
    UART0_WCLK_156M             = 3,

    SSP0_WCLK_26M               = 0,
    SSP0_WCLK_52M               = 1,
    SSP0_WCLK_78M               = 2,
    SSP0_WCLK_156M              = 3,

    SPILCD_WCLK_26M             = 0,
    SPILCD_WCLK_52M             = 1,
    SPILCD_WCLK_78M             = 2,
    SPILCD_WCLK_104M            = 3,

    SPICAM_WCLK_26M             = 0,
    SPICAM_WCLK_52M             = 1,
    SPICAM_WCLK_78M             = 2,
    SPICAM_WCLK_156M            = 3,

    ADC_WCLK_32K                = 0,
    ADC_WCLK_19M2               = 1,
    ADC_WCLK_38M4               = 2,
    ADC_WCLK_61M4               = 3,

    RFFE_26M                    = 0,
    RFFE_52M                    = 1,
    SSC_26M                     = 0,
    SSC_52M                     = 1,


};

enum ACS_IrqBitMap {
    EDMA_INT = 0,
    MODEM_INT0 = 0,
    MODEM_INT1,
    MODEM_INT2,
    MODEM_INT3,
    MODEM_INT4,
    MODEM_INT5,
    MODEM_INT6,
    MODEM_INT7,
};

#ifdef OS_USING_PM
#define CLK_IDLESAVE_BASE_ADDR          PSM_IDLE_CLK_ADDR_BASE
#define CLK_IDLESAVE_CPM_MEM            (CLK_IDLESAVE_BASE_ADDR)
#define CLK_IDLESAVE_ACG                (CLK_IDLESAVE_BASE_ADDR + 0x4)
#define CLK_IDLESAVE_SWCLK              (CLK_IDLESAVE_BASE_ADDR + 0x8)
#define CLK_IDLESAVE_SWSEL              (CLK_IDLESAVE_BASE_ADDR + 0xC)
#define CLK_IDLESAVE_PDSYSCTL           (CLK_IDLESAVE_BASE_ADDR + 0x10)
#define CLK_IDLESAVE_I2C0               (CLK_IDLESAVE_BASE_ADDR + 0x14)
#define CLK_IDLESAVE_I2C1               (CLK_IDLESAVE_BASE_ADDR + 0x18)
#define CLK_IDLESAVE_SPILCD             (CLK_IDLESAVE_BASE_ADDR + 0x1C)
#define CLK_IDLESAVE_SPICAM             (CLK_IDLESAVE_BASE_ADDR + 0x20)
#define CLK_IDLESAVE_SSP0               (CLK_IDLESAVE_BASE_ADDR + 0x24)
#define CLK_IDLESAVE_I2S0               (CLK_IDLESAVE_BASE_ADDR + 0x28)
#define CLK_IDLESAVE_I2S1               (CLK_IDLESAVE_BASE_ADDR + 0x2C)
#define CLK_IDLESAVE_I2S2               (CLK_IDLESAVE_BASE_ADDR + 0x30)
#define CLK_IDLESAVE_I2S3               (CLK_IDLESAVE_BASE_ADDR + 0x34)
#define CLK_IDLESAVE_USIM               (CLK_IDLESAVE_BASE_ADDR + 0x38)
#define CLK_IDLESAVE_I2C2               (CLK_IDLESAVE_BASE_ADDR + 0x3C)
#define CLK_IDLESAVE_I2C3               (CLK_IDLESAVE_BASE_ADDR + 0x40)
#define CLK_IDLESAVE_SSP1               (CLK_IDLESAVE_BASE_ADDR + 0x44)
#endif
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief           配置TopCrm 时钟，开关，复位，时钟源，自动门控等
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 * @param[in]       volVal      配置值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void CLK_SetTopCrmRegs(CLK_TopCrmRegs regBit, uint32_t bitVal);

/**
 ************************************************************************************
 * @brief           获取 TopCrm 时钟，开关，复位，时钟源，自动门控等配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
uint32_t CLK_GetTopCrmRegs(CLK_TopCrmRegs regBit);

/**
 ************************************************************************************
 * @brief           配置AonCrm 时钟，开关，复位，时钟源，自动门控等
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 * @param[in]       volVal      配置值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void CLK_SetregGenLbAonCrmRegs(CLK_regGenLbAonCrmRegs regBit, uint32_t bitVal);

/**
 ************************************************************************************
 * @brief           获取 AonCrm 时钟，开关，复位，时钟源，自动门控等配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
uint32_t CLK_GetregGenLbAonCrmRegs(CLK_regGenLbAonCrmRegs regBit);

/**
 ************************************************************************************
 * @brief           配置SbyCrm 时钟，开关，复位，时钟源，自动门控等
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 * @param[in]       volVal      配置值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void CLK_SetregGenLbSbyCrmRegs(CLK_regGenLbSbyCrmRegs regBit, uint32_t bitVal);

/**
 ************************************************************************************
 * @brief           获取 SbyCrm 时钟，开关，复位，时钟源，自动门控等配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
uint32_t CLK_GetregGenLbSbyCrmRegs(CLK_regGenLbSbyCrmRegs regBit);

/**
 ************************************************************************************
 * @brief           配置PdcoreLspCrm 时钟，开关，复位，时钟源，自动门控等
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 * @param[in]       volVal      配置值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void CLK_SetPdcoreLspCrmRegs(CLK_PdcoreLspCrmRegs regBit, uint32_t bitVal);

/**
 ************************************************************************************
 * @brief           获取 LspCrm 时钟，开关，复位，时钟源，自动门控等配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
uint32_t CLK_GetPdcoreLspCrmRegs(CLK_PdcoreLspCrmRegs regBit);

/**
 ************************************************************************************
 * @brief           配置CpuMatrixSubsysCrm 时钟，开关，复位，时钟源，自动门控等
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 * @param[in]       volVal      配置值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void CLK_SetCpuMatrixSubsysCrmRegs(CLK_regGenSubsysCrmRegs regBit, uint32_t bitVal);

/**
 ************************************************************************************
 * @brief           获取 CpuMatrixSubsysCrm 时钟，开关，复位，时钟源，自动门控等配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
uint32_t CLK_GetCpuMatrixSubsysCrmRegs(CLK_regGenSubsysCrmRegs regBit);

/**
 ************************************************************************************
 * @brief           配置Core csr 时钟，开关，复位，时钟源，自动门控等
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 * @param[in]       volVal      配置值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void CLK_SetCoreCsrReg(CLK_CoreCsrReg regBit, uint32_t bitVal);

/**
 ************************************************************************************
 * @brief           获取Core csr 时钟，开关，复位，时钟源，自动门控等配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
uint32_t CLK_GetCoreCsrReg(CLK_CoreCsrReg regBit);

/**
 ************************************************************************************
 * @brief           配置Core自动门控低频时钟 208M
 *
 * @param[in]       id
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void CLK_SetAutoLowFreq208M(enum ACS_IrqBitMap id);

/**
 ************************************************************************************
 * @brief           配置Core自动门控低频时钟 26M
 *
 * @param[in]       id
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void CLK_SetAutoLowFreq26M(enum ACS_IrqBitMap id);

#if defined (OS_USING_PM)
/**
 ************************************************************************************
 * @brief           Idle 休眠关闭外设时钟
 *
 * @param[in]       void
 *
 * @return          void
 ************************************************************************************
*/
void CLK_IdleSuspend(void);

/**
 ************************************************************************************
 * @brief           Idle 唤醒恢复外设时钟
 *
 * @param[in]       void
 *
 * @return          void
 ************************************************************************************
*/
void CLK_IdleResume(void);
#endif

#ifdef _CPU_AP
void CLK_SscCfg(void);
#endif
#endif
