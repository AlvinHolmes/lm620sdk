

#include "os.h"
#include "audio_monitor.h"
#include "audio_codec.h"
#include "nvparam_pubcfg.h"
#include "nvparam_top.h"
#include "nvm.h"


//#define AUDIO_MONITOR_DEBUG_PRINTF
#ifdef  AUDIO_MONITOR_DEBUG_PRINTF
    #define AUDIO_PRINTF        osPrintf
#else
    #define AUDIO_PRINTF(...)
#endif 


// static AudioSate g_audioState = AUDIO_IDLE_STATE;
static AudioMonitorHandle *g_curAudioMonitorHandle = NULL;

//todo Mutex

AudioMonitorHandle* Audio_Request(uint8_t prio, uint8_t type, AudioCallBack cb)
{
    AUDIO_PRINTF("audio request \r\n");
    if(cb == NULL && type != LOCAL_AUDIO)
    {
        osPrintf("Audio Request Cb is NULL \r\n");
        return NULL;
    }
        
    if(g_curAudioMonitorHandle && g_curAudioMonitorHandle->prio > prio)
    {
        osPrintf(" high prio is running %d, %d \r\n", g_curAudioMonitorHandle->prio, prio);
        return NULL;
    }

    AudioMonitorHandle *handle = (AudioMonitorHandle *)osMalloc(sizeof(AudioMonitorHandle));
    if(g_curAudioMonitorHandle)
    {
        AUDIO_PRINTF("force quit %d, %d\r\n",  g_curAudioMonitorHandle->prio, prio);
        if(g_curAudioMonitorHandle->cb)
            g_curAudioMonitorHandle->cb(SCHEDULE_QUIT_EVENT, NULL);

        // 强制关闭低优先级
        g_curAudioMonitorHandle->close(AUDIO_RECORD_MODE | AUDIO_PLAY_MODE);
    }

    if(!handle)
    {
        return NULL;
    }

    memset(handle, 0, sizeof(AudioMonitorHandle));
    handle->prio = prio;
    if(type == LOCAL_AUDIO)
    {
        handle->cb = cb;
        handle->open = AudioLocal_Open;
        handle->close = AudioLocal_Close;
    }
#if 0
    else if(type == VOICE_CALL)
    {
        handle->cb = cb;
        handle->open = Voice_Open;
        handle->close = Voice_Close;
        handle->start = Voice_Start;
        handle->stop = Voice_Stop;
        handle->read = Voice_ReadOneFrame;
        handle->write = Voice_WriteOneFrame;
    }
#endif
    else
    {
        osFree(handle);
        osPrintf("audio request type error : %d \r\n", type);
        return NULL;
    }

    g_curAudioMonitorHandle = handle;
    return handle;
}

void Audio_Release(AudioMonitorHandle** audioHandle)
{
    AUDIO_PRINTF("audio release \r\n");
    if(audioHandle)
    {
        if((*audioHandle) && ((*audioHandle) == g_curAudioMonitorHandle))
        {
            osFree(*audioHandle);
            *audioHandle = NULL;
            g_curAudioMonitorHandle = NULL;
        }
    }
}

bool Audio_IsIdle(void)
{
    if(g_curAudioMonitorHandle)
        return false;
    else
        return true;
}

int8_t Audio_SetVolume(uint8_t level)
{
    if(level >= AUDIO_VOLUME_MAX_LEVEL)
        level = AUDIO_VOLUME_MAX_LEVEL - 1;
    Codec_SetVolumeLevel(level);

    return 0;
}

static AudioMonitorHandle *g_localAudioHandle = NULL;
void Audio_SysCb (uint32_t event, void *para)
{
    Audio_Release(&g_localAudioHandle);
}

AudioMonitorHandle* Audio_LocalOpen(uint8_t prio, AudioCallBack cb, LocalAudioCfg *cfg)
{
    AudioMonitorHandle *audioHandle = NULL;

    audioHandle = Audio_Request(prio, LOCAL_AUDIO, cb);
    if(!audioHandle)
        return NULL;

    g_localAudioHandle = audioHandle;
    cfg->sysCb = Audio_SysCb;
    cfg->cb = cb;
    if(0 != g_localAudioHandle->open(cfg))
    {
        return NULL;
    }

    return audioHandle;
}

int8_t Audio_LocalClose(AudioMonitorHandle *audioHandle, uint8_t ctrl)
{
    if(!audioHandle || !g_localAudioHandle || (g_localAudioHandle != audioHandle))
        return -1;
    
    g_localAudioHandle->close(ctrl);
    Audio_Release(&g_localAudioHandle);

    return 0;
}

