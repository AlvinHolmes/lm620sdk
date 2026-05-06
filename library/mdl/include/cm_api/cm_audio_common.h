/**
 * @file        cm_audio_common.h
 * @brief       Audio 通用接口
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By WangPeng
 * @date        2021/4/12
 *
 * @defgroup audio_common
 * @ingroup audio_common
 * @{
 */

#ifndef __OPENCPU_AUDIO_COMMON_H__
#define __OPENCPU_AUDIO_COMMON_H__
/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/** 音频播放格式支持
 *
 * @note 实际支持情况：
 *       - PCM: 直接透传，不解码
 *       - AMRNB: 支持，通过 AMR 解码器解码为 8kHz PCM
 *       - AMRWB: 支持，通过 AMR 解码器解码为 16kHz PCM
 *       - WAV/MP3/FLAC: 暂不支持，返回错误
 */
typedef enum
{
    CM_AUDIO_PLAY_FORMAT_PCM = 1,               /*!< PCM格式 */
    // CM_AUDIO_PLAY_FORMAT_WAV,                   /*!< WAV格式 */
    // CM_AUDIO_PLAY_FORMAT_MP3 = 3,               /*!< MP3格式 */
    CM_AUDIO_PLAY_FORMAT_AMRNB = 4,             /*!< AMR-NB格式 */
    // CM_AUDIO_PLAY_FORMAT_FLAC,                  /*!< FLAC格式 */
    CM_AUDIO_PLAY_FORMAT_AMRWB = 6,             /*!< AMR-WB格式 */
#if 0
    CM_AUDIO_PLAY_FORMAT_FR,                    /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_HR,                    /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_EFR,                   /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_AAC,                   /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_MID,                   /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_QTY,                   /*!< 预留，暂不支持 */
#endif
} cm_audio_play_format_e;

/** 音频录音格式支持，本平台需要外置codec并在完成外置codec驱动开发后可使用 */
typedef enum
{
    CM_AUDIO_RECORD_FORMAT_PCM = 1,             /*!< PCM格式 */
#if 0
    CM_AUDIO_RECORD_FORMAT_WAVPCM,              /*!< WAV格式 */
    CM_AUDIO_RECORD_FORMAT_MP3,                 /*!< MP3格式 */
#endif
    CM_AUDIO_RECORD_FORMAT_AMRNB_475 = 4,       /*!< 4.75 kbps AMR格式 */
    CM_AUDIO_RECORD_FORMAT_AMRNB_515,           /*!< 5.15 kbps AMR格式 */
    CM_AUDIO_RECORD_FORMAT_AMRNB_590,           /*!< 5.90 kbps AMR格式 */
    CM_AUDIO_RECORD_FORMAT_AMRNB_670,           /*!< 6.70 kbps AMR格式 */
    CM_AUDIO_RECORD_FORMAT_AMRNB_740,           /*!< 7.40 kbps AMR格式 */
    CM_AUDIO_RECORD_FORMAT_AMRNB_795,           /*!< 7.95 kbps AMR格式 */
    CM_AUDIO_RECORD_FORMAT_AMRNB_1020,          /*!< 10.20 kbps AMR格式 */
    CM_AUDIO_RECORD_FORMAT_AMRNB_1220,          /*!< 12.20 kbps AMR格式 */

    CM_AUDIO_RECORD_FORMAT_AMRWB_660,           /*!< 6.60 kbps AWB格式 */
    CM_AUDIO_RECORD_FORMAT_AMRWB_885,           /*!< 8.85 kbps AWB格式 */
    CM_AUDIO_RECORD_FORMAT_AMRWB_1265,          /*!< 12.65 kbps AWB格式 */
    CM_AUDIO_RECORD_FORMAT_AMRWB_1425,          /*!< 14.25 kbps AWB格式 */
    CM_AUDIO_RECORD_FORMAT_AMRWB_1585,          /*!< 15.85 kbps AWB格式 */
    CM_AUDIO_RECORD_FORMAT_AMRWB_1825,          /*!< 18.25 kbps AWB格式 */
    CM_AUDIO_RECORD_FORMAT_AMRWB_1985,          /*!< 19.85 kbps AWB格式 */
    CM_AUDIO_RECORD_FORMAT_AMRWB_2305,          /*!< 23.05 kbps AWB格式 */
    CM_AUDIO_RECORD_FORMAT_AMRWB_2385,          /*!< 23.85 kbps AWB格式 */
} cm_audio_record_format_e;

/** PCM音频采样格式
 *
 * @note 实际支持情况：
 *       - 仅 16BIT 被实现和使用
 *       - 8BIT/24BIT/32BIT 未实现
 */
typedef enum
{
    CM_AUDIO_SAMPLE_FORMAT_8BIT = 8,            /*!< 8位，暂不开放，请使用CM_AUDIO_SAMPLE_FORMAT_16BIT */
    CM_AUDIO_SAMPLE_FORMAT_16BIT = 16,               /*!< 16位, 小端 */
    CM_AUDIO_SAMPLE_FORMAT_24BIT = 24,               /*!< 24位, 本平台不支持 */
    CM_AUDIO_SAMPLE_FORMAT_32BIT = 32,               /*!< 32位, 暂不开放，请使用CM_AUDIO_SAMPLE_FORMAT_16BIT */
} cm_audio_sample_format_e;

/** 音频播放支持的采样率 */
typedef enum
{
    CM_AUDIO_SAMPLE_RATE_8000HZ  =  8000,
#if 0
    CM_AUDIO_SAMPLE_RATE_9600HZ  =  9600,
#endif
    CM_AUDIO_SAMPLE_RATE_11025HZ = 11025,
    CM_AUDIO_SAMPLE_RATE_12000HZ = 12000,
    CM_AUDIO_SAMPLE_RATE_16000HZ = 16000,
    CM_AUDIO_SAMPLE_RATE_22050HZ = 22050,
    CM_AUDIO_SAMPLE_RATE_24000HZ = 24000,
    CM_AUDIO_SAMPLE_RATE_32000HZ = 32000,
    CM_AUDIO_SAMPLE_RATE_44100HZ = 44100,
    CM_AUDIO_SAMPLE_RATE_48000HZ = 48000,
    CM_AUDIO_SAMPLE_RATE_96000HZ = 96000,
#if 0
    CM_AUDIO_SAMPLE_RATE_128000HZ = 128000,
#endif
} cm_audio_sample_rate_e;

/** PCM音频采样通道 */
typedef enum
{
    CM_AUDIO_SOUND_MONO = 1,            /*!< （默认）单通道，录音时默认且只能使用CM_AUDIO_SOUND_MONO */
    CM_AUDIO_SOUND_STEREO = 2,          /*!< 双通道（立体声） */
} cm_audio_sound_channel_e;

/** 音频采样参数结构体
 *
 * @note 实际使用说明：
 *       - format=PCM 时：必须传入有效的采样参数
 *       - format=AMRNB/AMRWB 时：传入 NULL，解码器自动使用 AMR 文件内的参数
 *       - format=AMRNB 时：自动使用 8000Hz 采样率
 *       - format=AMRWB 时：自动使用 16000Hz 采样率
 *       - 默认参数（传入 NULL 时）：16000Hz, 16BIT, MONO
 */
typedef struct
{
    cm_audio_sample_format_e sample_format;     /*!< 采样格式（PCM） */
    cm_audio_sample_rate_e rate;                /*!< 采样率（PCM） */
    cm_audio_sound_channel_e num_channels;      /*!< 采样声道（PCM） */
} cm_audio_sample_param_t;

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
/** @}*/