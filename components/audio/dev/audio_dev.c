#include "os.h"
#include "drv_i2s.h"
#include "drv_dac.h"
#include "drv_pwm_audio.h"
#include "audio_codec.h"
#include "audio_local.h"
#include "audio_dev.h"
#include "audio_pa.h"

#if defined(USE_MOBVOI_DSP)
#include "voice_process.h"
#if defined(USE_CMAPI) 
#include "cm_audio_local.h"
#endif
#endif

int8_t AudioDev_Init(AudioDevCfg *devCfg)
{
    if(devCfg->dev == AUDIO_DEV_I2S_CODEC)
    {
        I2S_BusCfg busCfg = {0};
        I2S_TimingCfg i2sTimingCfg = {0};
        VoiceProcFunc voiceProc = {0};

        i2sTimingCfg.chnl = I2S_LEFT_CHNL;
        if(devCfg->chnls == 2)
            i2sTimingCfg.chnl = I2S_DOUBLE_CHNL;

        i2sTimingCfg.dataBits = devCfg->dataBits;
        i2sTimingCfg.dataCycle = DATA_32_BITS;
        busCfg.frameSize = devCfg->frameSize;
        busCfg.workMode = devCfg->ctrl;
        busCfg.timing = TIMING_I2S;
        busCfg.timingCfg = &i2sTimingCfg;
        busCfg.sampleRate = devCfg->sampleRate;
        busCfg.fs = Codec_GetFs(busCfg.sampleRate);


#if defined(USE_MOBVOI_DSP)
        if(devCfg->ctrl & I2S_RECORD_MODE)
        {
#if defined(USE_CMAPI)
        /* CM 模式下，根据 Mobvoi 使能状态配置 VoiceProcFunc */
            if (CmAudio_GetMobvoiEnabled()) {
                static VoiceProcFunc cmVoiceFunc = {0};
                cmVoiceFunc.init = VoiceProc_Initialize;
                cmVoiceFunc.deinit = VoiceProc_UnInitialize;
                cmVoiceFunc.rx_proc_notify = VoiceProc_SendMsg;
                cmVoiceFunc.tx_proc = VoiceProc_TxProc;
                voiceProc = cmVoiceFunc;
            }
#else
            voiceProc.init = VoiceProc_Initialize;
            voiceProc.deinit = VoiceProc_UnInitialize;
            voiceProc.rx_proc_notify = VoiceProc_SendMsg;
            voiceProc.tx_proc = VoiceProc_TxProc;
#endif
        }
#endif

        I2S_UnInitialize();
        if(I2S_Initialize(&busCfg, NULL, &voiceProc) < 0)
        {
            osPrintf("i2s init failed %d\r\n", devCfg->ctrl);
            return DRV_ERR;
        }
        //codec config
        Codec_Initialize(devCfg->sampleRate, devCfg->dataBits);  

        Codec_Start(devCfg->ctrl);
        if(devCfg->ctrl | I2S_RECORD_MODE)
            osThreadMsSleep(200);

        I2S_BusStart(devCfg->ctrl);
    }
    else if(devCfg->dev == AUDIO_DEV_DAC)
    {
        DacCfg dacCfg = {0};

        dacCfg.sampleRate = devCfg->sampleRate;
        dacCfg.sampleWidth = devCfg->dataBits;
        dacCfg.frameSize = devCfg->frameSize;
        if(devCfg->chnls == 2)
            dacCfg.frameSize = devCfg->frameSize/2;

        Dac_Open(&dacCfg);
        PA_Enable();
    }
    else if(devCfg->dev == AUDIO_DEV_PWM)
    {
        PwmAudioCfg pwmCfg = {0};

        pwmCfg.sampleRate = devCfg->sampleRate;
        pwmCfg.sampleWidth = devCfg->dataBits;
        pwmCfg.frameSize = devCfg->frameSize;
        PwmAudio_Open(&pwmCfg);
        PA_Enable();
    }
    else
    {
        osPrintf("AudioDev_Init, Not Support Audio Dev\r\n");
        return DRV_ERR_UNSUPPORTED;
    }

    osPrintf("AudioDev_Init Ok \r\n"); 
    return DRV_OK;
}


int8_t AudioDev_Write(AudioDev dev, uint8_t *audioData, uint32_t size, uint32_t timeOut)
{
    if(dev == AUDIO_DEV_I2S_CODEC)
    {
        if(timeOut != 0)
        {
            I2S_BusWrite(audioData, size, timeOut);
            return DRV_OK;
        }
        else
        {
            if(I2S_BusGetAvaliableBufCount(I2S_PLAY_MODE) > 0)
            {
                I2S_BusWrite(audioData, size, timeOut);

                return DRV_OK;
            }
            else
            {
                return DRV_ERR_BUSY;
            }
        }
    }
    else if(dev == AUDIO_DEV_DAC)
    {
        if(Dac_Write(audioData, size, timeOut) > 0)
            return DRV_OK;
        else
            return DRV_ERR_BUSY;
    }
    else if(dev == AUDIO_DEV_PWM)
    {
        if(PwmAudio_Write(audioData, size, timeOut) > 0)
            return DRV_OK;
        else
            return DRV_ERR_BUSY;
    }
    else
    {
        osPrintf("AudioDev_Write, Not Support Audio Dev\r\n");
        return DRV_ERR_UNSUPPORTED;
    }
}

int32_t AudioDev_Read(AudioDev dev, uint8_t *audioData, uint32_t size, uint32_t timeOut)
{
    if(dev == AUDIO_DEV_I2S_CODEC)
    {
        if(I2S_BusGetAvaliableBufCount(I2S_RECORD_MODE) > 0)
        {
            return I2S_BusRead(audioData, size, timeOut);
        }
        else
        {
            return DRV_ERR_BUSY;
        }
    }
    else
    {
        osPrintf("AudioDev_Read, Not Support Audio Dev\r\n");
        return DRV_ERR_UNSUPPORTED;
    }  
}


void AudioDev_Deinit(AudioDevCfg *devCfg, bool forceStop)
{
    if(devCfg->dev == AUDIO_DEV_I2S_CODEC)
    {
        //非强制退出，需预留时间给底层buf 播放完
        if((devCfg->ctrl & I2S_PLAY_MODE) && !forceStop)
        {
            uint8_t waitFrameCnt = I2S_FRAME_BUF_NUM - I2S_BusGetAvaliableBufCount(I2S_PLAY_MODE);

            if(waitFrameCnt > I2S_FRAME_BUF_NUM)
                waitFrameCnt = I2S_FRAME_BUF_NUM;

            osThreadMsSleep( waitFrameCnt * (1000 * devCfg->frameSize)/(devCfg->sampleRate * devCfg->dataBits / 8));
        }
        
        Codec_Stop(devCfg->ctrl);
        I2S_BusStop(devCfg->ctrl);
        I2S_UnInitialize();
        Codec_UnInitialize();
    }
    else if(devCfg->dev == AUDIO_DEV_DAC)
    {
        PA_Disable();
        Dac_Close();
    }
    else if(devCfg->dev == AUDIO_DEV_PWM)
    {
        PA_Disable();
        PwmAudio_Close();
    }
    else
    {
        osPrintf("AudioDev_Deinit, Not Support Audio Dev\r\n");
    }

    osPrintf("AudioDev_Deinit over \r\n");
}
