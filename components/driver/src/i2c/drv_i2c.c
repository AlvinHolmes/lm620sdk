/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_i2c.h
 *
 * @brief       I2C驱动接口.
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
#include <drv_i2c.h>
#include <drv_soc.h>
#if defined(OS_USING_PM)  
#include <drv_pcu.h>
#include <drv_psm_sys.h>
#include "psm_sys.h"
#endif
/************************************************************************************
 *                                 配置开关
 ************************************************************************************/
/**
 * @brief 是否打开调试
 */
//#define I2C_DEBUG

#ifdef I2C_DEBUG
#define I2C_PrintDebug(fmt, ...) osPrintf(fmt, ##__VA_ARGS__)
#define I2C_PrintError(fmt, ...) osPrintf(fmt, ##__VA_ARGS__)
#else
#define I2C_PrintDebug(fmt, ...)
#define I2C_PrintError(fmt, ...)
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define I2C_FIFO_DEPTH (32)

/* I2C Driver state flags */
#define I2C_FLAG_INIT       (1 << 0)        // Driver initialized
#define I2C_FLAG_POWER      (1 << 1)        // Driver power on
#define I2C_FLAG_SETUP      (1 << 2)        // Master configured, clock set
#define I2C_FLAG_SLAVE_RX   (1 << 3)        // Slave receive registered

/*** Bit definition for [version] register(0x00000000) ****/
#define I2C_VER_H_Pos (24)
#define I2C_VER_H_Msk (0xFFUL << I2C_VER_H_Pos)
#define I2C_VER_H     I2C_VER_H_Msk /*!<High 8bits of hardware of I2C Module */
#define I2C_VER_L_Pos (16)
#define I2C_VER_L_Msk (0xFFUL << I2C_VER_L_Pos)
#define I2C_VER_L     I2C_VER_L_Msk /*!<Low 8bits of hardware of I2C Module */

/*** Bit definition for [cmd] register(0x00000004) ****/
#define I2C_IE_SLV_STOP_Pos  (12)
#define I2C_IE_SLV_STOP_Msk  (0x1UL << I2C_IE_SLV_STOP_Pos)
#define I2C_IE_SLV_STOP      I2C_IE_SLV_STOP_Msk /*!<slave模式下停止检测中断使能 */
#define I2C_IE_RXF_FULL_Pos  (11)
#define I2C_IE_RXF_FULL_Msk  (0x1UL << I2C_IE_RXF_FULL_Pos)
#define I2C_IE_RXF_FULL      I2C_IE_RXF_FULL_Msk /*!<接收FIFO满中断使能 */
#define I2C_IE_TXF_EMPTY_Pos (10)
#define I2C_IE_TXF_EMPTY_Msk (0x1UL << I2C_IE_TXF_EMPTY_Pos)
#define I2C_IE_TXF_EMPTY     I2C_IE_TXF_EMPTY_Msk /*!<发送FIFO空中断使能 */
#define I2C_IE_TIME_OUT_Pos  (9)
#define I2C_IE_TIME_OUT_Msk  (0x1UL << I2C_IE_TIME_OUT_Pos)
#define I2C_IE_TIME_OUT      I2C_IE_TIME_OUT_Msk /*!<master模式下等待从机响应超时使能 */
#define I2C_IE_MST_STOP_Pos  (8)
#define I2C_IE_MST_STOP_Msk  (0x1UL << I2C_IE_MST_STOP_Pos)
#define I2C_IE_MST_STOP      I2C_IE_MST_STOP_Msk /*!<master模式下传输结束中断使能 */
#define I2C_START_Pos        (6)
#define I2C_START_Msk        (0x1UL << I2C_START_Pos)
#define I2C_START            I2C_START_Msk /*!<启动传输 */
#define I2C_CMB_RW_Pos       (5)
#define I2C_CMB_RW_Msk       (0x1UL << I2C_CMB_RW_Pos)
#define I2C_CMB_RW           I2C_CMB_RW_Msk /*!<简单模式/组合模式 */
#define I2C_RW_Pos           (4)
#define I2C_RW_Msk           (0x1UL << I2C_RW_Pos)
#define I2C_RW               I2C_RW_Msk /*!<读写模式 */
#define I2C_ADDR_MODE_Pos    (1)
#define I2C_ADDR_MODE_Msk    (0x1UL << I2C_ADDR_MODE_Pos)
#define I2C_ADDR_MODE        I2C_ADDR_MODE_Msk /*!<地址模式 */
#define I2C_MS_SEL_Pos       (0)
#define I2C_MS_SEL_Msk       (0x1UL << I2C_MS_SEL_Pos)
#define I2C_MS_SEL           I2C_MS_SEL_Msk /*!<master/slave模式 */

/*** Bit definition for [deviceAddr] register(0x0000000c) ****/
#define I2C_DEVICE_ADDR_Pos (0)
#define I2C_DEVICE_ADDR_Msk (0x3FFUL << I2C_DEVICE_ADDR_Pos)
#define I2C_DEVICE_ADDR     I2C_DEVICE_ADDR_Msk /*!<设备地址 */

/*** Bit definition for [clkDiv] register(0x00000014) ****/
#define I2C_CLK_DIV_Pos (0)
#define I2C_CLK_DIV_Msk (0xFFUL << I2C_CLK_DIV_Pos)
#define I2C_CLK_DIV     I2C_CLK_DIV_Msk /*!<时钟分频因子 */

