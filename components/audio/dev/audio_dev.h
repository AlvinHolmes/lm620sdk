#ifndef _AUDIO_DEV_H_
#define _AUDIO_DEV_H_

#include "drv_common.h"

/**
 * @brief 支持的音频设备
 * 
 */
typedef enum
{
    AUDIO_DEV_I2S_CODEC,    /** I2S+Codec 支持播放和录音*/
    AUDIO_DEV_DAC,          /** DAC, 只支持播放*/
    AUDIO_DEV_PWM,          /** PWM, 只支持播放*/
}AudioDev;

/**
 * @brief 音频设备的配置
 * 
 */
typedef struct 
{
    uint16_t sampleRate;        /** 采样率*/
    uint16_t frameSize;         /** 帧大小*/
    uint8_t dataBits;           /** 采样点位数*/
    uint8_t chnls;              /** 通道个数*/
    uint8_t ctrl;               /** 控制播放or 录音*/

    AudioDev dev;               /** 使用哪个音频设备*/
}AudioDevCfg;

/**
 ************************************************************************************
 * @brief           音频设备初始化
 *
 * @param[in]       devCfg                     配置参数
 * 
 * @retval          0                       成功                           
 *                  <0                      失败
 ************************************************************************************
 */
int8_t AudioDev_Init(AudioDevCfg *devCfg);

/**
 ************************************************************************************
 * @brief           音频设备播放
 *
 * @param[in]       dev                     设备类型
 * @param[in]       audioData               指向音频数据
 * @param[in]       dev                     音频数据大小
 * @param[in]       timeOut                 超时时间，通常时osNoWait 和 osWaitForever
 * 
 * @retval          0                       成功                           
 *                  <0                      失败
 ************************************************************************************
 */
int8_t AudioDev_Write(AudioDev dev, uint8_t *audioData, uint32_t size, uint32_t timeOut);

/**
 ************************************************************************************
 * @brief           音频设备录音
 *
 * @param[in]       dev                     设备类型
 * @param[in]       audioData               指向音频数据缓冲区
 * @param[in]       dev                     缓冲区大小
 * @param[in]       timeOut                 超时时间，通常时osNoWait 和 osWaitForever
 * 
 * @retval          > 0                      成功, 返回读取的数据大小                        
 *                  <=0                      失败
 ************************************************************************************
 */
int32_t AudioDev_Read(AudioDev dev, uint8_t *audioData, uint32_t size, uint32_t timeOut);

/**
 ************************************************************************************
 * @brief           音频设备去初始化
 *
 * @param[in]       devCfg                     配置参数
 * @param[in]       forceStop                  是否强制停止，播放时，如果不是强制停止，
 *                                              会等待底层播放完再停止
 * @retval          > 0                      成功, 返回读取的数据大小                        
 *                  <=0                      失败
 ************************************************************************************
 */
void AudioDev_Deinit(AudioDevCfg *devCfg, bool forceStop);
#endif
