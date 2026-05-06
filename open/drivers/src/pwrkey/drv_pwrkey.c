/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_pwrkey.c
 *
 * @brief       实现电源按键及充电接口
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2024-08-29     ICT Team          创建
 ************************************************************************************
 */
/************************************************************************************
 *                                 头文件定义
 ************************************************************************************/
#include <os.h>
#include <os_hw.h>
#include <os_workqueue.h>
#include <drv_common.h>
#include <drv_soc.h>
#include <drv_psm_sys.h>
#include <drv_pcu.h>
#include <drv_pmu.h>
#include <drv_pwrkey.h>

#include <psm_common.h>
#if defined(OS_USING_PM)
#include <psm_wakelock.h>
#include <psm_sys.h>
#endif

/************************************************************************************
 *                                 变量定义
 ************************************************************************************/
static volatile SHUTDOWN_fp g_sd_fp = OS_NULL;
static volatile PWRKEY_Callback g_pwrkey_fp[PWRKEY_MAX_EVENT] = {0};
#if defined(OS_USING_PM)
#define MS_PER_S                1000
static struct osWork g_wakeup_work = {0};
static WakeLock g_sp_wakelock;
static uint32_t g_keyup_time = 3000;
static struct osTimer g_sp_wakelock_timer;
#endif
static volatile uint8_t g_keyint_trig = 0;
static volatile uint8_t g_pwrdnSby = 0;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
#if defined(OS_USING_PM)
static void PWRKEY_Wakeup(void *data)
{
    if (osTickFromMs(g_keyup_time) != 0) {
        int msr = osInterruptDisable();
        if (g_sp_wakelock.refCount != 0)
            PSM_WakeUnlock(&g_sp_wakelock);
        PSM_WakeLock(&g_sp_wakelock);
        osInterruptEnable(msr);
        g_sp_wakelock_timer.timeout = osTickFromMs(g_keyup_time);
        osTimerStartEX(&g_sp_wakelock_timer);
    }
}
static void PWRKEY_WakeupTimeout(void *data)
{
    PSM_WakeUnlock(&g_sp_wakelock);
}

void PWRKEY_WakeupCb(void)
{
    osInterruptUnmask(SBY_SP_INT);
    //sp wakeup sby, need unlock
    if (g_sp_wakelock.refCount) {
        PSM_WakeUnlock(&g_sp_wakelock);
    }
}

