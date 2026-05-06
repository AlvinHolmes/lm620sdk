/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_soft_i2c.h
 *
 * @brief       I2C软件模拟驱动接口.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-04-21     ict team          创建
 ************************************************************************************
 */

#ifndef DRIVER_SOFT_I2C_H_
#define DRIVER_SOFT_I2C_H_

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdint.h>
#include <drv_common.h>

/**
 * @addtogroup I2c
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define I2C_SOFT_NAME_LEN              (10)
#define I2C_SOFT_LIST                  (1)

#define SOFT_I2C_ERR_OK                (0U)             // 正常
#define SOFT_I2C_ERR_NACK              (1U)             // 无应答
#define SOFT_I2C_ERR_BUSY              (2U)             // 总线忙
#define SOFT_I2C_ERR_PARAM             (3U)             // 参数错误
#define SOFT_I2C_ERR_TIMEOUT           (4U)             // 超时
#define SOFT_I2C_ERR_WRITE_ADDR        (5U)             // 发送写地址失败
#define SOFT_I2C_ERR_WRITE_REG_ADDR    (6U)             // 发送寄存器地址失败
#define SOFT_I2C_ERR_WRITE_DATA        (7U)             // 写数据失败
#define SOFT_I2C_ERR_READ_ADDR         (8U)             // 发送读地址失败
#define SOFT_I2C_ERR_READ_DATA         (9U)             // 读数据失败

#define SOFT_I2C_TIME_OUT              1000                // 超时时间(us)
#define SOFT_I2C_CLOCK_STRECH_EN  
/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum {
    I2C_SOFT_GPIO_INPUT_MODE = 0,
    I2C_SOFT_GPIO_OUTPUT_MODE,
}I2C_SOFT_GPIO_DIR;

typedef struct {
    void (*init)(void *param);
    void (*scl_write)(void *param, uint8_t value);
    void (*sda_write)(void *param, uint8_t value);
    void (*scl_dir)(void *param, I2C_SOFT_GPIO_DIR dir);
    void (*sda_dir)(void *param, I2C_SOFT_GPIO_DIR dir);
    uint8_t (*sda_read)(void *param);
    uint8_t (*scl_read)(void *param);
} I2C_SOFT_Ops;

typedef struct I2C_SOFT_Handle {
    void *userData;
    I2C_SOFT_Ops *ops;
    uint32_t speed;
    uint32_t speed_us;
#if (I2C_SOFT_LIST)
    char name[I2C_SOFT_NAME_LEN];
    struct I2C_SOFT_Handle *next;
#endif
} I2C_SOFT_Handle;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

#if (I2C_SOFT_LIST)
I2C_SOFT_Handle* I2C_SOFT_Find(const char* name);
#endif

int32_t I2C_SOFT_Initialize(I2C_SOFT_Handle *hdl, const char *name, I2C_SOFT_Ops *ops);
int32_t I2C_SOFT_WriteByte(I2C_SOFT_Handle *hdl, uint8_t byte);
uint8_t I2C_SOFT_ReadByte(I2C_SOFT_Handle *hdl, uint8_t ack);
int32_t I2C_SOFT_WriteBytes(I2C_SOFT_Handle *hdl, uint8_t slave_addr, void *pbuf, uint16_t length);
int32_t I2C_SOFT_WriteRegBytes(I2C_SOFT_Handle *hdl, uint8_t slave_addr, uint8_t reg_addr, void *pbuf, uint16_t length);
int32_t I2C_SOFT_ReadBytes(I2C_SOFT_Handle *hdl, uint8_t slave_addr, uint8_t reg_addr, void *pbuf, uint16_t length);
int32_t I2C_SOFT_WriteBytes_16bit(I2C_SOFT_Handle *hdl, uint8_t slave_addr, uint16_t reg_addr, void *pbuf, uint16_t length);
int32_t I2C_SOFT_ReadBytes_16bit(I2C_SOFT_Handle *hdl, uint8_t slave_addr, uint16_t reg_addr, void *pbuf, uint16_t length);

#endif /* __SF_I2C_H */
/** @} */