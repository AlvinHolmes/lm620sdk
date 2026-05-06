#include <stdio.h>
#include "os.h"
#include "audio_local.h"
#include "enc_dec.h"

#ifdef USE_AMR
#include "interf_dec.h"
#include "dec_if.h"


typedef struct 
{
    void *state;
    bool isWb;
    void *pData;        /** 输入时文件，则指向文件句柄*/
    // uint8_t *oneAmrFrame;
    uint16_t amrOneFrameSize;
    LocalAudioCfg *cfg;
}AmrDecHandle;

static uint32_t g_amrNbBitsRate[] = {4750, 5150, 5900, 6700, 7400, 7950, 10200, 12200};
static uint32_t g_amrWbBitsRate[] = {6600, 8850, 12650, 14250, 15850, 18250, 19850, 23050, 23850};

static int8_t AmrDec_GetInfo(AmrDecHandle *handle, uint8_t *pData, uint32_t size)
{
    if(memcmp(pData, "#!AMR-WB\n", AMR_WB_FILE_TAG_LEN) == 0)
        handle->isWb = true;
    else if(memcmp(pData, "#!AMR\n", AMR_NB_FILE_TAG_LEN) == 0)
        handle->isWb = false;  
    else
        return  -1;

    
    if(handle->isWb)
    {
        handle->cfg->frameSize = AMR_WB_FRAME_SIZE;
        handle->cfg->sampleRate = AMR_WB_SAMPLERATE;
    } 
    else
    {
        handle->cfg->frameSize = AMR_NB_FRAME_SIZE;
        handle->cfg->sampleRate = AMR_NB_SAMPLERATE;
    }  

    handle->cfg->dataBits = AMR_BITS_PER_SAMPLE;
    handle->cfg->chnls = 1;

    uint8_t ft = 0;
    
    if(handle->isWb)
        ft = (pData[AMR_WB_FILE_TAG_LEN] >> 3) & 0x0f;
    else
        ft = (pData[AMR_NB_FILE_TAG_LEN] >> 3) & 0x0f;

    uint32_t *amrBitsRate = NULL;

    if(handle->cfg->sampleRate == AMR_WB_SAMPLERATE)
    {
        amrBitsRate = g_amrWbBitsRate;

        if(ft >= sizeof(g_amrWbBitsRate)/sizeof(uint32_t))
            ft = sizeof(g_amrWbBitsRate)/sizeof(uint32_t) - 1;

    }
    else
    {
        amrBitsRate = g_amrNbBitsRate;

        if(ft >= sizeof(g_amrNbBitsRate)/sizeof(uint32_t))
            ft = sizeof(g_amrNbBitsRate)/sizeof(uint32_t) - 1;
    }

    uint32_t tmpSize = amrBitsRate[ft] * 10 / 400;
    if((tmpSize % 10) != 0)
    {
        handle->amrOneFrameSize = (tmpSize / 10) + 1;
    }
    else
    {
        handle->amrOneFrameSize = (tmpSize / 10);
    }
    handle->amrOneFrameSize += 1;

    return 0;
}

void *AmrDec_Init(void *para)
{
    uint8_t *readData = NULL;
    AmrDecHandle *amrDec = NULL;
    AudioInfo *cfg = (AudioInfo *)para;

    amrDec = (AmrDecHandle *)osMalloc(sizeof(AmrDecHandle));
    memset(amrDec, 0, sizeof(AmrDecHandle));
    OS_ASSERT(amrDec);

    amrDec->cfg = cfg;

    if(cfg->audioType == AMR_FILE)
    {
        amrDec->pData = (void *)fopen(cfg->file, "r");
        if(!amrDec->pData)
        {
            osPrintf("open amr file failed \r\n");
            osFree(amrDec);
            return NULL;
        }

        readData = osMalloc(AMR_WB_FILE_TAG_LEN + 2);
        OS_ASSERT(readData);

        if(fread(readData, 1, AMR_WB_FILE_TAG_LEN + 1, amrDec->pData) < 0)
        {
            osPrintf("read amr file failed \r\n");
            fclose(amrDec->pData);
            osFree(readData);
            osFree(amrDec);
            return NULL;
        }

        if(AmrDec_GetInfo(amrDec, readData, AMR_WB_FILE_TAG_LEN + 2) != 0)
        {
            osPrintf("amr dec get info failed\r\n");
            fclose(amrDec->pData);
            osFree(readData);
            osFree(amrDec);
            return NULL;
        }

        if(amrDec->isWb)
            fseek((FILE *)amrDec->pData, AMR_WB_FILE_TAG_LEN, SEEK_SET);
        else
            fseek((FILE *)amrDec->pData, AMR_NB_FILE_TAG_LEN, SEEK_SET);
    }
    else if(cfg->audioType == AMR_FILE_DATA)
    {
        amrDec->pData = (void *)amrDec->cfg->data;
        if(AmrDec_GetInfo(amrDec, amrDec->pData, AMR_WB_FILE_TAG_LEN) != 0)
        {
            osFree(amrDec);
            return NULL;
        }

        if(amrDec->isWb)
            amrDec->pData += AMR_WB_FILE_TAG_LEN;
        else
            amrDec->pData += AMR_NB_FILE_TAG_LEN;
    }
    else
        return NULL;

    if(amrDec->isWb)
        amrDec->state = D_IF_init();
    else 
        amrDec->state = Decoder_Interface_init();

    if(readData) 
        osFree(readData);

    return (void *)amrDec;
}

