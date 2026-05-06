/**
 * @file example_main.c
 * @brief Audio 测试用例
 * @date 2026-02-05
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
#include <math.h>
#include <stdio.h>
#include "cm_os.h"
#include "cm_sys.h"
#include "cm_gpio.h"
#include "cm_mem.h"
#include "cm_iomux.h"
#include "cm_pm.h"
#include "cm_audio_player.h"
#include "cm_audio_recorder.h"
#include "cm_fs.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* 测试用例宏定义 */
#define TEST_CASE_FILE_PLAY       (1 << 0)
#define TEST_CASE_PAUSE_RESUME    (1 << 1)
#define TEST_CASE_STOP            (1 << 2)
#define TEST_CASE_STREAM_PLAY     (1 << 3)
#define TEST_CASE_STREAM_CB       (1 << 4)
// #define TEST_CASE_WAV_FILE        (1 << 5) /* 暂不支持 */
#define TEST_CASE_AMR_FILE        (1 << 6)
#define TEST_CASE_RECORD_AMR      (1 << 7)
#define TEST_CASE_RECORD_PCM      (1 << 8)
#define TEST_CASE_SET_VOLUME      (1 << 9)
#define TEST_CASE_SET_GAIN        (1 << 10)
#define TEST_CASE_DAC_FILE_PLAY   (1 << 11)

/* 启用的测试用例 - 打开全部示例 */
/* 启用的测试用例 - 打开全部示例 */
#define ENABLED_TEST_CASES  (TEST_CASE_SET_VOLUME \
                                | TEST_CASE_SET_GAIN \
                                | TEST_CASE_FILE_PLAY \
                                | TEST_CASE_PAUSE_RESUME \
                                | TEST_CASE_STOP \
                                | TEST_CASE_STREAM_PLAY \
                                | TEST_CASE_STREAM_CB \
                                | TEST_CASE_AMR_FILE \
                                | TEST_CASE_RECORD_AMR \
                                | TEST_CASE_RECORD_PCM \
                                | TEST_CASE_DAC_FILE_PLAY)

/* PCM 测试音频参数 */
#define TEST_SAMPLE_RATE        CM_AUDIO_SAMPLE_RATE_16000HZ
#define TEST_SAMPLE_FORMAT      CM_AUDIO_SAMPLE_FORMAT_16BIT
#define TEST_SAMPLE_CHANNELS    CM_AUDIO_SOUND_MONO
#define TEST_FRAME_SIZE         320

/* 测试结果定义 */
#define TEST_RESULT_PASS        0
#define TEST_RESULT_FAIL        -1

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void test_play_callback(cm_audio_play_event_e event, void *param);
static void test_stream_callback(cm_audio_player_stream_event_e event);
static int32_t test_case_file_play(void);
static int32_t test_case_pause_resume(void);
static int32_t test_case_stop(void);
static int32_t test_case_stream_play(void);
static int32_t test_case_stream_cb(void);
static void test_case_wav_file(void);
static int32_t test_case_amr_file(void);
static int32_t test_case_record_amr(void);
static int32_t test_case_record_pcm(void);
static int32_t test_case_set_volume(void);
static int32_t test_case_set_gain(void);
static int32_t play_amr_and_wait(const char *file_path);
static uint8_t *generate_pcm_sine_wave(uint32_t *size);

/* 录音回调函数 */
static void test_record_callback(cm_audio_record_event_e event, void *param);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile bool g_play_finished = false;
static volatile bool g_stream_started = false;
static volatile bool g_stream_closed = false;
static volatile bool g_stream_overrun = false;

/* 录音测试相关 */
static FILE *g_record_fd = NULL;
static volatile bool g_recording_finished = false;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief 播放回调函数
 */
static void test_play_callback(cm_audio_play_event_e event, void *param)
{
    (void)param;
    cm_printf("Play callback: event=%d\r\n", event);
    if (event == CM_AUDIO_PLAY_EVENT_FINISHED) {
        g_play_finished = true;
    }
}

/**
 * @brief 流播放回调函数
 */