/*** Bit definition for [txCtrl] register(0x0000001c) ****/
#define I2C_TX_FIFO_RST_Pos (16)
#define I2C_TX_FIFO_RST_Msk (0x1UL << I2C_TX_FIFO_RST_Pos)
#define I2C_TX_FIFO_RST     I2C_TX_FIFO_RST_Msk /*!<复位读写指针和TX FIFO内容 */
#define I2C_TXD_QTY_Pos     (0)
#define I2C_TXD_QTY_Msk     (0xFFFFUL << I2C_TXD_QTY_Pos)
#define I2C_TXD_QTY         I2C_TXD_QTY_Msk /*!<传输数据量 */

/*** Bit definition for [rxCtrl] register(0x00000020) ****/
#define I2C_RX_FIFO_RST_Pos (16)
#define I2C_RX_FIFO_RST_Msk (0x1UL << I2C_RX_FIFO_RST_Pos)
#define I2C_RX_FIFO_RST     I2C_RX_FIFO_RST_Msk /*!<复位读写指针和RX FIFO内容 */
#define I2C_RXD_QTY_Pos     (0)
#define I2C_RXD_QTY_Msk     (0xFFFFUL << I2C_RXD_QTY_Pos)
#define I2C_RXD_QTY         I2C_RXD_QTY_Msk /*!<接收数据量 */

/*** Bit definition for [data] register(0x00000024) ****/
#define I2C_DATA_Pos (0)
#define I2C_DATA_Msk (0xFFUL << I2C_DATA_Pos)
#define I2C_DATA     I2C_DATA_Msk /*!<读写FIFO */

/*** Bit definition for [status] register(0x00000028) ****/
#define I2C_BUSY_Pos           (7)
#define I2C_BUSY_Msk           (0x1UL << I2C_BUSY_Pos)
#define I2C_BUSY               I2C_BUSY_Msk /*!<BUSY */
#define I2C_IRQ_SLV_STOP_Pos   (6)
#define I2C_IRQ_SLV_STOP_Msk   (0x1UL << I2C_IRQ_SLV_STOP_Pos)
#define I2C_IRQ_SLV_STOP       I2C_IRQ_SLV_STOP_Msk /*!<slave模式下停止检测中断 */
#define I2C_IRQ_RXF_FULL_Pos   (5)
#define I2C_IRQ_RXF_FULL_Msk   (0x1UL << I2C_IRQ_RXF_FULL_Pos)
#define I2C_IRQ_RXF_FULL       I2C_IRQ_RXF_FULL_Msk /*!<RX FIFO满中断 */
#define I2C_IRQ_TXF_EMPTY_Pos  (4)
#define I2C_IRQ_TXF_EMPTY_Msk  (0x1UL << I2C_IRQ_TXF_EMPTY_Pos)
#define I2C_IRQ_TXF_EMPTY      I2C_IRQ_TXF_EMPTY_Msk /*!<TX FIFO空中断 */
#define I2C_IRQ_TIME_OUT_Pos   (3)
#define I2C_IRQ_TIME_OUT_Msk   (0x1UL << I2C_IRQ_TIME_OUT_Pos)
#define I2C_IRQ_TIME_OUT       I2C_IRQ_TIME_OUT_Msk /*!<等待状态超时中断 */
#define I2C_STA_ERR_DATA_Pos   (2)
#define I2C_STA_ERR_DATA_Msk   (0x1UL << I2C_STA_ERR_DATA_Pos)
#define I2C_STA_ERR_DATA       I2C_STA_ERR_DATA_Msk /*!<slave没有响应 */
#define I2C_STA_ERR_DEVICE_Pos (1)
#define I2C_STA_ERR_DEVICE_Msk (0x1UL << I2C_STA_ERR_DEVICE_Pos)
#define I2C_STA_ERR_DEVICE     I2C_STA_ERR_DEVICE_Msk /*!<没有找到地址对应的设备 */
#define I2C_IRQ_MST_STOP_Pos   (0)
#define I2C_IRQ_MST_STOP_Msk   (0x1UL << I2C_IRQ_MST_STOP_Pos)
#define I2C_IRQ_MST_STOP       I2C_IRQ_MST_STOP_Msk /*!<master模式下传输结束中断 */

/*** Bit definition for [txfStatus] register(0x0000002c) ****/
#define I2C_TX_FIFO_CNTR_Pos  (2)
#define I2C_TX_FIFO_CNTR_Msk  (0x3FUL << I2C_TX_FIFO_CNTR_Pos)
#define I2C_TX_FIFO_CNTR      I2C_TX_FIFO_CNTR_Msk /*!<当前TX FIFO中数据数量 */
#define I2C_TX_FIFO_EMPTY_Pos (1)
#define I2C_TX_FIFO_EMPTY_Msk (0x1UL << I2C_TX_FIFO_EMPTY_Pos)
#define I2C_TX_FIFO_EMPTY     I2C_TX_FIFO_EMPTY_Msk /*!<TX FIFO空 */
#define I2C_TX_FIFO_FULL_Pos  (0)
#define I2C_TX_FIFO_FULL_Msk  (0x1UL << I2C_TX_FIFO_FULL_Pos)
#define I2C_TX_FIFO_FULL      I2C_TX_FIFO_FULL_Msk /*!<TX FIFO满 */

