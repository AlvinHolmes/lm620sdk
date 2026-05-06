/**
  ******************************************************************************
  * @file    pt8311_drv.h
  * @brief   Header file for pt8311 driver compatible interface
  ******************************************************************************
  */

#ifndef _PT8311_DRV_H_
#define _PT8311_DRV_H_

#include <os.h>
#include <drv_common.h>
#include <drv_i2c.h>
#include "drv_pin.h"
#include "pt8311.h"

/**
 * @addtogroup PT8311_Driver
 */
/**@{*/

#include "slog_print.h"

#define PT8311_PRINT_ENABLE 1

#if PT8311_PRINT_ENABLE
#   ifdef PT8311_PRINT
#       define PT8311_Print_Info osPrintf
#       define PT8311_Print_Err osPrintf
#   else
#       define PT8311_Print_Info(fmt, ...)     slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_APP, "[pt8311_drv]" fmt "\r\n", ##__VA_ARGS__)
#       define PT8311_Print_Err(fmt, ...)      slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_APP, "[pt8311_drv]" fmt "\r\n", ##__VA_ARGS__)
#   endif
#else
#   define PT8311_Print_Info(fmt, ...)
#   define PT8311_Print_Err(fmt, ...)
#endif

#define MEDIA_HAL_VOL_DEFAULT 85

// PT8311 I2C地址定义
#define PT8311_I2C_SLAVE_ADDR           PT8311_I2C_ADDR_0

/**
 * @brief Select media hal codec mode
 */
typedef enum {
    MEDIA_HAL_CODEC_MODE_ENCODE = 1,  //select adc
    MEDIA_HAL_CODEC_MODE_DECODE,      //select dac
    MEDIA_HAL_CODEC_MODE_BOTH,        //select adc and dac both
    MEDIA_HAL_CODEC_MODE_LINE_IN,
} media_hal_codec_mode_t;

/**
 * @brief Select operating mode i.e. master or slave for audio codec chip
 */
typedef enum {
    MEDIA_HAL_MODE_SLAVE = 0x00,   //set audio codec chip in slave  mode
    MEDIA_HAL_MODE_MASTER = 0x01,  //set audio codec chip in master mode
} media_hal_op_mode_t;

/**
 * @brief Select operating mode i.e. master or slave for audio codec chip
 */
typedef enum {
    MEDIA_HAL_STOP_STATE  = 0x00,  //stop  audio codec chip mode
    MEDIA_HAL_START_STATE = 0x01,  //start audio codec chip mode
} media_hal_sel_state_t;

/**
 * @brief Select number of bits per sample
 */
typedef enum {
    MEDIA_HAL_BIT_LENGTH_8BITS = 8,    //set  8 bits per sample
    MEDIA_HAL_BIT_LENGTH_16BITS = 16,  //set 16 bits per sample
    MEDIA_HAL_BIT_LENGTH_18BITS = 18,  //set 18 bits per sample
    MEDIA_HAL_BIT_LENGTH_20BITS = 20,  //set 20 bits per sample
    MEDIA_HAL_BIT_LENGTH_24BITS = 24,  //set 24 bits per sample
    MEDIA_HAL_BIT_LENGTH_32BITS = 32,  //set 32 bits per sample
} media_hal_bit_length_t;

/**
 * @brief Select i2s format for audio codec chip
 */
typedef enum {
    MEDIA_HAL_I2S_NORMAL = 0,  //set normal i2s format
    MEDIA_HAL_I2S_LEFT,        //set all left format
    MEDIA_HAL_I2S_RIGHT,       //set all right format
    MEDIA_HAL_I2S_DSP,         //set dsp/pcm format
} media_hal_format_t;

typedef enum {
    MIC_GAIN_MIN = -1,
    MIC_GAIN_0DB,
    MIC_GAIN_6DB,
    MIC_GAIN_12DB,
    MIC_GAIN_18DB,
    MIC_GAIN_24DB,
    MIC_GAIN_30DB,
    MIC_GAIN_36DB,
    MIC_GAIN_42DB,
    MIC_GAIN_MAX
} mic_gain_t;

typedef struct {
    int (*i2c_init)(void);   /* return 0:success; others:failure */
    int (*i2c_write)(uint8_t address, uint8_t reg, uint8_t *bytes, uint8_t size);
    int (*i2c_read)(uint8_t address, uint8_t reg, uint8_t *bytes, uint8_t size);
    void (*pa_ctl)(uint8_t enable); /* PA, 1 : enable, 0 : disable */
} pt8311_api_t;

