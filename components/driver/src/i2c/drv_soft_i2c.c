/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_i2c.h
 *
 * @brief       I2C驱动接口.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-04-21     ict team          创建
 ************************************************************************************
 */
/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <drv_soft_i2c.h>
#include <string.h>
#include <os.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define I2C_WRITE(SLAVE_ADDR)           ((SLAVE_ADDR << 1) & 0xFE)
#define I2C_READ(SLAVE_ADDR)            ((SLAVE_ADDR << 1) | 0x01)

#define I2C_PIN_SCL_LOW(hdl)            hdl->ops->scl_write(hdl->userData, 0)
#define I2C_PIN_SCL_HIGH(hdl)           hdl->ops->scl_write(hdl->userData, 1)
#define I2C_PIN_SDA_LOW(hdl)            hdl->ops->sda_write(hdl->userData, 0)
#define I2C_PIN_SDA_HIGH(hdl)           hdl->ops->sda_write(hdl->userData, 1)
#define I2C_PIN_SCL_READ(hdl)           hdl->ops->scl_read(hdl->userData)
#define I2C_PIN_SDA_READ(hdl)           hdl->ops->sda_read(hdl->userData)
#define I2C_PIN_SCL_DIR_INPUT(hdl)      hdl->ops->scl_dir(hdl->userData, I2C_SOFT_GPIO_INPUT_MODE)
#define I2C_PIN_SCL_DIR_OUTPUT(hdl)     hdl->ops->scl_dir(hdl->userData, I2C_SOFT_GPIO_OUTPUT_MODE)
#define I2C_PIN_SDA_DIR_INPUT(hdl)      hdl->ops->sda_dir(hdl->userData, I2C_SOFT_GPIO_INPUT_MODE)
#define I2C_PIN_SDA_DIR_OUTPUT(hdl)     hdl->ops->sda_dir(hdl->userData, I2C_SOFT_GPIO_OUTPUT_MODE)

#define I2C_SOFT_OPS_CHECK(ops) \
    OS_ASSERT(ops->init);       \
    OS_ASSERT(ops->scl_write);  \
    OS_ASSERT(ops->sda_write);  \
    OS_ASSERT(ops->sda_dir);    \
    OS_ASSERT(ops->sda_read)

 /**
 * @brief 软件模拟 I2C 等待 SCL 释放
 *
 * @param [in]  hdl          I2C结构体指针
 * @param [in]  waitCnt        等待次数, 必须在前文声明
 *
 * @details 用于开启时钟延展功能, 等待从机释放时钟线
 */
#define SOFT_I2C_WAIT_SCL_RELEASE(hdl, waitCnt)                       \
        while (waitCnt > 0 && I2C_PIN_SCL_READ(hdl) == 0) {           \
            osUsDelay(1);                                             \
            waitCnt--;                                                \
        }

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
static void I2C_SOFT_Delay(I2C_SOFT_Handle *hdl);
static void I2C_SOFT_Ack(I2C_SOFT_Handle *hdl);
static void I2C_SOFT_NoAck(I2C_SOFT_Handle *hdl);

/************************************************************************************
*                                 全局变量定义
************************************************************************************/
#if (I2C_SOFT_LIST)
static I2C_SOFT_Handle* g_i2cSOftHandle = NULL;
#endif

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

static void I2C_SOFT_Delay(I2C_SOFT_Handle *hdl)
{
    if(hdl->speed_us == 0) {
        return;
    }
    osUsDelay(hdl->speed_us);
}

static void I2C_SOFT_Start(I2C_SOFT_Handle *hdl)
{
    I2C_PIN_SDA_HIGH(hdl);
    I2C_PIN_SCL_HIGH(hdl);

#if defined(SOFT_I2C_CLOCK_STRECH_EN)
    uint32_t waitCnt = SOFT_I2C_TIME_OUT;    // 等待计数
    I2C_PIN_SCL_DIR_INPUT(hdl);
    SOFT_I2C_WAIT_SCL_RELEASE(hdl, waitCnt); // 等待SCL释放
    I2C_PIN_SCL_DIR_OUTPUT(hdl);
#endif

    I2C_SOFT_Delay(hdl);
    I2C_PIN_SDA_LOW(hdl);
    I2C_SOFT_Delay(hdl);
    I2C_PIN_SCL_LOW(hdl);
}

