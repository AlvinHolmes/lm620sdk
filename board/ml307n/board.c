/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        board.c
 *
 * @brief       板级配置实现.
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
#include <os.h>
#include <drv_uart.h>
#include <drv_pin.h>
#include "board.h"
#include <drv_icp.h>
#include <drv_uicc.h>
#include "nr_micro_shell.h"
#if defined(_CPU_AP)
#include <nvm.h>
#endif


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define MUX_PORT_CFG_MAGIC         (0x78789A9A)


#if defined(_CPU_AP) && defined(USE_DUAL_CARD)
#define BUS_USING_SECOND_CARD
#endif

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct UICC_PlugRes{
    uint8_t wakeEnable : 1;             // 是否支持唤醒
    uint8_t init : 1;
    UICC_CARD_NUMBER cardNumber;        // 卡槽
    uint8_t normMuxId;                  // 普通GPIO复用号
    uint8_t monitorMuxId;               // 支持唤醒时，唤醒GPIO号
    uint8_t irqNum;                     // 支持唤醒时，唤醒中断ID
    const struct PIN_Res *pinRes;
}UICC_PlugRes_t;

typedef struct {
    struct PIN_MultiMux reset;
    struct PIN_MultiMux clock;
    struct PIN_MultiMux data;
}UICC_CardRes_t;

#if defined(_CPU_AP)
typedef struct {
    uint32_t      headMagic;
    NV_MuxPortCfg muxPortCfg;
    uint32_t      tailMagic;
} MuxPortCfg;
#endif

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
UART_Handle g_atHandle = {.pRes = DRV_RES(UART, UART_ID_AT)};
UART_Handle g_logHandle = {.pRes = DRV_RES(UART, UART_ID_LOG)};
UART_Handle g_consoleHandle = {.pRes = DRV_RES(UART, UART_ID_CONSOLE)};


#if defined(_CPU_AP) && !defined(USE_TOP_MTS) && !defined(USE_TOP_MINI)
static UICC_PlugRes_t g_uiccCard0PlugRes = {0};
#endif

#ifdef BUS_USING_SECOND_CARD
static UICC_CardRes_t g_uiccCardRes = {0};
#endif
    
static uint32_t g_logUartBaudRate = 12000000;       // LOG UART波特率;
static bool_t g_logUartFlowCtrlEnable = OS_TRUE;    // OS_FALSE:关闭LOG UART流控功能; OS_TRUE:打开LOG UART流控功能
static uint32_t g_dumpUartBaudRate = 6000000;       // DUMP UART波特率;
static bool_t g_dumpUartFlowCtrlEnable = OS_FALSE;  // OS_FALSE:关闭DUMP UART流控功能; OS_TRUE:打开DUMP UART流控功能
bool_t g_dumpLogEnable = OS_FALSE;                  // Dump功能调试日志打印, 通过控制台口打印
bool_t g_consoleOutputEnable = OS_TRUE;             // OS_TRUE: 控制台输出到串口; OS_FALSE: 控制台输出到LOG口;
bool_t g_Flash2_Enable = OS_FALSE;                 // OS_TRUE: 使用外挂flash,不使用SPIMUX; OS_FALSE: 不使用外挂flash;