/**
 * @brief Initialize PT8311 codec chip
 *
 * @param sample_rate  Sample rate
 * @param mclk         MCLK frequency
 * @param mode  Operating mode
 *
 * @return
 *      - 0 : success
 *      - other : failure
 */
int pt8311_codec_init(uint32_t sample_rate, uint32_t mclk, media_hal_op_mode_t mode);

/**
 * @brief Deinitialize PT8311 codec chip
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int pt8311_codec_deinit(void);

int pt8311_codec_powerup(void);
int pt8311_codec_powerdown(void);

int pt8311_i2c_config(uint8_t busNo);
void pt8311_i2c_deconfig(uint8_t busNo);

/**
 * @brief Control PT8311 codec chip
 *
 * @param mode codec mode
 * @param ctrl_state start or stop decode or encode progress
 *
 * @return
 *     - other : failure Parameter error
 *     - 0 : success   Success
 */
int pt8311_codec_ctrl_state(media_hal_codec_mode_t mode, media_hal_sel_state_t ctrl_state);

/**
 * @brief  Set voice volume
 *
 * @param volume:  voice volume (0~100)
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int pt8311_codec_set_volume(int volume);

/**
 * @brief Get voice volume
 *
 * @param[out] *volume:  voice volume (0~100)
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int pt8311_codec_get_volume(int *volume);

/**
 * @brief Configure PT8311 codec mode and I2S interface
 *
 * @param mode codec mode
 * @param fmt I2S format
 *
 * @return
 *     - other : failure Parameter error
 *     - 0 : success   Success
 */
int pt8311_codec_config_format(media_hal_codec_mode_t mode, media_hal_format_t fmt);

/**
 * @brief Configure PT8311 I2S format
 *
 * @param mod:  set ADC or DAC or both
 * @param bit_per_sample:  bit number of per sample
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int pt8311_codec_set_bits_per_sample(media_hal_codec_mode_t mode, media_hal_bit_length_t bits_per_sample);

/**
 * @brief  Start PT8311 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int pt8311_codec_start(int mode);

/**
 * @brief  Stop PT8311 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int pt8311_codec_stop(int mode);

int pt8311_codec_set_mute(int enable);
int pt8311_codec_get_mute(int *mute);
int pt8311_codec_set_mic_gain(int32_t gain_db);

int pt8311_read_reg(uint8_t reg_addr);
int pt8311_write_reg(uint8_t reg_addr, uint8_t data);
void pt8311_read_all(void);

// 扩展接口，用于支持EQ、DRC等功能
int pt8311_codec_set_adc_digit_gain(int32_t db);
int pt8311_codec_get_adc_digit_gain(uint8_t *db, uint8_t *len);
int pt8311_codec_set_dac_digit_gain(int32_t db);
int pt8311_codec_get_dac_digit_gain(uint8_t *db, uint8_t *len);
int pt8311_codec_set_adc_pga_gain(int32_t db);
int pt8311_codec_get_adc_pga_gain(uint8_t *db, uint8_t *len);
int pt8311_codec_get_mic_gain(uint8_t *db, uint8_t *len);
int pt8311_codec_set_nr_gate(int32_t db);
int pt8311_codec_get_nr_gate(uint8_t *db, uint8_t *len);
int pt8311_codec_set_adc_eq(uint8_t *para, uint8_t len);
int pt8311_codec_get_adc_eq(uint8_t *para, uint8_t *len);
int pt8311_codec_set_dac_eq(uint8_t *para, uint8_t len);
int pt8311_codec_get_dac_eq(uint8_t *para, uint8_t *len);
int pt8311_codec_set_drc(uint8_t *para, uint8_t len);
int pt8311_codec_get_drc(uint8_t *para, uint8_t *len);
int pt8311_codec_set_alc(uint8_t *para, uint8_t len);
int pt8311_codec_get_alc(uint8_t *para, uint8_t *len);
int pt8311_codec_get_side_tone(uint8_t *side_tone, uint8_t *len);
int pt8311_codec_set_side_tone(uint8_t side_tone);
int pt8311_codec_get_id(uint8_t *id, uint8_t *len);

#endif
/** @} */