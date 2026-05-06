
#include "gc032a.h"
#include "drv_spi_cam.h"
#include "os.h"
#include <i2c_device.h>
#include <res_tree.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define GC032A_PrintDebug(fmt, ...) osPrintf("[%-5d]    %-15s %-25s :" fmt "\r\n", __LINE__, __FILE__, __func__, ##__VA_ARGS__)
#define GC032A_PrintError(fmt, ...) osPrintf("[%-5d]    %-15s %-25s :" "\033[" "31m" fmt "\r\n" "\033[0m", __LINE__, __FILE__, __func__, ##__VA_ARGS__)
// #define GC032A_PrintDebug(fmt, ...)

#define GC032A_PIN_RES_POWER_DOWN(config)   ((config)->pinPowerDown.pinRes)
#define GC032A_PIN_RES_MCLK(config)         ((config)->pinMclk.pinRes)
#define GC032A_PIN_RES_SPI_CLK(config)      ((config)->pinSpiClk.pinRes)
#define GC032A_PIN_RES_SPI_DI0(config)      ((config)->pinSpiDi0.pinRes)
#define GC032A_PIN_RES_SPI_DI1(config)      ((config)->pinSpiDi1.pinRes)

#define GC032A_PIN_MUX_POWER_DOWN(config)   ((config)->pinPowerDown.pinMux)
#define GC032A_PIN_MUX_MCLK(config)         ((config)->pinMclk.pinMux)
#define GC032A_PIN_MUX_SPI_CLK(config)      ((config)->pinSpiClk.pinMux)
#define GC032A_PIN_MUX_SPI_DI0(config)      ((config)->pinSpiDi0.pinMux)
#define GC032A_PIN_MUX_SPI_DI1(config)      ((config)->pinSpiDi1.pinMux)

#define GC032A_I2C_BUS_NUM(config)          ((config)->i2cBusNum)

#define GC032A_MSB_MODE                     (1)

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct
{
    uint8_t reg;
    uint8_t value;
} regval_t;

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
// static GC032A_Handle g_gc032aHdl = {0};

static GC032A_PinConfig *g_gc032aPinConfig = NULL;

static const regval_t H_W_640_480[] = 
{
    //image
	{0xfe, 0x03},
	{0x5b, 0x80}, 
	{0x5c, 0x02},  
	{0x5d, 0xe0}, 
	{0x5e, 0x01}, 

    {0xfe, 0x00},   //window
    {0x55, 0x01},
    {0x56, 0xe0},
    {0x57, 0x02},
    {0x58, 0x80},

    {0xff, 0xff},
};

static const regval_t H_W_240_320[] = 
{
    //image
	{0xfe, 0x03},
	{0x5b, 0xf0}, 
	{0x5c, 0x00},  
	{0x5d, 0x40}, 
	{0x5e, 0x01}, 

    {0xfe, 0x00},   //window
    {0x55, 0x01},
    {0x56, 0x40},
    {0x57, 0x00},
    {0x58, 0xf0},

    {0xff, 0xff},
};

