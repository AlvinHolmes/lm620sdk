#include "stddef.h"
#include "stdint.h"
#include "test_data.h"

#include <os.h>

#include "os.h"

#include "audio_local.h"
#include "audio_dev.h"
#include "audio_monitor.h"

static bool g_recordOver = false;
static bool g_playOver = false;
static void testAudioCb(uint32_t event, void *para)
{
    if (event == RECORD_OVER_CB_EVENT) {
        g_recordOver = true;
    }
    if (event == PLAY_OVER_CB_EVENT) {
        g_playOver = true;
    }
}

#define EXAMPLE_AUDIO_DATA_SIZE (32 * 1024)

/* 16 位 * 双声道 * FRAME_SIZE */
static uint16_t audio_buf[EXAMPLE_AUDIO_DATA_SIZE * 2];

void test_audio_play(void)
{
    AudioDevCfg devCfg = {0};
    devCfg.chnls        = 1;
    devCfg.sampleRate   = 8000;
    devCfg.dataBits     = 16;
    devCfg.frameSize    = AUDIO_DEFAULT_FRAME_SIZE;
    devCfg.ctrl         = AUDIO_PLAY_RECORD_MODE;
    devCfg.dev          = AUDIO_DEV_I2S_CODEC;
    AudioDev_Init(&devCfg);

    AudioInfo info = {0};
    info.isLoop     = false;
    info.audioType  = WAV_FILE_DATA;
    info.sampleRate = 8000;
    info.dataBits   = 16;
    info.chnls      = 1;
    info.frameSize  = AUDIO_DEFAULT_FRAME_SIZE;
    info.cb         = testAudioCb;

    /* AUDIO_PLAY_RECORD_MODE 工作不正常，不使用 pingpong，录一会播放一会 */
    while (1) {
        info.ctrl       = AUDIO_RECORD_MODE;
        info.recordData = (uint8_t *)audio_buf;
        info.recordSize = EXAMPLE_AUDIO_DATA_SIZE * 2;
        g_recordOver    = false;
        AudioLocal_Open(&info);
        while (!g_recordOver) {
            osThreadMsSleep(10);
        }
        AudioLocal_Close(AUDIO_RECORD_MODE);

        info.ctrl       = AUDIO_PLAY_MODE;
        info.data       = (uint8_t *)audio_buf;
        info.size       = EXAMPLE_AUDIO_DATA_SIZE * 2;
        g_playOver      = false;
        AudioLocal_Open(&info);
        while (!g_playOver) {
            osThreadMsSleep(10);
        }
        AudioLocal_Close(AUDIO_PLAY_MODE);
    }
}



// void test_audio_play(void)
// {
//     AudioDevCfg devCfg = {0};

//     devCfg.chnls        = 1;
//     devCfg.sampleRate   = 8000;
//     devCfg.dataBits     = 16;
//     devCfg.frameSize    = AUDIO_DEFAULT_FRAME_SIZE;
//     devCfg.ctrl         = AUDIO_PLAY_MODE;
//     devCfg.dev          = AUDIO_DEV_I2S_CODEC;

//     AudioDev_Init(&devCfg);

//     AudioInfo info = {0};
//     info.ctrl       = AUDIO_RECORD_MODE;
//     info.isLoop     = false;

//     // info.audioType  = PCM_RAW_DATA;
//     // info.sampleRate = 8000;
//     // // info.recordData = osMalloc(32 * 1024);
//     // info.recordData = osMallocTrace(32 * 1024, "test_data_pcm.c", __LINE__);
//     // info.recordSize = 32 * 1024;

//     info.audioType = WAV_FILE_DATA;
//     info.sampleRate = 8000;
//     info.dataBits = 16;
//     info.chnls = 1;
//     info.frameSize = 1024;
//     info.recordData = osMallocTrace(32 * 1024, "test_data_pcm.c", __LINE__);;
//     info.recordSize = 32 * 1024;

//     info.cb         = testAudioCb;

//     while (1) {
//         info.ctrl = AUDIO_RECORD_MODE;
//         g_recordOver = false;
//         AudioLocal_Open(&info);
//         while (!g_recordOver) {
//             osThreadMsSleep(10);
//         }
//         // AudioLocal_Close(AUDIO_RECORD_MODE);

//         info.ctrl = AUDIO_PLAY_MODE;
//         info.data = info.recordData;
//         info.size = info.recordSize;
//         AudioLocal_Open(&info);
//         g_playOver = false;
//         while (!g_playOver) {
//             osThreadMsSleep(10);
//         }
//         // AudioLocal_Close(AUDIO_PLAY_MODE);
//         // osDelayMs(10);
//     }

//     osFreeTrace(info.recordData, "test_data_pcm.c", __LINE__);

//     // uint32_t cnt_write_total = 0;
//     // uint32_t cnt_write_once = 0;

//     // while (1) {
//     //     xt_log_info("test_audio_play: PCM RAW");
//     //     cnt_write_total = 0;
//     //     cnt_write_once = 0;
//     //     while (cnt_write_total < test_data_size()) {
//     //         cnt_write_once = devCfg.frameSize;
//     //         if ((cnt_write_once + devCfg.frameSize) > test_data_size()) {
//     //             cnt_write_once = test_data_size() - cnt_write_total;
//     //         }
//     //         AudioDev_Write(devCfg.dev, (uint8_t *)&test_data[cnt_write_total], cnt_write_once, osWaitForever);
//     //         cnt_write_total += cnt_write_once;
//     //     }
//     //     osDelayMs(100);
//     // }
// }

