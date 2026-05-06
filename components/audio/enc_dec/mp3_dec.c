#include <stdio.h>
#include "os.h"
#include "audio_local.h"
#include "enc_dec.h"
#ifdef USE_MP3
#include "mp3dec.h"

#define MP3_MAX_FRAME_SIZE                  (4096 + 1024)

typedef struct 
{
    void *state;
    void *pData;        /** 输入时文件，则指向文件句柄*/
    uint8_t *readData;
    uint16_t dataSize;
    uint16_t lastDataSize;
    uint32_t mp3HeadSize;
    uint16_t mp3frameSize;
    LocalAudioCfg *cfg;
}Mp3DecHandle;


static int8_t Mp3Dec_GetMp3FileInfo(uint8_t *data, uint32_t dataSize, uint8_t *dataBits, uint32_t *sampleRate, uint8_t *chnls, uint32_t *frameSize)
{
    unsigned int syncOffset = 0;
    int bytesLeft = 0;
    uint8_t *decodeDataPtr = NULL;
    MP3FrameInfo mp3frameInfo = {0};
    uint8_t *decodePcmDataBuf = NULL;
    HMP3Decoder mp3Decode = NULL;

    decodePcmDataBuf = osMalloc(MP3_MAX_FRAME_SIZE);
    OS_ASSERT(decodePcmDataBuf);

    decodeDataPtr = data;
    bytesLeft = dataSize;

    mp3Decode = MP3InitDecoder();
    syncOffset = MP3FindSyncWord(decodeDataPtr, bytesLeft);
    bytesLeft -= syncOffset;
    decodeDataPtr += syncOffset;

    MP3Decode(mp3Decode, &decodeDataPtr, &bytesLeft, (short *)decodePcmDataBuf, 0);
    MP3GetLastFrameInfo(mp3Decode, &mp3frameInfo);

    *dataBits = mp3frameInfo.bitsPerSample;
    *sampleRate = mp3frameInfo.samprate;
    *frameSize =  mp3frameInfo.outputSamps *mp3frameInfo.bitsPerSample/8;
    *chnls = mp3frameInfo.nChans;

    MP3FreeDecoder(mp3Decode);
    osFree(decodePcmDataBuf);

    return 0;
}

void *Mp3Dec_Init(void *para)
{
    Mp3DecHandle *mp3Dec = NULL;
    AudioInfo *cfg = (AudioInfo *)para;

    mp3Dec = (Mp3DecHandle *)osMalloc(sizeof(Mp3DecHandle));
    memset(mp3Dec, 0, sizeof(Mp3DecHandle));
    OS_ASSERT(mp3Dec);

    mp3Dec->cfg = cfg;

    if(cfg->audioType == MP3_FILE)
    {
        mp3Dec->pData = (void *)fopen(cfg->file, "r");
        if(!mp3Dec->pData)
        {
            osPrintf("open MP3 file failed \r\n");
            osFree(mp3Dec);
            return NULL;
        }

        mp3Dec->readData = osMalloc(MP3_MAX_FRAME_SIZE);
        OS_ASSERT(mp3Dec->readData);

        int32_t readSize = fread(mp3Dec->readData, 1, MP3_MAX_FRAME_SIZE, (FILE *)mp3Dec->pData);
        if(readSize < 0)
        {
            osPrintf("read mp3 file failed \r\n");
            fclose(mp3Dec->pData);
            osFree(mp3Dec->readData);
            mp3Dec->readData = NULL;
            osFree(mp3Dec);
            return NULL;
        }

        MP3GetID3TagSize(mp3Dec->readData, readSize, (unsigned int *)&mp3Dec->mp3HeadSize);
        
        fseek((FILE *)mp3Dec->pData, mp3Dec->mp3HeadSize, SEEK_SET);
        readSize = fread(mp3Dec->readData, 1, MP3_MAX_FRAME_SIZE, (FILE *)mp3Dec->pData);
        if(readSize < 0)
        {
            osPrintf("read mp3 file failed \r\n");
            fclose(mp3Dec->pData);
            osFree(mp3Dec->readData);
            mp3Dec->readData = NULL;
            osFree(mp3Dec);
            return NULL;
        }
        // osPrintf("mp3HeadSize %d\r\n", mp3Dec->mp3HeadSize);
        Mp3Dec_GetMp3FileInfo(&mp3Dec->readData[0], readSize, 
                            &cfg->dataBits,
                            &cfg->sampleRate,
                            &cfg->chnls,
                            &cfg->frameSize);

        fseek((FILE *)mp3Dec->pData, mp3Dec->mp3HeadSize, SEEK_SET);
        mp3Dec->dataSize = 0;
        mp3Dec->lastDataSize = MP3_MAX_FRAME_SIZE;
    }
    else if(cfg->audioType == MP3_FILE_DATA)
    {
        mp3Dec->readData = (uint8_t *)mp3Dec->cfg->data;
        MP3GetID3TagSize(mp3Dec->readData, cfg->size, (unsigned int *)&mp3Dec->mp3HeadSize);
        Mp3Dec_GetMp3FileInfo(&mp3Dec->readData[mp3Dec->mp3HeadSize], cfg->size - mp3Dec->mp3HeadSize, 
                            &cfg->dataBits,
                            &cfg->sampleRate,
                            &cfg->chnls,
                            &cfg->frameSize);

        mp3Dec->readData += mp3Dec->mp3HeadSize;
        mp3Dec->dataSize = mp3Dec->cfg->size - mp3Dec->mp3HeadSize;
    }
    else
        return NULL;

    mp3Dec->state = (void *)MP3InitDecoder();
    osPrintf("--%d, %d, %d, %d \r\n", cfg->dataBits, cfg->sampleRate, cfg->chnls, cfg->frameSize);
    return (void *)mp3Dec;
}

