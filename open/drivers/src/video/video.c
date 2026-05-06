#include "os.h"
#include "nr_micro_shell.h"
#include "drv_display.h"
#include "drv_capture.h"
#include <stdlib.h>
#include <slog_print.h>

#define DISP_TEST_MAX_FPS       (0)

#define DISP_TEST_WIDTH         (240)
#define DISP_TEST_HEIGHT        (320)

#define SHELL_Debug(format, ...)        slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_UI_FRAME, format, ##__VA_ARGS__)
#define SHELL_Info(format, ...)         slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_UI_FRAME, format, ##__VA_ARGS__)
#define SHELL_Warning(format, ...)      slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_UI_FRAME, format, ##__VA_ARGS__)
#define SHELL_Error(format, ...)        slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_UI_FRAME, format, ##__VA_ARGS__)

typedef struct Video_FpsMonitor{
    uint32_t dispCount;
    uint32_t captureCount;

    struct osTimer timer;
}Video_FpsMonitor_t;

static osThreadId_t g_videoThreadID = NULL;

static int g_camCaptureFlag = 0;


static uint8_t *g_camImage = NULL;
static uint8_t *g_captureImage = NULL;

static osCompletion g_videoStopCmpl = {0};
static osCompletion g_dispCmpl = {0};
static osCompletion g_campureCmpl = {0};
static osCompletion captureMsg = {0};

static bool flip_x = false;
static bool flip_y = false;

static uint8_t *g_capturePicture = NULL;

static bool g_videoStopFlag = false;

static CaptureDevice_t *g_captureDev = NULL;
static DispDevice_t *g_dispDev = NULL;
static DispOps_t *g_dispOps = NULL;

static Video_FpsMonitor_t *g_videoFpsMonitor = NULL;

extern uint32_t SpiCam_ErrNumber(void);

static void Video_FpsCalcTimerEntry(void *param)
{
    if(param == NULL) {
        return;
    }

    Video_FpsMonitor_t *fps = (Video_FpsMonitor_t *)param;

    // SHELL_Debug("lcd:%d  camera:%d overrun:%d", lcd, cam, SpiCam_ErrNumber());
    SHELL_Info("lcd:%d  camera:%d", fps->dispCount, fps->captureCount);

    fps->dispCount = 0;
    fps->captureCount = 0;
}

static void Video_FpsCheckStart(void)
{
    if(g_videoFpsMonitor) {
        return;
    }

    Video_FpsMonitor_t *fps = (Video_FpsMonitor_t *)osMalloc(sizeof(Video_FpsMonitor_t));
    memset(fps, 0, sizeof(Video_FpsMonitor_t));
    osTimerInit(&fps->timer, Video_FpsCalcTimerEntry, fps, 0xffffffff, OS_TIMER_FLAG_PERIODIC | OS_TIMER_FLAG_SOFT_TIMER);
    osTimerStart(&fps->timer, osTickFromMs(1000));

    g_videoFpsMonitor = fps;
}

static void Video_FpsCheckStop(void)
{
    if(g_videoFpsMonitor == NULL) {
        return;
    }

    Video_FpsMonitor_t *fps = g_videoFpsMonitor;
    g_videoFpsMonitor = NULL;

    osTimerStop(&fps->timer);
    osTimerDetach(&fps->timer);
    osFree(fps);
}

static void Capture_InterruptServer(void *userData)
{
    if(g_videoFpsMonitor) {
        g_videoFpsMonitor->captureCount++;
    }

    if(userData) {
        osComplete(userData);
    }
}

static void Disp_InterruptServer(void *userData)
{
    if(g_videoFpsMonitor) {
        g_videoFpsMonitor->dispCount++;
    }

    if(userData) {
        osComplete(userData);
    }
}

static void Video_ThreadEntry(void *param)
{
    uint8_t *image = NULL;

    while (1)
    {
    #if DISP_TEST_MAX_FPS
        osStatus_t status = osWaitForCompletion(&g_campureCmpl, osNoWait);
    #else
        osWaitForCompletion(&g_campureCmpl, osWaitForever);
    #endif

    #if DISP_TEST_MAX_FPS
        if(status != osOK && g_camImage != NULL) {
            g_dispOps->flip(g_dispDev, flip_x, flip_y);
            g_dispOps->flush(g_dispDev, 0, g_dispDev->horRes - 1, 0, g_dispDev->verRes - 1, g_camImage);
            osWaitForCompletion(&g_dispCmpl, osWaitForever);
        }
        else if(status == osOK) {
    #endif
            uint8_t *newImage = g_captureDev->capture(g_captureDev);
            if(newImage) {
                image = newImage;
                // SHELL_Debug("get new image");
                g_camImage = image;
            }
    #if DISP_TEST_MAX_FPS == 0
            else {
                SHELL_Debug("get NULL image");
                g_camImage = NULL;
                continue;
            }
    #endif

            // g_dispOps->flip(g_dispDev, flip_x, flip_y);
            g_dispOps->flush(g_dispDev, 0, g_dispDev->horRes - 1, 0, g_dispDev->verRes - 1, image);
            osWaitForCompletion(&g_dispCmpl, osWaitForever);
            if(g_camCaptureFlag) {
                // memcpy(g_capturePicture, image, sizeof(g_capturePicture));
                g_captureImage = image;
                osComplete(&captureMsg);
                g_camCaptureFlag = 0;
                continue;
            }
    #if DISP_TEST_MAX_FPS == 0
            g_camImage = NULL;
            g_captureDev->release(g_captureDev, image);
    #endif
    #if DISP_TEST_MAX_FPS
        }
        else {
            osThreadSleep(100);
        }
    #else
    #endif

        if(g_videoStopFlag) {
            break;
        }
    }

    osComplete(&g_videoStopCmpl);
}

