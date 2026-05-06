/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
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
 
#include <string.h>
#include "cjc8910.h"
#include "i2c_device.h"

typedef struct 
{
    uint16_t sampleRate;
    uint8_t r8_src;
    uint32_t mclk;

}Cjc8910ClkCfg;

static Cjc8910ClkCfg g_clkCfgTable[] = 
{
    {8000,  0x06,   12288000},
    {11025, 0x18,   11289600},
    {12000, 0x08,   12288000},
    {16000, 0x0a,   12288000},
    {22050, 0x1a,   11289600},
    {24000, 0x1c,   12288000},
    {32000, 0x0c,   12288000},
    {44100, 0x10,   11289600},
    {48000, 0x0,    12288000},
};

uint16_t cjc8910_get_fs(uint16_t sampleRate)
{
    uint8_t i = 0;
    for(i = 0; i < sizeof(g_clkCfgTable)/sizeof(Cjc8910ClkCfg); i++)
    {
        if(g_clkCfgTable[i].sampleRate == sampleRate)
        {
            return (g_clkCfgTable[i].mclk / g_clkCfgTable[i].sampleRate);
        }
    }

    if(i >= sizeof(g_clkCfgTable)/sizeof(Cjc8910ClkCfg))
        OS_ASSERT(0);

    return 64;
}

struct CJC8910_reg{
    unsigned char reg_index;
    unsigned char reg_value;
};

#define I2CWRNBYTE_CODEC    cjc8910_iic_write

static I2C_BusDevice g_codecI2c = {
    .addr = CJC8910_IIC_ADDRESS,
};

void cjc8910_i2c_config(uint8_t busNo)
{
    static bool isRegister = false;

    if(!isRegister)
    {
        isRegister = true;
        I2C_BusDevice_Register(busNo, &g_codecI2c);
    }
}

void cjc8910_i2c_deconfig(uint8_t busNo)
{
}

int cjc8910_write_reg(uint8_t reg_addr, uint8_t data)
{ 
    uint8_t regConfig[2] = {0};

    regConfig[0] = reg_addr;
    regConfig[1] = data;

    I2C_BusLock(&g_codecI2c);
    I2C_BusDevice_Write(&g_codecI2c, regConfig, 2);
    I2C_BusUnlock(&g_codecI2c);
    return 0;
}

int cjc8910_read_reg(uint8_t reg_addr)
{
    uint8_t data = 0;

    I2C_BusLock(&g_codecI2c);
    I2C_BusDevice_ReadReg(&g_codecI2c, reg_addr, &data, 1);
    I2C_BusUnlock(&g_codecI2c);

    return (int)data;
}
void cjc8910_iic_write(uint8_t reg, uint8_t val)
{
	cjc8910_write_reg(reg,val);
}

/*
* set cjc8910 dac mute or not
* if mute = 0, dac un-mute
* if mute = 1, dac mute
*/
static void cjc8910_mute(int mute)
{
    uint8_t regv = 0;
    Print_Info("Enter into cjc8910_mute(), mute = %d\r\n", mute);
    regv = cjc8910_read_reg(CJC8910_R5_ADC_DAC_CONTROL) & 0xff;
    if (mute) {
        /* Turning the DAC off results in click noise */
        cjc8910_write_reg(CJC8910_R5_ADC_DAC_CONTROL, regv | 0x8);
    } else {
        cjc8910_write_reg(CJC8910_R5_ADC_DAC_CONTROL, regv & 0xf7);
    }
}


/*
* set cjc8910 into suspend mode
*/
static void cjc8910_suspend(void)
{
    Print_Info("CJC8910 suspend\r\n");
   	cjc8910_write_reg(CJC8910_R25_PWR_MGMT1_H , 0xC0);
	cjc8910_write_reg(CJC8910_R26_PWR_MGMT2_H , 0x0);
}

/*
* enable pa power
*/
void cjc8910_pa_power(uint8_t enable)
{

}

int cjc8910_codec_config_format(media_hal_codec_mode_t mode, media_hal_format_t fmt)
{
    int ret = 0;
	uint8_t iface = 0;
    iface = cjc8910_read_reg(CJC8910_R7_AUDIO_INTERFACE);

    switch (fmt) {
        case MEDIA_HAL_I2S_NORMAL:
            Print_Info("CJC8910 in I2S Format\r\n");
            iface &= 0xFC;
			iface |= 0x2;
            break;
        case MEDIA_HAL_I2S_LEFT:
        case MEDIA_HAL_I2S_RIGHT:
            Print_Info("CJC8910 in Left Justified Format\r\n");
            iface &= 0xFC;
			iface |= 0x1;
            break;
        case MEDIA_HAL_I2S_DSP:
            Print_Info("CJC8910 in DSP-A Format\r\n");
            iface &= 0xFC;
			iface |= 0x3;
            break;
        default:
            Print_Info("Unsupported I2S format. Setting to default I2S format\r\n");
            iface &= 0xFC;
            break;
    }
    ret |= cjc8910_write_reg(CJC8910_R7_AUDIO_INTERFACE, iface);

    return ret;
}