// width : 320, heigth: 240
static const regval_t regvalue[] = 
{
    {0xf3, 0x83},
	{0xf5, 0x0c},
	{0xf7, 0x01}, // gavin 20160820
	{0xf8, 0x01},
	{0xf9, 0x4e},
	{0xfa, 0x10}, // gavin 20160820
	{0xfc, 0x02},
	{0xfe, 0x02},
	{0x81, 0x03},
	{0xfe, 0x00},
	{0x77, 0x64},
	{0x78, 0x40},
	{0x79, 0x60},

	/*Analog&Cisctl*/
	{0xfe, 0x00},
	{0x03, 0x01},
	{0x04, 0xc2},
	{0x05, 0x01},
	{0x06, 0xb8},
	{0x07, 0x00},
	{0x08, 0x08},
	{0x0a, 0x00},
	{0x0c, 0x00},

	{0x0d, 0x01},
	{0x0e, 0xe8},
	{0x0f, 0x02},
	{0x10, 0x88},

	{0x17, 0x54},
	{0x19, 0x08},
	{0x1a, 0x0a},
	{0x1f, 0x40},
	{0x20, 0x30},
	{0x2e, 0x80},
	{0x2f, 0x2b},
	{0x30, 0x1a},
	{0xfe, 0x02},
	{0x03, 0x02},
	{0x05, 0xd7},
	{0x06, 0x60},
	{0x08, 0x80},
	{0x12, 0x89},

	/*SPI*/
	{0xfe, 0x03},
	{0x51, 0x01},
#if GC032A_MSB_MODE
    {0x52, 0xd8}, // DDR Disable gavin 20160820
#else
    {0x52, 0x58},
#endif
	{0x53, 0xa4},
	{0x54, 0x20},
	{0x55, 0x00},
	{0x59, 0x10},
	{0x5a, 0x00},

	{0x5b, 0x40},   // 宽度 240
	{0x5c, 0x01},   // 宽度 
	{0x5d, 0xf0},   // 高度  320
	{0x5e, 0x00},   // 高度

	{0x64, 0x04}, //SCK Always OFF , gavin 20160820

	/*blk*/
	{0xfe, 0x00},
	{0x18, 0x02},
	{0xfe, 0x02},
	{0x40, 0x22},
	{0x45, 0x00},
	{0x46, 0x00},
	{0x49, 0x20},
	{0x4b, 0x3c},
	{0x50, 0x20},
	{0x42, 0x10},

	/*isp*/
	{0xfe, 0x01},
	{0x0a, 0xc5},
	{0x45, 0x00},
	{0xfe, 0x00},
	{0x40, 0xff},
	{0x41, 0x25},
	{0x42, 0xcf},
	{0x43, 0x10},
	{0x44, 0x06},
	{0x46, 0x22},
	{0x49, 0x03},
	{0x52, 0x02},
	{0x54, 0x00},

    {0x55, 0x00},
    {0x56, 0xf0},
    {0x57, 0x01},
    {0x58, 0x40},

	{0xfe, 0x02},
	{0x22, 0xf6},

	/*Shading*/
	{0xfe, 0x01},
	{0xc1, 0x38},
	{0xc2, 0x4c},
	{0xc3, 0x00},
	{0xc4, 0x32},
	{0xc5, 0x24},
	{0xc6, 0x16},
	{0xc7, 0x08},
	{0xc8, 0x08},
	{0xc9, 0x00},
	{0xca, 0x20},
	{0xdc, 0x8a},
	{0xdd, 0xa0},
	{0xde, 0xa6},
	{0xdf, 0x75},

	/*AWB*/ /*20170110*/
	{0xfe, 0x01},
	{0x7c, 0x09},
	{0x65, 0x06},
	{0x7c, 0x08},
	{0x56, 0xf4},
	{0x66, 0x0f},
	{0x67, 0x84},
	{0x6b, 0x80},
	{0x6d, 0x12},
	{0x6e, 0xb0},
	{0xfe, 0x01},
	{0x90, 0x00},
	{0x91, 0x00},
	{0x92, 0xf4},
	{0x93, 0xd5},
	{0x95, 0x0f},
	{0x96, 0xf4},
	{0x97, 0x2d},
	{0x98, 0x0f},
	{0x9a, 0x2d},
	{0x9b, 0x0f},
	{0x9c, 0x59},
	{0x9d, 0x2d},
	{0x9f, 0x67},
	{0xa0, 0x59},
	{0xa1, 0x00},
	{0xa2, 0x00},
	{0x86, 0x00},
	{0x87, 0x00},
	{0x88, 0x00},
	{0x89, 0x00},
	{0xa4, 0x00},
	{0xa5, 0x00},
	{0xa6, 0xd4},
	{0xa7, 0x9f},
	{0xa9, 0xd4},
	{0xaa, 0x9f},
	{0xab, 0xac},
	{0xac, 0x9f},
	{0xae, 0xd4},
	{0xaf, 0xac},
	{0xb0, 0xd4},
	{0xb1, 0xa3},
	{0xb3, 0xd4},
	{0xb4, 0xac},
	{0xb5, 0x00},
	{0xb6, 0x00},
	{0x8b, 0x00},
	{0x8c, 0x00},
	{0x8d, 0x00},
	{0x8e, 0x00},
	{0x94, 0x50},
	{0x99, 0xa6},
	{0x9e, 0xaa},
	{0xa3, 0x0a},
	{0x8a, 0x00},
	{0xa8, 0x50},
	{0xad, 0x55},
	{0xb2, 0x55},
	{0xb7, 0x05},
	{0x8f, 0x00},
	{0xb8, 0xb3},
	{0xb9, 0xb6},

	/*CC*/
	{0xfe, 0x01},
	{0xd0, 0x40},
	{0xd1, 0xf8},
	{0xd2, 0x00},
	{0xd3, 0xfa},
	{0xd4, 0x45},
	{0xd5, 0x02},

	{0xd6, 0x30},
	{0xd7, 0xfa},
	{0xd8, 0x08},
	{0xd9, 0x08},
	{0xda, 0x58},
	{0xdb, 0x02},
	{0xfe, 0x00},

	/*Gamma*/
	{0xfe, 0x00},
	{0xba, 0x00},
	{0xbb, 0x04},
	{0xbc, 0x0a},
	{0xbd, 0x0e},
	{0xbe, 0x22},
	{0xbf, 0x30},
	{0xc0, 0x3d},
	{0xc1, 0x4a},
	{0xc2, 0x5d},
	{0xc3, 0x6b},
	{0xc4, 0x7a},
	{0xc5, 0x85},
	{0xc6, 0x90},
	{0xc7, 0xa5},
	{0xc8, 0xb5},
	{0xc9, 0xc2},
	{0xca, 0xcc},
	{0xcb, 0xd5},
	{0xcc, 0xde},
	{0xcd, 0xea},
	{0xce, 0xf5},
	{0xcf, 0xff},

	/*Auto Gamma*/
	{0xfe, 0x00},
	{0x5a, 0x08},
	{0x5b, 0x0f},
	{0x5c, 0x15},
	{0x5d, 0x1c},
	{0x5e, 0x28},
	{0x5f, 0x36},
	{0x60, 0x45},
	{0x61, 0x51},
	{0x62, 0x6a},
	{0x63, 0x7d},
	{0x64, 0x8d},
	{0x65, 0x98},
	{0x66, 0xa2},
	{0x67, 0xb5},
	{0x68, 0xc3},
	{0x69, 0xcd},
	{0x6a, 0xd4},
	{0x6b, 0xdc},
	{0x6c, 0xe3},
	{0x6d, 0xf0},
	{0x6e, 0xf9},
	{0x6f, 0xff},

	/*Gain*/
	{0xfe, 0x00},
	{0x70, 0x50},

	/*AEC*/
	{0xfe, 0x00},
	{0x4f, 0x01},
	{0xfe, 0x01},
	{0x0d, 0x00}, //08 add 20170110
	{0x12, 0xa0},
	{0x13, 0x3a},
	{0x44, 0x04},
	{0x1f, 0x30},
	{0x20, 0x40},
	{0x26, 0x9a},
	{0x3e, 0x20},
	{0x3f, 0x2d},
	{0x40, 0x40},
	{0x41, 0x5b},
	{0x42, 0x82},
	{0x43, 0xb7},
	{0x04, 0x0a},
	{0x02, 0x79},
	{0x03, 0xc0},

	/*measure window*/
	{0xfe, 0x01},
	{0xcc, 0x08},
	{0xcd, 0x08},
	{0xce, 0xa4},
	{0xcf, 0xec},

	/*DNDD*/
	{0xfe, 0x00},
	{0x81, 0xb8},
	{0x82, 0x12},
	{0x83, 0x0a},
	{0x84, 0x01},
	{0x86, 0x50},
	{0x87, 0x18},
	{0x88, 0x10},
	{0x89, 0x70},
	{0x8a, 0x20},
	{0x8b, 0x10},
	{0x8c, 0x08},
	{0x8d, 0x0a},

	/*Intpee*/
	{0xfe, 0x00},
	{0x8f, 0xaa},
	{0x90, 0x9c},
	{0x91, 0x52},
	{0x92, 0x03},
	{0x93, 0x03},
	{0x94, 0x08},
	{0x95, 0x44},
	{0x97, 0x00},
	{0x98, 0x00},

	/*ASDE*/
	{0xfe, 0x00},
	{0xa1, 0x30},
	{0xa2, 0x41},
	{0xa4, 0x30},
	{0xa5, 0x20},
	{0xaa, 0x30},
	{0xac, 0x32},

	/*YCP*/
	{0xfe, 0x00},
	{0xd1, 0x3c},
	{0xd2, 0x3c},
	{0xd3, 0x38},
	{0xd6, 0xf4},
	{0xd7, 0x1d},
	{0xdd, 0x73},
	{0xde, 0x84},
    // preview mode
    {0xfe, 0x01},
    {0x3c, 0x00},
    {0xfe, 0x00},

    {0xff, 0xff},
};

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
static I2C_BusDevice g_camI2c = {
    .addr = 0x21,
};

