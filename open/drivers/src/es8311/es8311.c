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
#include "es8311.h"
#include "i2c_device.h"

/*
 * to define the clock soure of MCLK
 */
#define FROM_MCLK_PIN       0
#define FROM_SCLK_PIN       1
#define MCLK_SOURCE         FROM_SCLK_PIN

/*
 * to define whether to reverse the clock
 */
#define INVERT_MCLK         0 // do not invert
#define INVERT_SCLK         0

#define IS_DMIC             0 // Is it a digital microphone

/*
 * Clock coefficient structer
 */
struct _coeff_div {
    uint32_t mclk;        /* mclk frequency */
    uint32_t rate;        /* sample rate */
    uint8_t pre_div;      /* the pre divider with range from 1 to 8 */
    uint8_t pre_multi;    /* the pre multiplier with x1, x2, x4 and x8 selection */
    uint8_t adc_div;      /* adcclk divider */
    uint8_t dac_div;      /* dacclk divider */
    uint8_t fs_mode;      /* double speed or single speed, =0, ss, =1, ds */
    uint8_t lrck_h;       /* adclrck divider and daclrck divider */
    uint8_t lrck_l;
    uint8_t bclk_div;     /* sclk divider */
    uint8_t adc_osr;      /* adc osr */
    uint8_t dac_osr;      /* dac osr */
};

