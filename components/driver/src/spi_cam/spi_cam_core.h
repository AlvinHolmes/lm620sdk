
#ifndef __SPI_CAM_CORE_H__
#define __SPI_CAM_CORE_H__

#include <drv_common.h>

#define MTK_START       	(0x01)
#define MTK_DATA_PACKET   	(0x40)
#define MTK_END        		(0x00)
#define BT656_START        	(0xab)
#define BT656_LINE_START	(0x80)
#define BT656_END       	(0xb6)

#define     SPI_SSPE_BACK_POS               4
#define     SPI_SSPE_BACK_MASK              (1 << SPI_SSPE_BACK_POS)
#define     SPI_SSPE_POS                    (1)
#define     SPI_SSPE_MASK                   (1 << SPI_SSPE_POS)
#define     SPI_MS_POS                      2
#define     SPI_MS_MASK                     (1 << SPI_MS_POS)
#define     SPI_FRAME_FMT_POS               0
#define     SPI_FRAME_FMT_MASK              (0x3 << SPI_FRAME_FMT_POS)    
#define     SPI_PHA_POS                     3
#define     SPI_PHA_MASK                    (1 << SPI_PHA_POS)   
#define     SPI_POL_POS                     2
#define     SPI_POL_MASK                    (1 << SPI_POL_POS)
#define     SPI_DSS_POS                     4
#define     SPI_DSS_MASK                    (0x1F << SPI_DSS_POS)
#define     SPI_CAM_MODE_POS                12
#define     SPI_CAM_MODE_MASK               (1 << SPI_CAM_MODE_POS)
#define     SPI_LANE_NUM_POS                13
#define     SPI_LANE_NUM_MASK               (0x3 << SPI_LANE_NUM_POS)
#define     SPI_TX_FIFO_THRES_POS           8
#define     SPI_TX_FIFO_THRES_MASK          (0xf << SPI_TX_FIFO_THRES_POS)
#define     SPI_RX_FIFO_THRES_POS           4
#define     SPI_RX_FIFO_THRES_MASK          (0xf << SPI_RX_FIFO_THRES_POS)
#define     SPI_TX_DMA_EN_POS               3
#define     SPI_TX_DMA_EN_MASK              (1 << SPI_TX_DMA_EN_POS)
#define     SPI_RX_DMA_EN_POS               2
#define     SPI_RX_DMA_EN_MASK              (1 << SPI_RX_DMA_EN_POS)
#define     SPI_SYNC_CODE_POS               0
#define     SPI_SYNC_CODE_MASK              (1 << SPI_SYNC_CODE_POS)
#define     SPI_RST_CAM_FIFO_POS            1
#define     SPI_RST_CAM_FIFO_MASK           (1 << SPI_RST_CAM_FIFO_POS)
#define     SPI_SAMPLE_MODE_POS             2
#define     SPI_SAMPLE_MODE_MASK            (1 << SPI_SAMPLE_MODE_POS)
#define     SPI_ID_SOF_POS                  8
#define     SPI_ID_SOF_MASK                 (0xFF << SPI_ID_SOF_POS)
#define     SPI_ID_EOF_POS                  16
#define     SPI_ID_EOF_MASK                 (0xFF << SPI_ID_EOF_POS)
#define     SPI_ID_SOL_POS                  24
#define     SPI_ID_SOL_MASK                 (0xFF << SPI_ID_SOL_POS)
#define     SPI_TX_FIFO_CNT_POS             12
#define     SPI_TX_FIFO_CNT_MASK            (0x7F << SPI_TX_FIFO_CNT_POS)
#define     SPI_RX_FIFO_CNT_POS             5
#define     SPI_RX_FIFO_CNT_MASK            (0x7F << SPI_RX_FIFO_CNT_POS)
#define     SPI_CAM_SOF_INT_POS             7
#define     SPI_CAM_SOF_INT_MASK            (1 << SPI_CAM_SOF_INT_POS)
#define     SPI_CAM_EOF_INT_POS             8
#define     SPI_CAM_EOF_INT_MASK            (1 << SPI_CAM_EOF_INT_POS)
#define     SPI_CAM_SOF_INT_EN_MASK         (1 << 7)
#define     SPI_CAM_EOF_INT_EN_MASK         (1 << 8)
#define     SPI_CAM_RXOVERRUN_INT_POS             0
#define     SPI_CAM_RXOVERRUN_INT_MASK            (1 << SPI_CAM_RXOVERRUN_INT_POS)
#define     SPI_CAM_RXOVERRUN_INT_EN_MASK         (1 << 0)

typedef struct 
{
    __IO uint32_t verReg;
    __IO uint32_t comCtrl;
    __IO uint32_t fmtCtrl;
    __IO uint32_t dataReg;
    __IO uint32_t fifoCtrl;
    __IO uint32_t fifoSr;
    __IO uint32_t intrEn;
    __IO uint32_t intrSr;
    __IO uint32_t timing;
    __IO uint32_t resv[3];
    __IO uint32_t syncCode;
    __IO uint32_t debug;
    __IO uint32_t packetSize;
}SpiReg;


