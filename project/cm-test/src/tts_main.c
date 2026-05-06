/**
 * @file example_main.c
 * @brief TTS 测试用例
 * @date 2026-02-10
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/*
    FIXME: 已知问题：
    目前发现`test_case_tts_play_stop`设置`cfg.speed = 7;`后，在`test_case_tts_multi_play`设置`cfg.speed = 5;`，
    播放"第一次播放"时声音异常，后面两次正常
 */

/****************************************************************************
* Included Files
****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "cm_sys.h"
#include "cm_gpio.h"
#include "cm_mem.h"
#include "cm_iomux.h"
#include "cm_pm.h"
#include "cm_local_tts.h"
#include "cm_os.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* 测试结果定义 */
#define TEST_RESULT_PASS        0
#define TEST_RESULT_FAIL        -1

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void tts_event_callback(cm_local_tts_event_e event, void *param);
static int32_t test_case_tts_play(void);
static int32_t test_case_tts_play_stop(void);
static int32_t test_case_tts_multi_play(void);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile bool g_tts_finished = false;
static volatile bool g_tts_failed = false;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief TTS 回调函数
 */
static void tts_event_callback(cm_local_tts_event_e event, void *param)
{
    (void)param;

    switch (event) {
    case CM_LOCAL_TTS_EVENT_PLAY_FINISH:
        cm_printf("TTS event: PLAY_FINISH\r\n");
        g_tts_finished = true;
        break;

    case CM_LOCAL_TTS_EVENT_PLAY_FAIL:
        cm_printf("TTS event: PLAY_FAIL\r\n");
        g_tts_failed = true;
        g_tts_finished = true;
        break;

    case CM_LOCAL_TTS_EVENT_PLAY_INTERRUPT:
        cm_printf("TTS event: PLAY_INTERRUPT\r\n");
        g_tts_finished = true;
        break;

    default:
        cm_printf("TTS event: unknown %d\r\n", event);
        break;
    }
}

/**
 * @brief 测试用例 1: 基本播放
 */
static int32_t test_case_tts_play(void)
{
    int32_t ret;
    cm_local_tts_cfg_t cfg;
    // const char *text = "123456789, 分段3，已标记，59分1秒，1小时1分9秒。你已经跑步5公里，最近1公里耗时6分40秒，平均心率145，平均配速5分30秒。很棒，加油 加油 加油！下一段，放松。下一个动作，原地高抬腿。前方左转。前方到达CP1点。请加速。减脂中，继续保持。你已完成训练，请放松一下吧。准备开始，3 2 1，go!";
    const char *text = "你好，这是语音合成测试";

    cm_printf("\r\n========== Test Case: TTS Play ==========\r\n");

    g_tts_finished = false;
    g_tts_failed = false;

    /* 初始化 TTS */
    cfg.speed = 9;
    cfg.volume = 9;
    cfg.encode = CM_LOCAL_TTS_ENCODE_TYPE_UTF8;
    cfg.digit = CM_LOCAL_TTS_DIGIT_AUTO;
    cfg.tone = CM_LOCAL_TTS_TONE_NORMAL;
    cfg.effect = CM_LOCAL_TTS_EFFECT_NORMAL;

    ret = cm_local_tts_init(&cfg);
    cm_printf("cm_local_tts_init returned: %d\r\n", ret);

    if (ret != 0) {
        cm_printf("Failed to init TTS, ret=%d\r\n", ret);
        return TEST_RESULT_FAIL;
    }

    /* 开始播放 */
    ret = cm_local_tts_play(text, -1, tts_event_callback, NULL);
    cm_printf("cm_local_tts_play returned: %d\r\n", ret);

    if (ret != 0) {
        cm_printf("Failed to start playback, ret=%d\r\n", ret);
        cm_local_tts_deinit();
        return TEST_RESULT_FAIL;
    }

    cm_printf("Playing TTS...\r\n");

    /* 等待播放结束 */
    while (!g_tts_finished) {
        cm_local_tts_state_e state = cm_local_tts_get_state();
        if (state == CM_LOCAL_TTS_STATE_IDLE) {
            cm_printf("TTS state changed to IDLE\r\n");
            break;
        }
        osDelay(100);
    }

    cm_printf("TTS finished: %s\r\n", g_tts_failed ? "FAILED" : "SUCCESS");

    /* 去初始化 */
    cm_local_tts_deinit();

    cm_printf("========== Test Case: TTS Play End ==========\r\n\r\n");
    return (g_tts_failed) ? TEST_RESULT_FAIL : TEST_RESULT_PASS;
}

/**
 * @brief 测试用例 2: 停止播放
 */
