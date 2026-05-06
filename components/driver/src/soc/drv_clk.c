/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_clk.c
 *
 * @brief       实现时钟配置接口
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team          创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件定义
 ************************************************************************************/
#include <os.h>
#include <os_debug.h>
#include <drv_common.h>
#include <drv_soc.h>
#include <drv_spinlock.h>

#ifdef OS_USING_PM
#include <psm_common.h>
#include <psm_core.h>
#include <psm_sys.h>
#include <drv_pmu.h>
#endif

#ifdef _CPU_AP
#include <stdlib.h>
#include <string.h>
#include <drv_pin.h>
#include <nr_micro_shell.h>
#endif

#define LOW_FREQ        0x3330322

/************************************************************************************
 *                                 变量定义
 ************************************************************************************/
static volatile uint8_t g_bitmap = 0;
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
void CLK_SetTopCrmRegs(CLK_TopCrmRegs regBit, uint32_t bitVal)
{
    ubase_t plevel;
    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(TOP_CRM_ADDRBASE + ((uint32_t)regBit >> 16), bitVal);
        return;
    }
    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);

    Spinlock_HwLockIrq(CLK_HWLOCK, &plevel);
    MODIFY_U32(TOP_CRM_ADDRBASE + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
    Spinlock_HwUnlockIrq(CLK_HWLOCK, plevel);
}

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
uint32_t CLK_GetTopCrmRegs(CLK_TopCrmRegs regBit)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        return READ_U32(TOP_CRM_ADDRBASE + ((uint32_t)regBit >> 16));
    }

    return (READ_U32(TOP_CRM_ADDRBASE + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

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
void CLK_SetregGenLbAonCrmRegs(CLK_regGenLbAonCrmRegs regBit, uint32_t bitVal)
{
    ubase_t plevel;
    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(AON_CRM_ADDRBASE + ((uint32_t)regBit >> 16), bitVal);
        return;
    }

    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);
    Spinlock_HwLockIrq(CLK_HWLOCK, &plevel);
    MODIFY_U32(AON_CRM_ADDRBASE + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
    Spinlock_HwUnlockIrq(CLK_HWLOCK, plevel);
}

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
uint32_t CLK_GetregGenLbAonCrmRegs(CLK_regGenLbAonCrmRegs regBit)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        return READ_U32(AON_CRM_ADDRBASE + ((uint32_t)regBit >> 16));
    }

    return (READ_U32(AON_CRM_ADDRBASE + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

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
void CLK_SetregGenLbSbyCrmRegs(CLK_regGenLbSbyCrmRegs regBit, uint32_t bitVal)
{
    ubase_t plevel;
    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(SBY_CRM_ADDRBASE + ((uint32_t)regBit >> 16), bitVal);
        return;
    }

    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);

    Spinlock_HwLockIrq(CLK_HWLOCK, &plevel);
    MODIFY_U32(SBY_CRM_ADDRBASE + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
    Spinlock_HwUnlockIrq(CLK_HWLOCK, plevel);
}

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
uint32_t CLK_GetregGenLbSbyCrmRegs(CLK_regGenLbSbyCrmRegs regBit)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        return READ_U32(SBY_CRM_ADDRBASE + ((uint32_t)regBit >> 16));
    }

    return (READ_U32(SBY_CRM_ADDRBASE + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

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
void CLK_SetPdcoreLspCrmRegs(CLK_PdcoreLspCrmRegs regBit, uint32_t bitVal)
{
    ubase_t plevel;

    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(LSP_CRM_ADDRBASE + ((uint32_t)regBit >> 16), bitVal);
        return;
    }

    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);
    Spinlock_HwLockIrq(CLK_HWLOCK, &plevel);
    MODIFY_U32(LSP_CRM_ADDRBASE + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
    Spinlock_HwUnlockIrq(CLK_HWLOCK, plevel);
}

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
uint32_t CLK_GetPdcoreLspCrmRegs(CLK_PdcoreLspCrmRegs regBit)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        return READ_U32(LSP_CRM_ADDRBASE + ((uint32_t)regBit >> 16));
    }

    return (READ_U32(LSP_CRM_ADDRBASE + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

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
void CLK_SetCpuMatrixSubsysCrmRegs(CLK_regGenSubsysCrmRegs regBit, uint32_t bitVal)
{
    ubase_t plevel;

    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(SUBSYS_CRM_ADDRBASE + ((uint32_t)regBit >> 16), bitVal);
        return;
    }

    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);
    Spinlock_HwLockIrq(CLK_HWLOCK, &plevel);
    MODIFY_U32(SUBSYS_CRM_ADDRBASE + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
    Spinlock_HwUnlockIrq(CLK_HWLOCK, plevel);
}

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
uint32_t CLK_GetCpuMatrixSubsysCrmRegs(CLK_regGenSubsysCrmRegs regBit)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        return READ_U32(SUBSYS_CRM_ADDRBASE + ((uint32_t)regBit >> 16));
    }

    return (READ_U32(SUBSYS_CRM_ADDRBASE + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

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
void CLK_SetCoreCsrReg(CLK_CoreCsrReg regBit, uint32_t bitVal)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(CORE_CSR_ADDRBASE + ((uint32_t)regBit >> 16), bitVal);
        return;
    }

    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);

    MODIFY_U32(CORE_CSR_ADDRBASE + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
}

