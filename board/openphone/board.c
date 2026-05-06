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
#include <drv_spi.h>
#include <drv_pin.h>
#include "board.h"
#include <drv_icp.h>
#include <drv_uicc.h>
#if defined(_CPU_AP)
#include <nvm.h>
#endif


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define BOARD_PIN_RES_PRE_EXPAND(pin)           (&g_PIN_##pin##_Res)
#define PIN_MUX_GPIO_PRE_EXPAND(pin)            (PIN_##pin##_MUX_GPIO)
#define PIN_MUX_I2C_PRE_EXPAND(pin, bus, inf)   (PIN_##pin##_MUX_I2C##bus##_##inf)
#define PIN_MUX_SPI_PRE_EXPAND(pin, bus, inf)   (PIN_##pin##_MUX_SSP##bus##_##inf)
#define PIN_MUX_SPILCD_PRE_EXPAND(pin, inf)     (PIN_##pin##_MUX_SPI_LCD##_##inf)
#define PIN_MUX_CAM_MCLK_PRE_EXPAND(pin)        (PIN_##pin##_MUX_CAM_MCLK)
#define PIN_MUX_SPICAM_PRE_EXPAND(pin, inf)     (PIN_##pin##_MUX_SPI_CAM##_##inf)
#define PIN_MUX_AON_INT_EXPAND(pin, num)        (PIN_##pin##_MUX_AON_INT##_##num)
#define PIN_MUX_SIM_EXPAND(pin, inf)            (PIN_##pin##_MUX##_##inf)
#define PIN_MUX_PWM_EXPAND(pin, inf)            (PIN_##pin##_MUX##_##inf)

#define BOARD_PIN_RES(pin)          BOARD_PIN_RES_PRE_EXPAND(pin)
#define PIN_MUX_GPIO(pin)           PIN_MUX_GPIO_PRE_EXPAND(pin)
#define PIN_MUX_I2C(pin, bus, inf)  PIN_MUX_I2C_PRE_EXPAND(pin, bus, inf)
#define PIN_MUX_SPI(pin, bus, inf)  PIN_MUX_SPI_PRE_EXPAND(pin, bus, inf)
#define PIN_MUX_SPILCD(pin, inf)    PIN_MUX_SPILCD_PRE_EXPAND(pin, inf)
#define PIN_MUX_CAM_MCLK(pin)       PIN_MUX_CAM_MCLK_PRE_EXPAND(pin)
#define PIN_MUX_SPICAM(pin, inf)    PIN_MUX_SPICAM_PRE_EXPAND(pin, inf)
#define PIN_MUX_AON_INT(pin, num)   PIN_MUX_AON_INT_EXPAND(pin, num)
#define PIN_MUX_SIM(pin, inf)       PIN_MUX_SIM_EXPAND(pin, inf)
#define PIN_MUX_PWM(pin, inf)       PIN_MUX_PWM_EXPAND(pin, inf)

#if defined(_CPU_AP)
#define BSP_USING_I2C
#define BSP_I2C_BUS_NUM             2
#endif

#if defined(_CPU_AP) && defined(USE_DUAL_CARD)
#define BUS_USING_SECOND_CARD
#endif

#if defined(_CPU_AP) && defined(USE_TOP_VIDEO)
#ifndef CONFIG_USE_FLASH2
#define BUS_USING_EXT_LDO
#endif
#define BSP_USING_SPILCD
#define BSP_USING_SPICAM
#endif

#if defined(_CPU_AP) && !defined(USE_TOP_MTS) && !defined(USE_TOP_MINI)
#define BSP_USING_SPI
#define BSP_SPI_BUS_NUM             0
#endif

#if defined(_CPU_AP) && defined(USE_TOP_VIDEO)
#define LOG_USE_UART3              // VIDEO版本下, 打开此宏, LOG使用UART3; 关闭此宏, 控制台使用UART3
#endif

//#define CONSOLE_USE_UART2   //D口是控制台

#define MUX_PORT_CFG_MAGIC         (0x78789A9A)

/************************************************************************************
 *                                 管脚分配
 ************************************************************************************/
// 外部LDO管脚定义    （与SPI管脚冲突！！！）
#ifdef BUS_USING_EXT_LDO
#define EXT_LDO_3V3_PIN             PIN_21                          // PD_GPIO_0
#define EXT_LDO_1V8_PIN             PIN_22                          // PD_GPIO_1
#define EXT_LDO_2V8_PIN             PIN_24                          // PD_GPIO_3
#endif

#ifdef BUS_USING_SECOND_CARD
#define CARD_RST_PIN                PIN_29                          // PD_GPIO_8
#define CARD_CLK_PIN                PIN_30                          // PD_GPIO_9
#define CARD_DATA_PIN               PIN_31                          // PD_GPIO_10
#endif

#ifdef BSP_USING_SPI
#define SPI_CLK_PIN                 PIN_21                          // PD_GPIO_0
#define SPI_RX_PIN                  PIN_22                          // PD_GPIO_1
#define SPI_TX_PIN                  PIN_23                          // PD_GPIO_2
#define SPI_CS_PIN                  PIN_24                          // PD_GPIO_3
#define SPI_HRDY_PIN                PIN_9                           // AON_GPIO_1
#define SPI_HRDY_AON_INT_MUX        0
#define SPI_HRDY_AON_INT_NUM        OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_13)
#define SPI_DRDY_PIN                PIN_8                           // PD_GPIO_4
#define SPI_DINT_PIN                PIN_7                           // PD_GPIO_11
#endif

// I2C管脚定义
#ifdef BSP_USING_I2C
#define BSP_I2C_SCL_PIN             PIN_50                          // PD_GPIO_16
#define BSP_I2C_SDA_PIN             PIN_51                          // PD_GPIO_17
#endif

// SPICAM管脚定义
#ifdef BSP_USING_SPICAM
#define SPICAM_POWERDOWN_PIN        PIN_29                          // PD_GPIO_8
#define SPICAM_MCLK_PIN             PIN_11                          // CLK_EXT_OUT
#define SPICAM_CLK_PIN              PIN_30                          // PD_GPIO_9
#define SPICAM_DI0_PIN              PIN_31                          // PD_GPIO_10
#define SPICAM_DI1_PIN              PIN_32                          // PD_GPIO_11
#endif

// SPILCD管脚定义
#ifdef BSP_USING_SPILCD
#define SPILCD_TE_PIN               PIN_13                          // SWD0_SWDIO
#define SPILCD_BACKLIGHT_PIN        PIN_23                          // PD_GPIO_2
#define SPILCD_RESET_PIN            PIN_12                          // SWD0_SWCLK
// #define SPILCD_DCX_PIN              PIN_27                          // PD_GPIO_6
#define SPILCD_CSX_PIN              PIN_33                          // PD_GPIO_12
#define SPILCD_SCL_PIN              PIN_34                          // PD_GPIO_13
#define SPILCD_SDI_PIN              PIN_35                          // PD_GPIO_14
#define SPILCD_SDO_PIN              PIN_36                          // PD_GPIO_15
#endif

/************************************************************************************
 *                                 管脚定义
 ************************************************************************************/
#ifdef BUS_USING_EXT_LDO
#define EXT_LDO_3V3_PIN_RES         BOARD_PIN_RES(EXT_LDO_3V3_PIN)
#define EXT_LDO_3V3_PIN_MUX         PIN_MUX_GPIO(EXT_LDO_3V3_PIN)
#define EXT_LDO_1V8_PIN_RES         BOARD_PIN_RES(EXT_LDO_1V8_PIN)
#define EXT_LDO_1V8_PIN_MUX         PIN_MUX_GPIO(EXT_LDO_1V8_PIN)
#define EXT_LDO_2V8_PIN_RES         BOARD_PIN_RES(EXT_LDO_2V8_PIN)
#define EXT_LDO_2V8_PIN_MUX         PIN_MUX_GPIO(EXT_LDO_2V8_PIN)
#endif

#ifdef BUS_USING_SECOND_CARD
#define CARD_RST_PIN_RES            BOARD_PIN_RES(CARD_RST_PIN)
#define CARD_RST_PIN_MUX            PIN_MUX_SIM(CARD_RST_PIN, SIM0_RST)
#define CARD_RST_NORM_MUX           PIN_MUX_GPIO(CARD_RST_PIN)
#define CARD_CLK_PIN_RES            BOARD_PIN_RES(CARD_CLK_PIN)
#define CARD_CLK_PIN_MUX            PIN_MUX_SIM(CARD_CLK_PIN, SIM0_CLK)
#define CARD_CLK_NORM_MUX           PIN_MUX_GPIO(CARD_CLK_PIN)
#define CARD_DATA_PIN_RES           BOARD_PIN_RES(CARD_DATA_PIN)
#define CARD_DATA_PIN_MUX           PIN_MUX_SIM(CARD_DATA_PIN, SIM0_DATA)
#define CARD_DATA_NORM_MUX           PIN_MUX_GPIO(CARD_DATA_PIN)
#endif

#ifdef BSP_USING_SPI
#define BSP_SPI_CLK_PIN_RES         BOARD_PIN_RES(SPI_CLK_PIN)
#define BSP_SPI_CLK_PIN_MUX         PIN_MUX_SPI(SPI_CLK_PIN, BSP_SPI_BUS_NUM, CLK)
#define BSP_SPI_RX_PIN_RES          BOARD_PIN_RES(SPI_RX_PIN)
#define BSP_SPI_RX_PIN_MUX          PIN_MUX_SPI(SPI_RX_PIN, BSP_SPI_BUS_NUM, RXD)
#define BSP_SPI_TX_PIN_RES          BOARD_PIN_RES(SPI_TX_PIN)
#define BSP_SPI_TX_PIN_MUX          PIN_MUX_SPI(SPI_TX_PIN, BSP_SPI_BUS_NUM, TXD)
#define BSP_SPI_CS_PIN_RES          BOARD_PIN_RES(SPI_CS_PIN)
#define BSP_SPI_CS_PIN_MUX          PIN_MUX_SPI(SPI_CS_PIN, BSP_SPI_BUS_NUM, CS)

#define SPI_HRDY_PIN_RES            BOARD_PIN_RES(SPI_HRDY_PIN)
#define SPI_HRDY_PIN_MUX            PIN_MUX_GPIO(SPI_HRDY_PIN)
#ifdef SPI_HRDY_AON_INT_MUX
#define SPI_HRDY_PIN_INT_MUX        PIN_MUX_AON_INT(SPI_HRDY_PIN, SPI_HRDY_AON_INT_MUX)
#define SPI_HRDY_PIN_INT_NUM        SPI_HRDY_AON_INT_NUM
#endif
#define SPI_DRDY_PIN_RES            BOARD_PIN_RES(SPI_DRDY_PIN)
#define SPI_DRDY_PIN_MUX            PIN_MUX_GPIO(SPI_DRDY_PIN)
#define SPI_DINT_PIN_RES            BOARD_PIN_RES(SPI_DINT_PIN)
#define SPI_DINT_PIN_MUX            PIN_MUX_GPIO(SPI_DINT_PIN)
#endif

#ifdef BSP_USING_I2C
#define BSP_I2C_SCL_PIN_RES         BOARD_PIN_RES(BSP_I2C_SCL_PIN)
#define BSP_I2C_SCL_PIN_MUX         PIN_MUX_I2C(BSP_I2C_SCL_PIN, BSP_I2C_BUS_NUM, SCL)
#define BSP_I2C_SDA_PIN_RES         BOARD_PIN_RES(BSP_I2C_SDA_PIN)
#define BSP_I2C_SDA_PIN_MUX         PIN_MUX_I2C(BSP_I2C_SDA_PIN, BSP_I2C_BUS_NUM, SDA)
#endif

#ifdef BSP_USING_SPICAM
#define SPICAM_POWERDOWN_PIN_RES    BOARD_PIN_RES(SPICAM_POWERDOWN_PIN)
#define SPICAM_POWERDOWN_PIN_MUX    PIN_MUX_GPIO(SPICAM_POWERDOWN_PIN)
#define SPICAM_MCLK_PIN_RES         BOARD_PIN_RES(SPICAM_MCLK_PIN)
#define SPICAM_MCLK_PIN_MUX         PIN_MUX_CAM_MCLK(SPICAM_MCLK_PIN)
#define SPICAM_CLK_PIN_RES          BOARD_PIN_RES(SPICAM_CLK_PIN)
#define SPICAM_CLK_PIN_MUX          PIN_MUX_SPICAM(SPICAM_CLK_PIN, CLK)
#define SPICAM_DI0_PIN_RES          BOARD_PIN_RES(SPICAM_DI0_PIN)
#define SPICAM_DI0_PIN_MUX          PIN_MUX_SPICAM(SPICAM_DI0_PIN, DI0)
#define SPICAM_DI1_PIN_RES          BOARD_PIN_RES(SPICAM_DI1_PIN)
#define SPICAM_DI1_PIN_MUX          PIN_MUX_SPICAM(SPICAM_DI1_PIN, DI1)
#endif

// SPILCD管脚定义
#ifdef BSP_USING_SPILCD
#ifdef SPILCD_TE_PIN
#define SPILCD_TE_PIN_RES           BOARD_PIN_RES(SPILCD_TE_PIN)
#define SPILCD_TE_PIN_MUX           PIN_MUX_GPIO(SPILCD_TE_PIN)
#else
#define SPILCD_TE_PIN_RES           NULL
#define SPILCD_TE_PIN_MUX           0
#endif

#ifdef SPILCD_BACKLIGHT_PIN
#define SPILCD_BACKLIGHT_PIN_RES    BOARD_PIN_RES(SPILCD_BACKLIGHT_PIN)
#define SPILCD_BACKLIGHT_PIN_MUX    PIN_MUX_GPIO(SPILCD_BACKLIGHT_PIN)
#define SPILCD_BACKLIGHT_PIN_PWMMUX PIN_MUX_GPIO(SPILCD_BACKLIGHT_PIN)
#else
#define SPILCD_BACKLIGHT_PIN_RES    NULL
#define SPILCD_BACKLIGHT_PIN_MUX    0
#define SPILCD_BACKLIGHT_PIN_PWMMUX 0
#endif

#ifdef SPILCD_RESET_PIN
#define SPILCD_RESET_PIN_RES        BOARD_PIN_RES(SPILCD_RESET_PIN)
#define SPILCD_RESET_PIN_MUX        PIN_MUX_GPIO(SPILCD_RESET_PIN)
#else
#define SPILCD_RESET_PIN_RES        NULL
#define SPILCD_RESET_PIN_MUX        0
#endif

#ifdef SPILCD_DCX_PIN
#define SPILCD_DCX_PIN_RES          BOARD_PIN_RES(SPILCD_DCX_PIN)
#define SPILCD_DCX_PIN_MUX          PIN_MUX_GPIO(SPILCD_DCX_PIN)
#else
#define SPILCD_DCX_PIN_RES          NULL
#define SPILCD_DCX_PIN_MUX          0
#endif

#define SPILCD_CSX_PIN_RES          BOARD_PIN_RES(SPILCD_CSX_PIN)
#define SPILCD_CSX_PIN_MUX          PIN_MUX_SPILCD(SPILCD_CSX_PIN, CSX)

#define SPILCD_SCL_PIN_RES          BOARD_PIN_RES(SPILCD_SCL_PIN)
#define SPILCD_SCL_PIN_MUX          PIN_MUX_SPILCD(SPILCD_SCL_PIN, SCL)

#ifdef SPILCD_SDI_PIN
#define SPILCD_SDI_PIN_RES          BOARD_PIN_RES(SPILCD_SDI_PIN)
#define SPILCD_SDI_PIN_MUX          PIN_MUX_SPILCD(SPILCD_SDI_PIN, SDI)
#else
#define SPILCD_SDI_PIN_RES          NULL
#define SPILCD_SDI_PIN_MUX          0
#endif

#define SPILCD_SDO_PIN_RES          BOARD_PIN_RES(SPILCD_SDO_PIN)
#define SPILCD_SDO_PIN_MUX          PIN_MUX_SPILCD(SPILCD_SDO_PIN, SDO)
#endif



/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct {
    SPI_Handle           *spi;
    const struct PIN_Res *hrdyRes;
    const struct PIN_Res *drdyRes;
    const struct PIN_Res *dintRes;
    uint8_t         hrdyMuxNorm;
    uint8_t         hrdyMuxInt;
    uint8_t         drdyMux;
    uint8_t         dintMux;

    int             hrdyIrqNum;

    const struct PIN_Res *spiClkRes;
    const struct PIN_Res *spiCsRes;
    const struct PIN_Res *spiTxRes;
    const struct PIN_Res *spiRxRes;

    uint8_t         spiClkMux;
    uint8_t         spiCsMux;
    uint8_t         spiTxMux;
    uint8_t         spiRxMux;
} SPI_MuxHandle;

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

typedef enum {
    EXT_LDO_1V8 = 0,
    EXT_LDO_2V8,
    EXT_LDO_3V3
} EXT_LDO_INDEX;

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

UART_Handle g_consoleHandle;
UART_Handle g_logHandle;
UART_Handle g_atHandle;
#if defined(_CPU_AP) && !defined(USE_TOP_MTS) && !defined(USE_TOP_MINI)
#if defined(BSP_USING_SPI) && defined(USE_TOP_SPI2USB)
static SPI_Handle       g_spiHdl = {0};
static SPI_MuxHandle    g_spimuxHandle = {0};
#endif
static UICC_PlugRes_t g_uiccCard0PlugRes = {0};
#endif
#ifdef BUS_USING_SECOND_CARD
static UICC_CardRes_t g_uiccCardRes = {0};
#endif
static uint32_t g_logUartBaudRate = 12000000;      // LOG UART波特率;
static bool_t g_logUartFlowCtrlEnable = OS_TRUE;   // OS_FALSE:关闭LOG UART流控功能; OS_TRUE:打开LOG UART流控功能
static uint32_t g_dumpUartBaudRate = 12000000;      // DUMP UART波特率;
static bool_t g_dumpUartFlowCtrlEnable = OS_TRUE; // OS_FALSE:关闭DUMP UART流控功能; OS_TRUE:打开DUMP UART流控功能
bool_t g_dumpLogEnable = OS_FALSE;                 // Dump功能调试日志打印, 通过控制台口打印
bool_t g_consoleOutputEnable = OS_FALSE;            // OS_TRUE: 控制台输出到串口; OS_FALSE: 控制台输出到LOG口;
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
extern void SPI_MuxInit(SPI_MuxHandle *hdl);
extern void UICC_EXT_PLUG_ResRegister(UICC_PlugRes_t *res);
extern void UICC_EXT_PinRegister(UICC_CARD_NUMBER card, struct PIN_MultiMux *reset, struct PIN_MultiMux *clock, struct PIN_MultiMux *data);
#if defined(BUS_USING_EXT_LDO)
extern void EXTLDO_PinRegister(PIN_MultiMux_t *ldo_1v8, PIN_MultiMux_t *ldo_2v8, PIN_MultiMux_t *ldo_3v3);
#endif
extern void drvTest(char argc, char **argv);

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
#ifdef BUS_USING_SECOND_CARD
static int Board_UiccPinResInit(void)
{
    g_uiccCardRes.reset.res = CARD_RST_PIN_RES;
    g_uiccCardRes.reset.specMux = CARD_RST_PIN_MUX;
    g_uiccCardRes.reset.gpioMux = CARD_RST_NORM_MUX;

    g_uiccCardRes.clock.res = CARD_CLK_PIN_RES;
    g_uiccCardRes.clock.specMux = CARD_CLK_PIN_MUX;
    g_uiccCardRes.clock.gpioMux = CARD_CLK_NORM_MUX;

    g_uiccCardRes.data.res = CARD_DATA_PIN_RES;
    g_uiccCardRes.data.specMux = CARD_DATA_PIN_MUX;
    g_uiccCardRes.data.gpioMux = CARD_DATA_NORM_MUX;

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
#if defined(_CPU_AP) && defined(USE_TOP_VIDEO) && defined(LOG_USE_UART3)
    osConsoleSetUart(NULL);
#else
#if defined(_CPU_AP)
    //set pin27/28 func pd gpio 6/7,close boot uart
#ifdef CONSOLE_USE_UART2
    /* D口是控制台 */
    PIN_SetMux(PIN_RES(PIN_28), PIN_28_MUX_UART2_TX);
    PIN_SetMux(PIN_RES(PIN_27), PIN_27_MUX_UART2_RX);
#else
    /* C口是控制台 */
    PIN_SetMux(PIN_RES(PIN_28), PIN_28_MUX_GPIO);
    PIN_SetMux(PIN_RES(PIN_27), PIN_27_MUX_GPIO);
#endif

    UART_Handle *huart = &g_consoleHandle;

#ifdef CONSOLE_USE_UART2
    /* D口是控制台 */
    huart->pRes = DRV_RES(UART, 2);
#else
    /* C口是控制台 */
    huart->pRes = DRV_RES(UART, 3);
#endif

    // PIN_SetMux(PIN_RES(PIN_49), PIN_49_MUX_UART3_TX);
    // PIN_SetMux(PIN_RES(PIN_48), PIN_48_MUX_UART3_RX);

    huart->func = UART_PORT_CONSOLE;
    UART_Initialize(huart, NULL);
    UART_PowerControl(huart, DRV_POWER_FULL);
    UART_Control(huart, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 921600);

    osConsoleSetUart(huart);
#else
    osConsoleSetUart(NULL);
#endif
#endif

    return 0;
}
INIT_BOARD_EXPORT(Board_ConsoleInit, OS_INIT_SUBLEVEL_6);

void Board_ConsoleReinit(uint16_t msgId, void *data, uint16_t len)
{
    UART_Handle *huart = &g_consoleHandle;

#ifdef CONSOLE_USE_UART2
    /* D口是控制台 */
    huart->pRes = DRV_RES(UART, 2);
#else
    /* C口是控制台 */
    huart->pRes = DRV_RES(UART, 3);
#endif

    uint32_t *tmpRegBase = (uint32_t *)data;
    if (NULL != tmpRegBase && 0 != *tmpRegBase)
    {
        for (int32_t i = 0; i < (sizeof(g_UART_Res) / sizeof(struct UART_Res)); i++)
        {
            if ((uint32_t)DRV_RES(UART, i)->regBase == (*tmpRegBase))
            {
                huart->pRes = DRV_RES(UART, i);
                break;
            }
        }
    }

    huart->func = UART_PORT_CONSOLE;
    UART_Initialize(huart, NULL);
    UART_PowerControl(huart, DRV_POWER_FULL);
    UART_Control(huart, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 921600);

    osConsoleSetUart(huart);

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
    UART_Handle *huart = osConsoleGetUart();

    OS_ASSERT(huart != OS_NULL);

    uint32_t regBase = 0;
    if (NULL != huart && NULL != huart->pRes)
    {
        regBase = (uint32_t)((huart->pRes)->regBase);
    }

    osConsoleSetUart(NULL);

    UART_PowerControl(huart, DRV_POWER_OFF);
    UART_Uninitialize(huart);

    ICP_SendMsg(ICP_MSG_ID_CONSOLE, (uint8_t *)&regBase, sizeof(uint32_t));

    SHELL_Deinit();
}
NR_SHELL_CMD_EXPORT(switch_console, SHELL_switchConsole);
#endif

/**
 ************************************************************************************
 * @brief           Log初始化
 *
 * @param[in]       void
 *
 * @return          int32_t
 * @retval          0          成功
 ************************************************************************************
*/
int Board_LogInit(void)
{
    UART_Handle *huart = &g_logHandle;

#if defined(_CPU_AP)
    if (MUX_PORT_CFG_MAGIC == g_muxPortCfg.headMagic
        && MUX_PORT_CFG_MAGIC == g_muxPortCfg.tailMagic
        && 0 != g_muxPortCfg.muxPortCfg.portSet)
    {
        // 启用muxPort功能
        return 0;
    }
#endif

#if defined(_CPU_AP) && defined(USE_TOP_VIDEO) && defined(LOG_USE_UART3)
    huart->pRes = DRV_RES(UART, 3);
    // PIN_SetMux(PIN_RES(PIN_49), PIN_49_MUX_UART3_TX);
    // PIN_SetMux(PIN_RES(PIN_48), PIN_48_MUX_UART3_RX);
#else

    huart->pRes = DRV_RES(UART, 1);
#if defined(_CPU_AP)
#ifdef USE_TOP_FPGA
    PIN_SetMux(PIN_RES(PIN_9), PIN_9_MUX_UART1_TX);
    PIN_SetMux(PIN_RES(PIN_33), PIN_33_MUX_UART1_RX);
    PIN_SetMux(PIN_RES(PIN_35), PIN_35_MUX_UART1_CTS);
    PIN_SetMux(PIN_RES(PIN_36), PIN_36_MUX_UART1_RTS);
#else
    // PIN_SetMux(PIN_RES(PIN_34), PIN_34_MUX_UART1_TX);
    // PIN_SetMux(PIN_RES(PIN_33), PIN_33_MUX_UART1_RX);
    // PIN_SetMux(PIN_RES(PIN_35), PIN_35_MUX_UART1_CTS);
    // PIN_SetMux(PIN_RES(PIN_36), PIN_36_MUX_UART1_RTS);
#endif
#endif

#endif

    return 0;
}

void* osLogGetUart(void)
{
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
        return (void *)BASE_LP_UART;
    }
#endif

#if defined(_CPU_AP) && defined(USE_TOP_VIDEO)
    return (void *)BASE_UART3;
#else
    return (void *)BASE_UART0;
#endif
}