static int32_t test_case_tts_play_stop(void)
{
    int32_t ret;
    cm_local_tts_cfg_t cfg;
    const char *text =
        "这是一段较长的语音合成测试文本，用于测试停止功能，这是一段较长的语音合成测试文本，用于测试停止功能，这是一段较长的语音合成测试文本，用于测试停止功能";

    cm_printf("\r\n========== Test Case: TTS Play Stop ==========\r\n");

    g_tts_finished = false;
    g_tts_failed = false;

    /* 初始化 TTS */
    cfg.speed = 5;
    cfg.volume = 9;
    cfg.encode = CM_LOCAL_TTS_ENCODE_TYPE_UTF8;
    cfg.digit = CM_LOCAL_TTS_DIGIT_AUTO;
    cfg.tone = CM_LOCAL_TTS_TONE_NORMAL;
    cfg.effect = CM_LOCAL_TTS_EFFECT_NORMAL;

    ret = cm_local_tts_init(&cfg);
    if (ret != 0) {
        cm_printf("Failed to init TTS, ret=%d\r\n", ret);
        return TEST_RESULT_FAIL;
    }

    /* 开始播放 */
    ret = cm_local_tts_play(text, -1, tts_event_callback, NULL);
    if (ret != 0) {
        cm_printf("Failed to start playback, ret=%d\r\n", ret);
        cm_local_tts_deinit();
        return TEST_RESULT_FAIL;
    }

    cm_printf("Playing TTS...\r\n");

    osDelay(5000);

    ret = cm_local_tts_play_stop();
    cm_printf("cm_local_tts_play_stop returned: %d\r\n", ret);

    /* 等待停止完成 */
    osDelay(500);

    cm_local_tts_deinit();

    cm_printf("TTS stop test result: %s\r\n", (ret == 0) ? "PASS" : "FAIL");
    cm_printf("========== Test Case: TTS Play Stop End ==========\r\n\r\n");
    return (ret == 0) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
}

/**
 * @brief 测试用例 3: 多次播放
 */
static int32_t test_case_tts_multi_play(void)
{
    int32_t ret;
    cm_local_tts_cfg_t cfg;
    int32_t i;

    cm_printf("\r\n========== Test Case: TTS Multi Play ==========\r\n");

    /* 初始化 TTS */
    cfg.speed = 5;
    cfg.volume = 9;
    cfg.encode = CM_LOCAL_TTS_ENCODE_TYPE_UTF8;
    cfg.digit = CM_LOCAL_TTS_DIGIT_AUTO;
    cfg.tone = CM_LOCAL_TTS_TONE_NORMAL;
    cfg.effect = CM_LOCAL_TTS_EFFECT_NORMAL;

    ret = cm_local_tts_init(&cfg);
    if (ret != 0) {
        cm_printf("Failed to init TTS, ret=%d\r\n", ret);
        return TEST_RESULT_FAIL;
    }

    /* 连续播放三次 */
    for (i = 0; i < 3; i++) {
        const char *text[] = {
            "第一次播放",
            "第二次播放",
            "第三次播放"
        };

        g_tts_finished = false;
        g_tts_failed = false;

        ret = cm_local_tts_play(text[i], -1, tts_event_callback, NULL);
        if (ret != 0) {
            cm_printf("Failed to start playback %d, ret=%d\r\n", i + 1, ret);
            cm_local_tts_deinit();
            return TEST_RESULT_FAIL;
        }

        cm_printf("Playing TTS %d...\r\n", i + 1);

        /* 等待播放结束 */
        while (!g_tts_finished) {
            osDelay(100);
        }

        cm_printf("TTS %d finished\r\n", i + 1);

        /* 等待一下再播放下一次 */
        osDelay(500);
    }

    cm_local_tts_deinit();

    cm_printf("========== Test Case: TTS Multi Play End ==========\r\n\r\n");
    return TEST_RESULT_PASS;
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
    cm_printf("   CM TTS Test\r\n");
    cm_printf("========================================\r\n");

    /* 避免睡眠模式导致输出异常 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_work_lock();

    /* 打开 PA 和 Codec 的电源 */
    cm_iomux_set_pin_func(CM_GPIO_AON_4, CM_IOMUX_AON_4_MUX_GPIO);
    cm_iomux_set_pin_func(CM_GPIO_AON_8, CM_IOMUX_AON_8_MUX_GPIO);

    cm_gpio_cfg_t cfg = {0};
    cfg.direction = CM_GPIO_DIRECTION_OUTPUT;
    cfg.pull = CM_GPIO_PULL_NONE;
    cm_gpio_init(CM_GPIO_AON_4, &cfg);
    cm_gpio_set_level(CM_GPIO_AON_4, 1);

    cm_gpio_init(CM_GPIO_AON_8, &cfg);
    cm_gpio_set_level(CM_GPIO_AON_8, 1);

    osDelay(100);

    /* 运行测试用例 */
    result = test_case_tts_play();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }

    result = test_case_tts_play_stop();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }

    result = test_case_tts_multi_play();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }

    cm_printf("\r\n");
    cm_printf("========================================\r\n");
    cm_printf("   All Tests Completed\r\n");
    cm_printf("   Pass: %d, Fail: %d\r\n", pass_count, fail_count);
    cm_printf("========================================\r\n");

    return 0;
}
