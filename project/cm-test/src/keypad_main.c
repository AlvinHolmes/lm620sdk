/**
 * @file example_main.c
 * @brief Keypad 测试用例
 * @date 2025-03-12
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "cm_os.h"
#include "cm_keypad.h"
#include "cm_iomux.h"
#include "cm_gpio.h"
#include "cm_sys.h"
#include "cm_pm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* 消息队列配置 */
#define KEY_MSG_QUEUE_SIZE      10

/* 任务配置 */
#define KEY_TASK_STACK_SIZE     (4 * 1024)
#define KEY_TASK_PRIORITY       osPriorityNormal

/****************************************************************************
 * Private Types
 ****************************************************************************/

/**
 * @brief 按键事件消息结构
 */
typedef struct {
    cm_keypad_map_t key;
    cm_keypad_event_e event;
} key_event_msg_t;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void keypad_event_callback(cm_keypad_map_t key, cm_keypad_event_e event);
static void key_event_task(void *argument);
static int32_t test_keypad_init(void);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static osMessageQueueId_t g_key_msg_queue = NULL;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief 按键事件回调函数 (在中断上下文调用，只发送消息队列)
 */
static void keypad_event_callback(cm_keypad_map_t key, cm_keypad_event_e event)
{
    key_event_msg_t msg;

    if (g_key_msg_queue == NULL) {
        return;
    }

    msg.key = key;
    msg.event = event;

    osMessageQueuePut(g_key_msg_queue, &msg, 0, 0);
}

/**
 * @brief 按键事件处理任务
 */
static void key_event_task(void *argument)
{
    key_event_msg_t msg;
    osStatus_t status;
    const char *event_str;

    (void)argument;

    while (1) {
        status = osMessageQueueGet(g_key_msg_queue, &msg, NULL, osWaitForever);
        if (status == osOK) {
            event_str = (msg.event == CM_KEY_EVENT_PRESS) ? "PRESS" : "RELEASE";
            cm_printf("Key event: key=%u, event=%s\r\n", msg.key, event_str);
        }
    }
}

/**
 * @brief 初始化 keypad 引脚
 */
static void keypad_pin_init(void)
{
    /* 配置行引脚 (KEYOUT) - 2 行 */
    cm_iomux_set_pin_func(CM_GPIO_AON_4, CM_IOMUX_AON_4_MUX_KBR_2);
    cm_iomux_set_pin_func(CM_GPIO_AON_5, CM_IOMUX_AON_5_MUX_KBR_3);
    /* 行引脚配置上拉 */
    cm_iomux_set_pin_cmd(CM_GPIO_AON_4, CM_IOMUX_PINCMD3_PULL, CM_IOMUX_PINCMD3_FUNC2_PULL_HIGH);
    cm_iomux_set_pin_cmd(CM_GPIO_AON_5, CM_IOMUX_PINCMD3_PULL, CM_IOMUX_PINCMD3_FUNC2_PULL_HIGH);

    /* 配置列引脚 (KEYIN) - 3 列 */
    cm_iomux_set_pin_func(CM_GPIO_PD_0, CM_IOMUX_PD_0_MUX_KBC_0);
    cm_iomux_set_pin_func(CM_GPIO_PD_1, CM_IOMUX_PD_1_MUX_KBC_1);
    cm_iomux_set_pin_func(CM_GPIO_PD_2, CM_IOMUX_PD_2_MUX_KBC_2);
}

/**
 * @brief 测试 keypad 初始化
 */
static int32_t test_keypad_init(void)
{
    int32_t ret;
    /* 只有在数组中的键才会上报 */
    cm_keypad_kp_mkin_e col[3] = {CM_KP_MKI0, CM_KP_MKI1, CM_KP_MKI2};
    cm_keypad_kp_mkout_e row[2] = {CM_KP_MKO2, CM_KP_MKO3};
    cm_keypad_cfg_t keypad_cfg = {0};
    osThreadId_t thread_id;
    osThreadAttr_t thread_attr = {0};

    cm_printf("Keypad Test Init\r\n");

    /* 创建消息队列 */
    g_key_msg_queue = osMessageQueueNew(KEY_MSG_QUEUE_SIZE, sizeof(key_event_msg_t), NULL);
    if (g_key_msg_queue == NULL) {
        cm_printf("osMessageQueueNew failed\r\n");
        return -1;
    }

    /* 配置引脚 */
    keypad_pin_init();

    /* 注册回调 */
    ret = cm_keypad_register(keypad_event_callback);
    if (ret != 0) {
        cm_printf("cm_keypad_register failed: %d\r\n", ret);
        return -1;
    }
    cm_printf("cm_keypad_register success\r\n");

    /* 配置 keypad: 2 行 3 列 */
    keypad_cfg.cm_col = col;
    keypad_cfg.cm_row = row;
    keypad_cfg.cm_col_len = 3;
    keypad_cfg.cm_row_len = 2;

    ret = cm_keypad_config(&keypad_cfg);
    if (ret != 0) {
        cm_printf("cm_keypad_config failed: %d\r\n", ret);
        return -1;
    }
    cm_printf("cm_keypad_config success\r\n");

    /* 初始化 keypad */
    ret = cm_keypad_init();
    if (ret != 0) {
        cm_printf("cm_keypad_init failed: %d\r\n", ret);
        return -1;
    }
    cm_printf("cm_keypad_init success\r\n");

    /* 创建按键事件处理任务 */
    thread_attr.name = "key_event";
    thread_attr.stack_size = KEY_TASK_STACK_SIZE;
    thread_attr.priority = KEY_TASK_PRIORITY;

    thread_id = osThreadNew(key_event_task, NULL, &thread_attr);
    if (thread_id == NULL) {
        cm_printf("osThreadNew failed\r\n");
        return -1;
    }
    cm_printf("Key event task created\r\n");

    cm_printf("Keypad initialized, waiting for key events...\r\n");

    return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void cm_pm_powerkey_dummy(cm_powerkey_event_e event)
{
    cm_printf("cm_pm_powerkey_dummy: %d\r\n", event);
}

/**
 * @brief 主函数
 */
int main(void)
{
    /* 键盘只能在工作模式下启用 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_powerkey_regist_callback(cm_pm_powerkey_dummy);
    cm_pm_work_lock();

    /* 
        keypad 内部也会上锁，如果需要睡眠，除了这个锁要解锁外，还需要解锁 keypad 的锁
        cm_keypad_sleep(1);
     */

    cm_printf("CM Keypad Test\r\n");

    /* 等待系统稳定 */
    osDelay(100);

    if (test_keypad_init() != 0) {
        cm_printf("Keypad test init failed\r\n");
        return -1;
    }

    cm_printf("Keypad initialized, waiting for key events...\r\n");

    return 0;
}