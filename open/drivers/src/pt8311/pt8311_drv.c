/**
  ******************************************************************************
  * @file    pt8311_drv.c
  * @brief   PT8311 driver compatible interface implementation
  ******************************************************************************
  */

#include "pt8311_drv.h"
#include "pt8311_user_cfg.h"
#include <string.h>

// 全局变量
static bool g_pt8311Initialized = false;
static uint32_t g_currentSampleRate = 0;
static PT8311_I2SCfgTypeDef g_currentI2sCfg;
static PT8311_SysclkCfgTypeDef g_currentSysclkCfg;
static int g_currentVolume = MEDIA_HAL_VOL_DEFAULT;

// 内部辅助函数
static PT8311_I2S_FmtETypeDef convert_format(media_hal_format_t media_hal_fmt);
static PT8311_I2S_WordLenETypeDef convert_word_len(media_hal_bit_length_t bits);
static PT8311_UserSmpRateETypeDef convert_sample_rate(uint32_t rate);
static PT8311_InitModETypeDef convert_init_mode(media_hal_codec_mode_t mode);

/**
 * @brief Initialize PT8311 codec chip
 */
int pt8311_codec_init(uint32_t sample_rate, uint32_t mclk, media_hal_op_mode_t pt8311_mode)
{
    PT8311_StatusETypeDef status;
    PT8311_InitModETypeDef init_mod;

    PT8311_Print_Info("PT8311 codec init: sample_rate=%d, mclk=%d, mode=%d", sample_rate, mclk, pt8311_mode);

    // 初始化时钟和I2S配置
    memset(&g_currentI2sCfg, 0, sizeof(g_currentI2sCfg));
    memset(&g_currentSysclkCfg, 0, sizeof(g_currentSysclkCfg));

    g_currentSampleRate = sample_rate;
    g_currentSysclkCfg.extclk_freq = mclk;

    // 转换初始化模式
    init_mod = PT8311_INIT_MOD_ADC_DAC; // 默认ADC和DAC都初始化
    // 注意：pt8311_mode 是操作模式（master/slave），不是编码模式
    // 这里默认初始化ADC和DAC，具体模式由后续的 codec_start/stop 控制

    // 初始化PT8311
    status = pt8311_init(PT8311_I2C_SLAVE_ADDR, init_mod);
    if (status != PT8311_OK) {
        PT8311_Print_Err("PT8311 init failed: %d", status);
        return -1;
    }

    g_pt8311Initialized = true;
    PT8311_Print_Info("PT8311 codec initialized successfully");
    return 0;
}

/**
 * @brief Deinitialize PT8311 codec chip
 */
int pt8311_codec_deinit(void)
{
    PT8311_StatusETypeDef status;

    if (!g_pt8311Initialized) {
        return 0;
    }

    status = pt8311_deinit(PT8311_I2C_SLAVE_ADDR, PT8311_INIT_MOD_ADC_DAC);
    if (status != PT8311_OK) {
        PT8311_Print_Err("PT8311 deinit failed: %d", status);
        return -1;
    }

    g_pt8311Initialized = false;
    PT8311_Print_Info("PT8311 codec deinitialized");
    return 0;
}

/**
 * @brief Power up PT8311 codec
 */
int pt8311_codec_powerup(void)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    // PT8311通过init函数自动上电，这里可以留空
    return 0;
}

/**
 * @brief Power down PT8311 codec
 */
int pt8311_codec_powerdown(void)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    // 停止播放和录音
    pt8311_play_stop(PT8311_I2C_SLAVE_ADDR);
    pt8311_record_stop(PT8311_I2C_SLAVE_ADDR);

    return 0;
}

/**
 * @brief Configure PT8311 codec format
 */
int pt8311_codec_config_format(media_hal_codec_mode_t mode, media_hal_format_t fmt)
{
    PT8311_StatusETypeDef status;

    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    // 配置I2S格式
    g_currentI2sCfg.bclk_invert = false;
    /* I2S 配置默认左声道， pt8311 默认右声道，这里翻转 */
    g_currentI2sCfg.lrck_invert = true;
    g_currentI2sCfg.fmt = convert_format(fmt);
    g_currentI2sCfg.role = PT8311_I2S_ROLE_SLAVE; // 默认slave模式
    g_currentI2sCfg.word_len = PT8311_I2S_WORD_LEN_16bit; // 默认16bit

    status = pt8311_i2s_cfg(PT8311_I2C_SLAVE_ADDR, &g_currentI2sCfg);
    if (status != PT8311_OK) {
        PT8311_Print_Err("PT8311 I2S config failed: %d", status);
        return -1;
    }

    PT8311_Print_Info("PT8311 I2S format configured: fmt=%d", fmt);
    return 0;
}

