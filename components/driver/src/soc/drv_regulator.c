/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_regulator.c
 *
 * @brief       实现电源配置接口
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
#include "drv_regulator.h"
#include <drv_pcu.h>

#ifdef OS_USING_PM
#include <psm_common.h>
#include <psm_core.h>
#endif

/************************************************************************************
 *                                 变量定义
 ************************************************************************************/
static uint32_t g_ldo8trim = (5 << 6);
#ifdef _CPU_AP
static LDO_BaseInfo g_ldoBaseInfo[LDO_MAX]= {
    {LDO_DIG_LPPMU_LPBGR_TRIM, LPBGR_BASEVOL,       LPBGR_TRIMVOL,       LDO_INV_VOL, LPBGR_STEPVOL},        // lpbgr
    {LDO_DCDC2_VO_CH,          DCDC2_VO_CH_BASEVOL, DCDC2_VO_CH_TRIMVOL, LDO_INV_VOL, DCDC2_VO_CH_STEPVOL},  // dcdc voch
    {LDO_DCDC2_TRIM_BG,        DCDC2_BASEVOL,       DCDC2_TRIMVOL,       LDO_INV_VOL, DCDC2_STEPVOL},        // dcdc trim
    {LDO_DIG_LPPMU_OPM_TRIM,   OPM_BASEVOL,         OPM_TRIMVOL,         LDO_INV_VOL, OPM_STEPVOL},          // opm
    {LDO_DIG_LPPMU_LDO1_TRIM,  LDO1_4_6_BASEVOL,    LDO1_TRIMVOL,        LDO_INV_VOL, LDO1_4_6_STEPVOL},     // ldo1
    {LDO_DIG_LPPMU_LDO2_TRIM,  LDO2_3_BASEVOL,      LDO2_TRIMVOL,        LDO_INV_VOL, LDO2_3_STEPVOL},       // ldo2
    {LDO_DIG_LPPMU_LDO3_TRIM,  LDO2_3_BASEVOL,      LDO3_TRIMVOL,        LDO_INV_VOL, LDO2_3_STEPVOL},       // ldo3
    {LDO_DIG_LPPMU_LDO4_TRIM,  LDO1_4_6_BASEVOL,    LDO4_TRIMVOL,        LDO_INV_VOL, LDO1_4_6_STEPVOL},     // ldo4
    {LDO_DIG_LPPMU_LDO5_TRIM,  LDO5_BASEVOL,        LDO5_TRIMVOL,        LDO_INV_VOL, LDO5_STEPVOL},         // ldo5
    {LDO_DIG_LPPMU_LDO6_TRIM,  LDO1_4_6_BASEVOL,    LDO6_TRIMVOL,        LDO_INV_VOL, LDO1_4_6_STEPVOL},     // ldo6
    {LDO_DIG_LPPMU_LDO7_TRIM,  LDO7_8_BASEVOL,      LDO7_TRIMVOL,        LDO_INV_VOL, LDO7_8_STEPVOL},       // ldo7
    {LDO_DIG_LPPMU_LDO8_TRIM,  LDO7_8_BASEVOL,      LDO8_TRIMVOL,        LDO_INV_VOL, LDO7_8_STEPVOL},       // ldo8
};
#endif
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief           配置LDO: eco/eco mode/discharge
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 * @param[in]       bitVal      配置值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
#ifdef _CPU_AP
void LDO_SetReg(LDO_RegGenLdo regBit, uint32_t bitVal)
#else
static void LDO_SetReg(LDO_RegGenLdo regBit, uint32_t bitVal)
#endif
{
    uint32_t base_addr;
    //fix ldo8 trim read error
    if (regBit == LDO_DIG_LPPMU_LDO7_TRIM) {
        if (bitVal > 0xF)
            return;
        WRITE_U32(LSP_SBY_LDO78_TRIM, g_ldo8trim | bitVal);
        return;
    } else if (regBit == LDO_DIG_LPPMU_LDO8_TRIM) {
        g_ldo8trim = bitVal << 6;
    }

    if (((uint32_t)regBit & 0xff) != 32)
        OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);

    base_addr = regBit > LDO_BASE_ADDR_SPLIT_FLAG ? AON_SYS_CTL_BASE_ADDR : SBY_LSP_LDO_BASE_ADDR;

    if (((regBit >> 16) & 0xfff) == (LPPMU_PMU_LDO_SEL & 0xfff))
        base_addr = PMU_CTRL_AON_BASE_ADDR;

    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(base_addr + ((uint32_t)regBit >> 16), bitVal);
        return;
    }

    MODIFY_U32(base_addr + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
}