static int gc032a_write_reg(GC032A_Handle *hdl, uint8_t reg_addr, uint8_t data)
{
    uint8_t regConfig[2] = {0};

    regConfig[0] = reg_addr;
    regConfig[1] = data;

    I2C_BusLock(&g_camI2c);
    I2C_BusDevice_Write(&g_camI2c, regConfig, 2);
    I2C_BusUnlock(&g_camI2c);

    return 0;
}

static int gc032a_read_reg(GC032A_Handle *hdl, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    int ret;

    I2C_BusLock(&g_camI2c);
    ret = I2C_BusDevice_ReadReg(&g_camI2c, reg_addr, data, len);
    I2C_BusUnlock(&g_camI2c);

    return ret;
}

static int GC032A_I2CInit(GC032A_Handle *hdl, uint8_t busNum)
{
    I2C_BusDevice_Register(busNum, &g_camI2c);

    return 0;
}

static int GC032A_I2CDeInit(GC032A_Handle *hdl)
{


    return 0;
}

static int32_t GC032A_SetImageSize(GC032A_Handle *hdl, uint16_t imageW, uint16_t imageH)
{
    uint16_t i = 0;

    const regval_t val[] = 
    {
        //image
        {0xfe, 0x03},
        {0x5b, (uint8_t)(imageW & 0xff)}, 
        {0x5c, (uint8_t)(imageW >> 8)},  
        {0x5d, (uint8_t)(imageH & 0xff)}, 
        {0x5e, (uint8_t)(imageH >> 8)}, 

        {0xfe, 0x00},   //window
        {0x55, (uint8_t)(imageH >> 8)},
        {0x56, (uint8_t)(imageH & 0xff)},
        {0x57, (uint8_t)(imageW >> 8)},
        {0x58, (uint8_t)(imageW & 0xff)},

        {0xff, 0xff},
    };

    while(val[i].reg != 0xff)
    {
        if(gc032a_write_reg(hdl, val[i].reg, val[i].value)) {
            GC032A_PrintError("GC032A wirte reg error");
            return -1;
        }
        i++;
    }

    return 0;
}

