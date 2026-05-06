/**
 * @file main.c
 * @brief ELOOP 测试项目 - 开机自动运行（不需要联网）
 * @date 2026-02-12
 */

#include "cm_eloop.h"
#include "cm_sys.h"
#include "cm_os.h"
#include <stddef.h>
#include <stdint.h>

/* 声明系统相关函数 */
extern int cm_eloop_init_default(void);

/* 测试事件回调函数 */
static void test_event_callback(cm_eloop_event_handle_t event, void *user_data)
{
    (void)event;
    (void)user_data;
    cm_printf("[ELOOP] Test event callback called\n");
}

/* 自动运行ELOOP测试 */
static void run_eloop_test(void)
{
    cm_eloop_handle_t test_eloop = NULL;
    cm_eloop_event_handle_t test_event = NULL;

    cm_printf("[ELOOP] Starting ELOOP test...\n");

    /* 创建eloop */
    test_eloop = cm_eloop_create(10);
    if (test_eloop == NULL) {
        cm_printf("[ELOOP] Failed to create eloop\n");
        return;
    }
    cm_printf("[ELOOP] Eloop created successfully\n");

    /* 延迟一下 */
    osDelay(100);

    /* 注册事件 */
    test_event = cm_eloop_register_event(test_eloop, test_event_callback, NULL);
    if (test_event == NULL) {
        cm_printf("[ELOOP] Failed to register event\n");
        cm_eloop_delete(test_eloop);
        return;
    }
    cm_printf("[ELOOP] Event registered\n");

    /* 延迟一下 */
    osDelay(100);

    /* 投递事件 */
    int ret = cm_eloop_post_event(test_event);
    if (ret < 0) {
        cm_printf("[ELOOP] Failed to post event\n");
    } else {
        cm_printf("[ELOOP] Event posted\n");
    }

    /* 延迟一下 */
    osDelay(100);

    /* 等待事件 */
    ret = cm_eloop_wait_event(test_eloop, 1000);
    if (ret < 0) {
        cm_printf("[ELOOP] Wait event error\n");
    } else if (ret == 0) {
        cm_printf("[ELOOP] Wait timeout\n");
    } else {
        cm_printf("[ELOOP] Event received\n");
    }

    /* 清理 */
    cm_eloop_delete(test_eloop);
    cm_printf("[ELOOP] Eloop test completed\n");
}

int main(void)
{
    cm_eloop_init_default();

    /* 运行ELOOP测试 */
    run_eloop_test();

    return 0;
}