/**
 * @brief Configure PT8311 bits per sample
 */
int pt8311_codec_set_bits_per_sample(media_hal_codec_mode_t mode, media_hal_bit_length_t bits_per_sample)
{
    (void)mode;
    PT8311_StatusETypeDef status;

    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    g_currentI2sCfg.word_len = convert_word_len(bits_per_sample);

    status = pt8311_i2s_cfg(PT8311_I2C_SLAVE_ADDR, &g_currentI2sCfg);
    if (status != PT8311_OK) {
        PT8311_Print_Err("PT8311 I2S word length config failed: %d", status);
        return -1;
    }

    PT8311_Print_Info("PT8311 I2S word length configured: bits=%d", bits_per_sample);
    return 0;
}

/**
 * @brief Start PT8311 codec chip
 */
int pt8311_codec_start(int mode)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    // 配置系统时钟
    /* 在 pt8311_codec_init 设置了 */
    // g_currentSysclkCfg.extclk_freq = (8000 * 128);
    g_currentSysclkCfg.extclk_src = PT8311_EXT_CLK_SRC_MCLK;
    // 根据模式设置采样率
    if (mode & MEDIA_HAL_CODEC_MODE_DECODE) {
        g_currentSysclkCfg.play_smp_rate = convert_sample_rate(g_currentSampleRate);
    } else {
        g_currentSysclkCfg.play_smp_rate = PT8311_USER_SR_NONE;
    }
    if (mode & MEDIA_HAL_CODEC_MODE_ENCODE) {
        g_currentSysclkCfg.rec_smp_rate = convert_sample_rate(g_currentSampleRate);
    } else {
        g_currentSysclkCfg.rec_smp_rate = PT8311_USER_SR_NONE;
    }
    g_currentSysclkCfg.work_mode = PT8311_ADDA_WORK_MODE_NORMAL;
    g_currentSysclkCfg.i2s_lrck_period =
        0; // i2s_lrck_period参数表示LRCK周期内的BCLK数量，等于slot_width *slot_nums， 仅在I2S主机模式下使用。在I2S从机模式下，请配置为0。
    PT8311_StatusETypeDef status = pt8311_sysclk_cfg(PT8311_I2C_SLAVE_ADDR, &g_currentSysclkCfg);
    if (status != PT8311_OK) {
        PT8311_Print_Err("PT8311 sysclk config failed: %d", status);
        return -1;
    }

    // 启动播放/录音
    if (mode & MEDIA_HAL_CODEC_MODE_DECODE) {
        pt8311_play_start(PT8311_I2C_SLAVE_ADDR);
        PT8311_Print_Info("PT8311 play started");
    }
    if (mode & MEDIA_HAL_CODEC_MODE_ENCODE) {
        pt8311_record_start(PT8311_I2C_SLAVE_ADDR);
        PT8311_Print_Info("PT8311 record started");
    }

    return 0;
}

/**
 * @brief Stop PT8311 codec chip
 */
int pt8311_codec_stop(int mode)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    // 停止播放/录音
    if (mode & MEDIA_HAL_CODEC_MODE_DECODE) {
        pt8311_play_stop(PT8311_I2C_SLAVE_ADDR);
        PT8311_Print_Info("PT8311 play stopped");
    }
    if (mode & MEDIA_HAL_CODEC_MODE_ENCODE) {
        pt8311_record_stop(PT8311_I2C_SLAVE_ADDR);
        PT8311_Print_Info("PT8311 record stopped");
    }

    return 0;
}

/**
 * @brief Set voice volume
 */
