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
#ifndef __DRV_PWRKEY__
#define __DRV_PWRKEY__


#include <drv_common.h>

/**
 * @addtogroup PwrKey
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define PSM_KEYON_FINISH_INT    OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_95)

enum {
    PWR_CHARING = 0,
    PWR_NONCHARING = 1,
    PWR_KEY_CHARING_BOOT = 0,
    PWR_CHARING_BOOT = 1,
    PWR_KEY_BOOT = 2,
    PWR_KEY_CHARING_RSV = 3,
    PWR_NORMAL_STATE = 0,
    PWR_OFF_CHARING_STATE,
    PWR_ON_CHARING_STATE,
    PWR_INV_STATE,
};

typedef enum PWRKEY_Id{
    PWRKEY_SP,
    PWRKEY_LP_TRIG,  // 长按触发
    PWRKEY_LP,       // 长按完成
    PWRKEY_LLP_TRIG, // 长长按触发
    PWRKEY_LLP,      // 长长按完成
    PWRKEY_UP,
    PWRKEY_MAX_EVENT,
} PWRKEY_Id;

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef uint32_t (*SHUTDOWN_fp)(void);
typedef void (*PWRKEY_fp)(void *param);

typedef struct PWRKEY_Callback{
    PWRKEY_fp func;
    void *param;
} PWRKEY_Callback;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief           注册应用关机回调接口
 * @param[in]       fp                  关机回调函数指针
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void PWRKEY_RegisterShutdownCallback(SHUTDOWN_fp fp);
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
void PWRKEY_RegisterCallback(PWRKEY_Id id, PWRKEY_fp func, void *param);
#if defined(OS_USING_PM)
/**
 ************************************************************************************
 * @brief           短按唤醒时长
 * @param[in]       len             短按唤醒时长
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void PWRKEY_SetWakeupTime(uint16_t len);

/**
 ************************************************************************************
 * @brief           standby休眠时保存、清除当前状态
 * @param[in]       en 1:保存 0：清除
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void PWRKEY_PSMChargingState(uint8_t en);

#endif
#endif
/** @} */