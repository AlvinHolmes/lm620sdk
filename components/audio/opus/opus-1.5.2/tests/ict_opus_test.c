
#include "stdlib.h"

#include "nr_micro_shell.h"
#include "opus_multistream.h"
#include "opus.h"

#include "os.h"
#include "drv_i2s.h"
#include "audio_codec.h"
#include "riscv_general.h"

static int8_t opus_audioInit(uint32_t frameSize, uint32_t sampleRate)
{
    I2S_BusCfg busCfg = {0};
    I2S_TimingCfg i2sTimingCfg = {0};

    i2sTimingCfg.chnl = I2S_LEFT_CHNL;
    i2sTimingCfg.dataBits = DATA_16_BITS;
    i2sTimingCfg.dataCycle = DATA_32_BITS;

    busCfg.workMode = I2S_PLAY_MODE | I2S_RECORD_MODE;
    busCfg.timing = TIMING_I2S;
    busCfg.frameSize = frameSize;
    busCfg.timingCfg = &i2sTimingCfg;
    busCfg.sampleRate = sampleRate;
    busCfg.fs = Codec_GetFs(busCfg.sampleRate);
   
    if(I2S_Initialize(&busCfg, NULL, NULL) < 0)   /** I2S Init   */
    {
        osPrintf("i2s init failed \r\n");
        return -1;
    }

    if(Codec_Initialize(sampleRate, DATA_16_BITS) < 0)           /** Codec Init */
        return -2;

    I2S_BusStart(I2S_PLAY_MODE | I2S_RECORD_MODE);
    Codec_Start(CODEC_PLAY | CODEC_RECORD);

    return 0;
}

static int8_t opus_audioRead(uint8_t *frame, uint32_t size)
{
    I2S_BusRead(frame, size, osWaitForever);
    return 0;
}

static int8_t opus_audioWrite(uint8_t *frame, uint32_t size)
{
    I2S_BusWrite(frame, size, osWaitForever);
    return 0;
}

static int8_t opus_audioClose(void)
{
    I2S_BusStop(I2S_PLAY_MODE | I2S_RECORD_MODE);
    Codec_Stop(CODEC_PLAY | CODEC_RECORD);
    Codec_UnInitialize();
    I2S_UnInitialize();
    return 0;
}

static void opus_encoder_config(OpusEncoder *enc)
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
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(16)) == OPUS_OK);    //采样点位数，16bits
    // OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_PREDICTION_DISABLED(1)) == OPUS_OK);
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_DTX(0)) == OPUS_OK);
    //20ms 一帧
    OS_ASSERT(opus_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_20_MS)) == OPUS_OK);
}