#if defined(_CPU_AP)
MuxPortCfg g_muxPortCfg = { MUX_PORT_CFG_MAGIC, { 0 },  MUX_PORT_CFG_MAGIC};
#endif

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
extern int lwip_system_init(void);
extern int MiddleWare_AT_Init(void);
extern int SHELL_Init(void);
extern int SHELL_Deinit(void);
extern void UICC_EXT_PLUG_ResRegister(UICC_PlugRes_t *res);
extern void UICC_EXT_PinRegister(UICC_CARD_NUMBER card, struct PIN_MultiMux *reset, struct PIN_MultiMux *clock, struct PIN_MultiMux *data);
extern void drvTest(char argc, char **argv);
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
#ifdef BUS_USING_SECOND_CARD
static int Board_UiccPinResInit(void)
{
    g_uiccCardRes.reset.res = PIN_RES(PIN_29);
    g_uiccCardRes.reset.specMux = PIN_29_MUX_SIM0_RST;
    g_uiccCardRes.reset.gpioMux = PIN_29_MUX_GPIO;

    g_uiccCardRes.clock.res = PIN_RES(PIN_30);
    g_uiccCardRes.clock.specMux = PIN_30_MUX_SIM0_CLK;
    g_uiccCardRes.clock.gpioMux = PIN_30_MUX_GPIO;

    g_uiccCardRes.data.res = PIN_RES(PIN_31);
    g_uiccCardRes.data.specMux = PIN_31_MUX_SIM0_DATA;
    g_uiccCardRes.data.gpioMux = PIN_31_MUX_GPIO;

    UICC_EXT_PinRegister(UICC_CARD_NUMBER_0, &g_uiccCardRes.reset, &g_uiccCardRes.clock, &g_uiccCardRes.data);
    UICC_EXT_PinRegister(UICC_CARD_NUMBER_1, &g_uiccCardRes.reset, &g_uiccCardRes.clock, &g_uiccCardRes.data);

    return 0;
}
INIT_BOARD_EXPORT(Board_UiccPinResInit, OS_INIT_SUBLEVEL_MIDDLE);
#endif

/**
 ************************************************************************************
 * @brief           Console初始化
 *
 * @param[in]       void
 *
 * @return          int32_t
 * @retval          0          成功
 ************************************************************************************
*/
int Board_ConsoleInit(void)
{
    // return 0;     // 关闭CONSOLE功能 直接返回0
    UART_Handle *handle = &g_consoleHandle;

#if defined(_CPU_AP)
    handle->func = UART_PORT_CONSOLE;
    UART_Initialize(handle, NULL);
    UART_PowerControl(handle, DRV_POWER_FULL);
    UART_Control(handle, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 921600);

    osConsoleSetUart(handle);
    osShellSetUart(handle);
#else
    osConsoleSetUart(NULL);
#endif

    return 0;
}
INIT_BOARD_EXPORT(Board_ConsoleInit, OS_INIT_SUBLEVEL_LOW);

void Board_ConsoleReinit(uint16_t msgId, void *data, uint16_t len)
{
    UART_Handle *handle = &g_consoleHandle;

    uint32_t *tmpRegBase = (uint32_t *)data;
    if(NULL != tmpRegBase && 0 != *tmpRegBase)
    {
        for(int32_t i = 0;i < (sizeof(g_UART_Res) / sizeof(struct UART_Res));i++)
        {
            if((uint32_t)DRV_RES(UART, i)->regBase == (*tmpRegBase))
            {
                handle->pRes = DRV_RES(UART, i);
                break;
            }
        }
    }

    handle->func = UART_PORT_CONSOLE;
    UART_Initialize(handle, NULL);
    UART_PowerControl(handle, DRV_POWER_FULL);
    UART_Control(handle, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 921600);

    osConsoleSetUart(handle);
    osShellSetUart(handle);

    SHELL_Init();

#if defined(_CPU_CP) && defined(_PLAT_TEST) && defined(_PLAT_AUTO_TEST)
    drvTest(1, OS_NULL);
#endif
}

#ifdef OS_USING_SHELL
#include <stdlib.h>
#include "nr_micro_shell.h"

void SHELL_switchConsole(char argc, char **argv)
{
    UART_Handle *handle = osConsoleGetUart();

    OS_ASSERT(handle != OS_NULL);

    uint32_t regBase = 0;
    if (NULL != handle && NULL != handle->pRes)
    {
        regBase = (uint32_t)((handle->pRes)->regBase);
    }

    osConsoleSetUart(NULL);

    UART_PowerControl(handle, DRV_POWER_OFF);
    UART_Uninitialize(handle);

    ICP_SendMsg(ICP_MSG_ID_CONSOLE, (uint8_t *)&regBase, sizeof(uint32_t));

    SHELL_Deinit();
}
NR_SHELL_CMD_EXPORT(switch_console, SHELL_switchConsole);
#endif