static void CLK_SetCpCsrReg(CLK_CoreCsrReg regBit, uint32_t bitVal)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(BASE_CPCPU_CSR + ((uint32_t)regBit >> 16), bitVal);
        return;
    }

    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);

    MODIFY_U32(BASE_CPCPU_CSR + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
}

/**
 ************************************************************************************
 * @brief           获取 Core csr  时钟，开关，复位，时钟源，自动门控等配置
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t
 * @retval                      功能配置值
 ************************************************************************************
*/
uint32_t CLK_GetCoreCsrReg(CLK_CoreCsrReg regBit)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        return READ_U32(CORE_CSR_ADDRBASE + ((uint32_t)regBit >> 16));
    }

    return (READ_U32(CORE_CSR_ADDRBASE + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

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
void CLK_SetAutoLowFreq208M(enum ACS_IrqBitMap id)
{
#ifdef _CPU_AP
    int msr = osInterruptDisable();
    CLK_SetTopCrmRegs(CLK_AP_CORE_CLK_SEL_ACS, 2);
    g_bitmap |= (1 << id);
    osInterruptEnable(msr);
#else
#ifndef USE_TOP_SSC
    int msr = osInterruptDisable();
    CLK_SetTopCrmRegs(CLK_CP_CORE_CLK_SEL_ACS, 2);
    g_bitmap |= (1 << id);
    osInterruptEnable(msr);
#endif
#endif
}

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
void CLK_SetAutoLowFreq26M(enum ACS_IrqBitMap id)
{
#ifdef _CPU_AP
    int msr = osInterruptDisable();
    g_bitmap &= ~(1 << id);
    if (!g_bitmap) {
        CLK_SetTopCrmRegs(CLK_AP_CORE_CLK_SEL_ACS, 3);
    }
    osInterruptEnable(msr);
#else
#ifndef USE_TOP_SSC
    int msr = osInterruptDisable();
    g_bitmap &= ~(1 << id);
    if (!g_bitmap) {
        CLK_SetTopCrmRegs(CLK_CP_CORE_CLK_SEL_ACS, 3);
    }
    osInterruptEnable(msr);
#endif
#endif
}

void CLK_SscCfg(void)
{
    CLK_SetTopCrmRegs(CLK_SPLL0_DSMPD, 0);
    CLK_SetTopCrmRegs(CLK_PLL_SSC_CLKSSCG_EN, 1);
    CLK_SetTopCrmRegs(CLK_DOWNSPREAD, 1);
    CLK_SetTopCrmRegs(CLK_SPREAD, 15);
    CLK_SetTopCrmRegs(CLK_DIGABLE_SSCG, 0);
}

#ifdef _CPU_AP
static void CLK_AutoGateInit(void)
{
#ifdef OS_USING_PM
    //enable sby fsm auto gate
    PMU_FuncCfg(PMU_CTRL_FSM_ID, 1);
#endif
    //psram_acg_byp
    SYSCTRL_SetPdRegGen(SYSCTRL_PSRAM_ACG_BYP, 0);
    //aon crm
    CLK_SetregGenLbAonCrmRegs(CLK_FB_PS_PWM_TIMER_PCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_FB_CP_PWM_TIMER_PCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_AON_GPIO_PCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_APB_KEY_PCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_APB_KEY_WCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_AON_PCU_PCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_AON_SYS_CTRL_PCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_AON_PAD_CTRL_PCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_LPUART_PCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_LPUART_WCLK_REQEN, 1);
    CLK_SetregGenLbAonCrmRegs(CLK_LTE_LPM_PCLK_REQEN, 1);
    //pd crm
    CLK_SetPdcoreLspCrmRegs(CLK_PD_SYS_CTRL_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_PD_GPIO_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_PD_CTRL_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C0_WCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C0_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C1_WCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C1_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_UART0_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_UART0_UCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_CAM_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_CAM_WCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_WCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_USIM_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_USIM_WCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_UART2_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_UART2_UCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_UART3_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_UART3_UCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C2_WCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C2_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C3_WCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C3_PCLK_REQEN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP1_PCLK_REQEN, 1);
    //top crm
    CLK_SetTopCrmRegs(CLK_IRAM_ACLK_REQEN_RAM0, 1);
    CLK_SetTopCrmRegs(CLK_IRAM_ACLK_REQEN_RAM1, 1);
    CLK_SetTopCrmRegs(CLK_IRAM_ACLK_REQEN_RAM2, 1);
    CLK_SetTopCrmRegs(CLK_AP_CORE_CLK_ACS_EN, 1);
    CLK_SetTopCrmRegs(CLK_CP_CORE_CLK_ACS_EN, 1);
    CLK_SetTopCrmRegs(CLK_PSRAM_CACHE_CLK_ACS_EN, 1);
    CLK_SetTopCrmRegs(CLK_XIP_SFC_CLK_ACS_EN, 1);
//    CLK_SetTopCrmRegs(CLK_APSS_MATRIX_CLK_ACS_EN, 1);
    CLK_SetTopCrmRegs(CLK_PSRAM_PHY_CTRL_CLK_ACS_EN, 1);
//    CLK_SetTopCrmRegs(CLK_APSS_MATRIX_CLK_ACG_EN, 1);
    CLK_SetTopCrmRegs(CLK_PSRAM_CACHE_CLK_ACG_EN, 1);
//    CLK_SetTopCrmRegs(CLK_PSRAM_PHY_CTRL_CLK_ACG_EN, 1);
    CLK_SetTopCrmRegs(CLK_XIP_SFC_CLK_ACG_EN, 1);
    //cpu sys mem
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_EDMA_CLK_REQEN, 1);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APDMA_PCLK_REQEN, 1);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APDMA_ACLK_REQEN, 1);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_IMEM_6M5S_ACG_BYPASS, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APSS_MATRIX_CLK_BYPASS, 0);
    //ap core csr
    CLK_SetCoreCsrReg(CLK_CPU_CSR_PCLK_REQEN, 1);
    CLK_SetCoreCsrReg(CLK_TIMER0_PCLK_REQEN, 1);
    CLK_SetCoreCsrReg(CLK_TIMER1_PCLK_REQEN, 1);
    CLK_SetCoreCsrReg(CLK_ICP_PCLK_REQEN, 1);
    CLK_SetCoreCsrReg(CLK_WDT_PCLK_REQEN, 1);
    CLK_SetCoreCsrReg(CLK_AXI2ICB_CPUCLK_REQEN, 1);
    CLK_SetCoreCsrReg(CLK_ICB1TON_CPUCLK_REQEN, 1);
    CLK_SetCoreCsrReg(CLK_CLK_BYPASS_SYS156, 0);
    CLK_SetCoreCsrReg(CLK_CLK_BYPASS_AS0_CPU, 0);
    CLK_SetCoreCsrReg(CLK_CLK_BYPASS_2M1S_MEM312, 0);
    CLK_SetCoreCsrReg(CLK_CLK_BYPASS_2M1S_MAIN, 0);
    //cp core csr
    CLK_SetCpCsrReg(CLK_CPU_CSR_PCLK_REQEN, 1);
    CLK_SetCpCsrReg(CLK_TIMER0_PCLK_REQEN, 1);
    CLK_SetCpCsrReg(CLK_TIMER1_PCLK_REQEN, 1);
    CLK_SetCpCsrReg(CLK_ICP_PCLK_REQEN, 1);
    CLK_SetCpCsrReg(CLK_WDT_PCLK_REQEN, 1);
    CLK_SetCpCsrReg(CLK_AXI2ICB_CPUCLK_REQEN, 1);
    CLK_SetCpCsrReg(CLK_ICB1TON_CPUCLK_REQEN, 1);
    CLK_SetCpCsrReg(CLK_CLK_BYPASS_SYS156, 0);
    CLK_SetCpCsrReg(CLK_CLK_BYPASS_AS0_CPU, 0);
    CLK_SetCpCsrReg(CLK_CLK_BYPASS_2M1S_MEM312, 0);
    CLK_SetCpCsrReg(CLK_CLK_BYPASS_2M1S_MAIN, 0);
}

