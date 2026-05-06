/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_KPD.c
 *
 * @brief       KPD驱动实现.
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
#include <drv_soc.h>
#include <drv_kpd.h>

#define DEBUG_KPD
#ifdef DEBUG_KPD
#define kpd_log(fmt, ...) osPrintf(fmt, ##__VA_ARGS__)
#else
#define kpd_log(fmt, ...)
#endif
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
/*** Bit definition for [keyVersion] register(0x00000000) ****/
#define KEY_VERSION_Pos                   (16)
#define KEY_VERSION_Msk                   (0xFFFFUL << KEY_VERSION_Pos)
#define KEY_VERSION                       KEY_VERSION_Msk                   /*!<Version Register */


/*** Bit definition for [interval] register(0x00000004) ****/
#define INTERVAL_Pos                      (0)
#define INTERVAL_Msk                      (0xFFFFUL << INTERVAL_Pos)
#define INTERVAL                          INTERVAL_Msk                      /*!<This register plus a const produce the scan period */


/*** Bit definition for [debounce] register(0x00000008) ****/
#define DEBOUNCE_Pos                      (0)
#define DEBOUNCE_Msk                      (0xFFUL << DEBOUNCE_Pos)
#define DEBOUNCE                          DEBOUNCE_Msk                      /*!<This register together with register INTERVAL determain the debounce time(debounce time=scan period * DEBOUNCE) */


/*** Bit definition for [intEn] register(0x0000000c) ****/
#define INT_EN_Pos                        (0)
#define INT_EN_Msk                        (0x1UL << INT_EN_Pos)
#define INT_EN                            INT_EN_Msk                        /*!<Keypad interrupt enable */


/*** Bit definition for [cfgUpt] register(0x00000010) ****/
#define CFG_UPT_Pos                       (0)
#define CFG_UPT_Msk                       (0x1UL << CFG_UPT_Pos)
#define CFG_UPT                           CFG_UPT_Msk                       /*!<When write 1,the configuration updated */


/*** Bit definition for [keyIndex] register(0x00000020) ****/
#define KEY_COUNT_Pos                     (16)
#define KEY_COUNT_Msk                     (0x7FUL << KEY_COUNT_Pos)
#define KEY_COUNT                         KEY_COUNT_Msk                     /*!<Number of keys pressed */
#define KEY2_Pos                          (8)
#define KEY2_Msk                          (0x7FUL << KEY2_Pos)
#define KEY2                              KEY2_Msk                          /*!<Second key detected */
#define KEY1_Pos                          (0)
#define KEY1_Msk                          (0x7FUL << KEY1_Pos)
#define KEY1                              KEY1_Msk                          /*!<First key detected */


/*** Bit definition for [keySta1] register(0x00000024) ****/
#define KEY_STA1_Pos                      (0)
#define KEY_STA1_Msk                      (0xFFFFFFFFUL << KEY_STA1_Pos)
#define KEY_STA1                          KEY_STA1_Msk                      /*!<KEY_STATUS[31:0] */


/*** Bit definition for [keySta2] register(0x00000028) ****/
#define KEY_STA2_Pos                      (0)
#define KEY_STA2_Msk                      (0xFFFFFFFFUL << KEY_STA2_Pos)
#define KEY_STA2                          KEY_STA2_Msk                      /*!<KEY_STATUS[63:32] */


/*** Bit definition for [keySta3] register(0x0000002c) ****/
#define KEY_STA3_Pos                      (0)
#define KEY_STA3_Msk                      (0xFFUL << KEY_STA3_Pos)
#define KEY_STA3                          KEY_STA3_Msk                      /*!<KEY_STATUS[71:64] */


#define KPD_TASK_STACK_SIZE     (1024 * 2)
#define KPD_TASK_PRIORITY       (osPriorityBelowNormal1)
/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
/**
 * @brief  REG_Kpd
 */