static void I2C_SOFT_Stop(I2C_SOFT_Handle *hdl)
{
    I2C_PIN_SCL_LOW(hdl);
    I2C_PIN_SDA_LOW(hdl);
    I2C_SOFT_Delay(hdl);
    I2C_PIN_SCL_HIGH(hdl);
    
#if defined(SOFT_I2C_CLOCK_STRECH_EN)
    uint32_t waitCnt = SOFT_I2C_TIME_OUT;    // 等待计数
    I2C_PIN_SCL_DIR_INPUT(hdl);
    SOFT_I2C_WAIT_SCL_RELEASE(hdl, waitCnt); // 等待SCL释放
    I2C_PIN_SCL_DIR_OUTPUT(hdl);
#endif

    I2C_SOFT_Delay(hdl);
    I2C_PIN_SDA_HIGH(hdl);
    I2C_SOFT_Delay(hdl);
}

static void I2C_SOFT_Ack(I2C_SOFT_Handle *hdl)
{
    I2C_PIN_SCL_LOW(hdl);
    I2C_PIN_SDA_DIR_OUTPUT(hdl);
    I2C_PIN_SDA_LOW(hdl);
    I2C_SOFT_Delay(hdl);
    I2C_PIN_SCL_HIGH(hdl);
    I2C_SOFT_Delay(hdl);
    I2C_PIN_SCL_LOW(hdl);
}

static void I2C_SOFT_NoAck(I2C_SOFT_Handle *hdl)
{
    I2C_PIN_SCL_LOW(hdl);
    I2C_PIN_SDA_DIR_OUTPUT(hdl);
    I2C_PIN_SDA_LOW(hdl);
    I2C_SOFT_Delay(hdl);
    I2C_PIN_SDA_HIGH(hdl);
    I2C_SOFT_Delay(hdl);
    I2C_PIN_SCL_HIGH(hdl);
    I2C_SOFT_Delay(hdl);
    I2C_PIN_SCL_LOW(hdl);
}

static int32_t I2C_SOFT_WaitAck(I2C_SOFT_Handle *hdl)
{
    uint16_t wait_time = 255;

    I2C_PIN_SCL_LOW(hdl);
    I2C_PIN_SDA_DIR_INPUT(hdl);
    I2C_SOFT_Delay(hdl);
    I2C_PIN_SCL_HIGH(hdl);

#if defined(SOFT_I2C_CLOCK_STRECH_EN)
    uint32_t waitCnt = SOFT_I2C_TIME_OUT;    // 等待计数
    I2C_PIN_SCL_DIR_INPUT(hdl);
    SOFT_I2C_WAIT_SCL_RELEASE(hdl, waitCnt); // 等待SCL释放
    I2C_PIN_SCL_DIR_OUTPUT(hdl);
    if (waitCnt == 0) {
        return SOFT_I2C_ERR_TIMEOUT;
    }
#endif

    while (I2C_PIN_SDA_READ(hdl)) {
        if ((wait_time--) == 0) {
            I2C_PIN_SDA_DIR_OUTPUT(hdl);
            I2C_PIN_SDA_HIGH(hdl);
            I2C_SOFT_Stop(hdl);
            return SOFT_I2C_ERR_NACK;
        }
    }
    I2C_SOFT_Delay(hdl);
    I2C_PIN_SCL_LOW(hdl);
    I2C_PIN_SDA_HIGH(hdl);
    I2C_PIN_SDA_DIR_OUTPUT(hdl);
    
    return SOFT_I2C_ERR_OK;
}

#if (I2C_SOFT_LIST)
I2C_SOFT_Handle *I2C_SOFT_Find(const char* name)
{
    I2C_SOFT_Handle* hdl;

    for (hdl = g_i2cSOftHandle; hdl != NULL; hdl = hdl->next) {
        if (strcmp(hdl->name, name) == 0) {
            return hdl;
        }
    }

    return NULL;
}
#endif