static int32_t GC032A_DrvInit(GC032A_Handle *hdl, uint8_t busNum, uint16_t imageW, uint16_t imageH)
{
    uint16_t i = 0;

    if(GC032A_I2CInit(hdl, busNum)) {
        return -1;
    }

    while(regvalue[i].reg != 0xff)
    {
        if(gc032a_write_reg(hdl, regvalue[i].reg, regvalue[i].value)) {
            goto err;
        }
        i++;
    }

    i = 0;
    if(imageW == 640 && imageH == 480)
    {
        GC032A_PrintDebug("use 640*480");
        while(H_W_640_480[i].reg != 0xff)
        {
            if(gc032a_write_reg(hdl, H_W_640_480[i].reg, H_W_640_480[i].value)) {
                goto err;
            }
            i++;
        }
    }
    
    else if(imageW == 240 && imageH == 320)
    {
        GC032A_PrintDebug("use 240*320");
        while(H_W_240_320[i].reg != 0xff)
        {
            if(gc032a_write_reg(hdl, H_W_240_320[i].reg, H_W_240_320[i].value)) {
                goto err;
            }
            i++;
        }
    }
    else if(imageW == 320 && imageH == 240) {
        GC032A_PrintDebug("use 320*240");
        return 0;
    }
    else {
        return GC032A_SetImageSize(hdl, imageW, imageH);
    }

    GC032A_PrintDebug("cam id = %x", GC032A_IdGet(hdl));

   return 0;

err:
    GC032A_I2CDeInit(hdl);
    return -1;
}

void GC032A_PinInit(GC032A_PinConfig *config)
{
    g_gc032aPinConfig = config;
}

static void SPICAM_CallBack(void* data, uint32_t event)
{
    GC032A_Handle *hdl = (GC032A_Handle *)data;

    hdl->captureTotal++;

    if(hdl->cbEvent) {
        hdl->cbEvent(hdl->userData);
    }
}

uint32_t GC032A_IdGet(GC032A_Handle *hdl)
{
    uint8_t high, low;

    gc032a_read_reg(hdl, 0xf0, &high, 1);
    gc032a_read_reg(hdl, 0xf1, &low, 1);

    return (high << 8) | low;
}

static void GC032A_AwbSet(GC032A_Handle *hdl, uint8_t mode)
{

}

static void GC032A_BrightnessSet(GC032A_Handle *hdl, uint8_t level)
{
    
}

static void GC032A_ContrastSet(GC032A_Handle *hdl, uint8_t level)
{
    
}

