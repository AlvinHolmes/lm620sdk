/**
 * @file example_main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-12-22
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
#include "cm_gpio.h"
#include "cm_iomux.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define TEST_IRQ        0   /* 测试中断的时候 打开该宏 测试GPIO的读写的时候关闭该宏                         */
/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
static int irq_count = 0;
/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
****************************************************************************/
static volatile int g_irq_is_success = 0;
/*该例程实现gpio控制LED灯点亮功能，参照如下例程*/
static void cm_test_gpio_irq_callback(void)
{
    g_irq_is_success = 1;
}

void gpio_test_case_1(void)
{
    cm_gpio_cfg_t cfg;
    cm_gpio_level_e level;

    cm_gpio_level_e level1 = CM_GPIO_LEVEL_HIGH;
    cm_iomux_func_e fun = CM_IOMUX_FUNC_FUNCTION0;
    cm_iomux_func_e fun1 = 0;
    cm_gpio_direction_e dir = 0;
    cm_gpio_pull_e pull = 0;

    cfg.direction = CM_GPIO_DIRECTION_OUTPUT;
    cfg.mode = CM_GPIO_MODE_NUM;
    cfg.pull = CM_GPIO_PULL_NONE;

    cm_printf("gpio_test_case_1 start ...\r\n");

    /* pin 和 test_gpio 指向同一个引脚资源 */
    cm_iomux_set_pin_func(CM_GPIO_AON_7, fun);
    cm_iomux_get_pin_func(CM_GPIO_AON_7, &fun1);
    if (fun1 == fun) {
        cm_printf("cm_iomux_set_pin_func success fun = %d \r\n", fun1);
    } else {
        cm_printf("cm_iomux_set_pin_func failed \r\n");
        return;
    }

    cm_gpio_init(CM_GPIO_AON_7, &cfg);
    cm_gpio_set_level(CM_GPIO_AON_7, level1);
    /* 延迟一定时间避免看不到 */
    osDelay(1000);
    cm_gpio_get_level(CM_GPIO_AON_7, &level);
    if (level == level1) {
        cm_printf("cm_gpio_get_level success level = %d \r\n", level);
    } else {
        cm_printf("cm_gpio_get_level failed \r\n");
        return;
    }

    cm_gpio_get_direction(CM_GPIO_AON_7, &dir);
    if (dir == cfg.direction) {
        cm_printf("cm_gpio_get_direction success dir = %d \r\n", dir);
    } else {
        cm_printf("cm_gpio_get_direction failed \r\n");
        return;
    }

    cm_gpio_get_pull(CM_GPIO_AON_7, &pull);
    if (pull == cfg.pull) {
        cm_printf("cm_gpio_get_pull success dir = %d \r\n", dir);
    } else {
        cm_printf("cm_gpio_get_pull failed \r\n");
        return;
    }

    cm_gpio_ioctl(CM_GPIO_AON_7, CM_GPIO_CMD_GET_DIRECTION, &dir);
    cm_printf("read gpio_%d dir = %d\r\n", 2, dir);

    cm_gpio_ioctl(CM_GPIO_AON_7, CM_GPIO_CMD_GET_LEVEL, &level);
    cm_printf("read gpio_%d level = %d\r\n", 2, level);
    cm_gpio_deinit(CM_GPIO_AON_7);

    cm_printf("gpio_test_case_1 end ...\r\n");
}

void gpio_test_case_2(void)
{
    (void)irq_count;
    cm_gpio_cfg_t cfg;
#if 0
    uint32_t timeout = 0;
    cm_gpio_level_e level;

    cm_printf("gpio_test_case_2 start ...\r\n");
    //GPIO IRQ test
    cm_iomux_set_pin_func(CM_GPIO_AON_7, CM_IOMUX_FUNC_FUNCTION0);//初始化之前一定要先设置引脚复用
    cfg.pull = CM_GPIO_PULL_DOWN;
    cfg.direction = CM_GPIO_DIRECTION_INPUT;
    cm_gpio_init(CM_GPIO_AON_7, &cfg);
    cm_gpio_interrupt_disable(CM_GPIO_AON_7);
    cm_gpio_interrupt_register(CM_GPIO_AON_7, cm_test_gpio_irq_callback);
    cm_gpio_interrupt_enable(CM_GPIO_AON_7, CM_GPIO_IT_EDGE_RISING);
    osDelay(100);
    while ((irq_count <= 0) && (timeout < 600)) {
        cm_gpio_get_level(CM_GPIO_AON_7, &level);
        cm_printf("get gpio_%d level = %d\r\n", CM_GPIO_AON_7, level);
        cm_gpio_set_level(CM_GPIO_AON_7, (level == CM_GPIO_LEVEL_LOW) ? CM_GPIO_LEVEL_HIGH : CM_GPIO_LEVEL_LOW);
        osDelay(2000);
        timeout += 5;
    }
    cm_printf(" gpio irq_count = %d!\n", irq_count);
    cm_gpio_interrupt_disable(CM_GPIO_AON_7);

    cm_gpio_deinit(CM_GPIO_AON_7);
#endif
    cm_printf("gpio_test_case_2 start ...\r\n");
    /*PIN复用*/
    /* 连接 PD_GPIO_1 和 PD_GPIO_2 */
    cm_iomux_set_pin_func(CM_GPIO_PD_1, CM_IOMUX_FUNC_FUNCTION0);
    cm_iomux_set_pin_func(CM_GPIO_PD_2, CM_IOMUX_FUNC_FUNCTION0);

    cfg.direction = CM_GPIO_DIRECTION_OUTPUT;
    cm_gpio_init(CM_GPIO_PD_1, &cfg);
    cm_gpio_set_level(CM_GPIO_PD_1, CM_GPIO_LEVEL_LOW);

    cfg.direction = CM_GPIO_DIRECTION_INPUT;
    cm_gpio_init(CM_GPIO_PD_2, &cfg);
    cm_gpio_interrupt_register(CM_GPIO_PD_2, cm_test_gpio_irq_callback);
    cm_gpio_interrupt_enable(CM_GPIO_PD_2, CM_GPIO_IT_EDGE_RISING);

    osDelay(100);

    cm_gpio_set_level(CM_GPIO_PD_1, CM_GPIO_LEVEL_HIGH);

    cm_printf("gpio_test_case_2 wait irq\r\n");
    while (g_irq_is_success == 0) {
        osDelay(100);
    }

    // cm_printf("gpio_test_case_2 pass\r\n");
    cm_gpio_interrupt_disable(CM_GPIO_PD_2);
    cm_printf("gpio_test_case_2 pass\r\n");
}

int main(void)
{
    gpio_test_case_1();
    gpio_test_case_2();
    return 0;
}
