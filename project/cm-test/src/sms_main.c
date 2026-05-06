/**
 * @file example_main.c
 * @brief SMS 测试用例
 * @date 2025-02-28
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
#include "cm_sys.h"
#include "cm_sms.h"
#include "cm_pm.h"
#include "cm_virt_at.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* 测试电话号码 */
#define TEST_SMS_PHONE_NUM          "137XXXXXXXX"

/* 测试间隔时间 (ms) */
#define TEST_SMS_INTERVAL_MS        3000

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int32_t test_sms_send_txt_gsm7(void);
static int32_t test_sms_send_txt_ucs2(void);
static int32_t test_sms_send_pdu(void);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief 测试用例: 发送 GSM 7bit 编码的英文短信
 */
static int32_t test_sms_send_txt_gsm7(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: Send TXT GSM7 ==========\r\n");

    cm_printf("Sending SMS to %s with GSM7 encoding...\r\n", TEST_SMS_PHONE_NUM);
    cm_printf("Message: Hello LM620!\r\n");

    ret = cm_sms_send_txt("Hello LM620!", TEST_SMS_PHONE_NUM, CM_MSG_MODE_GSM_7, CM_SIM_ID_0);
    if (ret != 0) {
        cm_printf("cm_sms_send_txt GSM7 failed, ret=%d\r\n", ret);
        cm_printf("========== Test Case: Send TXT GSM7 FAILED ==========\r\n\r\n");
        return -1;
    }

    cm_printf("cm_sms_send_txt GSM7 success\r\n");
    cm_printf("========== Test Case: Send TXT GSM7 PASSED ==========\r\n\r\n");
    return 0;
}

/**
 * @brief 测试用例: 发送 UCS2 编码的中文短信
 */
static int32_t test_sms_send_txt_ucs2(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: Send TXT UCS2 ==========\r\n");

    cm_printf("Sending SMS to %s with UCS2 encoding...\r\n", TEST_SMS_PHONE_NUM);
    /* UCS2 编码的 "测试短信" - 每个 Unicode 字符用 4 位十六进制表示 */
    cm_printf("Message: 6D4B8BD577ED4FE1 (测试短信)\r\n");

    ret = cm_sms_send_txt("6D4B8BD577ED4FE1", TEST_SMS_PHONE_NUM, CM_MSG_MODE_GSM_UCS2, CM_SIM_ID_0);
    if (ret != 0) {
        cm_printf("cm_sms_send_txt UCS2 failed, ret=%d\r\n", ret);
        cm_printf("========== Test Case: Send TXT UCS2 FAILED ==========\r\n\r\n");
        return -1;
    }

    cm_printf("cm_sms_send_txt UCS2 success\r\n");
    cm_printf("========== Test Case: Send TXT UCS2 PASSED ==========\r\n\r\n");
    return 0;
}

/**
 * @brief 测试用例: 发送 PDU 模式短信
 */
static int32_t test_sms_send_pdu(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: Send PDU ==========\r\n");

    cm_printf("Sending SMS to %s in PDU mode...\r\n", TEST_SMS_PHONE_NUM);
    //PDU格式数据可采用网上的pdu工具生成，由于SIM卡不同，短信中心号码可能不同，建议使用AT+CSCA?指令查询短信的中心号码，本示例中心号码：+8613800230500
    //PDU模式向18883993627发送“欢迎使用中移物联网Cat.1模组”
    ret = cm_sms_send_pdu("0891683108200305F011000D916831XXXXXXXX000800064F60597D", "18", CM_MSG_MODE_GSM_7, CM_SIM_ID_0);
    if (ret != 0) {
        cm_printf("cm_sms_send_pdu failed, ret=%d\r\n", ret);
        cm_printf("========== Test Case: Send PDU FAILED ==========\r\n\r\n");
        return -1;
    }

    cm_printf("cm_sms_send_pdu success\r\n");
    cm_printf("========== Test Case: Send PDU PASSED ==========\r\n\r\n");
    return 0;
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

    /* 避免睡眠模式导致输出异常 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_work_lock();

    cm_printf("========================================\r\n");
    cm_printf("   CM SMS Test\r\n");
    cm_printf("   Target: %s\r\n", TEST_SMS_PHONE_NUM);
    cm_printf("========================================\r\n");

    /* 等待模组初始化完成 */
    cm_printf("Waiting for module initialization...\r\n");
    osDelay(3000);

#if 1
    /* 测试 GSM 7bit 编码短信 */
    result = test_sms_send_txt_gsm7();
    if (result == 0) {
        pass_count++;
    } else {
        fail_count++;
    }

    /* 等待短信发送完成 */
    osDelay(TEST_SMS_INTERVAL_MS);
#endif

#if 1
    /* 测试 UCS2 编码短信 */
    result = test_sms_send_txt_ucs2();
    if (result == 0) {
        pass_count++;
    } else {
        fail_count++;
    }

    /* 等待短信发送完成 */
    osDelay(TEST_SMS_INTERVAL_MS);
#endif

#if 1
    /* 测试 PDU 模式短信 */
    result = test_sms_send_pdu();
    if (result == 0) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

    cm_printf("\r\n");
    cm_printf("========================================\r\n");
    cm_printf("   All Tests Completed\r\n");
    cm_printf("   Pass: %d, Fail: %d\r\n", pass_count, fail_count);
    cm_printf("========================================\r\n");

    return 0;
}