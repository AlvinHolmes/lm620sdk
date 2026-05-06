#include "os.h"
#include "os_hw.h"
#include "drv_soc.h"
#include "wdt_core.h"
#include "drv_wdt.h"
#if defined(OS_USING_PM)  
#include <psm_core.h>
#endif
#include <drv_reset.h>
#include <nr_micro_shell.h>

/************************************************************************************
 *                                 配置
 ************************************************************************************/

#define  WDT_OPEN_TO_CUSTOM     (0)        // Watchdog完全开放给客户使用， 1表示开放

/************************************************************************************
 *                                 类型
 ************************************************************************************/

#define     MS_PER_SECOND                   (1000UL)
#define     WDT_MONITOR_ENABLE              (0)
#define     WDT_TRACE_NUM                   (10)

typedef struct
{
    uint32_t    load;           // 初始计数值
    uint32_t    timeout;        // 超时断言值
    uint32_t    count;          // 当前计数值
}WDT_RegTrace;

typedef enum {
    WDT_NONE = 0,
    WDT_FEED,
    WDT_TIMEOUT,
    WDT_ABORT,
    WDT_RESUME,
} WDT_TRACE_EVENT;

typedef struct
{
    uint32_t        systick;
    WDT_TRACE_EVENT event;
    WDT_RegTrace    regTrace;
}WDT_Trace;

void *g_wdtThread = NULL;
static uint16_t g_wdtTraceOffset = 0;
static WDT_Trace g_wdtTrace[WDT_TRACE_NUM] = {0};

#if WDT_MONITOR_ENABLE
static struct osTimer g_wdtMonitorTimer = {0};
#endif

static bool g_hardWdtCheckEnable =false;

static __IRAM_DATA_PSM_RE WDT_CoreHandle g_hardWdtInfo = {0};

#if WDT_OPEN_TO_CUSTOM
static bool g_hardWdtOpenToCustom = true;
#else
static bool g_hardWdtOpenToCustom = false;
#endif

static void WDT_TraceRefresh(WDT_TRACE_EVENT event)
{
    WDT_RegTrace *regTrace = &g_wdtTrace[g_wdtTraceOffset].regTrace;

    regTrace->load = WDT_LoadGet(&g_hardWdtInfo);
    regTrace->timeout = WDT_TimeoutGet(&g_hardWdtInfo);
    regTrace->count = WDT_CountGet(&g_hardWdtInfo);

    g_wdtTrace[g_wdtTraceOffset].systick = osGetSysTimeCnt();
    g_wdtTrace[g_wdtTraceOffset].event = event;

    g_wdtTraceOffset++;
    if (g_wdtTraceOffset >= WDT_TRACE_NUM)
    {
        g_wdtTraceOffset = 0;
    }

#if WDT_MONITOR_ENABLE
    osPrintf("trace[%d] - init count : %d, timeout count : %d, current count : %d\r\n", event, regTrace->load, regTrace->timeout, regTrace->count);
#endif
}

#if !WDT_OPEN_TO_CUSTOM
static void WDT_ThreadCallBack(OS_UNUSED void *parameter)
{
    while (1)
    {
        WDT_TraceRefresh(WDT_FEED);
        WDT_Config(&g_hardWdtInfo);
#if WDT_MONITOR_ENABLE
        osPrintf("Hard Wdt Feed Dog \r\n");
#endif

#if defined(_CPU_AP)
        osThreadMsSleepRelaxed(g_hardWdtInfo.userData.feedDogPeriod * MS_PER_SECOND, osWaitForever);
#else
    #if WDT_MONITOR_ENABLE
        WDT_Monitor(OS_NULL);
    #endif
        osThreadSuspend(g_wdtThread);
#endif
    }
}
#endif

void WDT_Monitor(void *parameter)
{
    osPrintf("timer - init count : %d, timeout count : %d, current count : %d\r\n", 
            WDT_LoadGet(&g_hardWdtInfo), WDT_TimeoutGet(&g_hardWdtInfo), WDT_CountGet(&g_hardWdtInfo));
}

static void WDT_Isr(OS_UNUSED int vector, OS_UNUSED void *param)
{
    WDT_TraceRefresh(WDT_TIMEOUT);
#if WDT_MONITOR_ENABLE
    osPrintf("hard wdt timeout \r\n");
#endif
#if defined(_CPU_AP)
    HardWDT_SetTimeoutFlag(WDT_AP_TIMEOUT_FLAG);
#else
    HardWDT_SetTimeoutFlag(WDT_CP_TIMEOUT_FLAG);
#endif
    OS_ASSERT(0);
}

__IRAM_CODE_PSM_RE void HardWDT_FeedDog(void)
{
    WDT_Stop(&g_hardWdtInfo);
    WDT_Config(&g_hardWdtInfo);
    WDT_Start(&g_hardWdtInfo);
}