static void test_stream_callback(cm_audio_player_stream_event_e event)
{
    cm_printf("Stream callback: event=%d\r\n", event);
    switch (event) {
    case CM_AUDIO_TRACK_EVENT_STARTED:
        g_stream_started = true;
        break;
    case CM_AUDIO_TRACK_EVENT_CLOSED:
        g_stream_closed = true;
        break;
    case CM_AUDIO_TRACK_EVENT_OVERRUN:
        g_stream_overrun = true;
        break;
    default:
        break;
    }
}

/**
 * @brief 生成PCM正弦波测试数据
 * @note 采样数取400的倍数，确保循环播放时首尾相位一致
 *       16kHz/440Hz = 400/11，所以400的倍数能保证整数个周期
 */
static uint8_t *generate_pcm_sine_wave(uint32_t *size)
{
    uint32_t i;
    const uint32_t samples_per_cycle_base = 400;  /* 16kHz/440Hz = 400/11 的分母 */
    const uint32_t num_cycles_base = 11;  /* 400个采样点对应11个周期 */
    const uint32_t repeat = 40;          /* 重复40次 */
    const uint32_t sample_count = samples_per_cycle_base * repeat;  /* 16000个采样 = 440个周期 */
    /* 每个采样点的角度增量 */
    const float angle_step = (2.0f * 3.14159f * num_cycles_base) / samples_per_cycle_base;
    int16_t *pcm_data;

    pcm_data = cm_malloc(sample_count * sizeof(int16_t));
    if (pcm_data == NULL) {
        return NULL;
    }

    /* 生成正弦波 */
    for (i = 0; i < sample_count; i++) {
        pcm_data[i] = (int16_t)(16000.0f * sinf(angle_step * i));
    }

    *size = sample_count * sizeof(int16_t);
    return (uint8_t *)pcm_data;
}

/**
 * @brief 播放 AMR 文件并等待播放结束
 */
static int32_t play_amr_and_wait(const char *file_path)
{
    int32_t ret;

    g_play_finished = false;
    ret = cm_audio_play_file(file_path, CM_AUDIO_PLAY_FORMAT_AMRNB, NULL,
                             test_play_callback, NULL);
    if (ret != 0) {
        cm_printf("Failed to start AMR playback, ret=%d\r\n", ret);
        return ret;
    }

    while (!g_play_finished) {
        osDelay(10);
    }

    return 0;
}

/**
 * @brief 测试用例1: 文件播放 (使用 AMR 文件)
 */
static int32_t test_case_file_play(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: AMR File Play ==========\r\n");

    g_play_finished = false;

    /* 播放 AMR 文件 */
    ret = cm_audio_play_file("/test.amr", CM_AUDIO_PLAY_FORMAT_AMRNB, NULL,
                             test_play_callback, NULL);

    cm_printf("cm_audio_play_file(AMR) returned: %d\r\n", ret);

    if (ret == 0) {
        cm_printf("Playing AMR file...\r\n");

        /* 等待播放结束 */
        while (!g_play_finished) {
            osDelay(10);
        }
        cm_printf("Play finished: %s\r\n", g_play_finished ? "yes" : "no");
    } else {
        cm_printf("Failed to start AMR playback, ret=%d\r\n", ret);
    }

    cm_printf("========== Test Case: AMR File Play End ==========\r\n\r\n");
    return (ret == 0) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
}

/**
 * @brief 测试用例2: 暂停/继续
 * @note 播放 2 秒后暂停，间隔 1 秒后恢复
 */
static int32_t test_case_pause_resume(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: Pause/Resume ==========\r\n");

    g_play_finished = false;

    /* 启动播放 */
    ret = cm_audio_play_file("/test.amr", CM_AUDIO_PLAY_FORMAT_AMRNB, NULL,
                             test_play_callback, NULL);
    if (ret != 0) {
        cm_printf("Failed to start playback, ret=%d\r\n", ret);
        return TEST_RESULT_FAIL;
    }
    cm_printf("Playback started, ret=%d\r\n", ret);

    /* 播放 2 秒 */
    cm_printf("Playing for 2 seconds...\r\n");
    osDelay(2000);

    /* 暂停 */
    ret = cm_audio_player_pause();
    cm_printf("cm_audio_player_pause returned: %d (expected 0)\r\n", ret);
    if (ret != 0) {
        cm_printf("Pause failed!\r\n");
        cm_audio_player_stop();
        return TEST_RESULT_FAIL;
    }

    /* 等待 1 秒 */
    cm_printf("Paused for 1 second...\r\n");
    osDelay(1000);

    /* 恢复 */
    ret = cm_audio_player_resume();
    cm_printf("cm_audio_player_resume returned: %d (expected 0)\r\n", ret);
    if (ret != 0) {
        cm_printf("Resume failed!\r\n");
        cm_audio_player_stop();
        return TEST_RESULT_FAIL;
    }

    /* 等待播放结束 */
    cm_printf("Resumed, waiting for playback to finish...\r\n");
    while (!g_play_finished) {
        osDelay(10);
    }

    cm_printf("Pause/Resume test passed\r\n");
    cm_printf("========== Test Case: Pause/Resume End ==========\r\n\r\n");
    return TEST_RESULT_PASS;
}

