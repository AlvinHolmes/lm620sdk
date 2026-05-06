#ifndef _DRV_SPI_PRIVATE_H
#define _DRV_SPI_PRIVATE_H

#include <drv_common.h>

#define SPI_TX_FIFO_DEPTH           (64)
#define SPI_RX_FIFO_DEPTH           (64)

typedef enum{
    SPI_TX_ONLY_MODE = 0,
    SPI_RX_ONLY_MODE,
    SPI_RTX_BOTH,
}SPI_RtxMode;

typedef enum{
    SPI_POLARITY_LOW = 0,
    SPI_POLARITY_HIGH,
}SPI_Polarity;

typedef enum {
    SPI_FIRST_PHASE = 0,
    SPI_SECOND_PHASE,
}SPI_Phase;

static inline void SPI_SlvSwSsnEnable(REG_Spi *regs)
{
    SET_BIT(regs->protocol, SPI_SLV_SW_SSN_EN);
}

static inline void SPI_SlvSwSsnDisable(REG_Spi *regs)
{
    CLEAR_BIT(regs->protocol, SPI_SLV_SW_SSN_EN);
}

static inline void SPI_SlvSwSsnSetHigh(REG_Spi *regs)
{
    SET_BIT(regs->protocol, SPI_SLV_SW_SSN);
}

static inline void SPI_SlvSwSsnSetLow(REG_Spi *regs)
{
    CLEAR_BIT(regs->protocol, SPI_SLV_SW_SSN);
}

static inline void SPI_UnBiDirPinMode(REG_Spi *regs)
{
    CLEAR_BIT(regs->protocol, SPI_PIN_MODE);
}

static inline void SPI_BiDirPinMode(REG_Spi *regs)
{
    SET_BIT(regs->protocol, SPI_PIN_MODE);
}

static inline void SPI_WordSizeSet(REG_Spi *regs, uint8_t width)
{
    if(width == 0){
        MODIFY_REG(regs->protocol, SPI_WORD_SIZE, width << SPI_WORD_SIZE_Pos);
    }else{
        MODIFY_REG(regs->protocol, SPI_WORD_SIZE, (width - 1) << SPI_WORD_SIZE_Pos);
    }
}

static inline void SPI_DevReadySignalDulMode(REG_Spi *regs)
{
    CLEAR_BIT(regs->protocol, SPI_DRDY_MOD);
}

static inline void SPI_DevReadySignalSingleMode(REG_Spi *regs)
{
    SET_BIT(regs->protocol, SPI_DRDY_MOD);
}

static inline void SPI_HardFlowCtrlEnable(REG_Spi *regs)
{
    SET_BIT(regs->protocol, SPI_DRDY_EN);
}

static inline void SPI_HardFlowCtrlDisable(REG_Spi *regs)
{
    CLEAR_BIT(regs->protocol, SPI_DRDY_EN);
}

static inline void SPI_SlaveSelect(REG_Spi *regs, uint8_t idx)
{
    MODIFY_REG(regs->protocol, SPI_SLV_SEL, idx << SPI_SLV_SEL_Pos);
}

static inline void SPI_RtxModeCfg(REG_Spi *regs, SPI_RtxMode mode)
{
    MODIFY_REG(regs->protocol, SPI_RTX_CTRL, mode << SPI_RTX_CTRL_Pos);
}

static inline void SPI_SpeculatEnable(REG_Spi *regs)
{
    SET_BIT(regs->protocol, SPI_SPECULAT);
}

static inline void SPI_SpeculatDisable(REG_Spi *regs)
{
    SET_BIT(regs->protocol, SPI_SPECULAT);
}

static inline void SPI_ClkNormalMode(REG_Spi *regs)
{
    CLEAR_BIT(regs->protocol, SPI_SCLK_STYLE);
}

static inline void SPI_ClkExtraMode(REG_Spi *regs)
{
    SET_BIT(regs->protocol, SPI_SCLK_STYLE);
}

static inline void SPI_PhaseSet(REG_Spi *regs, SPI_Phase pha)
{
    if(pha == SPI_FIRST_PHASE){
        CLEAR_BIT(regs->protocol, SPI_CHPA);
    }else{
        SET_BIT(regs->protocol, SPI_CHPA);
    }
}

static inline void SPI_PolaritySet(REG_Spi *regs, SPI_Polarity pol)
{
    if(pol == SPI_POLARITY_LOW){
        CLEAR_BIT(regs->protocol, SPI_CPOL);
    }else{
        SET_BIT(regs->protocol, SPI_CPOL);
    }
}

static inline void SPI_MasterEnable(REG_Spi *regs)
{
    CLEAR_BIT(regs->protocol, SPI_MS_SEL);
}

static inline void SPI_SlaveEnable(REG_Spi *regs)
{
    SET_BIT(regs->protocol, SPI_MS_SEL);
}

static inline void SPI_TxLengthSet(REG_Spi *regs, uint16_t len)
{
    WRITE_REG(regs->txCtrl, len);
}

