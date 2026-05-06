
#include "os.h"
#include "drv_i2s.h"
#include "audio_codec.h"
#if defined(CODEC_ES8311)
#include "es8311.h"
#elif defined(CODEC_PT8311)
#include "pt8311_drv.h"
#elif defined(CODEC_CJC8910)
#include "cjc8910.h"
#endif
#include "audio_pa.h"
#include "drv_pin.h"

#include "nvparam_pubcfg.h"
#include "nvparam_top.h"
#include "nvm.h"

static CodecPin *g_CodecPinPtr = NULL;
static NV_AudioCfgItem *g_audioCfg = NULL;

const static int32_t g_CodecVolumeDefaultLevel[] = 
{
    -6000,  -5000, -4000, -3000, -2000, -1000, -1000, -1000, -1000, -1000
};

void Codec_PinBoardInit(CodecPin *pin)
{
    g_CodecPinPtr = pin;
}

void Codec_PinInit(void)
{
    PIN_SetMux(g_CodecPinPtr->codecWs.pinRes, g_CodecPinPtr->codecWs.func);
    PIN_SetMux(g_CodecPinPtr->codecSck.pinRes, g_CodecPinPtr->codecSck.func);
    PIN_SetMux(g_CodecPinPtr->codecIn.pinRes, g_CodecPinPtr->codecIn.func);
	PIN_SetMux(g_CodecPinPtr->codecOut.pinRes, g_CodecPinPtr->codecOut.func); 

    if(g_CodecPinPtr->codecMclk.pinRes) 
        PIN_SetMux(g_CodecPinPtr->codecMclk.pinRes, g_CodecPinPtr->codecMclk.func);  
}

void Codec_PinUnInit(void)
{
    PIN_SetMux(g_CodecPinPtr->codecWs.pinRes, 0);
    GPIO_SetDir(g_CodecPinPtr->codecWs.pinRes, GPIO_OUTPUT);
    GPIO_Write(g_CodecPinPtr->codecWs.pinRes, GPIO_LOW);

    PIN_SetMux(g_CodecPinPtr->codecSck.pinRes, 0);
    GPIO_SetDir(g_CodecPinPtr->codecSck.pinRes, GPIO_OUTPUT);
    GPIO_Write(g_CodecPinPtr->codecSck.pinRes, GPIO_LOW);

    PIN_SetMux(g_CodecPinPtr->codecIn.pinRes, 0);
    GPIO_SetDir(g_CodecPinPtr->codecIn.pinRes, GPIO_OUTPUT);
    GPIO_Write(g_CodecPinPtr->codecIn.pinRes, GPIO_LOW);

	PIN_SetMux(g_CodecPinPtr->codecOut.pinRes, 0); 
    GPIO_SetDir(g_CodecPinPtr->codecOut.pinRes, GPIO_OUTPUT);
    GPIO_Write(g_CodecPinPtr->codecOut.pinRes, GPIO_LOW); 

    if(g_CodecPinPtr->codecMclk.pinRes) 
    {
        PIN_SetMux(g_CodecPinPtr->codecMclk.pinRes, 0); 
        GPIO_SetDir(g_CodecPinPtr->codecMclk.pinRes, GPIO_OUTPUT);
        GPIO_Write(g_CodecPinPtr->codecMclk.pinRes, GPIO_LOW);
    }
}

static void Codec_EnablePA(void)
{
    PA_Enable();
}

static void Codec_DisablePA(void)
{
    PA_Disable();
}

uint16_t Codec_GetFs(uint16_t sampleRate)
{
#if defined(CODEC_ES8311)
    return I2S_64FS;
#elif defined(CODEC_PT8311)
    /* TODO Confirm 128 ??   */
    return I2S_128FS;
#elif defined(CODEC_CJC8910)
    return cjc8910_get_fs(sampleRate);
#else
    return I2S_64FS;
#endif
} 