int32_t Mp3Dec_GetFrame(void *handle, uint8_t *out, uint32_t outSize)
{
    OS_ASSERT(handle);
    OS_ASSERT(out);
    Mp3DecHandle *mp3Dec = (Mp3DecHandle *)handle;

    if(mp3Dec->cfg->audioType == MP3_FILE)
    {
        int readSize = 0;

        if(mp3Dec->dataSize > 0 && mp3Dec->dataSize < MP3_MAX_FRAME_SIZE)
        {
            memmove((void *)mp3Dec->readData, (const void *)&mp3Dec->readData[mp3Dec->lastDataSize - mp3Dec->dataSize], mp3Dec->dataSize);
        }
    
        readSize = fread(&mp3Dec->readData[mp3Dec->dataSize], 1, MP3_MAX_FRAME_SIZE - mp3Dec->dataSize, (FILE *)mp3Dec->pData);
        if(readSize <= 0)
        {
            if(mp3Dec->cfg->isLoop)
            {
                fseek((FILE *)mp3Dec->pData, mp3Dec->mp3HeadSize, SEEK_SET);

                readSize = fread(&mp3Dec->readData[mp3Dec->dataSize], 1, MP3_MAX_FRAME_SIZE - mp3Dec->dataSize, (FILE *)mp3Dec->pData);
            }
        }

        if(readSize > 0)
            mp3Dec->dataSize += readSize;

        if(readSize <= 0 && mp3Dec->dataSize < mp3Dec->mp3frameSize )
            return -1;

        mp3Dec->lastDataSize = mp3Dec->dataSize;
    }
    else if(mp3Dec->cfg->audioType == MP3_FILE_DATA)
    {
        if(mp3Dec->cfg->isLoop)
        {
            mp3Dec->readData = (uint8_t *)mp3Dec->cfg->data + mp3Dec->mp3HeadSize;
            mp3Dec->dataSize = mp3Dec->cfg->size - mp3Dec->mp3HeadSize;
        }

        if(mp3Dec->dataSize < mp3Dec->mp3frameSize)
            return -1;
    }

    return mp3Dec->dataSize;
}

int8_t Mp3Dec_DecodeFrame(void *handle, uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t *outSize)
{
    OS_ASSERT(handle);
    OS_ASSERT(out);

    Mp3DecHandle *mp3Dec = (Mp3DecHandle *)handle;
    MP3FrameInfo mp3frameInfo = {0};
    int syncOffset = 0;

    syncOffset = MP3FindSyncWord(mp3Dec->readData, mp3Dec->dataSize);
    if(syncOffset < 0)
    {
        osPrintf("mp3 find sync failed \r\n");
        mp3Dec->dataSize = 0;
        return -1;
    }

    uint8_t *pMp3Data = NULL;
    uint32_t Mp3DatSize = 0;

    pMp3Data = mp3Dec->readData + syncOffset;
    Mp3DatSize = mp3Dec->dataSize - syncOffset;
 
    if(MP3Decode((HMP3Decoder)mp3Dec->state, &pMp3Data, (int *)&Mp3DatSize, (short *)out, 0) < 0)
    {
        mp3Dec->dataSize -= 2;
        osPrintf("mp3 decode failed %d\r\n", mp3Dec->dataSize);
        return -1;
    }

    mp3Dec->mp3frameSize = pMp3Data - mp3Dec->readData;
    mp3Dec->dataSize -= (pMp3Data - mp3Dec->readData);

    if(mp3Dec->cfg->audioType == MP3_FILE_DATA)
    {
        mp3Dec->readData = pMp3Data;
    }

    MP3GetLastFrameInfo((HMP3Decoder)mp3Dec->state, &mp3frameInfo); 

    *outSize = mp3frameInfo.outputSamps * mp3frameInfo.bitsPerSample/8;
    return 0;
}

void Mp3Dec_DeInit(void *handle)
{
    Mp3DecHandle *mp3Dec = (Mp3DecHandle *)handle;

    if(mp3Dec)
    {
        if(mp3Dec->readData && mp3Dec->cfg->audioType == MP3_FILE)
            osFree(mp3Dec->readData);
        
        if(mp3Dec->cfg->audioType == MP3_FILE)
            fclose((FILE *)mp3Dec->pData);

        MP3FreeDecoder(mp3Dec->state);

        osFree(mp3Dec);
    }
}

static EncDecOps mp3DecOps =
{
    .init = Mp3Dec_Init,
    .get = Mp3Dec_GetFrame,
    .proc = Mp3Dec_DecodeFrame,
    .deinit = Mp3Dec_DeInit,
};

int Mp3Dec_Register(void)
{
    Audio_EncDecRegister(&mp3DecOps, MP3_DEC);
    return 0;
}

INIT_MIDDLEWARE_EXPORT(Mp3Dec_Register, OS_INIT_SUBLEVEL_HIGH);
#endif
