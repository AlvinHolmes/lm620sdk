#include "stddef.h"
#include "stdint.h"
#include "test_data.h"

#include <os.h>

#include "os.h"

#include "audio_local.h"
#include "audio_dev.h"
#include "audio_monitor.h"

#define EXAMPLE_SINE_WAVE   0
#define EXAMPLE_VOICE       1
#define EXAMPLE             EXAMPLE_SINE_WAVE

#if (EXAMPLE == EXAMPLE_SINE_WAVE)
static const int16_t test_data[] = {
#include "audio_data_pcm_sine_wave_8k_16b_1ch.csv"
};
#elif (EXAMPLE == EXAMPLE_VOICE)
#include "pcm.c"
#endif

static unsigned int test_data_size(void)
{
    return sizeof(test_data);
}

void test_audio_play(void)
{
    AudioDevCfg devCfg = {0};

    devCfg.chnls        = 1;
    devCfg.sampleRate   = 8000;
    devCfg.dataBits     = 16;
    devCfg.frameSize    = AUDIO_DEFAULT_FRAME_SIZE;
    devCfg.ctrl         = AUDIO_PLAY_MODE;
    devCfg.dev          = AUDIO_DEV_I2S_CODEC;

    AudioDev_Init(&devCfg);

    uint32_t cnt_write_total = 0;
    uint32_t cnt_write_once = 0;

    while (1) {
        osPrintf("test_audio_play: PCM RAW");
        cnt_write_total = 0;
        cnt_write_once = 0;
        while (cnt_write_total < test_data_size()) {
            cnt_write_once = devCfg.frameSize;
            if ((cnt_write_once + devCfg.frameSize) > test_data_size()) {
                cnt_write_once = test_data_size() - cnt_write_total;
            }
            AudioDev_Write(devCfg.dev, &((uint8_t *)test_data)[cnt_write_total], cnt_write_once, osWaitForever);
            cnt_write_total += cnt_write_once;
        }
    }
}