static struct CJC8910_reg    cjc8910_dac_to_out[] = {
#if 0
    {CJC8910_R15_RESET,   0x00},   // reset
    {CJC8910_R0_LEFT_INPUT_VOLUME, 0x3F},
    {CJC8910_R2_LOUT1_VOLUME , 0x79},
    {CJC8910_R5_ADC_DAC_CONTROL , 0x00},   // ADC and DAC CONTROL; R/W
    {CJC8910_R7_AUDIO_INTERFACE , 0x0A},   // Digital Audio interface format; R/W
    {CJC8910_R8_SAMPLE_RATE , 0x00},   // Clock and Sample rate control; R/W
#endif
    {CJC8910_R10_LEFT_DAC_VOLUME , 0xC3},   // Left channel DAC digital volume; R/W
    {CJC8910_R12_BASS_CONTROL , 0x0f},   // BASS control; R/W
    {CJC8910_R13_TREBLE_CONTROL , 0x0f},   // Treble control; R/W
    {CJC8910_R17_ALC1_CONTROL , 0x7B},   // ALC1 control; R/W
    {CJC8910_R18_ALC2_CONTROL , 0x00},   // ACL2 control; R/W
    {CJC8910_R19_ALC3_CONTROL , 0x00},   // ALC3 control; R/W
    {CJC8910_R20_NOISE_GATE_CONTROL , 0x00},   // noise gate; R/W
    {CJC8910_R21_LEFT_ADC_VOLUME , 0xc3},   // Left ADC digital volume; R/W
    {CJC8910_R23_ADDITIONAL_CONTROL1 , 0x00},   // Additional control 1; R/W
    {CJC8910_R24_ADDITIONAL_CONTROL2 , 0x00},   // Additional control 2; R/W
    {CJC8910_R27_ADDITIONAL_CONTROL3 , 0x00},   // Additional control 3; R/W
    {CJC8910_R32_ADCL_SIGNAL_PATH , 0x00},   //  ADC signal path control; R/W
    {CJC8910_R33_MIC , 0x0A},   // micR/W   MIX
    {CJC8910_R34_AUX , 0x0A},   // aux R/W   MIX
    {CJC8910_R35_LEFT_OUT_MIX2_H , 0x00},   //  Left out Mix (2) ; R/W
    {CJC8910_R37_ADC_PDN , 0x08},   // Adc_pdn sel; R/W
    {CJC8910_R67_LOW_POWER_PLAYBACK , 0x08},   // Low power playback; R/W
    {CJC8910_R25_PWR_MGMT1_H , 0xE8},   // Power management1 and VMIDSEL; R/W
    {CJC8910_R26_PWR_MGMT2_H , 0x40},   // Power management2 and DAC left power down; R/W;
};