INIT_COMPONENT_EXPORT(Board_LogInit, OS_INIT_SUBLEVEL_HIGH);

uint32_t Board_GetLogUartBaudRate(void)
{
    return g_logUartBaudRate;
}

__PSRAM_CODE bool_t Board_GetLogUartFlowCtrlEnable(void)
{
    if (NULL != g_logHandle.pRes && BASE_UART3 == (uint32_t)((g_logHandle.pRes)->regBase))
    {
        return OS_FALSE;
    }
    else
    {
        return g_logUartFlowCtrlEnable;
    }
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
UART_Handle* Board_GetAtUartHandle(void)
{
#if defined(_CPU_AP)
    switch (g_muxPortCfg.muxPortCfg.portSet)
    {
    case 1:
        {
            return &g_atHandle;
        }
        break;

    case 2:
    case 3:
        {
            return NULL;
        }
        break;

    case 0:
    default:
        {
            UART_Handle *huart = &g_atHandle;
            huart->pRes = DRV_RES(UART, 0);
            PIN_SetPull(PIN_RES(PIN_5), PULL_UP);
            return huart;
        }
        break;
    }
#else
    return NULL;
#endif
}

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

#if defined(_CPU_AP)
#include "drv_soc.h"
int Board_MuxPortInit(void)
{
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
        UART_Handle *huart = osConsoleGetUart();
        if (huart)
        {
            osConsoleSetUart(NULL);
            UART_PowerControl(huart, DRV_POWER_OFF);
            UART_Uninitialize(huart);
        }
    }

    //set pin27/28 func pd gpio 6/7,close boot uart
    PIN_SetMux(PIN_RES(PIN_28), PIN_28_MUX_GPIO);
    PIN_SetMux(PIN_RES(PIN_27), PIN_27_MUX_GPIO);

    switch (g_muxPortCfg.muxPortCfg.portSet)
    {
    case 1:
        {
            // at
            UART_Handle *huart = &g_atHandle;
            huart->pRes = DRV_RES(UART, 0);
            PIN_SetPull(PIN_RES(PIN_5), PULL_UP);

            // 管脚复用配置, PIN_SetMux
        }
        break;

    case 2:
        {
            // console
            UART_Handle *huart = &g_consoleHandle;
            huart->pRes = DRV_RES(UART, 0);

            // 管脚复用配置, PIN_SetMux

            huart->func = UART_PORT_CONSOLE;
            UART_Initialize(huart, NULL);
            UART_PowerControl(huart, DRV_POWER_FULL);
            UART_Control(huart, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 921600);

            osConsoleSetUart(huart);
        }
        break;

    case 3:
        {
            // log
            UART_Handle *huart = &g_logHandle;
            huart->pRes = DRV_RES(UART, 0);
            g_logUartFlowCtrlEnable = OS_FALSE; // evb板子lpuart流控管脚硬件没接

            // 管脚复用配置, PIN_SetMux
        }
        break;

    default:
        break;
    }

    return 0;
}
#ifndef CONSOLE_USE_UART2
INIT_CORE_EXPORT(Board_MuxPortInit, OS_INIT_SUBLEVEL_LOW);
#endif
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
 * @brief           SPI MUX 初始化
 *
 * @param[in]       void
 *
 * @return          int
 * @retval          0          成功
 ************************************************************************************
