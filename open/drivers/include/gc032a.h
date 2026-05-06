#ifndef __GC032A_H__
#define __GC032A_H__

#include <os.h>
#include <drv_common.h>
#include <drv_pin.h>
#include "drv_spi_cam.h"
#if defined(OS_USING_PM)
#include <psm_wakelock.h>
#endif

/**
 * @addtogroup Gc032a
 */

/**@{*/

/************************************************************************************
 *                                 宏开关
 ************************************************************************************/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

#ifdef CONFIG_CAMERA_SINGLE_BUFFER
#define CAMERA_BUFFER_DEPTH     (1)
#else
#define CAMERA_BUFFER_DEPTH     (2)
#endif

#define GC032A_PIN_DEFINE(config, busNum,                                       \
                          pinPowerDownRes, pinPowerDownMux,                     \
                          pinMclkRes, pinMclkMux,                               \
                          pinSpiClkRes, pinSpiClkMux,                           \
                          pinSpiDi0Res, pinSpiDi0Mux,                           \
                          pinSpiDi1Res, pinSpiDi1Mux)                           \
                                                                                \
    GC032A_PinConfig config = {                                                 \
        .i2cBusNum = busNum,                                                    \
        .pinPowerDown = {.pinRes = pinPowerDownRes, .pinMux = pinPowerDownMux}, \
        .pinMclk = {.pinRes = pinMclkRes, .pinMux = pinMclkMux},                \
        .pinSpiClk = {.pinRes = pinSpiClkRes, .pinMux = pinSpiClkMux},          \
        .pinSpiDi0 = {.pinRes = pinSpiDi0Res, .pinMux = pinSpiDi0Mux},          \
        .pinSpiDi1 = {.pinRes = pinSpiDi1Res, .pinMux = pinSpiDi1Mux},          \
    }

typedef enum {
    GC032A_SET_AWB,
    GC032A_SET_BRIGHTNESS,
    GC032A_SET_CONTRAST,
    GC032A_SET_EV,
}GC032A_CMD;


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct 
{
    const void *pinRes; 
    uint8_t pinMux;
}GC032A_Pin;

typedef struct {
    uint8_t       i2cBusNum;
    GC032A_Pin    pinPowerDown;
    GC032A_Pin    pinMclk;
    GC032A_Pin    pinSpiClk;
    GC032A_Pin    pinSpiDi0;
    GC032A_Pin    pinSpiDi1;
}GC032A_PinConfig;

typedef struct {  
    void (*cbEvent)(void *userData);
    void  *userData;

    GC032A_PinConfig   *pinConfig;
    SPICAM_Handle       spiCam;

#define GC032A_INIT            ((uint8_t)0x01)
#define GC032A_RUNNING          ((uint8_t)0x02)
    uint8_t             ctrl;

#if defined(OS_USING_PM)
    WakeLock        wakeLock;
#endif

    uint8_t             captureCached;
    uint32_t            captureTotal;
}GC032A_Handle;

typedef struct
{
    bool (*cameraInit)(GC032A_Handle *hdl, void (*callback)(void *data), void *data, uint16_t width, uint16_t height);
    bool (*cameraDeInit)(GC032A_Handle *hdl);
    uint32_t (*cameraGetID)(GC032A_Handle *hdl);
    uint8_t* (*cameraCaptureImage)(GC032A_Handle *hdl);
    int32_t (*cameraStartPrev)(GC032A_Handle *hdl);
    int32_t (*cameraStopPrev)(GC032A_Handle *hdl);
    void (*cameraImageRelease)(GC032A_Handle *hdl, uint8_t *image);
} SensorOps_t;

void GC032A_PinInit(GC032A_PinConfig *config);

int32_t GC032A_Init(GC032A_Handle *hdl, void (*callback)(void *data), void *data, uint16_t width, uint16_t height);
int32_t GC032A_DeInit(GC032A_Handle *hdl);
int32_t GC032A_Start(GC032A_Handle *hdl);
int32_t GC032A_Stop(GC032A_Handle *hdl);
uint8_t* GC032A_Capture(GC032A_Handle *hdl);
int32_t GC032A_ImageRelease(GC032A_Handle *hdl, uint8_t *image);
uint32_t GC032A_IdGet(GC032A_Handle *hdl);

void GC032A_Control(GC032A_Handle *hdl, uint32_t cmd, uint32_t arg);

#endif
/** @} */