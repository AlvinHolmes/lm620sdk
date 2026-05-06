/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        lcd_priv.h
 *
 * @brief       LCD内部定义，寄存器定义等.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team         创建
 ************************************************************************************
 */

#ifndef _LCD_PRIV_H_
#define _LCD_PRIV_H_

#include <drv_common.h>


/*** Bit definition for [version] register(0x00000000) ****/
#define VER_REG_L_Pos                     (16)
#define VER_REG_L_Msk                     (0xFFUL << VER_REG_L_Pos)
#define VER_REG_L                         VER_REG_L_Msk                     /*!<Low 8bits of Hardware version of SPI_LCD module. */
#define VER_REG_H_Pos                     (24)
#define VER_REG_H_Msk                     (0xFFUL << VER_REG_H_Pos)
#define VER_REG_H                         VER_REG_H_Msk                     /*!<High 8bits of Hardware version of SPI_LCD module. */


/*** Bit definition for [protocol] register(0x00000004) ****/
#define SPI_BIDIR_Pos                     (0)
#define SPI_BIDIR_Msk                     (0x1UL << SPI_BIDIR_Pos)
#define SPI_BIDIR                         SPI_BIDIR_Msk                     /*!<SPI data bus signal definition
1: bi-dir DIO configuration
0: uni-dir DIN/DOUT configuration */
#define DBI_DCX_Pos                       (1)
#define DBI_DCX_Msk                       (0x1UL << DBI_DCX_Pos)
#define DBI_DCX                           DBI_DCX_Msk                       /*!<Dedicated D/CX
1: dedicated D/CX
0: D/CX encoded in DOUT */
#define SAMP_SEL_Pos                      (2)
#define SAMP_SEL_Msk                      (0x1UL << SAMP_SEL_Pos)
#define SAMP_SEL                          SAMP_SEL_Msk                      /*!<Sample condition selection
0: sample input data at next rise edge of SCLK
1: sample input data at next fall edge of SCLK */
#define DMY_CYC_Pos                       (4)
#define DMY_CYC_Msk                       (0xFUL << DMY_CYC_Pos)
#define DMY_CYC                           DMY_CYC_Msk                       /*!<Dummy cycle in read operation
0: no dummy cycle in read operation
others:dummy cycle setting in read operation */


/*** Bit definition for [operation] register(0x00000008) ****/
#define LCD_RW_Pos                        (0)
#define LCD_RW_Msk                        (0x1UL << LCD_RW_Pos)
#define LCD_RW                            LCD_RW_Msk                        /*!<Read/write indication:
1: read status
0: write register/frame-memory */
#define WR_MEM_Pos                        (1)
#define WR_MEM_Msk                        (0x1UL << WR_MEM_Pos)
#define WR_MEM                            WR_MEM_Msk                        /*!<Write object indication 
1: write frame memory
0: write LCD register */
#define FIFO_THRES_Pos                    (2)
#define FIFO_THRES_Msk                    (0x3UL << FIFO_THRES_Pos)
#define FIFO_THRES                        FIFO_THRES_Msk                    /*!<2’h0:threshold is 4
2’h1:threshold is 8
2’h2:threshold is 12
2’h3:threshold is 16 */
#define DMA_EN_Pos                        (4)
#define DMA_EN_Msk                        (0x1UL << DMA_EN_Pos)
#define DMA_EN                            DMA_EN_Msk                        /*!<Use system Dma to handle write data. */
#define FIFO_INT_EN_Pos                   (5)
#define FIFO_INT_EN_Msk                   (0x1UL << FIFO_INT_EN_Pos)
#define FIFO_INT_EN                       FIFO_INT_EN_Msk                   /*!<FIFO interrupt enable
1:interrupt enabled
0:interrupt disabled */
#define TRANS_INT_EN_Pos                  (6)
#define TRANS_INT_EN_Msk                  (0x1UL << TRANS_INT_EN_Pos)
#define TRANS_INT_EN                      TRANS_INT_EN_Msk                  /*!<Transaction complete interrupt enable
1:interrupt enabled
0:interrupt disabled */


/*** Bit definition for [dcsCmd] register(0x0000000c) ****/
#define DCS_CMD_Pos                       (0)
#define DCS_CMD_Msk                       (0xFFUL << DCS_CMD_Pos)
#define DCS_CMD                           DCS_CMD_Msk                       /*!<DCS command code */


/*** Bit definition for [transLen] register(0x00000010) ****/
#define TRANS_LEN_Pos                     (0)
#define TRANS_LEN_Msk                     (0x3FFFFUL << TRANS_LEN_Pos)
#define TRANS_LEN                         TRANS_LEN_Msk                     /*!<Parameter/data length,DCS_CMD doesnot take into account.unit is byte. */


/*** Bit definition for [start] register(0x00000014) ****/
#define START_Pos                         (0)
#define START_Msk                         (0x1UL << START_Pos)
#define START                             START_Msk                         /*!<Write 1 will start operation;write 0 has no affection.
Read will return real time busy status. */