/**
 ************************************************************************************
 * @brief           短按唤醒时长
 * @param[in]       len             短按唤醒时长
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void PWRKEY_SetWakeupTime(uint16_t len)
{
    g_keyup_time = len * MS_PER_S;
}

/**
 ************************************************************************************
 * @brief           standby休眠时保存、清除当前状态
 * @param[in]       en 1:保存 0：清除
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void PWRKEY_PSMChargingState(uint8_t en)
{
    PCU_SetRegGenPcuSby(PCU_REG_PWRDOWN_SBY, 0);
}

#endif

/**
 ************************************************************************************
 * @brief           注册应用关机回调接口
 * @param[in]       fp                  关机回调函数指针
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void PWRKEY_RegisterShutdownCallback(SHUTDOWN_fp fp)
{
    g_sd_fp = fp;
}

static void PWRKEY_Shutdown(void *data)
{
    if (g_sd_fp)
        g_sd_fp();
}

/**
 ************************************************************************************
 * @brief           注册按键中断回调接口
 * @param[in]       id             按键中断id
 * @param[in]       func           按键中断回调
 * @param[in]       param          按键中断回调入参
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void PWRKEY_RegisterCallback(PWRKEY_Id id, PWRKEY_fp func, void *param)
{
    int msr = osInterruptDisable();
    g_pwrkey_fp[id].func  = func;
    g_pwrkey_fp[id].param = param;
    osInterruptEnable(msr);
}

static void PWRKEY_DefaultCallbackSP(void *param)
{
#if defined(OS_USING_PM)
    osWorkSubmit(&g_wakeup_work, 0);
#endif
}

static void PWRKEY_DefaultCallbackLP(void *param)
{
    PCU_ShutDown();
}

static void PWRKEY_IrqIsr(int irq_id, void* irq_data)
{
    osPrintf("PWRKEY_IrqIsr irq_id %d \r\n", irq_id);
    switch (irq_id) {
        case SBY_SP_INT:
        #if defined(OS_USING_PM)
            PCU_WakeupIrqClrpending(irq_id);
            if (!PCU_GetRegGenPcuSby(PCU_REG_KEYON)) {
                PSM_WakeLock(&g_sp_wakelock);
                g_keyint_trig = SBY_SP_INT;
            } else {
                osPrintf("Keyon Up without Interrupt, SP handler\r\n");
                //keyon up already，case: keyon press and up  trigger when sys in deepsleep
                if (g_pwrkey_fp[PWRKEY_SP].func)
                    g_pwrkey_fp[PWRKEY_SP].func(g_pwrkey_fp[PWRKEY_SP].param);
                if (g_pwrkey_fp[PWRKEY_UP].func)
                    g_pwrkey_fp[PWRKEY_UP].func(g_pwrkey_fp[PWRKEY_UP].param);
                g_keyint_trig = PSM_KEYON_FINISH_INT;
                g_pwrdnSby = 0;
            }
        #else
             g_keyint_trig = SBY_SP_INT;
        #endif
            break;
        case SBY_LP_INT:
        #if defined(OS_USING_PM)
            PCU_WakeupIrqClrpending(irq_id);
            if (!PCU_GetRegGenPcuSby(PCU_REG_KEYON)) {
                g_keyint_trig = SBY_LP_INT;
                if (g_pwrkey_fp[PWRKEY_LP_TRIG].func)
                    g_pwrkey_fp[PWRKEY_LP_TRIG].func(g_pwrkey_fp[PWRKEY_LP_TRIG].param);
            } else {
                PSM_WakeUnlock(&g_sp_wakelock);
                osPrintf("Keyon Up without Interrupt, LP handler: %d\r\n", g_pwrdnSby);
                //keyon up already，case: keyon press and up  trigger when sys in deepsleep
                if (g_pwrkey_fp[PWRKEY_LP].func && g_pwrdnSby == 0)
                    g_pwrkey_fp[PWRKEY_LP].func(g_pwrkey_fp[PWRKEY_LP].param);
                if (g_pwrkey_fp[PWRKEY_UP].func && g_pwrdnSby == 0)
                    g_pwrkey_fp[PWRKEY_UP].func(g_pwrkey_fp[PWRKEY_UP].param);
                g_keyint_trig = PSM_KEYON_FINISH_INT;
                g_pwrdnSby = 0;
            }
        #else
            g_keyint_trig = SBY_LP_INT;
            if (g_pwrkey_fp[PWRKEY_LP_TRIG].func)
                g_pwrkey_fp[PWRKEY_LP_TRIG].func(g_pwrkey_fp[PWRKEY_LP_TRIG].param);
        #endif
            break;
        case SBY_LLP_INT:
        #if defined(OS_USING_PM)
            PCU_WakeupIrqClrpending(irq_id);
            if (!PCU_GetRegGenPcuSby(PCU_REG_KEYON)) {
                g_keyint_trig = SBY_LLP_INT;
            if (g_pwrkey_fp[PWRKEY_LLP_TRIG].func)
                g_pwrkey_fp[PWRKEY_LLP_TRIG].func(g_pwrkey_fp[PWRKEY_LLP_TRIG].param);
            } else {
                PSM_WakeUnlock(&g_sp_wakelock);
                osPrintf("Keyon Up without Interrupt, LLP handler\r\n");
                //keyon up already，case: keyon press and up  trigger when sys in deepsleep
                if (g_pwrkey_fp[PWRKEY_LLP].func)
                    g_pwrkey_fp[PWRKEY_LLP].func(g_pwrkey_fp[PWRKEY_LLP].param);
                if (g_pwrkey_fp[PWRKEY_UP].func)
                    g_pwrkey_fp[PWRKEY_UP].func(g_pwrkey_fp[PWRKEY_UP].param);
                g_keyint_trig = PSM_KEYON_FINISH_INT;
                g_pwrdnSby = 0;
            }
        #else
            g_keyint_trig = SBY_LLP_INT;
            if (g_pwrkey_fp[PWRKEY_LLP_TRIG].func)
                g_pwrkey_fp[PWRKEY_LLP_TRIG].func(g_pwrkey_fp[PWRKEY_LLP_TRIG].param);
        #endif
            break;
        case PSM_KEYON_FINISH_INT:
        #if defined(OS_USING_PM)
            if (g_keyint_trig != PSM_KEYON_FINISH_INT && g_keyint_trig != 0)
                PSM_WakeUnlock(&g_sp_wakelock);
        #endif
            if (g_keyint_trig == SBY_SP_INT && g_pwrkey_fp[PWRKEY_SP].func) {
                g_pwrkey_fp[PWRKEY_SP].func(g_pwrkey_fp[PWRKEY_SP].param);
            } else if (g_keyint_trig == SBY_LP_INT && g_pwrkey_fp[PWRKEY_LP].func) {
                if (g_pwrdnSby == 0)
                    g_pwrkey_fp[PWRKEY_LP].func(g_pwrkey_fp[PWRKEY_LP].param);
            } else if(g_keyint_trig == SBY_LLP_INT && g_pwrkey_fp[PWRKEY_LLP].func) {
                g_pwrkey_fp[PWRKEY_LLP].func(g_pwrkey_fp[PWRKEY_LLP].param);
            }
            if (g_keyint_trig != PSM_KEYON_FINISH_INT && g_pwrkey_fp[PWRKEY_UP].func)
                g_pwrkey_fp[PWRKEY_UP].func(g_pwrkey_fp[PWRKEY_UP].param);
            g_keyint_trig = 0;
            g_pwrdnSby = 0;
            break;
        default :
            osPrintf("unexpect PWRKEY_IrqIsr irq_id %d \r\n", irq_id);
            break;
    }

}

static void PWRKEY_IrqInit(void)
{
    if (PCU_GetRegGenPcuSby(PCU_REG_PWRDOWN_SBY) && PCU_GetRegGenPcuSby(PCU_REG_CHIP_SBY_MODE)) {
        if (!PCU_GetRegGenPcuSby(PCU_REG_KEYON))
            g_pwrdnSby = 1;
    }
    PCU_SetRegGenPcuSby(PCU_REG_PWRDOWN_SBY, 0);

    PSM_SET_BIT(LSP_SBY_KEY_DETECT, KEY_CLKGATE_OFFSET);

#if defined(OS_USING_PM)
    PSM_WakelockInit(&g_sp_wakelock, PSM_DEEP_SLEEP);
    osWorkInit(&g_wakeup_work, PWRKEY_Wakeup, NULL, OS_FALSE);
    osTimerInit(&g_sp_wakelock_timer, PWRKEY_WakeupTimeout, NULL, g_keyup_time, OS_TIMER_FLAG_ONE_SHOT);
#endif
#if defined(OS_USING_PM)
    osInterruptMask(SBY_SP_INT);
    if (!PSM_IsSbyBoot()) {
        PCU_SbyCallbackRegister(SBY_SPINT_EVENTID, PWRKEY_WakeupCb);
        PCU_WakeupIrqRegister(SBY_SP_INT, ICT_PCU_POSITIVE_EDGE);
    } else {
        osInterruptWakeup(SBY_SP_INT, 1);
    }
    osInterruptConfig(SBY_SP_INT, 2, IRQ_HIGH_LEVEL);
#else
    osInterruptConfig(SBY_SP_INT, 2, IRQ_POSITIVE_EDGE);
#endif
    osInterruptInstall(SBY_SP_INT, PWRKEY_IrqIsr, NULL);

    osInterruptUnmask(SBY_SP_INT);

    PMU_FuncCfg(LP_SHUTDOWN_ID, 0);
#if defined(OS_USING_PM)
    if (!PSM_IsSbyBoot()) {
        PCU_WakeupIrqRegister(SBY_LP_INT, ICT_PCU_POSITIVE_EDGE);
    } else {
        osInterruptWakeup(SBY_LP_INT, 1);
    }
    osInterruptConfig(SBY_LP_INT, 2, IRQ_HIGH_LEVEL);
#else
    osInterruptConfig(SBY_LP_INT, 2, IRQ_POSITIVE_EDGE);
#endif
    osInterruptInstall(SBY_LP_INT, PWRKEY_IrqIsr, NULL);
    osInterruptUnmask(SBY_LP_INT);

#ifndef PMU_LLP_SW_RESET
    PMU_FuncCfg(LLP_SHUTDOWN_ID, 1);
    PMU_FuncCfg(LLP_RESTART_ID, 1);
#else
    osInterruptConfig(SBY_LLP_INT, 2, IRQ_POSITIVE_EDGE);
    osInterruptInstall(SBY_LLP_INT, PWRKEY_IrqIsr, NULL);
    osInterruptUnmask(SBY_LLP_INT);
#endif

    osInterruptConfig(PSM_KEYON_FINISH_INT, 1, IRQ_POSITIVE_EDGE);
    osInterruptInstall(PSM_KEYON_FINISH_INT, PWRKEY_IrqIsr, NULL);
    osInterruptUnmask(PSM_KEYON_FINISH_INT);

    PWRKEY_RegisterCallback(PWRKEY_SP, PWRKEY_DefaultCallbackSP, NULL);
    PWRKEY_RegisterCallback(PWRKEY_LP, PWRKEY_DefaultCallbackLP, NULL);
}

static int PWRKEY_Init(void)
{
    PWRKEY_IrqInit();
    return 0;
}
INIT_DEVICE_EXPORT(PWRKEY_Init, OS_INIT_SUBLEVEL_MIDDLE);


