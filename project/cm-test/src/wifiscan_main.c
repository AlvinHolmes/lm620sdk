/**
 * @file example_main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-12-25
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cm_os.h"
#include "cm_demo_common.h"
#include "cm_wifiscan.h"
#include "cm_mem.h"
#include "cm_pm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile uint8_t s_scan_done = 0;
static volatile uint32_t s_scan_count = 0;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void _num_to_hex(uint8_t *in_data, uint32_t in_len, char *out_data)
{
    char tmp_buf[10];
    if (in_len > 0) {
        for (uint32_t i = 0; i < in_len; i++) {
            memset(tmp_buf, 0, sizeof(tmp_buf));
            sprintf(tmp_buf, "%02X", in_data[i]);
            strcat(out_data, tmp_buf);
        }
    }
}

static void _wifiscan_callback(cm_wifi_scan_info_t *param, void *user_param)
{
    char mac_buf[20];

    s_scan_done = 1;
    if (param == NULL) {
        cm_demo_printf("wifiscan callback: param is NULL\r\n");
        return;
    }

    s_scan_count++;
    cm_demo_printf("wifiscan callback (round %lu): found %d AP(s)\r\n", s_scan_count, param->bssid_number);

    for (uint8_t i = 0; i < param->bssid_number; i++) {
        memset(mac_buf, 0, sizeof(mac_buf));
        _num_to_hex(param->channel_cell_list[i].bssid, SIZE_MAC_ADDRESS, mac_buf);
        cm_demo_printf("AP[%d]: MAC=%s, RSSI=%d, CH=%lu\r\n",
                       i + 1,
                       mac_buf,
                       param->channel_cell_list[i].rssi,
                       param->channel_cell_list[i].channel_number);
    }

    if (user_param != NULL) {
        cm_demo_printf("user_param: %s\r\n", (char *)user_param);
    }
}

static void _simple_wifiscan_callback(cm_wifi_scan_info_t *param, void *user_param)
{
    (void)(param);
    (void)(user_param);
    s_scan_done = 1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(void)
{
    /* 避免睡眠模式导致输出异常 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_work_lock();

    /* 延迟一段时间等待 cp 核正常 */
    osDelay(5 * 1000);

    int32_t ret;
    uint8_t max_count = 20;
    uint8_t timeout = 10;
    cm_wifi_scan_info_t *scan_result;

    cm_demo_printf("CM wifiscan test starts\r\n");

    cm_demo_printf("\r\n=== Test cm_wifiscan_cfg ===\r\n");
    ret = cm_wifiscan_cfg(CM_WIFI_SCAN_CFG_MAX_COUNT, &max_count);
    if (ret != 0) {
        cm_demo_printf("cm_wifiscan_cfg MAX_COUNT failed, ret=%d\r\n", ret);
        return 0;
    }
    cm_demo_printf("cm_wifiscan_cfg MAX_COUNT=%d success\r\n", max_count);

    ret = cm_wifiscan_cfg(CM_WIFI_SCAN_CFG_TIMEOUT, &timeout);
    if (ret != 0) {
        cm_demo_printf("cm_wifiscan_cfg TIMEOUT failed, ret=%d\r\n", ret);
        return 0;
    }
    cm_demo_printf("cm_wifiscan_cfg TIMEOUT=%d success\r\n", timeout);

    cm_demo_printf("\r\n=== Test cm_wifiscan_start ===\r\n");
    ret = cm_wifiscan_start(_wifiscan_callback, "test_param");
    if (ret != 0) {
        cm_demo_printf("cm_wifiscan_start failed, ret=%d\r\n", ret);
        return 0;
    }
    cm_demo_printf("cm_wifiscan_start success\r\n");

    cm_demo_printf("\r\n=== Wait for scan result ===\r\n");
    while (s_scan_done == 0) {
        osDelay(1000);
    }

    cm_demo_printf("\r\n=== Test cm_wifiscan_query ===\r\n");
    s_scan_done = 0;
    ret = cm_wifiscan_start(_simple_wifiscan_callback, NULL);
    if (ret != 0) {
        cm_demo_printf("cm_wifiscan_start for query failed, ret=%d\r\n", ret);
    } else {
        while (s_scan_done == 0) {
            osDelay(1000);
        }
        scan_result = (cm_wifi_scan_info_t *)cm_malloc(sizeof(cm_wifi_scan_info_t));
        if (scan_result == NULL) {
            cm_demo_printf("malloc failed\r\n");
        } else {
            ret = cm_wifiscan_query(&scan_result);
            if (ret != 0) {
                cm_demo_printf("cm_wifiscan_query failed, ret=%d\r\n", ret);
            } else {
                cm_demo_printf("cm_wifiscan_query: bssid_number=%d\r\n", scan_result->bssid_number);
                for (uint8_t i = 0; i < scan_result->bssid_number; i++) {
                    char mac_buf[20];
                    memset(mac_buf, 0, sizeof(mac_buf));
                    _num_to_hex(scan_result->channel_cell_list[i].bssid, SIZE_MAC_ADDRESS, mac_buf);
                    cm_demo_printf("AP[%d]: MAC=%s, RSSI=%d, CH=%lu\r\n",
                                   i + 1,
                                   mac_buf,
                                   scan_result->channel_cell_list[i].rssi,
                                   scan_result->channel_cell_list[i].channel_number);
                }
            }
            cm_free(scan_result);
        }
    }

    cm_demo_printf("\r\n=== Start loop scan ===\r\n");
    /* 手动开启一次 */
    s_scan_done = 1;
    while (1) {
        if (s_scan_done != 0) {
            s_scan_done = 0;
            cm_demo_printf("start new scan (round %lu)\r\n", s_scan_count + 1);
            ret = cm_wifiscan_start(_wifiscan_callback, "test_param");
            if (ret != 0) {
                cm_demo_printf("cm_wifiscan_start failed, ret=%d\r\n", ret);
            }
        }
        osDelay(1000);
    }

    return 0;
}