void _OpusTestTask(void *para)
{
    uint32_t sampleRate = 16000;

    int err = -1;

    OpusEncoder *enc = NULL;
    enc = opus_encoder_create(sampleRate, 1, OPUS_APPLICATION_AUDIO, &err); // 单声道，OPUS_APPLICATION_AUDIO 类型
    if(enc == NULL || err != OPUS_OK)
    {
        osPrintf("opus encoder create failed enc: %d, err: %d\r\n", (uint32_t)enc, err);
        return ;
    }

    OpusDecoder *dec = NULL;
    dec = opus_decoder_create(sampleRate, 1, &err);
    if(dec == NULL || err != OPUS_OK)
    {
        osPrintf("opus decoder create failed dec: %d, err: %d\r\n", (uint32_t)dec, err);
        return ;
    }

    opus_encoder_config(enc);

    uint32_t frameSize = ( sampleRate * 2 / 1000 ) * 20;
    opus_audioInit(frameSize, 16000);
    uint8_t *encodeBuf = osMalloc(1500);
    uint8_t *pcmBuf = osMalloc(frameSize);
    uint8_t *pcmBufDecode = osMalloc(frameSize);

    uint32_t sumEncodeUsTime = 0;
    uint32_t sumDecodeUsTime = 0;
    uint32_t loopTimes = 0;
    int32_t encode_size = 0;
    int32_t decode_size = 0;

    while(loopTimes < 100)
    {
        opus_audioRead(pcmBuf, frameSize);

        // 编码一帧
        unsigned long start_cycle =  (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
        encode_size = opus_encode(enc, (opus_int16 *)pcmBuf, frameSize / 2, encodeBuf, 1500);
        unsigned long end_cycle =  (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
        sumEncodeUsTime += ((end_cycle - start_cycle)/416);
        
        // 解码一帧
        start_cycle =  (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
        decode_size = opus_decode(dec, encodeBuf, encode_size, (opus_int16 *)pcmBufDecode, frameSize / 2, 0);
        end_cycle =  (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
        sumDecodeUsTime += ((end_cycle - start_cycle)/416);

        opus_audioWrite(pcmBuf, decode_size * 2);

        loopTimes ++;
    }

    osPrintf("loopTimes: %d, Encode Time: %d us, Decode Time: %d us \r\n", loopTimes, sumEncodeUsTime / loopTimes, sumDecodeUsTime / loopTimes);
    osPrintf("encode_size: %d, decode_size: %d \r\n", encode_size, decode_size);

    opus_audioClose();
    opus_encoder_destroy(enc);
    opus_decoder_destroy(dec);

    osFree(encodeBuf);
    osFree(pcmBuf);
    osFree(pcmBufDecode);

    osPrintf("opus test over \r\n");
}

#if 1
#include "opus_user.h"
#include "ogg_user.h"
void _OpusOggTestTask(void *para)
{
    uint32_t sampleRate = 16000;

    OpusCfg cfg = {0};
    cfg.sampleRate = sampleRate;
    cfg.chnls = 1;
    cfg.application = OPUS_APPLICATION_AUDIO;
    cfg.frametime = OPUS_FRAMESIZE_100_MS;
    cfg.sampleWidth = 16;
    void *OpusEncHandle = Opus_Init(OPUS_ENCODE_MODE, &cfg);

    uint32_t frameSize = ( sampleRate * 2 / 1000 ) * 100;
    opus_audioInit(frameSize, sampleRate);
    uint8_t *encodeBuf = osMalloc(1500);
    uint8_t *pcmBuf = osMalloc(frameSize);
    uint8_t *pcmBufDecode = osMalloc(frameSize);

    uint32_t loopTimes = 0;
    uint32_t encode_size = 0;
    // int32_t decode_size = 0;

    static OggHandle ogg_DecHandle = {0};
    static OggHandle ogg_EncHandle = {0};
    static OggAudioInfo info = {0};

    uint8_t *oggPackBuf = osMalloc(8*1024);
    OS_ASSERT(oggPackBuf);
    uint32_t oggPackOutSize = 0;

    uint8_t *oggUnPackBuf = osMalloc(8*1024);
    OS_ASSERT(oggUnPackBuf);
    uint32_t oggUnPackOutSize = 0;


    while(loopTimes < 5)
    {
        opus_audioRead(pcmBuf, frameSize);

        // 编码一帧
        encode_size = 1500;
        Opus_Encode((OpusEncoder *)OpusEncHandle, pcmBuf, frameSize, encodeBuf, &encode_size);

        info.audioData = encodeBuf;
        info.audioDataSize = encode_size;
        info.chnls = 1;
        info.sampleRate = sampleRate;
        info.sampleSize = frameSize / 2;
        
        OGG_Pack(&ogg_EncHandle, &info, oggPackBuf, &oggPackOutSize, false);
        osPrintf("oggPackOutSize: %d \r\n", oggPackOutSize);

        if(oggPackOutSize != 0)
        {
            oggUnPackOutSize = 8*1024;
            OGG_Unpack(&ogg_DecHandle, oggPackBuf, oggPackOutSize, oggUnPackBuf, &oggUnPackOutSize);
        }

        loopTimes ++;
    }

    opus_audioClose();
    Opus_DeInit(OpusEncHandle, OPUS_ENCODE_MODE);

    osFree(encodeBuf);
    osFree(pcmBuf);
    osFree(pcmBufDecode);
    osFree(oggPackBuf);
    osFree(oggUnPackBuf);
}
#endif

static void opus_test(char argc, char **argv)
{
    osThreadAttr_t attr = {"opus-test", osThreadDetached, NULL, 0U, NULL, 25*1024, 10, 0U, 0U};
    osThreadId_t *TaskHandle = osThreadNew((osThreadFunc_t)_OpusTestTask, NULL, &attr);
    OS_ASSERT(TaskHandle);
}
NR_SHELL_CMD_EXPORT(opus_test, opus_test);

static void ogg_test(char argc, char **argv)
{
    osThreadAttr_t attr = {"ogg-test", osThreadDetached, NULL, 0U, NULL, 25*1024, 10, 0U, 0U};
    osThreadId_t *TaskHandle = osThreadNew((osThreadFunc_t)_OpusOggTestTask, NULL, &attr);
    OS_ASSERT(TaskHandle);
}
NR_SHELL_CMD_EXPORT(ogg_test, ogg_test);