void cjc8910_init(uint16_t sampleRate)
{
    int i, val = 0;

    cjc8910_write_reg(CJC8910_R15_RESET,   0x00);   // reset
    cjc8910_write_reg(CJC8910_R0_LEFT_INPUT_VOLUME , 0x3F);   // Audio input left channel volume; R/W
    cjc8910_write_reg(CJC8910_R2_LOUT1_VOLUME , 0x79);   // Audio output letf channel1 volume; R/W
    cjc8910_write_reg(CJC8910_R5_ADC_DAC_CONTROL , 0x00);   // ADC and DAC CONTROL; R/W
    cjc8910_write_reg(CJC8910_R7_AUDIO_INTERFACE , 0x0A);   // Digital Audio interface format; R/W

    for(uint8_t i = 0; i < sizeof(g_clkCfgTable) / sizeof(Cjc8910ClkCfg); i++)
    {
        if(g_clkCfgTable[i].sampleRate == sampleRate)
        {
            val = (g_clkCfgTable[i].r8_src << 1);
        }
    }
 
    cjc8910_write_reg(CJC8910_R8_SAMPLE_RATE , val);   // Clock and Sample rate control; R/W
#if 0
    cjc8910_write_reg(CJC8910_R10_LEFT_DAC_VOLUME , 0xC3);   // Left channel DAC digital volume; R/W

    cjc8910_write_reg(CJC8910_R12_BASS_CONTROL , 0x0f);   // BASS control; R/W
    cjc8910_write_reg(CJC8910_R13_TREBLE_CONTROL , 0x0f);   // Treble control; R/W

    cjc8910_write_reg(CJC8910_R17_ALC1_CONTROL , 0x00);   // ALC1 control; R/W
    cjc8910_write_reg(CJC8910_R18_ALC2_CONTROL , 0x00);   // ACL2 control; R/W
    cjc8910_write_reg(CJC8910_R19_ALC3_CONTROL , 0x00);   // ALC3 control; R/W
    cjc8910_write_reg(CJC8910_R20_NOISE_GATE_CONTROL , 0x00);   // noise gate; R/W
    cjc8910_write_reg(CJC8910_R21_LEFT_ADC_VOLUME , 0xc3);   // Left ADC digital volume; R/W

    cjc8910_write_reg(CJC8910_R23_ADDITIONAL_CONTROL1 , 0x00);   // Additional control 1; R/W
    cjc8910_write_reg(CJC8910_R24_ADDITIONAL_CONTROL2 , 0x00);   // Additional control 2; R/W
    cjc8910_write_reg(CJC8910_R27_ADDITIONAL_CONTROL3 , 0x00);   // Additional control 3; R/W

    cjc8910_write_reg(CJC8910_R32_ADCL_SIGNAL_PATH , 0x00);   //  ADC signal path control; R/W
    cjc8910_write_reg(CJC8910_R33_MIC , 0x0A);   // micR/W   MIX

    cjc8910_write_reg(CJC8910_R34_AUX , 0x0A);   // aux R/W   MIX
    cjc8910_write_reg(CJC8910_R35_LEFT_OUT_MIX2_H , 0x00);   //  Left out Mix (2) ; R/W

    cjc8910_write_reg(CJC8910_R37_ADC_PDN , 0x08);   // Adc_pdn sel; R/W

    cjc8910_write_reg(CJC8910_R67_LOW_POWER_PLAYBACK , 0x08);   // Low power playback; R/W

    cjc8910_write_reg(CJC8910_R25_PWR_MGMT1_H , 0xE8);   // Power management1 and VMIDSEL; R/W
    cjc8910_write_reg(CJC8910_R26_PWR_MGMT2_H , 0x40);   // Power management2 and DAC left power down; R/W;
#else
    for(i = 0; i < sizeof(cjc8910_dac_to_out) / sizeof(struct CJC8910_reg) ; i++)
    {
        cjc8910_write_reg(cjc8910_dac_to_out[i].reg_index, cjc8910_dac_to_out[i].reg_value);
    }
#endif
}

int cjc8910_codec_powerup()
{
//TODO
    cjc8910_pa_power(1);
    return 0;
}

int cjc8910_codec_powerdown()
{
//TODO
    cjc8910_pa_power(0);
    return 0;
}

int cjc8910_codec_deinit(void)
{
//TODO
    return 0;
}


int cjc8910_codec_set_bits_per_sample(media_hal_codec_mode_t mode, media_hal_bit_length_t bits_per_sample)
{
    int ret = 0;
    uint8_t iface = 0;
    iface = cjc8910_read_reg(CJC8910_R7_AUDIO_INTERFACE) & 0xf3;
    switch (bits_per_sample) {
        case MEDIA_HAL_BIT_LENGTH_16BITS:
            iface |= 0x0;
            break;
        case MEDIA_HAL_BIT_LENGTH_24BITS:
			iface |= 0x8;
            break;
        case MEDIA_HAL_BIT_LENGTH_32BITS:
            iface |= 0xc;
            break;
        default:
            Print_Info("Unsupported bit per sample. Setting to default 16 bits\r\n");
            iface |= 0x0;
            break;

    }
    ret = cjc8910_write_reg(CJC8910_R7_AUDIO_INTERFACE, iface);

    return ret;
}

/*
int cjc8910_codec_config_i2s(media_hal_codec_mode_t mode, media_hal_bit_length_t iface)
{
    int ret = 0;
    ret |= cjc8910_set_bits_per_sample(bits);
    ret |= cjc8910_config_format(fmt);
    return ret;
}
*/

int cjc8910_codec_ctrl_state(media_hal_codec_mode_t mode, media_hal_sel_state_t ctrl_state)
{
    int ret = 0;
  

    return ret;
}

int cjc8910_codec_start(int mode)
{
    int ret = 0;
    uint8_t adc_iface = 0, dac_iface = 0;

    /* unmute */
    dac_iface = cjc8910_read_reg(CJC8910_R5_ADC_DAC_CONTROL)  & 0xff;
    adc_iface = cjc8910_read_reg(CJC8910_R0_LEFT_INPUT_VOLUME-1) & 0xff;

    if (mode == MEDIA_HAL_CODEC_MODE_LINE_IN) {
        Print_Err("The codec cjc8910 doesn't support ES_MODULE_LINE mode\r\n");
        return -1;
    }
    if (mode == MEDIA_HAL_CODEC_MODE_ENCODE || mode == MEDIA_HAL_CODEC_MODE_BOTH) 
    {
        adc_iface &= ~(1 << 7);
    }
    if (mode == MEDIA_HAL_CODEC_MODE_DECODE || mode == MEDIA_HAL_CODEC_MODE_BOTH) 
    {
        dac_iface &= ~(1 << 3);
    }

    ret |= cjc8910_write_reg(CJC8910_R5_ADC_DAC_CONTROL, dac_iface);
    ret |= cjc8910_write_reg(CJC8910_R0_LEFT_INPUT_VOLUME, adc_iface);

    /*  */
    ret |= cjc8910_write_reg(CJC8910_R25_PWR_MGMT1_H , 0xE8);   // Power management1 and VMIDSEL; R/W
    ret |= cjc8910_write_reg(CJC8910_R26_PWR_MGMT2_H , 0x40);   // Power management2 and DAC left power down; R/W;

   

    return ret;
}


