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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cm_os.h"
#include "cm_sys.h"
#include "cm_spi.h"
#include "cm_iomux.h"
#include "cm_gpio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TEST_SPI_DEV         CM_SPI_DEV_0
#define TEST_DATA_LEN        128

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint8_t g_tx_data[TEST_DATA_LEN];
static uint8_t g_rx_data[TEST_DATA_LEN];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(void)
{
    int32_t ret;
    uint32_t i;
    bool pass;
    cm_spi_cfg_t config;

    cm_printf("CM SPI test starts\n");

    cm_printf("=== Configure IOMUX ===\n");
    cm_iomux_set_pin_func(CM_GPIO_PD_0, CM_IOMUX_PD_0_MUX_SSP0_CLK);
    cm_iomux_set_pin_func(CM_GPIO_PD_2, CM_IOMUX_PD_2_MUX_SSP0_TXD);
    cm_iomux_set_pin_func(CM_GPIO_PD_1, CM_IOMUX_PD_1_MUX_SSP0_RXD);
    cm_iomux_set_pin_func(CM_GPIO_PD_3, CM_IOMUX_PD_3_MUX_SSP0_CS);

    cm_printf("\n=== Configure SPI ===\n");
    config = (cm_spi_cfg_t) {
        .mode = CM_SPI_MODE_MASTER,
        .work_mode = CM_SPI_WOKR_MODE_0,
        .data_width = CM_SPI_DATA_WIDTH_8BIT,
        .nss = CM_SPI_NSS_HARD,
        .clk = CM_SPI_CLK_13MHZ,
    };
    cm_printf("SPI config: mode=%d, work_mode=%d, data_width=%d, nss=%d, clk=%u\n",
                   config.mode, config.work_mode, config.data_width, config.nss, config.clk);

    cm_printf("\n=== Open SPI ===\n");
    ret = cm_spi_open(TEST_SPI_DEV, &config);
    if (ret != 0) {
        cm_printf("cm_spi_open() failed, ret=%d\n", ret);
        return 0;
    }
    cm_printf("cm_spi_open() success\n");

    cm_printf("\n=== Prepare test data ===\n");
    for (i = 0; i < TEST_DATA_LEN; i++) {
        g_tx_data[i] = (uint8_t)i;
        g_rx_data[i] = 0;
    }
    cm_printf("Test data prepared: %u bytes\n", TEST_DATA_LEN);

    cm_printf("\n=== Write data ===\n");
    ret = cm_spi_write(TEST_SPI_DEV, g_tx_data, TEST_DATA_LEN);
    if (ret < 0) {
        cm_printf("cm_spi_write() failed, ret=%d\n", ret);
        goto spi_stop;
    }
    cm_printf("cm_spi_write() success, sent=%d bytes\n", ret);

    cm_printf("\n=== Read data ===\n");
    ret = cm_spi_read(TEST_SPI_DEV, g_rx_data, TEST_DATA_LEN);
    if (ret < 0) {
        cm_printf("cm_spi_read() failed, ret=%d\n", ret);
        goto spi_stop;
    }
    cm_printf("cm_spi_read() success, received=%d bytes\n", ret);

    cm_printf("\n=== Verify data ===\n");
    cm_printf("Received data:\n");
    for (i = 0; i < TEST_DATA_LEN; i += 16) {
        uint32_t j;
        uint32_t count = ((i + 16) <= TEST_DATA_LEN) ? 16 : (TEST_DATA_LEN - i);
        char line_buf[128];
        int pos = 0;
        for (j = 0; j < count; j++) {
            pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, "%02X ", g_rx_data[i + j]);
        }
        cm_printf("%s\n", line_buf);
    }

    cm_printf("\n=== Test write_then_read ===\n");
    memset(g_rx_data, 0, TEST_DATA_LEN);
    ret = cm_spi_write_then_read(TEST_SPI_DEV, g_tx_data, TEST_DATA_LEN, g_rx_data, TEST_DATA_LEN);
    if (ret != 0) {
        cm_printf("cm_spi_write_then_read() failed, ret=%d\n", ret);
        goto spi_stop;
    }
    cm_printf("cm_spi_write_then_read() success\n");

    cm_printf("\n=== Verify write_then_read data ===\n");
    pass = true;
    for (i = 0; i < TEST_DATA_LEN; i++) {
        if (g_rx_data[i] != g_tx_data[i]) {
            cm_printf("Data mismatch at index %u: tx=0x%02X, rx=0x%02X\n",
                           i, g_tx_data[i], g_rx_data[i]);
            pass = false;
            break;
        }
    }
    if (pass) {
        cm_printf("Write_then_read data verification PASSED\n");
    } else {
        cm_printf("Write_then_read data verification FAILED\n");
    }

spi_stop:
    cm_printf("\n=== Close SPI ===\n");
    ret = cm_spi_close(TEST_SPI_DEV);
    if (ret != 0) {
        cm_printf("cm_spi_close() failed, ret=%d\n", ret);
    } else {
        cm_printf("cm_spi_close() success\n");
    }

    cm_printf("\nCM SPI test ends\n");
    return 0;
}