*/
#if defined(BSP_USING_SPI) && defined(USE_TOP_SPI2USB)
int Board_SpiMuxInit(void)
{
    g_spimuxHandle.hrdyRes        = SPI_HRDY_PIN_RES;
    g_spimuxHandle.hrdyMuxNorm    = SPI_HRDY_PIN_MUX;
    g_spimuxHandle.hrdyMuxInt     = SPI_HRDY_PIN_INT_MUX;
    g_spimuxHandle.hrdyIrqNum     = SPI_HRDY_PIN_INT_NUM;
    g_spimuxHandle.drdyRes        = SPI_DRDY_PIN_RES;
    g_spimuxHandle.drdyMux        = SPI_DRDY_PIN_MUX;
    g_spimuxHandle.dintRes        = SPI_DINT_PIN_RES;
    g_spimuxHandle.dintMux        = SPI_DINT_PIN_MUX;

    g_spimuxHandle.spiClkRes      = BSP_SPI_CLK_PIN_RES;
    g_spimuxHandle.spiClkMux      = BSP_SPI_CLK_PIN_MUX;
    g_spimuxHandle.spiRxRes       = BSP_SPI_RX_PIN_RES;
    g_spimuxHandle.spiRxMux       = BSP_SPI_RX_PIN_MUX;
    g_spimuxHandle.spiTxRes       = BSP_SPI_TX_PIN_RES;
    g_spimuxHandle.spiTxMux       = BSP_SPI_TX_PIN_MUX;
    g_spimuxHandle.spiCsRes       = BSP_SPI_CS_PIN_RES;
    g_spimuxHandle.spiCsMux       = BSP_SPI_CS_PIN_MUX;

    g_spiHdl.res                  = DRV_RES(SPI, BSP_SPI_BUS_NUM);
    g_spimuxHandle.spi            = &g_spiHdl;

    SPI_MuxInit(&g_spimuxHandle);

    return 0;
}
INIT_BOARD_EXPORT(Board_SpiMuxInit, OS_INIT_SUBLEVEL_MIDDLE);
#endif