/*** Bit definition for [rxfStatus] register(0x00000030) ****/
#define I2C_RX_FIFO_CNTR_Pos  (2)
#define I2C_RX_FIFO_CNTR_Msk  (0x3FUL << I2C_RX_FIFO_CNTR_Pos)
#define I2C_RX_FIFO_CNTR      I2C_RX_FIFO_CNTR_Msk /*!<当前RX FIFO中数据数量 */
#define I2C_RX_FIFO_EMPTY_Pos (1)
#define I2C_RX_FIFO_EMPTY_Msk (0x1UL << I2C_RX_FIFO_EMPTY_Pos)
#define I2C_RX_FIFO_EMPTY     I2C_RX_FIFO_EMPTY_Msk /*!<RX FIFO空 */
#define I2C_RX_FIFO_FULL_Pos  (0)
#define I2C_RX_FIFO_FULL_Msk  (0x1UL << I2C_RX_FIFO_FULL_Pos)
#define I2C_RX_FIFO_FULL      I2C_RX_FIFO_FULL_Msk /*!<RX FIFO满 */


#define I2C_MIN(a, b) ((a) < (b) ? (a) : (b))

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief           写FIFO
 *
 * @param[in]       reg            I2C控制器地址
 * @param[in]       buf            数据地址
 * @param[in]       num            数据数量
 *
 * @return          int32_t
 * @retval          DRV_OK            成功
 *                  DRV_ERR_PARAMETER 写FIFO失败   
 * 
 ************************************************************************************