static int CLK_Init(void)
{
    //CLK_AutoGateInit();
    //32K RC Trim
    SYSCTRL_SetAonRegGen(SYSCTRL_REG_RC_32K_AUTO_CTRL_EN, 1);

    //close all peri clk
    CLK_SetPdcoreLspCrmRegs(CLK_I2C0_PCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C0_WCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C1_PCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_I2C1_WCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_LCD_SW_PCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_LCD_SW_WCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_CAM_SW_PCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_CAM_SW_WCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_SW_PCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_SW_WCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_SW_PCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_SW_WCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_PWM_SW_WCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_PWM_SW_PCLK_EN, 0);

    CLK_SetTopCrmRegs(CLK_AU_MCLK_EN, 0);
    CLK_SetTopCrmRegs(CLK_CAM_MCLK_EN, 0);

    CLK_SetregGenLbAonCrmRegs(CLK_FB_PS_PWM_TIMER_PCLK_EN, 0);
    CLK_SetregGenLbAonCrmRegs(CLK_FB_PS_PWM_TIMER_WCLK_EN, 0);
    CLK_SetregGenLbAonCrmRegs(CLK_FB_CP_PWM_TIMER_PCLK_EN, 0);
    CLK_SetregGenLbAonCrmRegs(CLK_FB_CP_PWM_TIMER_WCLK_EN, 0);
    CLK_SetregGenLbAonCrmRegs(CLK_APB_KEY_PCLK_EN, 0);
    CLK_SetregGenLbAonCrmRegs(CLK_APB_KEY_WCLK_EN, 0);

    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APM0_PCLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APM1_PCLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APM2_PCLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APM3_PCLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APM0_ACLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APM1_ACLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APM2_ACLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APM3_ACLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_HDLC_SW_PCLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_HDLC_SW_ACLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APB_MON0_PCLK_EN, 0);
    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APB_MON1_PCLK_EN, 0);

    //enable core clk acs, ap/cp core lowest freq 208M