/* codec hifi mclk clock divider coefficients */
static const struct _coeff_div coeff_div[] = {
    //mclk     rate   pre_div  mult  adc_div dac_div fs_mode lrch  lrcl  bckdiv osr
    /* 8k */
    {12288000, 8000 , 0x06, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {18432000, 8000 , 0x03, 0x02, 0x03, 0x03, 0x00, 0x05, 0xff, 0x18, 0x10, 0x20},
    {16384000, 8000 , 0x08, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {8192000 , 8000 , 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {6144000 , 8000 , 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {4096000 , 8000 , 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {3072000 , 8000 , 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {2048000 , 8000 , 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {1536000 , 8000 , 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {1024000 , 8000 , 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {512000 ,  8000 , 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0x3f, 0x04, 0x20, 0x20},

    /* 11.025k */
    {11289600, 11025, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {5644800 , 11025, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {2822400 , 11025, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {1411200 , 11025, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},

    /* 12k */
    {12288000, 12000, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {6144000 , 12000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {3072000 , 12000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {1536000 , 12000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},

    /* 16k */
    {12288000, 16000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x03, 0x10, 0x20},
    {18432000, 16000, 0x03, 0x02, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x20},
    {16384000, 16000, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {8192000 , 16000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {6144000 , 16000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {4096000 , 16000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {3072000 , 16000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {2048000 , 16000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {1536000 , 16000, 0x03, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},
    {1024000 , 16000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x20},

    /* 22.05k */
    {11289600, 22050, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 22050, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 22050, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 22050, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 24k */
    {12288000, 24000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 24000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 24000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 24000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 24000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 32k */
    {12288000, 32000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 32000, 0x03, 0x04, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x10},
    {16384000, 32000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 32000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 32000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 32000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 32000, 0x03, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 32000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 32000, 0x03, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
    {1024000 , 32000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 44.1k */
    {11289600, 44100, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 44100, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 44100, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 44100, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 48k */
    {12288000, 48000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 48000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 48000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 48000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 48000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 64k */
    {12288000, 64000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 64000, 0x03, 0x04, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {16384000, 64000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 64000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 64000, 0x01, 0x04, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {4096000 , 64000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 64000, 0x01, 0x08, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {2048000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0xbf, 0x03, 0x18, 0x18},
    {1024000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 88.2k */
    {11289600, 88200, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 88200, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 88200, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 88200, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 96k */
    {12288000, 96000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 96000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 96000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 96000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 96000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
};

static I2C_BusDevice g_codecI2c = {
    .addr = 0x18,
};

void es8311_i2c_config(uint8_t busNo)
{
    static bool isRegister = false;

    if(!isRegister)
    {
        isRegister = true;
        I2C_BusDevice_Register(busNo, &g_codecI2c);
    }
}

void es8311_i2c_deconfig(uint8_t busNo)
{
}

int es8311_write_reg(uint8_t reg_addr, uint8_t data)
{ 
    uint8_t regConfig[2] = {0};

    regConfig[0] = reg_addr;
    regConfig[1] = data;

    I2C_BusLock(&g_codecI2c);
    I2C_BusDevice_Write(&g_codecI2c, regConfig, 2);
    I2C_BusUnlock(&g_codecI2c);
    return 0;
}

int es8311_read_reg(uint8_t reg_addr)
{
    uint8_t data = 0;

    I2C_BusLock(&g_codecI2c);
    I2C_BusDevice_ReadReg(&g_codecI2c, reg_addr, &data, 1);
    I2C_BusUnlock(&g_codecI2c);

    return (int)data;
}
/*
* look for the coefficient in coeff_div[] table
*/
static int es8311_get_coeff(uint32_t mclk, uint32_t rate)
{
    for (unsigned int i = 0; i < (sizeof(coeff_div) / sizeof(coeff_div[0])); i++) {
        if (coeff_div[i].rate == rate && coeff_div[i].mclk == mclk)
            return (int)i;
    }
    return -1;
}

/*
* set es8311 dac mute or not
* if mute = 0, dac un-mute
* if mute = 1, dac mute
*/
static void es8311_mute(int mute)
{
    uint8_t regv;
    Print_Info("Enter into es8311_mute(), mute = %d\r\n", mute);
    regv = es8311_read_reg(ES8311_DAC_REG31) & 0x9f;
    if (mute) {
        /* Turning the DAC off results in click noise */
        es8311_write_reg(ES8311_SYSTEM_REG12, 0x02);
        es8311_write_reg(ES8311_DAC_REG31, regv | 0x60);
        es8311_write_reg(ES8311_DAC_REG32, 0x00);
        es8311_write_reg(ES8311_DAC_REG37, 0x08);
    } else {
        es8311_write_reg(ES8311_DAC_REG31, regv);
        es8311_write_reg(ES8311_SYSTEM_REG12, 0x00);
    }
}

/*
* set es8311 into suspend mode
*/
static void es8311_suspend(void)
{
    Print_Info("ES8311 suspend\r\n");
    /* ADC & DAC volume */
    es8311_write_reg(ES8311_DAC_REG32, 0x00);
    es8311_write_reg(ES8311_ADC_REG17, 0x00);

    es8311_write_reg(ES8311_SYSTEM_REG0E, 0xFF);
    
    es8311_write_reg(ES8311_SYSTEM_REG12, 0x02);
    es8311_write_reg(ES8311_SYSTEM_REG14, 0x00);

    es8311_write_reg(ES8311_SYSTEM_REG0D, 0xFA);
    es8311_write_reg(ES8311_ADC_REG15, 0x00);
    es8311_write_reg(ES8311_DAC_REG37, 0x08);
    es8311_write_reg(ES8311_GP_REG45, 0x01);
}

/*
* enable pa power
*/
void es8311_pa_power(OS_UNUSED uint8_t enable)
{

}


int es8311_codec_config_format(OS_UNUSED media_hal_codec_mode_t mode, media_hal_format_t fmt)
{
    int ret = 0;

    uint8_t dac_iface = es8311_read_reg(ES8311_SDPIN_REG09);
    uint8_t adc_iface = es8311_read_reg(ES8311_SDPOUT_REG0A);

    switch (fmt) {
        case MEDIA_HAL_I2S_NORMAL:
            Print_Info("ES8311 in I2S Format\r\n");
            dac_iface &= 0xFC;
            adc_iface &= 0xFC;
            break;
        case MEDIA_HAL_I2S_LEFT:
        case MEDIA_HAL_I2S_RIGHT:
            Print_Info("ES8311 in Left Justified Format\r\n");
            adc_iface &= 0xFC;
            dac_iface &= 0xFC;
            adc_iface |= 0x01;
            dac_iface |= 0x01;
            break;
        case MEDIA_HAL_I2S_DSP:
            Print_Info("ES8311 in DSP-A Format\r\n");
            adc_iface &= 0xDC;
            dac_iface &= 0xDC;
            adc_iface |= 0x03;
            dac_iface |= 0x03;
            break;
        default:
            Print_Info("Unsupported I2S format. Setting to default I2S format\r\n");
            dac_iface &= 0xFC;
            adc_iface &= 0xFC;
            break;
    }
    ret |= es8311_write_reg(ES8311_SDPIN_REG09, dac_iface);
    ret |= es8311_write_reg(ES8311_SDPOUT_REG0A, adc_iface);

    return ret;
}


int es8311_codec_init(uint32_t sample_rate, uint32_t mclk, media_hal_op_mode_t es8311_mode)
{
    int ret = 0;
    uint8_t datmp, regv;
    int coeff;

    osPrintf("codec id : %x %x \r\n", es8311_read_reg(ES8311_CHD1_REGFD), es8311_read_reg(ES8311_CHD2_REGFE));
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, 0x00);
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG02, 0x40); // //MCLK DIV=3

    // osPrintf("read reg 02: %x \r\n ", es8311_read_reg(ES8311_CLK_MANAGER_REG02));

    /* clock manager :  ADC single/double speed and over sample rate, this config not uesd */
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG03, 0x10);
    /* not used */
    ret |= es8311_write_reg(ES8311_ADC_REG16, 0x24);
    /* clock manager : DAC over sample */
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG04, 0x10);
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG05, 0x00);

    ret |= es8311_write_reg(ES8311_SYSTEM_REG0B, 0x00);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG0C, 0x00);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG10, 0x03);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG11, 0x56);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG0F, 0x7F); //
    /* CSM Power On */
    ret |= es8311_write_reg(ES8311_RESET_REG00, 0x80);
    
    /*
     * Set master/slave audio interface, The Bit6(MSC) fo register 0X00, Reg0x00.bit[6]
     */
    regv = es8311_read_reg(ES8311_RESET_REG00);
    if (MEDIA_HAL_MODE_MASTER == es8311_mode)
    {
        Print_Info("ES8311 in Master mode\r\n");
        regv |= 0x40;
    } 
    else 
    {
        Print_Info("ES8311 in Slave mode\r\n");
        regv &= 0xBF;
    }
	
    ret |= es8311_write_reg(ES8311_RESET_REG00, regv);
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, 0x3F);
    /*
     * Select clock source for internal mclk, Reg0X01.bit[7], 0 : MCLK Pin, 1 : SCLK Pin
     */
    Print_Info("ES8311 Mclk source: %s.\r\n", MCLK_SOURCE?"sclk pin" : "mclk pin");	    
    regv = es8311_read_reg(ES8311_CLK_MANAGER_REG01);
    (MCLK_SOURCE == FROM_SCLK_PIN) ? (regv |= 0x80) : (regv &= 0x7F);
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, regv);

    coeff = es8311_get_coeff(mclk, sample_rate);
    if (coeff < 0) {
        Print_Err("Unable to configure sample rate %dHz with %dHz MCLK\r\n", sample_rate, mclk);
        return -1;
    }

    Print_Info("ES8311 Configuration: \r\n");
    Print_Info("MCLK       : %ld Hz\r\n", mclk);
    Print_Info("rate       : %ld\r\n", coeff_div[coeff].rate);
    Print_Info("pre_div    : %ld\r\n", coeff_div[coeff].pre_div);
    Print_Info("pre_multi  : %ld\r\n", coeff_div[coeff].pre_multi);
    Print_Info("adc_aiv    : %ld\r\n", coeff_div[coeff].adc_div);
    Print_Info("dac_aiv    : %ld\r\n", coeff_div[coeff].dac_div);
    Print_Info("fs_mode    : %ld\r\n", coeff_div[coeff].fs_mode);
    Print_Info("lrck_h     : %02X\r\n", coeff_div[coeff].lrck_h);
    Print_Info("lrck_l     : %02X\r\n", coeff_div[coeff].lrck_l);
    Print_Info("bit_clk_div: %02X\r\n", coeff_div[coeff].bclk_div);
    Print_Info("adc osr    : %ld\r\n", coeff_div[coeff].adc_osr);
    Print_Info("dac osr    : %ld\r\n", coeff_div[coeff].dac_osr);

    /*
     * Set clock parammeters, Reg0X02.bit[7:5] Pre-Divide, Reg0X02.bit[4:3] Pre-multiply
     */
    regv = es8311_read_reg(ES8311_CLK_MANAGER_REG02) & 0x07;
    regv |= (coeff_div[coeff].pre_div - 1) << 5;
    datmp = 0;
    switch (coeff_div[coeff].pre_multi) {
        case 1:
            datmp = 0;
            break;
        case 2:
            datmp = 1;
            break;
        case 4:
            datmp = 2;
            break;
        case 8:
            datmp = 3;
            break;
        default:
            break;
    }
    regv |= (datmp) << 3;
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG02, regv);

    /* set internal ADC Clock and internal DAC Clock, refer to section 8.5 of the datasheet  */
    regv = 0x00;
    regv |= (coeff_div[coeff].adc_div - 1) << 4;    /* ADC divide Reg0x05.bit[7:4] */
    regv |= (coeff_div[coeff].dac_div - 1) << 0;    /* DAC divide Reg0x05.bit[3:0] */
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG05, regv);

    /* ADC speed mode(single/double), over sample rate and scale */
    regv = es8311_read_reg(ES8311_CLK_MANAGER_REG03) & 0x80;
    regv |= coeff_div[coeff].fs_mode << 6;  /* Reg0X03.bit[6] */
    regv |= coeff_div[coeff].adc_osr << 0;  /* Reg0x03.bit[5:0] , don't set the value equal or lower than 14 */
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG03, regv);

    /* DAC over sample rate */
    regv = es8311_read_reg(ES8311_CLK_MANAGER_REG04) & 0x80;
    regv |= coeff_div[coeff].dac_osr << 0;
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG04, regv);
    
    /* LRCLK divide set, bit[11:8] */
    regv = es8311_read_reg(ES8311_CLK_MANAGER_REG07) & 0xC0;
    regv |= coeff_div[coeff].lrck_h << 0;
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG07, regv);

    /* LRCLK divide set, bit[7:0] */
    regv = es8311_read_reg(ES8311_CLK_MANAGER_REG08) & 0x00;
    regv |= coeff_div[coeff].lrck_l << 0;
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG08, regv);

    /* set bit clock <BCLK>, that SCLK Pin, bi-direction. 
     * it is input when the slave mode. it is output when the master mode */
    if (MEDIA_HAL_MODE_MASTER == es8311_mode)
    {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG06) & 0xE0;
        if (coeff_div[coeff].bclk_div < 19) {
            regv |= (coeff_div[coeff].bclk_div - 1) << 0;
        } else {
            regv |= (coeff_div[coeff].bclk_div) << 0;
        }
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG06, regv);
    }

    /*
     * mclk inverted or not, Reg0x00.bit[6], 1 : invert, 0 : not
     */
    if (INVERT_MCLK) {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG01);
        regv |= 0x40;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, regv);
    } else {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG01);
        regv &= ~(0x40);
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, regv);
    }
    /*
     * sclk inverted or not, Reg0x06.bit[5], 1 : invert, 0 : not
     */
    if (INVERT_SCLK) {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG06);
        regv |= 0x20;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG06, regv);
    } else {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG06);
        regv &= ~(0x20);
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG06, regv);
    }

    ret |= es8311_write_reg(ES8311_SYSTEM_REG13, 0x00);

    /* ADC high pass filter config, not use */
    ret |= es8311_write_reg(ES8311_ADC_REG1B, 0x0A);
    ret |= es8311_write_reg(ES8311_ADC_REG1C, 0x6A);

    /* digital feedback */
    ret |= es8311_write_reg(ES8311_GPIO_REG44, 0x00);

    return 0;
}

int es8311_codec_powerup()
{
//TODO
    es8311_pa_power(1);
    return 0;
}

int es8311_codec_powerdown()
{
//TODO
    es8311_pa_power(0);
    return 0;
}

int es8311_codec_deinit(void)
{
//TODO
    return 0;
}


int es8311_codec_set_bits_per_sample(OS_UNUSED media_hal_codec_mode_t mode, media_hal_bit_length_t bits_per_sample)
{
    int ret = 0;
    
    uint8_t dac_iface = es8311_read_reg(ES8311_SDPIN_REG09) & 0xe3;
    uint8_t adc_iface = es8311_read_reg(ES8311_SDPOUT_REG0A) & 0xe3;
    switch (bits_per_sample) {
        case MEDIA_HAL_BIT_LENGTH_16BITS:
            dac_iface |= 0x0c;
            adc_iface |= 0x0c;
            break;
        case MEDIA_HAL_BIT_LENGTH_24BITS:
            break;
        case MEDIA_HAL_BIT_LENGTH_32BITS:
            dac_iface |= 0x10;
            adc_iface |= 0x10;
            break;
        default:
            Print_Info("Unsupported bit per sample. Setting to default 16 bits\r\n");
            dac_iface |= 0x0c;
            adc_iface |= 0x0c;
            break;

    }
    ret |= es8311_write_reg(ES8311_SDPIN_REG09, dac_iface);
    ret |= es8311_write_reg(ES8311_SDPOUT_REG0A, adc_iface);

    return ret;
}

int es8311_codec_ctrl_state(media_hal_codec_mode_t mode, media_hal_sel_state_t ctrl_state)
{
    int ret = 0;
    int es_mode = 0;

    switch (mode) {
        case MEDIA_HAL_CODEC_MODE_ENCODE:
            es_mode  = 0x01;
            break;
        case MEDIA_HAL_CODEC_MODE_DECODE:
            es_mode  = 0x02;
            break;
        case MEDIA_HAL_CODEC_MODE_BOTH:
            es_mode  = 0x03;
            break;
        case MEDIA_HAL_CODEC_MODE_LINE_IN:
            es_mode  = 0x04;
            break;
        default:
            es_mode = 0x02;
            Print_Info("Codec mode not support, default is decode mode\r\n");
            break;
    }

    if (ctrl_state == MEDIA_HAL_START_STATE) {
        ret |= es8311_codec_start(es_mode);
    } else {
        Print_Info("The codec is about to stop\r\n");
        ret |= es8311_codec_stop(es_mode);
    }

    return ret;
}

int es8311_codec_start(int mode)
{
    int ret = 0;
    uint8_t adc_iface = 0, dac_iface = 0;

    /* unmute */
    dac_iface = es8311_read_reg(ES8311_SDPIN_REG09)  & 0xBF;
    adc_iface = es8311_read_reg(ES8311_SDPOUT_REG0A) & 0xBF;

    if (mode == MEDIA_HAL_CODEC_MODE_LINE_IN) {
        Print_Err("The codec es8311 doesn't support ES_MODULE_LINE mode\r\n");
        return -1;
    }
    if (mode == MEDIA_HAL_CODEC_MODE_ENCODE || mode == MEDIA_HAL_CODEC_MODE_BOTH) 
    {
        adc_iface |= (1 << 6);
        adc_iface &= ~(1 << 6);
    }
    if (mode == MEDIA_HAL_CODEC_MODE_DECODE || mode == MEDIA_HAL_CODEC_MODE_BOTH) 
    {
        dac_iface |= (1 << 6);
        dac_iface &= ~(1 << 6);
    }

    ret |= es8311_write_reg(ES8311_SDPIN_REG09, dac_iface);
    ret |= es8311_write_reg(ES8311_SDPOUT_REG0A, adc_iface);

    /* DAC volume 0XBF->0dB */
    // ret |= es8311_write_reg(ES8311_DAC_REG32, 0xBF - 20);
    /* ADC volume 0XBF->0dB */
    ret |= es8311_write_reg(ES8311_ADC_REG17, 0xB8);

    /*  */
    ret |= es8311_write_reg(ES8311_SYSTEM_REG0E, 0x02);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG12, 0x00);
    /* select Mic1p-Mic1n, 30dB */
    //ret |= es8311_write_reg(ES8311_SYSTEM_REG14, 0x0A);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG14, 0x17);

    /*
     * PDM(paulse density modulation) dmic(digital microphone) enable or disable
     */
    int regv = 0;
    if (IS_DMIC) {
        regv = es8311_read_reg(ES8311_SYSTEM_REG14);
        regv |= 0x40;
        ret |= es8311_write_reg(ES8311_SYSTEM_REG14, regv);
    } else {
        regv = es8311_read_reg(ES8311_SYSTEM_REG14);
        regv &= ~(0x40);
        ret |= es8311_write_reg(ES8311_SYSTEM_REG14, regv);
    }

    /* start up vmind normal speed charge */
    ret |= es8311_write_reg(ES8311_SYSTEM_REG0D, 0x01);
    
    /* ADC fade in and fade out : 0.25dB/32LRCK */
    ret |= es8311_write_reg(ES8311_ADC_REG15, 0x40);
    /* DAC fade in and fade out : 0.25dB/32LRCK, equalizer : DACEQ bypass open */
    ret |= es8311_write_reg(ES8311_DAC_REG37, 0x48);
    ret |= es8311_write_reg(ES8311_GP_REG45, 0x00);

    /* set internal reference signal (ADCL + DACR) */
    ret |= es8311_write_reg(ES8311_GPIO_REG44, 0x50);

    return ret;
}


int es8311_codec_mic_selection(int val)
{
    int ret = 0;
	
    ret |= es8311_write_reg(ES8311_SYSTEM_REG14, val);

    return ret;
}

int es8311_codec_mic_loopback(void)
{
    int ret = 0;

    //ret |= es8311_write_reg(ES8311_GPIO_REG44, 0x50 | (0x1<<7));
    ret |= es8311_write_reg(ES8311_GPIO_REG44, 0x80 );

    return ret;
}

int es8311_codec_stop(OS_UNUSED int mode)
{
    es8311_suspend();
    return 0;
}

int es8311_codec_set_volume(int volume)
{
    int ret = 0;
    if (volume <= 0) {
        return es8311_codec_set_mute(1);
    } 
    if (volume > 100) {
        volume = 100;
    }

    int vol = (volume) * 2550 / 1000;
    Print_Info("ES8311 volume:%d\r\n", volume);
    ret = es8311_write_reg(ES8311_DAC_REG32, vol);
    return ret;
}

int es8311_codec_get_volume(int *volume)
{
    int ret = 0;
    int regv = 0;
    regv = es8311_read_reg(ES8311_DAC_REG32);
    if (regv == -1) {
        *volume = 0;
        ret = -1;
    } else {
        *volume = regv * 100 / 256;
    }
    Print_Info("GET: res:%d, volume:%d\r\n", regv, *volume);
    return ret;
}

int es8311_codec_set_mute(int enable)
{
    es8311_mute(enable);
    return 0;
}

int es8311_codec_get_mute(int *mute)
{
    int ret = -1;
    uint8_t reg = 0;
    ret = es8311_read_reg(ES8311_DAC_REG31);
    if (ret != -1) {
        reg = (ret & 0x20) >> 5;
    }
    *mute = reg;
    return ret;
}



int es8311_codec_set_adc_digit_gain(int32_t db)
{
    int res = 0;
    uint8_t regVal = 0;
    regVal = (db - (-9550)) / 50;
    res = es8311_write_reg(ES8311_ADC_REG17, regVal); 
    return res; 
}

int es8311_codec_get_adc_digit_gain(uint8_t *db, uint8_t *len)
{
    int res = 0;
    int32_t dbVal = 0;
    
    res = es8311_read_reg(ES8311_ADC_REG17); 
    if(res >= 0)
    {
        dbVal = (int32_t)(res * 50 + (-9550));
        db[0] = (dbVal >> 24) & 0xff;
        db[1] = (dbVal >> 16) & 0xff;
        db[2] = (dbVal >> 8) & 0xff;
        db[3] = (dbVal) & 0xff;
        *len = 4;
    }
        
    return res; 
}

int es8311_codec_set_dac_digit_gain(int32_t db)
{
    int res = 0;
    uint8_t regVal = 0;
    regVal = (db - (-9550)) / 50;

    res = es8311_write_reg(ES8311_DAC_REG32, regVal); 
    return res; 
}

int es8311_codec_get_dac_digit_gain(uint8_t *db, uint8_t *len)
{
    int res = 0;
    int32_t dbVal = 0;

    res = es8311_read_reg(ES8311_DAC_REG32); 
    if(res >= 0)
    {
        dbVal = (int32_t)(res * 50 + (-9550));
        db[0] = (dbVal >> 24) & 0xff;
        db[1] = (dbVal >> 16) & 0xff;
        db[2] = (dbVal >> 8) & 0xff;
        db[3] = (dbVal) & 0xff;
        *len = 4;
    }
        
    return res; 
}

int es8311_codec_set_adc_pga_gain(int32_t db)
{
    int res = 0;
    uint8_t regVal = 0;
    regVal = es8311_read_reg(ES8311_SYSTEM_REG14);

    regVal = (regVal & 0xF0) | ((db / 300) & 0x0F);
    res = es8311_write_reg(ES8311_SYSTEM_REG14, regVal); 
    return res; 
}

int es8311_codec_get_adc_pga_gain(uint8_t *db, uint8_t *len)
{
    int res = 0;
    int32_t dbVal = 0;
    
    res = es8311_read_reg(ES8311_SYSTEM_REG14); 
    if(res >= 0)
    {
        dbVal = (res & 0x0f) * 300;
        db[0] = (dbVal >> 24) & 0xff;
        db[1] = (dbVal >> 16) & 0xff;
        db[2] = (dbVal >> 8) & 0xff;
        db[3] = (dbVal) & 0xff;
        *len = 4;
    }
        
    return res; 
}


int es8311_codec_set_mic_gain(int32_t db)
{
    int res = 0;
    uint8_t regVal = 0;
    regVal = es8311_read_reg(ES8311_ADC_REG16);
    
    regVal = (regVal & 0xF8) | ((db / 600) & 0x07);
    res = es8311_write_reg(ES8311_ADC_REG16, regVal); 
    return res; 
}

int es8311_codec_get_mic_gain(uint8_t *db, uint8_t *len)
{
    int res = 0;
    int32_t dbVal = 0;
    
    res = es8311_read_reg(ES8311_ADC_REG16); 
    if(res >= 0)
    {
        dbVal = (res & 0x07) * 600;
        db[0] = (dbVal >> 24) & 0xff;
        db[1] = (dbVal >> 16) & 0xff;
        db[2] = (dbVal >> 8) & 0xff;
        db[3] = (dbVal) & 0xff;
        *len = 4;
    }
        
    return res; 
}


int es8311_codec_set_nr_gate(int32_t db)
{
    int res = 0;
    uint8_t regVal = 0;
    regVal = es8311_read_reg(ES8311_ADC_REG1A);
    
    if(db <= -5400)
        regVal = (regVal & 0xF0) | (((db - (-9600)) / 600) & 0x0F);
    else
        regVal = (regVal & 0xF0) | ((((db - (-5400)) / 300) & 0x0F) + 7);

    res = es8311_write_reg(ES8311_ADC_REG1A, regVal); 
    return res; 
}

int es8311_codec_get_nr_gate(uint8_t *db, uint8_t *len)
{
    int res = 0;
    int32_t dbVal = 0;
    
    res = es8311_read_reg(ES8311_ADC_REG1A); 
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
        
    return res; 
}


int es8311_codec_set_adc_eq(uint8_t *para, uint8_t len)
{
    if(para[45] == 0)
        return 0;

    for(uint8_t i = 0; i < len / 2; i++)
    {
        es8311_write_reg(para[i*2], para[i*2+1]);
    }
    return 0;
}

int es8311_codec_get_adc_eq(uint8_t *para, uint8_t *len)
{
    uint8_t i = 0;
    uint8_t reg_addr = 0;

    if(es8311_read_reg(ES8311_ADC_REG1C) & 0x40)
    {
        para[i++] = ES8311_ADC_REG1C;   para[i++] = 0;

        for(reg_addr = 0x1D; reg_addr <= 0x30; reg_addr ++)
        {
            para[i++] = reg_addr;               
            para[i++] = 0;
        }

        para[i++] = ES8311_ADC_REG1B;   para[i++] = 0;
        para[i++] = ES8311_ADC_REG1C;   para[i++] = 0;
        *len = i;   
    }
    else
    {
        para[i++] = ES8311_ADC_REG1C;   para[i++] = (es8311_read_reg(ES8311_ADC_REG1C) & 0x3f) | 0x40;

        for(reg_addr = 0x1D; reg_addr <= 0x30; reg_addr ++)
        {
            para[i++] = reg_addr;               
            para[i++] = es8311_read_reg(reg_addr) & 0xff;
        }

        para[i++] = ES8311_ADC_REG1B;   para[i++] = es8311_read_reg(ES8311_ADC_REG1B);
        para[i++] = ES8311_ADC_REG1C;   para[i++] = (es8311_read_reg(ES8311_ADC_REG1C) & 0x3f);
        *len = i;
    }

    return 0;
}

int es8311_codec_set_dac_eq(uint8_t *para, uint8_t len)
{
    if(para[31] == 0)
        return 0;

    for(uint8_t i = 0; i < len / 2; i++)
    {
        es8311_write_reg(para[i*2], para[i*2+1]);
    }
    return 0;
}

int es8311_codec_get_dac_eq(uint8_t *para, uint8_t *len)
{
    uint8_t i = 0;
    uint8_t reg_addr = 0;

    if(es8311_read_reg(ES8311_DAC_REG37) & 0x08)
    {
        para[i++] = ES8311_DAC_REG31;   para[i++] = 0;
        para[i++] = ES8311_DAC_REG37;   para[i++] = 0;

        for(reg_addr = 0x38; reg_addr <= 0x43; reg_addr ++)
        {
            para[i++] = reg_addr;               
            para[i++] = 0;
        }

        para[i++] = ES8311_DAC_REG37;   para[i++] = 0;
        para[i++] = ES8311_DAC_REG31;   para[i++] = 0;
        *len = i;
    }
    else
    {
        para[i++] = ES8311_DAC_REG31;   para[i++] = es8311_read_reg(ES8311_DAC_REG31) | 0x60;
        para[i++] = ES8311_DAC_REG37;   para[i++] = es8311_read_reg(ES8311_DAC_REG37) | 0x08;

        for(reg_addr = 0x38; reg_addr <= 0x43; reg_addr ++)
        {
            para[i++] = reg_addr;               
            para[i++] = es8311_read_reg(reg_addr) & 0xff;
        }

        para[i++] = ES8311_DAC_REG37;   para[i++] = es8311_read_reg(ES8311_DAC_REG37) & (~0x08);
        para[i++] = ES8311_DAC_REG31;   para[i++] = es8311_read_reg(ES8311_DAC_REG31) & (~0x60);
        *len = i;
    }


    return 0;
}

int es8311_codec_set_drc(uint8_t *para, uint8_t len)
{
    if(para[9] == 0)
        return 0;

    for(uint8_t i = 0; i < len / 2; i++)
    {
        es8311_write_reg(para[i*2], para[i*2+1]);
    }
    return 0; 
}

int es8311_codec_get_drc(uint8_t *para, uint8_t *len)
{
    uint8_t i = 0;

    if((uint8_t)(ES8311_DAC_REG34) & 0x80)
    {
        para[i++] = ES8311_DAC_REG34;   para[i++] = es8311_read_reg(ES8311_DAC_REG34) & 0x7F;
        para[i++] = ES8311_DAC_REG35;   para[i++] = es8311_read_reg(ES8311_DAC_REG35) & 0xFF;
        para[i++] = ES8311_DAC_REG37;   para[i++] = es8311_read_reg(ES8311_DAC_REG37) & 0xFF;
        para[i++] = ES8311_DAC_REG32;   para[i++] = es8311_read_reg(ES8311_DAC_REG32) & 0xFF;
        para[i++] = ES8311_DAC_REG34;   para[i++] = es8311_read_reg(ES8311_DAC_REG34) | 0x80;

        *len = i;
    }
    else
    {
        para[i++] = ES8311_DAC_REG34;   para[i++] = 0;
        para[i++] = ES8311_DAC_REG35;   para[i++] = 0;
        para[i++] = ES8311_DAC_REG37;   para[i++] = 0;
        para[i++] = ES8311_DAC_REG32;   para[i++] = 0;
        para[i++] = ES8311_DAC_REG34;   para[i++] = 0;

        *len = i;
    }
    return 0;
}

int es8311_codec_set_alc(uint8_t *para, uint8_t len)
{
    if(para[11] == 0)
        return 0;

    for(uint8_t i = 0; i < len / 2; i++)
    {
        es8311_write_reg(para[i*2], para[i*2+1]);
    }
    return 0;
}

int es8311_codec_get_alc(uint8_t *para, uint8_t *len)
{
    uint8_t i = 0;

    if((uint8_t)es8311_read_reg(ES8311_ADC_REG18) & 0x80)
    {
        para[i++] = ES8311_ADC_REG18;   para[i++] = es8311_read_reg(ES8311_ADC_REG18) & 0x7F;
        para[i++] = ES8311_ADC_REG19;   para[i++] = es8311_read_reg(ES8311_ADC_REG19) & 0xFF;
        para[i++] = ES8311_ADC_REG15;   para[i++] = es8311_read_reg(ES8311_ADC_REG15) & 0xFF;
        para[i++] = ES8311_ADC_REG1B;   para[i++] = es8311_read_reg(ES8311_ADC_REG1B) & 0xFF;
        para[i++] = ES8311_ADC_REG17;   para[i++] = es8311_read_reg(ES8311_ADC_REG17) & 0xFF;
        para[i++] = ES8311_ADC_REG18;   para[i++] = es8311_read_reg(ES8311_ADC_REG18) | 0x80;
        *len = i;
    }
    else
    {
        para[i++] = ES8311_ADC_REG18;   para[i++] = 0;
        para[i++] = ES8311_ADC_REG19;   para[i++] = 0;
        para[i++] = ES8311_ADC_REG15;   para[i++] = 0;
        para[i++] = ES8311_ADC_REG1B;   para[i++] = 0;
        para[i++] = ES8311_ADC_REG17;   para[i++] = 0;
        para[i++] = ES8311_ADC_REG18;   para[i++] = 0;
        *len = i;
    }

    return 0;
}

int es8311_codec_get_side_tone(uint8_t *side_tone, uint8_t *len)
{
    int res = 0;

    res = es8311_read_reg(ES8311_GPIO_REG44); 
    if(res >= 0)
    {
        *side_tone = (res >> 7) & 0x01;
        *len = 1;
    }

    return res;
}

int es8311_codec_set_side_tone(uint8_t side_tone)
{
    int res = 0;
    uint8_t regVal = 0;

    regVal = es8311_read_reg(ES8311_GPIO_REG44);

    if(side_tone)
    {
        regVal |= (1 << 7);
    }
    else
        regVal &= ~(1 << 7);
    
    res = es8311_write_reg(ES8311_GPIO_REG44, regVal);
    return res;
}

int es8311_codec_get_id(uint8_t *id, uint8_t *len)
{
    id[2] = es8311_read_reg(ES8311_CHD1_REGFD); 
    id[3] = es8311_read_reg(ES8311_CHD2_REGFE); 
    id[0] = 0;
    id[1] = 0;
    *len = 4;

    return 0; 
}