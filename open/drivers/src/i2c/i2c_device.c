#include <i2c_device.h>
#include <drv_pin.h>
#ifdef OS_USING_PM
#include <psm_sys.h>
#include <drv_lp.h>
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define I2C_BUS_PIN_RES_I2C_SCL(bus)        ((bus)->pin->sclRes)
#define I2C_BUS_PIN_RES_I2C_SDA(bus)        ((bus)->pin->sdaRes)
#define I2C_BUS_PIN_MUX_I2C_SCL(bus)        ((bus)->pin->sclMux)
#define I2C_BUS_PIN_MUX_I2C_SDA(bus)        ((bus)->pin->sdaMux)
#define I2C_DEVICE_BUS(device)              ((device)->bus)

#define I2C_BUS_TRANS_END                   (1)

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
static I2C_BusDevice *g_i2cBusDeviceList[I2C_BUS_NUM] = {0};
static I2C_Bus g_i2cBus[I2C_BUS_NUM] = {0};
static uint32_t g_i2cEvent = 0;
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
static inline void I2C_SDA_SCL_Init(void *param)
{
    I2C_Bus *bus = (I2C_Bus *)param;

    PIN_SetMux(I2C_BUS_PIN_RES_I2C_SCL(bus), I2C_BUS_PIN_MUX_I2C_SCL(bus));
    PIN_SetMux(I2C_BUS_PIN_RES_I2C_SDA(bus), I2C_BUS_PIN_MUX_I2C_SDA(bus));

    GPIO_SetDir(I2C_BUS_PIN_RES_I2C_SCL(bus), GPIO_OUTPUT);
    GPIO_SetDir(I2C_BUS_PIN_RES_I2C_SDA(bus), GPIO_OUTPUT);
}

static void I2C_SDA_Write(void *param, uint8_t value)
{
    I2C_Bus *bus = (I2C_Bus *)param;

    GPIO_Write(I2C_BUS_PIN_RES_I2C_SDA(bus), value);
}

static uint8_t I2C_SDA_Read(void *param)
{
    I2C_Bus *bus = (I2C_Bus *)param;

    return GPIO_Read(I2C_BUS_PIN_RES_I2C_SDA(bus));
}

static void I2C_SDA_Dir(void *param, I2C_SOFT_GPIO_DIR dir)
{
    I2C_Bus *bus = (I2C_Bus *)param;
    GPIO_DIR gpio_dir = (dir == I2C_SOFT_GPIO_INPUT_MODE ? GPIO_INPUT : GPIO_OUTPUT);

    GPIO_SetDir(I2C_BUS_PIN_RES_I2C_SDA(bus), gpio_dir);
}

static void I2C_SCL_Write(void *param, uint8_t value)
{
    I2C_Bus *bus = (I2C_Bus *)param;

    GPIO_Write(I2C_BUS_PIN_RES_I2C_SCL(bus), value);
}

#if I2C_BUS_USING_SOFT
static I2C_SOFT_Ops i2c_soft_osp = {
    .init       = I2C_SDA_SCL_Init,
    .sda_write  = I2C_SDA_Write,
    .sda_read   = I2C_SDA_Read,
    .sda_dir    = I2C_SDA_Dir,
    .scl_write  = I2C_SCL_Write,
};
#endif

static void I2C_BusCallback(void *i2c, uint32_t event)
{
    I2C_Handle *hdl = (I2C_Handle *)i2c;
    if(!hdl->userData) {
        return;
    }

    I2C_Bus *bus = (I2C_Bus *)hdl->userData;

    g_i2cEvent = event;
    
    if(bus->threadId) {
        osThreadFlagsSet(bus->threadId, I2C_BUS_TRANS_END);
        bus->threadId = NULL;
    }
}

static int32_t I2C_BusInit(I2C_Bus *bus, uint32_t busNum)
{
    bus->num = busNum;
    // I2C pin mux
    PIN_SetMux(I2C_BUS_PIN_RES_I2C_SCL(bus), I2C_BUS_PIN_MUX_I2C_SCL(bus));
    PIN_SetMux(I2C_BUS_PIN_RES_I2C_SDA(bus), I2C_BUS_PIN_MUX_I2C_SDA(bus));

#if I2C_BUS_USING_SOFT
    bus->softI2c.userData = (void *)bus;
    if(I2C_SOFT_Initialize(&bus->softI2c, "i2c", &i2c_soft_osp)) {
        return -1;
    }
#else
    bus->i2c.p_I2C_Res = (void *)DRV_RES(I2C, bus->num);
    bus->i2c.userData = bus;
    bus->threadId = NULL;
    
    I2C_Initialize(&bus->i2c, I2C_BusCallback);
    I2C_PowerControl(&bus->i2c, DRV_POWER_FULL);
    I2C_Control(&bus->i2c, I2C_BUS_SPEED, I2C_BUS_SPEED_STANDARD);
#endif

    osMutexInit(&bus->lock, OS_IPC_FLAG_PRIO);

#ifdef OS_USING_PM
    PSM_WakelockInit(&bus->wakeLock, PSM_DEEP_SLEEP);
#endif

    bus->ref = 1;

    return 0;
}

static int32_t I2C_BusDeInit(I2C_Bus *bus)
{
#if I2C_BUS_USING_SOFT
    
#else
    I2C_PowerControl(&bus->i2c, DRV_POWER_OFF);
    I2C_Uninitialize(&bus->i2c);
#endif

    osMutexDetach(&bus->lock);

#ifdef OS_USING_PM
    // psm wakelock deinit
#endif

    return 0;
}

