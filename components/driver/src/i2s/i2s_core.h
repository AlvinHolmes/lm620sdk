

#ifndef __I2S_CORE_H__
#define __I2S_CORE_H__

#include "drv_common.h"
#include "drv_i2s.h"


#define I2S_FIFO_THRESHOLD                          8

#define I2S_PROCESS_CTRL_EN_MASK                    (1 << 2)
#define I2S_PROCESS_CTRL_RX_EN_MASK                 (1 << 1)
#define I2S_PROCESS_CTRL_TX_EN_MASK                 (1 << 0)

#define I2S_TIMING_MASTER_MODE_MASK                 (1 << 0)
#define I2S_TIMING_LOOP_BACK_MASK                   (1 << 1)
#define I2S_TIMING_PHA_SEL_MASK                     (1 << 2)
#define I2S_TIMING_TIMING_SEL_MASK                  (1 << 3)
#define I2S_TIMING_LONG_FSYNC_MASK                  (1 << 4)
#define I2S_TIMING_EXTRA_CYCLE_MASK                 (1 << 5)
#define I2S_TIMING_ALIGN_MODE_POS                   (6)
#define I2S_TIMING_ALIGN_MODE_MASK                  (0x3 << I2S_TIMING_ALIGN_MODE_POS)
#define I2S_TIMING_CHNL_NUM_POS                     (8)
#define I2S_TIMING_CHNL_NUM_MASK                    (0x7 << I2S_TIMING_CHNL_NUM_POS)
#define I2S_TIMING_LANE_NUM_MASK                    (0x3 << 11)
#define I2S_TIMING_TS_CFG_POS                       13
#define I2S_TIMING_TS_CFG_MASK                      (0x7 << I2S_TIMING_TS_CFG_POS)
#define I2S_TIMING_TS_WIDTH_POS                     16
#define I2S_TIMING_TS_WIDTH_MASK                    (0x1f << 16)
#define I2S_TIMING_DATA_SIZE_POS                    21
#define I2S_TIMING_DATA_SIZE_MASK                   (0x1f << 21)
#define I2S_TIMING_SMJZ_MODE_MASK                   (1 << 30)
#define I2S_TIMING_FORMAT_ERROR_MASK                (((uint32_t)1) << 31)

#define I2S_FIFO_RX0_THRES_POS                      (16)
#define I2S_FIFO_RX0_THRES_MASK                     (0x1f << I2S_FIFO_RX0_THRES_POS)
#define I2S_FIFO_TX_THRES_POS                       (8)
#define I2S_FIFO_TX_THRES_MASK                      (0x1f << I2S_FIFO_TX_THRES_POS)
#define I2S_FIFO_RX0_DMA_EN_MASK                    (1 << 5)
#define I2S_FIFO_TX_DMA_EN_MASK                     (1 << 4)


typedef struct 
{
    __IO uint32_t version;
    __IO uint32_t processCtrl;
    __IO uint32_t timingCtrl;
    __IO uint32_t fifoCtrl;
    __IO uint32_t fifoStatus;
    __IO uint32_t intEn;
    __IO uint32_t intStatus;
    __IO uint32_t data;
    __IO uint32_t frameCntr;
    __IO uint32_t data2;
}I2S_Reg;

typedef struct 
{
    I2S_Reg *reg;
    I2S_BusCfg *cfg;
    uint32_t workClk;
}I2S_CoreHandle;


int8_t I2S_Config(I2S_CoreHandle *coreHandle);

void I2S_TxEnable(bool enable);

void I2S_RxEnable(bool enable);

uint32_t I2S_GetDataFifoAddr(void);

void I2S_DeConfig(void);
#endif