#if defined(_CPU_AP)
/**
 ************************************************************************************
 * @brief           clk io 初始化，关闭时钟输出
 *
 * @param[in]       void
 *
 * @return          int32_t
 * @retval          0          成功
 ************************************************************************************
*/
int Board_ClkioInit(void)
{
    //32k clkio
    PIN_SetMux(PIN_RES(PIN_8), PIN_8_MUX_GPIO);
    GPIO_SetDir(PIN_RES(PIN_8), GPIO_INPUT);
    //26M clkio
    PIN_SetMux(PIN_RES(PIN_11), PIN_11_MUX_GPIO);
    GPIO_SetDir(PIN_RES(PIN_11), GPIO_INPUT);
    //auclk clkio
    PIN_SetMux(PIN_RES(PIN_14), PIN_14_MUX_GPIO);
    GPIO_SetDir(PIN_RES(PIN_14), GPIO_INPUT);
    //aon gpio0
    PIN_SetPull(PIN_RES(PIN_9), PULL_NONE);
    return 0;
}
INIT_COMPONENT_EXPORT(Board_ClkioInit, OS_INIT_SUBLEVEL_HIGH);
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
    g_uiccCard0PlugRes.pinRes       = PIN_RES(PIN_10);
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

#if defined(BUS_USING_EXT_LDO)
static PIN_MultiMux_t ldo_1v8 = {.res = EXT_LDO_1V8_PIN_RES, .gpioMux = EXT_LDO_1V8_PIN_MUX, .specMux = EXT_LDO_1V8_PIN_MUX};
static PIN_MultiMux_t ldo_2v8 = {.res = EXT_LDO_2V8_PIN_RES, .gpioMux = EXT_LDO_2V8_PIN_MUX, .specMux = EXT_LDO_2V8_PIN_MUX};
static PIN_MultiMux_t ldo_3v3 = {.res = EXT_LDO_3V3_PIN_RES, .gpioMux = EXT_LDO_3V3_PIN_MUX, .specMux = EXT_LDO_3V3_PIN_MUX};