void I2C_BusPinInit(uint32_t busNum, I2C_BusPin *pin)
{
    if((busNum + 1) > I2C_BUS_NUM) {
        return;
    }

    g_i2cBus[busNum].pin = pin;
}

void I2C_BusDevice_Register(uint32_t busNum, I2C_BusDevice *device)
{
    OS_ASSERT((busNum + 1) <= I2C_BUS_NUM);
    OS_ASSERT(g_i2cBus[busNum].pin);

    I2C_Bus *bus = &g_i2cBus[busNum];
    
    device->bus = bus;
    device->next = NULL;

    // 如果当前总线未初始化，则初始化
    if(g_i2cBusDeviceList[busNum] == NULL) {
        g_i2cBusDeviceList[busNum] = device;
        I2C_BusInit(bus, busNum);
        return;
    }

    I2C_BusDevice *devList = g_i2cBusDeviceList[busNum];

    while (devList->next)
    {
        devList = devList->next;
    }

    devList->next = device;
}

void I2C_BusDevice_UnRegister(uint32_t busNum, I2C_BusDevice *device)
{
    OS_ASSERT((busNum + 1) <= I2C_BUS_NUM);

    if(g_i2cBusDeviceList[busNum] == NULL) {
        return;
    }

    I2C_BusDevice *dev = g_i2cBusDeviceList[busNum];

    do
    {
        if(dev == device) {
            break;
        }

        dev = dev->next;
    } while (dev);
}


int32_t I2C_BusLock(I2C_BusDevice *device)
{
    int ret = osMutexAcquire(&I2C_DEVICE_BUS(device)->lock, osWaitForever);

#ifdef OS_USING_PM
    PSM_WakeLock(&I2C_DEVICE_BUS(device)->wakeLock);
#endif

    return ret;
}

int32_t I2C_BusUnlock(I2C_BusDevice *device)
{
#ifdef OS_USING_PM
    PSM_WakeUnlock(&I2C_DEVICE_BUS(device)->wakeLock);
#endif

    return osMutexRelease(&I2C_DEVICE_BUS(device)->lock);
}

int32_t I2C_BusDevice_ReadReg(I2C_BusDevice *device, uint8_t reg, uint8_t *data, uint32_t len)
{
    int32_t ret = 0;

#if I2C_BUS_USING_SOFT
    I2C_SOFT_ReadBytes(&I2C_DEVICE_BUS(device)->softI2c, device->addr, reg, data, len);
    return 0;
#else
    I2C_DEVICE_BUS(device)->threadId = osThreadSelf();
    I2C_Control(&I2C_DEVICE_BUS(device)->i2c, I2C_BUS_SPEED, I2C_BUS_SPEED_STANDARD);
    ret = I2C_MasterTransfer(&I2C_DEVICE_BUS(device)->i2c, device->addr, &reg, 1, data, len);
    if(osThreadFlagsWait(I2C_BUS_TRANS_END, osFlagsWaitAny, I2C_BUS_TIMEOUT) == osFlagsErrorTimeout){
        I2C_SoftReset(&I2C_DEVICE_BUS(device)->i2c);
        return DRV_ERR_TIMEOUT;
    }

    return ret;
#endif
}

int32_t I2C_BusDevice_Write(I2C_BusDevice *device, uint8_t *data, uint32_t len)
{
    int32_t ret = 0;

#if I2C_BUS_USING_SOFT
    if (I2C_SOFT_WriteBytes(&I2C_DEVICE_BUS(device)->softI2c, device->addr, data, len)) {
        osPrintf("soft i2c write error\r\n");
        return -1;
    }
#else
    I2C_DEVICE_BUS(device)->threadId = osThreadSelf();
    I2C_Control(&I2C_DEVICE_BUS(device)->i2c, I2C_BUS_SPEED, I2C_BUS_SPEED_STANDARD);
    ret = I2C_MasterSend(&I2C_DEVICE_BUS(device)->i2c, device->addr, data, len);
    if(osThreadFlagsWait(I2C_BUS_TRANS_END, osFlagsWaitAny, I2C_BUS_TIMEOUT) == osFlagsErrorTimeout){
        I2C_SoftReset(&I2C_DEVICE_BUS(device)->i2c);
        return DRV_ERR_TIMEOUT;
    }
#endif

    return ret;
}

#if 0
#include "nr_micro_shell.h"
static I2C_BusDevice g_testI2c = {
    .addr = 0x11,
};

static void i2c_funcThread(void *param)
{
    uint32_t i = 1;
    uint8_t data[2] = {0};

    while (1)
    {
        data[0] = i;
        data[1] = i;
        I2C_BusLock(&g_testI2c);
        osPrintf("i2c send start : 0x%02x\r\n", i++);
        I2C_BusDevice_Write(&g_testI2c, data, 2);
        I2C_BusUnlock(&g_testI2c);

        osDelay(1000);
    }
}

static i2c_func(char argc, char **argv)
{
    I2C_BusDevice_Register(2, &g_testI2c);

    osThreadAttr_t attr = {"testI2c", osThreadDetached, NULL, 0U, NULL, 1024, osPriorityISR - 20, 0U, 0U};
    osThreadNew(i2c_funcThread, NULL, &attr);
}

NR_SHELL_CMD_EXPORT(i2c, i2c_func);
#endif
