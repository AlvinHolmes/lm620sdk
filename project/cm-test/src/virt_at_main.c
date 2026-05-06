/**
 * @file example_main.c
 * @brief Virtual AT 测试用例
 * @date 2025-02-09
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
#include "cm_demo_common.h"
#include "cm_sys.h"
#include "cm_virt_at.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TEST_VIRT_AT_RSP_BUF_SIZE    512

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void test_async_response_cb(cm_virt_at_param_t *param);
static int32_t test_case_send_sync(void);
static int32_t test_case_send_async(void);
static void test_urc_cb(uint8_t *urc, int32_t urc_len);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile bool g_async_received = false;
static volatile bool g_urc_received = false;
static volatile bool g_cereg_rsp_received = false;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief 异步响应回调函数
 */
static void test_async_response_cb(cm_virt_at_param_t *param)
{
    if (param == NULL) {
        cm_printf("async callback: param is NULL\r\n");
        return;
    }
    cm_printf("async callback: event=%d, rsp_len=%d\r\n", param->event, param->rsp_len);
    if ((param->rsp != NULL) && (param->rsp_len > 0)) {
        cm_printf("async callback: rsp=%s\r\n", param->rsp);
    }
    if (param->user_param != NULL) {
        cm_printf("async callback: user_param=%s\r\n", (char *)param->user_param);
    }
    g_async_received = true;
}

/**
 * @brief URC 回调函数
 */
static void test_urc_cb(uint8_t *urc, int32_t urc_len)
{
    (void)urc_len;
    cm_printf("URC received: %s\r\n", urc);
    g_urc_received = true;
}

/**
 * @brief CEREG 查询回调函数
 */
static void test_cereg_query_cb(cm_virt_at_param_t *param)
{
    if (param == NULL) {
        cm_printf("cereg query callback: param is NULL\r\n");
        return;
    }
    cm_printf("CEREG query callback: event=%d, rsp_len=%d\r\n", param->event, param->rsp_len);
    if ((param->rsp != NULL) && (param->rsp_len > 0)) {
        cm_printf("CEREG query response: %s\r\n", param->rsp);
        if ((strstr((const char *)param->rsp, "+CEREG:") != NULL) &&
            (strstr((const char *)param->rsp, "OK") != NULL)) {
            g_cereg_rsp_received = true;
        }
    }
}

/**
 * @brief 测试用例: 同步发送 AT 命令
 */
static int32_t test_case_send_sync(void)
{
    int32_t ret;
    uint8_t rsp[TEST_VIRT_AT_RSP_BUF_SIZE];
    int32_t rsp_len = 0;

    cm_printf("\r\n========== Test Case: Send Sync ==========\r\n");

    const uint8_t *at_cmd = (const uint8_t *)"AT\r\n";
    cm_printf("Sending AT command sync: %s", at_cmd);

    ret = cm_virt_at_send_sync(at_cmd, rsp, &rsp_len, 5000);
    if (ret != 0) {
        cm_printf("cm_virt_at_send_sync failed, ret=%d\r\n", ret);
        return -1;
    }

    cm_printf("cm_virt_at_send_sync success\r\n");
    cm_printf("Response length: %d\r\n", rsp_len);
    if (rsp_len > 0) {
        cm_printf("Response: %s\r\n", rsp);
    }

    cm_printf("========== Test Case: Send Sync End ==========\r\n\r\n");
    return 0;
}

/**
 * @brief 测试用例: 异步发送 AT 命令
 */
static int32_t test_case_send_async(void)
{
    int32_t ret;
    const char *user_data = "test_user_param";

    cm_printf("\r\n========== Test Case: Send Async ==========\r\n");

    g_async_received = false;

    const uint8_t *at_cmd = (const uint8_t *)"AT+CGSN\r\n";
    cm_printf("Sending AT command async: %s", at_cmd);

    ret = cm_virt_at_send_async(at_cmd, test_async_response_cb, (void *)user_data);
    if (ret != 0) {
        cm_printf("cm_virt_at_send_async failed, ret=%d\r\n", ret);
        return -1;
    }

    cm_printf("cm_virt_at_send_async success, waiting for callback...\r\n");

    uint32_t wait_count = 0;
    while ((!g_async_received) && (wait_count < 100)) {
        osDelay(50);
        wait_count++;
    }

    if (g_async_received) {
        cm_printf("Async callback received\r\n");
    } else {
        cm_printf("Async callback timeout\r\n");
    }

    cm_printf("========== Test Case: Send Async End ==========\r\n\r\n");
    return g_async_received ? 0 : -1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * @brief 主函数
 */
int main(void)
{
    int32_t result;
    int32_t pass_count = 0;
    int32_t fail_count = 0;

    cm_printf("========================================\r\n");
    cm_printf("   CM Virtual AT Test\r\n");
    cm_printf("========================================\r\n");

    /* 等待 AT 模块完全初始化 */
    osDelay(2000);

#if 1
    result = test_case_send_sync();
    if (result == 0) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if 1
    result = test_case_send_async();
    if (result == 0) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if 1
    cm_printf("\r\n========== Test Case: URC Callback ==========\r\n");
    g_urc_received = false;
    g_cereg_rsp_received = false;
    result = cm_virt_at_urc_cb_reg((const uint8_t *)"+CEREG:", test_urc_cb);
    if (result == 0) {
        cm_printf("URC callback registered, sending AT+CEREG?...\r\n");
        /* 发送 AT+CEREG? 查询命令，通过异步回调接收响应 */
        result = cm_virt_at_send_async((const uint8_t *)"AT+CEREG?\r\n",
                                        test_cereg_query_cb, NULL);
        if (result != 0) {
            cm_printf("AT+CEREG? send failed, ret=%d\r\n", result);
        }
        /* 等待一段时间，看是否有主动上报的 +CEREG: URC */
        osDelay(3 * 1000);
        if (g_cereg_rsp_received) {
            cm_printf("CEREG query callback received expected response\r\n");
            pass_count++;
        } else {
            cm_printf("CEREG query callback did not receive expected response\r\n");
            fail_count++;
        }
        if (g_urc_received) {
            cm_printf("URC received test PASSED\r\n");
            pass_count++;
        } else {
            cm_printf("URC not received in timeout\r\n");
            fail_count++;
        }
        cm_virt_at_urc_cb_dereg((const uint8_t *)"+CEREG:", test_urc_cb);
    } else {
        cm_printf("Failed to register URC callback\r\n");
        fail_count++;
    }
    cm_printf("========== Test Case: URC Callback End ==========\r\n\r\n");
#endif

    cm_printf("\r\n");
    cm_printf("========================================\r\n");
    cm_printf("   All Tests Completed\r\n");
    cm_printf("   Pass: %d, Fail: %d\r\n", pass_count, fail_count);
    cm_printf("========================================\r\n");

    return 0;
}
