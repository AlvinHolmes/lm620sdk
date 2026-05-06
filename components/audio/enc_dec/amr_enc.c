#include <stdio.h>
#include "os.h"
#include "audio_local.h"
#include "enc_dec.h"

#ifdef USE_AMR
#include "interf_enc.h"
#include "enc_if.h"

typedef struct 
{
    void *state;
    bool isWb;
    LocalAudioCfg *cfg;
}AmrEncHandle;

void *AmrEnc_Init(void *para)
{
    AmrEncHandle *amrEnc = NULL;
    AudioInfo *cfg = (AudioInfo *)para;

    amrEnc = (AmrEncHandle *)osMalloc(sizeof(AmrEncHandle));
    memset(amrEnc, 0, sizeof(AmrEncHandle));
    OS_ASSERT(amrEnc);

    amrEnc->cfg = cfg;   

    amrEnc->cfg->chnls = 1;
    amrEnc->cfg->dataBits = AMR_BITS_PER_SAMPLE;
    amrEnc->isWb = false;
    if(amrEnc->cfg->sampleRate == AMR_WB_SAMPLERATE)
    {
        amrEnc->isWb = true;
        amrEnc->cfg->frameSize = AMR_WB_FRAME_SIZE;
        amrEnc->state = E_IF_init(0);
        strcpy((char *)amrEnc->cfg->recordData, "#!AMR-WB\n");
        amrEnc->cfg->recordIndex = AMR_WB_FILE_TAG_LEN;
    }
    else
    {
        amrEnc->cfg->frameSize = AMR_NB_FRAME_SIZE;
        amrEnc->state = Encoder_Interface_init(0);
        strcpy((char *)amrEnc->cfg->recordData, "#!AMR\n");
        amrEnc->cfg->recordIndex = AMR_NB_FILE_TAG_LEN;
    }

    return (void *)amrEnc;

}

int8_t AmrEnc_EncodeFrame(void *handle, uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t *outSize)
{
    AmrEncHandle *amrEnc = (AmrEncHandle *)handle;
    int32_t size = 0;
    if(amrEnc->isWb)
    {
        size = E_IF_encode(amrEnc->state, MR122, (short *)in, out, 5);
    }
    else
    {
        size = Encoder_Interface_Encode(amrEnc->state, MR122, (short *)in, out, 2);
    }

    if(size <= 0)
    {
        return -1;
    }

    *outSize = size;
    return 0;
}

void AmrEnc_DeInit(void *handle)
{
    AmrEncHandle *amrEnc = (AmrEncHandle *)handle;
    
    if(amrEnc)
    {
        if(amrEnc->isWb)
            E_IF_exit(amrEnc->state);
        else
            Encoder_Interface_exit(amrEnc->state);

        osFree(amrEnc);
    }
}

static EncDecOps amrEncOps =
{
    .init = AmrEnc_Init,
    .get = NULL,
    .proc = AmrEnc_EncodeFrame,
    .deinit = AmrEnc_DeInit,
};

int AmrEnc_Register(void)
{
    Audio_EncDecRegister(&amrEncOps, AMR_ENC);
    return 0;
}

INIT_MIDDLEWARE_EXPORT(AmrEnc_Register, OS_INIT_SUBLEVEL_HIGH);
#endif