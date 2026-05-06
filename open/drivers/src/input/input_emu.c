/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        input_emu.c
 *
 * @brief       模拟器输入设备.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team         创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "drv_input.h"
#include "drv_kti.h"

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
static KTI_Handle g_emuKtiKeyboard;
static KTI_Handle g_emuKtiTouch;
static uint8_t g_emuKtiKeyboardInitCount;
static uint8_t g_emuKtiKeypadCode;
static uint8_t g_emuKtiButtonCode;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
 
void InputEmu_TouchRead(struct InputDevice *pDev, lv_indev_data_t *data)
{
    InputTouch_t *pTouchDev = (InputTouch_t*)pDev;
    uint8_t ktiData[8];
    int ret;
    
    ret = KTI_GetData(&g_emuKtiTouch, ktiData, 8);
    if(ret > 0)
    {        
        if(ktiData[0])
        {
            data->state = LV_INDEV_STATE_PR;
        }
        else
        {
            data->state = LV_INDEV_STATE_REL;
        }
        /*Set the last pressed coordinates*/
        data->point.x = (((uint32_t)ktiData[1] | (uint32_t)ktiData[2] << 8) * pTouchDev->touchWidth) >> 15;
        data->point.y = (((uint32_t)ktiData[3] | (uint32_t)ktiData[4] << 8) * pTouchDev->touchHeight) >> 15;
        
        //osPrintf("Touch: len %d, %02x %02x %02x %02x %02x %02x\r\n", ret, ktiData[0], ktiData[1], ktiData[2], ktiData[3], ktiData[4], ktiData[5]);
    }
    
}

static int InputEmu_TouchInit(InputDevice_t *pDev)
{
    g_emuKtiTouch.pRes = DRV_RES(KTI, 1);
    g_emuKtiTouch.userData = pDev;
    
    return KTI_Initialize(&g_emuKtiTouch);
}

static int InputEmu_TouchDeinit(InputDevice_t *pDev)
{
    return KTI_Uninitialize(&g_emuKtiTouch);
}

/**
 ************************************************************************************
 * @brief           获取模拟器触摸屏输入设备
 *
 * @return          输入设备.
 * @retval          InputDevice_t                      
 ************************************************************************************
 */
/*
 * 操作设备
 */
static InputTouch_t g_inputTouchDev ={
.inputDev = {
        .type = INPUT_TYPE_TOUCH,
        .init = InputEmu_TouchInit,
        .deinit = InputEmu_TouchDeinit,    
        .read = InputEmu_TouchRead,
    },
};

InputDevice_t * InputEmu_GetTouchDevice(void)
{
    return &g_inputTouchDev.inputDev;
}

static void InputEmu_KeyboardCallback(void *data)
{
    uint8_t ktiData[8];
    int ret;
    
    ret = KTI_GetData(&g_emuKtiKeyboard, ktiData, 8);
    if(ret > 0)
    {
        uint8_t key = ktiData[2];
        
        if((key >= 0x4) && (key <= 0x1D))
        {
            g_emuKtiKeypadCode = 'a' + key - 0x4;
        }
        else if((key >= 0x1E) && (key <= 0x27))
        {
            if(key == 0x27)
            {
                g_emuKtiKeypadCode = '0';
            }
            else
            {
                g_emuKtiKeypadCode = '1' + key - 0x1E;
            }
        }
        else if(key == 0x4F)
        {
            g_emuKtiKeypadCode = LV_KEY_RIGHT;
        }
        else if(key == 0x51)
        {
            g_emuKtiKeypadCode = LV_KEY_DOWN;
        }
        else if(key == 0x52)
        {
            g_emuKtiKeypadCode = LV_KEY_UP;
        }
        else if(key == 0x28)
        {
            g_emuKtiKeypadCode = LV_KEY_ENTER;
        }
        else if(key == 0x29)
        {
            g_emuKtiKeypadCode = LV_KEY_ESC;
        }
        else if(key == 0x2A)
        {
            g_emuKtiKeypadCode = LV_KEY_BACKSPACE;
        }
        else if(key == 0x3A)  //F1
        {
            g_emuKtiButtonCode = 1;
        }
        else if(key == 0x3B)  //F2
        {
            g_emuKtiButtonCode = 2;
        }
        else if(key == 0x3C)  //F3
        {
            g_emuKtiButtonCode = 3;
        }

        //osPrintf("Keyboard: len %d, %02x %02x %02x %02x %02x %02x\r\n", ret, ktiData[0], ktiData[1], ktiData[2], ktiData[3], ktiData[4], ktiData[5]);
    }    
}

static int InputEmu_KeyboardInit(InputDevice_t *pDev)
{
    if(g_emuKtiKeyboardInitCount)
    {
        return 0;
    }
    
    g_emuKtiKeyboard.pRes = DRV_RES(KTI, 0);
    g_emuKtiKeyboard.userData = pDev;
    g_emuKtiKeyboard.InputCallback = InputEmu_KeyboardCallback;

    g_emuKtiKeyboardInitCount++;
    
    return KTI_Initialize(&g_emuKtiKeyboard);
}

static int InputEmu_KeyboardDeinit(InputDevice_t *pDev)
{    
    g_emuKtiKeyboardInitCount--;

    if(g_emuKtiKeyboardInitCount)
    {
        return 0;
    }
    else
    {
        return KTI_Uninitialize(&g_emuKtiKeyboard);
    }
}

void InputEmu_KeypadRead(struct InputDevice *pDev, lv_indev_data_t *data)
{
   if(g_emuKtiKeypadCode)
   {
       data->key = g_emuKtiKeypadCode;
       data->state = LV_INDEV_STATE_PR;
       g_emuKtiKeypadCode = 0;
   }
   else
   {
       data->state = LV_INDEV_STATE_REL;
   }
}

static int InputEmu_KeypadInit(InputDevice_t *pDev)
{
    return InputEmu_KeyboardInit(pDev);
}

static int InputEmu_KeypadDeinit(InputDevice_t *pDev)
{
    return InputEmu_KeyboardDeinit(pDev);
}

static InputDevice_t g_inputKeypadDev ={
    .type = INPUT_TYPE_KEYPAD,
    .init = InputEmu_KeypadInit,
    .deinit = InputEmu_KeypadDeinit,    
    .read = InputEmu_KeypadRead,
};

InputDevice_t * InputEmu_GetKeypadDevice(void)
{
    return &g_inputKeypadDev;
}

void InputEmu_ButtonRead(struct InputDevice *pDev, lv_indev_data_t *data)
{
    if(g_emuKtiButtonCode)
    {
        data->btn_id = g_emuKtiButtonCode - 1;
        data->state = LV_INDEV_STATE_PR;
        g_emuKtiButtonCode = 0;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

static int InputEmu_ButtonInit(InputDevice_t *pDev)
{
    return InputEmu_KeyboardInit(pDev);
}

static int InputEmu_ButtonDeinit(InputDevice_t *pDev)
{
    return InputEmu_KeyboardDeinit(pDev);
}

static InputDevice_t g_inputButtonDev ={
    .type = INPUT_TYPE_BUTTON,
    .init = InputEmu_ButtonInit,
    .deinit = InputEmu_ButtonDeinit,
    .read = InputEmu_ButtonRead,
};

InputDevice_t * InputEmu_GetButtonDevice(void)
{
    return &g_inputButtonDev;
}

