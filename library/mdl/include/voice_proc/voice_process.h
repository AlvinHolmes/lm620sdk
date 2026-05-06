

#ifndef __AUDIO_PROC__
#define __AUDIO_PROC__

#include "stdint.h"
#include "stddef.h"

void VoiceProc_Initialize(uint16_t sampleRate, int nrLevel, int drc_gain);

void VoiceProc_UnInitialize(void);

void VoiceProc_TxProc(uint8_t *frameBuf, uint16_t frameSize);

void VoiceProc_RxProc(uint8_t *frameBuf, uint16_t frameSize);

void VoiceProc_SendMsg(uint32_t bufAddr);

/**
 ************************************************************************************
 * @brief           设置算法后音频数据倍数因子
 *
 * @param[in]       factor              倍数因子
 *
 * @return          0 成功， 非0失败
 ************************************************************************************
 */
int8_t Voice_Set_Multiplier(float factor);

/**
 ************************************************************************************
 * @brief           获取当前倍数因子
 *
 * @param[in]       void
 *
 * @return          当前倍数因子
 ************************************************************************************
 */
float Voice_Get_Multiplier(void);
#endif