int8_t Codec_Initialize(uint32_t sampleRate, uint8_t dataBits)
{
    int ret = CODEC_OK;
    Codec_PinInit();
    Codec_DisablePA();
#if defined(CODEC_ES8311)
    es8311_i2c_config(g_CodecPinPtr->busNo);
    es8311_codec_init(sampleRate, I2S_64FS *sampleRate, MEDIA_HAL_MODE_SLAVE);

    es8311_codec_config_format(MEDIA_HAL_CODEC_MODE_BOTH, MEDIA_HAL_I2S_LEFT);
    es8311_codec_set_bits_per_sample(MEDIA_HAL_CODEC_MODE_BOTH, dataBits);
    //es8311_codec_ctrl_state(MEDIA_HAL_CODEC_MODE_BOTH, MEDIA_HAL_START_STATE);
#elif defined(CODEC_PT8311)
    ret = pt8311_i2c_config(g_CodecPinPtr->busNo);
    if (ret != 0)
    {
        return CODEC_ERR;
    }
    pt8311_codec_init(sampleRate, Codec_GetFs(sampleRate) * sampleRate, MEDIA_HAL_MODE_SLAVE);
    pt8311_codec_config_format(MEDIA_HAL_CODEC_MODE_BOTH, MEDIA_HAL_I2S_LEFT);
    pt8311_codec_set_bits_per_sample(MEDIA_HAL_CODEC_MODE_BOTH, dataBits);
    //pt8311_codec_ctrl_state(MEDIA_HAL_CODEC_MODE_BOTH, MEDIA_HAL_START_STATE);
#elif defined(CODEC_CJC8910)
    cjc8910_i2c_config(g_CodecPinPtr->busNo);
    cjc8910_init(sampleRate);
    cjc8910_codec_config_format(MEDIA_HAL_CODEC_MODE_BOTH, MEDIA_HAL_I2S_LEFT);
    cjc8910_codec_set_bits_per_sample(MEDIA_HAL_CODEC_MODE_BOTH, dataBits);
#endif

    if(!g_audioCfg)
    {
        g_audioCfg = (NV_AudioCfgItem *)osMalloc(sizeof(NV_AudioCfgItem));
        OS_ASSERT(g_audioCfg);

        Codec_LoadAllCfgFromNv();
        Codec_SyncAllToDev();
    }
    return ret;
}

int8_t Codec_UnInitialize(void)
{
    if(g_audioCfg)
    {
        Codec_DisablePA();
        Codec_PinUnInit();

        osFree(g_audioCfg);
        g_audioCfg = NULL;
    } 
    
    return 0;
}

int8_t Codec_Start(uint8_t mode)
{
    if(!g_audioCfg)
        return CODEC_ERR_EMPTY;

    if(mode & CODEC_PLAY)
    {
#if defined(CODEC_ES8311)
        es8311_codec_start(MEDIA_HAL_CODEC_MODE_DECODE);
#elif defined(CODEC_PT8311)
        pt8311_codec_start(MEDIA_HAL_CODEC_MODE_DECODE);
#elif defined(CODEC_CJC8910)
        cjc8910_codec_start(MEDIA_HAL_CODEC_MODE_DECODE);
#endif
        osThreadMsSleep(50);
        Codec_EnablePA();
    }

    if(mode & CODEC_RECORD)
    {
#if defined(CODEC_ES8311)
        es8311_codec_start(MEDIA_HAL_CODEC_MODE_ENCODE);
#elif defined(CODEC_PT8311)
        pt8311_codec_start(MEDIA_HAL_CODEC_MODE_ENCODE);
#elif defined(CODEC_CJC8910)
        cjc8910_codec_start(MEDIA_HAL_CODEC_MODE_ENCODE);
#endif
    }
    Codec_SyncAllToDev();
    return CODEC_OK;
}