#ifndef _PLAT_TEST
    WRITE_U32(ACG_ACS_CFG1, LOW_FREQ);
    WRITE_U32(ACG_ACS_CFG0, 0x3);
#endif

#ifdef USE_TOP_SSC
    CLK_SscCfg();
    //change SPLL freq from 1248M to1244M
    CLK_SetTopCrmRegs(CLK_SPLL0_REFDIV, 13);
    CLK_SetTopCrmRegs(CLK_SPLL0_FBDIV, 622);
#endif
    //uart、dma、usim、edcp clk can not close
//    CLK_SetPdcoreLspCrmRegs(CLK_USIM_SW_PCLK_EN, 0);
//    CLK_SetPdcoreLspCrmRegs(CLK_USIM_SW_WCLK_EN, 0);
//    CLK_SetPdcoreLspCrmRegs(CLK_UART2_SW_PCLK_EN, 0);
//    CLK_SetPdcoreLspCrmRegs(CLK_UART2_SW_WCLK_EN, 0);
//    CLK_SetregGenLbAonCrmRegs(CLK_LPUART_PCLK_EN, 0);
//    CLK_SetregGenLbAonCrmRegs(CLK_LPUART_WCLK_EN, 0);
//    CLK_SetCpuMatrixSubsysCrmRegs(CLK_EDMA_PCLK_EN, 0);
//    CLK_SetCpuMatrixSubsysCrmRegs(CLK_EDMA_ACLK_EN, 0);
//    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APDMA_PCLK_EN, 0);
//    CLK_SetCpuMatrixSubsysCrmRegs(CLK_APDMA_ACLK_EN, 0);
//    CLK_SetCpuMatrixSubsysCrmRegs(CLK_EDCP_ULDL_ACLK_EN, 0);
//    CLK_SetCpuMatrixSubsysCrmRegs(CLK_EDCP_HCLK_EN, 0);

    return 0;
}
INIT_CORE_EXPORT(CLK_Init, OS_INIT_SUBLEVEL_HIGH);