int32_t AmrDec_GetFrame(void *handle, uint8_t *out, uint32_t outSize)
{
    OS_ASSERT(handle);
    OS_ASSERT(out);

    AmrDecHandle *amrDec = (AmrDecHandle *)handle;

    OS_ASSERT(outSize >= amrDec->amrOneFrameSize);

    if(amrDec->cfg->audioType == AMR_FILE)
    {
        int readSize = fread(out, 1, amrDec->amrOneFrameSize, (FILE *)amrDec->pData);
        if(readSize <= 0)
        {
            if(amrDec->cfg->isLoop)
            {
                if(amrDec->isWb)
                    fseek((FILE *)amrDec->pData, AMR_WB_FILE_TAG_LEN, SEEK_SET);
                else
                    fseek((FILE *)amrDec->pData, AMR_NB_FILE_TAG_LEN, SEEK_SET);

                readSize = fread(out, 1, amrDec->amrOneFrameSize, (FILE *)amrDec->pData);
                if(readSize < 0)
                {
                    return -1;
                }
            }
            else
            {
                return -1;
            }
        }
    }
    else if(amrDec->cfg->audioType == AMR_FILE_DATA)
    {
        memcpy(out, (uint8_t *)amrDec->pData, amrDec->amrOneFrameSize);

        if(amrDec->cfg->audioType == AMR_FILE_DATA)
        {
            if((uint8_t *)amrDec->pData - amrDec->cfg->data + amrDec->amrOneFrameSize >= amrDec->cfg->size)
            {
                if(amrDec->cfg->isLoop)
                {
                    if(amrDec->isWb)
                        amrDec->pData = (void *)&amrDec->cfg->data[AMR_WB_FILE_TAG_LEN];
                    else
                        amrDec->pData = (void *)&amrDec->cfg->data[AMR_NB_FILE_TAG_LEN];
                }
                else
                {
                    return -1;
                }
            } 
            else
                amrDec->pData = (void *)((uint8_t *)amrDec->pData + amrDec->amrOneFrameSize);
        }
    }

    return amrDec->amrOneFrameSize;
}

int8_t AmrDec_DecodeFrame(void *handle, uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t *outSize)
{
    OS_ASSERT(handle);
    OS_ASSERT(out);
    AmrDecHandle *amrDec = (AmrDecHandle *)handle;
    OS_ASSERT(amrDec->state);

    if(amrDec->isWb)
    {
        D_IF_decode(amrDec->state, in, (short *)out, 0);
        *outSize = AMR_WB_FRAME_SIZE;
    } 
    else
    {
        Decoder_Interface_Decode(amrDec->state, in, (short *)out, 2);
        *outSize = AMR_NB_FRAME_SIZE;
    }

    return 0;
}

void AmrDec_DeInit(void *handle)
{
    if(handle)
    {
        AmrDecHandle *amrDec = (AmrDecHandle *)handle;

        if(amrDec->isWb)
            D_IF_exit(amrDec->state);
        else
            Decoder_Interface_exit(amrDec->state);
        
        if(amrDec->cfg->audioType == AMR_FILE)
            fclose((FILE *)amrDec->pData);

        osFree(amrDec);
    }
}

static EncDecOps amrDecOps =
{
    .init = AmrDec_Init,
    .get = AmrDec_GetFrame,
    .proc = AmrDec_DecodeFrame,
    .deinit = AmrDec_DeInit,
};

int AmrDec_Register(void)
{
    Audio_EncDecRegister(&amrDecOps, AMR_DEC);
    return 0;
}

INIT_MIDDLEWARE_EXPORT(AmrDec_Register, OS_INIT_SUBLEVEL_HIGH);
#endif