#ifndef __OGG_USER_H__
#define __OGG_USER_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "ogg.h"

typedef struct 
{
    ogg_sync_state sync_state;
    ogg_stream_state stream_state;
    ogg_page page;
    ogg_packet packet;
    bool initFlag;
    uint32_t pageCnt;
    uint32_t packetCnt;

    uint32_t sampleRate;
    uint8_t dataBits;
    uint8_t chnls;
}OggHandle;

typedef struct 
{
    uint32_t sampleRate;
    uint32_t sampleSize;
    uint8_t chnls;
    uint8_t *audioData;
    uint32_t audioDataSize;
}OggAudioInfo;

int8_t OGG_Unpack(OggHandle *ogg_handle, uint8_t *inData, uint32_t inSize, uint8_t *outData, uint32_t *outSize);

int8_t OGG_UnpackEnd(OggHandle *ogg_handle);

int8_t OGG_Pack(OggHandle *ogg_handle, OggAudioInfo *info, uint8_t *outData, uint32_t *outSize, bool isEnd);

#endif