void tc_26mclk(char argc, char **argv)
{
    if (argc != 2)
        return;
    int temp = atoi(argv[1]);

    PIN_SetMux(PIN_RES(PIN_11), PIN_11_MUX_CLK_AU_MCLK);
    PIN_SetMux(PIN_RES(PIN_14), PIN_14_MUX_CLK_AU_MCLK);
    CLK_SetTopCrmRegs(CLK_AU_MCLK_EN, 1);
    CLK_SetTopCrmRegs(CLK_AU_MCLK_SEL, temp);
    CLK_SetTopCrmRegs(CLK_AU_MCLK_DIV_SEL, 0);

    osPrintf("tc_clk_test 0: DCXO_26M\r\n 1: SPLL0_24M\r\n 2: SPLL0_52M\r\n 3: Codec_clk\r\n 4: LTE_HIGH\r\n clk out : sel %d \r\n", temp);
}
NR_SHELL_CMD_EXPORT(tc_26mclk, tc_26mclk);

void tc_32kclk(char argc, char **argv)
{
    if (argc != 2)
        return;
    int temp = atoi(argv[1]);

    PIN_SetMux(PIN_RES(PIN_8), PIN_8_MUX_CLK_32K_EXTOUT);

    if (temp == 0)
        CLK_SetregGenLbAonCrmRegs(CLK_CLK_32K_OUT_SEL, temp);
    else if (temp == 1) {
        CLK_SetregGenLbAonCrmRegs(CLK_CLK_32K_OUT_SEL, 1);
        CLK_SetregGenLbAonCrmRegs(CLK_DIG_DCXO_32KLESS_CLK_EN, 1);
        CLK_SetregGenLbAonCrmRegs(CLK_REG_26M_32KLESSCLK_SEL, 1);
        CLK_SetregGenLbAonCrmRegs(CLK_AON_32K_CLK_SEL1, 1);
    } else if (temp == 2) {
        CLK_SetregGenLbAonCrmRegs(CLK_REG_26M_32KLESSCLK_SEL, 0);
        CLK_SetregGenLbAonCrmRegs(CLK_DIG_DCXO_32KLESS_CLK_EN, 0);
        CLK_SetregGenLbAonCrmRegs(CLK_AON_32K_CLK_SEL1, 1);
        CLK_SetregGenLbAonCrmRegs(CLK_CLK_32K_OUT_SEL, 1);
    }
    osPrintf("tc_clk_test 0: RC_32K\r\n 1: 32K LESS\r\n 2: DCXO26M 794DIV\r\n clk out : sel %d \r\n", temp);
}
NR_SHELL_CMD_EXPORT(tc_32kclk, tc_32kclk);

