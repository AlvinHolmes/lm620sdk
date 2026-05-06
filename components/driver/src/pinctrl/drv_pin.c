/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_pin.c
 *
 * @brief       pin驱动实现.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-04-21     ict team          创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <os.h>
#include <os_hw.h>
#include <drv_pin.h>
#include "drv_pin_prv.h"
#include <drv_soc.h>
#include <drv_psm_sys.h>
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 变量定义
 ************************************************************************************/
extern const struct PIN_CTRL_Res g_PIN_Ctrl_Res[ 2 ];
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

/**
 ***********************************************************************************************************************
 * @brief           设置pin寄存器
 *
 * @param[in]       reg             寄存器基地址
 * @param[in]       regBit          指定寄存器的组成包括地址，偏移以及占的位数
 * @param[in]       bitVal          设置的值
 *
 * @return          void
 *
 ***********************************************************************************************************************
 */
void RBIT_SetPadCtrl(uint32_t regBase,uint32_t regBit, uint32_t bitVal)
{
    OS_ASSERT((bitVal & (~((0x1 << ((uint32_t)regBit & 0xff)) - 1))) == 0);

    MODIFY_U32(regBase + ((uint32_t)regBit >> 16),
               ((0x1 << ((uint32_t)regBit & 0xff)) - 1) << (((uint32_t)regBit >> 8) & 0xff),
               bitVal << (((uint32_t)regBit >> 8) & 0xff));
}
/**
 ***********************************************************************************************************************
 * @brief           获取pin寄存器某些BIT
 *
 * @param[in]       reg             寄存器基地址
 * @param[in]       regBit          指定寄存器的组成包括地址，偏移以及占的位数
 *
 * @return          uint32_t
 * @retval          uint32_t        获取寄存器的值
 ***********************************************************************************************************************
 */
uint32_t RBIT_GetPadCtrl(uint32_t regBase, uint32_t regBit)
{
    return (READ_U32(regBase + ((uint32_t)regBit >> 16)) >> (((uint32_t)regBit >> 8) & 0xff))
           & ((0x1 << ((uint32_t)regBit & 0xff)) - 1);
}

/**
 ************************************************************************************
 * @brief           设置pin复用
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       func           复用功能
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
void PIN_SetMux(const struct PIN_Res *pResource, uint8_t func)
{
    void *regBase = g_PIN_Ctrl_Res[pResource->gpioPort].regBase;
    uint32_t func_sel = pResource->pinConfig.func;

    RBIT_SetPadCtrl((uint32_t)regBase, func_sel, func);
}

/**
 ************************************************************************************
 * @brief           获取pin复用
 *
 * @param[in]       pResource      PIN资源句柄
 *
 * @return          复用功能
 * @retval          uint32_t
 ************************************************************************************
*/
uint32_t PIN_GetMux(const struct PIN_Res *pResource)
{
    void *regBase = g_PIN_Ctrl_Res[pResource->gpioPort].regBase;
    uint32_t func_sel = pResource->pinConfig.func;

    return RBIT_GetPadCtrl((uint32_t)regBase, func_sel);
}

/**
 ************************************************************************************
 * @brief           获取pin上下拉
 *
 * @param[in]       pResource      PIN资源句柄
 *
 * @return          获取上拉、下拉、浮空
 * @retval          PIN_PL
 ************************************************************************************
*/
PIN_PL PIN_GetPull(const struct PIN_Res *pResource)
{
    void *regBase = g_PIN_Ctrl_Res[pResource->gpioPort].regBase;
    uint32_t  pin_pull = pResource->pinConfig.ps;
    uint32_t  pin_pe = pResource->pinConfig.pe;

    if(RBIT_GetPadCtrl((uint32_t)regBase, pin_pe) == 1 && RBIT_GetPadCtrl((uint32_t)regBase, pin_pull) == 1)
    {
        return PULL_UP;
    }
    else if(RBIT_GetPadCtrl((uint32_t)regBase, pin_pe) == 1 && RBIT_GetPadCtrl((uint32_t)regBase, pin_pull) == 0)
    {
        return PULL_DOWN;
    }
    else
    {
        return PULL_NONE;
    }
}

