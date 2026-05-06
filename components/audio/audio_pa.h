#ifndef _AUDIO_PA_H_
#define _AUDIO_PA_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "drv_gpio.h"

typedef struct 
{
    /* GPIO 控制的PA 使用*/
    const struct PIN_Res *res;      // 控制的PIN, 不使用，设置为NULL
    GPIO_LEVEL activeLevel;         // 有效电平

    /* IIC  控制的 PA 使用*/
    uint8_t busNo;                  // IIC 总线号
}AudioPaCfg;

/**
 ************************************************************************************
 * @brief           音频PA 配置，board.c 中使用
 *
 * @param[in]       cfg             PA 的 配置

 * 
 * @retval          void
 *                 
 ************************************************************************************
 */
void PA_SetCfg(AudioPaCfg* cfg);

/**
 ************************************************************************************
 * @brief           音频PA 打开
 *
 * @param[in]       void

 * 
 * @retval          void
 *                 
 ************************************************************************************
 */
void PA_Enable(void);

/**
 ************************************************************************************
 * @brief           音频PA 关闭
 *
 * @param[in]       void

 * 
 * @retval          void
 *                 
 ************************************************************************************
 */
void PA_Disable(void);

#endif