/**
 * @brief 测试用例3: 停止播放
 */
static int32_t test_case_stop(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: Stop ==========\r\n");

    g_play_finished = false;

    /* 启动播放 */
    ret = cm_audio_play_file("/test.amr", CM_AUDIO_PLAY_FORMAT_AMRNB, NULL,
                             test_play_callback, NULL);
    if (ret != 0) {
        cm_printf("Failed to start playback, ret=%d\r\n", ret);
        return TEST_RESULT_FAIL;
    }
    cm_printf("Playback started\r\n");

    /* 播放 1 秒后停止 */
    osDelay(1000);

    ret = cm_audio_player_stop();
    cm_printf("cm_audio_player_stop returned: %d (expected 0)\r\n", ret);

    /* 等待停止完成 */
    osDelay(100);

    cm_printf("Stop test result: %s\r\n", (ret == 0) ? "PASS" : "FAIL");
    cm_printf("========== Test Case: Stop End ==========\r\n\r\n");
    return (ret == 0) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
}

/**
 * @brief 测试用例4: 流播放
 */
static int32_t test_case_stream_play(void)
{
    int32_t ret;
    uint8_t *pcm_data = NULL;
    uint32_t pcm_size;
    uint32_t play_time_ms = 10000; /* 播放10秒 */
    uint32_t start_time;
    uint32_t offset = 0;
    const uint32_t chunk_size = 320; /* 每次推送320字节 (20ms @ 16kHz mono 16bit) */
    uint32_t push_size;

    cm_printf("\r\n========== Test Case: Stream Play ==========\r\n");

    g_stream_started = false;
    g_stream_closed = false;

    /* 注册回调 - 必须在 stream_open 之前注册 */
    cm_audio_player_stream_cb_reg(test_stream_callback);

    /* PCM 参数 */
    cm_audio_sample_param_t sample_param = {
        .sample_format = TEST_SAMPLE_FORMAT,
        .rate = TEST_SAMPLE_RATE,
        .num_channels = TEST_SAMPLE_CHANNELS,
    };

    /* 打开流播放 */
    ret = cm_audio_player_stream_open(CM_AUDIO_PLAY_FORMAT_PCM, &sample_param);
    cm_printf("cm_audio_player_stream_open returned: %d\r\n", ret);

    if (ret != 0) {
        cm_printf("Failed to open stream, ret=%d\r\n", ret);
        return TEST_RESULT_FAIL;
    }

    /* 生成测试数据 */
    pcm_data = generate_pcm_sine_wave(&pcm_size);
    cm_printf("Generated PCM data: %d bytes\r\n", pcm_size);

    if (pcm_data == NULL) {
        cm_printf("Failed to generate PCM data\r\n");
        cm_audio_player_stream_clear_close();
        return TEST_RESULT_FAIL;
    }

    /* 启动后等待 */
    osDelay(100);

    start_time = osKernelGetTickCount();

    /* 循环播放 */
    uint32_t play_ticks = (play_time_ms * osKernelGetTickFreq()) / 1000;
    while ((osKernelGetTickCount() - start_time) < play_ticks) {
        /* 计算本次推送大小 */
        if ((offset + chunk_size) > pcm_size) {
            push_size = pcm_size - offset;
        } else {
            push_size = chunk_size;
        }

        if (push_size == 0) {
            /* 重新开始循环 */
            offset = 0;
            push_size = chunk_size;
        }

        ret = cm_audio_player_stream_push(pcm_data + offset, push_size);
        if (ret < 0) {
            cm_printf("Stream push failed: %d\r\n", ret);
        }

        offset += push_size;

        osDelay(10);
    }

    cm_printf("Stream played for %d seconds\r\n", play_time_ms / 1000);

    /* 关闭流播放 */
    cm_audio_player_stream_close();
    cm_printf("Stream closed\r\n");

    cm_free(pcm_data);

    cm_printf("Stream started: %s\r\n", g_stream_started ? "yes" : "no");
    cm_printf("Stream closed: %s\r\n", g_stream_closed ? "yes" : "no");

    cm_printf("Stream play test result: %s\r\n",
              (g_stream_started && g_stream_closed) ? "PASS" : "FAIL");
    cm_printf("========== Test Case: Stream Play End ==========\r\n\r\n");

    return (g_stream_started && g_stream_closed) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
}

