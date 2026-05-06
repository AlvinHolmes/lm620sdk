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
#include "drv_uart.h"
#include "board.h"
#include "at_api.h"
#include "nvm.h"
#include "app_pub.h"
#include "platcfg.h"
#include "amt.h"
#if defined(OS_USING_PM) && defined(_CPU_AP)
#include <psm_wakelock.h>
#endif
#include "app_urc_ring_indicate.h"

/************************************************************************************
 *                                 外部函数声明
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
static UART_Handle *g_AT_UartHandle;
static bool g_AT_UartRIBusy = false;
#ifdef AT_CMDSET_Q
static const char* g_AT_Uart_FilterOutUrcList[] =
{
    "\r\n+CMURDY: ",
    "\r\n+CMPBIC: ",
    "\r\n+CMGIPDNS: ",
    "\r\n+NETSTAT: ",
    "\r\n+NETDEVCTL: "
};
#else
static const char* g_AT_Uart_FilterOutUrcList[] =
{
    "\r\n+URC_OUT_DEMO: ",
};
#endif
/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void AT_UART_Callback(void *UART, uint32_t event)
{
    if(event & UART_EVENT_SEND_COMPLETE)
    {
        event &= (~(uint32_t)(UART_EVENT_SEND_COMPLETE));
        (void)AT_DeviceSendDone(AT_CHANNEL_ID_UART);
    }

    if(event & UART_EVENT_RECEIVE_COMPLETE)
    {
        uint32_t rxCount;
        event &= (~(uint32_t)(UART_EVENT_RECEIVE_COMPLETE));
        rxCount = UART_GetRxCount(UART);
        (void)AT_DeviceReceived(AT_CHANNEL_ID_UART, NULL, rxCount);
    }
}

static int32_t AT_UARTReceive(uint8_t channelId, char* buf, uint32_t bufLen)
{
    (void)channelId;
    if(g_AT_UartHandle != NULL)
    {
        if(buf == NULL)
        {
            UART_Control(g_AT_UartHandle, UART_ABORT_RECEIVE, 0);
        }
        else
        {
            UART_Receive(g_AT_UartHandle, buf, bufLen);
        }
    }
    return 0;
}

static int32_t AT_UARTSend(uint8_t channelId, char* data, uint32_t dataLen)
{

    if(g_AT_UartHandle != NULL)
    {
        int32_t err;
        err = UART_Send(g_AT_UartHandle, data, dataLen);
        if(err != DRV_OK)
        {
            APP_PRINT_ERROR("AT_UARTSend Fail %d\r\n", err);
            AT_DeviceSendDone(channelId); //   通知AT框架释放内存
        }
    }
    else
    {
        AT_DeviceSendDone(channelId);
    }
    return 0;
}

//   add filter out urc string in g_AT_Uart_FilterOutUrcList
static bool AT_UARTFilterOutUnsolicited(char* urcStr, uint32_t urcLen)
{
    uint32_t i;

    if(urcStr != NULL)
    {
        //  过滤平台部分自定义命令
        for(i = 0; i < sizeof(g_AT_Uart_FilterOutUrcList)/sizeof(g_AT_Uart_FilterOutUrcList[0]); i++)
        {
            if(strncmp(urcStr, g_AT_Uart_FilterOutUrcList[i], strlen(g_AT_Uart_FilterOutUrcList[i])) == 0)
            {
                return true;
            }
        }
    }
    return false;
}

static void AT_UARTRISetStatus(bool status)
{
    //  写需要串行，加锁保护
    osKernelLock();
    g_AT_UartRIBusy = status;
    osKernelUnlock();
    return;
}

static bool AT_UARTRIIsBusy(void)
{
    //  读不修改变量值，不需要保护
    return g_AT_UartRIBusy;
}

static int32_t AT_UnsolicitedTimeOut(uint8_t channelId, char* data, uint32_t dataLen)
{
    AT_UARTRISetStatus(false);
    AT_ChannelUnsolicitedIndicateTimeOut(channelId);
    return 0;
}

static int32_t AT_UARTSendUnsolicited(uint8_t channelId, char* data, uint32_t dataLen)
{
    if(AT_UARTFilterOutUnsolicited(data, dataLen))
    {
        APP_PRINT_INFO("AT_UARTFilterOutUnsolicited CHL:%u %s\r\n", channelId, data);
        AT_DeviceSendDone(channelId);
    }
    else
    {
        if(APP_URC_RI_Start(channelId, data, dataLen, AT_UARTSend) != osOK)
        {
            AT_UARTSend(channelId, data, dataLen);
        }
    }
    return 0;
}

static int32_t AT_UARTUnsolicitedRI(uint8_t channelId, char* data, uint32_t dataLen)
{
    if(AT_UARTFilterOutUnsolicited(data, dataLen))
    {
        APP_PRINT_INFO("AT_UARTUnsolicitedRI CHL:%u %s\r\n", channelId, data);
        return AT_URC_RI_DROP;
    }
    else
    {
        APP_PRINT_INFO("AT_UARTUnsolicitedRI busy %u CHL:%u %s\r\n", AT_UARTRIIsBusy(), channelId, data);
        if(AT_UARTRIIsBusy())
        {
            return AT_URC_RI_DELAY;
        }
        else
        {
            if(APP_URC_RI_Start(channelId, data, dataLen, AT_UnsolicitedTimeOut) != osOK)
            {
                return AT_URC_RI_NORMAL;
            }
            else
            {
                AT_UARTRISetStatus(true);
                return AT_URC_RI_DELAY;
            }
        }
    }
    return 0;
}

/**
 * @brief   等待uart发送结束
 *
 */