*/
int32_t I2C_WriteFifo(I2C_TypeDef *reg, uint8_t *buf, uint32_t num)
{
  if (num == 0)
  {
    return DRV_ERR_PARAMETER;
  }

  do
  {
    WRITE_REG(reg->data, *buf++);
  } while (--num != 0);

  return DRV_OK;
}
/**
 ************************************************************************************
 * @brief           读FIFO
 *
 * @param[in]       reg            I2C控制器地址
 * @param[in]       buf            数据地址
 * @param[in]       num            数据数量
 *
 * @return          int32_t
 * @retval          DRV_OK            成功
 *                  DRV_ERR_PARAMETER 读FIFO失败   
 * 
 ************************************************************************************
*/
static int32_t I2C_ReadFifo(I2C_TypeDef *reg, uint8_t *buf, uint32_t num)
{
  if (!buf || !num)
  {
    I2C_PrintError("I2C Buffer is NULL\r\n");
    return DRV_ERR;
  }

  if (!READ_BIT(reg->rxfStatus, I2C_RX_FIFO_CNTR))
  {
    I2C_PrintError("I2C Fifo is NULL\r\n");
    return DRV_ERR;
  }

  do
  {
    *buf++ = READ_REG(reg->data);
  } while (--num);

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           获取发送FIFO剩余空间
 *
 * @param[in]       reg            I2C控制器地址
 *
 * @return          uint8_t
 * @retval          >=0            剩余空间
 * 
 ************************************************************************************
*/
static inline uint8_t I2C_TxFifoVacancyGet(I2C_TypeDef *reg)
{
  return (I2C_FIFO_DEPTH - (READ_BIT(reg->txfStatus, I2C_TX_FIFO_CNTR) >> I2C_TX_FIFO_CNTR_Pos));
}

/**
 ************************************************************************************
 * @brief           获取接收FIFO剩余空间
 *
 * @param[in]       reg            I2C控制器地址
 *
 * @return          uint8_t
 * @retval          >=0            剩余空间
 * 
 ************************************************************************************
*/
static inline uint8_t I2C_RxFifoVacancyGet(I2C_TypeDef *reg)
{

  return (READ_BIT(reg->rxfStatus, I2C_RX_FIFO_CNTR) >> I2C_RX_FIFO_CNTR_Pos);
}

/**
 ************************************************************************************
 * @brief           检查I2C控制器忙状态
 *
 * @param[in]       reg            I2C控制器地址
 *
 * @return          uint8_t
 * @retval          1              忙
 *                  0              空闲
 * 
 ************************************************************************************
*/
static inline uint8_t I2C_IsBusy(I2C_TypeDef *reg)
{
  return READ_BIT(reg->status, I2C_BUSY);
}

/**
 ************************************************************************************
 * @brief           复位FIFO
 *
 * @param[in]       reg            I2C控制器地址
 *
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_FifoReset(I2C_TypeDef *reg)
{
  SET_BIT(reg->txCtrl, I2C_TX_FIFO_RST);
  SET_BIT(reg->rxCtrl, I2C_RX_FIFO_RST);
}

/**
 ************************************************************************************
 * @brief           使能slave
 *
 * @param[in]       reg            I2C控制器地址
 *
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_MasterDisable(I2C_TypeDef *reg)
{
  CLEAR_BIT(reg->cmd, I2C_MS_SEL);
}

/**
 ************************************************************************************
 * @brief           使能master
 *
 * @param[in]       reg            I2C控制器地址
 *
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_MasterEnable(I2C_TypeDef *reg)
{
  SET_BIT(reg->cmd, I2C_MS_SEL);
}

/**
 ************************************************************************************
 * @brief           设置分频
 *
 * @param[in]       reg            I2C控制器地址
 * @param[in]       feq            SCL频率
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_FreqSetDiv(I2C_TypeDef *reg, uint32_t freq)
{
  uint32_t pdiv = (26 * 1000 * 1000 / 4 / freq) - 1;
  if ((26 * 1000 * 1000 / 4) % freq != 0)
    pdiv++;
  WRITE_REG(reg->clkDiv, pdiv);
}

/**
 ************************************************************************************
 * @brief           设置地址模式（10 bit or 7 bit）
 *
 * @param[in]       reg            I2C控制器地址
 * @param[in]       en             
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_LongAddrEnable(I2C_TypeDef *reg, bool en)
{
  if (en)
  {
    SET_BIT(reg->cmd, I2C_ADDR_MODE);
  }
  else
  {
    CLEAR_BIT(reg->cmd, I2C_ADDR_MODE);
  }
}

/**
 ************************************************************************************
 * @brief           设置地址模式（10 bit or 7 bit）
 *
 * @param[in]       reg            I2C控制器地址
 * @param[in]       deviceAddr     设备地址（从模式）
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_deviceAddrSet(I2C_TypeDef *reg, uint16_t deviceAddr)
{
  WRITE_REG(reg->deviceAddr, deviceAddr);
}

/**
 ************************************************************************************
 * @brief           获取接收FIFO数据长度
 *
 * @param[in]       reg            I2C控制器地址
 * 
 * @return          uint32_t
 * @retval          >=0            接收FIFO数据个数
 * 
 ************************************************************************************
*/
static inline uint32_t I2C_RecvLen(I2C_TypeDef *reg)
{
  return (READ_BIT(reg->rxfStatus, I2C_RX_FIFO_CNTR) >> I2C_RX_FIFO_CNTR_Pos);
}

/**
 ************************************************************************************
 * @brief           获取中断状态
 *
 * @param[in]       reg            I2C控制器地址
 * 
 * @return          uint32_t
 * @retval          >=0            中断标记
 * 
 ************************************************************************************
*/
static inline uint32_t I2C_InterruptStatusGet(I2C_TypeDef *reg)
{
  return READ_REG(reg->status);
}

/**
 ************************************************************************************
 * @brief           清除中断
 *
 * @param[in]       reg            I2C控制器地址
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_InterruptClearAll(I2C_TypeDef *reg)
{
  SET_BIT(reg->status, I2C_IRQ_MST_STOP | I2C_IRQ_TIME_OUT | I2C_IRQ_TXF_EMPTY | I2C_IRQ_RXF_FULL | I2C_IRQ_SLV_STOP);
}

/**
 ************************************************************************************
 * @brief           使能中断
 *
 * @param[in]       reg            I2C控制器地址
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_InterruptEnableAll(I2C_TypeDef *reg)
{
  SET_BIT(reg->cmd, I2C_IE_MST_STOP | I2C_IE_TIME_OUT | I2C_IE_TXF_EMPTY | I2C_IE_RXF_FULL | I2C_IE_SLV_STOP);
}

/**
 ************************************************************************************
 * @brief           去使能中断
 *
 * @param[in]       reg            I2C控制器地址
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_InterruptDisableAll(I2C_TypeDef *reg)
{
  CLEAR_BIT(reg->cmd, I2C_IE_MST_STOP | I2C_IE_TIME_OUT | I2C_IE_TXF_EMPTY | I2C_IE_RXF_FULL | I2C_IE_SLV_STOP);
}

/**
 ************************************************************************************
 * @brief           设置I2C简单或者组合模式
 *
 * @param[in]       reg            I2C控制器地址
 * @param[in]       mode           简单读写或者组合模式
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_ModeSet(I2C_TypeDef *reg, I2C_Mode mode)
{
  switch (mode)
  {
  case I2C_SIMPLE_WRITE:
    CLEAR_BIT(reg->cmd, I2C_CMB_RW | I2C_RW);
    break;
  case I2C_COMBINE_READ:
    SET_BIT(reg->cmd, I2C_CMB_RW);
    CLEAR_BIT(reg->cmd, I2C_RW);
    break;
  default:
    CLEAR_BIT(reg->cmd, I2C_CMB_RW);
    SET_BIT(reg->cmd, I2C_RW);
    break;
  }
}
/**
 ************************************************************************************
 * @brief           设置发送的数据的数量
 *
 * @param[in]       reg            I2C控制器地址
 * @param[in]       len            数量
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_TxQuantitySet(I2C_TypeDef *reg, uint32_t len)
{
  WRITE_REG(reg->txCtrl, len);
}

/**
 ************************************************************************************
 * @brief           设置接收的数据的数量
 *
 * @param[in]       reg            I2C控制器地址
 * @param[in]       len            数量
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static inline void I2C_RxQuantitySet(I2C_TypeDef *reg, uint32_t len)
{
  WRITE_REG(reg->rxCtrl, len);
}

static inline void I2C_Start(I2C_TypeDef *reg)
{
  SET_BIT(reg->cmd, I2C_START);
}

/**
 ************************************************************************************
 * @brief           释放clock
 *
 * @param[in]       i2c            I2C控制器
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static int32_t I2C_ClkSrcRelease(I2C_Handle *i2c)
{
  CLK_SetPdcoreLspCrmRegs((CLK_PdcoreLspCrmRegs)(i2c->p_I2C_Res->pclk), 0);
  CLK_SetPdcoreLspCrmRegs((CLK_PdcoreLspCrmRegs)(i2c->p_I2C_Res->wclk), 0);
  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           申请clock
 *
 * @param[in]       i2c            I2C控制器
 * 
 * @return          void
 * 
 ************************************************************************************
*/
static int32_t I2C_ClkSrcRequest(I2C_Handle *i2c)
{

  if (i2c->info.flags&I2C_FLAG_SETUP)
  {
    return DRV_OK;
  }
  CLK_SetPdcoreLspCrmRegs((CLK_PdcoreLspCrmRegs)(i2c->p_I2C_Res->pclk), 1);
  CLK_SetPdcoreLspCrmRegs((CLK_PdcoreLspCrmRegs)(i2c->p_I2C_Res->wclk), 1);
  i2c->info.flags = I2C_FLAG_SETUP;

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           配置发送
 *
 * @param[in]       i2c            I2C控制器
 * 
 * @return          int32_t
 * @retval          DRV_ERR       无FIFO空间
 *                  >0            写入FIFO的数据长度         
 ************************************************************************************
*/
static int32_t I2C_WriteMsg(I2C_Handle *i2c)
{
  I2C_TypeDef       *reg = (I2C_TypeDef *)i2c->p_I2C_Res->regBase;
  I2C_TRANSFER_INFO *msg = &i2c->info.tx;

  uint8_t  depth  = I2C_TxFifoVacancyGet(reg);
  uint16_t remain = msg->len - msg->cnt;
  uint8_t *txBuf  = msg->buf + msg->cnt;

  if (!depth)
  {
    I2C_PrintError("depth %d\r\n", depth);
    return DRV_ERR;
  }

  uint16_t txLen = I2C_MIN(depth, remain);
  msg->cnt += txLen;

  I2C_PrintDebug("I2C Will Send %d bytes Msg\r\n", txLen);

  I2C_WriteFifo(reg, txBuf, txLen);

  return txLen;
}

/**
 ************************************************************************************
 * @brief           配置接收
 *
 * @param[in]       i2c            I2C控制器
 * 
 * @return          int32_t
 * @retval          DRV_ERR       无FIFO空间
 *                  >0            读取FIFO的数据长度         
 ************************************************************************************
*/
static int32_t I2C_ReadMsg(I2C_Handle *i2c)
{
  I2C_TypeDef       *reg = (I2C_TypeDef *)i2c->p_I2C_Res->regBase;
  I2C_TRANSFER_INFO *msg = &i2c->info.rx;

  uint8_t  depth  = I2C_RxFifoVacancyGet(reg);
  uint16_t remain = msg->len - msg->cnt;
  uint8_t *rxBuf  = msg->buf + msg->cnt;

  if (!depth)
  {
    I2C_PrintError("I2C RxFifo has no data\r\n");
    return DRV_ERR;
  }

  // I2C_PrintDebug("I2C Will Recv rxfifo : %d remain : %d bytes Msg", depth, remain);

  uint16_t rxLen = I2C_MIN(depth, remain);
  msg->cnt += rxLen;

  I2C_PrintDebug("I2C Will Recv %d bytes Msg\r\n", rxLen);

  I2C_ReadFifo(reg, rxBuf, rxLen);

  return rxLen;
}

/**
 ************************************************************************************
 * @brief           中断处理程序
 *
 * @param[in]       id            中断号
 * @param[in]       param         中断处理函数参数
 * 
 * @return          void
 *         
 ************************************************************************************
*/
static void I2C_InterruptServer(int id, void *param)
{
  I2C_Handle  *i2c             = (I2C_Handle *)param;
  I2C_TypeDef *reg             = (I2C_TypeDef *)i2c->p_I2C_Res->regBase;
  uint32_t     event           = 0;
  I2C_STATUS  *status          = &i2c->info.status;
  I2C_Callback signalEventFunc = i2c->info.cbEvent;

  uint32_t irqStatus = I2C_InterruptStatusGet(reg);
  I2C_InterruptClearAll(reg);
  I2C_PrintDebug("I2C Irq Status = 0x%x\r\n", irqStatus);

  if (irqStatus & (I2C_STA_ERR_DEVICE | I2C_STA_ERR_DATA | I2C_IRQ_TIME_OUT))
  {
    event |= I2C_EVENT_ADDRESS_NACK;
    I2C_InterruptDisableAll(reg);
    if (signalEventFunc)
    {
      signalEventFunc(i2c,event);
    }
    
    I2C_PrintError("I2C Error Occur\r\n");

    status->busy = 0;
    return;
  }

  if (irqStatus & (I2C_IRQ_MST_STOP))
  {
    event |= I2C_EVENT_TRANSFER_DONE;
    I2C_InterruptDisableAll(reg);

    if (i2c->info.status.direction)
    {
      I2C_ReadMsg(i2c);
    }

    if (signalEventFunc)
    {
      signalEventFunc(i2c,event);
    }

    I2C_PrintDebug("I2C Master Xfer Done\r\n");

    status->busy = 0;
    return;
  }
  else if (irqStatus & (I2C_IRQ_SLV_STOP))
  {
    event |= I2C_EVENT_TRANSFER_DONE;
    I2C_InterruptDisableAll(reg);
    if (i2c->info.status.direction)
    {
      I2C_ReadMsg(i2c);
    }

    if (signalEventFunc)
    {
      signalEventFunc(i2c, event);
    }

    I2C_PrintDebug("I2C Slave Xfer Done\r\n");

    status->busy = 0;
    return;
  }

  if (irqStatus & I2C_IRQ_TXF_EMPTY)
  {
    I2C_WriteMsg(i2c);
  }
  else if (irqStatus & I2C_IRQ_RXF_FULL)
  {
    I2C_ReadMsg(i2c);
  }

  return;
}

/**
 ************************************************************************************
 * @brief           准备写
 *
 * @param[in]       i2c           I2C控制器句柄
 * @param[in]       tx            需要发送的数据的信息
 * 
 * @return          int32_t
 *                  DRV_OK        准备成功
 *         
 ************************************************************************************
*/
static int32_t I2C_WritePrepare(I2C_Handle *i2c, I2C_TRANSFER_INFO *tx)
{
  I2C_TypeDef *reg = (I2C_TypeDef *)i2c->p_I2C_Res->regBase;

  I2C_TxQuantitySet(reg, tx->len);
  I2C_WriteMsg(i2c);

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           准备读
 *
 * @param[in]       i2c            I2C控制器句柄
 * @param[in]       rx            需要接收的数据的信息
 * 
 * @return          int32_t
 *                  DRV_OK        准备成功
 *         
 ************************************************************************************
*/
static int32_t I2C_ReadPrepare(I2C_Handle *i2c, I2C_TRANSFER_INFO *rx)
{
  I2C_TypeDef *reg = (I2C_TypeDef *)i2c->p_I2C_Res->regBase;

  I2C_RxQuantitySet(reg, rx->len);

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           执行传输
 *
 * @param[in]       i2c            I2C控制器句柄
 * @param[in]       rx             需要接收的数据的信息
 * 
 * @return          int32_t
 *                  DRV_ERR_BUSY        设备忙
 *                  DRV_OK              执行成功 
 ************************************************************************************
*/
static int32_t I2C_DoXfer(I2C_Handle *i2c)
{
  I2C_TypeDef       *regs   = (I2C_TypeDef *)i2c->p_I2C_Res->regBase;
  I2C_INFO          *info   = &i2c->info;
  I2C_STATUS        *status = &info->status;
  I2C_TRANSFER_INFO *tx     = &info->tx;
  I2C_TRANSFER_INFO *rx     = &info->rx;

  if (I2C_IsBusy(regs))
  {
    I2C_PrintError("I2C is in busy\r\n");
    return DRV_ERR_BUSY;
  }

  if (status->mode)
  {
    I2C_PrintDebug("I2C Set Master Mode\r\n");
    I2C_MasterEnable(regs);
  }
  else
  {
    I2C_PrintDebug("I2C Set Slave Mode\r\n");
    I2C_MasterDisable(regs);
  }

  I2C_FifoReset(regs);
  I2C_InterruptEnableAll(regs);

  // slave mode address set by i2c_control function
  if (status->mode) // Master
  {
    I2C_deviceAddrSet(regs, info->deviceAddr);
  }

  if (info->ctrl & I2C_XFER_CTRL_CMB_READ)
  {
    I2C_ModeSet(regs, I2C_COMBINE_READ);
    I2C_WritePrepare(i2c, tx);
    I2C_ReadPrepare(i2c, rx);
  }
  else if (status->direction)
  {
    I2C_ModeSet(regs, I2C_SIMPLE_READ);
    I2C_ReadPrepare(i2c, rx);
  }
  else
  {
    I2C_ModeSet(regs, I2C_SIMPLE_WRITE);
    I2C_WritePrepare(i2c, tx);
  }

  I2C_PrintDebug("0x%08x\r\n", regs->cmd);
  I2C_PrintDebug("0x%08x\r\n", regs->deviceAddr);
  I2C_PrintDebug("0x%08x\r\n", regs->clkDiv);
  I2C_PrintDebug("0x%08x\r\n", regs->txCtrl);
  I2C_PrintDebug("0x%08x\r\n", regs->rxCtrl);

  I2C_Start((I2C_TypeDef *)i2c->p_I2C_Res->regBase);

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           执行传输
 *
 * @param[in]       i2c            I2C控制器句柄
 * @param[in]       addr           设备地址
 * @param[in]      txData          发送数据地址
 * @param[in]      txNum           发送数据长度
 * @param[in]      rxData          接收数据地址
 * @param[in]      rxNum           接收数据长度
 * @param[in]      master          控制器模式（master or slave）
 * 
 * @return          int32_t
 *                  DRV_ERR_BUSY        设备忙
 *                  DRV_OK              执行成功
 *                  DRV_ERR_PARAMETER   参数错误 
 ************************************************************************************
*/
static int32_t I2C_Transfer(I2C_Handle *i2c,
                                   uint32_t    addr,
                                   uint8_t    *txData,
                                   uint32_t    txNum,
                                   uint8_t    *rxData,
                                   uint32_t    rxNum,
                                   bool        master)
{
  I2C_INFO          *info   = &i2c->info;
  I2C_STATUS        *status = &info->status;
  I2C_TRANSFER_INFO *tx     = &info->tx;
  I2C_TRANSFER_INFO *rx     = &info->rx;

  if (!txData && !rxData)
  {
    I2C_PrintError("I2C Transmit Data NULL\r\n");
    return DRV_ERR_PARAMETER;
  }

  if (txData && txNum)
  {
    tx->buf = txData;
    tx->len = txNum;
    tx->cnt = 0;
    I2C_PrintDebug("I2C Start Send %d bytes Msg\r\n", txNum);
  }

  if (rxData && rxNum)
  {
    rx->buf = rxData;
    rx->len = rxNum;
    rx->cnt = 0;
    I2C_PrintDebug("I2C Start Recv %d bytes Msg\r\n", rxNum);
  }

  status->busy     = 1;
  status->mode     = master ? 1 : 0;
  info->deviceAddr = (uint16_t)addr;
  info->ctrl       = 0;

  if (txData && rxData)
  {
    info->ctrl |= I2C_XFER_CTRL_CMB_READ;
    status->direction = 1;
  }
  else if (txData)
  {
    status->direction = 0;
  }
  else
  {
    status->direction = 1;
  }

  return I2C_DoXfer(i2c);
}
/**
 ************************************************************************************
 * @brief           初始化I2C控制器
 *
 * @param[in]       I2C            I2C控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_OK          成功   
 ************************************************************************************
*/
int32_t I2C_Initialize(I2C_Handle *i2c, I2C_Callback cbEvent)
{
  i2c->info.cbEvent = cbEvent;

  if (i2c->info.flags&I2C_FLAG_INIT)
  {
    return DRV_OK;
  }

  i2c->info.flags = I2C_FLAG_INIT;

  return DRV_OK;
}
/**
 ************************************************************************************
 * @brief           去初始化I2C控制器
 *
 * @param[in]       i2c            I2C控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_OK          成功   
 ************************************************************************************
*/
int32_t I2C_Uninitialize(I2C_Handle *i2c)
{
  i2c->info.flags = 0;

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           设置I2C控制器的供电
 *
 * @param[in]       I2C             I2C控制器句柄
 * @param[in]       state           设置控制的状态
 *
 * @return          int32_t
 * @retval          DRV_ERR_PARAMETER             错误的参数
 *                  DRV_ERR_UNSUPPORTED           不支持             
 *                  DRV_ERR                       错误
 *                  DRV_OK                        成功   
 ************************************************************************************
*/
int32_t I2C_PowerControl(I2C_Handle *i2c, DRV_POWER_STATE state)
{

  switch (state)
  {
  case DRV_POWER_OFF:
    if (!(i2c->info.flags & I2C_FLAG_POWER))
    {
      return DRV_OK;
    }

    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(i2c->p_I2C_Res->intNum));
    osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(i2c->p_I2C_Res->intNum));

    I2C_ClkSrcRelease(i2c);

    i2c->info.flags &= ~I2C_FLAG_POWER;
    break;

  case DRV_POWER_LOW:
    return DRV_ERR_UNSUPPORTED;
    break;

  case DRV_POWER_FULL:
    if (!(i2c->info.flags & I2C_FLAG_INIT))
    {
      return DRV_ERR;
    }

    if (i2c->info.flags & I2C_FLAG_POWER)
    {
      return DRV_OK;
    }

    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(i2c->p_I2C_Res->intNum), 2, IRQ_HIGH_LEVEL);
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(i2c->p_I2C_Res->intNum), I2C_InterruptServer, i2c);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(i2c->p_I2C_Res->intNum));

    I2C_ClkSrcRequest(i2c);

    i2c->info.flags = I2C_FLAG_POWER;
    break;
  }
  return DRV_OK;
}


/**
 ************************************************************************************
 * @brief           获取I2C控制器收或发数据的数量.
 *
 * @param[in]       i2c             I2C控制器句柄
 * 
 * @return          int32_t         
 * @retval          >=0             收取或者发送的字节数
 ************************************************************************************
 */
int32_t I2C_GetDataCount(I2C_Handle *i2c)
{
  I2C_INFO   *info   = &i2c->info;
  I2C_STATUS *status = &info->status;

  if (status->direction)
  {
    return info->rx.cnt;
  }
  else
  {
    return info->tx.cnt;
  }
}

/**
 ************************************************************************************
 * @brief           控制I2C控制器.
 *
 * @param[in]       UART            I2C控制器句柄
 * @param[in]       control         控制命令
 * @param[in]       arg             控制命令参数
 * 
 * @return          初始化返回值.
 * @retval          ==DRV_OK        执行成功
 *                  ==DRV_ERR_UNSUPPORTED  不支持的命令
 ************************************************************************************
 */
int32_t I2C_Control(I2C_Handle *i2c, uint32_t control, uint32_t arg)
{
  I2C_TypeDef *reg    = (I2C_TypeDef *)i2c->p_I2C_Res->regBase;
  
  if (!(i2c->info.flags & I2C_FLAG_POWER)) {
    /* Driver not powered */
    return DRV_ERR;
  }
  switch (control)
  {
  case I2C_OWN_ADDRESS:
    // slave mode
    I2C_deviceAddrSet(reg, (uint16_t)arg);
    break;

  case I2C_BUS_SPEED:
    switch (arg)
    {
    case I2C_BUS_SPEED_STANDARD:
      I2C_FreqSetDiv(reg, 100000);
      break;
    case I2C_BUS_SPEED_FAST:
      I2C_FreqSetDiv(reg, 400000);
      break;
    case I2C_BUS_SPEED_FAST_PLUS:
      I2C_FreqSetDiv(reg, 1000000);
      break;
    case I2C_BUS_SPEED_HIGH:
      I2C_FreqSetDiv(reg, 3400000);
      break;
    default:
      return DRV_ERR_UNSUPPORTED;
    }
    break;

  case I2C_BUS_CLEAR:
    return DRV_ERR_UNSUPPORTED;
    break;

  case I2C_ABORT_TRANSFER:
    return DRV_ERR_UNSUPPORTED;
    break;

  case I2C_LONG_ADDRESS:
    if ((arg == 1) || (arg == 0))
    {
      I2C_LongAddrEnable(reg, arg);
    }
    else
    {
      return DRV_ERR_UNSUPPORTED;
    }
    break;

  default:
    return DRV_ERR_UNSUPPORTED;
  }

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           master执行写传输
 *
 * @param[in]       i2c            I2C控制器句柄
 * @param[in]       addr           设备地址
 * @param[in]      data          发送数据地址
 * @param[in]      num           发送数据长度
 * 
 * @return          int32_t
 *                  DRV_ERR_BUSY        设备忙
 *                  DRV_OK              执行成功
 *                  DRV_ERR_PARAMETER   参数错误 
 ************************************************************************************
*/
int32_t I2C_MasterSend(I2C_Handle *i2c, uint32_t addr, const uint8_t *data, uint32_t num)
{
  if (!i2c)
  {
    return DRV_ERR;
  }

  if (i2c->info.status.busy)
  {
    return DRV_ERR_BUSY;
  }

  return I2C_Transfer(i2c, addr, (uint8_t *)data, num, NULL, 0, true);
}

/**
 ************************************************************************************
 * @brief           master执行读传输
 *
 * @param[in]       i2c            I2C控制器句柄
 * @param[in]       addr           设备地址
 * @param[in]      data          接收数据地址
 * @param[in]      num           接收数据长度
 * 
 * @return          int32_t
 *                  DRV_ERR_BUSY        设备忙
 *                  DRV_OK              执行成功
 *                  DRV_ERR_PARAMETER   参数错误 
 ************************************************************************************
*/
int32_t I2C_MasterReceive(I2C_Handle *i2c, uint32_t addr, uint8_t *data, uint32_t num)
{
  if (!i2c)
  {
    return DRV_ERR_PARAMETER;
  }

  if (i2c->info.status.busy)
  {
    return DRV_ERR_BUSY;
  }

  return I2C_Transfer(i2c, addr, NULL, 0, data, num, true);
}

/**
 ************************************************************************************
 * @brief           master执行组合传输
 *
 * @param[in]       i2c            I2C控制器句柄
 * @param[in]       addr           设备地址
 * @param[in]      txData          发送数据地址
 * @param[in]      txNum           发送数据长度
 * @param[in]      rxData          接收数据地址
 * @param[in]      rxNum           接收数据长度
 * @param[in]      master          控制器模式（master or slave）
 * 
 * @return          int32_t
 *                  DRV_ERR_BUSY        设备忙
 *                  DRV_OK              执行成功
 *                  DRV_ERR_PARAMETER   参数错误 
 ************************************************************************************
*/
int32_t I2C_MasterTransfer(I2C_Handle *i2c, uint32_t addr, uint8_t *txData, uint32_t txNum, uint8_t *rxData, uint32_t rxNum)
{
  if (!i2c)
  {
    return DRV_ERR_PARAMETER;
  }

  if (i2c->info.status.busy)
  {
    return DRV_ERR_BUSY;
  }

  return I2C_Transfer(i2c, addr, txData, txNum, rxData, rxNum, true);
}


/**
 ************************************************************************************
 * @brief           slave执行写传输
 *
 * @param[in]       i2c            I2C控制器句柄
 * @param[in]       addr           设备地址
 * @param[in]      data            发送数据地址
 * @param[in]      num             发送数据长度
 * 
 * @return          int32_t
 *                  DRV_ERR_BUSY        设备忙
 *                  DRV_OK              执行成功
 *                  DRV_ERR_PARAMETER   参数错误 
 ************************************************************************************
*/
int32_t I2C_SlaveSend(I2C_Handle *i2c, const uint8_t *data, uint32_t num)
{
  if (!i2c)
  {
    return DRV_ERR_PARAMETER;
  }

  if (i2c->info.status.busy)
  {
    return DRV_ERR_BUSY;
  }

  return I2C_Transfer(i2c, 0x0, (uint8_t *)data, num, NULL, 0, false);
}

/**
 ************************************************************************************
 * @brief           slave执行读传输
 *
 * @param[in]       i2c            I2C控制器句柄
 * @param[in]      data          接收数据地址
 * @param[in]      num           接收数据长度
 * 
 * @return          int32_t
 *                  DRV_ERR_BUSY        设备忙
 *                  DRV_OK              执行成功
 *                  DRV_ERR_PARAMETER   参数错误 
 ************************************************************************************
*/
int32_t I2C_SlaveReceive(I2C_Handle *i2c, uint8_t *data, uint32_t num)
{
  if (!i2c)
  {
    return DRV_ERR_PARAMETER;
  }

  if (i2c->info.status.busy)
  {
    return DRV_ERR_BUSY;
  }

  return I2C_Transfer(i2c, 0x0, NULL, 0, data, num, false);
}

/**
 ************************************************************************************
 * @brief           获取I2C控制器状态
 *
 * @param[in]       i2c            I2C控制器句柄
 *
 * @return          I2C控制器状态.
 * @retval          I2C_STATUS
 ************************************************************************************
 */
I2C_STATUS I2C_GetStatus(I2C_Handle *i2c)
{
  return i2c->info.status;
}

/**
 ************************************************************************************
 * @brief           软复位I2C控制器
 *
 * @param[in]       i2c            I2C控制器句柄
 *
 * @return          void
 ************************************************************************************
 */
void I2C_SoftReset(I2C_Handle *i2c)
{
  i2c->info.status.busy = 0;

  CLK_SetPdcoreLspCrmRegs((CLK_PdcoreLspCrmRegs)(i2c->p_I2C_Res->pclkRst), 0);
  CLK_SetPdcoreLspCrmRegs((CLK_PdcoreLspCrmRegs)(i2c->p_I2C_Res->wclkRst), 0);
  CLK_SetPdcoreLspCrmRegs((CLK_PdcoreLspCrmRegs)(i2c->p_I2C_Res->pclkRst), 1);
  CLK_SetPdcoreLspCrmRegs((CLK_PdcoreLspCrmRegs)(i2c->p_I2C_Res->wclkRst), 1);
}
