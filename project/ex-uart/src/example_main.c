/**
 *************************************************************************************
 * 版权所有 (C) 2024, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file		ex-uart
 *
 * @brief		uart示例代码
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2024-10-22     ict team          创建
  ************************************************************************************
  */

#if defined(_CPU_AP)
/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <drv_uart.h>
#include <os.h>
#include <drv_pin.h>
#include "nr_micro_shell.h"


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
UART_Handle g_uartHandle_2;
UART_Handle g_uartHandle_1;
UART_Handle g_uartHandle_0;
osCompletion uartCmpl_tx;
osCompletion uartCmpl_rx;

static ALIGN(32) char           data_save[ 2048 ];

ALIGN(OS_CACHE_LINE_SZ) uint8_t data_send[OS_CACHE_LINE_SZ] = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x12,0x13,0x14};
ALIGN(OS_CACHE_LINE_SZ) uint8_t data_recv[OS_CACHE_LINE_SZ] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/


/**
 ************************************************************************************
 * @brief           TEST_UART_Callback.
 *
 * @param[in]        hUart
 * @param[in]        event
 *
 * @return          void.
 * @retval
 *
 ************************************************************************************
 */

static void TEST_UART_Callback(void *hUart, uint32_t event)
{

    if(((UART_Handle *)hUart)->userData)
    {
        osComplete(((UART_Handle *)hUart)->userData);
    }

    if(((event & UART_EVENT_SEND_COMPLETE) !=0) && (((UART_Handle *)hUart)->userData == OS_NULL))
    {
      osPrintf("UART_EVENT_SEND_COMPLETE\r\n");
      osComplete(&uartCmpl_tx);
    }

    if(((event & UART_EVENT_RECEIVE_COMPLETE) !=0)  &&  (((UART_Handle *)hUart)->userData == OS_NULL))
    {
      osPrintf("UART_EVENT_RECEIVE_COMPLETE\r\n");
      osComplete(&uartCmpl_rx);
    }
}
/**
 ************************************************************************************
 * @brief           TEST_UART_0_FIFO.
 *
 * @param[in]       argc
 * @param[in]       argv
 *
 * @return          测试返回值.
 * @retval
 ************************************************************************************
 */

void  TEST_UART_0_FIFO(char argc, char **argv)
{
    osCompletion uartCmpl;
    UART_Handle *huart = &g_uartHandle_0;
    memset(&g_uartHandle_0, 0x0, sizeof(g_uartHandle_0));

	huart->pRes = (void *)DRV_RES(UART, 0);
  huart->func = UART_PORT_CONSOLE;


	/*设置引脚复用*/
	PIN_SetMux(PIN_RES(PIN_5), PIN_5_MUX_LPUART0_RX);
	PIN_SetMux(PIN_RES(PIN_6), PIN_6_MUX_LPUART0_TX);

    UART_Initialize(huart, TEST_UART_Callback);

    /*回调功能初始化*/
    osInitCompletion(&uartCmpl);
    huart->userData = &uartCmpl;
    UART_PowerControl(huart, DRV_POWER_FULL);
    UART_Control(huart, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 921600);

    UART_Send(huart, data_send, sizeof(data_send));
    if (osWaitForCompletion(huart->userData, osWaitForever))
    {
    }
    for (int i = 0; i < UART_GetTxCount(huart);i++)
    {
		osPrintf("send[%d] 0x%x\r\n", i,data_send[ i ]);
    }

    UART_Receive(huart, data_recv, sizeof(data_recv));
    if (osWaitForCompletion(huart->userData, osWaitForever))
    {
    }
    for (int i = 0; i < UART_GetRxCount(huart);i++)
    {
		osPrintf("recv[%d] 0x%x\r\n", i,data_recv[ i ]);
    }
	osDelay(100);
    UART_PowerControl(huart, DRV_POWER_OFF);
}
NR_SHELL_CMD_EXPORT(testUartFIFO, TEST_UART_0_FIFO);

/**
 ************************************************************************************
 * @brief           TEST_UART_0.
 *
 * @param[in]
 *
 * @return          测试返回值.
 * @retval          ==0              测试成功
 *                  < 0              失败
 ************************************************************************************
 */