void AT_UARTWaitTxComplete(void)
{
    UART_STATUS status;
    memset(&status, 0, sizeof(UART_STATUS));
    if (g_AT_UartHandle != NULL)
    {
        do
        {
            UART_GetStatus(g_AT_UartHandle, &status);
        } while (status.tx_busy != 0);
    }
}

void UARTReAdaptBaude(void)
{
  if (g_AT_UartHandle != NULL)
  {
    //波特率自适应必须复位控制器，不能为LP_MODE,工作时钟不能为32K
    UART_PowerControl(g_AT_UartHandle, DRV_POWER_OFF);
    UART_Uninitialize(g_AT_UartHandle);
    UART_Initialize(g_AT_UartHandle, AT_UART_Callback);
    UART_PowerControl(g_AT_UartHandle, DRV_POWER_FULL);
    UART_Control(g_AT_UartHandle, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 115200);
    UART_Control(g_AT_UartHandle, UART_CONTROL_AUTO_BAUD, 1);
    AT_OpenChannel(AT_CHANNEL_ID_UART);
  }
}

void AT_UARTSetBauderateNV(uint8_t channelId, uint8_t is_result,  void *user_data)
{

  uint32_t bauderate = (uint32_t )user_data;
  int32_t       result   = DRV_OK;

  if (g_AT_UartHandle != NULL)
  {
    AT_UARTWaitTxComplete();
    //波特率自适应必须复位控制器，不能为LP_MODE,工作时钟不能为32K
    UART_PowerControl(g_AT_UartHandle, DRV_POWER_OFF);
    UART_Uninitialize(g_AT_UartHandle);
    UART_Initialize(g_AT_UartHandle, AT_UART_Callback);
    UART_PowerControl(g_AT_UartHandle, DRV_POWER_FULL);
    g_AT_UartHandle->re_adaptbaud = UARTReAdaptBaude;

    if (bauderate == 0)
    {
      result = UART_Control(g_AT_UartHandle, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 115200);
      if(result == DRV_OK)
      {
        result = UART_Control(g_AT_UartHandle, UART_CONTROL_AUTO_BAUD, 1);
        if(result == DRV_OK)
        {
            if(amt_IsAmtMode() == FALSE)
            {
                PlatCfg_ATBaud(bauderate);
            }
        }
      }
    }
    else
    {
      result = UART_Control(g_AT_UartHandle, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, bauderate);
      if(result == DRV_OK)
      {
        if(amt_IsAmtMode() == FALSE)
        {
            PlatCfg_ATBaud(bauderate);
        }
      }
    }
    AT_OpenChannel(channelId);
  }
}

