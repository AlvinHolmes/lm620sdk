/*
 * ES8311_ERRRESSIF MIT License
 *
 * Copyright (c) 2019 <ES8311_ERRRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ES8311_ERRRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _ES8311_H
#define _ES8311_H

#include <os.h>
#include <drv_common.h>
#include <drv_i2c.h>
#include "drv_pin.h"

/**
 * @addtogroup Es8311
 */

/**@{*/

#ifdef ES8311_PRINT
#define Print_Info osPrintf
#define Print_Err osPrintf
#else
#define Print_Info(...)
#define Print_Err(...) 
#endif

#define MEDIA_HAL_VOL_DEFAULT 85

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
 * @brief Select adc channel for input mic signal
 */
typedef enum {
    MEDIA_HAL_ADC_INPUT_LINE1 = 0x00,  //mic input to adc channel 1
    MEDIA_HAL_ADC_INPUT_LINE2,         //mic input to adc channel 2
    //MEDIA_HAL_ADC_INPUT_ALL =
    MEDIA_HAL_ADC_INPUT_DIFFERENCE,
} media_hal_adc_input_t;

/**
 * @brief Select dac channel for output voice signal
 */
typedef enum {
    MEDIA_HAL_DAC_OUTPUT_LINE1 = 0x00,  //voice output to dac channel 1
    MEDIA_HAL_DAC_OUTPUT_LINE2,         //voive output to dac channel 2
    MEDIA_HAL_DAC_OUTPUT_ALL,
} media_hal_dac_output_t;

/**
 * @brief Select play speed (1x speed is for 16 bit sample)
 */
typedef enum {
    MEDIA_HAL_PLAY_SPEED_1X,  //play speed is normal(considering 16 bit per sample)
    MEDIA_HAL_PLAY_SPEED_2X,  //play speed is twice
    MEDIA_HAL_PLAY_SPEED_4X,  //set play speed to 4x
    MEDIA_HAL_PLAY_SPEED_6X,  //set play speed to 6x
} media_hal_play_speed_t;

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
 * @brief Select voices samples per second
 */
typedef enum {
    MEDIA_HAL_8K_SAMPLES,    //set to  8k voice samples in one second
    MEDIA_HAL_16K_SAMPLES,   //set to 16k voice samples in one second
    MEDIA_HAL_24K_SAMPLES,   //set to 24k voice samples in one second
    MEDIA_HAL_44K_SAMPLES,   //set to 44k voice samples in one second
} media_hal_sel_samples_t;

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


/* ES8311 address
 * 0x32 >>1:CE=1;0x30 >>1:CE=0
 */
//#define ES8311_I2C_SLAVE_ADDR         0x19
#define ES8311_I2C_SLAVE_ADDR           0x18


/*
 *   ES8311_REGISTER NAME_REG_REGISTER ADDRESS
 */
#define ES8311_RESET_REG00              0x00  /*reset digital,csm,clock manager etc.*/

/*
 * Clock Scheme Register definition
 */
#define ES8311_CLK_MANAGER_REG01        0x01 /* select clk src for mclk, enable clock for codec */
#define ES8311_CLK_MANAGER_REG02        0x02 /* clk divider and clk multiplier */
#define ES8311_CLK_MANAGER_REG03        0x03 /* adc fsmode and osr  */
#define ES8311_CLK_MANAGER_REG04        0x04 /* dac osr */
#define ES8311_CLK_MANAGER_REG05        0x05 /* clk divier for adc and dac */
#define ES8311_CLK_MANAGER_REG06        0x06 /* bclk inverter and divider */
#define ES8311_CLK_MANAGER_REG07        0x07 /* tri-state, lrck divider */
#define ES8311_CLK_MANAGER_REG08        0x08 /* lrck divider */
/*
 * SDP
 */
#define ES8311_SDPIN_REG09              0x09 /* dac serial digital port */
#define ES8311_SDPOUT_REG0A             0x0A /* adc serial digital port */
/*
 * SYSTEM
 */