int8_t Codec_Stop(uint8_t mode)
{
    if(!g_audioCfg)
        return CODEC_ERR_EMPTY;
       
    if(mode & CODEC_PLAY)
    {
        Codec_DisablePA();
#if defined(CODEC_ES8311)
        es8311_codec_stop(MEDIA_HAL_CODEC_MODE_DECODE);
#elif defined(CODEC_PT8311)
        pt8311_codec_stop(MEDIA_HAL_CODEC_MODE_DECODE);
#elif defined(CODEC_CJC8910)
        cjc8910_codec_stop(MEDIA_HAL_CODEC_MODE_DECODE);
#endif
    }

    if(mode & CODEC_RECORD)
    {
#if defined(CODEC_ES8311)
        es8311_codec_stop(MEDIA_HAL_CODEC_MODE_ENCODE);
#elif defined(CODEC_PT8311)
        pt8311_codec_stop(MEDIA_HAL_CODEC_MODE_ENCODE);
#elif defined(CODEC_CJC8910)
        cjc8910_codec_stop(MEDIA_HAL_CODEC_MODE_ENCODE);
#endif
    }

    return CODEC_OK;
}

// int8_t Codec_SetUserVolumeLevel(uint8_t level)
// {
//     if(level > AUDIO_VOLUME_MAX_LEVEL)
//         return CODEC_ERR_PARAMETER;

//     g_userVolumeLevel = level;

//     return CODEC_OK;
// }

int8_t Codec_SetVolume(uint8_t volume)
{
    if ((volume % AUDIO_VOLUME_MAX_LEVEL) != 0) // (0, 10 ,20  .... 100)
        return CODEC_ERR_PARAMETER;

    if(volume == 0)
    {
#if defined(CODEC_ES8311)
        es8311_codec_set_mute(1);
#elif defined(CODEC_PT8311)
        pt8311_codec_set_mute(1);
#elif defined(CODEC_CJC8910)
        cjc8910_codec_set_mute(1);
#endif
        return CODEC_OK;

    }  
    else
    {
        return Codec_SetVolumeLevel((volume / AUDIO_VOLUME_MAX_LEVEL) - 1);    // 0 - 9
    }
}

int8_t Codec_GetVolume(int *volume)
{
    if (volume == OS_NULL)
        return CODEC_ERR_PARAMETER;

    if (!g_audioCfg)
        return CODEC_ERR_EMPTY;

    *volume = AUDIO_VOLUME_MAX_LEVEL * (g_audioCfg->userVolumeLevel + 1);

    return CODEC_OK;
}

int8_t Codec_SetVolumeLevel(uint8_t level)
{
    if (level > AUDIO_VOLUME_MAX_LEVEL || !g_audioCfg)
        return CODEC_ERR_PARAMETER;

    if(g_audioCfg->userVolumeLevel != level)
    {
        g_audioCfg->userVolumeLevel = level;
        NVM_Write(NV_ITEM_ADDR(pubConfig.AudioCfg.MusicCfg.userVolumeLevel), (uint8_t *)&g_audioCfg->userVolumeLevel, sizeof(uint8_t));
    }
    Codec_SetDacDigitGain((uint8_t *)&g_audioCfg->volumeLevel[level], 4);

    return CODEC_OK;
}

int8_t  Codec_LoadAllCfgFromDev(void)
{
    uint8_t len = 0;
    if(!g_audioCfg)
        return CODEC_ERR;

    Codec_GetAdcDigitGain((uint8_t *)&g_audioCfg->AdcDigitGain, &len);
    Codec_GetDacDigitGain((uint8_t *)&g_audioCfg->DacDigitGain, &len);
    Codec_GetAdcPgaGain((uint8_t *)&g_audioCfg->AdcPgaGain, &len);
    Codec_GetMicGain((uint8_t *)&g_audioCfg->MicGain, &len);
    Codec_GetNrGate((uint8_t *)&g_audioCfg->NrGate, &len);
    Codec_GetSideTone((uint8_t *)&g_audioCfg->SideTone, &len);
    Codec_GetAdcEQ((uint8_t *)g_audioCfg->AdcEQ, &len);
    Codec_GetDacEQ((uint8_t *)g_audioCfg->DacEQ, &len);
    Codec_GetAlc((uint8_t *)g_audioCfg->Alc, &len);
    Codec_GetDrc((uint8_t *)g_audioCfg->Drc, &len);

    g_audioCfg->userVolumeLevel = AUDIO_VOLUME_MAX_LEVEL - 2;
    memcpy(g_audioCfg->volumeLevel, g_CodecVolumeDefaultLevel, AUDIO_VOLUME_MAX_LEVEL * 4);
    for(uint8_t i = 0; i < AUDIO_VOLUME_MAX_LEVEL; i++)
        g_audioCfg->volumeLevel[i] = HTONL(g_audioCfg->volumeLevel[i]);  
    
    osPrintf("Dev: AdcDigitGain: %x, DacDigitGain: %x, AdcPgaGain: %x, MicGain: %x, NrGate: %x, sideTone: %x \r\n",
    g_audioCfg->AdcDigitGain, g_audioCfg->DacDigitGain, g_audioCfg->AdcPgaGain, g_audioCfg->MicGain, g_audioCfg->NrGate, g_audioCfg->SideTone);
    return CODEC_OK;
}