int pt8311_codec_set_volume(int volume)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    if (volume < 0 || volume > 100) {
        PT8311_Print_Err("Invalid volume: %d", volume);
        return -1;
    }

    // 将0-100的音量值转换为PT8311的0-255范围
    // PT8311: 0=mute, 1=-95dB, 0xBF=0dB, 0xFF=32dB
    uint8_t pt8311_vol = (uint8_t)(volume * 255 / 100);

    pt8311_play_vol_cfg(PT8311_I2C_SLAVE_ADDR, pt8311_vol);
    g_currentVolume = volume;

    PT8311_Print_Info("PT8311 volume set: %d (pt8311_vol=%d)", volume, pt8311_vol);
    return 0;
}

/**
 * @brief Get voice volume
 */
int pt8311_codec_get_volume(int *volume)
{
    if (!g_pt8311Initialized || !volume) {
        return -1;
    }

    *volume = g_currentVolume;
    return 0;
}

/**
 * @brief Control PT8311 codec chip state
 */
int pt8311_codec_ctrl_state(media_hal_codec_mode_t mode, media_hal_sel_state_t ctrl_state)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    if (ctrl_state == MEDIA_HAL_START_STATE) {
        return pt8311_codec_start(mode);
    } else {
        return pt8311_codec_stop(mode);
    }
}

/**
 * @brief Set mute
 */
int pt8311_codec_set_mute(int enable)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    if (enable) {
        // 设置为mute
        pt8311_play_vol_cfg(PT8311_I2C_SLAVE_ADDR, 0); // 0 = mute
    } else {
        // 恢复音量
        pt8311_codec_set_volume(g_currentVolume);
    }

    PT8311_Print_Info("PT8311 mute set: %d", enable);
    return 0;
}

/**
 * @brief Get mute status
 */
int pt8311_codec_get_mute(int *mute)
{
    if (!g_pt8311Initialized || !mute) {
        return -1;
    }
    uint8_t reg_val = __PT8311_DAC_DigVol_Get(PT8311_I2C_SLAVE_ADDR);
    if (reg_val == 0) {
        *mute = 1;
    } else {
        *mute = 0;
    }
    return 0;
}

/**
 * @brief Set microphone gain
 */
int pt8311_codec_set_mic_gain(int32_t gain_db)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }
    /* 见 NV: MicGain  */
    gain_db = gain_db / 100;

    // 将dB值转换为PT8311的增益枚举
    PT8311_ADC_PGAGainETypeDef pt8311_gain = PT8311_ADC_PGA_GAIN_0dB;

    if (gain_db <= 0) {
        pt8311_gain = PT8311_ADC_PGA_GAIN_0dB;
    } else if (gain_db <= 6) {
        pt8311_gain = PT8311_ADC_PGA_GAIN_6dB;
    } else if (gain_db <= 12) {
        pt8311_gain = PT8311_ADC_PGA_GAIN_12dB;
    } else if (gain_db <= 18) {
        pt8311_gain = PT8311_ADC_PGA_GAIN_18dB;
    } else if (gain_db <= 24) {
        pt8311_gain = PT8311_ADC_PGA_GAIN_24dB;
    } else if (gain_db <= 30) {
        pt8311_gain = PT8311_ADC_PGA_GAIN_30dB;
    } else if (gain_db <= 36) {
        pt8311_gain = PT8311_ADC_PGA_GAIN_36dB;
    }  else if (gain_db <= 39) {
        pt8311_gain = PT8311_ADC_PGA_GAIN_39dB;
    } else {
        pt8311_gain = PT8311_ADC_PGA_GAIN_42dB;
    }

    pt8311_record_gain_cfg(PT8311_I2C_SLAVE_ADDR, pt8311_gain);

    PT8311_Print_Info("PT8311 mic gain set: %d dB", gain_db);
    return 0;
}

/**
 * @brief Read register
 */
int pt8311_read_reg(uint8_t reg_addr)
{
    if (!g_pt8311Initialized) {
        return -1;
    }

    return (int)pt8311_i2c_read_byte(PT8311_I2C_SLAVE_ADDR, reg_addr);
}

/**
 * @brief Write register
 */
int pt8311_write_reg(uint8_t reg_addr, uint8_t data)
{
    if (!g_pt8311Initialized) {
        return -1;
    }

    return (int)pt8311_i2c_write_byte(PT8311_I2C_SLAVE_ADDR, reg_addr, data);
}

/**
 * @brief Read all registers
 */
