/**
  ******************************************************************************
  * @file    pt8311_adapter.c
  * @brief   PT8311 driver adapter implementation for LM620 SDK
  ******************************************************************************
  */

#include "pt8311_user_cfg.h"
#include "pt8311.h"
#include "i2c_device.h"

// PT8311 I2C地址配置
#define PT8311_I2C_ADDR                  PT8311_I2C_ADDR_0

// PT8311 I2C设备结构
static I2C_BusDevice g_pt8311I2c = {
    .addr = PT8311_I2C_ADDR,
};

static bool g_pt8311Initialized = false;

/**
 * @brief  PT8311 delay ms
 * @param  ms millisecond
 */
void pt8311_delay_ms(int ms)
{
    osDelay(osKernelGetTickFreq() * ms / 1000);
}

/**
 * @brief  PT8311 I2C read byte
 * @param  i2c_addr PT8311 I2C address
 * @param  reg PT8311 register address to read
 * @return register read value
 */
unsigned char pt8311_i2c_read_byte(unsigned char i2c_addr, unsigned char reg)
{
    unsigned char data = 0;

    if (!g_pt8311Initialized) {
        PT8311_USER_DBG_LOG("PT8311 I2C not initialized");
        return 0xFF;
    }

    I2C_BusLock(&g_pt8311I2c);
    I2C_BusDevice_ReadReg(&g_pt8311I2c, reg, &data, 1);
    I2C_BusUnlock(&g_pt8311I2c);

    return data;
}

/**
 * @brief  PT8311 I2C write byte
 * @param  i2c_addr PT8311 I2C address
 * @param  reg PT8311 register address to write
 * @param  val register value
 * @retval 0      Register write Success
 * @retval others Register write Failed
 */
signed char pt8311_i2c_write_byte(unsigned char i2c_addr, unsigned char reg, unsigned char val)
{
    unsigned char regConfig[2];
    int32_t result = 0;

    if (!g_pt8311Initialized) {
        PT8311_USER_DBG_LOG("PT8311 I2C not initialized");
        return -1;
    }

    regConfig[0] = reg;
    regConfig[1] = val;

    I2C_BusLock(&g_pt8311I2c);
    result = I2C_BusDevice_Write(&g_pt8311I2c, regConfig, 2);
    I2C_BusUnlock(&g_pt8311I2c);

    return (result == 0) ? 0 : -1;
}

/**
 * @brief  PT8311 I2C config
 * @param  busNo I2C bus number
 */
int pt8311_i2c_config(uint8_t busNo)
{
    static bool isRegister = false;

    if (!isRegister) {
        isRegister = true;
        I2C_BusDevice_Register(busNo, &g_pt8311I2c);
        g_pt8311Initialized = true;
        PT8311_USER_DBG_LOG("PT8311 I2C initialized on bus %d, addr 0x%02X", busNo, PT8311_I2C_ADDR);
    }
    /* 通过读寄存器到 buf， 检查 buf 前后是否有变化来判断器件是否在线（因为无 id 寄存器），避免无器件 I2C 写操作时堵住*/
    uint8_t check_data = 0xFF;
    I2C_BusDevice_ReadReg(&g_pt8311I2c, PWR_CTRL1, &check_data, 1);
    if (check_data == 0xFF)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief  PT8311 I2C deconfig
 * @param  busNo I2C bus number
 */
void pt8311_i2c_deconfig(uint8_t busNo)
{
    if (g_pt8311Initialized) {
        I2C_BusDevice_UnRegister(busNo, &g_pt8311I2c);
        g_pt8311Initialized = false;
        PT8311_USER_DBG_LOG("PT8311 I2C deinitialized from bus %d", busNo);
    }
}