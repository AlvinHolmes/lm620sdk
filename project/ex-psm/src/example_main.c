/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file main.c
*
* @brief  main函数入口文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/
#include <os.h>
#include <os_def.h>
#include <os_debug.h>
#include <os_list.h>
#include <drv_common.h>
#include <psm_common.h>
#include <psm_core.h>
#include <psm_timemgr.h>
#include <psm_sys.h>
#include <drv_pcu.h>
#include <drv_pmu.h>
#include <drv_psm_sys.h>
#include <drv_lp.h>
#include <nr_micro_shell.h>
#include <stdlib.h>
#include <string.h>

#include <psm_wakelock.h>
#include <drv_soc.h>
#include <drv_rtc.h>
#include <cmsis_os2.h>

static WakeLock g_psm_wakelock;
struct osTimer work_timer;

void psm_enter(void)
{
    PSM_SET_BIT(LSP_SBY_32K_CFG, RC32K_EN_BIT);
    CLK_SetregGenLbSbyCrmRegs(CLK_SBY_32K_CLK_SEL, 0);
    CLK_SetregGenLbAonCrmRegs(CLK_AON_32K_CLK_SEL0, 0);
    CLK_SetregGenLbAonCrmRegs(CLK_AON_32K_CLK_SEL1, 0);

#ifdef OS_USING_PM
    PCU_WakeupIrqClrAll();
    WRITE_U32(PCU_AON_WU_INT_DISSBY, 0xFFFFFFFF);
    WRITE_U32(PCU_AON_WU_INT_DISAON, 0xFFFFFFFF);
    WRITE_U32(PCU_AON_WU_INT_DISAPSS, 0xFFFFFFFF);
    WRITE_U32(PCU_AON_WU_INT_DISCPSS, 0xFFFFFFFF);
    WRITE_U32(PCU_AON_WU_INT_DISSBY1, 0xFFFFFFFF);
    WRITE_U32(PCU_AON_WU_INT_DISAON1, 0xFFFFFFFF);
    WRITE_U32(PCU_AON_WU_INT_DISAPSS1, 0xFFFFFFFF);
    WRITE_U32(PCU_AON_WU_INT_DISCPSS1, 0xFFFFFFFF);
#endif
    osInterruptMaskAll();

    //PCU_SetSubSysSleepMode(true);

    WRITE_BIT_U32(PCU_AON_APSS_CFG, 0x3C, 6, 4);
    PSM_SET_BIT(PCU_AON_APSS_CFG, 0);

    WRITE_BIT_U32(PCU_AON_CPSS_CFG, 0x3C, 6, 4);
    PSM_SET_BIT(PCU_AON_CPSS_CFG, 0);

    PCU_ChipModeCfg(CHIP_SBY_MODE, true);
    //bypass cp
    PSM_SET_BIT(PCU_AON_SLEEP_BYPASS, 1);
    //bypass ap
    PSM_SET_BIT(PCU_AON_SLEEP_BYPASS, 0);
    osEnterWfi();
}

static void work_timeout(void * parameter)
{
    //创建RTC Timer 10s 后唤醒
    RTC_Time rtcTime = RTC_GetTime();
    int g_rtcid = RTC_RegisterAlarm(rtcTime + 10, 0, NULL);
    if (g_rtcid < 0) {
        osPrintf("work_timeout set RTC Alarm failed\r\n");
        return;
    }
    //强制进入psm
    psm_enter();
}

static void osPsmThreadEntry(void *parameter)
{
    //初始化休眠锁
    PSM_WakelockInit(&g_psm_wakelock, PSM_DEEP_SLEEP);
    //禁止系统进入休眠态
    PSM_WakeLock(&g_psm_wakelock);
    //创建timer,10s超时后进入psm
    osTimerInit(&work_timer, work_timeout, 0, 10000, OS_TIMER_FLAG_ONE_SHOT);
    osTimerStartEX(&work_timer);
    //允许系统进入休眠态
    PSM_WakeUnlock(&g_psm_wakelock);
}

static void tc_psmthread(char argc, char **argv)
{
    //创建psm周期性唤醒线程
    osThreadAttr_t taskAttr = {"psmThread", osThreadDetached, NULL, 0U, NULL, 512, 30, 0U, 0U};
    osThreadNew(osPsmThreadEntry, NULL, &taskAttr);

}
NR_SHELL_CMD_EXPORT(tc_psmthread, tc_psmthread);
