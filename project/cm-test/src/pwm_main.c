/**
 * @file example_main.c
 * @brief PWM测试例程
 * @date 2025-01-23
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/****************************************************************************
* Included Files
****************************************************************************/

#include <stdlib.h>
#include "cm_os.h"
#include "cm_sys.h"
#include "cm_pwm.h"
#include "cm_iomux.h"
#include "cm_gpio.h"
#include "cm_pm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
****************************************************************************/

/**
 * @brief PWM DEV_1 测试用例
 * @details 使用 AON_5 (PIN_23) 输出PWM, 周期10ms, 占空比50%
 */
static void pwm_test_case_dev1(void)
{
    int32_t ret;
    uint32_t period = 10000000;   /* 10ms = 10000000ns */
    uint32_t period_h = period / 2; /* 50% 占空比 */

    cm_printf("pwm_test_case_dev1 start ...\r\n");

    /* 配置引脚复用: AON_5 -> AON_PWM_1 */
    cm_iomux_set_pin_func(CM_GPIO_AON_5, CM_IOMUX_AON_5_MUX_AON_PWM_1);

    /* 打开PWM */
    ret = cm_pwm_open_ns(CM_PWM_DEV_1, period, period_h);
    if (ret != 0) {
        cm_printf("cm_pwm_open_ns failed, ret=%d\r\n", ret);
        return;
    }
    cm_printf("cm_pwm_open_ns success: period=%u, period_h=%u\r\n", period, period_h);

    /* 运行2秒 */
    cm_printf("PWM running for 2000ms ...\r\n");
    osDelay(2000);

    /* 关闭PWM */
    ret = cm_pwm_close(CM_PWM_DEV_1);
    if (ret != 0) {
        cm_printf("cm_pwm_close failed, ret=%d\r\n", ret);
    } else {
        cm_printf("cm_pwm_close success\r\n");
    }

    cm_printf("pwm_test_case_dev1 end\r\n");
}

/**
 * @brief PWM DEV_2 测试用例
 * @details 使用 PD_26 (PIN_14) 输出PWM, 周期10ms, 占空比50%
 */
static void pwm_test_case_dev2(void)
{
    int32_t ret;
    uint32_t period = 10000000;   /* 10ms = 10000000ns */
    uint32_t period_h = period / 2;  /* 50% 占空比 */

    cm_printf("pwm_test_case_dev2 start ...\r\n");

    /* 配置引脚复用: PD_26 -> PD_PWM */
    cm_iomux_set_pin_func(CM_GPIO_PD_26, CM_IOMUX_PD_26_MUX_PD_PWM);

    /* 打开PWM */
    ret = cm_pwm_open_ns(CM_PWM_DEV_2, period, period_h);
    if (ret != 0) {
        cm_printf("cm_pwm_open_ns failed, ret=%d\r\n", ret);
        return;
    }
    cm_printf("cm_pwm_open_ns success: period=%u, period_h=%u\r\n", period, period_h);

    /* 运行2秒 */
    cm_printf("PWM running for 2000ms ...\r\n");
    osDelay(2000);

    /* 关闭PWM */
    ret = cm_pwm_close(CM_PWM_DEV_2);
    if (ret != 0) {
        cm_printf("cm_pwm_close failed, ret=%d\r\n", ret);
    } else {
        cm_printf("cm_pwm_close success\r\n");
    }

    cm_printf("pwm_test_case_dev2 end\r\n");
}

/**
 * @brief PWM 占空比测试用例
 * @details 测试不同占空比: 25%, 50%, 75%
 */
static void pwm_test_case_duty(void)
{
    int32_t ret;
    uint32_t period = 10000000;   /* 10ms */
    uint32_t duty_cycle[] = {25, 50, 75};
    uint32_t i;

    cm_printf("pwm_test_case_duty start ...\r\n");

    /* 配置引脚复用: AON_5 -> AON_PWM_1 */
    cm_iomux_set_pin_func(CM_GPIO_AON_5, CM_IOMUX_AON_5_MUX_AON_PWM_1);

    for (i = 0; i < 3; i++) {
        uint32_t period_h = period * duty_cycle[i] / 100;

        cm_printf("Testing duty cycle: %u%%\r\n", duty_cycle[i]);

        ret = cm_pwm_open_ns(CM_PWM_DEV_1, period, period_h);
        if (ret != 0) {
            cm_printf("cm_pwm_open_ns failed, ret=%d\r\n", ret);
            return;
        }

        osDelay(1000);

        ret = cm_pwm_close(CM_PWM_DEV_1);
        if (ret != 0) {
            cm_printf("cm_pwm_close failed, ret=%d\r\n", ret);
        }

        osDelay(500);
    }

    cm_printf("pwm_test_case_duty end\r\n");
}

/**
 * @brief PWM DEV_2 频率测试用例
 * @details 测试不同频率: 100Hz, 10kHz, 50kHz
 */
static void pwm_test_case_dev2_freq(void)
{
    int32_t ret;
    uint32_t freq_arr[] = {100, 10000, 50000};
    uint32_t period_us;
    uint32_t i;

    cm_printf("pwm_test_case_dev2_freq start ...\r\n");

    /* 配置引脚复用: PD_26 -> PD_PWM */
    cm_iomux_set_pin_func(CM_GPIO_PD_26, CM_IOMUX_PD_26_MUX_PD_PWM);

    for (i = 0; i < 3; i++) {
        period_us = 1000000U / freq_arr[i];     /* 频率转周期(微秒) */
        uint32_t period = period_us * 1000U;    /* 转纳秒 */
        uint32_t period_h = period / 2;         /* 50% 占空比 */

        cm_printf("Testing freq: %uHz (period=%uus)\r\n", freq_arr[i], period_us);

        ret = cm_pwm_open_ns(CM_PWM_DEV_2, period, period_h);
        if (ret != 0) {
            cm_printf("cm_pwm_open_ns failed, ret=%d\r\n", ret);
            return;
        }

        osDelay(1000);

        ret = cm_pwm_close(CM_PWM_DEV_2);
        if (ret != 0) {
            cm_printf("cm_pwm_close failed, ret=%d\r\n", ret);
        }

        osDelay(500);
    }

    cm_printf("pwm_test_case_dev2_freq end\r\n");
}

int main(void)
{
    /* 避免睡眠模式导致输出异常 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_work_lock();

    cm_printf("PWM test start\r\n");

    /* 测试 DEV_1 */
    pwm_test_case_dev1();

    osDelay(1000);

    /* 测试 DEV_2 */
    pwm_test_case_dev2();

    osDelay(1000);

    /* 测试不同占空比 */
    pwm_test_case_duty();

    osDelay(1000);

    /* 测试 DEV_2 不同频率 */
    pwm_test_case_dev2_freq();

    cm_printf("PWM test completed\r\n");

    return 0;
}