int Board_EXTLDO_PinInit(void)
{
    EXTLDO_PinRegister(&ldo_1v8, &ldo_2v8, &ldo_3v3);

    return 0;
}
INIT_BOARD_EXPORT(Board_EXTLDO_PinInit, OS_INIT_SUBLEVEL_MIDDLE);
#endif

#if defined(_CPU_AP) && defined(USE_AUDIO) && !defined(USE_TOP_MTS) && !defined(USE_TOP_MINI)
#include "audio_codec.h"

static CodecPin g_CodecPin =
{
    #if 0
    // volte self test debug
    .codecWs = {PIN_RES(PIN_29), PIN_29_MUX_I2S0_WS},
    .codecSck = {PIN_RES(PIN_30), PIN_30_MUX_I2S0_CLK},
    .codecIn = {PIN_RES(PIN_31), PIN_31_MUX_I2S0_DIN},
    .codecOut = {PIN_RES(PIN_32), PIN_32_MUX_I2S0_DOUT},
    .codecScl = {PIN_RES(PIN_23), PIN_23_MUX_I2C1_SCL},
    .codecSda = {PIN_RES(PIN_24), PIN_24_MUX_I2C1_SDA},
    .busNo = 1,
    #else   //system test
    .codecMclk = {PIN_RES(PIN_14), PIN_14_MUX_CLK_AU_MCLK},
    .codecWs = {PIN_RES(PIN_25), PIN_25_MUX_I2S0_WS},
    .codecSck = {PIN_RES(PIN_26), PIN_26_MUX_I2S0_CLK},
    .codecIn = {PIN_RES(PIN_27), PIN_27_MUX_I2S0_DIN},
    .codecOut = {PIN_RES(PIN_28), PIN_28_MUX_I2S0_DOUT},

    .codecScl = {PIN_RES(PIN_50), PIN_50_MUX_I2C2_SCL},
    .codecSda = {PIN_RES(PIN_51), PIN_51_MUX_I2C2_SDA},
    .busNo = 2,
    #endif
};