/**
 * @brief 测试用例5: 流播放回调测试
 */
static int32_t test_case_stream_cb(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: Stream Callback ==========\r\n");

    g_stream_started = false;
    g_stream_closed = false;
    g_stream_overrun = false;

    /* 注册回调 */
    cm_audio_player_stream_cb_reg(test_stream_callback);

    /* PCM 参数 */
    cm_audio_sample_param_t sample_param = {
        .sample_format = TEST_SAMPLE_FORMAT,
        .rate = TEST_SAMPLE_RATE,
        .num_channels = TEST_SAMPLE_CHANNELS,
    };

    /* 打开流播放 */
    ret = cm_audio_player_stream_open(CM_AUDIO_PLAY_FORMAT_PCM, &sample_param);
    cm_printf("cm_audio_player_stream_open returned: %d\r\n", ret);

    if (ret == 0) {
        /* 等待启动事件 */
        uint32_t wait_count = 0;
        while ((!g_stream_started) && (wait_count < 100)) {
            osDelay(10);
            wait_count++;
        }

        /* 推送一些数据 */
        uint8_t *pcm_data;
        uint32_t pcm_size;
        pcm_data = generate_pcm_sine_wave(&pcm_size);
        if (pcm_data != NULL) {
            cm_audio_player_stream_push(pcm_data, 320);
            osDelay(100);
            cm_free(pcm_data);
        }

        /* 关闭流播放 */
        cm_audio_player_stream_close();

        /* 等待关闭事件 */
        wait_count = 0;
        while ((!g_stream_closed) && (wait_count < 100)) {
            osDelay(10);
            wait_count++;
        }
    }

    cm_printf("Callback test results:\r\n");
    cm_printf("  STARTED event: %s\r\n", g_stream_started ? "yes" : "no");
    cm_printf("  CLOSED event: %s\r\n", g_stream_closed ? "yes" : "no");
    cm_printf("  OVERRUN event: %s\r\n", g_stream_overrun ? "yes" : "no");

    cm_printf("Callback test result: %s\r\n",
              (g_stream_started && g_stream_closed) ? "PASS" : "FAIL");
    cm_printf("========== Test Case: Stream Callback End ==========\r\n\r\n");

    return (g_stream_started && g_stream_closed) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
}

#if 0 /* 暂不支持 */
/**
 * @brief 测试用例6: WAV 文件播放
 */
static void test_case_wav_file(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: WAV File Play ==========\r\n");

    g_play_finished = false;

    /* 播放 WAV 文件 */
    ret = cm_audio_play_file("/test.wav", CM_AUDIO_PLAY_FORMAT_WAV, NULL,
                             test_play_callback, NULL);

    cm_printf("cm_audio_play_file(WAV) returned: %d\r\n", ret);

    if (ret == 0) {
        cm_printf("Playing WAV file...\r\n");

        /* 等待播放结束 */
        uint32_t wait_count = 0;
        while ((!g_play_finished) && (wait_count < 1000)) {
            osDelay(10);
            wait_count++;
        }
        cm_printf("Play finished: %s\r\n", g_play_finished ? "yes" : "timeout");
    } else {
        cm_printf("Failed to start WAV playback\r\n");
    }

    cm_printf("========== Test Case: WAV File Play End ==========\r\n\r\n");
}
#endif

/**
 * @brief 测试用例: AMR 文件播放
 */