#define ES8311_SYSTEM_REG0B             0x0B /* system */
#define ES8311_SYSTEM_REG0C             0x0C /* system */
#define ES8311_SYSTEM_REG0D             0x0D /* system, power up/down */
#define ES8311_SYSTEM_REG0E             0x0E /* system, power up/down */
#define ES8311_SYSTEM_REG0F             0x0F /* system, low power */
#define ES8311_SYSTEM_REG10             0x10 /* system */
#define ES8311_SYSTEM_REG11             0x11 /* system */
#define ES8311_SYSTEM_REG12             0x12 /* system, Enable DAC */
#define ES8311_SYSTEM_REG13             0x13 /* system */
#define ES8311_SYSTEM_REG14             0x14 /* system, select DMIC, select analog pga gain */
/*
 * ADC
 */
#define ES8311_ADC_REG15                0x15 /* ADC, adc ramp rate, dmic sense */
#define ES8311_ADC_REG16                0x16 /* ADC */
#define ES8311_ADC_REG17                0x17 /* ADC, volume */
#define ES8311_ADC_REG18                0x18 /* ADC, alc enable and winsize */
#define ES8311_ADC_REG19                0x19 /* ADC, alc maxlevel */
#define ES8311_ADC_REG1A                0x1A /* ADC, alc automute */
#define ES8311_ADC_REG1B                0x1B /* ADC, alc automute, adc hpf s1 */
#define ES8311_ADC_REG1C                0x1C /* ADC, equalizer, hpf s2 */
/*
 * DAC
 */
#define ES8311_DAC_REG31                0x31 /* DAC, mute */
#define ES8311_DAC_REG32                0x32 /* DAC, volume */
#define ES8311_DAC_REG33                0x33 /* DAC, offset */
#define ES8311_DAC_REG34                0x34 /* DAC, drc enable, drc winsize */
#define ES8311_DAC_REG35                0x35 /* DAC, drc maxlevel, minilevel */
#define ES8311_DAC_REG37                0x37 /* DAC, ramprate */
/*
 *GPIO
 */
#define ES8311_GPIO_REG44               0x44 /* GPIO, dac2adc for test */
#define ES8311_GP_REG45                 0x45 /* GP CONTROL */
/*
 * CHIP
 */
#define ES8311_CSM_REGFC                0xFC /* CHIP Status Machine */
#define ES8311_CHD1_REGFD               0xFD /* CHIP ID1 */
#define ES8311_CHD2_REGFE               0xFE /* CHIP ID2 */
#define ES8311_CHVER_REGFF              0xFF /* VERSION */

#define ES8311_MAX_REGISTER             0xFF

typedef enum {
    ES8311_MIC_GAIN_MIN = -1,
    ES8311_MIC_GAIN_0DB,
    ES8311_MIC_GAIN_6DB,
    ES8311_MIC_GAIN_12DB,
    ES8311_MIC_GAIN_18DB,
    ES8311_MIC_GAIN_24DB,
    ES8311_MIC_GAIN_30DB,
    ES8311_MIC_GAIN_36DB,
    ES8311_MIC_GAIN_42DB,
    ES8311_MIC_GAIN_MAX
} es8311_mic_gain_t;

typedef struct 
{
    int  (*i2c_init)(void);  /* return 0:success; others:failure */
    int  (*i2c_write)(uint8_t address, uint8_t reg, uint8_t *bytes, uint8_t size);
    int  (*i2c_read)(uint8_t address, uint8_t reg, uint8_t *bytes, uint8_t size);
    void (*pa_ctl)(uint8_t enable); /* PA, 1 : enable, 0 : disable */
} es8311_api_t;
/*
 * @brief Initialize ES8311 codec chip
 *
 * @param codec_cfg  configuration of ES8311
 *
 * @return
 *      - 0 : success
 *      - other : failure
 */
int es8311_codec_init(uint32_t sample_rate, uint32_t mclk, media_hal_op_mode_t es8311_mode);

