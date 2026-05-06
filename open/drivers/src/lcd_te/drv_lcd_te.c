// #include <st7789v.h>
#include <drv_lcd_te.h>
#include <drv_pin.h>
#include <drv_pcu.h>

#include <slog_print.h>
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
// #define SHELL_Debug(fmt, ...) osPrintf("[%-5d]    %-15s %-25s :" fmt "\r\n", __LINE__, __FILE__, __func__, ##__VA_ARGS__)
// #define SHELL_Info(fmt, ...)  osPrintf("[%-5d]    %-15s %-25s :" "\033[" "32m" fmt "\r\n" "\033[0m", __LINE__, __FILE__, __func__, ##__VA_ARGS__)
// #define SHELL_Info(fmt, ...)  osPrintf("\033[" "32m" fmt "\r\n" "\033[0m", ##__VA_ARGS__)
// #define SHELL_Error(fmt, ...) osPrintf("[%-5d]    %-15s %-25s :" "\033[" "31m" fmt "\r\n" "\033[0m", __LINE__, __FILE__, __func__, ##__VA_ARGS__)
// #define SHELL_Debug(fmt, ...)

#define SHELL_Debug(format, ...)        //slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_UI_FRAME, format, ##__VA_ARGS__)
#define SHELL_Info(format, ...)         slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_UI_FRAME, format, ##__VA_ARGS__)
#define SHELL_Warning(format, ...)      //slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_UI_FRAME, format, ##__VA_ARGS__)
#define SHELL_Error(format, ...)        //slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_UI_FRAME, format, ##__VA_ARGS__)

#define LCD_TE_FLAG                     (1)

#define LCD_TE_PIN_RES(config)          ((config)->te.res)
#define LCD_TE_PIN_MUX(config)          ((config)->te.mux)
#define LCD_TE_PIN_WAKEABLE(config)     ((config)->wakeEnable)
#define LCD_TE_PIN_WAKE_IRQn(config)    ((config)->irqNum)
#define LCD_TE_PIN_WAKEINT_LVL(level)   (level ? ICT_PCU_HIGH_LEVEL : ICT_PCU_LOW_LEVEL)

// 时长自适应
#define LCD_TE_CALC_START           0
#define LCD_TE_CALC_END             1
#define LCD_TE_CALC_MONITOR         2
#define LCD_TE_CALC_MONITOR_PASS    3
#define LCD_TE_CALC_MONITOR_NOPASS  4

static inline void LCD_TeMsgSend(LCD_TeMsg *msg, uint32_t type)
{
    msg->msg = type;
    osComplete(&msg->signal);

    // osWorkSubmit(ptr, 0)
}

static inline void LCD_TeMsgInit(LCD_TeMsg *msg)
{
    memset(msg, 0, sizeof(LCD_TeMsg));
    osInitCompletion(&msg->signal);
}

static inline void LCD_TeMsgDeInit(LCD_TeMsg *msg)
{
    osSemaphoreDetach(&msg->signal);
}

static inline void LCD_TeMsgWait(LCD_TeMsg *msg, uint32_t timeout)
{
    osWaitForCompletion(&msg->signal, timeout);
}

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum {
    LCD_LE_LEVEL_MODE = 0,
    LCD_LE_EDGE_MODE,
}LCD_LE_MODE;

typedef struct {
    uint8_t     leValid;
    LCD_LE_MODE mode;
    uint8_t     irqMode;
}LCD_TeMap;

typedef enum {
    LCD_TE_MAP_VALID = 0,
    LCD_TE_MAP_INVALID,
    LCD_TE_MAP_MAX,
}LCD_TE_MAP;
/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
static LCD_TePinConfig *g_tePinCfg = NULL;

static LCD_TeMap g_lcdTeMap[LCD_TE_MAP_MAX] = {
    {GPIO_HIGH, LCD_LE_EDGE_MODE, PIN_IRQ_MODE_RISING},
    {GPIO_LOW, LCD_LE_LEVEL_MODE, PIN_IRQ_MODE_LOW_LEVEL},
};

static LCD_TeMap g_lcdTeAonMap[LCD_TE_MAP_MAX] = {
    {GPIO_HIGH, LCD_LE_LEVEL_MODE, ICT_PCU_HIGH_LEVEL},
    {GPIO_LOW, LCD_LE_LEVEL_MODE, ICT_PCU_LOW_LEVEL},
};


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
static void LCD_TeIqrEnable(LCD_TeHandle *handle)
{
    GPIO_ModifyIrqMode(LCD_TE_PIN_RES(g_tePinCfg), g_lcdTeMap[LCD_TE_MAP_VALID].irqMode);

    if(LCD_TE_PIN_WAKEABLE(g_tePinCfg)) {
        PCU_WakeupIrqClrpending(LCD_TE_PIN_WAKE_IRQn(g_tePinCfg));
        osInterruptClrPending(LCD_TE_PIN_WAKE_IRQn(g_tePinCfg));
        PCU_WakeupIrqRegister(LCD_TE_PIN_WAKE_IRQn(g_tePinCfg), LCD_TE_PIN_WAKEINT_LVL(g_lcdTeMap[LCD_TE_MAP_VALID].irqMode));

        osInterruptUnmask(LCD_TE_PIN_WAKE_IRQn(g_tePinCfg));
    }
    else {
        GPIO_IrqEnable(LCD_TE_PIN_RES(g_tePinCfg), 1);
    }
}

