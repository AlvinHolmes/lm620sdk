/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_gpio.c
 *
 * @brief       GPIO驱动实现.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-04-21     ict team          创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <os.h>
#include <os_hw.h>
#include <drv_pin.h>
#if defined(OS_USING_PM)
#include <drv_pcu.h>
#endif
#include <drv_soc.h>
#include <drv_psm_sys.h>
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define GPIOPDD_REG0(pResource)              ((pResource->gpioPort) == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16) * 4)
#define EDGE_LEVEL_REG0(pResource)                (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 1) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 1) * 4)
#define HIGH_LOW_REG0(pResource)                  (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 2) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 2) * 4)
#define POS_EDGE_REG0(pResource)                  (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 3) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 3) * 4)
#define NEG_EDGE_REG0(pResource)                  (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 4) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 4) * 4)
#define RECV_REG0(pResource)                      (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 5) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 5) * 4)
#define SET1_SEND_REG0(pResource)                 (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 6) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 6) * 4)
#define SET0_SEND_REG0(pResource)                 (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 7) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 7) * 4)
#define SEND_REG0(pResource)                      (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 8) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 8) * 4)
#define INT_MASK_REG0(pResource)                  (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 10) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 10) * 4)
#define INT_ENABLE_REG0(pResource)                (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 11) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 11) * 4)
#define INT_STATUS_REG0(pResource)                (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 12) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 12) * 4)
#define INT_CLEAR_REG0(pResource)                 (pResource->gpioPort == GPIO_PORT_AON) ? ((uint32_t)g_GPIO_Res[GPIO_PORT_AON].regBase + (((pResource->gpioNum) >> 4) * 16 + 13) * 4) : \
                                                                                      ((uint32_t)g_GPIO_Res[GPIO_PORT_PD].regBase + ((pResource->gpioNum >> 4) * 16 + 13) * 4)


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

typedef struct _PIN_irq_hdr
{
    osList_t irq_node;
    uint8_t  pin;              ///< pin number
    uint8_t  group;
    int16_t mode;             ///< pin mode
    void (*hdr)(void *args);      ///< pin interrupt function
    void *args;                   ///< pin argument
} PIN_irq_hdr;

/************************************************************************************
 *                                 变量定义
 ************************************************************************************/
#ifdef _CPU_AP
static osList_t pin_irq_hdr_list = OS_LIST_INIT(pin_irq_hdr_list);
#endif
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief           设置GPIO方向
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       dir            方向
 *
 * @return          void
 * @retval          null
 ************************************************************************************
 */
void GPIO_SetDir(const struct PIN_Res *pResource, GPIO_DIR dir)
{
    uint32_t gpio_addr = GPIOPDD_REG0(pResource);

    WRITE_BIT_U32(gpio_addr, dir, 1, (pResource->gpioNum) % 16);
}

/**
 ************************************************************************************
 * @brief           获取GPIO方向
 *
 * @param[in]       pResource      PIN资源句柄
 *
 * @return          GPIO_DIR
 * @retval          GPIO_INPUT or GPIO_OUTPUT      方向
 ************************************************************************************
*/
GPIO_DIR GPIO_GetDir(const struct PIN_Res *pResource)
{
  GPIO_DIR dir;
  uint32_t gpio_addr = GPIOPDD_REG0(pResource);

  dir = (READ_U32(gpio_addr) >> ((pResource->gpioNum) % 16)) & 1;
  return dir;
}

/**
 ************************************************************************************
 * @brief           设置GPIO输出电平
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       level          电平
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void GPIO_Write(const struct PIN_Res *pResource, GPIO_LEVEL level)
{
  uint32_t gpio_addr = 0;
  if(level == GPIO_LOW)
  {
      gpio_addr = SET0_SEND_REG0(pResource);
  } else if(level == GPIO_HIGH)
  {
      gpio_addr = SET1_SEND_REG0(pResource);
  }
  WRITE_BIT_U32(gpio_addr, 1, 1, (pResource->gpioNum) % 16);
}

/**
 ************************************************************************************
 * @brief           获取GPIO电平
 *
 * @param[in]       pResource      PIN资源句柄
 *
 * @return          GPIO_LEVEL
 * @retval
 ************************************************************************************
*/
GPIO_LEVEL GPIO_Read(const struct PIN_Res *pResource)
{
    uint32_t gpio_addr = 0;
    uint32_t tmp;
    gpio_addr = GPIOPDD_REG0(pResource);
    tmp      = READ_U32(gpio_addr);
    if ((tmp >> (pResource->gpioNum % 16)) & 0x1)
    {
        gpio_addr = SEND_REG0(pResource);

    } else
    {
        gpio_addr = RECV_REG0(pResource);
    }
    tmp       = READ_U32(gpio_addr);

    if ((tmp >> (pResource->gpioNum % 16)) & 0x1)
        return GPIO_HIGH;
    else
        return GPIO_LOW;
}