void AT_UARTSetBauderate(uint8_t channelId, uint8_t is_result,  void *user_data)
{

  uint32_t bauderate = (uint32_t )user_data;

  if (g_AT_UartHandle != NULL)
  {
    AT_UARTWaitTxComplete();
    //波特率自适应必须复位控制器，不能为LP_MODE,工作时钟不能为32K
    UART_PowerControl(g_AT_UartHandle, DRV_POWER_OFF);
    UART_Uninitialize(g_AT_UartHandle);
    UART_Initialize(g_AT_UartHandle, AT_UART_Callback);
    UART_PowerControl(g_AT_UartHandle, DRV_POWER_FULL);
    g_AT_UartHandle->re_adaptbaud = UARTReAdaptBaude;

    if (bauderate == 0)
    {
      UART_Control(g_AT_UartHandle, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 115200);
      UART_Control(g_AT_UartHandle, UART_CONTROL_AUTO_BAUD, 1);
    }
    else
    {
      UART_Control(g_AT_UartHandle, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, bauderate);
    }
    AT_OpenChannel(channelId);
  }
}
uint32_t  AT_UARTGetSupportBauderate(uint32_t ** bauderateList)
{

  UartSupportedBauderate bauderateSupport;
  if (g_AT_UartHandle != NULL)
  {
      UART_Control(g_AT_UartHandle, UART_GET_SUPPORTED_BAUDERATE_LIST, (uint32_t)&bauderateSupport);
  }
  *bauderateList = bauderateSupport.bauderateList;
  return bauderateSupport.bauderateNum;
}


osStatus_t AT_BauderateValid(uint32_t baudRate)
{

  UartSupportedBauderate bauderateSupport;
  bauderateSupport.bauderateNum = AT_UARTGetSupportBauderate(&(bauderateSupport.bauderateList));

  for (int i = 0; i < bauderateSupport.bauderateNum; i++)
  {
      if(baudRate == bauderateSupport.bauderateList[i])
      {
        return osOK;
      }
  }
  return osError;
}

uint32_t AT_UARTGetBauderate(void)
{
    uint32_t    bauderate = 0;

    if (g_AT_UartHandle != NULL)
    {
        UART_STATUS uartStatus = {0};
        UART_GetStatus(g_AT_UartHandle, &uartStatus);
        bauderate = uartStatus.setBauderate;
    }

    return bauderate;
}

int32_t AT_InitializeUartDevice(UART_Handle *huart, uint32_t bauderate)
{
    huart->capabilities.wakeup = 1;

    huart->func = UART_PORT_AT;
    UART_Initialize(huart, AT_UART_Callback);
    UART_PowerControl(huart, DRV_POWER_OFF);
    UART_PowerControl(huart, DRV_POWER_FULL);
    UART_Control(huart, UART_FLOW_CONTROL_RTS_CTS, 0);
    UART_Control(huart, UART_UART0_RESET, 0);
    g_AT_UartHandle->re_adaptbaud = UARTReAdaptBaude;

    if(bauderate != 0)
    {
        UART_Control(huart, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, bauderate);
    }
    else
    {
        UART_Control(huart, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 115200);
        UART_Control(huart, UART_CONTROL_AUTO_BAUD, 1);
    }

    return 0;
}

int32_t AT_InitializeUartChannel(void)
{
    AT_AllocChannel(AT_CHANNEL_ID_UART, AT_UARTReceive, AT_UARTSend, AT_UARTSend, AT_CHANNEL_ATTRS_TIMER);
    AT_ChannelSetURCIndicateCB(AT_CHANNEL_ID_UART, AT_UARTUnsolicitedRI);
    AT_OpenChannel(AT_CHANNEL_ID_UART);
    return 0;
}

int32_t AT_DeInitializeUartChannel(void)
{
    AT_CloseChannel(AT_CHANNEL_ID_UART);
    AT_FreeChannel(AT_CHANNEL_ID_UART);
    return 0;
}

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief           UART上实现AT命令收发
 *
 * @param[in]       none
 *
 * @return
 ************************************************************************************
*/
int APP_Uart_AT(void)
{
    uint32_t addr = 0;
    NV_UartCfg  uartConfig ={0};
    g_AT_UartHandle = Board_GetAtUartHandle();
    if(g_AT_UartHandle != NULL)
    {
        addr = NV_ITEM_ADDR(pubConfig.drvConfig.uartConfig);
        NVM_Read(addr, (uint8_t *)&uartConfig, sizeof(uartConfig));

        g_AT_UartHandle->adaptChClock = uartConfig.adaptChClock;
        g_AT_UartHandle->info.select_Clock = uartConfig.select_Clock;
        AT_InitializeUartDevice(g_AT_UartHandle, uartConfig.baudrate);
        AT_InitializeUartChannel();
    }
    return 0;
}