typedef struct {
  __IO uint32_t  keyVersion;                       /*!< Offset 0x00000000 */
  __IO uint32_t  interval;                         /*!< Offset 0x00000004 */
  __IO uint32_t  debounce;                         /*!< Offset 0x00000008 */
  __IO uint32_t  intEn;                            /*!< Offset 0x0000000c */
  __IO uint32_t  cfgUpt;                           /*!< Offset 0x00000010 */
  __IO uint32_t  rsvd0[3];                         /*!< Offset 0x00000014 */
  __IO uint32_t  keyIndex;                         /*!< Offset 0x00000020 */
  __IO uint32_t  keySta1;                          /*!< Offset 0x00000024 */
  __IO uint32_t  keySta2;                          /*!< Offset 0x00000028 */
  __IO uint32_t  keySta3;                          /*!< Offset 0x0000002c */
} REG_Kpd;
/************************************************************************************
 *                                 变量定义
 ************************************************************************************/
static KPD_VIRTUAL_KEY   sHalKpd_Keymap[ KPD_ROW_NUM ][ KPD_COL_NUM ];
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief           添加按键事件至队列中
 *
 * @param[in]       KPD            KPD控制器句柄
 * @param[in]       pKeyEvent      按键事件
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
void KPD_KeyQueueAdd(kpd_key_item *pKeyEvent, KPD_Handle *KPD)
{

  kpd_key_queue *s_kpdKeyQueue = &KPD->info.key_queue;

  osMutexAcquire(&KPD->info.kpdQueueMutex, osWaitForever);
  /* if the key event queue is not full. */
  if (KPD_KEY_QUEUE_SIZE > s_kpdKeyQueue->count)
  {
    /* copy the new event to the end of queue */
    memcpy(&(s_kpdKeyQueue->items[ s_kpdKeyQueue->tail ]), pKeyEvent, sizeof(kpd_key_item));
    s_kpdKeyQueue->count++;
    s_kpdKeyQueue->tail = (s_kpdKeyQueue->tail + 1) % KPD_KEY_QUEUE_SIZE;
  }
  else
  {
    kpd_log("Key queue overflow.\n");
  }
  osMutexRelease(&KPD->info.kpdQueueMutex);
}