#ifdef _CPU_AP
int32_t GPIO_ModifyIrqMode(const struct PIN_Res *pResource, uint32_t mode)
{
    ubase_t  level;
    osList_t *pos;
    int32_t   ret = DRV_OK;
    bool       exist = false;
    PIN_irq_hdr *iter_irq_hdr;
    level = osInterruptDisable();

    osListForEach(pos, &pin_irq_hdr_list)
    {
        iter_irq_hdr = osListEntry(pos, PIN_irq_hdr, irq_node);
        if (iter_irq_hdr->pin == pResource->gpioNum &&iter_irq_hdr->group == pResource->gpioPort)
        {
            iter_irq_hdr->mode = mode;
            osInterruptEnable(level);
            ret = DRV_OK;
            exist = true;
            break;
        }
    }

    if(!exist)
    {
        return DRV_ERR;
    }

    // set the interrupt polarity
    if (PIN_IRQ_MODE_HIGH_LEVEL == mode)
    {
        WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
        WRITE_BIT_U32(HIGH_LOW_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
    }
    else if (PIN_IRQ_MODE_RISING == mode)
    {

        WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
        WRITE_BIT_U32(POS_EDGE_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
        WRITE_BIT_U32(NEG_EDGE_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
    }
    else if (PIN_IRQ_MODE_LOW_LEVEL == mode)
    {

        WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
        WRITE_BIT_U32(HIGH_LOW_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
    }
    else if (PIN_IRQ_MODE_FALLING == mode)
    {
        WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
        WRITE_BIT_U32(POS_EDGE_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
        WRITE_BIT_U32(NEG_EDGE_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
    }
    else if (PIN_IRQ_MODE_RISING_FALLING == mode)
    {

        WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
        WRITE_BIT_U32(POS_EDGE_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
        WRITE_BIT_U32(NEG_EDGE_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
    }

    // clear any pre-existing interrupts
    WRITE_U32(INT_CLEAR_REG0(pResource), 1 << ((pResource->gpioNum) % 16));

    return ret;
}
/**
 ************************************************************************************
 * @brief           注册GPIO中断
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       mode           中断触发方式
 * @param[in]       hdr            中断处理函数
 * @param[in]       args           中断处理入参
 *
 * @return          GPIO_LEVEL
 * @retval
 ************************************************************************************
*/
int32_t GPIO_AttachIrq(const struct PIN_Res *pResource, uint32_t mode, void (*hdr)(void *args), void *args)
{

  ubase_t  level;
  osList_t *pos;
  int32_t   ret = DRV_OK;
  bool       exist = false;
  PIN_irq_hdr *iter_irq_hdr;
  PIN_irq_hdr *p_irq_hdr;
  level = osInterruptDisable();
  osListForEach(pos, &pin_irq_hdr_list)
  {
      iter_irq_hdr = osListEntry(pos, PIN_irq_hdr, irq_node);
      if (iter_irq_hdr->pin == pResource->gpioNum &&iter_irq_hdr->group == pResource->gpioPort && iter_irq_hdr->hdr == hdr\
                                                    && iter_irq_hdr->mode == mode && iter_irq_hdr->args == args)
      {
        osPrintf("%d pin has exist\r\n", pResource->gpioNum);
        osInterruptEnable(level);
        exist = true;
        ret = DRV_OK;
        break;
      }
      if(iter_irq_hdr->pin == pResource->gpioNum && iter_irq_hdr->group == pResource->gpioPort)
      {
        osPrintf("%d pin has registered\r\n", pResource->gpioNum);
        osInterruptEnable(level);
        exist = true;
        ret = DRV_ERR_BUSY;
        break;
      }
  }
  if(exist == OS_FALSE)
  {
    p_irq_hdr = (PIN_irq_hdr *)osMalloc(sizeof(PIN_irq_hdr));
    if(p_irq_hdr == OS_NULL)
    {
      ret = DRV_ERR;
    }
    else
    {
    //   memset(p_irq_hdr, 0xff, sizeof(PIN_irq_hdr));
      p_irq_hdr->pin = pResource->gpioNum;
      p_irq_hdr->group = pResource->gpioPort;
      p_irq_hdr->hdr  = hdr;
      p_irq_hdr->mode = mode;
      p_irq_hdr->args = args;
      osListInsertAfter(&pin_irq_hdr_list, &(p_irq_hdr->irq_node));
      ret = DRV_OK;
    }
  }
      osInterruptEnable(level);
  return ret;
}

/**
 ************************************************************************************
 * @brief           Detach GPIO中断
 *
 * @param[in]       pResource      PIN资源句柄

 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void GPIO_DetachIrq(const struct PIN_Res *pResource)
{
  ubase_t  level;
  osList_t *pos;
  PIN_irq_hdr *iter_irq_hdr;

  level = osInterruptDisable();
  osListForEach(pos, &pin_irq_hdr_list)
  {
      iter_irq_hdr = osListEntry(pos, PIN_irq_hdr, irq_node);
      if (iter_irq_hdr->pin == pResource->gpioNum && iter_irq_hdr->group == pResource->gpioPort)
      {
        osListRemove(&(iter_irq_hdr->irq_node));
        osFree(iter_irq_hdr);
        break;
      }
  }
  osInterruptEnable(level);
}

/**
 ************************************************************************************
 * @brief           使能GPIO中断
 *
 * @param[in]       pResource      PIN资源句柄
 * @param[in]       enable         使能
 *
 * @return          int32_t
 * @retval          OS_ENOSYS     未attach
 *                  OS_ERROR      错误
 *                  OS_EOK        成功
 ************************************************************************************
*/
int32_t GPIO_IrqEnable(const struct PIN_Res *pResource, uint32_t enable)
{

  ubase_t   level;
  uint16_t mode;
  osList_t *pos;
  PIN_irq_hdr *iter_irq_hdr;
  bool       exist = false;

  osListForEach(pos, &pin_irq_hdr_list)
  {
      iter_irq_hdr = osListEntry(pos, PIN_irq_hdr, irq_node);
      if (iter_irq_hdr->pin == pResource->gpioNum && iter_irq_hdr->group == pResource->gpioPort)
      {
        exist = OS_TRUE;
        break;
      }
  }

  if(exist == OS_FALSE)
  {
    return osErrorNoSys;
  }
  mode = iter_irq_hdr->mode;
  // set the pin as inputs
  GPIO_SetDir(pResource, GPIO_INPUT);

  // set the interrupt polarity
  if (PIN_IRQ_MODE_HIGH_LEVEL == mode)
  {
    WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(HIGH_LOW_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
  }
  else if (PIN_IRQ_MODE_RISING == mode)
  {

    WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(POS_EDGE_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(NEG_EDGE_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
  }
  else if (PIN_IRQ_MODE_LOW_LEVEL == mode)
  {

    WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(HIGH_LOW_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
  }
  else if (PIN_IRQ_MODE_FALLING == mode)
  {
    WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(POS_EDGE_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(NEG_EDGE_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
  }
  else if (PIN_IRQ_MODE_RISING_FALLING == mode)
  {

    WRITE_BIT_U32(EDGE_LEVEL_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(POS_EDGE_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(NEG_EDGE_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
  }

  // clear any pre-existing interrupts
  WRITE_U32(INT_CLEAR_REG0(pResource), 1 << ((pResource->gpioNum) % 16));
  if (enable == 1)
  {
    level = osInterruptDisable();
    WRITE_BIT_U32(INT_MASK_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(INT_ENABLE_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
    osInterruptEnable(level);
  }
  else if(enable == 0)
  {
    level = osInterruptDisable();
    WRITE_BIT_U32(INT_MASK_REG0(pResource), 1, 1, (pResource->gpioNum) % 16);
    WRITE_BIT_U32(INT_ENABLE_REG0(pResource), 0, 1, (pResource->gpioNum) % 16);
    osInterruptEnable(level);
  }
  else
  {
    return osError;
  }
  return osOK;
}



/**
 ***********************************************************************************************************************
 * @brief           GPIO中断总入口
 *
 * @param[in]       null
 *
 * @return         void
 ***********************************************************************************************************************
 */
static void pin_irq_entry(int irqid, void *arg)
{
    osList_t *pos;
    PIN_irq_hdr *iter_irq_hdr;
    uint32_t               status;
    struct PIN_Res         pin_res;

    osListForEach(pos, &pin_irq_hdr_list)
    {
        iter_irq_hdr = osListEntry(pos, PIN_irq_hdr, irq_node);
        pin_res.gpioNum= iter_irq_hdr->pin;
        pin_res.gpioPort= iter_irq_hdr->group;
        status = READ_U32(INT_STATUS_REG0((&pin_res)));
        if ((status >> ((pin_res.gpioNum) % 16)) & 1)
        {
          WRITE_U32(INT_CLEAR_REG0((&pin_res)), 1 << (pin_res.gpioNum % 16));
          (iter_irq_hdr->hdr)(iter_irq_hdr->args);
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           GPIO初始化
 *
 * @param[in]       null
 *
 * @return          int32_t
 * @retval          0              success
 ***********************************************************************************************************************
 */
int DrvGpio_Init()
{
    #define GPIO_GROUPS_NUM 2
    for (int i = 0; i < GPIO_GROUPS_NUM;i++)
    {
        // mask all IRQs
        WRITE_U32((g_GPIO_Res[1].regBase + 0x02c + i*64), 0);
        WRITE_U32((g_GPIO_Res[1].regBase + 0x028 + i*64) , 0xFFFF);
        WRITE_U32((g_GPIO_Res[1].regBase + 0x034 + + i*64), 0xFFFF);
        //配置PD CORE GPIO输出，低电平
        WRITE_U32((g_GPIO_Res[1].regBase + i*64), 0xFFFF);
        WRITE_U32((g_GPIO_Res[1].regBase + 0x01C + i*64), 0xFFFF);
    }

    //配置AON GPIO输出，低电平
    WRITE_U32((g_GPIO_Res[0].regBase), 0xFFFF);
    WRITE_U32((g_GPIO_Res[0].regBase + 0x01C), 0xFFFF);

    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[1].intNum), 1, IRQ_HIGH_LEVEL);
#if defined(OS_USING_PM)
    PCU_WakeupIrqRegister (OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[1].intNum),ICT_PCU_HIGH_LEVEL);
#endif
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[1].intNum), pin_irq_entry, OS_NULL);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[1].intNum));

    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[1].intNum + 1), 1, IRQ_HIGH_LEVEL);
#if defined(OS_USING_PM)
    PCU_WakeupIrqRegister (OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[1].intNum+1),ICT_PCU_HIGH_LEVEL);
#endif
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[1].intNum + 1), pin_irq_entry, OS_NULL);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[1].intNum + 1));

    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[0].intNum), 1, IRQ_HIGH_LEVEL);
#if defined(OS_USING_PM)
    PCU_WakeupIrqRegister (OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[0].intNum),ICT_PCU_HIGH_LEVEL);
#endif
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[0].intNum), pin_irq_entry, OS_NULL);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(g_GPIO_Res[0].intNum));

  return 0;
}
INIT_DEVICE_EXPORT(DrvGpio_Init, OS_INIT_SUBLEVEL_HIGH);
#endif

#ifdef OS_USING_PM
static int PD_GPIO_Suspend(void *param, PSM_Mode mode,uint32_t *save_addr)
{
  uint32_t *addr = (uint32_t *)save_addr;
  int i = 0;
  int j = 0;
  if (mode == PSM_DEEP_SLEEP)
  {
    for (i = 0; i < 2; i++)
    {
      for (j = 0; j < 14; j++)
      {
        *((uint32_t *)(addr) + i * 14 + j) = READ_U32(g_GPIO_Res[ 1 ].regBase + i * 14 * 4 + j * 4);
      }
    }
  }


  return i*j*4;
}

static int PD_GPIO_Resume(void *param, PSM_Mode mode,uint32_t *save_addr)
{
    uint32_t *addr = (uint32_t *)save_addr;
    int i = 0;
    int j = 0;
    if (mode == PSM_DEEP_SLEEP)
    {
		for(i = 0; i < 2; i++)
		{
      /*
      唤醒的时候，先恢io状态。再恢复其他配置。
      */
			j = 8;
			WRITE_U32(g_GPIO_Res[1].regBase + i *14*4 + j *4, *((uint32_t *)(addr) + i*14 + j) );
		}

        for (i = 0; i < 2; i++)
        {
          for (j = 0; j < 14; j++)
          {
            /*
              1. 只读属性的寄存器，不恢复
              2. 只写的寄存器，不恢复
            **/
            if((j != 5) && (j!= 6) && (j != 7) && (j!= 9) && (j != 12))
            {
              WRITE_U32(g_GPIO_Res[1].regBase + i *14*4 + j *4, *((uint32_t *)(addr) + i*14 + j) );
            }
          }
        }

    }

    return i*j*4;
}


static PSM_DpmOps pd_gpio_dpmops = {
    .PsmSuspendNoirq = PD_GPIO_Suspend,
    .PsmResumeNoirq = PD_GPIO_Resume,
};

PSM_CMNDPM_INFO_DEFINE(gpio, pd_gpio_dpmops, OS_NULL, PSM_CMNDEV_GPIO);
#endif