void pt8311_read_all(void)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return;
    }

    pt8311_all_regs_read(PT8311_I2C_SLAVE_ADDR, true);
}

int pt8311_codec_set_adc_digit_gain(int32_t db)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    // PT8311 ADC 数字增益通过 ADC_DVC_CTRL 寄存器控制
    // 范围: -95dB 到 +32dB, 0.5dB/step, 寄存器值 0-255
    // 0 = Mute, 1 = -95dB, 0xBF = 0dB, 0xFF = +32dB
    // 默认值: -1500 (从 nvOutFile.json, -15dB)

    if (db < -9500) {
        db = -9500;    // -95dB
    }
    if (db > 3200) {
        db = 3200;    // +32dB
    }

    // 将 dB 值转换为寄存器值
    uint8_t reg_val;
    if (db <= -9500) {
        /* TODO 区分静音 */
        reg_val = 0;  // Mute
    } else {
        reg_val = (uint8_t)((db + 9500) / 50);
        if (reg_val == 0) {
            reg_val = 1;    // 避免 0 值（mute），除非明确要求
        }
        if (reg_val > 255) {
            reg_val = 255;
        }
    }

    // 设置 PT8311 ADC 数字音量
    __PT8311_ADC_DigVol_Set(PT8311_I2C_SLAVE_ADDR, reg_val);

    PT8311_Print_Info("PT8311 ADC digital gain set: %d dB (reg: 0x%02X)", db, reg_val);
    return 0;
}

int pt8311_codec_get_adc_digit_gain(uint8_t *db, uint8_t *len)
{
    if (!db || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }

    // 读取当前 ADC 数字音量寄存器值
    uint8_t reg_val = pt8311_i2c_read_byte(PT8311_I2C_SLAVE_ADDR, ADC_DVC_CTRL);

    // 转换为 dB 值
    int32_t current_db;
    if (reg_val == 0) {
        /* TODO 区分静音 */
        current_db = -9500;  // Mute
    } else {
        current_db = (int32_t)reg_val * 50 - 9550;
    }

    db[0] = (current_db >> 24) & 0xFF;
    db[1] = (current_db >> 16) & 0xFF;
    db[2] = (current_db >> 8) & 0xFF;
    db[3] = current_db & 0xFF;
    *len = 4;

    PT8311_Print_Info("PT8311 ADC digital gain get: %d dB (reg: 0x%02X)", current_db, reg_val);
    return 0;
}

int pt8311_codec_set_dac_digit_gain(int32_t db)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    // PT8311 DAC 数字增益通过 DAC_DVC_CTRL 寄存器控制
    // 范围: -95dB 到 +32dB, 0.5dB/step, 寄存器值 0-255
    // 0 = Mute, 1 = -95dB, 0xBF = 0dB, 0xFF = +32dB
    // 默认值: -1500 (从 nvOutFile.json)

    if (db < -9500) {
        db = -9500;    // -95dB
    }
    if (db > 3200) {
        db = 3200;    // +32dB
    }

    // 将 dB 值转换为寄存器值
    // reg_val = (db + 95dB) * 2，但需要处理 mute 情况
    uint8_t reg_val;
    if (db <= -9500) {
        /* TODO 区分静音 */
        reg_val = 0;  // Mute
    } else {
        reg_val = (uint8_t)((db + 9500) / 50);
        if (reg_val == 0) {
            reg_val = 1;    // 避免 0 值（mute），除非明确要求
        }
        if (reg_val > 255) {
            reg_val = 255;
        }
    }

    // 设置 PT8311 DAC 数字音量
    __PT8311_DAC_DigVol_Set(PT8311_I2C_SLAVE_ADDR, reg_val);

    PT8311_Print_Info("PT8311 DAC digital gain set: %d dB (reg: 0x%02X)", db, reg_val);
    return 0;
}

int pt8311_codec_get_dac_digit_gain(uint8_t *db, uint8_t *len)
{
    if (!db || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }

    // 读取当前 DAC 数字音量寄存器值
    uint8_t reg_val = __PT8311_DAC_DigVol_Get(PT8311_I2C_SLAVE_ADDR);

    // 转换为 dB 值
    int32_t current_db;
    if (reg_val == 0) {
        /* TODO 区分静音 */
        current_db = -9500;  // Mute
    } else {
        current_db = (int32_t)reg_val * 50 - 9550;
    }

    db[0] = (current_db >> 24) & 0xFF;
    db[1] = (current_db >> 16) & 0xFF;
    db[2] = (current_db >> 8) & 0xFF;
    db[3] = current_db & 0xFF;
    *len = 4;

    PT8311_Print_Info("PT8311 DAC digital gain get: %d dB (reg: 0x%02X)", current_db, reg_val);
    return 0;
}

