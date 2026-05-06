/**
 * @file cm_audio_local.h
 * @author your name (you@domain.com)
 * @brief CM 音频编解码器本地接口
 * @version 0.1
 * @date 2026-02-05
 *
 * Copyright (c) 2025 深圳市天工聚创科技有限公司. All rights reserved.
 *
 */

#ifndef CM_AUDIO_LOCAL_H
#define CM_AUDIO_LOCAL_H

#include <stdbool.h>
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_AUDIO
#include "enc_dec.h"

/* CM 音频编解码器类型 */
typedef enum {
    CM_AUDIO_ENC_DEC_TYPE_INVALID = 0,
    CM_AUDIO_ENC_DEC_TYPE_AMR_DEC,
    CM_AUDIO_ENC_DEC_TYPE_AMR_ENC,
    CM_AUDIO_ENC_DEC_TYPE_MP3_DEC,
    CM_AUDIO_ENC_DEC_TYPE_MP3_ENC,
    CM_AUDIO_ENC_DEC_TYPE_WAV_DEC,
    CM_AUDIO_ENC_DEC_TYPE_WAV_ENC,
    CM_AUDIO_ENC_DEC_TYPE_PCMAU_DEC,
    CM_AUDIO_ENC_DEC_TYPE_PCMAU_ENC,
    CM_AUDIO_ENC_DEC_TYPE_OPUS_DEC,
    CM_AUDIO_ENC_DEC_TYPE_OPUS_ENC,
} cm_audio_enc_dec_type_e;

/**
 * @brief 注册 CM 音频编解码器
 */
void CmAudio_EncDecRegister(EncDecOps *ops, cm_audio_enc_dec_type_e type);

/**
 * @brief 查找 CM 音频编解码器
 */
EncDecOps *CmAudio_FindEncDecOps(cm_audio_enc_dec_type_e type);

/**
 * @brief 注册 CM AMR 解码器（初始化时调用）
 */
int CmAmrDec_Register(void);

/**
 * @brief 注册 CM AMR 编码器（初始化时调用）
 */
int CmAmrEnc_Register(void);

/**
 * @brief 将 SDK AudioEncDecType 转换为 cm_audio_enc_dec_type_e
 */
cm_audio_enc_dec_type_e CmAudio_ConvertEncDecType(AudioEncDecType sdk_type);

#if defined(USE_CMAPI) && defined(USE_MOBVOI_DSP)
/**
 * @brief 获取 Mobvoi DSP 使能状态
 * @return true: 使能, false: 失能
 */
bool CmAudio_GetMobvoiEnabled(void);
#endif

#endif /* USE_AUDIO */

#ifdef __cplusplus
}
#endif

#endif /* CM_AUDIO_LOCAL_H */