/**
 ************************************************************************************
 * @brief           上报按键按下
 *
 * @param[in]       KPD            KPD控制器句柄
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
static void KPD_ReportPressedKey(KPD_Handle *KPD)
{
  kpd_key_item temp;
  Kpd_MatrixStatus (*keyMatrix)[KPD_COL_NUM] = KPD->info.keyMatrix;
  uint32_t  i = 0, j = 0;
  temp.times = 0;
  temp.state = KEY_PRESSED;
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 9; j++)
    {
      if ((keyMatrix[ i ][ j ].current == KEY_PRESSED) && (keyMatrix[ i ][ j ].report == KEY_RELEASED))
      {
        temp.key                           = sHalKpd_Keymap[ i ][ j ];
        keyMatrix[ i ][ j ].report = KEY_PRESSED;
        temp.callback = keyMatrix[ i ][ j ].callback;
        temp.ctx = keyMatrix[ i ][ j ].ctx;
        KPD->info.row = i;
        KPD->info.col = j;
        KPD->info.key_value = temp.key;
        KPD->info.key_status = KEY_PRESSED;
        kpd_log("s_kpdMatrixStatus[ %d][ %d ] KEY_PRESSED ,temp.key  %d\r\n", i, j, temp.key);

        KPD_KeyQueueAdd(&temp, KPD);
      }
    }
  }
}

/**
 ************************************************************************************
 * @brief           上报按键释放
 *
 * @param[in]       KPD            KPD控制器句柄
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
static void KPD_ReportReleasedKey(KPD_Handle *KPD)
{
  kpd_key_item temp;
  Kpd_MatrixStatus (*keyMatrix)[KPD_COL_NUM] = KPD->info.keyMatrix;
  uint32_t  i = 0, j = 0;
  temp.times = 0;
  temp.state = KEY_RELEASED;
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 9; j++)
    {
      if ((keyMatrix[ i ][ j ].report == KEY_PRESSED) && (keyMatrix[ i ][ j ].current == KEY_RELEASED))
      {
        temp.key                           = sHalKpd_Keymap[ i ][ j ];
        keyMatrix[ i ][ j ].report = KEY_RELEASED;
        temp.callback = keyMatrix[ i ][ j ].callback;
        temp.ctx = keyMatrix[ i ][ j ].ctx;
        KPD->info.row = i;
        KPD->info.col = j;
        KPD->info.key_value = temp.key;
        KPD->info.key_status = KEY_RELEASED;
        kpd_log("s_kpdMatrixStatus[ %d][ %d ] KEY_RELEASED temp.key %d\r\n", i, j, temp.key);
        KPD_KeyQueueAdd(&temp, KPD);
      }
    }
  }
}

/**
 ************************************************************************************
 * @brief           检查按键状态
 *
 * @param[in]       KPD            KPD控制器句柄
 *
 * @return          uint32_t
 * @retval          uint32_t       按键按下的数量
 ************************************************************************************
*/
static uint32_t KPD_CheckKeyStatus(KPD_Handle *KPD)
{
  uint32_t         i, j;
  uint32_t         sta;
  uint8_t          scan;
  uint32_t         key_pressed_cnt = 0;
  REG_Kpd *regBase = (REG_Kpd *)KPD->pRes->regBase;
  //uint32_t key_sta1 = regBase->keySta1;
  Kpd_MatrixStatus (*keyMatrix)[KPD_COL_NUM] = KPD->info.keyMatrix;
  for (i = 0; i < KPD_COL_NUM; i++)
  {

    //[0-3]keySta1
    if(( i >= 0) && (i <= 3))
    {
      sta = regBase->keySta1;
    }
    //[4-7]keySta2
    else if((i>=4) && (i <= 7))
    {

      sta = regBase->keySta2;
    }
    //8列使用keySta3
    else
    {
      sta = regBase->keySta3;
    }

    scan = (sta >> (KPD_ROW_NUM * (i%4))) & 0xFF;

    for (j = 0; j < KPD_ROW_NUM; j++)
    {
      if (((scan >> j) & 1) == 1)
      {
        keyMatrix[ j ][ i ].current = KEY_PRESSED;
        // osPrintf("keyMatrix[ %d ][ %d ] pressed %d\r\n", j, i, keyMatrix[ j ][ i ].report);
        key_pressed_cnt++;
      }
      else
      {
        keyMatrix[ j ][ i ].current = KEY_RELEASED;
        // osPrintf("keyMatrix[ %d ][ %d ] released %d\r\n", j, i, keyMatrix[ j ][ i ].report);
      }
    }
  }
  return key_pressed_cnt;
}

/**
 ************************************************************************************
 * @brief          任务处理函数
 *
 * @param[in]       irq_id         中断号
 * @param[in]       irq_data       中断参数
 *
 * @return          void
 * @retval          null
 ************************************************************************************
 */