int pt8311_codec_set_adc_pga_gain(int32_t db)
{
    PT8311_Print_Info("PT8311 %s", __func__);
    return pt8311_codec_set_mic_gain(db);
}

int pt8311_codec_get_adc_pga_gain(uint8_t *db, uint8_t *len)
{
    PT8311_Print_Info("PT8311 %s", __func__);
    // ADC PGA 增益和 Mic 增益使用相同的实现
    return pt8311_codec_get_mic_gain(db, len);
}

int pt8311_codec_get_mic_gain(uint8_t *db, uint8_t *len)
{
    if (!db || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }

    uint8_t reg;
    reg = PT8311_READ_BIT_SHIFT(PT8311_I2C_SLAVE_ADDR, ANA_ADC_CTRL1, ADC_PGA_GAIN_Msk, ADC_PGA_GAIN_Pos);

    // 返回默认值: 2400 (从 nvOutFile.json, 24dB)
    int32_t default_gain = reg * 1000;
    db[0] = (default_gain >> 24) & 0xFF;
    db[1] = (default_gain >> 16) & 0xFF;
    db[2] = (default_gain >> 8) & 0xFF;
    db[3] = default_gain & 0xFF;
    *len = 4;

    PT8311_Print_Info("PT8311 mic gain get: %d dB", default_gain);
    return 0;
}

int pt8311_codec_set_nr_gate(int32_t db)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }

    // PT8311 ADC DRC 噪声门限通过 ADC_DRC_THD4 寄存器的 ADC_DRC_NG_THD 位控制
    // 范围: -35dB 到 -98dB, 步长: -1dB, 寄存器值: 0-63
    // 0 = -35dB, 63 = -98dB

    if (db < -9800) {
        db = -9800;  // -98dB
    }
    if (db > -3500) {
        db = -3500;  // -35dB
    }

    // 将 dB 值转换为寄存器值
    // reg_val = (db + 35dB) / -1dB，但需要反向映射
    uint8_t reg_val = (uint8_t)((-db - 3500) / 100);
    if (reg_val > 63) {
        reg_val = 63;
    }

    // 读取当前寄存器值
    uint8_t current_reg = pt8311_i2c_read_byte(PT8311_I2C_SLAVE_ADDR, ADC_DRC_THD4);

    // 清除噪声门限位并设置新值
    uint8_t new_reg = (current_reg & ~0x3F) | (reg_val & 0x3F);

    // 写入寄存器
    int result = pt8311_i2c_write_byte(PT8311_I2C_SLAVE_ADDR, ADC_DRC_THD4, new_reg);
    if (result != 0) {
        PT8311_Print_Err("Failed to set ADC DRC noise gate threshold");
        return -1;
    }

    PT8311_Print_Info("PT8311 ADC DRC noise gate set: %d dB (reg: 0x%02X)", db, reg_val);
    return 0;
}

int pt8311_codec_get_nr_gate(uint8_t *db, uint8_t *len)
{
    if (!db || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }

    // 读取 ADC DRC 噪声门限寄存器值
    uint8_t reg_val = pt8311_i2c_read_byte(PT8311_I2C_SLAVE_ADDR, ADC_DRC_THD4);

    // 提取噪声门限位 (0-5位)
    uint8_t noise_gate_reg = reg_val & 0x3F;

    // 将寄存器值转换为 dB 值
    // reg_val: 0-63, 对应 -35dB 到 -98dB
    // dB = -(35 + reg_val)
    int32_t current_db = -(3500 + noise_gate_reg * 100);

    // 返回 4 字节的 dB 值
    db[0] = (current_db >> 24) & 0xFF;
    db[1] = (current_db >> 16) & 0xFF;
    db[2] = (current_db >> 8) & 0xFF;
    db[3] = current_db & 0xFF;
    *len = 4;

    PT8311_Print_Info("PT8311 ADC DRC noise gate get: %d dB (reg: 0x%02X)", current_db, noise_gate_reg);
    return 0;
}