/*** Bit definition for [txFifo] register(0x00000018) ****/
#define TX_FIFO_Pos                       (0)
#define TX_FIFO_Msk                       (0x1UL << TX_FIFO_Pos)
#define TX_FIFO                           TX_FIFO_Msk                       /*!<Write data will write to FIFO;read will return 0; */


/*** Bit definition for [transStatus] register(0x0000001c) ****/
#define TXF_SPACE_CNT_Pos                 (0)
#define TXF_SPACE_CNT_Msk                 (0x3FUL << TXF_SPACE_CNT_Pos)
#define TXF_SPACE_CNT                     TXF_SPACE_CNT_Msk                 /*!<Transmit FIFO space. */
#define TXF_NOT_FULL_Pos                  (8)
#define TXF_NOT_FULL_Msk                  (0x1UL << TXF_NOT_FULL_Pos)
#define TXF_NOT_FULL                      TXF_NOT_FULL_Msk                  /*!<0:full
1:not full */
#define FIFO_INT_Pos                      (9)
#define FIFO_INT_Msk                      (0x1UL << FIFO_INT_Pos)
#define FIFO_INT                          FIFO_INT_Msk                      /*!<Real time status of FIFO_INT */
#define TRANS_INT_Pos                     (10)
#define TRANS_INT_Msk                     (0x1UL << TRANS_INT_Pos)
#define TRANS_INT                         TRANS_INT_Msk                     /*!<Real time status of TRANS_INT,write 1 will clear interrupt */


/*** Bit definition for [wrParam] register(0x00000020) ****/
#define WR_PARAM_Pos                      (0)
#define WR_PARAM_Msk                      (0xFFUL << WR_PARAM_Pos)
#define WR_PARAM                          WR_PARAM_Msk                      /*!<Parameter in write transaction. */


/*** Bit definition for [rdValue] register(0x00000040) ****/
#define RD_VALUE_Pos                      (0)
#define RD_VALUE_Msk                      (0xFFUL << RD_VALUE_Pos)
#define RD_VALUE                          RD_VALUE_Msk                      /*!<Return value in read transaction. */


/*** Bit definition for [debug] register(0x00000080) ****/
#define RGB565_SWAP_Pos                   (0)
#define RGB565_SWAP_Msk                   (0x1UL << RGB565_SWAP_Pos)
#define RGB565_SWAP                       RGB565_SWAP_Msk                   /*!<Swap RGB565 hi/lo byte. */


/*** Bit definition for [emuSize] register(0x00000090) ****/
#define FORMAT_Pos                        (0)
#define FORMAT_Msk                        (0xFFUL << FORMAT_Pos)
#define FORMAT                            FORMAT_Msk                        /*!<lcd simulator pixel data format. */


/*** Bit definition for [emuSize] register(0x00000094) ****/
#define HEIGHT_Pos                        (0)
#define HEIGHT_Msk                        (0xFFFFUL << HEIGHT_Pos)
#define HEIGHT                            HEIGHT_Msk                        /*!<lcd simulator height pixel. */
#define WIDTH_Pos                         (16)
#define WIDTH_Msk                         (0xFFFFUL << WIDTH_Pos)
#define WIDTH                             WIDTH_Msk                         /*!<lcd simulator width pixel. */


/**
 * @brief  REG_Lcd
 */
typedef struct {
  __IO uint32_t  version;                          /*!< Offset 0x00000000 */
  __IO uint32_t  protocol;                         /*!< Offset 0x00000004 */
  __IO uint32_t  operation;                        /*!< Offset 0x00000008 */
  __IO uint32_t  dcsCmd;                           /*!< Offset 0x0000000c */
  __IO uint32_t  transLen;                         /*!< Offset 0x00000010 */
  __IO uint32_t  start;                            /*!< Offset 0x00000014 */
  __IO uint32_t  txFifo;                           /*!< Offset 0x00000018 */
  __IO uint32_t  transStatus;                      /*!< Offset 0x0000001c */
  __IO uint32_t  wrParam[8];                       /*!< Offset 0x00000020 */
  __IO uint32_t  rdValue[8];                       /*!< Offset 0x00000040 */
  __IO uint32_t  rsvd1[8];                        /*!< Offset 0x00000044 */
  __IO uint32_t  debug;                            /*!< Offset 0x00000080 */
  __IO uint32_t  rsvd2[3];                         /*!< Offset 0x00000084 */
  __IO uint32_t  emuFormat;                       /*!< Offset 0x00000090 */
  __IO uint32_t  emuSize;                         /*!< Offset 0x00000094 */
} REG_Lcd;

#define LCD_FIFO_THRES_4        (0)
#define LCD_FIFO_THRES_8        (1)
#define LCD_FIFO_THRES_12       (2)
#define LCD_FIFO_THRES_16       (3)

#define LCD_USE_APB_MONITOR     (0)

#endif /* End of _LCD_PRIV_H_*/
