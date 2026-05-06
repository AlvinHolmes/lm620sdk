/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_sysctrl.c
 *
 * @brief       实现系统控制配置接口
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
#include <drv_spinlock.h>
#include "drv_sysctrl.h"

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

void SYSCTRL_SetPdRegGen(SYSCTRL_PdRegGen regBit, uint32_t bitVal)
{
    ubase_t plevel;

    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(PD_SYSCTRL_ADDRBASE + ((uint32_t)regBit >> 16), bitVal);
        return;
    }

    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);

    Spinlock_HwLockIrq(REGLOCK_HWLOCK, &plevel);
    MODIFY_U32(PD_SYSCTRL_ADDRBASE + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
    Spinlock_HwUnlockIrq(REGLOCK_HWLOCK, plevel);
}

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
uint32_t SYSCTRL_GetPdRegGen(SYSCTRL_PdRegGen regBit)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        return READ_U32(PD_SYSCTRL_ADDRBASE + ((uint32_t)regBit >> 16));
    }

    return (READ_U32(PD_SYSCTRL_ADDRBASE + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

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

void SYSCTRL_SetAonRegGen(SYSCTRL_AonRegGen regBit, uint32_t bitVal)
{
    ubase_t plevel;

    //cannot config SPINLOCK
    if (regBit == SYSCTRL_REG_SHARED_DEVICE0 || regBit == SYSCTRL_REG_SHARED_DEVICE1) {
        osPrintf("Unexpected error: cfg aon spinlock with func SYSCTRL_SetAonRegGen\r\n");
        return;
    }

    if (((uint32_t)regBit & 0xff) == 32) {
        WRITE_U32(AON_SYSCTRL_ADDRBASE + ((uint32_t)regBit >> 16), bitVal);
        return;
    }

    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);

    Spinlock_HwLockIrq(REGLOCK_HWLOCK, &plevel);
    MODIFY_U32(AON_SYSCTRL_ADDRBASE + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
    Spinlock_HwUnlockIrq(REGLOCK_HWLOCK, plevel);
}

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
uint32_t SYSCTRL_GetAonRegGen(SYSCTRL_AonRegGen regBit)
{
    if (((uint32_t)regBit & 0xff) == 32) {
        return READ_U32(AON_SYSCTRL_ADDRBASE + ((uint32_t)regBit >> 16));
    }

    return (READ_U32(AON_SYSCTRL_ADDRBASE + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

#ifdef _CPU_AP
static int SYSCTRL_Init(void)
{
    //DMA axi QOS config
    SYSCTRL_SetPdRegGen(SYSCTRL_AWQOS_DMA, 8);
    SYSCTRL_SetPdRegGen(SYSCTRL_ARQOS_DMA, 4);
    WRITE_U32(AON_SLOCK1_ADDR, 0);
    WRITE_U32(AON_SLOCK0_ADDR, 0);
    return 0;
}
INIT_CORE_EXPORT(SYSCTRL_Init, OS_INIT_SUBLEVEL_HIGH);
#endif