int cjc8910_codec_mic_selection(int val)
{
    int ret = 0;	
    

    return ret;
}

int cjc8910_codec_mic_loopback(void)
{
    int ret = 0;

    

    return ret;
}

int cjc8910_codec_stop(int mode)
{
    cjc8910_suspend();
    return 0;
}

int cjc8910_codec_set_volume(int volume)
{
    int ret = 0;
    if (volume <= 0) {
        return cjc8910_codec_set_mute(1);
    } 
    if (volume > 100) {
        volume = 100;
    }

    int vol = (volume) * 2550 / 1000;
    Print_Info("CJC8910 volume:%d\r\n", volume);
    ret = cjc8910_write_reg(CJC8910_R10_LEFT_DAC_VOLUME, vol);
    return ret;
}

int cjc8910_codec_get_volume(int *volume)
{
    int ret = 0;
    int regv = 0;
    regv = cjc8910_read_reg(CJC8910_R10_LEFT_DAC_VOLUME-1);
    if (regv == -1) {
        *volume = 0;
        ret = -1;
    } else {
        *volume = regv * 100 / 256;
    }
    Print_Info("GET: res:%d, volume:%d\r\n", regv, *volume);
    return ret;
}

int cjc8910_codec_set_mute(int enable)
{
    cjc8910_mute(enable);
    return 0;
}

int cjc8910_codec_get_mute(int *mute)
{
    int ret = -1;
    uint8_t reg = 0;
    ret = cjc8910_read_reg(CJC8910_R5_ADC_DAC_CONTROL);
    if (ret != -1) {
        reg = (ret & 0x8) >> 3;
    }
    *mute = reg;
    return ret;
}



int cjc8910_codec_set_adc_digit_gain(int32_t db)
{
    int res = 0;
    uint8_t regVal = 0;
    regVal = (db - (-9750)) / 50;
    res = cjc8910_write_reg(CJC8910_R21_LEFT_ADC_VOLUME, regVal); 
    return res; 
}

int cjc8910_codec_get_adc_digit_gain(uint8_t *db, uint8_t *len)
{
    int res = 0;
    int32_t dbVal = 0;
    
    res = cjc8910_read_reg(CJC8910_R21_LEFT_ADC_VOLUME-1); 
    if(res >= 0)
    {
        dbVal = (int32_t)(res * 50 + (-9750));
        db[0] = (dbVal >> 24) & 0xff;
        db[1] = (dbVal >> 16) & 0xff;
        db[2] = (dbVal >> 8) & 0xff;
        db[3] = (dbVal) & 0xff;
        *len = 4;
    }
        
    return res; 
}

int cjc8910_codec_set_dac_digit_gain(int32_t db)
{
    int res = 0;
    uint8_t regVal = 0;
    regVal = (db - (-9750)) / 50;

    // os_kprintf("regVal : %d \r\n", regVal);

    //if(regVal <= 0xBF)      // 0db
    res = cjc8910_write_reg(CJC8910_R10_LEFT_DAC_VOLUME, regVal); 
    return res; 
}

int cjc8910_codec_get_dac_digit_gain(uint8_t *db, uint8_t *len)
{
    int res = 0;
    int32_t dbVal = 0;

    res = cjc8910_read_reg(CJC8910_R10_LEFT_DAC_VOLUME-1); 
    if(res >= 0)
    {
        dbVal = (int32_t)(res * 50 + (-9750));
        db[0] = (dbVal >> 24) & 0xff;
        db[1] = (dbVal >> 16) & 0xff;
        db[2] = (dbVal >> 8) & 0xff;
        db[3] = (dbVal) & 0xff;
        *len = 4;
    }
        
    return res; 
}

int cjc8910_codec_set_adc_pga_gain(int32_t db)
{
    int res = 0;
    uint8_t regVal = 0;
    regVal = cjc8910_read_reg(CJC8910_R0_LEFT_INPUT_VOLUME-1);

    regVal = (regVal & 0xC0) | ((db / 300) & 0x3F);
    res = cjc8910_write_reg(CJC8910_R0_LEFT_INPUT_VOLUME, regVal); 
    return res; 
}

int cjc8910_codec_get_adc_pga_gain(uint8_t *db, uint8_t *len)
{
    int res = 0;
    int32_t dbVal = 0;
    
    res = cjc8910_read_reg(CJC8910_R0_LEFT_INPUT_VOLUME-1); 
    if(res >= 0)
    {
        dbVal = (res & 0x3f) * 300;
        db[0] = (dbVal >> 24) & 0xff;
        db[1] = (dbVal >> 16) & 0xff;
        db[2] = (dbVal >> 8) & 0xff;
        db[3] = (dbVal) & 0xff;
        *len = 4;
    }
        
    return res; 
}