void TEST_UART_0_DMA(char argc, char **argv)
{
    osCompletion uartCmpl;
    UART_Handle *huart = &g_uartHandle_0;
    memset(&g_uartHandle_0, 0x0, sizeof(g_uartHandle_0));

    /*修改非HDLC,该寄存器的作用是配置HDLC搬运，仅仅用于LOG使用。用户不需要关心*/
    //WRITE_U32(0xf2801090, 0);

    huart->pRes = (void *)DRV_RES(UART, 0);
    huart->func = UART_PORT_CONSOLE;

    huart->capabilities.rxDMAEnable = 1;
    huart->capabilities.txDMAEnable = 1;
    huart->capabilities.useForHDLC =  0;

    UART_Initialize(huart, TEST_UART_Callback);

    /*回调功能初始化*/
    osInitCompletion(&uartCmpl);
    huart->userData = &uartCmpl;

    UART_PowerControl(huart, DRV_POWER_FULL);
    UART_Control(huart, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 921600);

    osPrintf("PC open uart1 to input string, then will display in console\r\n");
    UART_Receive(huart, data_save, sizeof(data_save));
    if (osWaitForCompletion(huart->userData, osWaitForever))
    {

    }
    for (int i = 0; i < UART_GetRxCount(huart);i++)
    {
    	osPrintf("ch save[%d] 0x%x\r\n", i,data_save[ i ]);
    }
    UART_Send(huart, data_save, UART_GetRxCount(huart));
    if (osWaitForCompletion(huart->userData, osWaitForever))
    {

    }
	    for (int i = 0; i < UART_GetTxCount(huart);i++)
    {
		osPrintf("ch send[%d] 0x%x\r\n", i,data_save[ i ]);
    }
    memset(data_save, 0x0, sizeof(data_save));
    osDelay(100);
    UART_PowerControl(huart, DRV_POWER_OFF);
}
NR_SHELL_CMD_EXPORT(testUartDMA, TEST_UART_0_DMA);

/*增加ringbuffer模式示例代码
  需要注意的是，使用Ringbuffer功能，对应的接收函数为UART_Receive_Copy。使用的时候应当配置uart->func == UART_PORT_USER.
  UART_Receive_Copy函数第三个参数需要传递应用buf的大小。
*/
UART_Handle g_uartHandle_2;
osCompletion uartCmpl_rx;
osCompletion uartCmpl_tx;
static void TEST_UART_Callback(void *hUart, uint32_t event)
{

    if(((UART_Handle *)hUart)->userData)
    {
        osComplete(((UART_Handle *)hUart)->userData);
    }
    if(((event & UART_EVENT_SEND_COMPLETE) !=0) && (((UART_Handle *)hUart)->userData == OS_NULL))
    {
    //   osPrintf("UART_EVENT_SEND_COMPLETE\r\n");
      osComplete(&uartCmpl_tx);
    }

    if(((event & UART_EVENT_RECEIVE_COMPLETE) !=0))
    {
      osPrintf("UART_EVENT_RECEIVE_COMPLETE 0x%x\r\n", event);
      osComplete(&uartCmpl_rx);
    }

}

void TEST_UART_RINFBUFFER(char argc, char **argv)
{
    UART_Handle *huart = &g_uartHandle_2;
    memset(&g_uartHandle_2, 0x0, sizeof(g_uartHandle_2));

    huart->pRes = (void *)DRV_RES(UART, 2);
    huart->func = UART_PORT_USER;
    PIN_SetMux(PIN_RES(PIN_27), PIN_27_MUX_UART2_RX);
    PIN_SetMux(PIN_RES(PIN_28), PIN_28_MUX_UART2_TX);

    UART_Initialize(huart, TEST_UART_Callback);

    osInitCompletion(&uartCmpl_rx);
    huart->userData = OS_NULL;//&uartCmpl_rx;
    UART_PowerControl(huart, DRV_POWER_FULL);
    UART_Control(huart, UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1, 115200);
    osPrintf("PC open uart1 and then run python script\r\n");

    osPrintf("等待不定长数据...\r\n");
    
    uint8_t data_buffer[1024];
    uint32_t i = 0;

    while(1)
    {
        uint32_t read_cnt = 0;
        osWaitForCompletion(&uartCmpl_rx, osWaitForever); 
            read_cnt = UART_Receive_Copy(huart, data_buffer, sizeof(data_buffer));     
            osPrintf("收到 %u 字节 包数 %d\r\n", read_cnt, i);
            
      
            // 处理完整数据包
            for(uint32_t i = 0; i < read_cnt; i++) {
                // if (i % 16 == 0) osPrintf("\r\n");
                osPrintf("%02X ", data_buffer[i]);
            }
            osPrintf("\r\n");
       i++;
    }
  UART_PowerControl(huart, DRV_POWER_OFF);
}
NR_SHELL_CMD_EXPORT(uart, TEST_UART_RINFBUFFER);

#endif