void Codec_LoadAllCfgFromNv(void)
{
    uint32_t audioCfgNvAddr = 0;
    audioCfgNvAddr = NV_ITEM_ADDR(pubConfig.AudioCfg.MusicCfg.codecID);

   if(NVM_Read(audioCfgNvAddr, (uint8_t *)g_audioCfg, sizeof(NV_AudioCfgItem)) != 0 || g_audioCfg->magicNumber != AUDIO_NV_MAGIC_NUMBER)
    {
        osPrintf("load nv audio cfg error !\r\n");
        Codec_LoadAllCfgFromDev();
    }
    else
    {
        g_audioCfg->AdcDigitGain = HTONL(g_audioCfg->AdcDigitGain);
        g_audioCfg->DacDigitGain = HTONL(g_audioCfg->DacDigitGain);
        g_audioCfg->AdcPgaGain = HTONL(g_audioCfg->AdcPgaGain);
        g_audioCfg->MicGain = HTONL(g_audioCfg->MicGain);
        g_audioCfg->NrGate = HTONL(g_audioCfg->NrGate);

        for(uint8_t i = 0; i < AUDIO_VOLUME_MAX_LEVEL; i++)
            g_audioCfg->volumeLevel[i] = HTONL(g_audioCfg->volumeLevel[i]);  
    }
/*
    osPrintf("\r\naudio NV %d bytes: AdcDigitGain: %x, DacDigitGain: %x, AdcPgaGain: %x, MicGain: %x, NrGate: %x, sideTone: %x \r\n",
    sizeof(T_Cust_AudioCfg), g_audioCfg->AdcDigitGain, g_audioCfg->DacDigitGain, g_audioCfg->AdcPgaGain, g_audioCfg->MicGain, g_audioCfg->NrGate, g_audioCfg->SideTone);
*/
}

int8_t Codec_SyncAllToDev(void)
{
    if(!g_audioCfg)
        return CODEC_ERR;

    Codec_SetAdcDigitGain((uint8_t *)&g_audioCfg->AdcDigitGain, 4);
    Codec_SetDacDigitGain((uint8_t *)&g_audioCfg->DacDigitGain, 4);
    Codec_SetAdcPgaGain((uint8_t *)&g_audioCfg->AdcPgaGain,4);
    Codec_SetMicGain((uint8_t *)&g_audioCfg->MicGain, 4);
    Codec_SetNrGate((uint8_t *)&g_audioCfg->NrGate, 4);
    Codec_SetSideTone((uint8_t *)&g_audioCfg->SideTone, 1);
    Codec_SetAdcEQ((uint8_t *)g_audioCfg->AdcEQ, 46);
    Codec_SetDacEQ((uint8_t *)g_audioCfg->DacEQ, 32);
    Codec_SetAlc((uint8_t *)g_audioCfg->Alc, 12);
    Codec_SetDrc((uint8_t *)g_audioCfg->Drc, 10);
    Codec_SetVolumeLevel(g_audioCfg->userVolumeLevel);
    return CODEC_OK;
}

