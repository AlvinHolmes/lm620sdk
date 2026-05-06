#include <stdio.h>
#include "os.h"
#include "audio_local.h"
#include "enc_dec.h"
#include "wav.h"

typedef struct 
{
    void *pData;        /** 输入时文件，则指向文件句柄*/
    uint32_t dataChunkOffset;
    LocalAudioCfg *cfg;
}WavDecHandle;


void *WavDec_Init(void *para)
{
    WavDecHandle *wavDec = NULL;
    AudioInfo *cfg = (AudioInfo *)para;
    uint8_t *readData = NULL;

    wavDec = (WavDecHandle *)osMalloc(sizeof(WavDecHandle));
    memset(wavDec, 0, sizeof(WavDecHandle));
    OS_ASSERT(wavDec);

    wavDec->cfg = cfg;

    if(cfg->audioType == WAV_FILE)
    {
        wavDec->pData = (void *)fopen(cfg->file, "r");
        if(!wavDec->pData)
        {
            osPrintf("open WAV file failed \r\n");
            osFree(wavDec);
            return NULL;
        }

        readData = osMalloc(AUDIO_DEFAULT_FRAME_SIZE);
        OS_ASSERT(readData);

        int32_t readSize = fread(readData, 1, AUDIO_DEFAULT_FRAME_SIZE, (FILE *)wavDec->pData);
        if( readSize < 0)
        {
            osPrintf("read wav file failed \r\n");
            fclose(wavDec->pData);
            osFree(readData);
            readData = NULL;
            osFree(wavDec);
            return NULL;
        }

        if(WAV_Check(readData, readSize) < 0)
        {
            fclose(wavDec->pData);
            osFree(readData);
            readData = NULL;
            osFree(wavDec);
            return NULL;
        }
            
        WAV_GetBitsOfPerSample(readData, readSize, &wavDec->cfg->dataBits);
        WAV_GetSampleRate(readData, readSize, &wavDec->cfg->sampleRate);
        WAV_GetChnls(readData, readSize, &wavDec->cfg->chnls);
        wavDec->cfg->frameSize = AUDIO_DEFAULT_FRAME_SIZE;


        wavDec->dataChunkOffset = WAV_FindDataChunk(readData, readSize);
        if(wavDec->dataChunkOffset < 0)
        {
            fclose(wavDec->pData);
            osFree(readData);
            readData = NULL;
            osFree(wavDec);
            return NULL;  
        }

        fseek((FILE *)wavDec->pData, wavDec->dataChunkOffset, SEEK_SET);
    }
    else if(cfg->audioType == WAV_FILE_DATA)
    {
        wavDec->pData = (void *)wavDec->cfg->data;

        WAV_GetBitsOfPerSample((uint8_t *)wavDec->pData, AUDIO_DEFAULT_FRAME_SIZE, &wavDec->cfg->dataBits);
        WAV_GetSampleRate((uint8_t *)wavDec->pData, AUDIO_DEFAULT_FRAME_SIZE, &wavDec->cfg->sampleRate);
        WAV_GetChnls((uint8_t *)wavDec->pData, AUDIO_DEFAULT_FRAME_SIZE, &wavDec->cfg->chnls);
        wavDec->cfg->frameSize = AUDIO_DEFAULT_FRAME_SIZE;

        wavDec->dataChunkOffset = WAV_FindDataChunk((uint8_t *)wavDec->pData, AUDIO_DEFAULT_FRAME_SIZE);
        if(wavDec->dataChunkOffset < 0)
        {
            return NULL;
        }
        
        wavDec->pData += wavDec->dataChunkOffset;
    }
    else
        return NULL;

    if(readData)
        osFree(readData);

    return (void *)wavDec;
}

int32_t WavDec_GetFrame(void *handle, uint8_t *out, uint32_t outSize)
{
    OS_ASSERT(handle);
    OS_ASSERT(out);
    WavDecHandle *wavDec = (WavDecHandle *)handle;
    int readSize = 0;

    if(wavDec->cfg->audioType == WAV_FILE)
    {
        readSize = fread(out, 1, outSize, (FILE *)wavDec->pData);
        if(readSize <= 0)
        {
            if(wavDec->cfg->isLoop)
            {
                fseek((FILE *)wavDec->pData, wavDec->dataChunkOffset, SEEK_SET);

                readSize = fread(out, 1, outSize, (FILE *)wavDec->pData);
            }
        }

        if(readSize <= 0 )
            return -1;
    }
    else if(wavDec->cfg->audioType == WAV_FILE_DATA)
    {
        if((uint8_t *)wavDec->pData >= (uint8_t *)wavDec->cfg->data + wavDec->cfg->size)
        {
            if(wavDec->cfg->isLoop)
            {
                wavDec->pData = (void *)(wavDec->dataChunkOffset + (uint8_t *)wavDec->cfg->data);
                readSize = outSize;
            }
            else
                return -1;
        }
        else
        {
            if((uint8_t *)wavDec->pData + outSize > (uint8_t *)wavDec->cfg->data + wavDec->cfg->size)
            {
                memcpy(out, (uint8_t *)wavDec->pData, (uint8_t *)wavDec->cfg->data + wavDec->cfg->size - (uint8_t *)wavDec->pData);
                wavDec->pData =(void *)((uint8_t *)wavDec->cfg->data + wavDec->cfg->size);
                readSize = (uint8_t *)wavDec->cfg->data + wavDec->cfg->size - (uint8_t *)wavDec->pData;
            }
            else
            {
                memcpy(out, (uint8_t *)wavDec->pData, outSize);
                wavDec->pData = (void *)((uint8_t *)wavDec->pData + outSize);
                readSize = outSize;
            }
        }
    }

    return readSize;;
}

int8_t WavDec_DecodeFrame(void *handle, uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t *outSize)
{
    OS_ASSERT(handle);
    OS_ASSERT(out);

    memcpy(out, in, inSize);
    *outSize = inSize;
    return 0;
}

void WavDec_DeInit(void *handle)
{
    WavDecHandle *wavDec = (WavDecHandle *)handle;

    if(wavDec)
    {
        if(wavDec->cfg->audioType == WAV_FILE)
            fclose((FILE *)wavDec->pData);
        osFree(wavDec);
    }
}


static EncDecOps wavDecOps =
{
    .init = WavDec_Init,
    .get = WavDec_GetFrame,
    .proc = WavDec_DecodeFrame,
    .deinit = WavDec_DeInit,
};

int WavDec_Register(void)
{
    Audio_EncDecRegister(&wavDecOps, WAV_DEC);
    return 0;
}

INIT_MIDDLEWARE_EXPORT(WavDec_Register, OS_INIT_SUBLEVEL_HIGH);