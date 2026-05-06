
#include "os.h"
#include "opus_user.h"


static void opus_encoder_config(OpusEncoder *enc, uint8_t sampleWidth, uint16_t frameDuration)
{
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_BITRATE(OPUS_AUTO)) == OPUS_OK);   // 比特率设置，自动
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(1)) == OPUS_OK);    // 声道设置， 单声道
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_VBR(0)) == OPUS_OK);     // 可变比特率设置，关闭
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(0)) == OPUS_OK);    // 可变比特率相关，关闭
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(0)) == OPUS_OK);    // 音频质量相关，0-10， 越高质量越高，运算越复杂
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND)) == OPUS_OK); // 最大带宽设置，
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE)) == OPUS_OK);    // 同上
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(0)) == OPUS_OK);   //带内前向纠错，关闭
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(2)) == OPUS_OK);   //丢包百分比，编码器会根据这个值优化PLC策略,
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(sampleWidth)) == OPUS_OK);    //采样点位数，16bits
    // OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_PREDICTION_DISABLED(1)) == OPUS_OK);
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_DTX(0)) == OPUS_OK);
    //20ms 一帧
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(frameDuration)) == OPUS_OK);
}

void* Opus_Init(OpusMode mode, OpusCfg *cfg)
{
    void *handle = NULL;
    int err = 0;
    if(mode == OPUS_DECODE_MODE)
    {
        handle = (void *)opus_decoder_create(cfg->sampleRate, cfg->chnls, &err);
        if(handle == NULL || err != OPUS_OK)
        {
            osPrintf("opus decoder create failed dec: %d, err: %d\r\n", (uint32_t)handle, err);
            return NULL;
        }
    }
    else if(mode == OPUS_ENCODE_MODE)
    {
        handle = (void *)opus_encoder_create(cfg->sampleRate, cfg->chnls, cfg->application, &err); 
        if(handle == NULL || err != OPUS_OK)
        {
            osPrintf("opus encoder create failed enc: %d, err: %d\r\n", (uint32_t)handle, err);
            return NULL;
        }

        opus_encoder_config((OpusEncoder *)handle, cfg->sampleWidth, cfg->frametime);
    }

    return handle;
}

void Opus_DeInit(void *handle, OpusMode mode)
{
    if(mode == OPUS_DECODE_MODE)
    {
        OpusDecoder *dec = (OpusDecoder *)handle;
        opus_decoder_destroy(dec);

    }
    else if(mode == OPUS_ENCODE_MODE)
    {
        OpusEncoder *enc = (OpusEncoder *)handle;
        opus_encoder_destroy(enc);
    }
}

int8_t Opus_Decode(OpusDecoder *decHandle, uint8_t *opusFrame, uint32_t opusFrameSize, uint8_t *pcmFrame, uint32_t *pcmFrameSize)
{
    int32_t decodeSize = opus_decode(decHandle, opusFrame, opusFrameSize, (opus_int16 *)pcmFrame, *pcmFrameSize, 0);
    if(decodeSize > 0)
    {
        *pcmFrameSize = decodeSize;
        return 0;
    }
    else
    {
        return -1;
    }
}

int8_t Opus_Encode(OpusEncoder *encHandle, uint8_t *pcmFrame, uint32_t pcmFrameSize, uint8_t *opusFrame, uint32_t *opusFrameSize)
{
    int32_t encodeSize = opus_encode(encHandle, (opus_int16 *)pcmFrame, pcmFrameSize / 2, opusFrame, *opusFrameSize);
    if(encodeSize > 0)
    {
        *opusFrameSize = encodeSize;
        // osPrintf("encode size: %d \r\n", encodeSize);
        return 0;
    }
    else
    {
        return -1;
    }
}