static const CodecParaRange g_CodecParaRange [] = 
{
#if defined(CODEC_ES8311)
    {1, -9550,   3200,   50},           //x100
    {2, -9550,   3200,   50},           //x100
    {3,     0,   3000,  300},           //x100
    {4,     0,   4200,  600},           //x100
    {5, -9600,  -3000,  600},           //x100
    {8,     0,      1,      1},           //x1
    {10,    8,     32,      8},           //x1
#endif
};

static const uint32_t g_CodecSupportSampleRate[] = 
{
    8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 
    48000, 64000, 88200, 96000
};

int8_t Codec_CheckParaRange(uint8_t paraNo, uint8_t *para, uint8_t len)
{
    uint8_t i = 0;
    int32_t paraVal = 0;

    for(i = 0; i < sizeof(g_CodecParaRange)/sizeof(CodecParaRange); i++)
    {
        if(g_CodecParaRange[i].paraNo == paraNo)
        {
            switch (len)
            {
            case 1:
                paraVal = ((int32_t)para[0]) & 0x000000ff ;
                break;
            case 2:
                paraVal = (int32_t)((para[0] << 8) + para[1]) & 0x0000ffff;
                break;
            case 4:
                paraVal = (int32_t)((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]);
                break;
            default:
                return -1;
                break;
            }

            if( (paraVal < g_CodecParaRange[i].minVal) || (paraVal > g_CodecParaRange[i].MaxVal) ||
                ((paraVal - g_CodecParaRange[i].minVal)% g_CodecParaRange[i].step) != 0)
            {
                osPrintf("audio check para invalid%d, %d, %d \r\n",paraVal, 
                                                    g_CodecParaRange[i].minVal, 
                                                    g_CodecParaRange[i].MaxVal);
                return -1;
            }

            break;
        }
    }

    return 0;
}

int8_t Codec_CheckSampleRate(uint32_t sampleRate)
{
    uint8_t i = 0;

    for(i = 0; i < sizeof(g_CodecSupportSampleRate)/sizeof(uint32_t); i++)
    {
        if(sampleRate == g_CodecSupportSampleRate[i])
        {
            return CODEC_OK;
        }
    }

    return CODEC_ERR_UNSUPPORTED;
}

