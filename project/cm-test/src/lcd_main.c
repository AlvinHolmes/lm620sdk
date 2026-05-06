/**
 * @file example_main.c
 * @brief LCD测试例程，ST7735S
 * @date 2025-01-29
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
#include "cm_lcd.h"
#include "cm_iomux.h"
#include "cm_gpio.h"
#include "cm_pm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BOARD_E837N_V00 0
#define BOARD_E837N_V01 1

#define BOARD_VERSION BOARD_E837N_V01

/* 阻塞/非阻塞模式选择: 0=阻塞, 1=非阻塞 */
// #define LCD_BLOCK_MODE 0     /* 阻塞模式 */
#define LCD_BLOCK_MODE 1    /* 非阻塞模式 */

/* LCD 引脚定义 */
#if (BOARD_VERSION == BOARD_E837N_V00)
/* 暂未测试 */
// #define LCD_PIN_CSX        CM_GPIO_PD_12
// #define LCD_PIN_SCL        CM_GPIO_PD_13
// #define LCD_PIN_SDO        CM_GPIO_PD_15
// #define LCD_PIN_DCX        CM_GPIO_PD_14
// #define LCD_PIN_RESET      CM_GPIO_PD_24
// #define LCD_PIN_BACKLIGHT  CM_GPIO_AON_0
#elif (BOARD_VERSION == BOARD_E837N_V01)
#define LCD_PIN_CSX        CM_GPIO_PD_12
#define LCD_PIN_SCL        CM_GPIO_PD_13
#define LCD_PIN_SDO        CM_GPIO_PD_15
#define LCD_PIN_DCX        CM_GPIO_PD_14
#define LCD_PIN_RESET      CM_GPIO_AON_1
#define LCD_PIN_BACKLIGHT  CM_GPIO_AON_7
#define LCD_PIN_POWER      CM_GPIO_PD_22 /* 对应 boot 引脚  */
#endif

#define LCD_WIDTH          128
#define LCD_HEIGHT         128

