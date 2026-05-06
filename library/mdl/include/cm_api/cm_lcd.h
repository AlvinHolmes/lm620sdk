/**
 * @file        cm_lcd.h
 * @brief       内存管理
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By surui
 * @date        2023/08/17
 *
 * @defgroup lcd
 * @ingroup lcd
 * @{
 */

#ifndef __CM_LCD_H__
#define __CM_LCD_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "stdint.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define LCD_SPI_DCX_VAL       0
#define LCD_READ_DMY_CYC_VAL  9
#define LCD_SPI_BIDIR_VAL     0

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#if 1 /* 扩展接口，谨慎使用 */
/* 错误码定义 */
typedef enum
{
    CM_LCD_ERR_OK              = 0,   /*!< 成功 */
    CM_LCD_ERR_FAIL           = -1,   /*!< 失败 */
    CM_LCD_ERR_PARAM          = -2,   /*!< 参数错误 */
    CM_LCD_ERR_NOT_INIT       = -3,   /*!< 未初始化 */
    CM_LCD_ERR_BUSY           = -4,   /*!< 忙碌 */
    CM_LCD_ERR_ALREADY_INIT   = -5,   /*!< 已初始化 */
}cm_lcd_err_e;

typedef enum
{
    CM_LCD_EVENT_DONE = 0, /* cm_lcd_write_buf2 完成 */
}cm_lcd_event_e;

typedef void (*cm_lcd_callback_t)(int event, void *user_param);

/**
 * @brief 设置 cm_lcd_write_buf 传输完成回调
 *
 * @param [in] cb      回调函数指针
 * @param [in] user_param 用户参数
 *
 * @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 * @details 传输完成后会调用 cb 函数，event 为 CM_LCD_EVENT_DONE
 */
int cm_lcd_set_cb(cm_lcd_callback_t cb, void *user_param);

typedef enum
{
    CM_LCD_BLOCK_MODE_NONBLOCK = 0,  /*!< 非阻塞，调用 cm_lcd_write_buf 后立即返回。传输任务正在进行时，再次调用此接口立刻返回错误码 */
    CM_LCD_BLOCK_MODE_BLOCK,         /*!< （默认）阻塞，调用 cm_lcd_write_buf 后直到传输完成后才返回 */
}cm_lcd_block_mode_e;

/**
 * @brief 设置 cm_lcd_write_buf 阻塞模式
 *
 * @param [in] block_mode 阻塞模式
 *
 * @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 * @details 默认为 CM_LCD_BLOCK_MODE_BLOCK 阻塞模式
 */
int cm_lcd_set_block(int block_mode);

/* LCD 配置类型 */
typedef enum
{
    CM_LCD_CFG_SPI_BIDIR,     /*!< SPI 双向模式，0=单向 1=双向，默认0 */
    CM_LCD_CFG_SPI_DCX,       /*!< DCX 模式，0=编码在数据中 1=专用引脚，默认0 */
    CM_LCD_CFG_SAMP_SEL,      /*!< 采样时钟沿，0=上升沿 1=下降沿，默认0 */
    CM_LCD_CFG_READ_DMY_CYC,  /*!< 读操作虚拟周期，范围0-15，默认9 */
    CM_LCD_CFG_RGB565_SWAP,   /*!< RGB565 字节交换，0=正常序 1=交换序，默认0 */
}cm_lcd_cfg_type_e;

/**
 * @brief LCD 配置接口
 *
 * @param [in] type  配置类型
 * @param [in] value 配置值指针，指向存有配置值的变量
 *
 * @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 * @details CM_LCD_CFG_SPI_BIDIR ~ CM_LCD_CFG_RGB565_SWAP 需在 cm_lcd_init 之前调用菜生效。
 * 用法：
 *   uint8_t value = 1;
 *   cm_lcd_cfg(CM_LCD_CFG_SPI_BIDIR, &value);
 */
int cm_lcd_cfg(cm_lcd_cfg_type_e type, void *value);

/**
 * @brief 获取最后一次操作的错误码
 *
 * @return 错误码，参见 cm_lcd_err_e
 *
 * @details More details
 */
int cm_lcd_get_last_error(void);

/**
 * @brief LCD 读取数据
 *
 * @param [in]  cmd  需要发送的命令
 * @param [out] data 接收数据的缓冲区
 * @param [in]  len  数据长度
 *
 * @return
 *   =  0 - 成功 \n
 *   < 0 - 失败，返回错误码
 *
 * @details More details
 */
int cm_lcd_read(unsigned char cmd, unsigned char *data, int len);

/**
 * @brief 发送刷屏数据（指定命令）
 *
 * @param [in] cmd  需要发送的命令
 * @param [in] data 需要发送的数据
 * @param [in] len  数据长度
 *
 * @return
 *   =  0 - 成功 \n
 *   < 0 - 失败，返回错误码
 *
 * @details 此接口默认阻塞传输，传输完才会返回。如需改变行为，请在 cm_lcd_init 之前
 *          调用 `cm_lcd_set_block(CM_LCD_BLOCK_MODE_NONBLOCK)` 。
 */
int cm_lcd_write_buf2(unsigned char cmd, unsigned char *data, int len);
#endif /* 扩展接口，谨慎使用 */

/**
 * @brief 配置时发送cmd
 *  
 * @param [in] cmd 需要发送的命令
 *
 * @return 
 *
 * @details More details
 */
void cm_lcd_write_cmd(unsigned char cmd);

/**
 * @brief 配置时发送data
 *  
 * @param [in] data 需要发送的数据
 *
 * @return 
 *
 * @details 此平台只支持 cmd 和 data 一起发，因此调用此接口会先发一个 0x00 的 cmd 
 *          然后发送 data 。
 *          不推荐使用此接口，推荐使用 cm_lcd_write_cmd_and_data 。
 */
void cm_lcd_write_data(unsigned char data);

/**
 * @brief 发送刷屏数据
 *  
 * @param [in] data 需要发送的数据
 * @param [in] len  数据长度
 *
 * @return 
 *
 * @details 此接口默认阻塞传输，传输完才会返回。如需改变行为，请在 cm_lcd_init 之前
 *          调用 `cm_lcd_set_block(CM_LCD_BLOCK_MODE_NONBLOCK)` 。
 *          调用此接口默认会发送一个 `0x2C` 命令后再发送数据。
 *          如需改变 `0x2C` 命令，请调用 cm_lcd_write_buf 。
 */
void cm_lcd_write_buf(unsigned char *data, int len);

/**
 * @brief lcd 驱动初始化
 * 
 *
 * @return 
 *
 * @details More details
 */
void cm_lcd_init(void);

/**
 * @brief lcd 驱动去初始化
 * 
 *
 * @return 
 *
 * @details More details
 */
void cm_lcd_deinit(void);

/**
 * @brief lcd 获取ID
 * 
 *
 * @return 
 *   > 0 - lcd ID \n
 *   = 0 - 失败
 *
 * @details 只适配从 0x04 寄存器读取 3 字节的情况，如 ST7735S
 */
uint32_t cm_lcd_read_id(void);

/**
 * @brief lcd 发送cmd和data
 *
 * @param [in] cmd 需要发送的命令
 * @param [in] data 需要发送的数据
 * @param [in] len  数据长度
 *
 * @return 
 *
 * @details More details
 */
void cm_lcd_write_cmd_and_data(unsigned char cmd, unsigned char *data, int len);


#endif