/**
 * @brief Deinitialize ES8311 codec chip
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int es8311_codec_deinit(void);

int es8311_codec_powerup(void);

int es8311_codec_powerdown(void);

void es8311_i2c_config(uint8_t busNo);
void es8311_i2c_deconfig(uint8_t busNo);
/**
 * @brief Control ES8311 codec chip
 *
 * @param mode codec mode
 * @param ctrl_state start or stop decode or encode progress
 *
 * @return
 *     - other : failure Parameter error
 *     - 0 : success   Success
 */
int es8311_codec_ctrl_state(media_hal_codec_mode_t mode, media_hal_sel_state_t ctrl_state);

/**
 * @brief Configure ES8311 codec mode and I2S interface
 *
 * @param mode codec mode
 * @param iface I2S config
 *
 * @return
 *     - other : failure Parameter error
 *     - 0 : success   Success
 */
//int es8311_codec_config_i2s(audio_hal_codec_mode_t mode, audio_hal_codec_i2s_iface_t *iface);

/**
 * @brief  Set voice volume
 *
 * @param volume:  voice volume (0~100)
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int es8311_codec_set_volume(int volume);

/**
 * @brief Get voice volume
 *
 * @param[out] *volume:  voice volume (0~100)
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int es8311_codec_get_volume(int *volume);

/**
 * @brief Configure ES8311 I2S format
 *
 * @param mod:  set ADC or DAC or both
 * @param cfg:   ES8388 I2S format
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int es8311_codec_config_format(media_hal_codec_mode_t mode, media_hal_format_t fmt);

/**
 * @brief Configure ES8311 data sample bits
 *
 * @param mode:  set ADC or DAC or both
 * @param bit_per_sample:  bit number of per sample
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int es8311_codec_set_bits_per_sample(media_hal_codec_mode_t mode, media_hal_bit_length_t bits_per_sample);

/**
 * @brief  Start ES8311 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int es8311_codec_start(int mode);

/**
 * @brief  Stop ES8311 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int es8311_codec_stop(int mode);
int es8311_codec_set_mute(int enable);
int es8311_codec_get_mute(int *mute);
int es8311_codec_set_mic_gain(int32_t gain_db);

int  es8311_codec_mic_selection(int val);
int es8311_codec_mic_loopback(void);

int es8311_read_reg(uint8_t reg_addr);
int es8311_write_reg(uint8_t reg_addr, uint8_t data);
void es8311_codec_read_all(void);

int es8311_codec_set_adc_digit_gain(int32_t db);
int es8311_codec_get_adc_digit_gain(uint8_t *db, uint8_t *len);
int es8311_codec_set_dac_digit_gain(int32_t db);
int es8311_codec_get_dac_digit_gain(uint8_t *db, uint8_t *len);
int es8311_codec_set_adc_pga_gain(int32_t db);
int es8311_codec_get_adc_pga_gain(uint8_t *db, uint8_t *len);
int es8311_codec_set_mic_gain(int32_t db);
int es8311_codec_get_mic_gain(uint8_t *db, uint8_t *len);
int es8311_codec_set_nr_gate(int32_t db);
int es8311_codec_get_nr_gate(uint8_t *db, uint8_t *len);
int es8311_codec_set_adc_eq(uint8_t *para, uint8_t len);
int es8311_codec_get_adc_eq(uint8_t *para, uint8_t *len);
int es8311_codec_set_dac_eq(uint8_t *para, uint8_t len);
int es8311_codec_get_dac_eq(uint8_t *para, uint8_t *len);
int es8311_codec_set_drc(uint8_t *para, uint8_t len);
int es8311_codec_get_drc(uint8_t *para, uint8_t *len);
int es8311_codec_set_alc(uint8_t *para, uint8_t len);
int es8311_codec_get_alc(uint8_t *para, uint8_t *len);
int es8311_codec_get_side_tone(uint8_t *side_tone, uint8_t *len);
int es8311_codec_set_side_tone(uint8_t side_tone);
int es8311_codec_get_id(uint8_t *id, uint8_t *len);

int pa_read_reg(uint8_t reg_addr);
int pa_write_reg(uint8_t reg_addr, uint8_t data);
#endif
/** @} */