/* TODO pt8311_codec_set_adc_eq 目前使用示例参数 */
int pt8311_codec_set_adc_eq(uint8_t *para, uint8_t len)
{
    if (!g_pt8311Initialized || !para) {
        PT8311_Print_Err("PT8311 not initialized or invalid parameters");
        return -1;
    }

    // // PT8311 ADC EQ 设置，长度应为 46 字节 (23 * 2)
    // if (len != 46) {
    //     PT8311_Print_Err("ADC EQ parameter length should be 46");
    //     return -1;
    // }

    // // 将 para 数组转换为 int32_t 数组，PT8311 EQ 配置需要 int32_t 数组
    // int32_t eq_array[23];
    // for (int i = 0; i < 23; i++) {
    //     // 每个 EQ 系数由 2 个字节组成，转换为 int32_t
    //     eq_array[i] = (int32_t)((para[i * 2] << 8) | para[i * 2 + 1]);
    //     // 处理符号位（如果最高位为1，则表示负数）
    //     if (eq_array[i] & 0x8000) {
    //         eq_array[i] |= 0xFFFF0000;
    //     }
    // }

    // // 使用 PT8311 的 ADC EQ 配置函数
    // PT8311_StatusETypeDef status = pt8311_adc_eq_cfg(PT8311_I2C_SLAVE_ADDR, true, eq_array, 23);
    // if (status != PT8311_OK) {
    //     PT8311_Print_Err("PT8311 ADC EQ config failed: %d", status);
    //     return -1;
    // }

    static const int adc_eq_filt_coef[] = {
        0xd0307, 0xe5f9f1, 0xd0307, 0xe687af, 0xa93cc,
    };
    pt8311_adc_eq_cfg(PT8311_I2C_SLAVE_ADDR, true, (const int32_t *)adc_eq_filt_coef, PT8311_ARRAY_SIZE(adc_eq_filt_coef));

    PT8311_Print_Info("PT8311 ADC EQ set with %d bytes", len);
    return 0;
}

/* TODO pt8311_codec_get_adc_eq */
int pt8311_codec_get_adc_eq(uint8_t *para, uint8_t *len)
{
    if (!para || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }

    // 返回默认的 ADC EQ 参数（全零）
    memset(para, 0, 46);
    *len = 46;

    PT8311_Print_Info("PT8311 ADC EQ get: returned %d bytes", *len);
    return 0;
}

/* TODO pt8311_codec_set_dac_eq 目前使用示例参数 */
int pt8311_codec_set_dac_eq(uint8_t *para, uint8_t len)
{
    if (!g_pt8311Initialized || !para) {
        PT8311_Print_Err("PT8311 not initialized or invalid parameters");
        return -1;
    }

    // // PT8311 DAC EQ 设置，长度应为 32 字节
    // if (len != 32) {
    //     PT8311_Print_Err("DAC EQ parameter length should be 32");
    //     return -1;
    // }

    // // 将 para 数组转换为 int32_t 数组，PT8311 EQ 配置需要 int32_t 数组
    // int32_t eq_array[16];
    // for (int i = 0; i < 16; i++) {
    //     // 每个 EQ 系数由 2 个字节组成，转换为 int32_t
    //     eq_array[i] = (int32_t)((para[i * 2] << 8) | para[i * 2 + 1]);
    //     // 处理符号位（如果最高位为1，则表示负数）
    //     if (eq_array[i] & 0x8000) {
    //         eq_array[i] |= 0xFFFF0000;
    //     }
    // }

    // // 使用 PT8311 的 DAC EQ 配置函数
    // PT8311_StatusETypeDef status = pt8311_dac_eq_cfg(PT8311_I2C_SLAVE_ADDR, true, eq_array, 16);
    // if (status != PT8311_OK) {
    //     PT8311_Print_Err("PT8311 DAC EQ config failed: %d", status);
    //     return -1;
    // }

    static const int dac_eq_filt_coef[] = {
        0xfece5,  0xe02635, 0xfece5, 0xe0264c, 0xfd9e1,
        0x10a9fc, 0xe0aca1, 0xeb35e, 0xe0aca1, 0xf5d5b,
        0xc638b,  0xef09f8, 0x79ca0, 0xef09f8, 0x4002c,
    };
    pt8311_dac_eq_cfg(PT8311_I2C_SLAVE_ADDR, true, (const int32_t *)dac_eq_filt_coef, PT8311_ARRAY_SIZE(dac_eq_filt_coef));

    PT8311_Print_Info("PT8311 DAC EQ set with %d bytes", len);
    return 0;
}