int cjc8910_codec_set_mic_gain(int32_t db)
{
    int res = 0;
    // uint8_t regVal = 0;
	/*
    regVal = cjc8910_read_reg(CJC8910_ADC_REG16);
    
    regVal = (regVal & 0xF8) | ((db / 600) & 0x07);
    res = cjc8910_write_reg(CJC8910_ADC_REG16, regVal); */
    return res; 
}

int cjc8910_codec_get_mic_gain(uint8_t *db, uint8_t *len)
{
    int res = 0;
    // int32_t dbVal = 0;
    /*
    res = cjc8910_read_reg(CJC8910_ADC_REG16); 
    if(res >= 0)
    {
        dbVal = (res & 0x07) * 600;
        db[0] = (dbVal >> 24) & 0xff;
        db[1] = (dbVal >> 16) & 0xff;
        db[2] = (dbVal >> 8) & 0xff;
        db[3] = (dbVal) & 0xff;
        *len = 4;
    }
     */
    return res; 
}


int cjc8910_codec_set_nr_gate(int32_t db)
{
    int res = 0;
    // uint8_t regVal = 0;
	/*
    regVal = cjc8910_read_reg(CJC8910_ADC_REG1A);
    
    if(db <= -5400)
        regVal = (regVal & 0xF0) | (((db - (-9600)) / 600) & 0x0F);
    else
        regVal = (regVal & 0xF0) | ((((db - (-5400)) / 300) & 0x0F) + 7);

    res = cjc8910_write_reg(CJC8910_ADC_REG1A, regVal); */
    return res; 
}

int cjc8910_codec_get_nr_gate(uint8_t *db, uint8_t *len)
{
    int res = 0;
    // int32_t dbVal = 0;
    /*
    res = cjc8910_read_reg(CJC8910_ADC_REG1A); 
    if(res >= 0)
    {
        if((res & 0x0F) <= 7)
            dbVal = (res & 0x0F) * 600 + (-9600);
        else
            dbVal = ((res & 0x0F) - 7) * 300 + (-5400);
        db[0] = (dbVal >> 24) & 0xff;
        db[1] = (dbVal >> 16) & 0xff;
        db[2] = (dbVal >> 8) & 0xff;
        db[3] = (dbVal) & 0xff;
        *len = 4;
    }
     */   
    return res; 
}


int cjc8910_codec_set_adc_eq(uint8_t *para, uint8_t len)
{
   /* if(para[45] == 0)
        return 0;

    for(uint8_t i = 0; i < len / 2; i++)
    {
        cjc8910_write_reg(para[i*2], para[i*2+1]);
    }*/
    return 0;
}

int cjc8910_codec_get_adc_eq(uint8_t *para, uint8_t *len)
{
    // uint8_t i = 0;
    // uint8_t reg_addr = 0;
	/*
    if(cjc8910_read_reg(CJC8910_ADC_REG1C) & 0x40)
    {
        para[i++] = CJC8910_ADC_REG1C;   para[i++] = 0;

        for(reg_addr = 0x1D; reg_addr <= 0x30; reg_addr ++)
        {
            para[i++] = reg_addr;               
            para[i++] = 0;
        }

        para[i++] = CJC8910_ADC_REG1B;   para[i++] = 0;
        para[i++] = CJC8910_ADC_REG1C;   para[i++] = 0;
        *len = i;   
    }
    else
    {
        para[i++] = CJC8910_ADC_REG1C;   para[i++] = (cjc8910_read_reg(CJC8910_ADC_REG1C) & 0x3f) | 0x40;

        for(reg_addr = 0x1D; reg_addr <= 0x30; reg_addr ++)
        {
            para[i++] = reg_addr;               
            para[i++] = cjc8910_read_reg(reg_addr) & 0xff;
        }

        para[i++] = CJC8910_ADC_REG1B;   para[i++] = cjc8910_read_reg(CJC8910_ADC_REG1B);
        para[i++] = CJC8910_ADC_REG1C;   para[i++] = (cjc8910_read_reg(CJC8910_ADC_REG1C) & 0x3f);
        *len = i;
    }*/

    return 0;
}

int cjc8910_codec_set_dac_eq(uint8_t *para, uint8_t len)
{
    if(para[31] == 0)
        return 0;

    for(uint8_t i = 0; i < len / 2; i++)
    {
        cjc8910_write_reg(para[i*2], para[i*2+1]);
    }
    return 0;
}