int8_t Codec_GetAdcDigitGain(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)  
    if(es8311_codec_get_adc_digit_gain(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_adc_digit_gain(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_CJC8910)
    if(cjc8910_codec_get_adc_digit_gain(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;
}

int8_t Codec_SetAdcDigitGain(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;

    if(len != 4)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(ADC_DIGITAL_GAIN, para, len) < 0)
        return CODEC_ERR;

#if defined(CODEC_ES8311)
    if(es8311_codec_set_adc_digit_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;

#elif defined(CODEC_PT8311)
    if(pt8311_codec_set_adc_digit_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;

#elif defined(CODEC_CJC8910)
    if(cjc8910_codec_set_adc_digit_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;
#endif
    g_audioCfg->AdcDigitGain = (para[0] ) + (para[1] << 8) + (para[2] << 16) + (para[3] <<24);
    return CODEC_OK;
}

int8_t Codec_GetDacDigitGain(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(es8311_codec_get_dac_digit_gain(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_dac_digit_gain(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_CJC8910)
    if(cjc8910_codec_get_dac_digit_gain(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;  
}

int8_t Codec_SetDacDigitGain(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;

    if(len != 4)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(DAC_DIGITAL_GAIN, para, len) < 0)
        return CODEC_ERR;
#if defined(CODEC_ES8311)
    if(es8311_codec_set_dac_digit_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_set_dac_digit_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;
#elif defined(CODEC_CJC8910)
    if(cjc8910_codec_set_dac_digit_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3] ) < 0)
        return CODEC_ERR;
#endif

    g_audioCfg->DacDigitGain = (para[0] ) + (para[1] << 8) + (para[2] << 16) + (para[3] <<24);
    return CODEC_OK;
}


int8_t Codec_GetAdcPgaGain(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(es8311_codec_get_adc_pga_gain(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_adc_pga_gain(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;
}

int8_t Codec_SetAdcPgaGain(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(len != 4)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(ADC_PGA, para, len) < 0)
        return CODEC_ERR;
    if(es8311_codec_set_adc_pga_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;
    g_audioCfg->AdcPgaGain = (para[0] ) + (para[1] << 8) + (para[2] << 16) + (para[3] <<24);
#elif defined(CODEC_PT8311)
    if(len != 4)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(ADC_PGA, para, len) < 0)
        return CODEC_ERR;
    if(pt8311_codec_set_adc_pga_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;
    g_audioCfg->AdcPgaGain = (para[0] ) + (para[1] << 8) + (para[2] << 16) + (para[3] <<24);
#endif
    return CODEC_OK;
}

int8_t Codec_GetMicGain(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(es8311_codec_get_mic_gain(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_mic_gain(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;
}

int8_t Codec_SetMicGain(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(len != 4)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(MIC_GAIN, para, len) < 0)
        return CODEC_ERR;
    if(es8311_codec_set_mic_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;
    g_audioCfg->MicGain = (para[0] ) + (para[1] << 8) + (para[2] << 16) + (para[3] <<24);
#elif defined(CODEC_PT8311)
    if(len != 4)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(MIC_GAIN, para, len) < 0)
        return CODEC_ERR;
    if(pt8311_codec_set_mic_gain((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;
    g_audioCfg->MicGain = (para[0] ) + (para[1] << 8) + (para[2] << 16) + (para[3] <<24);
#endif
    return CODEC_OK;
}

int8_t Codec_GetNrGate(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(es8311_codec_get_nr_gate(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_nr_gate(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;
}

int8_t Codec_SetNrGate(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(len != 4)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(NR_GATE, para, len) < 0)
        return CODEC_ERR;
    if(es8311_codec_set_nr_gate((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;
    g_audioCfg->NrGate = (para[0] ) + (para[1] << 8) + (para[2] << 16) + (para[3] <<24);
#elif defined(CODEC_PT8311)
    if(len != 4)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(NR_GATE, para, len) < 0)
        return CODEC_ERR;
    if(pt8311_codec_set_nr_gate((para[0] << 24) + (para[1] << 16) + (para[2] << 8) + para[3]) < 0)
        return CODEC_ERR;
    g_audioCfg->NrGate = (para[0] ) + (para[1] << 8) + (para[2] << 16) + (para[3] <<24);
#endif
    return CODEC_OK;
}

int8_t Codec_GetAdcEQ(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(es8311_codec_get_adc_eq(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_adc_eq(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;
}

int8_t Codec_SetAdcEQ(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(len != 46)  // 23 * 2
        return CODEC_ERR_PARAMETER;

    if(es8311_codec_set_adc_eq(para, len) < 0)
        return CODEC_ERR;
    memcpy(&g_audioCfg->AdcEQ, para, len);
#elif defined(CODEC_PT8311)
    if(len != 46)  // 23 * 2
        return CODEC_ERR_PARAMETER;

    if(pt8311_codec_set_adc_eq(para, len) < 0)
        return CODEC_ERR;
    memcpy(&g_audioCfg->AdcEQ, para, len);
#endif
    return CODEC_OK;
}

int8_t Codec_GetDacEQ(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(es8311_codec_get_dac_eq(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_dac_eq(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;
}

int8_t Codec_SetDacEQ(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(len != 32)
        return CODEC_ERR_PARAMETER;

    if(es8311_codec_set_dac_eq(para, len) < 0)
        return CODEC_ERR;
    memcpy(&g_audioCfg->DacEQ, para, len);
#elif defined(CODEC_PT8311)
    if(len != 32)
        return CODEC_ERR_PARAMETER;

    if(pt8311_codec_set_dac_eq(para, len) < 0)
        return CODEC_ERR;
    memcpy(&g_audioCfg->DacEQ, para, len);
#endif
    return CODEC_OK;
}

int8_t Codec_GetDrc(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(es8311_codec_get_drc(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_drc(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;
}

int8_t Codec_SetDrc(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(len != 10)
        return CODEC_ERR_PARAMETER;

    if(es8311_codec_set_drc(para, len) < 0)
        return CODEC_ERR;

    memcpy(&g_audioCfg->Drc, para, len);
#elif defined(CODEC_PT8311)
    if(len != 10)
        return CODEC_ERR_PARAMETER;

    if(pt8311_codec_set_drc(para, len) < 0)
        return CODEC_ERR;

    memcpy(&g_audioCfg->Drc, para, len);
#endif
    return CODEC_OK;
}

int8_t Codec_GetAlc(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)  
    if(es8311_codec_get_alc(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_alc(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;  
}

int8_t Codec_SetAlc(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(len != 12)
        return CODEC_ERR_PARAMETER;

    if(es8311_codec_set_alc(para, len) < 0)
        return CODEC_ERR;

    memcpy(&g_audioCfg->Alc, para, len);
#elif defined(CODEC_PT8311)
    if(len != 12)
        return CODEC_ERR_PARAMETER;

    if(pt8311_codec_set_alc(para, len) < 0)
        return CODEC_ERR;

    memcpy(&g_audioCfg->Alc, para, len);
#endif
    return CODEC_OK;
}

int8_t Codec_GetCodecID(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(es8311_codec_get_id(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_id(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_CJC8910)
    if(cjc8910_codec_get_id(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;
}

int8_t Codec_GetMcuID(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
    para[0] = 0;
    para[1] = 0;
    para[2] = 0x21;
    para[3] = 0x10;
    *len = 4;

    return CODEC_OK;
}

int8_t Codec_GetSideTone(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(es8311_codec_get_side_tone(para, len) < 0)
        return CODEC_ERR;
#elif defined(CODEC_PT8311)
    if(pt8311_codec_get_side_tone(para, len) < 0)
        return CODEC_ERR;
#endif
    return CODEC_OK;
}

int8_t Codec_SetSideTone(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
#if defined(CODEC_ES8311)
    if(len != 1)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(SIDE_TONE, para, len) < 0)
        return CODEC_ERR;
    if(es8311_codec_set_side_tone(para[0]) < 0)
        return CODEC_ERR;
    g_audioCfg->SideTone = para[0];
#elif defined(CODEC_PT8311)
    if(len != 1)
        return CODEC_ERR_PARAMETER;
    if(Codec_CheckParaRange(SIDE_TONE, para, len) < 0)
        return CODEC_ERR;
    if(pt8311_codec_set_side_tone(para[0]) < 0)
        return CODEC_ERR;
    g_audioCfg->SideTone = para[0];
#endif
    return CODEC_OK;
}

int8_t Codec_GetVolumeLevelPara(uint8_t *para, uint8_t *len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;

    memcpy(para, g_audioCfg->volumeLevel, sizeof(g_audioCfg->volumeLevel));
    *len = sizeof(g_audioCfg->volumeLevel);

    osPrintf("%d \r\n",*len);

    return CODEC_OK;
}

int8_t Codec_SetVolumeLevelPara(uint8_t *para, uint8_t len)
{
    if(para == OS_NULL)
        return CODEC_ERR_PARAMETER;
    
    if(len != AUDIO_VOLUME_MAX_LEVEL * 4)
        return CODEC_ERR_PARAMETER;

    memcpy(g_audioCfg->volumeLevel, para, sizeof(g_audioCfg->volumeLevel));

    for(uint8_t i = 0; i < AUDIO_VOLUME_MAX_LEVEL; i++)
        osPrintf("%d: %x \r\n", i, g_audioCfg->volumeLevel[i]);
   return CODEC_OK;
}

#include "nr_micro_shell.h"
static void Codec_DumpReg(char argc, char **argv)
{
    #if defined(CODEC_ES8311)
    for(int i = 0; i <= 0xFF; i++ )
        osPrintf("reg0x%x:0x%x \r\n", i, es8311_read_reg(i));
    #elif defined(CODEC_PT8311)
        pt8311_read_all();
    #elif defined(CODEC_CJC8910)
        cjc8910_dump_reg();
    #endif
}
#ifdef USE_AUDIO
NR_SHELL_CMD_EXPORT(Codec_DumpReg, Codec_DumpReg);
#endif