int32_t I2C_SOFT_Initialize(I2C_SOFT_Handle *hdl, const char *name, I2C_SOFT_Ops *ops)
{
    OS_ASSERT(hdl);
    OS_ASSERT(name);
    OS_ASSERT(ops);
    I2C_SOFT_OPS_CHECK(ops);

    if (NULL != name) {
        (void)strncpy(hdl->name, name, I2C_SOFT_NAME_LEN);
        hdl->name[I2C_SOFT_NAME_LEN - 1] = '\0';
    }
    else {
        hdl->name[0] = '\0';
    }

    hdl->ops = ops;
    hdl->ops->init(hdl->userData);
    hdl->speed = 100000;
    hdl->speed_us = 1000000 / 2 / hdl->speed;

    I2C_PIN_SCL_DIR_OUTPUT(hdl);
    I2C_PIN_SDA_DIR_OUTPUT(hdl);
    I2C_PIN_SDA_HIGH(hdl);
    I2C_PIN_SCL_HIGH(hdl);

#if (I2C_SOFT_LIST)
    hdl->next = g_i2cSOftHandle;
    g_i2cSOftHandle = hdl;
#endif

    return 0;
}

int32_t I2C_SOFT_WriteByte(I2C_SOFT_Handle *hdl, uint8_t byte)
{
    uint8_t i;
    int32_t err;

#if defined(SOFT_I2C_CLOCK_STRECH_EN)
    uint32_t waitCnt    = SOFT_I2C_TIME_OUT;  // 等待计数
    bool     isFirstBit = true;               // 是否第一位
#endif

    for (i = 0; i < 8; i++) {
        I2C_PIN_SCL_LOW(hdl);
        I2C_SOFT_Delay(hdl);
        if (byte & 0x80)
            I2C_PIN_SDA_HIGH(hdl);
        else
            I2C_PIN_SDA_LOW(hdl);
        I2C_PIN_SCL_HIGH(hdl);

#if defined(SOFT_I2C_CLOCK_STRECH_EN)
        if (isFirstBit) {
            I2C_PIN_SCL_DIR_INPUT(hdl);
            SOFT_I2C_WAIT_SCL_RELEASE(hdl, waitCnt); // 等待SCL释放
            I2C_PIN_SCL_DIR_OUTPUT(hdl);
            if (waitCnt == 0) {
             return SOFT_I2C_ERR_TIMEOUT;
            }
            isFirstBit = false;
        }
#endif

        I2C_SOFT_Delay(hdl);
        byte <<= 1;
    }
    err = I2C_SOFT_WaitAck(hdl);
    return err;
}

uint8_t I2C_SOFT_ReadByte(I2C_SOFT_Handle *hdl, uint8_t ack)
{
    uint8_t i, byte = 0;
#if defined(SOFT_I2C_CLOCK_STRECH_EN)
    uint32_t waitCnt    = SOFT_I2C_TIME_OUT;  // 等待计数
    bool     isFirstBit = true;               // 是否第一位
#endif

    I2C_PIN_SDA_DIR_INPUT(hdl);
    for (i = 0; i < 8; i++) {
        byte <<= 1;
        I2C_PIN_SCL_LOW(hdl);
        I2C_SOFT_Delay(hdl);
        I2C_PIN_SCL_HIGH(hdl);

#if defined(SOFT_I2C_CLOCK_STRECH_EN)
        if (isFirstBit) {
            I2C_PIN_SCL_DIR_INPUT(hdl);
            SOFT_I2C_WAIT_SCL_RELEASE(hdl, waitCnt); // 等待SCL释放
            I2C_PIN_SCL_DIR_OUTPUT(hdl);
            if (waitCnt == 0) {
                return SOFT_I2C_ERR_TIMEOUT;
            }
            isFirstBit = false;
        }
#endif

        if (I2C_PIN_SDA_READ(hdl))
            byte |= 0x01;
        I2C_SOFT_Delay(hdl);
    }
    if (ack)
        I2C_SOFT_Ack(hdl);
    else
        I2C_SOFT_NoAck(hdl);
    return byte;
}