int cjc8910_codec_get_dac_eq(uint8_t *para, uint8_t *len)
{
    // uint8_t i = 0;
    // uint8_t reg_addr = 0;
	/*
    if(cjc8910_read_reg(CJC8910_DAC_REG37) & 0x08)
    {
        para[i++] = CJC8910_DAC_REG31;   para[i++] = 0;
        para[i++] = CJC8910_DAC_REG37;   para[i++] = 0;

        for(reg_addr = 0x38; reg_addr <= 0x43; reg_addr ++)
        {
            para[i++] = reg_addr;               
            para[i++] = 0;
        }

        para[i++] = CJC8910_DAC_REG37;   para[i++] = 0;
        para[i++] = CJC8910_DAC_REG31;   para[i++] = 0;
        *len = i;
    }
    else
    {
        para[i++] = CJC8910_DAC_REG31;   para[i++] = cjc8910_read_reg(CJC8910_DAC_REG31) | 0x60;
        para[i++] = CJC8910_DAC_REG37;   para[i++] = cjc8910_read_reg(CJC8910_DAC_REG37) | 0x08;

        for(reg_addr = 0x38; reg_addr <= 0x43; reg_addr ++)
        {
            para[i++] = reg_addr;               
            para[i++] = cjc8910_read_reg(reg_addr) & 0xff;
        }

        para[i++] = CJC8910_DAC_REG37;   para[i++] = cjc8910_read_reg(CJC8910_DAC_REG37) & (~0x08);
        para[i++] = CJC8910_DAC_REG31;   para[i++] = cjc8910_read_reg(CJC8910_DAC_REG31) & (~0x60);
        *len = i;
    }

	*/
    return 0;
}

int cjc8910_codec_set_drc(uint8_t *para, uint8_t len)
{
	/*
    if(para[9] == 0)
        return 0;

    for(uint8_t i = 0; i < len / 2; i++)
    {
        cjc8910_write_reg(para[i*2], para[i*2+1]);
    }*/
    return 0; 
}

int cjc8910_codec_get_drc(uint8_t *para, uint8_t *len)
{
    // uint8_t i = 0;
	/*
    if((uint8_t)(CJC8910_DAC_REG34) & 0x80)
    {
        para[i++] = CJC8910_DAC_REG34;   para[i++] = cjc8910_read_reg(CJC8910_DAC_REG34) & 0x7F;
        para[i++] = CJC8910_DAC_REG35;   para[i++] = cjc8910_read_reg(CJC8910_DAC_REG35) & 0xFF;
        para[i++] = CJC8910_DAC_REG37;   para[i++] = cjc8910_read_reg(CJC8910_DAC_REG37) & 0xFF;
        para[i++] = CJC8910_DAC_REG32;   para[i++] = cjc8910_read_reg(CJC8910_DAC_REG32) & 0xFF;
        para[i++] = CJC8910_DAC_REG34;   para[i++] = cjc8910_read_reg(CJC8910_DAC_REG34) | 0x80;

        *len = i;
    }
    else
    {
        para[i++] = CJC8910_DAC_REG34;   para[i++] = 0;
        para[i++] = CJC8910_DAC_REG35;   para[i++] = 0;
        para[i++] = CJC8910_DAC_REG37;   para[i++] = 0;
        para[i++] = CJC8910_DAC_REG32;   para[i++] = 0;
        para[i++] = CJC8910_DAC_REG34;   para[i++] = 0;

        *len = i;
    }*/
    return 0;
}

int cjc8910_codec_set_alc(uint8_t *para, uint8_t len)
{
    if(para[11] == 0)
        return 0;
	/*
    for(uint8_t i = 0; i < len / 2; i++)
    {
        cjc8910_write_reg(para[i*2], para[i*2+1]);
    }*/
    cjc8910_write_reg(CJC8910_R17_ALC1_CONTROL, 0x7b);
	cjc8910_write_reg(CJC8910_R18_ALC2_CONTROL, 0);
	cjc8910_write_reg(CJC8910_R19_ALC3_CONTROL, 0);
    return 0;
}

int cjc8910_codec_get_alc(uint8_t *para, uint8_t *len)
{
    // uint8_t i = 0;
	/*
    if((uint8_t)cjc8910_read_reg(CJC8910_ADC_REG18) & 0x80)
    {
        para[i++] = CJC8910_ADC_REG18;   para[i++] = cjc8910_read_reg(CJC8910_ADC_REG18) & 0x7F;
        para[i++] = CJC8910_ADC_REG19;   para[i++] = cjc8910_read_reg(CJC8910_ADC_REG19) & 0xFF;
        para[i++] = CJC8910_ADC_REG15;   para[i++] = cjc8910_read_reg(CJC8910_ADC_REG15) & 0xFF;
        para[i++] = CJC8910_ADC_REG1B;   para[i++] = cjc8910_read_reg(CJC8910_ADC_REG1B) & 0xFF;
        para[i++] = CJC8910_ADC_REG17;   para[i++] = cjc8910_read_reg(CJC8910_ADC_REG17) & 0xFF;
        para[i++] = CJC8910_ADC_REG18;   para[i++] = cjc8910_read_reg(CJC8910_ADC_REG18) | 0x80;
        *len = i;
    }
    else
    {
        para[i++] = CJC8910_ADC_REG18;   para[i++] = 0;
        para[i++] = CJC8910_ADC_REG19;   para[i++] = 0;
        para[i++] = CJC8910_ADC_REG15;   para[i++] = 0;
        para[i++] = CJC8910_ADC_REG1B;   para[i++] = 0;
        para[i++] = CJC8910_ADC_REG17;   para[i++] = 0;
        para[i++] = CJC8910_ADC_REG18;   para[i++] = 0;
        *len = i;
    }*/

    return 0;
}