static int32_t test_case_amr_file(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: AMR File Play ==========\r\n");

    g_play_finished = false;

    /* 播放 AMR 文件 */
    ret = cm_audio_play_file("/test.amr", CM_AUDIO_PLAY_FORMAT_AMRNB, NULL,
                             test_play_callback, NULL);

    cm_printf("cm_audio_play_file(AMR) returned: %d\r\n", ret);

    if (ret == 0) {
        cm_printf("Playing AMR file...\r\n");

        /* 等待播放结束 */
        while (!g_play_finished) {
            osDelay(10);
        }
        cm_printf("Play finished: %s\r\n", g_play_finished ? "yes" : "no");
    } else {
        cm_printf("Failed to start AMR playback, ret=%d\r\n", ret);
    }

    cm_printf("AMR file play test result: %s\r\n", (ret == 0) ? "PASS" : "FAIL");
    cm_printf("========== Test Case: AMR File Play End ==========\r\n\r\n");
    return (ret == 0) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
}

/**
 * @brief 录音回调函数
 */
static void test_record_callback(cm_audio_record_event_e event, void *param)
{
    if (event == CM_AUDIO_RECORD_EVENT_DATA) {
        cm_audio_record_data_t *data = (cm_audio_record_data_t *)param;
        if ((g_record_fd != NULL) && (data != NULL) && (data->data != NULL)) {
            int32_t write_len = cm_fs_write(g_record_fd, data->data, data->len);
            if (write_len != (int32_t)data->len) {
                cm_printf("Record write failed: expect %u, got %d\r\n", data->len, write_len);
            }
        }
    }
    else if (event == CM_AUDIO_RECORD_EVENT_FINISHED) {
        cm_printf("Record finished event\r\n");
        g_recording_finished = true;
    }
    else if (event == CM_AUDIO_RECORD_EVENT_INTERRUPT) {
        cm_printf("Record interrupted event\r\n");
        g_recording_finished = true;
    }
}

/**
 * @brief 测试用例: AMR 录音
 * @note 录制 10 秒 AMR，每秒输出倒计时，保存到 record.amr，然后播放验证
 *       AMR 文件头由录音器自动发送，无需手动写入
 */
static int32_t test_case_record_amr(void)
{
    int32_t ret;
    uint32_t record_time_ms = 10000;  /* 录制 10 秒 */
    uint32_t countdown;

    cm_printf("\r\n========== Test Case: Record AMR ==========\r\n");

    /* 检查并删除已存在的文件 */
    if (cm_fs_exist("record.amr") == 1) {
        cm_fs_delete("record.amr");
    }

    /* 打开文件用于保存录音 */
    g_record_fd = cm_fs_open("record.amr", CM_FS_WB);
    if (g_record_fd == NULL) {
        cm_printf("Failed to open record.amr for writing\r\n");
        return TEST_RESULT_FAIL;
    }
    cm_printf("Opened record.amr for recording\r\n");

    /* 开始录音 - AMR 文件头会由录音器自动发送 */
    g_recording_finished = false;
    ret = cm_audio_recorder_start(CM_AUDIO_RECORD_FORMAT_AMRNB_1220, NULL, test_record_callback, NULL);
    cm_printf("cm_audio_recorder_start returned: %d\r\n", ret);

    if (ret != 0) {
        cm_printf("Failed to start recording, ret=%d\r\n", ret);
        cm_fs_close(g_record_fd);
        g_record_fd = NULL;
        return TEST_RESULT_FAIL;
    }

    /* 录音倒计时 */
    cm_printf("Recording for %d seconds...\r\n", record_time_ms / 1000);
    for (countdown = record_time_ms / 1000; countdown > 0; countdown--) {
        cm_printf("Recording: %d seconds remaining...\r\n", countdown);
        osDelay(1000);
    }

    /* 停止录音 */
    cm_audio_recorder_stop();
    cm_printf("Recording stopped\r\n");

    /* 关闭文件 */
    if (g_record_fd != NULL) {
        cm_fs_close(g_record_fd);
        g_record_fd = NULL;
        cm_printf("Saved to record.amr\r\n");
    }

    /* 等待一下再播放 */
    osDelay(500);

    /* 播放录制的 AMR 文件 */
    cm_printf("Playing back record.amr...\r\n");
    g_play_finished = false;
    ret = cm_audio_play_file("record.amr", CM_AUDIO_PLAY_FORMAT_AMRNB, NULL,
                             test_play_callback, NULL);

    if (ret == 0) {
        /* 等待播放结束 */
        while (!g_play_finished) {
            osDelay(10);
        }
        cm_printf("Playback finished\r\n");
    } else {
        cm_printf("Failed to play record.amr, ret=%d\r\n", ret);
        return TEST_RESULT_FAIL;
    }

    cm_printf("Record and playback test result: PASS\r\n");
    cm_printf("========== Test Case: Record AMR End ==========\r\n\r\n");
    return TEST_RESULT_PASS;
}