static int Video_DispInit(void (*callback)(void *data), void *data)
{
    int ret = 0;

    g_dispDev = DispST7789V_GetDevice();
    g_dispOps = DispST7789V_GetOps();

    if(g_dispDev ==  NULL) {
        return -1;
    }

    g_dispDev->horRes = DISP_TEST_WIDTH;
    g_dispDev->verRes = DISP_TEST_HEIGHT;

    ret = g_dispOps->init(g_dispDev, callback, data);
    if(ret) {
        SHELL_Error("disp init error");
        return ret;
    }

    uint32_t id = g_dispOps->id(g_dispDev);
    SHELL_Debug("get disp id = 0x%08x", id);

    return 0;
}

static int Video_DispDeInit(void)
{
    g_dispOps->deinit(g_dispDev);

    return 0;
}

static int Video_CaptureInit(void (*callback)(void *data), void *data)
{
    g_captureDev = CamGC032A_GetDevice();

    if(g_captureDev == NULL) {
        return -1;
    }

    g_captureDev->horRes = DISP_TEST_WIDTH;
    g_captureDev->verRes = DISP_TEST_HEIGHT;

    return g_captureDev->init(g_captureDev, callback, data);
}

static int Video_CaptureDeInit(void)
{
    g_captureDev->stop(g_captureDev);
    g_captureDev->deinit(g_captureDev);

    return 0;
}

static void Video_WaitStop(void)
{
    osStatus_t status = osOK;
    g_videoStopFlag = true;

    do
    {
        status = osWaitForCompletion(&g_videoStopCmpl, osNoWait);
        osThreadSleep(100);
        SHELL_Debug("wait lcd end");
    } while (status != osOK);
}

static void DISP_FlushColor(uint32_t num, uint32_t delay)
{
    uint16_t *red = (uint16_t *)osMallocAlign(DISP_TEST_WIDTH * DISP_TEST_HEIGHT * 2, 32);
    uint16_t *green = (uint16_t *)osMallocAlign(DISP_TEST_WIDTH * DISP_TEST_HEIGHT * 2, 32);
    uint16_t *blue = (uint16_t *)osMallocAlign(DISP_TEST_WIDTH * DISP_TEST_HEIGHT * 2, 32);

    for(uint32_t index = 0; index < DISP_TEST_WIDTH * DISP_TEST_HEIGHT; index++)
    {
        red[index] = (uint16_t)RGB_5_6_5_VAL(255, 0, 0);
        green[index] = (uint16_t)RGB_5_6_5_VAL(0, 255, 0);
        blue[index] = (uint16_t)RGB_5_6_5_VAL(0, 0, 255);
    }

    g_dispOps->endianConvert(g_dispDev, true);

#if 0
    extern const unsigned char picture_src[153600];
    g_dispOps->flush(g_dispDev, 0, g_dispDev->horRes - 1, 0, g_dispDev->verRes - 1, (uint8_t *)picture_src);
    osWaitForCompletion(&g_dispCmpl, osWaitForever);
#else
    for(uint32_t i = 0; i < num; i++) {
        g_dispOps->flush(g_dispDev, 0, g_dispDev->horRes - 1, 0, g_dispDev->verRes - 1, (uint8_t *)red);
        osWaitForCompletion(&g_dispCmpl, osWaitForever);
        if(delay)
            osThreadSleep(delay);
        
        g_dispOps->flush(g_dispDev, 0, g_dispDev->horRes - 1, 0, g_dispDev->verRes - 1, (uint8_t *)green);
        osWaitForCompletion(&g_dispCmpl, osWaitForever);
        if(delay)
            osThreadSleep(delay);
        
        g_dispOps->flush(g_dispDev, 0, g_dispDev->horRes - 1, 0, g_dispDev->verRes - 1, (uint8_t *)blue);
        osWaitForCompletion(&g_dispCmpl, osWaitForever);
        if(delay)
            osThreadSleep(delay);
    }
#endif

    g_dispOps->endianConvert(g_dispDev, false);

    osFree(red);
    osFree(green);
    osFree(blue);
}

static void Video_DispOpen(void *param)
{
    int ret = 0;

    g_videoStopFlag = false;

    osInitCompletion(&g_dispCmpl);

    ret = Video_DispInit(Disp_InterruptServer, &g_dispCmpl);
    if(ret) {
        SHELL_Error("disp init error");
        return;
    }
}