#endif

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
SECTION_IRAM_FUNC void CLK_IdleSuspend(void)
{
#if 0
    WRITE_U32(CLK_IDLESAVE_CPM_MEM, READ_U32(CPM_MEM_CLK_EN));
    WRITE_U32(CPM_MEM_CLK_EN, READ_U32(CLK_IDLESAVE_CPM_MEM) & CPUMEMSYS_MASK);
    WRITE_U32(CLK_IDLESAVE_ACG, READ_U32(CPM_MEM_ACG_CFG));
    WRITE_U32(CPM_MEM_ACG_CFG, READ_U32(CLK_IDLESAVE_ACG) & CPUMEMACG_MASK);
    WRITE_U32(CLK_IDLESAVE_SWCLK, READ_U32(SW_CLK_EN));
    WRITE_U32(SW_CLK_EN, READ_U32(CLK_IDLESAVE_SWCLK) & SWCLK_MASK);
    WRITE_U32(CLK_IDLESAVE_SWSEL, READ_U32(SW_CLK_SEL));
    WRITE_U32(SW_CLK_SEL, (READ_U32(CLK_IDLESAVE_SWSEL) & SWSEL_MASK) | AXISEL_26M);
    WRITE_U32(CLK_IDLESAVE_PDSYSCTL, READ_U32(PD_SYSCTL_CTL));
    PSM_CLR_BIT(PD_SYSCTL_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_I2C0, READ_U32(I2C0_CTL));
    PSM_CLR_BIT(I2C0_CTL, 0);
    PSM_CLR_BIT(I2C0_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_I2C1, READ_U32(I2C1_CTL));
    PSM_CLR_BIT(I2C1_CTL, 0);
    PSM_CLR_BIT(I2C1_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_SPILCD, READ_U32(SPI_LCD_CTL));
    PSM_CLR_BIT(SPI_LCD_CTL, 0);
    PSM_CLR_BIT(SPI_LCD_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_SPICAM, READ_U32(SPI_LCD_CTL));
    PSM_CLR_BIT(SPI_CAM_CTL, 0);
    PSM_CLR_BIT(SPI_CAM_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_SSP0, READ_U32(SSP0_CTL));
    PSM_CLR_BIT(SSP0_CTL, 0);
    PSM_CLR_BIT(SSP0_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_I2S0, READ_U32(I2S0_CTL));
    PSM_CLR_BIT(I2S0_CTL, 0);
    PSM_CLR_BIT(I2S0_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_I2S1, READ_U32(I2S1_CTL));
    PSM_CLR_BIT(I2S1_CTL, 0);
    PSM_CLR_BIT(I2S1_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_I2S2, READ_U32(I2S2_CTL));
    PSM_CLR_BIT(I2S2_CTL, 0);
    PSM_CLR_BIT(I2S2_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_I2S3, READ_U32(I2S3_CTL));
    PSM_CLR_BIT(I2S3_CTL, 0);
    PSM_CLR_BIT(I2S3_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_USIM, READ_U32(USIM_CTL));
    PSM_CLR_BIT(USIM_CTL, 0);
    PSM_CLR_BIT(USIM_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_I2C2, READ_U32(I2C2_CTL));
    PSM_CLR_BIT(I2C2_CTL, 0);
    PSM_CLR_BIT(I2C2_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_I2C3, READ_U32(I2C3_CTL));
    PSM_CLR_BIT(I2C3_CTL, 0);
    PSM_CLR_BIT(I2C3_CTL, 4);
    WRITE_U32(CLK_IDLESAVE_SSP1, READ_U32(SSP1_CTL));
    PSM_CLR_BIT(SSP1_CTL, 0);
    PSM_CLR_BIT(SSP1_CTL, 4);
#endif

    PSM_SET_BIT(SPLL0_CTRL0, 0);
}

/**
 ************************************************************************************
 * @brief           Idle 唤醒恢复外设时钟
 *
 * @param[in]       void
 *
 * @return          void
 ************************************************************************************
*/
SECTION_IRAM_FUNC void CLK_IdleResume(void)
{
    while (READ_U32(SPLL0_CTRL0) & (0x1 << 0)) {
        /* Poweron pll*/
        PSM_CLR_BIT(SPLL0_CTRL0, 0);
    }
    //wait pll lock
    while(!(READ_U32(SPLL0_LOCK_CSR) & (0x1 << 1)));

#if 0
    WRITE_U32(CPM_MEM_CLK_EN, READ_U32(CLK_IDLESAVE_CPM_MEM));
    WRITE_U32(CPM_MEM_ACG_CFG, READ_U32(CLK_IDLESAVE_ACG));
    WRITE_U32(SW_CLK_EN, READ_U32(CLK_IDLESAVE_SWCLK));
    WRITE_U32(SW_CLK_SEL, READ_U32(CLK_IDLESAVE_SWSEL));
    WRITE_U32(PD_SYSCTL_CTL, READ_U32(CLK_IDLESAVE_PDSYSCTL));
    WRITE_U32(I2C0_CTL, READ_U32(CLK_IDLESAVE_I2C0));
    WRITE_U32(I2C1_CTL, READ_U32(CLK_IDLESAVE_I2C1));
    WRITE_U32(SPI_LCD_CTL, READ_U32(CLK_IDLESAVE_SPILCD));
    WRITE_U32(SPI_CAM_CTL, READ_U32(CLK_IDLESAVE_SPICAM));
    WRITE_U32(SSP0_CTL, READ_U32(CLK_IDLESAVE_SSP0));
    WRITE_U32(I2S0_CTL, READ_U32(CLK_IDLESAVE_I2S0));
    WRITE_U32(I2S1_CTL, READ_U32(CLK_IDLESAVE_I2S1));
    WRITE_U32(I2S2_CTL, READ_U32(CLK_IDLESAVE_I2S2));
    WRITE_U32(I2S3_CTL, READ_U32(CLK_IDLESAVE_I2S3));
    WRITE_U32(USIM_CTL, READ_U32(CLK_IDLESAVE_USIM));
    WRITE_U32(I2C2_CTL, READ_U32(CLK_IDLESAVE_I2C2));
    WRITE_U32(I2C3_CTL, READ_U32(CLK_IDLESAVE_I2C3));
    WRITE_U32(SSP1_CTL, READ_U32(CLK_IDLESAVE_SSP1));
#endif
}

