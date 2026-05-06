/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file main.c
*
* @brief  main函数入口文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/

#include <stdlib.h>
#include <os.h>
#include <drv_pin.h>
#include "nr_micro_shell.h"

/**
 ************************************************************************************
 * @brief           TEST_GPIO.
 *
 * @param[in]
 *
 * @return          测试返回值.
 * @retval          ==0              测试成功
 *                  < 0              失败
 ************************************************************************************
 */
void TEST_GPIO(char argc, char **argv)
{
  GPIO_LEVEL level = 0;
  /*PIN复用*/
  PIN_SetMux(PIN_RES(PIN_29), PIN_29_MUX_GPIO);
  PIN_SetMux(PIN_RES(PIN_30), PIN_30_MUX_GPIO);

  GPIO_SetDir(PIN_RES(PIN_29), GPIO_OUTPUT);
  GPIO_SetDir(PIN_RES(PIN_30), GPIO_OUTPUT);
  GPIO_Write(PIN_RES(PIN_29), GPIO_LOW);

  level = GPIO_Read(PIN_RES(PIN_29));
  if(level != GPIO_LOW)
  {
    osPrintf("output LOW fail\r\n");
    return;
  }
  osDelay(100);
  level = GPIO_Read(PIN_RES(PIN_30));
  if(level != GPIO_LOW)
  {
    osPrintf("detect LOW fail\r\n");
    return;
  }


 //outpu HIGH
  GPIO_Write(PIN_RES(PIN_29), GPIO_HIGH);
  level = GPIO_Read(PIN_RES(PIN_29));
  if(level != GPIO_HIGH)
  {
    osPrintf("output HIGH fail\r\n");
    return;
  }
  osDelay(100);
  GPIO_Write(PIN_RES(PIN_30), GPIO_HIGH);
  level = GPIO_Read(PIN_RES(PIN_30));
  if(level != GPIO_HIGH)
  {
    osPrintf("detect HIGH fail\r\n");
    return;
  }

  osPrintf("test_gpio pass\r\n");
}
NR_SHELL_CMD_EXPORT(test_gpio, TEST_GPIO);

/**
 ************************************************************************************
 * @brief           gpio_cb中断处理函数.
 *
 * @param[in]
 *
 * @return          测试返回值.
 * @retval          ==0              测试成功
 *                  < 0              失败
 ************************************************************************************
 */
static void gpio_cb_edge(void *arg)
{
    osPrintf("TEST_GPIO_INT pass\r\n");
	  GPIO_IrqEnable(PIN_RES(PIN_30), 0);

}

/**
 ************************************************************************************
 * @brief           TEST_GPIO_RINT.
 *                  测试上升沿触发中断
 * @param[in]
 *
 * @return          void
 * @retval
 ************************************************************************************
 */
void TEST_GPIO_RINT(char argc, char **argv)
{
  /*PIN复用*/
  PIN_SetMux(PIN_RES(PIN_29), PIN_29_MUX_GPIO);
  PIN_SetMux(PIN_RES(PIN_30), PIN_30_MUX_GPIO);

  GPIO_SetDir(PIN_RES(PIN_29), GPIO_OUTPUT);
  GPIO_Write(PIN_RES(PIN_29), GPIO_LOW);
  GPIO_Write(PIN_RES(PIN_30), GPIO_HIGH);
  

  GPIO_AttachIrq(PIN_RES(PIN_30), PIN_IRQ_MODE_HIGH_LEVEL, gpio_cb_edge, (void *)PIN_RES(PIN_30));
 
  GPIO_IrqEnable(PIN_RES(PIN_30), 1);

  GPIO_Write(PIN_RES(PIN_29), GPIO_HIGH);
  osDelay(100);

  GPIO_IrqEnable(PIN_RES(PIN_30), 0);
  GPIO_DetachIrq(PIN_RES(PIN_30));
}
NR_SHELL_CMD_EXPORT(test_gpioRInt, TEST_GPIO_RINT);


//增加外部中断使用示例
/*需要注意的是，外部中断的使用支持电平触发和边沿触发。但是不支持双边沿触发。
  PCU_WakeupIrqRegister函数是配置唤醒中断的，如果不需要使用该功能， 只需要配置osInterruptConfig即可
  当使用PCU的时候，osInterruptConfig最后一个参数，配置触发方式只需要配置IRQ_HIGH_LEVEL，不接使用其他方式*/
static void AON_IrqIsr(int irq_id, void* irq_data)
{
  #if defined(OS_USING_PM)
    osPrintf("IrqIsr\r\n");
    PCU_WakeupIrqClrpending(OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_13));
  #endif
}

void TEST_AON0_INT(char argc, char **argv)
{
    PIN_SetMux(PIN_RES(PIN_9), PIN_9_MUX_AON_INT_0);

    #if defined(OS_USING_PM)
    PCU_WakeupIrqRegister(OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_13), ICT_PCU_NEGATIVE_EDGE);
    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_13), 1, IRQ_HIGH_LEVEL);
#else
    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_13), 1, IRQ_HIGH_LEVEL);
#endif
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_13), AON_IrqIsr, NULL);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(AP_INT_NUM_13));
}
NR_SHELL_CMD_EXPORT(aon, TEST_AON0_INT);