static void KPD_ScanThread(void *param)
{
  uint32_t      key_count = 0;
  uint32_t event = 0;
  KPD_Handle *KPD   = (KPD_Handle  *)param;
  kpd_key_item *item;
  //struct KPD_INFO *info = &(KPD->info);
  kpd_key_queue  *key_queue = &KPD->info.key_queue;
  kpd_log("enter KPD_ScanThread\r\n");
  while (OS_TRUE)
  {
    osSemaphoreAcquire(&(KPD->info.kpdSema), osWaitForever);

    key_count = KPD_CheckKeyStatus(KPD);
    kpd_log("key_count %d\r\n", key_count);
    //对按键进行判断
    KPD_ReportReleasedKey(KPD);
    KPD_ReportPressedKey(KPD);
    event |= KPD_EVENT_CHNAGED;
    KPD->info.cb_event(KPD,event);
    //处理回调
    osMutexAcquire(&KPD->info.kpdQueueMutex, osWaitForever);
    kpd_log("key_queue->count %d\r\n", key_queue->count);
    while(key_queue->count != 0)
    {
      item =  &(key_queue->items[ key_queue->head ]);

      if(item->callback != NULL)
      {

          item->callback(item->key, item->state, item->ctx);
      }

      key_queue->count--;
      key_queue->head = (key_queue->head + 1) % KPD_KEY_QUEUE_SIZE;
    }
    osMutexRelease(&KPD->info.kpdQueueMutex);

    osInterruptClrPending(OS_EXT_IRQ_TO_IRQ(KPD->pRes->intNum));
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(KPD->pRes->intNum));
  }
}

/**
 ************************************************************************************
 * @brief          中断处理函数
 *
 * @param[in]       irq_id         中断号
 * @param[in]       irq_data       中断参数
 *
 * @return          void
 * @retval          null
 ************************************************************************************
 */
static void KPD_isr(int irq_id, void *irq_data)
{

  // uint32_t event = 0;
  KPD_Handle *KPD = (KPD_Handle *)irq_data;

  kpd_log("key_isr\r\n");

  osInterruptMask(OS_EXT_IRQ_TO_IRQ(KPD->pRes->intNum));
  //释放互斥量
  osSemaphoreRelease(&KPD->info.kpdSema);

  // event |= KPD_EVENT_CHNAGED;
  // KPD->info.cb_event(KPD,event);
}

/**
 ************************************************************************************
 * @brief           初始化KPD控制器
 *
 * @param[in]       KPD            KPD控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_OK          成功
 ************************************************************************************
*/
int32_t KPD_Initialize(KPD_Handle *KPD, KPD_Callback cb_event)
{
  REG_Kpd *regBase;
  osThreadId_t kpdThread = NULL;
  osThreadAttr_t attr = {"keyTask", osThreadDetached, NULL, 0U, NULL, KPD_TASK_STACK_SIZE, KPD_TASK_PRIORITY, 0U, 0U};

  regBase = (REG_Kpd *)KPD->pRes->regBase;
  if (KPD->info.flags & KPD_FLAG_INITIALIZED)
  {
    // Driver is already initialized
    return DRV_OK;
  }
  KPD->info.cb_event = cb_event;

  for (int i = 0; i < KPD_ROW_NUM;i++)
  {
    for (int j = 0; j < KPD_COL_NUM; j++)
    {
      KPD->info.keyMatrix[i][j].current = KEY_PRESSED;
      KPD->info.keyMatrix[i][j].report = KEY_RELEASED;
      sHalKpd_Keymap[ i ][ j ] = i * KPD_ROW_NUM + j + 1;
    }
  }

  //创建信号量以便通知任务读取按键信息
  osSemaphoreInit(&KPD->info.kpdSema, 0, OS_SEM_VALUE_MAX, OS_IPC_FLAG_PRIO);

  //创建互斥量，保护队列
  osMutexInit(&KPD->info.kpdQueueMutex, OS_IPC_FLAG_PRIO);

  CLK_SetregGenLbAonCrmRegs(CLK_APB_KEY_PCLK_EN, 1);
  CLK_SetregGenLbAonCrmRegs(CLK_APB_KEY_WCLK_EN, 1);
  MODIFY_REG(regBase->interval, INTERVAL, 0);
  MODIFY_REG(regBase->debounce, DEBOUNCE, 10);
  SET_BIT(regBase->intEn, INT_EN);
  SET_BIT(regBase->cfgUpt, CFG_UPT);

  kpdThread = osThreadNew(KPD_ScanThread, KPD, &attr);
  OS_ASSERT(kpdThread != NULL);
  KPD->info.flags = KPD_FLAG_INITIALIZED;
  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           设置KPD控制器的供电
 *
 * @param[in]       KPD            KPD控制器句柄
 * @param[in]       state           设置控制的状态
 *
 * @return          int32_t
 * @retval          DRV_ERR_PARAMETER             错误的参数
 *                  DRV_ERR_UNSUPPORTED           不支持
 *                  DRV_ERR                       错误
 *                  DRV_OK                        成功
 ************************************************************************************
*/
int32_t KPD_PowerControl(KPD_Handle *KPD, DRV_POWER_STATE state)
{

  if ((state != DRV_POWER_OFF) &&
      (state != DRV_POWER_FULL) &&
      (state != DRV_POWER_LOW))
  {
    return DRV_ERR_PARAMETER;
  }

  switch (state)
  {
  case DRV_POWER_OFF:

    KPD->info.flags &= ~KPD_FLAG_POWERED;
    break;

  case DRV_POWER_LOW:
    return DRV_ERR_UNSUPPORTED;

  case DRV_POWER_FULL:
    if ((KPD->info.flags & KPD_FLAG_INITIALIZED) == 0U)
    {
      return DRV_ERR;
    }
    if ((KPD->info.flags & KPD_FLAG_POWERED) != 0U)
    {
      return DRV_OK;
    }
    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(KPD->pRes->intNum), 1, IRQ_HIGH_LEVEL);
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(KPD->pRes->intNum), KPD_isr, KPD);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(KPD->pRes->intNum));

    KPD->info.flags = KPD_FLAG_POWERED;
    break;
  }
  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           去初始化KPD控制器
 *
 * @param[in]       KPD            KPD控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_OK          成功
 ************************************************************************************