static inline void SPI_RxLengthRegSet(REG_Spi *regs, uint16_t len)
{
    MODIFY_REG(regs->rxCtrl, SPI_RX_QTY, len << SPI_RX_QTY_Pos);
}

static inline void SPI_RxLengthAutoEnable(REG_Spi *regs)
{
    SET_BIT(regs->rxCtrl, SPI_RX_QTY_MOD);
}

static inline void SPI_RxLengthAutoDisable(REG_Spi *regs)
{
    CLEAR_BIT(regs->rxCtrl, SPI_RX_QTY_MOD);
}

static inline uint16_t SPI_SlaveRxCnt(REG_Spi *regs)
{
    return READ_REG(regs->slvRxCnt);
}

static inline void SPI_OutReflectionEnable(REG_Spi *regs)
{
    SET_BIT(regs->crcCtrl0, SPI_OUT_REF);
}

static inline void SPI_OutReflectionDisable(REG_Spi *regs)
{
    CLEAR_BIT(regs->crcCtrl0, SPI_OUT_REF);
}

static inline void SPI_InReflectionEnable(REG_Spi *regs)
{
    SET_BIT(regs->crcCtrl0, SPI_IN_REF);
}

static inline void SPI_InReflectionDisable(REG_Spi *regs)
{
    CLEAR_BIT(regs->crcCtrl0, SPI_IN_REF);
}

static inline void SPI_RxCrc16Enalbe(REG_Spi *regs)
{
    SET_BIT(regs->crcCtrl0, SPI_RX_CRC16_EN);
}

static inline void SPI_RxCrc16Disalbe(REG_Spi *regs)
{
    CLEAR_BIT(regs->crcCtrl0, SPI_RX_CRC16_EN);
}

static inline void SPI_TxCrc16Enalbe(REG_Spi *regs)
{
    SET_BIT(regs->crcCtrl0, SPI_TX_CRC16_EN);
}

static inline void SPI_TxCrc16Disable(REG_Spi *regs)
{
    CLEAR_BIT(regs->crcCtrl0, SPI_TX_CRC16_EN);
}

static inline void SPI_CrcOutXorValSet(REG_Spi *regs, uint16_t val)
{
    MODIFY_REG(regs->crcCtrl1, SPI_OUT_XOR, val << SPI_OUT_XOR_Pos);
}

static inline void SPI_CrcInitValSet(REG_Spi *regs, uint16_t val)
{
    MODIFY_REG(regs->crcCtrl1, SPI_CRC_INIT, val << SPI_CRC_INIT_Pos);
}

static inline void SPI_Crc16ValSet(REG_Spi *regs, uint32_t val)
{
    WRITE_REG(regs->crcCtrl1, val);
}

static inline uint16_t SPI_CrcRxStaGet(REG_Spi *regs)
{
    return READ_BIT(regs->crcSta, SPI_RX_CRC) >> SPI_RX_CRC_Pos;
}

static inline uint16_t SPI_CrcTxStaGet(REG_Spi *regs)
{
    return READ_BIT(regs->crcSta, SPI_TX_CRC);
}

static inline uint16_t SPI_RxCrcHist1(REG_Spi *regs)
{
    return READ_BIT(regs->crcDbg, SPI_RX_CRC_HIST1) >> SPI_RX_CRC_HIST1_Pos;
}

static inline uint16_t SPI_RxCrcHist0(REG_Spi *regs)
{
    return READ_BIT(regs->crcDbg, SPI_RX_CRC_HIST0);
}

static inline void SPI_WriteFifo(REG_Spi *regs, uint32_t data)
{
    WRITE_REG(regs->data, data);
}

static inline uint16_t SPI_ReadFifo(REG_Spi *regs)
{
    return READ_REG(regs->data);
}

static inline uint8_t SPI_TxVacancyGet(REG_Spi *regs)
{
    return READ_BIT(regs->fifoCsr, SPI_TXF_SPACE_CNT) >> SPI_TXF_SPACE_CNT_Pos;
}

static inline uint8_t SPI_TxValidyGet(REG_Spi *regs)
{
    return SPI_TX_FIFO_DEPTH - (READ_BIT(regs->fifoCsr, SPI_TXF_SPACE_CNT) >> SPI_TXF_SPACE_CNT_Pos);
}

static inline uint8_t SPI_RxValidGet(REG_Spi *regs)
{
    return READ_BIT(regs->fifoCsr, SPI_RXF_DAT_CNT) >> SPI_RXF_DAT_CNT_Pos;
}

static inline uint8_t SPI_RxVacancyGet(REG_Spi *regs)
{
    return SPI_RX_FIFO_DEPTH - (READ_BIT(regs->fifoCsr, SPI_RXF_DAT_CNT) >> SPI_RXF_DAT_CNT_Pos);
}

static inline bool SPI_TxFifoIsNotFull(REG_Spi *regs)
{
    return READ_BIT(regs->fifoCsr, SPI_TXF_NOT_FULL) ? true : false;
}