int cjc8910_codec_get_side_tone(uint8_t *side_tone, uint8_t *len)
{
    int res = 0;

    res = cjc8910_read_reg(CJC8910_R33_MIC); 
    if(res >= 0)
    {
        *side_tone = (res >> 4) & 0x01;
        *len = 1;
    }

    return res;
}

int cjc8910_codec_set_side_tone(uint8_t side_tone)
{
    int res = 0;
    uint8_t regVal = 0;

    regVal = cjc8910_read_reg(CJC8910_R33_MIC);

    if(side_tone)
    {
        regVal = (1 << 4);
    }
    else
        regVal &= ~(1 << 4);
    
    res = cjc8910_write_reg(CJC8910_R33_MIC, regVal);

    return res;
}

int cjc8910_codec_get_id(uint8_t *id, uint8_t *len)
{
   /* id[2] = cjc8910_read_reg(CJC8910_CHD1_REGFD); 
    id[3] = cjc8910_read_reg(CJC8910_CHD2_REGFE); 
    id[0] = 0;
    id[1] = 0;*/
    *len = 0;

    return 0; 
}

static void CJC8910_off(void)
{
    cjc8910_write_reg(CJC8910_R5_ADC_DAC_CONTROL, 8);
   // aic_udelay(1000);
   // snd_ext_pa_off();
    cjc8910_write_reg(CJC8910_R25_PWR_MGMT1_L, 0);
    cjc8910_write_reg(CJC8910_R26_PWR_MGMT2_L, 0);
}