static void Video_DispClose(void)
{
    Video_DispDeInit();

    osSemaphoreDetach(&g_dispCmpl);
}

static void Video_DispOpenThread(void)
{
    osThreadAttr_t attr = {"lcdint", osThreadDetached, NULL, 0U, NULL, 1024, osPriorityISR - 15, 0U, 0U};

    osThreadNew((osThreadFunc_t)Video_DispOpen, NULL, &attr);
}

static void Video_Open(void)
{
    int ret = 0;

    g_videoStopFlag = false;

    osInitCompletion(&g_videoStopCmpl);
    osInitCompletion(&g_dispCmpl);
    osInitCompletion(&g_campureCmpl);
    osInitCompletion(&captureMsg);

    osThreadAttr_t attr_lcdshow = {"lcdshow", osThreadDetached, NULL, 0U, NULL, 2048, osPriorityISR - 25, 0U, 0U};
    g_videoThreadID = osThreadNew(Video_ThreadEntry, NULL, &attr_lcdshow);

    ret = Video_DispInit(Disp_InterruptServer, &g_dispCmpl);
    if(ret) {
        SHELL_Error("disp init error");
        return;
    }

    ret = Video_CaptureInit(Capture_InterruptServer, &g_campureCmpl);
    if(ret) {
        SHELL_Error("camera init error");
        return;
    }

    g_captureDev->start(g_captureDev);
}

static void Video_Close(void)
{
    Video_WaitStop();

    Video_DispDeInit();
    Video_CaptureDeInit();

    osThreadDetach(g_videoThreadID);
    osSemaphoreDetach(&g_videoStopCmpl);
    osSemaphoreDetach(&g_dispCmpl);
    osSemaphoreDetach(&g_campureCmpl);
    osSemaphoreDetach(&captureMsg);
}

static void Video_Capture(void)
{
    g_capturePicture = (uint8_t *)osMallocAlign(DISP_TEST_WIDTH * DISP_TEST_HEIGHT * 2, 32);

    g_camCaptureFlag = 1;
    osWaitForCompletion(&captureMsg, osWaitForever);

    osPrintf("\r\nstart %d\r\n", sizeof(g_capturePicture));
    for(uint32_t i = 0; i < sizeof(g_capturePicture); i++) {
        osPrintf("%02x ", g_capturePicture[i]);
    }
    osPrintf("\r\nend\r\n");

    osFree(g_capturePicture);
}

static void Video_Shell(char argc, char **argv)
{
    g_dispDev = DispST7789V_GetDevice();
    g_dispOps = DispST7789V_GetOps();
    g_captureDev = CamGC032A_GetDevice();

    if(!strcmp(argv[1], "open")) {
        Video_Open();
    }
    else if(!strcmp(argv[1], "close")) {
        Video_Close();
    }
    else if(!strcmp(argv[1], "capture")) {
        Video_Capture();
    }
    else if(!strcmp(argv[1], "fps_start")) {
        Video_FpsCheckStart();
    }
    else if(!strcmp(argv[1], "fps_stop")) {
        Video_FpsCheckStop();
    }
    else if(!strcmp(argv[1], "lcd_max")) {
        Video_DispOpen(NULL);
        Video_FpsCheckStart();
        DISP_FlushColor(100000000, 0);
        Video_FpsCheckStop();
        Video_DispClose();
    }
    else if(!strcmp(argv[1], "lcd_open")) {
        // TC_LCD_Flush(NULL);
        // TC_LCD_FlushInThread();
        Video_DispOpen(NULL);
    }
    else if(!strcmp(argv[1], "lcd_close")) {
        // TC_LCD_Flush(NULL);
        // TC_LCD_FlushInThread();
        Video_DispClose();
    }
    else if(!strcmp(argv[1], "lcd_flush")) {
        DISP_FlushColor(10, 200);
    }
    else if(!strcmp(argv[1], "lcd_sleep")) {
        g_dispOps->sleep(g_dispDev);
    }
    else if(!strcmp(argv[1], "lcd_wakeup")) {
        g_dispOps->wakeup(g_dispDev);
    }
    else if(!strcmp(argv[1], "bg_on")) {
        g_dispOps->backlight(g_dispDev, 100);
    }
    else if(!strcmp(argv[1], "bg_off")) {
        g_dispOps->backlight(g_dispDev, 0);
    }
    else if(!strcmp(argv[1], "bg")) {
        uint8_t high = (uint8_t)strtoul(argv[2], NULL, 0);
        g_dispOps->backlight(g_dispDev, high);
    }
    else if(!strcmp(argv[1], "lcd_flipx")) {
        flip_x = !flip_x;
    }
    else if(!strcmp(argv[1], "lcd_flipy")) {
        flip_y = !flip_y;
    }
    else if(!strcmp(argv[1], "lcd_id")) {
        uint32_t chipid = g_captureDev->getChipID(g_captureDev);
        SHELL_Debug("chipid = 0x%x", chipid);
    }
    else {
        SHELL_Error("not support");
    }
}

NR_SHELL_CMD_EXPORT(video, Video_Shell);
