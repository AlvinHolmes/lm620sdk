/**
 * @file main.c
 * @brief PING 测试项目 - 开机自动运行
 * @date 2026-02-12
 */

#include "cm_ping.h"
#include "cm_sys.h"
#include "cm_os.h"
#include "cm_modem_info.h"
#include "cm_modem.h"
#include <stddef.h>
#include <stdint.h>

/* 声明系统相关函数 */
extern int cm_eloop_init_default(void);

/* 删除手动定义的 cm_cereg_state_t，使用头文件中的定义 */

/* PING回调函数 */
static void ping_callback(cm_ping_cb_type_e type, cm_ping_cb_result_e result, cm_ping_reply_t *resp, void *user_data)
{
    (void)user_data;

    if (type == 0) { // CM_PING_CB_TYPE_PING_ONCE
        cm_printf("[PING] PING once: %s, len=%d, time=%d, ttl=%d\n",
            (char *)((char*)resp + 8), *(int *)((char*)resp + 4), *(int *)((char*)resp + 12), *(int *)((char*)resp + 16));
    } else if (type == 1) { // CM_PING_CB_TYPE_PING_TOTAL
        cm_printf("[PING] PING total: %s, send=%d, recv=%d, lost=%d, rtt_max=%d, rtt_min=%d, rtt_avg=%d\n",
            (char *)((char*)resp + 24), *(int *)((char*)resp), *(int *)((char*)resp + 4),
            *(int *)((char*)resp + 8), *(int *)((char*)resp + 12), *(int *)((char*)resp + 16),
            *(int *)((char*)resp + 20));
    } else { // CM_PING_CB_TYPE_PING_ERROR
        cm_printf("[PING] PING error: %d\n", result);
    }
}

/* 等待联网成功 */
static void wait_for_network(void)
{
    cm_cereg_state_t cereg = {0};
    int retry_count = 0;
    const int max_retries = 120; // 最多等待2分钟

    cm_printf("[PING] Waiting for network to be ready...\n");

    while (retry_count < max_retries) {
        cm_modem_get_cereg_state(&cereg);
        if (cereg.state == 1) {
            cm_printf("[PING] Network is ready!\n");
            return;
        }
        cm_printf("[PING] Waiting for network... (state=%d, retry=%d/%d)\n", cereg.state, retry_count + 1, max_retries);
        osDelay(1000); // 等待1秒
        retry_count++;
    }

    cm_printf("[PING] Warning: Network timeout, proceeding anyway...\n");
}

/* 自动运行PING测试 */
static void run_ping_test(void)
{
    cm_printf("[PING] Starting PING test...\n");

    /* 配置PING参数 */
    cm_ping_cfg_t ping_cfg;
    ping_cfg.cid = 0;
    ping_cfg.host = "www.baidu.com";
    ping_cfg.packet_len = 16;
    ping_cfg.ping_num = 4;
    ping_cfg.timeout = 10;

    /* 初始化PING */
    int ret = cm_ping_init(&ping_cfg, ping_callback);
    if (ret != 0) {
        cm_printf("[PING] cm_ping_init() error\n");
        return;
    }

    /* 延迟一下再启动PING */
    osDelay(100);

    /* 启动PING */
    ret = cm_ping_start("ping_test");
    if (ret != 0) {
        cm_printf("[PING] cm_ping_start() error\n");
    } else {
        cm_printf("[PING] PING started\n");
    }
}

int main(void)
{
    cm_eloop_init_default();

    /* 等待联网成功 */
    wait_for_network();

    /* 运行PING测试 */
    run_ping_test();

    return 0;
}