#include "audio_pa.h"
static AudioPaCfg g_PaCfg =
{
    .busNo = 2,
    .res = NULL,
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
    Codec_PinBoardInit(&g_CodecPin);
    PA_SetCfg(&g_PaCfg);
    return 0;
}
INIT_BOARD_EXPORT(Board_AudioPinInit, OS_INIT_SUBLEVEL_LOW);
#endif

#if defined(BSP_USING_SPICAM) && defined(BSP_USING_SPILCD)
#include <gc032a.h>
#include <st7789v.h>

static GC032A_PIN_DEFINE(g_gc032aPin, BSP_I2C_BUS_NUM,
    SPICAM_POWERDOWN_PIN_RES, SPICAM_POWERDOWN_PIN_MUX,
    SPICAM_MCLK_PIN_RES, SPICAM_MCLK_PIN_MUX,
    SPICAM_CLK_PIN_RES, SPICAM_CLK_PIN_MUX,
    SPICAM_DI0_PIN_RES, SPICAM_DI0_PIN_MUX,
    SPICAM_DI1_PIN_RES, SPICAM_DI1_PIN_MUX);
int Board_CamPinInit(void)
{
    GC032A_PinInit(&g_gc032aPin);

    return 0;
}
INIT_BOARD_EXPORT(Board_CamPinInit, OS_INIT_SUBLEVEL_MIDDLE);