int32_t I2C_SOFT_WriteBytes(I2C_SOFT_Handle *hdl, uint8_t slave_addr, void *pbuf, uint16_t length)
{
    uint8_t i;
    int32_t err = SOFT_I2C_ERR_OK;
    uint8_t *p = (uint8_t*)pbuf;

    I2C_SOFT_Start(hdl);
    if(I2C_SOFT_WriteByte(hdl, I2C_WRITE(slave_addr))!= SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_ADDR; 

    for (i = 0; i < length; i++) {
        err = I2C_SOFT_WriteByte(hdl, p[i]);
    }
    I2C_SOFT_Stop(hdl);
    return err;
}

int32_t I2C_SOFT_WriteRegBytes(I2C_SOFT_Handle *hdl, uint8_t slave_addr, uint8_t reg_addr, void *pbuf, uint16_t length)
{
    uint8_t i;
    int32_t err = SOFT_I2C_ERR_OK;
    uint8_t *p = (uint8_t*)pbuf;

    I2C_SOFT_Start(hdl);
    if(I2C_SOFT_WriteByte(hdl, I2C_WRITE(slave_addr)) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_ADDR; 
    if(I2C_SOFT_WriteByte(hdl, reg_addr) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_REG_ADDR; 

    for (i = 0; i < length; i++) {
        err = I2C_SOFT_WriteByte(hdl, p[i]);
    }
    I2C_SOFT_Stop(hdl);
    return err;
}

int32_t I2C_SOFT_ReadBytes(I2C_SOFT_Handle *hdl, uint8_t slave_addr, uint8_t reg_addr, void *pbuf, uint16_t length)
{
    uint8_t i;
    uint8_t *p = (uint8_t*)pbuf;

    I2C_SOFT_Start(hdl);
    if(I2C_SOFT_WriteByte(hdl, I2C_WRITE(slave_addr)) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_ADDR; 
    if(I2C_SOFT_WriteByte(hdl, reg_addr) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_REG_ADDR; 
    I2C_SOFT_Start(hdl);
    if(I2C_SOFT_WriteByte(hdl, I2C_READ(slave_addr)) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_READ_ADDR;

    for (i = 0; i < length; i++) {
        if (i != (length - 1)) {
            p[i] = I2C_SOFT_ReadByte(hdl, 1);
        } else {
            p[i] = I2C_SOFT_ReadByte(hdl, 0);
        }
    }
    I2C_SOFT_Stop(hdl);

    return SOFT_I2C_ERR_OK;
}

int32_t I2C_SOFT_WriteBytes_16bit(I2C_SOFT_Handle *hdl, uint8_t slave_addr, uint16_t reg_addr, void *pbuf, uint16_t length)
{
    uint8_t i;
    int32_t err = SOFT_I2C_ERR_OK;
    uint8_t *p = (uint8_t*)pbuf;

    I2C_SOFT_Start(hdl);
    if(I2C_SOFT_WriteByte(hdl, I2C_WRITE(slave_addr)) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_ADDR; 
    if(I2C_SOFT_WriteByte(hdl, (uint8_t)(reg_addr >> 8)) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_REG_ADDR; 
    if(I2C_SOFT_WriteByte(hdl, reg_addr) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_REG_ADDR; 

    for (i = 0; i < length; i++) {
        err = I2C_SOFT_WriteByte(hdl, p[i]);
    }
    I2C_SOFT_Stop(hdl);
    return err;
}

int32_t I2C_SOFT_ReadBytes_16bit(I2C_SOFT_Handle *hdl, uint8_t slave_addr, uint16_t reg_addr, void *pbuf, uint16_t length)
{
    uint8_t i;
    uint8_t *p = (uint8_t*)pbuf;

    I2C_SOFT_Start(hdl);
    if(I2C_SOFT_WriteByte(hdl, I2C_WRITE(slave_addr)) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_ADDR; 
    if(I2C_SOFT_WriteByte(hdl, (uint8_t)(reg_addr >> 8)) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_REG_ADDR; 
    if(I2C_SOFT_WriteByte(hdl, reg_addr) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_WRITE_REG_ADDR; 
    I2C_SOFT_Start(hdl);
    if(I2C_SOFT_WriteByte(hdl, I2C_READ(slave_addr)) != SOFT_I2C_ERR_OK)
        return SOFT_I2C_ERR_READ_ADDR;

    for (i = 0; i < length; i++) {
        if (i != (length - 1)) {
            p[i] = I2C_SOFT_ReadByte(hdl, 1);
        } else {
            p[i] = I2C_SOFT_ReadByte(hdl, 0);
        }
    }
    I2C_SOFT_Stop(hdl);

    return SOFT_I2C_ERR_OK;
}