static inline bool SPI_RxFifoIsNotEmpty(REG_Spi *regs)
{
    return READ_BIT(regs->fifoCsr, SPI_RXF_NOT_EMPTY) ? true : false;
}

static inline void SPI_TxFifoThresSet(REG_Spi *regs, uint8_t val)
{
    MODIFY_REG(regs->fifoCsr, SPI_TXF_THRES, (val - 1) << SPI_TXF_THRES_Pos);
}

static inline void SPI_RxFifoThresSet(REG_Spi *regs, uint8_t val)
{
    MODIFY_REG(regs->fifoCsr, SPI_RXF_THRES, (val - 1) << SPI_RXF_THRES_Pos);
}

static inline void SPI_TxRxFifoClear(REG_Spi *regs)
{
    SET_BIT(regs->fifoCsr, SPI_TXF_CLR | SPI_RXF_CLR);
    CLEAR_BIT(regs->fifoCsr, SPI_TXF_CLR | SPI_RXF_CLR);
}

static inline void SPI_TxRxDmaEnable(REG_Spi *regs)
{
    SET_BIT(regs->fifoCsr, SPI_TX_DMA_EN | SPI_RX_DMA_EN);
}

static inline void SPI_TxRxDmaDisable(REG_Spi *regs)
{
    CLEAR_BIT(regs->fifoCsr, SPI_TX_DMA_EN | SPI_RX_DMA_EN);
}

static inline void SPI_TxOnlyDmaEnable(REG_Spi *regs)
{
    SET_BIT(regs->fifoCsr, SPI_TX_DMA_EN);
    CLEAR_BIT(regs->fifoCsr, SPI_RX_DMA_EN);
}

static inline void SPI_RxOnlyDmaEnable(REG_Spi *regs)
{
    SET_BIT(regs->fifoCsr, SPI_RX_DMA_EN);
    CLEAR_BIT(regs->fifoCsr, SPI_TX_DMA_EN);
}

// static inline void SPI_TxRxThresIntrEnable(REG_Spi *regs)
// {
//     SET_BIT(regs->intrEn, SPI_TXF_THRES_IE | SPI_RXF_THRES_IE);
// }

static inline void SPI_AllIntrEnable(REG_Spi *regs)
{
    SET_BIT(regs->intrEn, SPI_TXF_THRES_IE | SPI_RXF_THRES_IE);
    SET_BIT(regs->intrEn2, SPI_SLV_DONE_IE | SPI_MST_DONE_IE | SPI_TXF_UNDERRUN_IE | SPI_RXF_OVERRUN_IE);
}

static inline void SPI_AllIntrDisable(REG_Spi *regs)
{
    CLEAR_BIT(regs->intrEn, SPI_TXF_THRES_IE | SPI_RXF_THRES_IE);
    CLEAR_BIT(regs->intrEn2, SPI_SLV_DONE_IE | SPI_MST_DONE_IE | SPI_TXF_UNDERRUN_IE | SPI_RXF_OVERRUN_IE);
}

static inline void SPI_FifoIntrEnable(REG_Spi *regs)
{
    SET_BIT(regs->intrEn, SPI_TXF_THRES_IE | SPI_RXF_THRES_IE);
}

static inline void SPI_FifoIntrDisable(REG_Spi *regs)
{
    CLEAR_BIT(regs->intrEn, SPI_TXF_THRES_IE | SPI_RXF_THRES_IE);
}

static inline void SPI_TransIntrEnable(REG_Spi *regs)
{
    SET_BIT(regs->intrEn2, SPI_SLV_DONE_IE | SPI_MST_DONE_IE | SPI_TXF_UNDERRUN_IE | SPI_RXF_OVERRUN_IE);
}

static inline void SPI_TransIntrDisable(REG_Spi *regs)
{
    CLEAR_BIT(regs->intrEn2, SPI_SLV_DONE_IE | SPI_MST_DONE_IE | SPI_TXF_UNDERRUN_IE | SPI_RXF_OVERRUN_IE);
}

static inline void SPI_AllIntrClear(REG_Spi *regs)
{
    CLEAR_BIT(regs->intrSr, SPI_TXF_THRES_INTR | SPI_RXF_THRES_INTR);
    CLEAR_BIT(regs->intrSr2, SPI_SLV_DONE_INTR | SPI_MST_DONE_INTR | SPI_TXF_UNDERRUN_INTR | SPI_RXF_OVERRUN_INTR);
}

static inline void SPI_Intr2Clr(REG_Spi *regs, uint32_t clr)
{
    WRITE_REG(regs->intrSr2, clr);
}

static inline void SPI_Start(REG_Spi *regs)
{
    SET_BIT(regs->runCtrl, SPI_START);
}

static inline bool SPI_IsBusy(REG_Spi *regs)
{
    return READ_BIT(regs->runCtrl, SPI_START) ? true : false;
}

#endif