/**
 * @brief 测试用例: PCM 录音
 * @note 录制 5 秒 PCM，每秒输出倒计时，保存到 record.pcm，然后播放验证
 */
static int32_t test_case_record_pcm(void)
{
    int32_t ret;
    uint32_t record_time_ms = 5000;  /* 录制 5 秒 */
    uint32_t countdown;
    cm_audio_sample_param_t sample_param;

    cm_printf("\r\n========== Test Case: Record PCM ==========\r\n");

    /* 设置 PCM 采样参数 */
    sample_param.rate = CM_AUDIO_SAMPLE_RATE_16000HZ;
    sample_param.sample_format = CM_AUDIO_SAMPLE_FORMAT_16BIT;
    sample_param.num_channels = CM_AUDIO_SOUND_MONO;

    /* 检查并删除已存在的文件 */
    if (cm_fs_exist("record.pcm") == 1) {
        cm_fs_delete("record.pcm");
    }

    /* 打开文件用于保存录音 */
    g_record_fd = cm_fs_open("record.pcm", CM_FS_WB);
    if (g_record_fd == NULL) {
        cm_printf("Failed to open record.pcm for writing\r\n");
        return TEST_RESULT_FAIL;
    }
    cm_printf("Opened record.pcm for recording\r\n");

    /* 开始录音 */
    g_recording_finished = false;
    ret = cm_audio_recorder_start(CM_AUDIO_RECORD_FORMAT_PCM, &sample_param, test_record_callback, NULL);
    cm_printf("cm_audio_recorder_start(PCM) returned: %d\r\n", ret);

    if (ret != 0) {
        cm_printf("Failed to start recording, ret=%d\r\n", ret);
        cm_fs_close(g_record_fd);
        g_record_fd = NULL;
        return TEST_RESULT_FAIL;
    }

    /* 录音倒计时 */
    cm_printf("Recording for %d seconds...\r\n", record_time_ms / 1000);
    for (countdown = record_time_ms / 1000; countdown > 0; countdown--) {
        cm_printf("Recording: %d seconds remaining...\r\n", countdown);
        osDelay(1000);
    }

    /* 停止录音 */
    cm_audio_recorder_stop();
    cm_printf("Recording stopped\r\n");

    /* 关闭文件 */
    if (g_record_fd != NULL) {
        cm_fs_close(g_record_fd);
        g_record_fd = NULL;
        cm_printf("Saved to record.pcm\r\n");
    }

    /* 等待一下再播放 */
    osDelay(500);

    /* 播放录制的 PCM 文件 */
    cm_printf("Playing back record.pcm...\r\n");
    g_play_finished = false;
    ret = cm_audio_play_file("record.pcm", CM_AUDIO_PLAY_FORMAT_PCM, &sample_param,
                             test_play_callback, NULL);

    if (ret == 0) {
        /* 等待播放结束 */
        while (!g_play_finished) {
            osDelay(10);
        }
        cm_printf("Playback finished\r\n");
    } else {
        cm_printf("Failed to play record.pcm, ret=%d\r\n", ret);
        return TEST_RESULT_FAIL;
    }

    cm_printf("Record and playback test result: PASS\r\n");
    cm_printf("========== Test Case: Record PCM End ==========\r\n\r\n");
    return TEST_RESULT_PASS;
}

/**
 * @brief 测试用例: DAC 文件播放 (使用 AMR 文件)
 */
