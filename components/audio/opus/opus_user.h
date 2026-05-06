
#ifndef __OPUS_USER_H__
#define __OPUS_USER_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "opus_multistream.h"
#include "opus.h"

typedef enum
{
    OPUS_DECODE_MODE = 1,
    OPUS_ENCODE_MODE = 2
}OpusMode;

typedef struct 
{
    uint32_t sampleRate;
    int32_t application;
    uint8_t chnls;
    uint8_t sampleWidth;
    uint16_t frametime;
}OpusCfg;

void* Opus_Init(OpusMode mode, OpusCfg *cfg);
void Opus_DeInit(void *handle, OpusMode mode);
int8_t Opus_Decode(OpusDecoder *decHandle, uint8_t *opusFrame, uint32_t opusFrameSize, uint8_t *pcmFrame, uint32_t *pcmFrameSize);
int8_t Opus_Encode(OpusEncoder *encHandle, uint8_t *pcmFrame, uint32_t pcmFrameSize, uint8_t *opusFrame, uint32_t *opusFrameSize);
#endif