/**
 ************************************************************************************
 * @brief           设置施密特触发
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       is             是否使能施密特触发
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
void PIN_SetIS(const struct PIN_Res *pResource, uint8_t is)
{
    void *regBase = g_PIN_Ctrl_Res[pResource->gpioPort].regBase;
    uint32_t is_sel = pResource->pinConfig.is;

    RBIT_SetPadCtrl((uint32_t)regBase, is_sel, is);
}

/**
 ************************************************************************************
 * @brief           设置上下拉使能
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       pull           上下拉
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
void PIN_SetPull(const struct PIN_Res *pResource, PIN_PL pull)
{
    void *regBase = g_PIN_Ctrl_Res[pResource->gpioPort].regBase;
    uint32_t sel = pResource->pinConfig.pe;
    if(pull == PULL_NONE)
    {
        RBIT_SetPadCtrl((uint32_t)regBase, sel, 0);
    } else
    {
        RBIT_SetPadCtrl((uint32_t)regBase, sel, 1);
        sel = pResource->pinConfig.ps;
        if(pull == PULL_DOWN)
        {
            RBIT_SetPadCtrl((uint32_t)regBase, sel, 0);
        }else if(pull == PULL_UP)
        {
            RBIT_SetPadCtrl((uint32_t)regBase, sel, 1);
        }
    }
}

/**
 ************************************************************************************
 * @brief           设置驱动能力
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       drvCap         驱动等级
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
void PIN_SetDrCap(const struct PIN_Res *pResource, PIN_DR drvCap)
{
    void *regBase = g_PIN_Ctrl_Res[pResource->gpioPort].regBase;
    uint32_t sel = pResource->pinConfig.drvCap;
    RBIT_SetPadCtrl((uint32_t)regBase, sel, drvCap);
}

/**
 ************************************************************************************
 * @brief           设置slew rate
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       drvCap         slew rate
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
void PIN_SetSlewRate(const struct PIN_Res *pResource, PIN_SLEW_RATE slewRate)
{

    void *regBase = g_PIN_Ctrl_Res[pResource->gpioPort].regBase;
    uint32_t sel = pResource->pinConfig.slewRate;
    RBIT_SetPadCtrl((uint32_t)regBase, sel, slewRate);
}

/**
 ************************************************************************************
 * @brief           设置输入还是输出
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       ie             input enable
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
void PIN_SetIE(const struct PIN_Res *pResource, PIN_IE ie)
{

  OS_ASSERT(pResource->gpioPort == GPIO_PORT_AON);
  void    *regBase = g_PIN_Ctrl_Res[ pResource->gpioPort ].regBase;
  uint32_t sel     = pResource->pinConfig.ie;
  if(ie ==  PIN_INPUT_DISABLE)
  {
    RBIT_SetPadCtrl((uint32_t)regBase, sel, 0);
  }
  else if(ie ==  PIN_INPUT_ENABLE)
  {
    RBIT_SetPadCtrl((uint32_t)regBase, sel, 1);
  }
}

#ifdef OS_USING_PM
static int PD_PADCTRL_Suspend(void *param, PSM_Mode mode, uint32_t *save_addr)
{
    uint32_t *addr = (uint32_t *)save_addr;
    int       i    = 0;
    if (mode == PSM_DEEP_SLEEP)
    {

        for (i = 0; i < 16; i++)
        {
            *((uint32_t *)(addr)+i)= READ_U32(g_PIN_Ctrl_Res[1].regBase + i*4);
        }

    }
  return i*4;
}

static int PD_PADCTRL_Resume(void *param, PSM_Mode mode, uint32_t *save_addr)
{
    uint32_t *addr = (uint32_t *)save_addr;
    int       i    = 0;
    if (mode == PSM_DEEP_SLEEP)
    {
        for (i = 0; i < 16; i++)
        {
          WRITE_U32(g_PIN_Ctrl_Res[ 1 ].regBase + i * 4, *((uint32_t *)(addr) + i));
        }
    }
    return i*4;
}


static PSM_DpmOps pd_padctrl_dpmops = {
    .PsmSuspendNoirq = PD_PADCTRL_Suspend,
    .PsmResumeNoirq = PD_PADCTRL_Resume,
};

PSM_CMNDPM_INFO_DEFINE(pin, pd_padctrl_dpmops, OS_NULL, PSM_CMNDEV_PAD);
#endif