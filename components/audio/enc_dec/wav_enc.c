#include <stdio.h>
#include "os.h"
#include "audio_local.h"
#include "enc_dec.h"

#include "wav.h"

typedef struct 
{
    LocalAudioCfg *cfg;
}WavEncHandle;

void *WavEnc_Init(void *para)
{
    WavEncHandle *wavEnc = NULL;
    AudioInfo *cfg = (AudioInfo *)para;

    wavEnc = (WavEncHandle *)osMalloc(sizeof(WavEncHandle));
    memset(wavEnc, 0, sizeof(WavEncHandle));
    OS_ASSERT(wavEnc);

    wavEnc->cfg = cfg;   

    WAV_CreateHeader(wavEnc->cfg->recordData, wavEnc->cfg->recordSize, wavEnc->cfg->sampleRate, wavEnc->cfg->dataBits, wavEnc->cfg->chnls);
    wavEnc->cfg->recordIndex = WAV_HEADER_SIZE;
    return (void *)wavEnc;

}

int8_t WavEnc_EncodeFrame(void *handle, uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t *outSize)
{
    memcpy(out, in, inSize);
    *outSize = inSize;
    return 0;
}

void WavEnc_DeInit(void *handle)
{
    WavEncHandle *wavEnc = (WavEncHandle *)handle;
    
    if(wavEnc)
    {
        osFree(wavEnc);
    }
}

static EncDecOps WavEncOps =
{
    .init = WavEnc_Init,
    .get = NULL,
    .proc = WavEnc_EncodeFrame,
    .deinit = WavEnc_DeInit,
};

int WavEnc_Register(void)
{
    Audio_EncDecRegister(&WavEncOps, WAV_ENC);
    return 0;
}

INIT_MIDDLEWARE_EXPORT(WavEnc_Register, OS_INIT_SUBLEVEL_HIGH);
