/**
 * @file example_main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-12-25
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/****************************************************************************
* Included Files
****************************************************************************/

#include <stdlib.h>
#include "cm_os.h"
#include "cm_sys.h"
#include "cm_i2c.h"
#include "cm_iomux.h"
#include "cm_gpio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PT8311_ADDR             (0x44)
#define PT8311_REG_SRST         (0x01)
#define PT8311_REG_DAC_DVC_CTRL (0x3B)

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
****************************************************************************/

int main(void)
{
    int32_t ret;
    uint8_t tx_buf[2];
    uint8_t rx_buf[1];
    cm_i2c_cfg_t config;

    cm_printf("CM I2C test starts\n");

    /* e837 开发板打开音频电源 */
    cm_iomux_set_pin_func(CM_GPIO_AON_4, CM_IOMUX_FUNC_FUNCTION0);
    cm_gpio_set_level(CM_GPIO_AON_4, 1);
    osDelay(100);

    /* 配置I2C2引脚复用 */
    /* CM_GPIO_PD_16 对应 i2c2_scl */
    cm_iomux_set_pin_func(CM_GPIO_PD_16, CM_IOMUX_FUNC_FUNCTION3 /* PD_16_MUX_I2C2_SCL */);
    cm_printf("Set CM_GPIO_PD_16 to I2C2_SCL\n");

    /* CM_GPIO_PD_17 对应 PD_GPIO_17，用作 i2c2_sda */
    cm_iomux_set_pin_func(CM_GPIO_PD_17, CM_IOMUX_FUNC_FUNCTION3 /* PD_17_MUX_I2C2_SDA */);
    cm_printf("Set CM_GPIO_PD_17 to I2C2_SDA\n");

    /* 配置I2C参数 */
    config.addr_type = CM_I2C_ADDR_TYPE_7BIT;
    config.mode = CM_I2C_MODE_MASTER;
    config.clk = CM_I2C_CLK_100KHZ;

    cm_printf("Opening I2C device 2...\n");
    ret = cm_i2c_open(CM_I2C_DEV_2, &config);
    cm_printf("cm_i2c_open(CM_I2C_DEV_2) ret=%d\n", ret);

    if (ret != 0) {
        cm_printf("I2C open failed, ret=%d\n", ret);
        return 0;
    }

    /* 软复位：向 PT8311_REG_SRST 写 0x01 */
    tx_buf[0] = PT8311_REG_SRST;
    tx_buf[1] = 0x01;
    cm_printf("Writing soft reset: reg=0x%02x, val=0x%02x\n", tx_buf[0], tx_buf[1]);
    ret = cm_i2c_write(CM_I2C_DEV_2, PT8311_ADDR, tx_buf, 2);
    cm_printf("cm_i2c_write soft reset ret=%d\n", ret);

    /* 等待复位完成 */
    cm_printf("Waiting 200ms for reset...\n");
    osDelay(200);

    /* 从 PT8311_REG_DAC_DVC_CTRL 读 1 字节并打印输出 */
    tx_buf[0] = PT8311_REG_DAC_DVC_CTRL;
    cm_printf("Reading from register 0x%02x...\n", tx_buf[0]);
    ret = cm_i2c_write(CM_I2C_DEV_2, PT8311_ADDR, tx_buf, 1);
    cm_printf("cm_i2c_write register address ret=%d\n", ret);

    ret = cm_i2c_read(CM_I2C_DEV_2, PT8311_ADDR, rx_buf, 1);
    cm_printf("cm_i2c_read ret=%d, value=0x%02x\n", ret, rx_buf[0]);

    /* 向 PT8311_REG_DAC_DVC_CTRL 写 0x00 */
    tx_buf[0] = PT8311_REG_DAC_DVC_CTRL;
    tx_buf[1] = 0x00;
    cm_printf("Writing to register: reg=0x%02x, val=0x%02x\n", tx_buf[0], tx_buf[1]);
    ret = cm_i2c_write(CM_I2C_DEV_2, PT8311_ADDR, tx_buf, 2);
    cm_printf("cm_i2c_write ret=%d\n", ret);

    /* 从 PT8311_REG_DAC_DVC_CTRL 读 1 字节并打印输出，检查和刚刚的数值是否相同 */
    tx_buf[0] = PT8311_REG_DAC_DVC_CTRL;
    cm_printf("Reading from register 0x%02x again...\n", tx_buf[0]);
    ret = cm_i2c_write(CM_I2C_DEV_2, PT8311_ADDR, tx_buf, 1);
    cm_printf("cm_i2c_write register address ret=%d\n", ret);

    ret = cm_i2c_read(CM_I2C_DEV_2, PT8311_ADDR, rx_buf, 1);
    cm_printf("cm_i2c_read ret=%d, value=0x%02x\n", ret, rx_buf[0]);

    if (rx_buf[0] == 0x00) {
        cm_printf("Read value matches written value (0x00): PASS\n");
    } else {
        cm_printf("Read value (0x%02x) does not match written value (0x00): FAIL\n", rx_buf[0]);
    }

    cm_printf("CM I2C test ends\n");
    return 0;
}