*/
int32_t KPD_Uninitialize(KPD_Handle *KPD)
{
    KPD->info.flags = 0;
    osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(KPD->pRes->intNum));
    return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           获取信息
 *
 * @param[in]       KPD            KPD控制器句柄
 * @param[in]       buffer         二维数组指针
 *@param[in]        size           长度
 * @return          void
 * @retval          void       按键按下的数量
 ************************************************************************************
*/
uint32_t KPD_KeyStatus(KPD_Handle *KPD, void *buffer, uint32_t size)
{
  kpd_key_queue  *key_queue = &KPD->info.key_queue;
  uint32_t length_read = 0;
  if(buffer == OS_NULL || size < sizeof(kpd_key_item))
  {
    return DRV_ERR_PARAMETER;
  }
  osMutexAcquire(&KPD->info.kpdQueueMutex, osWaitForever);
  if(key_queue->count != 0)
  {
    memcpy(buffer, &(key_queue->items[ key_queue->head ]), sizeof(kpd_key_item));
    key_queue->count--;
    key_queue->head = (key_queue->head + 1) % KPD_KEY_QUEUE_SIZE;
    length_read         = sizeof(kpd_key_item);
  }
  osMutexRelease(&KPD->info.kpdQueueMutex);

  return length_read;
}


/**
 ************************************************************************************
 * @brief           注册按键回调
 *
 * @param[in]       KPD            KPD控制器句柄
 * @param[in]       hook_func      回调函数
 * @param[in]       ctx            回调入参
 * @return          void
 * @retval          void
 ************************************************************************************
*/
void KPD_Listen(KPD_Handle *KPD, KPD_VIRTUAL_KEY id, kpdHook_t hook_func, void *ctx)
{
  if(id > KPD_VK_MAX)
  {
    OS_ASSERT(0);
  }
  uint8_t col = (id - 1) / KPD_ROW_NUM;
  uint8_t row = id % KPD_ROW_NUM - 1;

  if(KPD->info.keyMatrix[row][col].callback == NULL)
  {
    KPD->info.keyMatrix[row][col].callback = hook_func;
    KPD->info.keyMatrix[row][col].ctx = ctx;
  }

}