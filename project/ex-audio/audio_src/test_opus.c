/**
 * @file test_opus.c
 * @author bosco (bosco@pthyidh.com)
 * @brief 
 * @version 0.1
 * @date 2025-11-20
 * 
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "stdlib.h"
#include <os.h>
#include "vfs.h"

#include "nr_micro_shell.h"
#include "opus_multistream.h"
#include "opus.h"

#include "os.h"
#include "riscv_general.h"

#define INPUT_FILE_PATH     "/usr/test.pcm"

static int opus_audioRead(uint8_t *frame, uint32_t size)
{
    VFS_File *fp = NULL;
    fp = VFS_OpenFile(INPUT_FILE_PATH, "rb");
    if (OS_NULL == fp) {
        return -1;
    }
    // 读取原始PCM数据
    size_t bytes_read = VFS_ReadFile(frame, 1, size, fp);
    if (bytes_read != size) {
        VFS_CloseFile(fp);
        return NULL;
    }
    VFS_CloseFile(fp);
    return 0;
}

// NOTE: 不定义这个编译报错
#define __FILE__ "test_opus"

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

void opus_test_task(void *parameter)
{
    uint32_t sampleRate = 8000;

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
    uint8_t *encodeBuf = osMalloc(1500);
    uint8_t *pcmBuf = osMalloc(frameSize);
    uint8_t *pcmBufDecode = osMalloc(frameSize);

    uint32_t sumEncodeUsTime = 0;
    uint32_t sumDecodeUsTime = 0;
    uint32_t loopTimes = 0;
    int32_t encode_size = 0;
    int32_t decode_size = 0;


    opus_audioRead(pcmBuf, frameSize);

    while(loopTimes < 100)
    {
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

        loopTimes ++;
    }

    osPrintf("loopTimes: %d, Encode Time: %d us, Decode Time: %d us \r\n", loopTimes, sumEncodeUsTime / loopTimes, sumDecodeUsTime / loopTimes);
    osPrintf("encode_size: %d, decode_size: %d \r\n", encode_size, decode_size);

    opus_encoder_destroy(enc);
    opus_decoder_destroy(dec);

    osFree(encodeBuf);
    osFree(pcmBuf);
    osFree(pcmBufDecode);

    osPrintf("opus test over \r\n");
}

static void opus_test1(char argc, char **argv)
{
    osThreadAttr_t attr = {"opus-test1", osThreadDetached, NULL, 0U, NULL, 25*1024, 10, 0U, 0U};
    osThreadId_t *TaskHandle = osThreadNew((osThreadFunc_t)opus_test_task, NULL, &attr);
    OS_ASSERT(TaskHandle);
}
NR_SHELL_CMD_EXPORT(opus_test1, opus_test1);

void test_audio_play(void)
{
    osPrintf("[%s]\r\n", __FUNCTION__);
}
