/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_lcd_te.h
 *
 * @brief       输入设备.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team         创建
 ************************************************************************************
 */

#ifndef _DRV_LCD_TE_H_
#define _DRV_LCD_TE_H_

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <os.h>
#include <os_workqueue.h>
#include <stdbool.h>

/**
 * @addtogroup LcdTe
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

#define LCD_TE_AUTO_CALC                // te时长自适应（开启该选项，需要同步开启LCD_TE_AUTO_RATE）
#define LCD_TE_AUTO_RATE                // 刷新率自适应（建议常开）
#define LCD_TE_ASYNC                    // 异步发送数据

#define LCD_TE_DEFINE(                   \
    config,                                  \
    teRes, tetMux, wken, irqn)               \
                                             \
    LCD_TePinConfig config = {               \
        .te = {.res = teRes, .mux = tetMux}, \
        .wakeEnable = wken,                  \
        .irqNum = irqn,                      \
    }
/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum {
    LCD_TE_WAIT_MSG = 1,
    LCD_TE_MSG_WAIT_TE,
    LCD_TE_MSG_WAIT_TE_OK,
    LCD_TE_MSG_XFER_END,
    LCD_TE_MSG_MAX,
}LCD_TE_MSG;

#define LCD_TE_EVENT_WAIT_TE   (0x1 << 0)

typedef struct 
{
    LCD_TE_MSG msg;
    struct osSemaphore signal;
}LCD_TeMsg;

typedef struct 
{
    const void *res; 
    uint8_t mux;
}LCD_TePin;

typedef struct {
    LCD_TePin    te;
    uint8_t wakeEnable : 1;             // 是否支持唤醒
    uint8_t irqNum;                     // 支持唤醒时，唤醒中断ID
}LCD_TePinConfig;

typedef struct {
    void  *userData;
    void (*cbEvent)(void *userData);

    uint8_t         monitorTeLevel;
    osThread_t      threadId;
    uint32_t        num;

    LCD_TE_MSG      msg;

    LCD_TeMsg       message;

    struct osSemaphore signal;

#ifdef LCD_TE_ASYNC
    uint8_t         *data;
    uint32_t        len;
    void (*flush)(void *userData, uint8_t *pixel, uint32_t len);
    void            *asyncThread;
#endif

#ifdef LCD_TE_AUTO_CALC
    uint8_t         step;
#endif

    void (*teEnable)(void *userData, bool en);

#ifdef LCD_TE_AUTO_RATE
    struct osWork   work;
    uint8_t         porchMaxVal;
    uint8_t         porchMinVal;
    uint8_t         fpsMaxVal;
    uint8_t         fpsMinVal;
    void (*porchFunc)(void *userData, uint8_t porch);
    void (*fpsFunc)(void *userData, uint8_t fps);
#endif
}LCD_TeHandle;


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

void LCD_TePinInit(LCD_TePinConfig *teConfig);
void LCD_TeInit(LCD_TeHandle *handle, void (*callback)(void *), void (*asyncFlush)(void *, uint8_t *, uint32_t ), void *param);
void LCD_TeDeInit(LCD_TeHandle *handle);
void LCD_TeAcquire(LCD_TeHandle *handle);
void LCD_TeRelease(LCD_TeHandle *handle);

void LCD_TeAcquireAsync(LCD_TeHandle *handle, uint8_t *data, uint32_t len);
#endif
/** @} */