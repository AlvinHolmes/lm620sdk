
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "os.h"


#if 0
#define Print_Info osPrintf
#define Print_Err osPrintf
#else
#define Print_Info(...)
#define Print_Err(...)
#endif

/**
 * @brief Select operating mode i.e. master or slave for audio codec chip
 */
typedef enum {
    MEDIA_HAL_STOP_STATE  = 0x00,  //stop  audio codec chip mode
    MEDIA_HAL_START_STATE = 0x01,  //start audio codec chip mode
} media_hal_sel_state_t;
/**
 * @brief Select operating mode i.e. master or slave for audio codec chip
 */
typedef enum {
    MEDIA_HAL_MODE_SLAVE = 0x00,   //set audio codec chip in slave  mode
    MEDIA_HAL_MODE_MASTER = 0x01,  //set audio codec chip in master mode
} media_hal_op_mode_t;
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



#define		CJC8910_IIC_ADDRESS                         0x18   //CE state low	0x30 ;	CE state high 0X32

#define     CJC8910_R0_LEFT_INPUT_VOLUME                0x01   // Audio input left channel volume; R/W
#define     CJC8910_R2_LOUT1_VOLUME                     0x05   // Audio output letf channel1 volume; R/W
#define     CJC8910_R5_ADC_DAC_CONTROL                  0x0A   // ADC and DAC CONTROL; R/W
#define     CJC8910_R7_AUDIO_INTERFACE                  0x0E   // Digital Audio interface format; R/W
#define     CJC8910_R8_SAMPLE_RATE                      0x10   // Clock and Sample rate control; R/W

#define     CJC8910_R10_LEFT_DAC_VOLUME                 0x15   // Left channel DAC digital volume; R/W

#define     CJC8910_R12_BASS_CONTROL                    0x18   // BASS control; R/W
#define     CJC8910_R13_TREBLE_CONTROL                  0x1A   // Treble control; R/W
#define     CJC8910_R15_RESET                           0x1E   // RESET  ; R/W

#define     CJC8910_R17_ALC1_CONTROL                   	0x22   // ALC1; R/W

#define     CJC8910_R18_ALC2_CONTROL                    0x24   // ALC2; R/W

#define     CJC8910_R19_ALC3_CONTROL                    0x26   // ALC3; R/W
#define     CJC8910_R20_NOISE_GATE_CONTROL              0x28  // NOISE_GATE_CONTROL; R/W
#define     CJC8910_R21_LEFT_ADC_VOLUME                 0x2B   // Left ADC digital volume; R/W


#define     CJC8910_R23_ADDITIONAL_CONTROL1             0x2E   // Additional control; R/W
#define     CJC8910_R24_ADDITIONAL_CONTROL2             0x30   // Additional control; R/W
#define     CJC8910_R27_ADDITIONAL_CONTROL3             0x36   // Additional control; R/W


#define     CJC8910_R32_ADCL_SIGNAL_PATH                0x41   // ADC signal path; R/W
#define     CJC8910_R33_MIC                             0x42   // MIC

#define     CJC8910_R34_AUX                             0x44   //AUX; R/W

#define     CJC8910_R35_LEFT_OUT_MIX2_L                 0x46   // Left out Mix (2); R/W
#define     CJC8910_R35_LEFT_OUT_MIX2_H                 0X47   //

#define     CJC8910_R37_ADC_PDN                         0x4A   // Adc_pdn se; R/W

#define     CJC8910_R67_LOW_POWER_PLAYBACK              0x86   // Low power playback; R/W

#define     CJC8910_R25_PWR_MGMT1_L                     0x32   // Power management1 and VMIDSEL; R/W
#define     CJC8910_R25_PWR_MGMT1_H                     0x33   // Power management1 and VMIDSEL; R/W
#define     CJC8910_R26_PWR_MGMT2_L                     0x34   // Power management2 and DAC left Power down; R/W
#define     CJC8910_R26_PWR_MGMT2_H                     0x35   // Power management2 and DAC right Power up; R/W




void cjc8910_init(uint16_t sampleRate);

