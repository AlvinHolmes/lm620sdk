#include "stddef.h"
#include "stdint.h"
#include "test_data.h"

#include <os.h>
#include "audio_local.h"
#include "audio_dev.h"
#include "audio_monitor.h"

#include "wav.c"

static unsigned int test_data_size(void)
{
    return sizeof(test_data);
}

static volatile uint8_t b_done = false;

static void _AudioCallBack(uint32_t event, void *para)
{
    b_done = true;
}

#include "os.h"

AudioMonitorHandle *g_audio_monitor = NULL;

void test_audio_play(void)
{
    LocalAudioCfg cfg = {0};
    cfg.audioType = WAV_FILE_DATA;
    cfg.ctrl = AUDIO_PLAY_MODE;
    cfg.data = test_data;
    cfg.size = test_data_size();
    while (1) {
        osPrintf("test_audio_play: WAV FILE DATA");
        b_done = false;
        g_audio_monitor = Audio_LocalOpen(POWER_ON_AUDIO_PRIO, _AudioCallBack, &cfg);
        while (b_done == false) {
            osDelay(100);
        }
    }
}