/**
 ************************************************************************************
 * @brief           获取LDO配置: eco/eco mode/discharge
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t    功能配置值
 * @retval
 ************************************************************************************
*/
uint32_t LDO_GetReg(LDO_RegGenLdo regBit)
{
    uint32_t base_addr = regBit > LDO_BASE_ADDR_SPLIT_FLAG ? AON_SYS_CTL_BASE_ADDR : SBY_LSP_LDO_BASE_ADDR;

    if (regBit == LDO_DIG_LPPMU_LDO8_TRIM) {
       return (g_ldo8trim >> 6);
    }

    if (((regBit >> 16) & 0xfff) == (LPPMU_PMU_LDO_SEL & 0xfff))
        base_addr = PMU_CTRL_AON_BASE_ADDR;

    if (((uint32_t)regBit & 0xff) == 32)
         return READ_U32(base_addr + ((uint32_t)regBit >> 16));

    return (READ_U32(base_addr + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

#ifdef _CPU_AP
/**
 ************************************************************************************
 * @brief           初始化ldo trim信息
 *
 * @param[in]       id      ldo id
 * @param[in]       bitVal  trim值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void LDO_BaseInfoInit(LDO_Id id, int16_t bitVal)
{
    if (g_ldoBaseInfo[id].board_basevol != LDO_INV_VOL) {
        osPrintf("unexpected error: LDO_BaseInfoInit id %d init twice !\r\n", id);
        return;
    }

    //ldo7 vol cannot config by step
    if (id != LDO_LDO7_INVLID) {
        g_ldoBaseInfo[id].board_basevol = g_ldoBaseInfo[id].trim_vol - (bitVal * g_ldoBaseInfo[id].step_vol);
    }
    LDO_SetReg(g_ldoBaseInfo[id].reg_addr, bitVal);
}

int16_t LDO_GetBoardBaseVol(LDO_Id id)
{
    if (g_ldoBaseInfo[id].board_basevol != LDO_INV_VOL) {
        return g_ldoBaseInfo[id].board_basevol;
    }
    OS_ASSERT(0);
    return 0;
}

/**
 ************************************************************************************
 * @brief           配置LDO电压
 *
 * @param[in]       id          id
 * @param[in]       volVal      电压值（mv）
 *
 * @return
 * @retval          DRV_OK              配置成功
 *                  DRV_ERR_PARAMETER   参数异常
 ************************************************************************************
*/
uint32_t LDO_SetVol(LDO_Id id, int16_t volVal)
{
    int16_t baseVol, stepVol, stepVal;
    uint32_t addr;

    baseVol = LDO_GetBoardBaseVol(id);
    stepVol = g_ldoBaseInfo[id].step_vol;
    addr = g_ldoBaseInfo[id].reg_addr;

    if (LDO_LDO7_INVLID == id)
        OS_ASSERT(0);

    if ((g_ldoBaseInfo[id].step_vol > 0 && volVal < baseVol) || (g_ldoBaseInfo[id].step_vol < 0 && volVal > baseVol)) {
        osPrintf("unexpected warning: ldo 0x%x: set vol underflow vol %d mv, base vol %d\r\n", addr, volVal, baseVol);
        return DRV_ERR_PARAMETER;
    }

    stepVal = (volVal - baseVol) / stepVol;

    if (stepVal >= (1 << (((uint32_t)addr) & 0xff))) {
        osPrintf("unexpected warning: ldo 0x%x: set vol overflow vol %d mv, base vol %d, step vol %d\r\n", addr, volVal, baseVol, stepVol);
        return DRV_ERR_PARAMETER;
    }

    LDO_SetReg(addr, stepVal);
    return DRV_OK;
}
#endif

#if defined (OS_USING_PM)
/**
 ************************************************************************************
 * @brief           Idle 休眠降低ldo电压
 * @param[in]       void
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
SECTION_IRAM_FUNC void Ldo_IdleSuspend(void)
{
    //set LDO6 0.75v
    PSM_LDO6_VOL = LDO_GetReg(LDO_DIG_LPPMU_LDO6_TRIM);
    if (PSM_LDO6_VOL >= 4)
        LDO_SetReg(LDO_DIG_LPPMU_LDO6_TRIM, PSM_LDO6_VOL - 4);
    else
        LDO_SetReg(LDO_DIG_LPPMU_LDO6_TRIM, 0);
    //set DCDC2 1.15v
    PSM_DCDC2_VOL = LDO_GetReg(LDO_DCDC2_VO_CH);
    if (PSM_DCDC2_VOL <= 0xD)
        LDO_SetReg(LDO_DCDC2_VO_CH, PSM_DCDC2_VOL + 2);
    else
        LDO_SetReg(LDO_DCDC2_VO_CH, 0xF);
    //set LDO5 2.45v
    PSM_LDO5_VOL = LDO_GetReg(LDO_DIG_LPPMU_LDO5_TRIM);
    if (PSM_LDO5_VOL > 0)
        LDO_SetReg(LDO_DIG_LPPMU_LDO5_TRIM, PSM_LDO5_VOL - 1);
    //set LDO2 1.76v
    //PSM_LDO2_VOL= LDO_GetReg(LDO_DIG_LPPMU_LDO2_TRIM);
    //if (PSM_LDO2_VOL > 0)
    //    LDO_SetReg(LDO_DIG_LPPMU_LDO2_TRIM, PSM_LDO2_VOL - 1);
    //set LDO3 1.8v
    PSM_LDO3_VOL= LDO_GetReg(LDO_DIG_LPPMU_LDO3_TRIM);
    if (PSM_LDO3_VOL > 0)
        LDO_SetReg(LDO_DIG_LPPMU_LDO3_TRIM, PSM_LDO3_VOL - 1);
}

/**
 ************************************************************************************
 * @brief           Idle 唤醒恢复ldo电压
 * @param[in]       void
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
SECTION_IRAM_FUNC void Ldo_IdleResume(void)
{
    //set DCDC2 1.2v
    LDO_SetReg(LDO_DCDC2_VO_CH, PSM_DCDC2_VOL);
    //set LDO6 0.85v
    LDO_SetReg(LDO_DIG_LPPMU_LDO6_TRIM, PSM_LDO6_VOL);
    //set LDO5 2.5v
    LDO_SetReg(LDO_DIG_LPPMU_LDO5_TRIM, PSM_LDO5_VOL);
    //set LDO2 1.8v
    //LDO_SetReg(LDO_DIG_LPPMU_LDO2_TRIM, PSM_LDO2_VOL);
    //set LDO3 1.84v
    LDO_SetReg(LDO_DIG_LPPMU_LDO3_TRIM, PSM_LDO3_VOL);
}

#if !defined(_CHIP_ICT2211) && defined(_CPU_AP)
int LDO_Init(void)
{
    // LDO7/8 switch to sw ctrl, default close
    LDO_SetReg(LDO_SW_LPPMU_LDO7_SEL, 1);
    LDO_SetReg(LDO_DIG_LPPMU_LDO7_EN, 0);
    LDO_SetReg(LDO_SW_LPPMU_LDO8_SEL, 1);
    LDO_SetReg(LDO_DIG_LPPMU_LDO8_EN, 0);
    return 0;
}
INIT_CORE_EXPORT(LDO_Init, OS_INIT_SUBLEVEL_3);
#endif
//ldo 7/8 没有缓启动影响DCDC，不能降压
#if 0
static int LDO_Suspend(void *param, PSM_Mode mode, uint32_t *save_addr)
{
    if (mode == PSM_DEEP_SLEEP) {
        //set DCDC2 1.15v
        LDO_SetReg(LDO_DCDC2_VO_CH, 0xA);
    }
    return 0;
}

static int LDO_Resume(void *param, PSM_Mode mode, uint32_t *save_addr)
{
    if (mode == PSM_DEEP_SLEEP) {
        //set DCDC2 1.2v
        LDO_SetReg(LDO_DCDC2_VO_CH, 0x8);
    }
    return 0;
}

static PSM_DpmOps g_ldoDpmops = {
    .PsmSuspendNoirq = LDO_Suspend,
    .PsmResumeNoirq = LDO_Resume,
};

PSM_CMNDPM_INFO_DEFINE(ldo, g_ldoDpmops, NULL, PSM_CMNDEV_LDO);
#endif
#endif