static int CLK_Suspend(void *param, PSM_Mode mode, uint32_t *save_addr)
{
    uint32_t* addr = save_addr;
    if (mode == PSM_DEEP_SLEEP) {
        //pd crm
        WRITE_U32(addr++, READ_U32(PD_SYSCTL_CTL));
        WRITE_U32(addr++, READ_U32(PD_GPIO_CTL));
        WRITE_U32(addr++, READ_U32(PAD_CTL));
        WRITE_U32(addr++, READ_U32(I2C0_CTL));
        WRITE_U32(addr++, READ_U32(I2C1_CTL));
        WRITE_U32(addr++, READ_U32(UART0_CTL));
        WRITE_U32(addr++, READ_U32(SPI_LCD_CTL));
        WRITE_U32(addr++, READ_U32(SPI_CAM_CTL));
        WRITE_U32(addr++, READ_U32(SSP0_CTL));
        WRITE_U32(addr++, READ_U32(I2S0_CTL));
        WRITE_U32(addr++, READ_U32(I2S1_CTL));
        WRITE_U32(addr++, READ_U32(I2S2_CTL));
        WRITE_U32(addr++, READ_U32(I2S3_CTL));
        WRITE_U32(addr++, READ_U32(USIM_CTL));
        WRITE_U32(addr++, READ_U32(AUDIO_PWM_CTL));
        WRITE_U32(addr++, READ_U32(UART2_CTL));
        WRITE_U32(addr++, READ_U32(AUDIO_PWM_DIV1));
        WRITE_U32(addr++, READ_U32(AUDIO_PWM_DIV2));
        WRITE_U32(addr++, READ_U32(UART3_CTL));
        WRITE_U32(addr++, READ_U32(I2C2_CTL));
        WRITE_U32(addr++, READ_U32(I2C3_CTL));
        WRITE_U32(addr++, READ_U32(SSP1_CTL));
        //sys mem
        WRITE_U32(addr++, READ_U32(CPM_MEM_CLK_EN));
        WRITE_U32(addr++, READ_U32(CPM_MEM_RST_SW));
        WRITE_U32(addr++, READ_U32(CPM_MEM_ACG_CFG));
        //top crm
        WRITE_U32(addr++, READ_U32(CLK_REQEN));
        WRITE_U32(addr++, READ_BIT_U32(SW_CLK_EN, 2, 23));
        WRITE_U32(addr++, READ_U32(CLK_OUT_CTRL));
        WRITE_U32(addr++, READ_U32(DIV_CLK_OUT_CTRL));
        WRITE_U32(addr++, READ_U32(ACG_ACS_CFG0));
        WRITE_U32(addr++, READ_U32(ACG_ACS_CFG1));
        //ap core csr
#ifdef OS_USING_TRACE_RAM
        WRITE_U32(addr++, READ_U32(AP_CSR_DBG));
#endif
        WRITE_U32(addr++, READ_U32(AP_CLK_REQ));
        WRITE_U32(addr++, READ_U32(AP_TIMER0_CFG));
        WRITE_U32(addr++, READ_U32(AP_TIMER1_CFG));
        WRITE_U32(addr++, READ_U32(AP_WDT_CFG));
        WRITE_U32(addr++, READ_U32(AP_BUS_GATE));
        //cp core csr
#ifdef OS_USING_TRACE_RAM
        WRITE_U32(addr++, READ_U32(CP_CSR_DBG));
#endif
        WRITE_U32(addr++, READ_U32(CP_CLK_REQ));
        WRITE_U32(addr++, READ_U32(CP_TIMER0_CFG));
        WRITE_U32(addr++, READ_U32(CP_TIMER1_CFG));
        WRITE_U32(addr++, READ_U32(CP_WDT_CFG));
        WRITE_U32(addr++, READ_U32(CP_BUS_GATE));
        //pd sysctrl
        WRITE_U32(addr++, READ_U32(PD_SYSCTRL_PSRAM_CFG));
    }
    return (addr - save_addr) * 4;
}