static void LCD_TeIrqDisable(LCD_TeHandle *handle)
{
    if(LCD_TE_PIN_WAKEABLE(g_tePinCfg)) {
        PCU_WakeupIrqClrpending(LCD_TE_PIN_WAKE_IRQn(g_tePinCfg));
        osInterruptMask(LCD_TE_PIN_WAKE_IRQn(g_tePinCfg));
        PCU_WakeupIrqUnregister(LCD_TE_PIN_WAKE_IRQn(g_tePinCfg));
    }
    else {
        GPIO_IrqEnable(LCD_TE_PIN_RES(g_tePinCfg), 0);
    }
}

static void LCD_TeModifyIrqMode(LCD_TeHandle *handle, uint32_t mode)
{
    if(LCD_TE_PIN_WAKEABLE(g_tePinCfg)) {
        // todo
    }
    else {
        GPIO_ModifyIrqMode(LCD_TE_PIN_RES(g_tePinCfg), mode);
    }
}

#ifdef LCD_TE_AUTO_CALC
static void LCD_TePinPdStep(LCD_TeHandle *handle)
{
    switch (handle->step) {
        case LCD_TE_CALC_START:
            handle->num++;

            LCD_TeMsgSend(&handle->message, LCD_TE_MSG_WAIT_TE_OK);

            if(g_lcdTeMap[LCD_TE_MAP_VALID].irqMode == LCD_LE_LEVEL_MODE) {
                handle->step = LCD_TE_CALC_END;
                LCD_TeModifyIrqMode(handle, g_lcdTeMap[LCD_TE_MAP_INVALID].irqMode);
            }
            else {
                handle->step = LCD_TE_CALC_MONITOR;
            }
            break;

        case LCD_TE_CALC_END:
            // SHELL_Debug("te end %d\r\n", GPIO_Read(LCD_TE_PIN_RES(g_tePinCfg)));
            handle->step = LCD_TE_CALC_MONITOR;
            LCD_TeModifyIrqMode(handle, g_lcdTeMap[LCD_TE_MAP_VALID].irqMode);
            break;

        case LCD_TE_CALC_MONITOR:
            // SHELL_Info("te protect error , num = %d, gpio level = %d, proch = 0x%x", handle->num, GPIO_Read(LCD_TE_PIN_RES(g_tePinCfg)), handle->porchMaxVal);
            SHELL_Info("te protect error , num = %d, proch = 0x%x", handle->num, handle->porchMaxVal);
            handle->porchMaxVal += 0x1;
            LCD_TeIrqDisable(handle);
            handle->step = LCD_TE_CALC_MONITOR_NOPASS;
            break;
        
        case LCD_TE_CALC_MONITOR_PASS:
            SHELL_Debug("te protect ok, num = %d", handle->num);
            LCD_TeIrqDisable(handle);
            break;
        
        default:
            break;
    }
}
#endif

static void LCD_TePinAonGpioCallback(int vector, void *param)
{
    LCD_TeHandle *handle = (LCD_TeHandle *)param;

    LCD_TeIrqDisable(handle);

    handle->num++;

    LCD_TeMsgSend(&handle->message, LCD_TE_MSG_WAIT_TE_OK);
}

static void LCD_TePinPdGpioCallback(void *param)
{
    LCD_TeHandle *handle = (LCD_TeHandle *)param;

#ifdef LCD_TE_AUTO_CALC
    LCD_TePinPdStep(handle);
#else
    LCD_TeIrqDisable(handle);
    handle->num++;
    LCD_TeMsgSend(&handle->message, LCD_TE_MSG_WAIT_TE_OK);
#endif
}

static void LCD_TePinMuxInit(LCD_TeHandle *handle)
{
    int irq_id = LCD_TE_PIN_WAKE_IRQn(g_tePinCfg);
    void *res = (void *)LCD_TE_PIN_RES(g_tePinCfg);

    if(LCD_TE_PIN_WAKEABLE(g_tePinCfg)) {
        osInterruptUninstall(irq_id);
        osInterruptMask(irq_id);
        PCU_WakeupIrqUnregister(irq_id);
        
        PCU_WakeupIrqClrpending(irq_id);
        osInterruptClrPending(irq_id);
        PCU_WakeupIrqRegister(irq_id, g_lcdTeAonMap[LCD_TE_MAP_VALID].irqMode);

        osInterruptConfig(irq_id, 1, IRQ_HIGH_LEVEL);
        osInterruptInstall(irq_id, LCD_TePinAonGpioCallback,  handle);
    }
    else {
        GPIO_DetachIrq(res);
        GPIO_IrqEnable(res, 0);
        PIN_SetMux(res, LCD_TE_PIN_MUX(g_tePinCfg));
        PIN_SetPull(res, PULL_UP);
        GPIO_SetDir(res, GPIO_INPUT);

        GPIO_AttachIrq(res, g_lcdTeMap[LCD_TE_MAP_VALID].irqMode, LCD_TePinPdGpioCallback, handle);
    }

    PIN_SetMux(res, LCD_TE_PIN_MUX(g_tePinCfg));
}

