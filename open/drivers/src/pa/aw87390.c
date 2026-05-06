
#include "i2c_device.h"

static I2C_BusDevice g_paI2c = {
    .addr = 0x58,
};

static void aw87390_i2c_config(uint8_t busNo)
{
    static bool isRegister = false;

    if(!isRegister)
    {
        isRegister = true;
        I2C_BusDevice_Register(busNo, &g_paI2c);
    }
}

static int aw87390_write_reg(uint8_t reg_addr, uint8_t data)
{
    uint8_t regConfig[2] = {0};

    regConfig[0] = reg_addr;
    regConfig[1] = data;

    I2C_BusLock(&g_paI2c);
    I2C_BusDevice_Write(&g_paI2c, regConfig, 2);
    I2C_BusUnlock(&g_paI2c);

    return 0;
}

static int aw87390_read_reg(uint8_t reg_addr)
{
    uint8_t data = 0;

    I2C_BusLock(&g_paI2c);
    I2C_BusDevice_ReadReg(&g_paI2c, reg_addr, &data, 1);
    I2C_BusUnlock(&g_paI2c);

    return (int)data;
}

void aw87390_init(uint8_t busNo)
{
    aw87390_i2c_config(busNo);

    aw87390_write_reg(0x67, 0x03);
    aw87390_write_reg(0x02,0x07);
    aw87390_write_reg(0x02, 0x00);
    aw87390_write_reg(0x02, 0x00);

    aw87390_write_reg(0x03, 0x08);
    aw87390_write_reg(0x04, 0x05);
    aw87390_write_reg(0x05, 0x19);   // gain
    aw87390_write_reg(0x06, 0x09);   
    aw87390_write_reg(0x07, 0x4e);   
    aw87390_write_reg(0x08, 0x08);   
    aw87390_write_reg(0x09, 0x08); 
    aw87390_write_reg(0x0a, 0x3a); 
    aw87390_write_reg(0x61, 0xb3);
    aw87390_write_reg(0x62, 0x24);  
    aw87390_write_reg(0x63, 0x05); 
    aw87390_write_reg(0x64, 0x48); 
    aw87390_write_reg(0x65, 0x17);
    aw87390_write_reg(0x79, 0x7a);
    aw87390_write_reg(0x7a, 0x6c);
    aw87390_write_reg(0x78, 0x80);
    aw87390_write_reg(0x66, 0x38);
    aw87390_write_reg(0x76, 0x00);
    aw87390_write_reg(0x78, 0x00);
    aw87390_write_reg(0x68, 0x1b);
    aw87390_write_reg(0x69, 0x5b);
    aw87390_write_reg(0x70, 0x1d);
    aw87390_write_reg(0x71, 0x10);
    aw87390_write_reg(0x72, 0xb4);
    aw87390_write_reg(0x73, 0x4f);
    aw87390_write_reg(0x74, 0x24);
    aw87390_write_reg(0x75, 0x02); 
    aw87390_write_reg(0x01, 0x07);               

}

void aw87390_deinit(uint8_t busNo)
{
    aw87390_i2c_config(busNo);
    aw87390_write_reg(0x01, 0x00); 
}