static PIN_MultiMux_t g_st7789v_bk_pin = {.res = SPILCD_BACKLIGHT_PIN_RES, .gpioMux = SPILCD_BACKLIGHT_PIN_MUX, .specMux = SPILCD_BACKLIGHT_PIN_PWMMUX};
static PIN_MultiMux_t g_st7789v_reset_pin = {.res = SPILCD_RESET_PIN_RES, .gpioMux = SPILCD_RESET_PIN_MUX, .specMux = SPILCD_RESET_PIN_MUX};
static PIN_MultiMux_t g_st7789v_csx_pin = {.res = SPILCD_CSX_PIN_RES, .gpioMux = SPILCD_CSX_PIN_MUX, .specMux = SPILCD_CSX_PIN_MUX};
static PIN_MultiMux_t g_st7789v_scl_pin = {.res = SPILCD_SCL_PIN_RES, .gpioMux = SPILCD_SCL_PIN_MUX, .specMux = SPILCD_SCL_PIN_MUX};
static PIN_MultiMux_t g_st7789v_sdi_pin = {.res = SPILCD_SDI_PIN_RES, .gpioMux = SPILCD_SDI_PIN_MUX, .specMux = SPILCD_SDI_PIN_MUX};
static PIN_MultiMux_t g_st7789v_sdo_pin = {.res = SPILCD_SDO_PIN_RES, .gpioMux = SPILCD_SDO_PIN_MUX, .specMux = SPILCD_SDO_PIN_MUX};
static PIN_MultiMux_t g_st7789v_dcx_pin = {.res = SPILCD_DCX_PIN_RES, .gpioMux = SPILCD_DCX_PIN_MUX, .specMux = SPILCD_DCX_PIN_MUX};

