/**
 * @file example_main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-12-26
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "cm_os.h"
#include "cm_sys.h"
#include "cm_uart.h"
#include "cm_iomux.h"
#include "cm_gpio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define UART_RX_LEN           128
#define TEST_UART_DEV         CM_UART_DEV_2

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void test_uart_callback(void *param, uint32_t event);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile uint8_t g_uart_event_flag = 0;
static volatile uint32_t g_uart_event_type = 0;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * @brief UART 测试回调函数
 */
static void test_uart_callback(void *param, uint32_t event)
{
    (void)param;
    /* 回调函数中不应进行打印等耗时操作 */
    g_uart_event_type = event;
    g_uart_event_flag = 1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(void)
{
    int32_t ret;
    uint8_t tx_data[2];
    uint8_t rx_data[UART_RX_LEN];
    cm_uart_cfg_t config;
    cm_uart_cfg_t get_config;
    cm_uart_event_t event;

    cm_printf("CM UART test starts\n");

    /* 注意短接 PD_6 / PD_7 */

    /* 配置引脚复用 */
    cm_printf("=== Configure IOMUX ===\n");
    ret = cm_iomux_set_pin_func(CM_GPIO_PD_6, CM_IOMUX_PD_6_MUX_UART2_RX);
    cm_printf("cm_iomux_set_pin_func(PD_6, UART2_RX) ret=%d\n", ret);

    ret = cm_iomux_set_pin_func(CM_GPIO_PD_7, CM_IOMUX_PD_7_MUX_UART2_TX);
    cm_printf("cm_iomux_set_pin_func(PD_7, UART2_TX) ret=%d\n", ret);

    /* 配置参数 */
    cm_printf("\n=== Configure UART ===\n");
    config = (cm_uart_cfg_t) {
        .byte_size = CM_UART_BYTE_SIZE_8,
        .parity = CM_UART_PARITY_NONE,
        .stop_bit = CM_UART_STOP_BIT_TWO,
        .flow_ctrl = CM_UART_FLOW_CTRL_NONE,
        .baudrate = CM_UART_BAUDRATE_921600,
        .is_lpuart = 0,          /* 普通串口模式 */
        .rxrb_buf_size = 0,      /* 环形缓存区大小按默认配置 */
        .fc_high_threshold = 0,
        .fc_low_threshold = 0,
    };
    cm_printf("UART config: baudrate=%u, byte_size=%d, parity=%d, stop_bit=%d, flow_ctrl=%d\n",
                   config.baudrate, config.byte_size, config.parity, config.stop_bit, config.flow_ctrl);

    /* 注册事件和回调函数 */
    cm_printf("\n=== Register event ===\n");
    event = (cm_uart_event_t) {
        .event_type = CM_UART_EVENT_TYPE_RX_ARRIVED | CM_UART_EVENT_TYPE_RX_OVERFLOW,
        .event_param = "uart2",
        .event_entry = test_uart_callback,
    };
    ret = cm_uart_register_event(TEST_UART_DEV, &event);
    cm_printf("cm_uart_register_event() ret=%d\n", ret);

    /* 打开串口 */
    cm_printf("\n=== Open UART ===\n");
    ret = cm_uart_open(TEST_UART_DEV, &config);
    if (ret != 0) {
        cm_printf("cm_uart_open() failed, ret=%d\n", ret);
        return 0;
    }
    cm_printf("cm_uart_open() success\n");

    /* 获取配置验证 */
    cm_printf("\n=== Get configuration ===\n");
    ret = cm_uart_get_cfg(TEST_UART_DEV, &get_config);
    if (ret != 0) {
        cm_printf("cm_uart_get_cfg() failed, ret=%d\n", ret);
    } else {
        cm_printf("cm_uart_get_cfg() success:\n");
        cm_printf("  baudrate=%u\n", get_config.baudrate);
        cm_printf("  byte_size=%d\n", get_config.byte_size);
        cm_printf("  parity=%d\n", get_config.parity);
        cm_printf("  stop_bit=%d\n", get_config.stop_bit);
        cm_printf("  flow_ctrl=%d\n", get_config.flow_ctrl);
        cm_printf("  is_lpuart=%u\n", get_config.is_lpuart);
        cm_printf("  rxrb_buf_size=%u\n", get_config.rxrb_buf_size);
        cm_printf("  fc_high_threshold=%u\n", get_config.fc_high_threshold);
        cm_printf("  fc_low_threshold=%u\n", get_config.fc_low_threshold);
    }

    g_uart_event_flag = 0;

    /* 发送测试数据 */
    cm_printf("\n=== Write data ===\n");
    tx_data[0] = 0xAA;
    tx_data[1] = 0x55;
    ret = cm_uart_write(TEST_UART_DEV, tx_data, 2, 0);
    if (ret < 0) {
        cm_printf("cm_uart_write() failed, ret=%d\n", ret);
    } else {
        cm_printf("cm_uart_write() success, sent=%d bytes, data=[0x%02X, 0x%02X]\n",
                       ret, tx_data[0], tx_data[1]);
    }

    /* 查询发送状态 */
    cm_printf("\n=== Check sending status ===\n");
    ret = cm_uart_is_sending(TEST_UART_DEV);
    cm_printf("cm_uart_is_sending()=%d\n", ret);

    /* 接收数据 */
    cm_printf("\n=== Read data ===\n");
    cm_printf("Waiting for data... (please send data to UART2)\n");

    /* 检查事件标志并处理具体事件 */
    while (g_uart_event_flag == 0) {
        osDelay(100);
    }

    cm_printf("UART event received: 0x%x\n", g_uart_event_type);
    if (g_uart_event_type & CM_UART_EVENT_TYPE_RX_ARRIVED) {
        cm_printf("  - RX arrived event\n");
    }
    if (g_uart_event_type & CM_UART_EVENT_TYPE_RX_OVERFLOW) {
        cm_printf("  - RX overflow event\n");
    }
    if (g_uart_event_type & CM_UART_EVENT_TYPE_RX_FLOWCTRL) {
        cm_printf("  - RX flowctrl event\n");
    }
    g_uart_event_flag = 0;
    g_uart_event_type = 0;

    /* 获取环形缓冲区待读数据长度 */
    cm_printf("\n=== Get RX buffer length ===\n");
    ret = cm_uart_get_rxrb_data_len(TEST_UART_DEV);
    cm_printf("cm_uart_get_rxrb_data_len()=%d\n", ret);

    ret = cm_uart_read(TEST_UART_DEV, rx_data, UART_RX_LEN, 0);
    if (ret < 0) {
        cm_printf("cm_uart_read() failed, ret=%d\n", ret);
    } else {
        cm_printf("cm_uart_read() success, received=%d bytes\n", ret);
        cm_printf("Data: ");
        for (int32_t i = 0; (i < ret) && (i < 16); i++) {
            cm_printf("%02X ", rx_data[i]);
        }
        cm_printf("\n");
    }

    /* 关闭串口 */
    cm_printf("\n=== Close UART ===\n");
    ret = cm_uart_close(TEST_UART_DEV);
    if (ret != 0) {
        cm_printf("cm_uart_close() failed, ret=%d\n", ret);
    } else {
        cm_printf("cm_uart_close() success\n");
    }

    cm_printf("\nCM UART test ends\n");
    return 0;
}