void * osLogGetUart(void)
{
    // return NULL;     // 关闭LOG功能 直接返回NULL
#if defined(_CPU_AP)
    if ((1 == g_muxPortCfg.muxPortCfg.portSet || 2 == g_muxPortCfg.muxPortCfg.portSet))
    {
        return NULL;
    }
    else
    {
        return &g_logHandle;
    }
#else
    return &g_logHandle;
#endif
}

void* osGetDumpUartRegBase(void)
{
    // 必须返回有效的uart regBase, 否则影响死机dump功能
#if defined(_CPU_AP)
    if (MUX_PORT_CFG_MAGIC == g_muxPortCfg.headMagic
        && MUX_PORT_CFG_MAGIC == g_muxPortCfg.tailMagic
        && 0 != g_muxPortCfg.muxPortCfg.portSet)
    {
        // 启用mux功能
        g_logUartFlowCtrlEnable = OS_FALSE; // evb板子lpuart流控管脚硬件没接
        return (void *)BASE_LP_UART;   //UART0
    }
#endif
    return (void *)BASE_UART0;   //UART1
}

uint32_t Board_GetLogUartBaudRate(void)
{
    return g_logUartBaudRate;
}

__PSRAM_CODE bool_t Board_GetLogUartFlowCtrlEnable(void)
{
    return g_logUartFlowCtrlEnable;
}

uint32_t Board_GetDumpUartBaudRate(void)
{
    return g_dumpUartBaudRate;
}

bool_t Board_GetDumpUartFlowCtrlEnable(void)
{
    return g_dumpUartFlowCtrlEnable;
}

/**
 ************************************************************************************
 * @brief           获取分配给AT的UART句柄
 *
 * @param[in]       void
 *
 * @return          UART_Handle指针
 ************************************************************************************
*/
UART_Handle * Board_GetAtUartHandle(void)
{
    // return NULL;     // 关闭AT功能 直接返回NULL
#if defined(_CPU_AP)
    switch (g_muxPortCfg.muxPortCfg.portSet)
    {
        case 2:
        case 3:
            return NULL;
            break;
        case 0:
        case 1:
        default:
            return &g_atHandle;
            break;
    }
#else
    return NULL;
#endif
}

#if defined(_CPU_AP)
#include "drv_soc.h"
int Board_MuxPortInit(void)
{
    UART_Handle *handle = NULL;

    int32_t ret = NVM_Read(NV_ITEM_ADDR(pubConfig.muxPortCfg), (uint8_t *)&g_muxPortCfg.muxPortCfg, NV_ITEM_SIZE(pubConfig.muxPortCfg));
    if (NVM_OK != ret)
    {
        g_muxPortCfg.muxPortCfg.portSet = 0;
    }

    if (g_muxPortCfg.muxPortCfg.portSet > 3)
    {
        g_muxPortCfg.muxPortCfg.portSet = 0;
    }

    if (MUX_PORT_CFG_MAGIC == g_muxPortCfg.headMagic
        && MUX_PORT_CFG_MAGIC == g_muxPortCfg.tailMagic
        && 0 != g_muxPortCfg.muxPortCfg.portSet)
    {
        // mux功能启用
        handle = osConsoleGetUart();
        if (handle)
        {
            osConsoleSetUart(NULL);
            UART_PowerControl(handle, DRV_POWER_OFF);
            UART_Uninitialize(handle);
        }
    }

    switch (g_muxPortCfg.muxPortCfg.portSet)
    {
        case 1:
        {
            // at
            g_atHandle.pRes = DRV_RES(UART, 0);
            PIN_SetPull(PIN_RES(PIN_5), PULL_UP);
            // 管脚复用配置, PIN_SetMux
        }
            break;
        case 2:
        {
            // console
            handle = &g_consoleHandle;
            handle->pRes = DRV_RES(UART, 0);
            // 管脚复用配置, PIN_SetMux

            handle->func = UART_PORT_CONSOLE;
            UART_Initialize(handle, NULL);
            UART_PowerControl(handle, DRV_POWER_FULL);
            UART_Control(handle, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 921600);

            osConsoleSetUart(handle);
        }
            break;

        case 3:
        {
            // log
            g_logHandle.pRes = DRV_RES(UART, 0);
            g_logUartFlowCtrlEnable = OS_FALSE; // evb板子lpuart流控管脚硬件没接

            // 管脚复用配置, PIN_SetMux
        }
            break;

        default:
            break;
    }

    return 0;
}
INIT_CORE_EXPORT(Board_MuxPortInit, OS_INIT_SUBLEVEL_LOW);
#endif

