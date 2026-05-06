/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-01     ict team          创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "at_api.h"
#include "app_pub.h"
#include "app_pseudo_at_device.h"
/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static APP_PseudoATDevice_Notify g_App_PseudoATDevice_Notify = NULL;
/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/

/***************************
 *   向AT虚拟设备写入数据，即向AT框架发送命令
 *
 *   注意：
 *   由于AT框架任务优先级高，发送命令后会立即执行，所以APP_PseudoATDevice_Read有可能会立即被调用
 *   在APP_PseudoATDevice_Read中依赖的状态需要在调用AT_DeviceReceived前设置
 */
static int32_t APP_PseudoATDevice_Write(char* data, uint32_t dataLen)
{
    int32_t ret;
    ret = AT_DeviceReceived(AT_CHANNEL_ID_CUSTOM1, data, dataLen);
    if(ret != osOK)
    {
        //出错处理, 可以sleep一会重试
        OS_ASSERT(0);
    }

    return ret;
}


/***************************
 *   从AT虚拟设备读取数据，即从AT框架接收命令响应和主动上报
 *
 *   注意：
 *   该回调函数注册给了AT框架，函数会在AT框架任务中执行，不能在该回调函数中阻塞或挂起任务
 *   回调函数中的data指向的内存仅限回调函数内使用，当需要将data发送给其他任务时，需要使用新内存并copy
 */
static int32_t APP_PseudoATDevice_Read(uint8_t channelId, char* data, uint32_t dataLen)
{
    // TODO 客户处理函数
    if(g_App_PseudoATDevice_Notify != NULL)
    {
        g_App_PseudoATDevice_Notify(data, dataLen);
    }
    AT_DeviceSendDone(channelId); //   通知AT框架释放内存,  必须要调用，否则有内存泄漏

    return osOK;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/

/**
 ************************************************************************************
 *   AT虚拟设备初始化
 *   实际是打开一个AT框架的通道
 *   在向AT虚拟设备写入数据前，需要初始化AT虚拟设备
 ************************************************************************************
*/
int32_t APP_PseudoATDevice_Initialize(void)
{
    AT_AllocChannel(AT_CHANNEL_ID_CUSTOM1, NULL, APP_PseudoATDevice_Read, APP_PseudoATDevice_Read, AT_CHANNEL_ATTRS_TIMER);
    AT_OpenChannel(AT_CHANNEL_ID_CUSTOM1);

    return osOK;
}

/**
 ************************************************************************************
 *   AT虚拟设备去初始化
 *   实际是关闭AT框架的通道
 *   当确认不需要使用AT虚拟设备时，可以去注册AT虚拟设备
 ************************************************************************************
*/
int32_t APP_PseudoATDevice_Uninitialize(void)
{
    AT_CloseChannel(AT_CHANNEL_ID_CUSTOM1);
    AT_FreeChannel(AT_CHANNEL_ID_CUSTOM1);

    return osOK;
}

/**
 ************************************************************************************
 *   注册AT虚拟设备通知回调
 *   当AT虚拟设备有数据要上报时会调用这个回调
 ************************************************************************************
*/
void APP_PseudoATDevice_Register_Notify(APP_PseudoATDevice_Notify notify)
{
    g_App_PseudoATDevice_Notify = notify;

    return;
}

#if 0
#include <stdlib.h>
#include "nr_micro_shell.h"

static void AT_TestPesudoDevice_Notify(const char *data, uint32_t dataLen)
{
    osPrintf("AT_TestDevice_Notify [%s]\r\n", data);
}

void AT_TestPesudoDevice(char argc, char **argv)
{
    int test_cmd = 0;
    test_cmd = atoi(argv[1]);
    int32_t ret;

    if(test_cmd == 1){
        //   注册回调，并初始化
        APP_PseudoATDevice_Register_Notify(AT_TestPesudoDevice_Notify);
        ret = APP_PseudoATDevice_Initialize();
        osPrintf("APP_PseudoATDevice_Init [%d]\r\n", ret);
    }

    if(test_cmd == 2){
        //   清空回调，并去初始化
        APP_PseudoATDevice_Register_Notify(NULL);
        ret = APP_PseudoATDevice_Uninitialize();
        osPrintf("APP_PseudoATDevice_DeInit [%d]\r\n", ret);
    }

    if(test_cmd == 3){
        if(argv[2] != NULL)
        {
            //   写入命令
            ret = APP_PseudoATDevice_Write(argv[2], strlen(argv[2]));
            ret |= APP_PseudoATDevice_Write("\r\n", 2);
            osPrintf("APP_PseudoATDevice_Write [%d][%s]\r\n", ret, argv[2]);
        }
    }
}
NR_SHELL_CMD_EXPORT(AT_PesudoDevice, AT_TestPesudoDevice);

#endif
