/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        audio_local.h
 *
 * @brief       This file provides operation functions declaration for audio.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __AUDIO_LOCAL_H__
#define __AUDIO_LOCAL_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "audio_codec.h"
#include "nvparam_pubcfg.h"
#include "enc_dec.h"
#include "audio_dev.h"

#define     AUDIO_PLAY_MODE          1
#define     AUDIO_RECORD_MODE        2
#define     AUDIO_PLAY_RECORD_MODE   3

/**
 * @brief 默认帧大小 
 * 
 */
#define     AUDIO_DEFAULT_FRAME_SIZE        4096

/**
 * @brief 音频文件类型
 * 
 */
typedef enum 
{
    WAV_FILE_DATA = 0,      /** WAV 文件转成 数组数据*/
    MP3_FILE_DATA,          /** MP3 文件转成 数组数据*/
    AMR_FILE_DATA,          /** AMR 文件转成 数组数据*/
    OPUS_FILE_DATA,         /** OPUS 文件转成 数组数据*/
    PCM_RAW_DATA,           /** 纯PCM 数据*/
    MP3_RAW_DATA,           /** 纯MP3帧数据*/
    AMR_RAW_DATA,           /** 纯AMR帧数据*/
    WAV_FILE,
    MP3_FILE,
    AMR_FILE,
    OPUS_FILE,
}AudioFileType;

typedef enum
{
    ONLINE_PLAYER_MODE = 1,
    LOCAL_PLAYER_MODE,
}AudioMode;


/**
 * @brief Audio 回调类型
 */
typedef void (*AudioCallBack) (uint32_t event, void *para); 

/**
 * @brief 音频数据信息
 * 
 */
typedef struct 
{
    AudioDev dev;           /** 音频播放的设备*/
    uint8_t ctrl;           /** 播放 或 录音*/
    uint8_t audioType;      /** 音频类型 AudioFileType*/
    uint8_t isLoop;         /** 是否循环播放*/
    CodecParaType usageType;  /** 使用场景， default music. nb wb voice-msg*/
    AudioMode mode;
    /* 纯PCM数据，
       需要指定音频参数*/
    uint8_t dataBits;       /** 采样点位数*/
    uint8_t chnls;          /** 通道个数*/
    uint32_t sampleRate;    /** 采样率*/
    uint32_t frameSize;     /** 帧大小*/

    char file[64];          /** 文件名， 用于文件系统音频文件*/
    const uint8_t *data;    /** 播放数据的空间， 用于代码块中的音频数据*/
    uint32_t size;          /** 播放数据大小或录音存储的空间大小*/

    uint8_t *recordData;    /** 录音数据的空间 */
    uint32_t recordIndex;/** 录音记录索引*/
    uint32_t recordSize;    /** 录音存储空间大小 */

    AudioCallBack cb;       /** 用户回调接口，用于通知使用者*/
    AudioCallBack sysCb;     /** 系统管理使用*/
}AudioInfo;

typedef AudioInfo   LocalAudioCfg;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief           本地音频打开，调用此接口，音频结束后，资源内部自动释放
 *
 * @param[in]       cfg                   音频信息的配置
 * 
 * @retval          0           成功
 *                 < 0          失败
 ************************************************************************************
 */
int8_t AudioLocal_Open(void * cfg);

/**
 ************************************************************************************
 * @brief           本地音频关闭，
 *
 * @param[in]       ctrl                   播放 or 录音
 * 
 * @retval          0           成功
 *                 < 0          失败
 ************************************************************************************
 */
int8_t AudioLocal_Close(uint8_t ctrl);

/**
 ************************************************************************************
 * @brief           编解码器注册
 *
 * @param[in]       ops                   编解码器的操作方法
 * @param[in]       type                  编解码器类型
 * 
 * @retval          void
 *                 
 ************************************************************************************
 */
void Audio_EncDecRegister(EncDecOps *ops, AudioEncDecType type);

#endif