static void _LCD_TeAcquire(LCD_TeHandle *handle)
{
#ifdef LCD_TE_AUTO_RATE
    if(handle->porchFunc) {
        handle->porchFunc(handle->userData, handle->porchMaxVal);
    }
    if(handle->fpsFunc) {
        handle->fpsFunc(handle->userData, handle->fpsMinVal);
    }
#endif

    if(handle->teEnable) {
        handle->teEnable(handle->userData, true);
    }

#ifdef LCD_TE_AUTO_CALC
    handle->step = LCD_TE_CALC_START;
#endif
}

#ifdef LCD_TE_AUTO_RATE
static void _LCD_TeRelease(void *arg)
{
    LCD_TeHandle *handle = (LCD_TeHandle *)arg;

    if(handle->porchFunc) {
        handle->porchFunc(handle->userData, handle->porchMinVal);
    }
    if(handle->fpsFunc) {
        handle->fpsFunc(handle->userData, handle->fpsMaxVal);
    }

    if(handle->teEnable) {
        handle->teEnable(handle->userData, false);
    }

    if(handle->cbEvent) {
        handle->cbEvent(handle->userData);
    }
}
#endif

static void LCD_TeThreadEntry(void *param)
{
    LCD_TeHandle *handle = (LCD_TeHandle *)param;

    while (1)
    {
        LCD_TeMsgWait(&handle->message, osWaitForever);

        switch (handle->message.msg) {
            case LCD_TE_MSG_WAIT_TE_OK:
                SHELL_Debug("get te signal");
                if(handle->flush) {
                    handle->flush(handle->userData, handle->data, handle->len);
                }
                break;
#ifdef LCD_TE_AUTO_RATE
            case LCD_TE_MSG_XFER_END:
                _LCD_TeRelease(handle);
                break;
#endif

            default:
                break;
        }
    }
}

void LCD_TePinInit(LCD_TePinConfig *teConfig)
{
    g_tePinCfg = teConfig;
}

void LCD_TeInit(LCD_TeHandle *handle, void (*callback)(void *), void (*asyncFlush)(void *, uint8_t *, uint32_t ), void *param)
{
    OS_ASSERT(g_tePinCfg);

    LCD_TePinMuxInit(handle);

    handle->userData = param;
    handle->cbEvent = callback;

#ifdef LCD_TE_AUTO_RATE
    // memset(&handle->work, 0, sizeof(struct osWork)); 
    // osWorkInit(&handle->work, _LCD_TeRelease, handle, OS_FALSE);
#endif

    if(handle->teEnable) {
        handle->teEnable(handle->userData, false);
    }

    LCD_TeMsgInit(&handle->message);

#ifdef LCD_TE_ASYNC
    OS_ASSERT(asyncFlush);
    handle->flush = asyncFlush;
    osThreadAttr_t attr = {"lcd_te", osThreadDetached, NULL, 0U, NULL, 1024, osPriorityISR - 10, 0U, 0U};
    handle->asyncThread = osThreadNew(LCD_TeThreadEntry, handle, &attr);
    OS_ASSERT(handle->asyncThread);
#endif
}

void LCD_TeDeInit(LCD_TeHandle *handle)
{
#ifdef LCD_TE_ASYNC
    osThreadTerminate(handle->asyncThread);
#endif
    LCD_TeMsgDeInit(&handle->message);
}

void LCD_TeAcquire(LCD_TeHandle *handle)
{
    LCD_TeMsgSend(&handle->message, LCD_TE_MSG_WAIT_TE);

    LCD_TeMsgWait(&handle->message, osWaitForever);

    // osThreadFlagsWait(LCD_TE_FLAG, osFlagsWaitAny, osWaitForever);
}

void LCD_TeAcquireAsync(LCD_TeHandle *handle, uint8_t *data, uint32_t len)
{
#ifdef LCD_TE_AUTO_RATE
    _LCD_TeAcquire(handle);
#endif

#ifdef LCD_TE_ASYNC
    handle->data = data;
    handle->len = len;
#endif

    SHELL_Debug("te acquire");

    LCD_TeIqrEnable(handle);
}

void LCD_TeRelease(LCD_TeHandle *handle)
{
#ifdef LCD_TE_AUTO_CALC
    if(handle->step == LCD_TE_CALC_MONITOR) {
        handle->step = LCD_TE_CALC_MONITOR_PASS;
    }
#endif

    SHELL_Debug("flush end, process te release");

#ifdef LCD_TE_AUTO_RATE
    LCD_TeMsgSend(&handle->message, LCD_TE_MSG_XFER_END);
#else
    if(handle->cbEvent) {
        handle->cbEvent(handle->userData);
    }
#endif
}