void cjc8910_dump_reg(void)
{
    Print_Info("CJC8910_R15_RESET %x : 0x%04x \r\n",CJC8910_R15_RESET, cjc8910_read_reg(CJC8910_R15_RESET) + ((cjc8910_read_reg(CJC8910_R15_RESET + 1) &0x01) << 8));
    Print_Info("CJC8910_R0_LEFT_INPUT_VOLUME %x : 0x%04x \r\n",CJC8910_R0_LEFT_INPUT_VOLUME, ((cjc8910_read_reg(CJC8910_R0_LEFT_INPUT_VOLUME) &0x01) << 8) + cjc8910_read_reg(CJC8910_R0_LEFT_INPUT_VOLUME-1));
    Print_Info("CJC8910_R2_LOUT1_VOLUME %x : 0x%04x \r\n",CJC8910_R2_LOUT1_VOLUME, ((cjc8910_read_reg(CJC8910_R2_LOUT1_VOLUME) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R2_LOUT1_VOLUME-1));
    Print_Info("CJC8910_R5_ADC_DAC_CONTROL %x : 0x%04x \r\n",CJC8910_R5_ADC_DAC_CONTROL, ((cjc8910_read_reg(CJC8910_R5_ADC_DAC_CONTROL + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R5_ADC_DAC_CONTROL));
    Print_Info("CJC8910_R7_AUDIO_INTERFACE %x : 0x%04x \r\n",CJC8910_R7_AUDIO_INTERFACE,((cjc8910_read_reg(CJC8910_R7_AUDIO_INTERFACE + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R7_AUDIO_INTERFACE));
    Print_Info("CJC8910_R8_SAMPLE_RATE %x : 0x%04x \r\n",CJC8910_R8_SAMPLE_RATE, ((cjc8910_read_reg(CJC8910_R8_SAMPLE_RATE + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R8_SAMPLE_RATE));
    Print_Info("CJC8910_R10_LEFT_DAC_VOLUME %x : 0x%04x \r\n",CJC8910_R10_LEFT_DAC_VOLUME, ((cjc8910_read_reg(CJC8910_R10_LEFT_DAC_VOLUME) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R10_LEFT_DAC_VOLUME - 1));
    Print_Info("CJC8910_R12_BASS_CONTROL %x : 0x%04x \r\n",CJC8910_R12_BASS_CONTROL, ((cjc8910_read_reg(CJC8910_R12_BASS_CONTROL + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R12_BASS_CONTROL));
    Print_Info("CJC8910_R13_TREBLE_CONTROL %x : 0x%04x \r\n",CJC8910_R13_TREBLE_CONTROL, ((cjc8910_read_reg(CJC8910_R13_TREBLE_CONTROL + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R13_TREBLE_CONTROL));
    Print_Info("CJC8910_R17_ALC1_CONTROL %x : 0x%04x \r\n",CJC8910_R17_ALC1_CONTROL, ((cjc8910_read_reg(CJC8910_R17_ALC1_CONTROL + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R17_ALC1_CONTROL));
    Print_Info("CJC8910_R18_ALC2_CONTROL %x : 0x%04x \r\n",CJC8910_R18_ALC2_CONTROL, ((cjc8910_read_reg(CJC8910_R18_ALC2_CONTROL + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R18_ALC2_CONTROL));
    Print_Info("CJC8910_R19_ALC3_CONTROL %x : 0x%04x \r\n",CJC8910_R19_ALC3_CONTROL, ((cjc8910_read_reg(CJC8910_R19_ALC3_CONTROL + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R19_ALC3_CONTROL));
    Print_Info("CJC8910_R20_NOISE_GATE_CONTROL %x : 0x%04x \r\n",CJC8910_R20_NOISE_GATE_CONTROL, ((cjc8910_read_reg(CJC8910_R20_NOISE_GATE_CONTROL + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R20_NOISE_GATE_CONTROL));
    Print_Info("CJC8910_R21_LEFT_ADC_VOLUME %x : 0x%04x \r\n",CJC8910_R21_LEFT_ADC_VOLUME, ((cjc8910_read_reg(CJC8910_R21_LEFT_ADC_VOLUME) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R21_LEFT_ADC_VOLUME - 1));
    Print_Info("CJC8910_R23_ADDITIONAL_CONTROL1 %x : 0x%04x \r\n",CJC8910_R23_ADDITIONAL_CONTROL1, ((cjc8910_read_reg(CJC8910_R23_ADDITIONAL_CONTROL1 + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R23_ADDITIONAL_CONTROL1));
    Print_Info("CJC8910_R24_ADDITIONAL_CONTROL2 %x : 0x%04x \r\n",CJC8910_R24_ADDITIONAL_CONTROL2, ((cjc8910_read_reg(CJC8910_R24_ADDITIONAL_CONTROL2 + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R24_ADDITIONAL_CONTROL2));
    Print_Info("CJC8910_R27_ADDITIONAL_CONTROL3 %x : 0x%04x \r\n",CJC8910_R27_ADDITIONAL_CONTROL3, ((cjc8910_read_reg(CJC8910_R27_ADDITIONAL_CONTROL3) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R27_ADDITIONAL_CONTROL3));
    Print_Info("CJC8910_R32_ADCL_SIGNAL_PATH %x : 0x%04x \r\n",CJC8910_R32_ADCL_SIGNAL_PATH, ((cjc8910_read_reg(CJC8910_R32_ADCL_SIGNAL_PATH) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R32_ADCL_SIGNAL_PATH -1));
    Print_Info("CJC8910_R33_MIC %x : 0x%04x \r\n",CJC8910_R33_MIC, ((cjc8910_read_reg(CJC8910_R33_MIC + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R33_MIC));
    Print_Info("CJC8910_R34_AUX %x : 0x%04x \r\n",CJC8910_R34_AUX, ((cjc8910_read_reg(CJC8910_R34_AUX + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R34_AUX));
    Print_Info("CJC8910_R35_LEFT_OUT_MIX2_L %x : %02x \r\n",CJC8910_R35_LEFT_OUT_MIX2_L, cjc8910_read_reg(CJC8910_R35_LEFT_OUT_MIX2_L));
    Print_Info("CJC8910_R35_LEFT_OUT_MIX2_H %x : %02x \r\n",CJC8910_R35_LEFT_OUT_MIX2_H, cjc8910_read_reg(CJC8910_R35_LEFT_OUT_MIX2_H));
    Print_Info("CJC8910_R37_ADC_PDN %x : 0x%04x \r\n",CJC8910_R37_ADC_PDN, ((cjc8910_read_reg(CJC8910_R37_ADC_PDN + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R37_ADC_PDN) );
    Print_Info("CJC8910_R67_LOW_POWER_PLAYBACK %x : 0x%04x \r\n",CJC8910_R67_LOW_POWER_PLAYBACK, ((cjc8910_read_reg(CJC8910_R67_LOW_POWER_PLAYBACK + 1) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R67_LOW_POWER_PLAYBACK));
    Print_Info("CJC8910_R25_PWR_MGMT1_H %x : 0x%04x \r\n",CJC8910_R25_PWR_MGMT1_H, ((cjc8910_read_reg(CJC8910_R25_PWR_MGMT1_H) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R25_PWR_MGMT1_H -1));
    Print_Info("CJC8910_R26_PWR_MGMT2_H %x : 0x%04x \r\n",CJC8910_R26_PWR_MGMT2_H, ((cjc8910_read_reg(CJC8910_R26_PWR_MGMT2_H) & 0x01) << 8) + cjc8910_read_reg(CJC8910_R26_PWR_MGMT2_H -1));
}