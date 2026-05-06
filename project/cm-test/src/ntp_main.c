/**
 * @file main.c
 * @brief NTP 测试项目 - 开机自动运行
 * @date 2026-02-12
 */

#include "cm_ntp.h"
#include "cm_sys.h"
#include "cm_os.h"
#include "cm_sys.h"
#include "cm_modem_info.h"
#include "cm_modem.h"
#include <stddef.h>
#include <stdint.h>

/* 声明系统相关函数 */
extern int cm_eloop_init_default(void);

/* 删除手动定义的 cm_cereg_state_t，使用头文件中的定义 */

/* NTP回调函数 */
static void ntp_callback(cm_ntp_event_e event, void *event_param, void *cb_param)
{
    (void)cb_param;

    if (event == CM_NTP_EVENT_SYNC_OK) {
        cm_printf("[NTP] NTP sync success: %s\n", (char *)event_param);
    } else if (event == CM_NTP_EVENT_SETTIME_FAIL) {
        cm_printf("[NTP] NTP get time OK but set time fail: %s\n", (char *)event_param);
    } else {
        cm_printf("[NTP] NTP error\n");
    }
}

/* 等待联网成功 */
static void wait_for_network(void)
{
    cm_cereg_state_t cereg = {0};
    int retry_count = 0;
    const int max_retries = 120; // 最多等待2分钟

    cm_printf("[NTP] Waiting for network to be ready...\n");

    while (retry_count < max_retries) {
        cm_modem_get_cereg_state(&cereg);
        if (cereg.state == 1) {
            cm_printf("[NTP] Network is ready!\n");
            return;
        }
        cm_printf("[NTP] Waiting for network... (state=%d, retry=%d/%d)\n", cereg.state, retry_count + 1, max_retries);
        osDelay(1000); // 等待1秒
        retry_count++;
    }

    cm_printf("[NTP] Warning: Network timeout, proceeding anyway...\n");
}

/* 自动运行NTP测试 */
static void run_ntp_test(void)
{
    cm_printf("[NTP] Starting NTP time synchronization test...\n");

    /* 配置NTP参数 */
    uint16_t port = 123;
    uint32_t timeout = 6000;
    uint32_t dns_priority = 1;
    bool set_rtc = true;

    cm_ntp_set_cfg(CM_NTP_CFG_SERVER, (void *)"ntp1.aliyun.com");
    cm_ntp_set_cfg(CM_NTP_CFG_PORT, &port);
    cm_ntp_set_cfg(CM_NTP_CFG_TIMEOUT, &timeout);
    cm_ntp_set_cfg(CM_NTP_CFG_DNS, &dns_priority);
    cm_ntp_set_cfg(CM_NTP_CFG_SET_RTC, &set_rtc);
    cm_ntp_set_cfg(CM_NTP_CFG_CB, ntp_callback);
    cm_ntp_set_cfg(CM_NTP_CFG_CB_PARAM, (void *)"NTP_SYNC");

    /* 执行NTP同步 */
    int ret = cm_ntp_sync();
    if (ret != 0) {
        cm_printf("[NTP] cm_ntp_sync() failed, ret=%d\n", ret);
    } else {
        cm_printf("[NTP] NTP sync request sent successfully\n");
    }
}

int main(void)
{
    cm_eloop_init_default();

    /* 等待联网成功 */
    wait_for_network();

    /* 运行NTP测试 */
    run_ntp_test();

    return 0;
}