/**
 ************************************************************************************
 * @brief           SPI MUX 初始化
 *
 * @param[in]       void
 *
 * @return          int
 * @retval          0          成功
 ************************************************************************************
*/
#if defined(_CPU_AP) && defined(USE_TOP_SPIMUX) && !defined(USE_TOP_MTS) && !defined(USE_TOP_MINI)
#define PIN_RES_SPI0_CLK                   PIN_RES(PIN_21)
#define PIN_RES_SPI0_RXD                   PIN_RES(PIN_22)
#define PIN_RES_SPI0_TXD                   PIN_RES(PIN_23)
#define PIN_RES_SPI0_CS                    PIN_RES(PIN_24)
#define PIN_RES_SPIMUX_HRDY                PIN_RES(PIN_9)     // GON_GPIO_0
#define PIN_RES_SPIMUX_DRDY                PIN_RES(PIN_8)     // LPUART0_RTS
#define PIN_RES_SPIMUX_DINT                PIN_RES(PIN_7)     // LPUART0_CTS

typedef struct {
    SPI_Handle           *spi;
    const struct PIN_Res *hrdyRes;
    const struct PIN_Res *drdyRes;
    const struct PIN_Res *dintRes;
    uint8_t               hrdyMuxNorm;
    uint8_t               hrdyMuxInt;
    uint8_t               drdyMux;
    uint8_t               dintMux;

    int                   hrdyIrqNum;

    const struct PIN_Res *spiClkRes;
    const struct PIN_Res *spiCsRes;
    const struct PIN_Res *spiTxRes;
    const struct PIN_Res *spiRxRes;

    uint8_t               spiClkMux;
    uint8_t               spiCsMux;
    uint8_t               spiTxMux;
    uint8_t               spiRxMux;
} SPI_DeviceHandle;

static SPI_Handle       g_spiHandle = {0};
static SPI_DeviceHandle g_spiDeviceHandle = {0};

extern void SPI_MuxInit(SPI_DeviceHandle *handle);

int Board_SpiMuxInit(void)
{
    g_spiDeviceHandle.hrdyRes        = PIN_RES_SPIMUX_HRDY;
    g_spiDeviceHandle.hrdyMuxNorm    = PIN_9_MUX_GPIO;
    g_spiDeviceHandle.hrdyMuxInt     = PIN_9_MUX_AON_INT_0;
    g_spiDeviceHandle.hrdyIrqNum     = OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_13);
    g_spiDeviceHandle.drdyRes        = PIN_RES_SPIMUX_DRDY;
    g_spiDeviceHandle.drdyMux        = PIN_8_MUX_GPIO;
    g_spiDeviceHandle.dintRes        = PIN_RES_SPIMUX_DINT;
    g_spiDeviceHandle.dintMux        = PIN_7_MUX_GPIO;

    g_spiDeviceHandle.spiClkRes      = PIN_RES_SPI0_CLK;
    g_spiDeviceHandle.spiClkMux      = PIN_21_MUX_SSP0_CLK;
    g_spiDeviceHandle.spiRxRes       = PIN_RES_SPI0_RXD;
    g_spiDeviceHandle.spiRxMux       = PIN_22_MUX_SSP0_RXD;
    g_spiDeviceHandle.spiTxRes       = PIN_RES_SPI0_TXD;
    g_spiDeviceHandle.spiTxMux       = PIN_23_MUX_SSP0_TXD;
    g_spiDeviceHandle.spiCsRes       = PIN_RES_SPI0_CS;
    g_spiDeviceHandle.spiCsMux       = PIN_24_MUX_SSP0_CS;

    g_spiHandle.res                  = DRV_RES(SPI, 0);
    g_spiDeviceHandle.spi            = &g_spiHandle;

    SPI_MuxInit(&g_spiDeviceHandle);

    return 0;
}
INIT_BOARD_EXPORT(Board_SpiMuxInit, OS_INIT_SUBLEVEL_HIGH);
#endif