static void GC032A_EvSet(GC032A_Handle *hdl, uint8_t level)
{
    
}

void GC032A_Control(GC032A_Handle *hdl, uint32_t cmd, uint32_t arg)
{
    switch (cmd) {
        case GC032A_SET_AWB:
            GC032A_AwbSet(hdl, arg);
            break;

        case GC032A_SET_BRIGHTNESS:
            GC032A_BrightnessSet(hdl, arg);
            break;

        case GC032A_SET_CONTRAST:
            GC032A_ContrastSet(hdl, arg);
            break;

        case GC032A_SET_EV:
            GC032A_EvSet(hdl, arg);
            break;
        
        default:
            break;
    }
}

void GC032A_PowerControl(GC032A_Handle *hdl, bool en)
{
    if(en) {
        EXTLDO_Acquire(EXT_LDO_1V8);
        EXTLDO_Acquire(EXT_LDO_2V8);
    }
    else {
        EXTLDO_Release(EXT_LDO_1V8);
        EXTLDO_Release(EXT_LDO_2V8);
    }
}

int32_t GC032A_Init(GC032A_Handle *hdl, void (*callback)(void *data), void *data, uint16_t width, uint16_t height)
{
    OS_ASSERT(g_gc032aPinConfig);

    GC032A_PinConfig *config = g_gc032aPinConfig;
    SPICAM_Handle *spiCamHdl = &hdl->spiCam;

    hdl->pinConfig = config;

    hdl->cbEvent = callback;
    hdl->userData = data;
    hdl->ctrl = 0;

    GC032A_PowerControl(hdl, true);
    
    PIN_SetMux(GC032A_PIN_RES_MCLK(config), GC032A_PIN_MUX_MCLK(config));               // MCLK 没有专用管脚，仅仅复用到GPIO管脚
    GPIO_SetDir(GC032A_PIN_RES_MCLK(config), GPIO_OUTPUT);
    PIN_SetMux(GC032A_PIN_RES_POWER_DOWN(config), GC032A_PIN_MUX_POWER_DOWN(config));   // SPI cam cs
    PIN_SetMux(GC032A_PIN_RES_SPI_CLK(config), GC032A_PIN_MUX_SPI_CLK(config));         // clk
    PIN_SetMux(GC032A_PIN_RES_SPI_DI0(config), GC032A_PIN_MUX_SPI_DI0(config));         // di0
    PIN_SetMux(GC032A_PIN_RES_SPI_DI1(config), GC032A_PIN_MUX_SPI_DI1(config));         // di1

    //Power On
    GPIO_SetDir(GC032A_PIN_RES_POWER_DOWN(config), GPIO_OUTPUT);
    GPIO_Write(GC032A_PIN_RES_POWER_DOWN(config), GPIO_LOW);
    osThreadMsSleep(10);
    GPIO_Write(GC032A_PIN_RES_POWER_DOWN(config), GPIO_HIGH);
    osThreadMsSleep(10);
    GPIO_Write(GC032A_PIN_RES_POWER_DOWN(config), GPIO_LOW);
    osThreadMsSleep(10);

    spiCamHdl->cbEvent = SPICAM_CallBack;
    spiCamHdl->userData = hdl;
    spiCamHdl->res  = DRV_RES(CAM, 0);

    if(DRV_OK != SPICAM_Initialize(spiCamHdl)) {
        GC032A_PrintError("SPICAM Init Fail");
        GC032A_PowerControl(hdl, false);
        return -1;
    }

    SPICAM_Control(spiCamHdl, SPICAM_CONFIG_FRAME_WIDTH, width);
    SPICAM_Control(spiCamHdl, SPICAM_CONFIG_FREME_HEIGHT, height);
    SPICAM_Control(spiCamHdl, SPICAM_CONFIG_FRAME_DEPTH, CAMERA_BUFFER_DEPTH);
    SPICAM_Control(spiCamHdl, SPICAM_CONFIG_LINE_NUM, SPI_CAM_2_WIRE);

    SPICAM_PowerControl(spiCamHdl, DRV_POWER_FULL);

    if(GC032A_DrvInit(hdl, GC032A_I2C_BUS_NUM(config), width, height)) {
        GC032A_PrintError("GC032A Init Fail");
        return -1;
    }

    hdl->ctrl |= GC032A_INIT;

#ifdef OS_USING_PM
    PSM_WakelockInit(&hdl->wakeLock, PSM_DEEP_SLEEP);
#endif

    GC032A_PrintDebug("GC032A init");

    return 0;
}