/* TODO pt8311_codec_get_dac_eq */
int pt8311_codec_get_dac_eq(uint8_t *para, uint8_t *len)
{
    if (!para || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }

    // 返回默认的 DAC EQ 参数（全零）
    memset(para, 0, 32);
    *len = 32;

    PT8311_Print_Info("[nonsupport]PT8311 DAC EQ get: returned %d bytes", *len);
    return -1;
}

/* TODO pt8311_codec_set_drc 目前使用示例参数 */
int pt8311_codec_set_drc(uint8_t *para, uint8_t len)
{
    if (!g_pt8311Initialized || !para) {
        PT8311_Print_Err("PT8311 not initialized or invalid parameters");
        return -1;
    }

    // // PT8311 DRC 设置，长度应为 10 字节
    // if (len != 10) {
    //     PT8311_Print_Err("DRC parameter length should be 10");
    //     return -1;
    // }

    // // 将 para 数组转换为 int32_t 数组，PT8311 DRC 配置需要 int32_t 数组
    // int32_t drc_array[5];
    // for (int i = 0; i < 5; i++) {
    //     // 每个 DRC 参数由 2 个字节组成，转换为 int32_t
    //     drc_array[i] = (int32_t)((para[i * 2] << 8) | para[i * 2 + 1]);
    //     // 处理符号位（如果最高位为1，则表示负数）
    //     if (drc_array[i] & 0x8000) {
    //         drc_array[i] |= 0xFFFF0000;
    //     }
    // }

    // // 使用 PT8311 的 DAC DRC 配置函数（因为通常 DRC 主要用于 DAC）
    // PT8311_StatusETypeDef status = pt8311_dac_drc_cfg(PT8311_I2C_SLAVE_ADDR, true, drc_array, 5);
    // if (status != PT8311_OK) {
    //     PT8311_Print_Err("PT8311 DRC config failed: %d", status);
    //     return -1;
    // }

    static const int dac_drc_filt_coef[] = {
        0x76, 0x27, 0x0,  0x12,
        0x11, 0x26, 0x74, 0x7a,
    };
    pt8311_dac_drc_cfg(PT8311_I2C_SLAVE_ADDR, true, (const int32_t *)dac_drc_filt_coef,
                       PT8311_ARRAY_SIZE(dac_drc_filt_coef));

    PT8311_Print_Info("PT8311 DRC set with %d bytes", len);
    return 0;
}

/* TODO pt8311_codec_get_drc */
int pt8311_codec_get_drc(uint8_t *para, uint8_t *len)
{
    if (!para || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }

    // 返回默认的 DRC 参数（全零）
    memset(para, 0, 10);
    *len = 10;

    PT8311_Print_Info("[nonsupport]PT8311 DRC get: returned %d bytes", *len);
    return -1;
}

int pt8311_codec_set_alc(uint8_t *para, uint8_t len)
{
    if (!g_pt8311Initialized || !para) {
        PT8311_Print_Err("PT8311 not initialized or invalid parameters");
        return -1;
    }

    /* NOTE 不支持 */

    // PT8311 ALC 设置，长度应为 12 字节
    if (len != 12) {
        PT8311_Print_Err("ALC parameter length should be 12");
        return -1;
    }

    // PT8311 可能没有专门的 ALC 寄存器，这里记录设置
    // 可以通过 ADC 的增益控制和 HPF 来模拟 ALC 功能
    PT8311_Print_Info("[nonsupport]PT8311 ALC set with %d bytes (simulated)", len);
    return -1;
}

int pt8311_codec_get_alc(uint8_t *para, uint8_t *len)
{
    if (!para || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }

    /* NOTE 不支持 */

    // 返回默认的 ALC 参数（全零）
    memset(para, 0, 12);
    *len = 12;

    PT8311_Print_Info("[nonsupport]PT8311 ALC get: returned %d bytes", *len);
    return -1;
}