#if defined(_CPU_AP) && !defined(USE_TOP_MTS) && !defined(USE_TOP_MINI)
/**
 ************************************************************************************
 * @brief           SIM卡插拔管脚
 *
 * @param[in]       void
 *
 * @return          int32_t
 * @retval          0          成功
 ************************************************************************************
*/
int Board_UiccPlugResInit(void)
{
    g_uiccCard0PlugRes.pinRes       = PIN_RES_USIM0_DET;
    g_uiccCard0PlugRes.wakeEnable   = 1;
    g_uiccCard0PlugRes.cardNumber   = UICC_CARD_NUMBER_0;
    g_uiccCard0PlugRes.normMuxId    = PIN_10_MUX_GPIO;
    g_uiccCard0PlugRes.monitorMuxId = PIN_10_MUX_AON_INT_1;
    g_uiccCard0PlugRes.irqNum       = OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_14);

    UICC_EXT_PLUG_ResRegister(&g_uiccCard0PlugRes);

    return 0;
}
INIT_BOARD_EXPORT(Board_UiccPlugResInit, OS_INIT_SUBLEVEL_MIDDLE);
#endif

#if defined(_CPU_AP) && defined(USE_AUDIO) && !defined(USE_TOP_MTS) && !defined(USE_TOP_MINI)
#include "audio_pa.h"
static AudioPaCfg g_PaCfg =
{
    .res = NULL,
    .activeLevel = GPIO_LOW,
};
/**
 ************************************************************************************
 * @brief           codec, i2s pin 初始化
 *
 * @param[in]       void
 *
 * @return          int32_t
 * @retval          0          成功
 ************************************************************************************
*/
int Board_AudioPinInit(void)
{
    PA_SetCfg(&g_PaCfg);
    return 0;
}
INIT_BOARD_EXPORT(Board_AudioPinInit, OS_INIT_SUBLEVEL_LOW);
#endif


/**
 ************************************************************************************
 * @brief           中间件启动入口
 *
 * @param[in]       void
 *
 * @return          int
 * @retval          0          成功
 ************************************************************************************
*/
#if defined(_CPU_AP) && !defined(USE_TOP_MTS) && !defined(USE_TOP_MINI)
int middleware_init()
{
    osPrintf("lwip_system_init\r\n");
    lwip_system_init();

    return 0;
}
INIT_MIDDLEWARE_EXPORT(middleware_init, OS_INIT_SUBLEVEL_HIGH);

/**
 ************************************************************************************
 * @brief           AT框架启动入口
 *
 * @param[in]       void
 *
 * @return          int
 * @retval          0          成功
 * @note  AT框架初始化需要在lwip之前, 在应用AT的模块之前
 ************************************************************************************
*/
INIT_COMPONENT_EXPORT(MiddleWare_AT_Init, OS_INIT_SUBLEVEL_6);
#endif

/**
 ************************************************************************************
 * @brief           Shell初始化
 *
 * @param[in]       void
 *
 * @return          int32_t
 * @retval          0          成功
 ************************************************************************************
*/
int Board_ShellInit(void)
{
    SHELL_Init();

    return 0;
}
INIT_APP_EXPORT(Board_ShellInit, OS_INIT_SUBLEVEL_HIGH);