static int CLK_Resume(void *param, PSM_Mode mode, uint32_t *save_addr)
{
    uint32_t* addr = save_addr;
    if (mode == PSM_DEEP_SLEEP) {
        //pd crm
        WRITE_U32(PD_SYSCTL_CTL, READ_U32(addr++));
        WRITE_U32(PD_GPIO_CTL, READ_U32(addr++));
        WRITE_U32(PAD_CTL, READ_U32(addr++));
        WRITE_U32(I2C0_CTL, READ_U32(addr++));
        WRITE_U32(I2C1_CTL, READ_U32(addr++));
        WRITE_U32(UART0_CTL, READ_U32(addr++));
        WRITE_U32(SPI_LCD_CTL, READ_U32(addr++));
        WRITE_U32(SPI_CAM_CTL, READ_U32(addr++));
        WRITE_U32(SSP0_CTL, READ_U32(addr++));
        WRITE_U32(I2S0_CTL, READ_U32(addr++));
        WRITE_U32(I2S1_CTL, READ_U32(addr++));
        WRITE_U32(I2S2_CTL, READ_U32(addr++));
        WRITE_U32(I2S3_CTL, READ_U32(addr++));
        WRITE_U32(USIM_CTL, READ_U32(addr++));
        WRITE_U32(AUDIO_PWM_CTL, READ_U32(addr++));
        WRITE_U32(UART2_CTL, READ_U32(addr++));
        WRITE_U32(AUDIO_PWM_DIV1, READ_U32(addr++));
        WRITE_U32(AUDIO_PWM_DIV2, READ_U32(addr++));
        WRITE_U32(UART3_CTL, READ_U32(addr++));
        WRITE_U32(I2C2_CTL, READ_U32(addr++));
        WRITE_U32(I2C3_CTL, READ_U32(addr++));
        WRITE_U32(SSP1_CTL, READ_U32(addr++));
        //sys mem
        WRITE_U32(CPM_MEM_CLK_EN, READ_U32(addr++));
        WRITE_U32(CPM_MEM_RST_SW, READ_U32(addr++));
        WRITE_U32(CPM_MEM_ACG_CFG, READ_U32(addr++));
        //top crm
        WRITE_U32(CLK_REQEN, READ_U32(addr++));
        WRITE_BIT_U32(SW_CLK_EN, READ_U32(addr++), 2, 23);
        WRITE_U32(CLK_OUT_CTRL, READ_U32(addr++));
        WRITE_U32(DIV_CLK_OUT_CTRL, READ_U32(addr++));
        WRITE_U32(ACG_ACS_CFG0, READ_U32(addr++));
        WRITE_U32(ACG_ACS_CFG1, READ_U32(addr++));
        //ap core csr
#ifdef OS_USING_TRACE_RAM
        WRITE_U32(AP_CSR_DBG, READ_U32(addr++));
#endif
        WRITE_U32(AP_CLK_REQ, READ_U32(addr++));
        WRITE_U32(AP_TIMER0_CFG, READ_U32(addr++));
        WRITE_U32(AP_TIMER1_CFG, READ_U32(addr++));
        WRITE_U32(AP_WDT_CFG, READ_U32(addr++));
        WRITE_U32(AP_BUS_GATE, READ_U32(addr++));
        //cp core csr
#ifdef OS_USING_TRACE_RAM
        WRITE_U32(CP_CSR_DBG, READ_U32(addr++));
#endif
        WRITE_U32(CP_CLK_REQ, READ_U32(addr++));
        WRITE_U32(CP_TIMER0_CFG, READ_U32(addr++));
        WRITE_U32(CP_TIMER1_CFG, READ_U32(addr++));
        WRITE_U32(CP_WDT_CFG, READ_U32(addr++));
        WRITE_U32(CP_BUS_GATE, READ_U32(addr++));
        //pd sysctrl
        WRITE_U32(PD_SYSCTRL_PSRAM_CFG, READ_U32(addr++));
#ifdef USE_TOP_SSC
        CLK_SscCfg();
#endif
    }
    return (addr - save_addr) * 4;
}

static PSM_DpmOps g_clkDpmops = {
    .PsmSuspendNoirq = CLK_Suspend,
    .PsmResumeNoirq = CLK_Resume,
};

PSM_CMNDPM_INFO_DEFINE(clk, g_clkDpmops, NULL, PSM_CMNDEV_CLK);
#endif