int pt8311_codec_get_side_tone(uint8_t *side_tone, uint8_t *len)
{
    if (!side_tone || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }
    /* NOTE 不支持 */
    PT8311_Print_Info("[nonsupport]PT8311 side tone get: %d", side_tone[0]);
    return -1;
}

int pt8311_codec_set_side_tone(uint8_t side_tone)
{
    if (!g_pt8311Initialized) {
        PT8311_Print_Err("PT8311 not initialized");
        return -1;
    }
    /* NOTE 不支持 */
    PT8311_Print_Info("[nonsupport]PT8311 side tone set: %d", side_tone);
    return -1;
}

int pt8311_codec_get_id(uint8_t *id, uint8_t *len)
{
    if (!id || !len) {
        PT8311_Print_Err("Invalid parameters");
        return -1;
    }

    /* NOTE 不支持 */

    /* 没有 ID 寄存器，只是确定能不能与 pt8311 通信 */
    uint8_t reg = pt8311_i2c_read_byte(PT8311_I2C_SLAVE_ADDR, ANA_DAC_TUNE2);
    if (reg == 0x00) {
        PT8311_Print_Err("Failed to read codec ID");
        return -1;
    }

    // 假的
    id[0] = 0x63;  // 'c'
    id[1] = 0x31;  // '1'
    id[2] = 0x00;
    id[3] = 0x00;
    *len = 4;

    PT8311_Print_Info("[nonsupport]PT8311 codec ID: %02X %02X %02X %02X", id[0], id[1], id[2], id[3]);
    return 0;
}

// 内部辅助函数实现
static PT8311_I2S_FmtETypeDef convert_format(media_hal_format_t media_hal_fmt)
{
    switch (media_hal_fmt) {
    case MEDIA_HAL_I2S_NORMAL:
        return PT8311_I2S_FMT_I2S;
    case MEDIA_HAL_I2S_LEFT:
        return PT8311_I2S_FMT_LEFT_J;
    /* 不支持 */
    // case MEDIA_HAL_I2S_RIGHT:
    //     return PT8311_I2S_FMT_RIGHT_JUSTIFIED;
    case MEDIA_HAL_I2S_DSP:
        return PT8311_I2S_FMT_PCM_A;
    default:
        return PT8311_I2S_FMT_I2S;
    }
}

static PT8311_I2S_WordLenETypeDef convert_word_len(media_hal_bit_length_t bits)
{
    switch (bits) {
    case 16:
        return PT8311_I2S_WORD_LEN_16bit;
    case 24:
        return PT8311_I2S_WORD_LEN_24bit;
    case 32:
        return PT8311_I2S_WORD_LEN_32bit;
    default:
        return PT8311_I2S_WORD_LEN_16bit;
    }
}

static PT8311_UserSmpRateETypeDef convert_sample_rate(uint32_t rate)
{
    switch (rate) {
    case 8000:
        return PT8311_USER_SR_8K;
    case 11025:
        return PT8311_USER_SR_11025;
    case 12000:
        return PT8311_USER_SR_12K;
    case 16000:
        return PT8311_USER_SR_16K;
    case 22050:
        return PT8311_USER_SR_22050;
    case 24000:
        return PT8311_USER_SR_24K;
    case 32000:
        return PT8311_USER_SR_32K;
    case 44100:
        return PT8311_USER_SR_44100;
    case 48000:
        return PT8311_USER_SR_48K;
    /* 不支持 */
    // case 64000:
    //     return PT8311_USER_SR_64K;
    case 88200:
        return PT8311_USER_SR_88200;
    case 96000:
        return PT8311_USER_SR_96K;
    default:
        return PT8311_USER_SR_48K;
    }
}

static PT8311_InitModETypeDef convert_init_mode(media_hal_codec_mode_t mode)
{
    switch (mode) {
    case MEDIA_HAL_CODEC_MODE_ENCODE:
        return PT8311_INIT_MOD_ADC;
    case MEDIA_HAL_CODEC_MODE_DECODE:
        return PT8311_INIT_MOD_DAC;
    case MEDIA_HAL_CODEC_MODE_BOTH:
    default:
        return PT8311_INIT_MOD_ADC_DAC;
    }
}
