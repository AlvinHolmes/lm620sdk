/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        extint.c
 *
 * @brief       extint管脚配置代码
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
#include <os_hw.h>
#include <drv_pcu.h>
#include <psm_sys.h>
#include <nvm.h>

#ifdef  OS_USING_PM
/************************************************************************************
 *                                 局部函数声明
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
static uint8_t g_wakeupKeep;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
static void Extint_IntHandler(int vector, void *param)
{
    //用户自定义中断回调接口
    PCU_WakeupIrqClrpending(vector);
}

static int Extint_init()
{
    uint8_t tmp = 0xFF;
    NVM_Read(NV_ITEM_ADDR(pubConfig.appConfig.wakeup_keep), &tmp, sizeof(g_wakeupKeep));
    g_wakeupKeep = tmp == 1 ? 1 : 0;

    //默认下降沿触发唤醒，用户可以自定义触发方式
    if (!PSM_IsSbyBoot())
        PCU_WakeupIrqRegister(SBY_EXT_INT, ICT_PCU_NEGATIVE_EDGE);
    else
        osInterruptWakeup(SBY_EXT_INT, 1);
    osInterruptConfig(SBY_EXT_INT, 2, IRQ_HIGH_LEVEL);
    osInterruptInstall(SBY_EXT_INT, Extint_IntHandler, NULL);
    osInterruptUnmask(SBY_EXT_INT);
    return 0;
}
INIT_MIDDLEWARE_EXPORT(Extint_init, OS_INIT_SUBLEVEL_HIGH);

static void prvExtIntModeGet(PSM_Mode *sleepMode, uint64_t *sleepTime)
{
    *sleepTime = PSM_SLEEPTIME_MAX;
    *sleepMode = PSM_IGNORE;
    if (g_wakeupKeep) {
        if (READ_BIT_U32(LSP_SBY_POR_RESET_CTL, 1, 11) == 0) {
            *sleepMode = PSM_IDLE;
        }
    }
}

PSM_IDLE_CALLBACK_DEFINE(prvExtIntModeGet, NULL, PSM_LEVEL_LOW);

#endif