int32_t GC032A_DeInit(GC032A_Handle *hdl)
{
    SPICAM_Handle *spiCamHdl = &hdl->spiCam;

    if(!(hdl->ctrl & GC032A_INIT)){
        return 0;
    }

    if(hdl->ctrl & GC032A_RUNNING){
        GC032A_PrintError("GC032A is running");
        return -1;
    }

    if(hdl->captureCached != 0) {
        GC032A_PrintError("GC032A has image used");
        return -1;
    }

    SPICAM_PowerControl(spiCamHdl, DRV_POWER_OFF);

    SPICAM_UnInitialize(spiCamHdl);

    hdl->ctrl &= ~GC032A_INIT;

    GC032A_PowerControl(hdl, false);

    GC032A_PrintDebug("GC032A deinit");

    return 0;
}

int32_t GC032A_Start(GC032A_Handle *hdl)
{
    SPICAM_Handle *spiCamHdl = &hdl->spiCam;

    if(hdl->ctrl & GC032A_RUNNING){
        GC032A_PrintDebug("GC032A is running, no need start");
        return 0;
    }

#ifdef OS_USING_PM
    PSM_WakeLock(&hdl->wakeLock);
#endif

    SPICAM_Start(spiCamHdl);

    hdl->ctrl |= GC032A_RUNNING;

    GC032A_PrintDebug("GC032A start");

    return 0;
}

int32_t GC032A_Stop(GC032A_Handle *hdl)
{
    SPICAM_Handle *spiCamHdl = &hdl->spiCam;

    if(!(hdl->ctrl & GC032A_RUNNING)){
        GC032A_PrintDebug("GC032A is stopped, no need stop");
        return 0;
    }

    SPICAM_Stop(spiCamHdl);

    hdl->ctrl &= ~GC032A_RUNNING;

#ifdef OS_USING_PM
    PSM_WakeUnlock(&hdl->wakeLock);
#endif

    GC032A_PrintDebug("GC032A stop");

    return 0;
}

uint8_t* GC032A_Capture(GC032A_Handle *hdl)
{
    SPICAM_Handle *spiCamHdl = &hdl->spiCam;
    uint8_t *image;

    if(!(hdl->ctrl & GC032A_RUNNING)){
        GC032A_PrintError("GC032A is not running");
        return NULL;
    }

    image = SPICAM_ImageCapture(spiCamHdl);
    if(image) {
        hdl->captureCached++;
    }

    return image;
}

int32_t GC032A_ImageRelease(GC032A_Handle *hdl, uint8_t *image)
{
    SPICAM_Handle *spiCamHdl = &hdl->spiCam;

    OS_ASSERT(image != NULL);

    SPICAM_ImageFree(spiCamHdl, image);

    hdl->captureCached--;

    return 0;
}

uint32_t camGc032aGetId(GC032A_Handle *hdl)
{
    return GC032A_IdGet(hdl);
}

bool camGc032aInit(GC032A_Handle *hdl, void (*callback)(void *data), void *data, uint16_t width, uint16_t height)
{
    if(GC032A_Init(hdl, callback, data, width, height)) {
        return false;
    }

    return true;
}

bool camGc032aDeInit(GC032A_Handle *hdl)
{
    if(GC032A_DeInit(hdl)) {
        return false;
    }

    return true;
}

int32_t camGc032aPrevStart(GC032A_Handle *hdl)
{
    return GC032A_Start(hdl);
}

int32_t camGc032aStopPrev(GC032A_Handle *hdl)
{
    return GC032A_Stop(hdl);
}

uint8_t* camGc032aCaptureImage(GC032A_Handle *hdl)
{
    return GC032A_Capture(hdl);
}

void camGc032aImageRelease(GC032A_Handle *hdl, uint8_t *image)
{
    GC032A_ImageRelease(hdl, image);
}

bool camGc032aReg(SensorOps_t *pSensorOpsCB)
{
        pSensorOpsCB->cameraInit = camGc032aInit;
        pSensorOpsCB->cameraDeInit = camGc032aDeInit;
        pSensorOpsCB->cameraGetID = camGc032aGetId;
        pSensorOpsCB->cameraCaptureImage = camGc032aCaptureImage;
        pSensorOpsCB->cameraStartPrev = camGc032aPrevStart;
        pSensorOpsCB->cameraStopPrev = camGc032aStopPrev;
        pSensorOpsCB->cameraImageRelease = camGc032aImageRelease;
        return true;
}