uint8_t HardWDT_Initlize(uint32_t timeOutPeriod, uint32_t feedDogPeriod)
{
#if WDT_MONITOR_ENABLE
    osPrintf("set wdt timeOutPeriod : %d,  feedDogPeriod : %d\r\n", timeOutPeriod, feedDogPeriod);
#endif
    g_hardWdtInfo.userData.feedDogPeriod = feedDogPeriod / MS_PER_SECOND;
    g_hardWdtInfo.userData.timeOutPeriod = timeOutPeriod / MS_PER_SECOND;

#if !WDT_OPEN_TO_CUSTOM
    if(!g_wdtThread)
    {
        osThreadAttr_t attr = {"wdt", osThreadDetached, NULL, 0U, NULL, 512, osPriorityIdle + 1, 0U, 0U};
        g_wdtThread = osThreadNew(WDT_ThreadCallBack, NULL, &attr);
        OS_ASSERT(g_wdtThread);

#if defined(_CPU_AP)
    #if WDT_MONITOR_ENABLE
        osTimerInit(&g_wdtMonitorTimer, WDT_Monitor, NULL, OS_TICK_PER_SECOND, OS_TIMER_FLAG_PERIODIC);
        osTimerStartEX(&g_wdtMonitorTimer);
    #endif
#endif
    }
#endif

    g_hardWdtCheckEnable = true;

    WDT_Initlize(&g_hardWdtInfo);
    HardWDT_FeedDog();

    osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(g_WDT_Res[0].intNum));
    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(g_WDT_Res[0].intNum), 7, IRQ_POSITIVE_EDGE);
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(g_WDT_Res[0].intNum), WDT_Isr,  NULL);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(g_WDT_Res[0].intNum));

#ifdef _CPU_AP
    HardWDT_SetTimeoutFlag(0);
    HardWDT_SetConfig(WDT_CP_DEFAULT_TIMEOUT_TIME_SECOND);
#endif

    return DRV_OK;
}

void HardWDT_UnInitlize(void)
{
    g_hardWdtInfo.reg = (WDT_Reg *)g_WDT_Res[0].regBase;

#if WDT_MONITOR_ENABLE
    osPrintf("uninitialize wdt\r\n");
#endif

    g_hardWdtCheckEnable = false;

#if defined(_CPU_AP)
    if(g_wdtThread)
    {
        unsigned long level = osInterruptDisable();
        osThreadTerminate(g_wdtThread);
        g_wdtThread = NULL;

        g_hardWdtInfo.userData.feedDogPeriod = 0;
        g_hardWdtInfo.userData.timeOutPeriod = 0;

    #if WDT_MONITOR_ENABLE
        osTimerStop(&g_wdtMonitorTimer);
        osTimerDetach(&g_wdtMonitorTimer);
        osPrintf("detect wdt timer\r\n");
    #endif
        osInterruptEnable(level);
    }
#endif

    WDT_UnInitlize(&g_hardWdtInfo);

    osInterruptMask(OS_EXT_IRQ_TO_IRQ(g_WDT_Res[0].intNum)); 
    osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(g_WDT_Res[0].intNum));

#ifdef _CPU_AP
    HardWDT_SetTimeoutFlag(0);
    HardWDT_SetConfig(0);
#endif
}

void HardWDT_Abort(void)
{
    if(!g_hardWdtInfo.reg) {
        return;
    }

    WDT_TraceRefresh(WDT_ABORT);

    WDT_Stop(&g_hardWdtInfo);
}

bool HardWDT_IsOpen(void)
{
    return g_hardWdtOpenToCustom;
}

static void watchdog_shell(char argc, char **argv)
{
    // 关闭喂狗测试
    if(g_wdtThread) {
        osThreadTerminate(g_wdtThread);
        g_wdtThread = NULL;
    }
}
NR_SHELL_CMD_EXPORT(watchdog, watchdog_shell);

#if defined(_CPU_AP)
static void SoftWdt_Timeout(void *param)
{
    if(param) {
        osPrintf("task[%p] timeout\r\n", param);
        if(((struct osThread *)param)->name) {
            osPrintf("%s\r\n", ((struct osThread *)param)->name);
        }
    }
    OS_ASSERT(0);
}

void *SoftWdt_Create(void *monitor_task)
{
    struct osTimer *timer = (struct osTimer *)osMalloc(sizeof(struct osTimer));
    OS_ASSERT(timer);
    osTimerInit(timer, SoftWdt_Timeout, monitor_task, 0xffffffff, OS_TIMER_FLAG_ONE_SHOT | OS_TIMER_FLAG_HARD_TIMER);
    return (void *)timer;
}

void SoftWdt_Feed(void *wdt, uint32_t ms)
{
    osTimerStop(wdt);
    osTimerStartRelaxed(wdt, osTickFromMs(ms), osWaitForever);
}

int SoftWdt_Start(void *wdt, uint32_t ms)
{
    return osTimerStartRelaxed(wdt, osTickFromMs(ms), osWaitForever);
}

void SoftWdt_Stop(void *wdt)
{
    osTimerStop(wdt);
}