static int32_t test_case_dac_file_play(void)
{
    int32_t ret;

    cm_printf("\r\n========== Test Case: DAC AMR File Play ==========\r\n");

    g_play_finished = false;

    /* 调用后会在关机前会一直记忆使用 DAC */
    cm_audio_player_set_driver(CM_AUDIO_PLAY_DRIVER_DAC);

    /* 播放 AMR 文件 */
    ret = cm_audio_play_file("/test.amr", CM_AUDIO_PLAY_FORMAT_AMRNB, NULL,
                             test_play_callback, NULL);

    cm_printf("cm_audio_play_file(AMR) returned: %d\r\n", ret);

    if (ret == 0) {
        cm_printf("Playing AMR file...\r\n");

        /* 等待播放结束 */
        while (!g_play_finished) {
            osDelay(10);
        }
        cm_printf("Play finished: %s\r\n", g_play_finished ? "yes" : "no");
    } else {
        cm_printf("Failed to start AMR playback, ret=%d\r\n", ret);
    }

    /* 改回使用 codec */
    cm_audio_player_set_driver(CM_AUDIO_PLAY_DRIVER_CODEC);

    cm_printf("========== Test Case: AMR File Play End ==========\r\n\r\n");
    return (ret == 0) ? TEST_RESULT_PASS : TEST_RESULT_FAIL;
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

    cm_printf("\r\n");
    cm_printf("========================================\r\n");
    cm_printf("   CM Audio Test\r\n");
    cm_printf("========================================\r\n");

    /* 避免睡眠模式导致输出异常 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_work_lock();

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

#if (ENABLED_TEST_CASES & TEST_CASE_SET_VOLUME)
    result = test_case_set_volume();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_SET_GAIN)
    result = test_case_set_gain();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_FILE_PLAY)
    result = test_case_file_play();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_PAUSE_RESUME)
    result = test_case_pause_resume();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_STOP)
    result = test_case_stop();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_STREAM_PLAY)
    result = test_case_stream_play();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_STREAM_CB)
    result = test_case_stream_cb();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_WAV_FILE)
    test_case_wav_file();
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_AMR_FILE)
    result = test_case_amr_file();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_RECORD_AMR)
    result = test_case_record_amr();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_RECORD_PCM)
    result = test_case_record_pcm();
    if (result == TEST_RESULT_PASS) {
        pass_count++;
    } else {
        fail_count++;
    }
#endif

#if (ENABLED_TEST_CASES & TEST_CASE_DAC_FILE_PLAY)
    result = test_case_dac_file_play();
    if (result == TEST_RESULT_PASS) {
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

/**
 * @brief 测试用例: 设置播放音量
 */
static int32_t test_case_set_volume(void)
{
    int32_t ret;
    int volume;

    cm_printf("\r\n========== Test Case: Set Volume ==========\r\n");

    /* 测试设置音量 */
    cm_printf("Setting volume to 50...\r\n");
    ret = cm_audio_player_set_volume(50);
    if (ret != 0) {
        cm_printf("Failed to set volume\r\n");
        return TEST_RESULT_FAIL;
    }

    /* 读取音量验证 */
    volume = 0;
    ret = cm_audio_player_get_volume(&volume);
    if (ret != 0) {
        cm_printf("Failed to get volume\r\n");
        return TEST_RESULT_FAIL;
    }
    cm_printf("Current volume: %d\r\n", volume);

    cm_printf("Playing /test.amr with volume 50...\r\n");
    ret = play_amr_and_wait("/test.amr");
    if (ret != 0) {
        return TEST_RESULT_FAIL;
    }
    cm_printf("Playback finished\r\n");

    /* 测试设置最大音量 */
    cm_printf("Setting volume to 100...\r\n");
    ret = cm_audio_player_set_volume(100);
    if (ret != 0) {
        cm_printf("Failed to set volume\r\n");
        return TEST_RESULT_FAIL;
    }

    /* 读取音量验证 */
    volume = 0;
    ret = cm_audio_player_get_volume(&volume);
    if (ret != 0) {
        cm_printf("Failed to get volume\r\n");
        return TEST_RESULT_FAIL;
    }
    cm_printf("Current volume: %d\r\n", volume);

    cm_printf("Playing /test.amr with volume 100...\r\n");
    ret = play_amr_and_wait("/test.amr");
    if (ret != 0) {
        return TEST_RESULT_FAIL;
    }
    cm_printf("Playback finished\r\n");

    /* 测试静音 */
    cm_printf("Setting volume to 0 (mute)...\r\n");
    ret = cm_audio_player_set_volume(0);
    if (ret != 0) {
        cm_printf("Failed to set volume\r\n");
        return TEST_RESULT_FAIL;
    }

    /* 读取音量验证 */
    volume = 0;
    ret = cm_audio_player_get_volume(&volume);
    if (ret != 0) {
        cm_printf("Failed to get volume\r\n");
        return TEST_RESULT_FAIL;
    }
    cm_printf("Current volume: %d\r\n", volume);

    cm_printf("Playing /test.amr with volume 0...\r\n");
    ret = play_amr_and_wait("/test.amr");
    if (ret != 0) {
        return TEST_RESULT_FAIL;
    }
    cm_printf("Playback finished\r\n");

    /* 恢复最大音量 */
    ret = cm_audio_player_set_volume(100);
    if (ret != 0) {
        cm_printf("Failed to restore volume\r\n");
        return TEST_RESULT_FAIL;
    }

    cm_printf("Set volume test result: PASS\r\n");
    cm_printf("========== Test Case: Set Volume End ==========\r\n\r\n");
    return TEST_RESULT_PASS;
}

/**
 * @brief 测试用例: 设置录音增益
 */
static int32_t test_case_set_gain(void)
{
    int32_t ret;
    uint8_t gain;

    cm_printf("\r\n========== Test Case: Set Gain ==========\r\n");

    /* 测试设置增益 24dB */
    cm_printf("Setting gain to 60 (24dB)...\r\n");
    gain = 60;
    ret = cm_audio_record_set_cfg(CM_AUDIO_RECORD_CFG_GAIN, &gain);
    if (ret != 0) {
        cm_printf("Failed to set gain\r\n");
        return TEST_RESULT_FAIL;
    }

    /* 读取增益验证 */
    gain = 0;
    ret = cm_audio_record_get_cfg(CM_AUDIO_RECORD_CFG_GAIN, &gain);
    if (ret != 0) {
        cm_printf("Failed to get gain\r\n");
        return TEST_RESULT_FAIL;
    }
    cm_printf("Current gain: %u (24dB)\r\n", gain);

    /* 测试设置 0dB */
    cm_printf("Setting gain to 36 (0dB)...\r\n");
    gain = 36;
    ret = cm_audio_record_set_cfg(CM_AUDIO_RECORD_CFG_GAIN, &gain);
    if (ret != 0) {
        cm_printf("Failed to set gain\r\n");
        return TEST_RESULT_FAIL;
    }

    /* 读取增益验证 */
    gain = 0;
    ret = cm_audio_record_get_cfg(CM_AUDIO_RECORD_CFG_GAIN, &gain);
    if (ret != 0) {
        cm_printf("Failed to get gain\r\n");
        return TEST_RESULT_FAIL;
    }
    cm_printf("Current gain: %u (0dB)\r\n", gain);

    /* 测试设置 -12dB */
    cm_printf("Setting gain to 24 (-12dB)...\r\n");
    gain = 24;
    ret = cm_audio_record_set_cfg(CM_AUDIO_RECORD_CFG_GAIN, &gain);
    if (ret != 0) {
        cm_printf("Failed to set gain\r\n");
        return TEST_RESULT_FAIL;
    }

    /* 读取增益验证 */
    gain = 0;
    ret = cm_audio_record_get_cfg(CM_AUDIO_RECORD_CFG_GAIN, &gain);
    if (ret != 0) {
        cm_printf("Failed to get gain\r\n");
        return TEST_RESULT_FAIL;
    }
    cm_printf("Current gain: %u (-12dB)\r\n", gain);

    /* 设置为 24dB */
    cm_printf("Setting gain to 60 (24dB)...\r\n");
    gain = 60;
    ret = cm_audio_record_set_cfg(CM_AUDIO_RECORD_CFG_GAIN, &gain);
    if (ret != 0) {
        cm_printf("Failed to set gain\r\n");
        return TEST_RESULT_FAIL;
    }

    /* 读取增益验证 */
    gain = 0;
    ret = cm_audio_record_get_cfg(CM_AUDIO_RECORD_CFG_GAIN, &gain);
    if (ret != 0) {
        cm_printf("Failed to get gain\r\n");
        return TEST_RESULT_FAIL;
    }
    cm_printf("Current gain: %u (24dB)\r\n", gain);

    cm_printf("Set gain test result: PASS\r\n");
    cm_printf("========== Test Case: Set Gain End ==========\r\n\r\n");
    return TEST_RESULT_PASS;
}