/* RGB565 颜色定义 */
#define COLOR_BLACK        0x0000
#define COLOR_WHITE        0xFFFF
#define COLOR_RED          0xF800
#define COLOR_GREEN        0x07E0
#define COLOR_BLUE         0x001F
#define COLOR_YELLOW       0xFFE0
#define COLOR_CYAN         0x07FF
#define COLOR_MAGENTA      0xF81F

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void lcd_pin_init(void);
static void lcd_backlight_on(void);
static void lcd_drv_init(void);
static void lcd_reset_sequence(void);
static void lcd_st7735_reg_init(void);
static void lcd_set_area(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static void lcd_test_show_colorbar(void);
static void lcd_transfer_cb(int event, void *user_param);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static osSemaphoreId_t g_lcd_sem = NULL;
static uint16_t s_lcdbuffer0[LCD_WIDTH * LCD_HEIGHT];
static uint16_t s_lcdbuffer1[LCD_WIDTH * LCD_HEIGHT];
static uint32_t g_flush_start_time = 0;
static uint32_t g_flush_count = 0;
static uint32_t g_flush_total_time = 0;
static uint32_t g_flush_min_time = UINT32_MAX;
static uint32_t g_flush_max_time = 0;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void lcd_pin_init(void)
{
    cm_gpio_cfg_t gpio_cfg;

    cm_iomux_set_pin_func(CM_GPIO_PD_12, CM_IOMUX_PD_12_MUX_SPI_LCD_CSX);
    cm_iomux_set_pin_func(CM_GPIO_PD_13, CM_IOMUX_PD_13_MUX_SPI_LCD_SCL);
    cm_iomux_set_pin_func(CM_GPIO_PD_15, CM_IOMUX_PD_15_MUX_SPI_LCD_SDO);
    cm_iomux_set_pin_func(CM_GPIO_PD_14, CM_IOMUX_PD_14_MUX_SPI_LCD_DCX);
#if (BOARD_VERSION == BOARD_E837N_V00)
    /* 暂未测试 */
#elif (BOARD_VERSION == BOARD_E837N_V01)
    cm_iomux_set_pin_func(CM_GPIO_AON_1, CM_IOMUX_AON_1_MUX_GPIO);
    cm_iomux_set_pin_func(CM_GPIO_AON_7, CM_IOMUX_AON_7_MUX_GPIO);
    cm_iomux_set_pin_func(CM_GPIO_PD_22, CM_IOMUX_PD_22_MUX_PD_GPIO_22);
#endif

    /* 初始化 LCD_POWER 为输出 */
    gpio_cfg.direction = CM_GPIO_DIRECTION_OUTPUT;
    gpio_cfg.pull = CM_GPIO_PULL_NONE;
    cm_gpio_init(LCD_PIN_POWER, &gpio_cfg);
    cm_gpio_set_level(LCD_PIN_POWER, CM_GPIO_LEVEL_HIGH);

    /* 初始化 RESET 为输出 */
    gpio_cfg.direction = CM_GPIO_DIRECTION_OUTPUT;
    gpio_cfg.pull = CM_GPIO_PULL_NONE;
    cm_gpio_init(LCD_PIN_RESET, &gpio_cfg);
    cm_gpio_set_level(LCD_PIN_RESET, CM_GPIO_LEVEL_HIGH);

    /* 初始化 BACKLIGHT 为输出 */
    cm_gpio_init(LCD_PIN_BACKLIGHT, &gpio_cfg);
    cm_gpio_set_level(LCD_PIN_BACKLIGHT, CM_GPIO_LEVEL_LOW);
    cm_printf("lcd_pin_init done\r\n");
}

static void lcd_backlight_on(void)
{
    cm_gpio_set_level(LCD_PIN_BACKLIGHT, CM_GPIO_LEVEL_HIGH);
    cm_printf("lcd_backlight_on\r\n");
}

static void lcd_drv_init(void)
{
    uint8_t value;
    /* SPI 单向模式 */
    value = 0;
    cm_lcd_cfg(CM_LCD_CFG_SPI_BIDIR, &value);
    /* DCX 专用引脚模式 */
    value = 1;
    cm_lcd_cfg(CM_LCD_CFG_SPI_DCX, &value);
    /* 虚拟周期 */
    value = 8;
    cm_lcd_cfg(CM_LCD_CFG_READ_DMY_CYC, &value);
    /* 采样时钟沿 - 下降沿 */
    value = 1;
    cm_lcd_cfg(CM_LCD_CFG_SAMP_SEL, &value);
    /* RGB565 字节交换 */
    value = 1;
    cm_lcd_cfg(CM_LCD_CFG_RGB565_SWAP, &value);
}

static void lcd_reset_sequence(void)
{
    cm_gpio_set_level(LCD_PIN_RESET, CM_GPIO_LEVEL_HIGH);
    osDelay(50);
    cm_gpio_set_level(LCD_PIN_RESET, CM_GPIO_LEVEL_LOW);
    osDelay(120);
    cm_gpio_set_level(LCD_PIN_RESET, CM_GPIO_LEVEL_HIGH);
    osDelay(120);
    cm_printf("lcd_reset_sequence done\r\n");
}

static void lcd_st7735_reg_init(void)
{
    uint8_t data[32];
    /* Sleep Out */
    cm_lcd_write_cmd_and_data(0x11, NULL, 0);
    osDelay(120);
    /* Enter Test Mode */
    data[0] = 0xa5;
    cm_lcd_write_cmd_and_data(0xff, data, 1);
    /* 0x3E */
    data[0] = 0x08;
    cm_lcd_write_cmd_and_data(0x3E, data, 1);
    /* 0x3A */
    data[0] = 0x65;
    cm_lcd_write_cmd_and_data(0x3A, data, 1);
    /* 0x82 */
    data[0] = 0x00;
    cm_lcd_write_cmd_and_data(0x82, data, 1);
    /* 0x98 */
    data[0] = 0x00;
    cm_lcd_write_cmd_and_data(0x98, data, 1);
    /* 0x63 */
    data[0] = 0x0f;
    cm_lcd_write_cmd_and_data(0x63, data, 1);
    /* 0x64 */
    data[0] = 0x0f;
    cm_lcd_write_cmd_and_data(0x64, data, 1);
    /* 0xB4 */
    data[0] = 0x34;
    cm_lcd_write_cmd_and_data(0xB4, data, 1);
    /* 0xB5 */
    data[0] = 0x30;
    cm_lcd_write_cmd_and_data(0xB5, data, 1);
    /* 0x83 */
    data[0] = 0x03;
    cm_lcd_write_cmd_and_data(0x83, data, 1);
    /* 0x86 */
    data[0] = 0x04;
    cm_lcd_write_cmd_and_data(0x86, data, 1);
    /* 0x87 */
    data[0] = 0x16;
    cm_lcd_write_cmd_and_data(0x87, data, 1);
    /* 0x88 VCOM */
    data[0] = 0x28;
    cm_lcd_write_cmd_and_data(0x88, data, 1);
    /* 0x89 */
    data[0] = 0x2F;
    cm_lcd_write_cmd_and_data(0x89, data, 1);
    /* 0x93 */
    data[0] = 0x63;
    cm_lcd_write_cmd_and_data(0x93, data, 1);
    /* 0x96 */
    data[0] = 0x81;
    cm_lcd_write_cmd_and_data(0x96, data, 1);
    /* 0xC3 */
    data[0] = 0x11;
    cm_lcd_write_cmd_and_data(0xC3, data, 1);
    /* 0xE6 */
    data[0] = 0x00;
    cm_lcd_write_cmd_and_data(0xE6, data, 1);
    /* 0x99 */
    data[0] = 0x01;
    cm_lcd_write_cmd_and_data(0x99, data, 1);
    /* VRP gamma setting 0x70 */
    data[0]  = 0x02;
    data[1]  = 0x0E;
    data[2]  = 0x0a;
    data[3]  = 0x12;
    data[4]  = 0x19;
    data[5]  = 0x1D;
    data[6]  = 0x46;
    data[7]  = 0x0B;
    data[8]  = 0x0E;
    data[9]  = 0x3D;
    data[10] = 0x05;
    data[11] = 0x07;
    data[12] = 0x12;
    data[13] = 0x0B;
    data[14] = 0x0B;
    data[15] = 0x08;
    cm_lcd_write_cmd_and_data(0x70, data, 16);
    /* VRN gamma setting 0xa0 */
    data[0]  = 0x1E;
    data[1]  = 0x3F;
    data[2]  = 0x0A;
    data[3]  = 0x0D;
    data[4]  = 0x08;
    data[5]  = 0x23;
    data[6]  = 0x3D;
    data[7]  = 0x04;
    data[8]  = 0x09;
    data[9]  = 0x30;
    data[10] = 0x0A;
    data[11] = 0x0E;
    data[12] = 0x0E;
    data[13] = 0x07;
    data[14] = 0x2D;
    data[15] = 0x10;
    cm_lcd_write_cmd_and_data(0xa0, data, 16);
    /* Exit Test Mode */
    data[0] = 0x00;
    cm_lcd_write_cmd_and_data(0xff, data, 1);
    /* Sleep Out again */
    cm_lcd_write_cmd_and_data(0x11, NULL, 0);
    osDelay(200);
    /* Memory Access Control */
    data[0] = 0x08;
    cm_lcd_write_cmd_and_data(0x36, data, 1);
    /* Display On */
    cm_lcd_write_cmd_and_data(0x29, NULL, 0);
    osDelay(200);
    cm_printf("lcd_st7735_reg_init done\r\n");
}

static void lcd_set_area(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t x[2];
    uint16_t y[2];
    x[0] = (x0 >> 8) | (x0 << 8);
    x[1] = (x1 >> 8) | (x1 << 8);
    cm_lcd_write_cmd_and_data(0x2A, (unsigned char *)x, 4);
    y[0] = (y0 >> 8) | (y0 << 8);
    y[1] = (y1 >> 8) | (y1 << 8);
    cm_lcd_write_cmd_and_data(0x2B, (unsigned char *)y, 4);
}

static void lcd_test_show_colorbar(void)
{
    int i, j;
    uint16_t colors[] = {
        COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_BLACK,
        COLOR_WHITE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA
    };
    uint32_t color_count = sizeof(colors) / sizeof(colors[0]);
    for (j = 0; j < LCD_HEIGHT; j++) {
        for (i = 0; i < LCD_WIDTH; i++) {
            s_lcdbuffer0[i + j * LCD_WIDTH] = colors[(j * color_count) / LCD_HEIGHT];
        }
    }
    lcd_set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    cm_lcd_write_buf((unsigned char *)s_lcdbuffer0, LCD_WIDTH * LCD_HEIGHT * 2);
    cm_printf("lcd_test_show_colorbar done\r\n");
}

static void lcd_transfer_cb(int event, void *user_param)
{
    (void)event;
    (void)user_param;
    osSemaphoreRelease(g_lcd_sem);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(void)
{
    /* 避免睡眠模式导致输出异常 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_work_lock();

    osDelay(1000);

    /* 创建传输完成信号量 */
    g_lcd_sem = osSemaphoreNew(1, 0, NULL);

    cm_printf("=== LCD Test Start ===\r\n");
    /* 引脚初始化 */
    lcd_pin_init();
    /* 驱动配置 */
    lcd_drv_init();

    /* cm_lcd_set_block/cm_lcd_set_cb 必须在 cm_lcd_init 之前调用 */    
    /* 设置阻塞/非阻塞模式 */
    cm_lcd_set_block(LCD_BLOCK_MODE);
#if (LCD_BLOCK_MODE == 1)
    /* 非阻塞模式需要设置回调 */
    cm_lcd_set_cb(lcd_transfer_cb, NULL);
#endif

    /* LCD 驱动初始化 */
    cm_lcd_init();
    /* LCD 复位 */
    lcd_reset_sequence();
#if 0 /* 测试用的屏幕不支持 */
    uint32_t lcd_id;
    /* 读取 LCD ID */
    lcd_id = cm_lcd_read_id();
    cm_printf("LCD ID: 0x%08lx\r\n", lcd_id);
#endif
    /* ST7735 寄存器初始化 */
    lcd_st7735_reg_init();
    /* 打开背光 */
    lcd_backlight_on();
    /* 显示彩条 */
    lcd_test_show_colorbar();
    osDelay(1000);

    /* 测速示例 */
    cm_printf("=== SPEED Example ===\r\n");
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        s_lcdbuffer0[i] = COLOR_RED;
        s_lcdbuffer1[i] = COLOR_YELLOW;
    }
    uint16_t *buffers[] = {s_lcdbuffer0, s_lcdbuffer1};
    uint32_t buf_idx = 0;
    while (1) {
        g_flush_start_time = osKernelGetTickCount();
        lcd_set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
#if (LCD_BLOCK_MODE == 1)
        cm_lcd_write_buf((unsigned char *)buffers[buf_idx], LCD_WIDTH * LCD_HEIGHT * 2);
        /* 可以做别的 */
        osSemaphoreAcquire(g_lcd_sem, osWaitForever);
#else
        cm_lcd_write_buf((unsigned char *)buffers[buf_idx], LCD_WIDTH * LCD_HEIGHT * 2);
#endif
        uint32_t flush_end_time = osKernelGetTickCount();
        uint32_t flush_duration = flush_end_time - g_flush_start_time;
        g_flush_count++;
        g_flush_total_time += flush_duration;
        if (flush_duration < g_flush_min_time) {
            g_flush_min_time = flush_duration;
        }
        if (flush_duration > g_flush_max_time) {
            g_flush_max_time = flush_duration;
        }
        if (g_flush_count >= 1000) {
            uint32_t avg_time = g_flush_total_time / g_flush_count;
            float max_fps = 1000.0f / avg_time;
            cm_printf("Flush Performance: Avg=%lums, Min=%lums, Max=%lums, MaxFPS=%.2f\r\n",
                      avg_time, g_flush_min_time, g_flush_max_time, max_fps);
            g_flush_count = 0;
            g_flush_total_time = 0;
            g_flush_min_time = UINT32_MAX;
            g_flush_max_time = 0;
        }
        buf_idx = (buf_idx + 1) % 2;
    }
    return 0;
}