static LCD_TE_DEFINE(g_st7789vTe, SPILCD_TE_PIN_RES, SPILCD_TE_PIN_MUX, 0, 0);

int Board_ST7789V_PinInit(void)
{
    ST7789V_PinInit(&g_st7789v_bk_pin, &g_st7789v_reset_pin, &g_st7789v_csx_pin,
                    &g_st7789v_scl_pin, &g_st7789v_sdi_pin, &g_st7789v_sdo_pin, &g_st7789v_dcx_pin);
    LCD_TePinInit(&g_st7789vTe);

    return 0;
}
INIT_BOARD_EXPORT(Board_ST7789V_PinInit, OS_INIT_SUBLEVEL_MIDDLE);
#endif

#if defined(BSP_USING_I2C)
#include <i2c_device.h>

static I2C_BusPin g_i2cPin = {
    .sclRes = BSP_I2C_SCL_PIN_RES,
    .sclMux = BSP_I2C_SCL_PIN_MUX,
    .sdaRes = BSP_I2C_SDA_PIN_RES,
    .sdaMux = BSP_I2C_SDA_PIN_MUX,
};

int Board_I2C_PinInit(void)
{
    I2C_BusPinInit(BSP_I2C_BUS_NUM, &g_i2cPin);

    return 0;
}
INIT_BOARD_EXPORT(Board_I2C_PinInit, OS_INIT_SUBLEVEL_MIDDLE);
#endif

/**
 ************************************************************************************
 * @brief           PIN配置
 *
 * @param[in]       void
 *
 * @return          int32_t
 * @retval          0          成功
 ************************************************************************************
*/
#if defined(_CPU_AP)
#include "pinmap.h"
int Board_PinInit(void)
{
    #if !defined(USE_TOP_VIDEO) && !defined(USE_TOP_VOLTE)
    Board_MdlPinInit();
    #endif

    #if defined(USE_TOP_VIDEO) &&  defined(USE_TOP_VOLTE)
    Board_VideoPinInit();
    #endif

    #if defined(USE_TOP_VOLTE) && !defined(USE_TOP_VIDEO)
    Board_VoltePinInit();
    #endif
    return 0;
}
INIT_BOARD_EXPORT(Board_PinInit, OS_INIT_SUBLEVEL_HIGH);
#endif