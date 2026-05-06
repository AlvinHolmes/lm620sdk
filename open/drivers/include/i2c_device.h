#ifndef __I2C_DEVICE_H__
#define __I2C_DEVICE_H__

#include <os.h>
#include <drv_common.h>
#include <stdint.h>
#include <drv_i2c.h>
#include <drv_soft_i2c.h>

#if defined(OS_USING_PM)
#include <psm_wakelock.h>
#endif

/**
 * @addtogroup I2cDev
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define I2C_BUS_NUM                         (4)     ///< 控制器总线数量
#define I2C_BUS_USING_SOFT                  (0)     ///< 是否使用软件模拟I2C
#define I2C_BUS_TIMEOUT                     (100)   ///< 超时时间，单位ms

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

typedef struct I2C_BusDeviceS I2C_BusDevice;

/**
 * @brief I2C管脚定义
 */
typedef struct 
{
    const void *sclRes;
    const void *sdaRes;
    uint8_t sclMux;
    uint8_t sdaMux;
}I2C_BusPin;

/**
 * @brief I2C频率
 */
typedef enum {
    I2C_BUS_DEVICE_FREQ_DEFAULT = 0,
    I2C_BUS_DEVICE_FREQ_100K,
    I2C_BUS_DEVICE_FREQ_400K,
    I2C_BUS_DEVICE_FREQ_1M,
    I2C_BUS_DEVICE_FREQ_3_4M,
}I2C_BUS_FREQ;

/**
 * @brief I2C设备
 */
typedef struct
{
    uint8_t         num;
    I2C_BUS_FREQ    freq;
    uint8_t         ref;
    I2C_BusPin      *pin;
    I2C_Handle      i2c;
    I2C_SOFT_Handle softI2c;
    struct osMutex  lock;

#if defined(OS_USING_PM)
    WakeLock        wakeLock;
#endif
    osThread_t      threadId;
}I2C_Bus;

typedef struct I2C_BusDeviceS{
    I2C_BusDevice   *next;

    I2C_Bus         *bus;
    I2C_BUS_FREQ    freq;

    uint16_t        addr;

    void *param;
    
}I2C_BusDevice;

/**
 ************************************************************************************
 * @brief           I2C管脚初始化
 *
 * @return          void
 ************************************************************************************
 */
void I2C_BusPinInit(uint32_t busNum, I2C_BusPin *pin);

/**
 ************************************************************************************
 * @brief           注册I2C设备
 *
 * @param[in]       busNum          总线ID
 * @param[in]       device          I2C设备
 *
 * @return          void
 ************************************************************************************
*/
void I2C_BusDevice_Register(uint32_t busNum, I2C_BusDevice *device);


/**
 ************************************************************************************
 * @brief           注销I2C设备
 *
 * @param[in]       busNum          总线ID
 * @param[in]       device          I2C设备
 *
 * @return          void
 ************************************************************************************
*/
void I2C_BusDevice_UnRegister(uint32_t busNum, I2C_BusDevice *device);

/**
 ************************************************************************************
 * @brief           给I2C设备发送数据
 *
 * @param[in]       device          I2C设备
 * @param[in]       data            发送的数据
 * @param[in]       len             发送的长度
 *
 * @return          int32_t
 * @retval          !0              错误
 *                  0               成功
 ************************************************************************************
*/
int32_t I2C_BusDevice_Write(I2C_BusDevice *device, uint8_t *data, uint32_t len);

/**
 ************************************************************************************
 * @brief           从I2C设备读取某个寄存器的值
 *
 * @param[in]       device          I2C设备
 * @param[in]       reg             寄存器
 * @param[in]       data            读取的数据
 * @param[in]       len             读取的长度
 *
 * @return          int32_t
 * @retval          !0              错误
 *                  0               成功
 ************************************************************************************
*/
int32_t I2C_BusDevice_ReadReg(I2C_BusDevice *device, uint8_t reg, uint8_t *data, uint32_t len);

/**
 ************************************************************************************
 * @brief           I2C设备上锁
 *
 * @param[in]       device          I2C设备
 *
 * @return          int32_t
 * @retval          !0              错误
 *                  0               成功
 ************************************************************************************
*/
int32_t I2C_BusLock(I2C_BusDevice *device);

/**
 ************************************************************************************
 * @brief           I2C设备解锁
 *
 * @param[in]       device          I2C设备
 *
 * @return          int32_t
 * @retval          !0              错误
 *                  0               成功
 ************************************************************************************
*/
int32_t I2C_BusUnlock(I2C_BusDevice *device);



#endif
/** @} */