void SoftWdt_Delete(void *wdt)
{
    osTimerStop(wdt);
    osTimerDetach(wdt);
    osFree(wdt);
}

#if 0
static void *softwdt = NULL;
static void watchdog2_shell(char argc, char **argv)
{
    if(softwdt == NULL) {
        softwdt = SoftWdt_Create(osThreadSelf());
    }

    if(!strcmp(argv[1], "start")) {
        SoftWdt_Start(softwdt, 5000);
    }
    else if(!strcmp(argv[1], "stop")) {
        SoftWdt_Stop(softwdt);
    }
    else if(!strcmp(argv[1], "delete")) {
        SoftWdt_Delete(softwdt);
    }
}
NR_SHELL_CMD_EXPORT(watchdog2, watchdog2_shell);
#endif
#endif

uint16_t HardWDT_GetConfig(void)
{
    return *((uint16_t *)IRAM_BASE_ADDR_WDT_CONFIG);
}

void HardWDT_SetConfig(uint16_t val)
{
    *((uint16_t *)IRAM_BASE_ADDR_WDT_CONFIG) = val;
}

uint8_t HardWDT_GetTimeoutFlag(void)
{
    return *((uint8_t *)IRAM_BASE_ADDR_WDT_TIMEOUT_FLAG);
}

void HardWDT_SetTimeoutFlag(uint8_t val)
{
    *((uint8_t *)IRAM_BASE_ADDR_WDT_TIMEOUT_FLAG) = val;
}

uint8_t HardWDT_GetExceptionResetFlag(void)
{
    return *((uint8_t *)IRAM_BASE_ADDR_EXCEPTION_RESET_FLAG);
}

void HardWDT_SetExceptionResetFlag(uint8_t val)
{
    *((uint8_t *)IRAM_BASE_ADDR_EXCEPTION_RESET_FLAG) = val;
}

#if defined(OS_USING_PM)
void HardWDT_PsmStart(uint32_t timeOutPeriod)
{
   uint32_t regVal = 0;

    //set div
    regVal = (WDT_CLK_DIV - 1) | (WDT_WRT_KEY << WDT_WRT_KEY_POS);
    WRITE_U32( BASE_PRIVATE_WDT + 0x04, regVal);

    //set load
    WRITE_U32(BASE_PRIVATE_WDT + 0x08, timeOutPeriod * 1000 | (WDT_WRT_KEY << WDT_WRT_KEY_POS));
    //set compare
    WRITE_U32(BASE_PRIVATE_WDT + 0x14,  2 * 1000 | (WDT_WRT_KEY << WDT_WRT_KEY_POS));

    //refresh
    regVal = READ_U32(BASE_PRIVATE_WDT + 0x18);
    regVal &= 0x0000FFFF;
    regVal = (regVal ^ WDT_SET_EN_MASK) & WDT_SET_EN_MASK;
    WRITE_U32(BASE_PRIVATE_WDT + 0x18, regVal | (WDT_WRT_KEY << WDT_WRT_KEY_POS));
    //check ack
    while((READ_U32(BASE_PRIVATE_WDT + 0x18) & WDT_SET_EN_MASK) != ((READ_U32(BASE_PRIVATE_WDT + 0x10) >> 4) & WDT_SET_EN_MASK));

    //start
    WRITE_U32(BASE_PRIVATE_WDT + 0x1C,  WDT_START_MASK | (WDT_WRT_KEY << WDT_WRT_KEY_POS));
}

void  HardWDT_PsmStop(void)
{
    WRITE_U32(BASE_PRIVATE_WDT + 0x1C,  WDT_WRT_KEY << WDT_WRT_KEY_POS);
}

#if defined(OS_USING_PM)
__IRAM_CODE_PSM_RE int HardWDT_Resume(OS_UNUSED void *param, OS_UNUSED PSM_Mode mode, OS_UNUSED uint32_t *saveAddr)
{
#ifdef USE_BOOT_PSM_STANDBY
    if (mode == PSM_DEEP_SLEEP) {
#endif
        if(g_hardWdtInfo.userData.feedDogPeriod != 0 && g_hardWdtInfo.userData.timeOutPeriod != 0)
        {
           HardWDT_FeedDog();
        }
#ifdef USE_BOOT_PSM_STANDBY
    }
#endif
    return 0;
}
__IRAM_CODE_PSM_RE int HardWDT_Suspend(OS_UNUSED void *param, OS_UNUSED PSM_Mode mode, OS_UNUSED uint32_t *saveAddr)
{
#ifdef USE_BOOT_PSM_STANDBY
    if (mode == PSM_DEEP_SLEEP) {
#endif
        if(g_hardWdtInfo.userData.feedDogPeriod != 0 && g_hardWdtInfo.userData.timeOutPeriod != 0)
        {
            WDT_Stop(&g_hardWdtInfo);
        }
#ifdef USE_BOOT_PSM_STANDBY
    }
#endif
    return 0;
}
#endif

#endif