/**
 * @brief Deinitialize cjc8910 codec chip
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int cjc8910_codec_deinit(void);

int cjc8910_codec_powerup(void);

int cjc8910_codec_powerdown(void);

void cjc8910_i2c_config(uint8_t busNo);
/**
 * @brief Control cjc8910 codec chip
 *
 * @param mode codec mode
 * @param ctrl_state start or stop decode or encode progress
 *
 * @return
 *     - other : failure Parameter error
 *     - 0 : success   Success
 */
int cjc8910_codec_ctrl_state(media_hal_codec_mode_t mode, media_hal_sel_state_t ctrl_state);

/**
 * @brief Configure cjc8910 codec mode and I2S interface
 *
 * @param mode codec mode
 * @param iface I2S config
 *
 * @return
 *     - other : failure Parameter error
 *     - 0 : success   Success
 */
//int cjc8910_codec_config_i2s(audio_hal_codec_mode_t mode, audio_hal_codec_i2s_iface_t *iface);

/**
 * @brief  Set voice volume
 *
 * @param volume:  voice volume (0~100)
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int cjc8910_codec_set_volume(int volume);

/**
 * @brief Get voice volume
 *
 * @param[out] *volume:  voice volume (0~100)
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int cjc8910_codec_get_volume(int *volume);

/**
 * @brief Configure cjc8910 I2S format
 *
 * @param mod:  set ADC or DAC or both
 * @param cfg:   ES8388 I2S format
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int cjc8910_codec_config_format(media_hal_codec_mode_t mode, media_hal_format_t fmt);

/**
 * @brief Configure cjc8910 data sample bits
 *
 * @param mode:  set ADC or DAC or both
 * @param bit_per_sample:  bit number of per sample
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int cjc8910_codec_set_bits_per_sample(media_hal_codec_mode_t mode, media_hal_bit_length_t bits_per_sample);

/**
 * @brief  Start cjc8910 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int cjc8910_codec_start(int mode);

/**
 * @brief  Stop cjc8910 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - 0 : success
 *     - other : failure
 */
int cjc8910_codec_stop(int mode);
int cjc8910_codec_set_mute(int enable);
int cjc8910_codec_get_mute(int *mute);
int cjc8910_read_reg(uint8_t reg_addr);
void cjc8910_dump_reg(void);
uint16_t cjc8910_get_fs(uint16_t sampleRate);
int  cjc8910_codec_mic_selection(int val);
int cjc8910_codec_mic_loopback(void);

int cjc8910_codec_set_adc_digit_gain(int32_t db);
int cjc8910_codec_get_adc_digit_gain(uint8_t *db, uint8_t *len);
int cjc8910_codec_set_dac_digit_gain(int32_t db);
int cjc8910_codec_get_dac_digit_gain(uint8_t *db, uint8_t *len);
int cjc8910_codec_set_adc_pga_gain(int32_t db);
int cjc8910_codec_get_adc_pga_gain(uint8_t *db, uint8_t *len);
int cjc8910_codec_set_mic_gain(int32_t db);
int cjc8910_codec_get_mic_gain(uint8_t *db, uint8_t *len);
int cjc8910_codec_set_nr_gate(int32_t db);
int cjc8910_codec_get_nr_gate(uint8_t *db, uint8_t *len);
int cjc8910_codec_set_adc_eq(uint8_t *para, uint8_t len);
int cjc8910_codec_get_adc_eq(uint8_t *para, uint8_t *len);
int cjc8910_codec_set_dac_eq(uint8_t *para, uint8_t len);
int cjc8910_codec_get_dac_eq(uint8_t *para, uint8_t *len);
int cjc8910_codec_set_drc(uint8_t *para, uint8_t len);
int cjc8910_codec_get_drc(uint8_t *para, uint8_t *len);
int cjc8910_codec_set_alc(uint8_t *para, uint8_t len);
int cjc8910_codec_get_alc(uint8_t *para, uint8_t *len);
int cjc8910_codec_get_side_tone(uint8_t *side_tone, uint8_t *len);
int cjc8910_codec_set_side_tone(uint8_t side_tone);
int cjc8910_codec_get_id(uint8_t *id, uint8_t *len);