static inline void SPI_Enable(SpiReg *reg)
{
    uint32_t reg_val = 0;

    reg_val = READ_REG(reg->comCtrl);
    reg_val |= (SPI_SSPE_MASK);
    WRITE_REG(reg->comCtrl, reg_val);
    while(!(READ_REG(reg->comCtrl) & SPI_SSPE_BACK_MASK));
}

static inline void SPI_Disable(SpiReg *reg)
{
    uint32_t reg_val = 0;

    reg_val = READ_REG(reg->comCtrl);
    reg_val &= (~SPI_SSPE_MASK);
    WRITE_REG(reg->comCtrl, reg_val);

    while((READ_REG(reg->comCtrl) & SPI_SSPE_BACK_MASK));
}

static inline void SPI_RstCamFifo(SpiReg *reg)
{
    uint32_t reg_val = 0;
    volatile uint8_t i = 0;

    reg_val = READ_REG(reg->syncCode);
    reg_val &= (~SPI_RST_CAM_FIFO_MASK);
    WRITE_REG(reg->syncCode, reg_val);

    for(i = 0; i < 3; i++)
        asm volatile("nop");

    reg_val = READ_REG(reg->syncCode);
    reg_val |= (SPI_RST_CAM_FIFO_MASK);
    WRITE_REG(reg->syncCode, reg_val);
}

static inline uint8_t SPI_GetTxFifoFreeCnt(SpiReg *reg)
{
    uint32_t reg_val = 0;

    reg_val = READ_REG(reg->fifoSr);

    return ((reg_val & SPI_TX_FIFO_CNT_MASK) >> SPI_TX_FIFO_CNT_POS);
}

static inline uint8_t SPI_GetRxFifoAvaliableCnt(SpiReg *reg)
{
    uint32_t reg_val = 0;

    reg_val = READ_REG(reg->fifoSr);

    return ((reg_val & SPI_RX_FIFO_CNT_MASK) >> SPI_RX_FIFO_CNT_POS);
}

static inline uint32_t SPI_GetDataFifoAddr(SpiReg *reg)
{
    return (uint32_t)&reg->dataReg;
}

static inline void SPI_ClrCamAllInterrupt(SpiReg *reg)
{
    WRITE_REG(reg->intrSr, READ_REG(reg->intrSr) | SPI_CAM_SOF_INT_MASK | SPI_CAM_EOF_INT_MASK | SPI_CAM_RXOVERRUN_INT_MASK);
}

static inline bool SPI_IsCamRxOverrunInterrupt(SpiReg *reg)
{
    return (reg->intrSr & SPI_CAM_RXOVERRUN_INT_MASK) >> SPI_CAM_RXOVERRUN_INT_POS;
}

static inline bool SPI_IsCamEOFInterrupt(SpiReg *reg)
{
    return (reg->intrSr & SPI_CAM_EOF_INT_MASK) >> SPI_CAM_EOF_INT_POS;
}

static inline bool SPI_IsCamSOFInterrupt(SpiReg *reg)
{
    return (reg->intrSr & SPI_CAM_SOF_INT_MASK) >> SPI_CAM_SOF_INT_POS;
}

static inline void SPI_ClrCamEOFInterrupt(SpiReg *reg)
{
    WRITE_REG(reg->intrSr, READ_REG(reg->intrSr) | SPI_CAM_EOF_INT_MASK);
}

static inline void SPI_ClrCamSOFInterrupt(SpiReg *reg)
{
    WRITE_REG(reg->intrSr, READ_REG(reg->intrSr) | SPI_CAM_SOF_INT_MASK);
}

static inline void SPI_ClrCamRxOverrunInterrupt(SpiReg *reg)
{
    WRITE_REG(reg->intrSr, READ_REG(reg->intrSr) | SPI_CAM_RXOVERRUN_INT_MASK);
}

static inline void SPI_EnableCamSOFInterrupt(SpiReg *reg, bool enable)
{
    if(enable)
    {
        WRITE_REG(reg->intrEn, READ_REG(reg->intrEn) | SPI_CAM_SOF_INT_EN_MASK);
    }
    else
    {
        WRITE_REG(reg->intrEn, READ_REG(reg->intrEn) & (~SPI_CAM_SOF_INT_EN_MASK));
    }
}

static inline void SPI_EnableRxOverrunInterrupt(SpiReg *reg, bool enable)
{
    if(enable)
    {
        WRITE_REG(reg->intrEn, READ_REG(reg->intrEn) | SPI_CAM_RXOVERRUN_INT_EN_MASK);
    }
    else
    {
        WRITE_REG(reg->intrEn, READ_REG(reg->intrEn) & (~SPI_CAM_RXOVERRUN_INT_EN_MASK));
    }
}

static inline void SPI_EnableCamEOFInterrupt(SpiReg *reg, bool enable)
{
    if(enable)
    {
        WRITE_REG(reg->intrEn, READ_REG(reg->intrEn) | SPI_CAM_EOF_INT_EN_MASK);
    }
    else
    {
        WRITE_REG(reg->intrEn, READ_REG(reg->intrEn) & (~SPI_CAM_EOF_INT_EN_MASK));
    }
}


#endif
