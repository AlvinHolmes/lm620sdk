#ifndef _DRV_SSP_PRIVATE_H
#define _DRV_SSP_PRIVATE_H

#include <drv_common.h>

/***********************************************************************************************/

//                                       寄存器定义

/***********************************************************************************************/

/*** Bit definition for [version] register(0x00000000) ****/
#define SSP_VER_H_Pos                     (24)
#define SSP_VER_H_Msk                     (0xFFUL << SSP_VER_H_Pos)
#define SSP_VER_H                         SSP_VER_H_Msk                     /*!<主版本号 */
#define SSP_VER_L_Pos                     (16)
#define SSP_VER_L_Msk                     (0xFFUL << SSP_VER_L_Pos)
#define SSP_VER_L                         SSP_VER_L_Msk                     /*!<次版本号 */


/*** Bit definition for [comCtrl] register(0x00000004) ****/
#define SSP_SSPE_BACK_Pos                 (4)
#define SSP_SSPE_BACK_Msk                 (0x1UL << SSP_SSPE_BACK_Pos)
#define SSP_SSPE_BACK                     SSP_SSPE_BACK_Msk                 /*!<SSPE同步信号 */
#define SSP_MS_Pos                        (2)
#define SSP_MS_Msk                        (0x1UL << SSP_MS_Pos)
#define SSP_MS                            SSP_MS_Msk                        /*!<主从模式选择：1-master 0-slave */
#define SSP_SSPE_Pos                      (1)
#define SSP_SSPE_Msk                      (0x1UL << SSP_SSPE_Pos)
#define SSP_SSPE                          SSP_SSPE_Msk                      /*!<SSP使能 */
#define SSP_LBM_Pos                       (0)
#define SSP_LBM_Msk                       (0x1UL << SSP_LBM_Pos)
#define SSP_LBM                           SSP_LBM_Msk                       /*!<回环模式使能 */


/*** Bit definition for [fmtCtrl] register(0x00000008) ****/
#define SSP_DSS_Pos                       (4)
#define SSP_DSS_Msk                       (0x1FUL << SSP_DSS_Pos)
#define SSP_DSS                           SSP_DSS_Msk                       /*!<数据大小 */
#define SSP_PHA_Pos                       (3)
#define SSP_PHA_Msk                       (0x1UL << SSP_PHA_Pos)
#define SSP_PHA                           SSP_PHA_Msk                       /*!<PHA(phase)SSP serial clk phase */
#define SSP_POL_Pos                       (2)
#define SSP_POL_Msk                       (0x1UL << SSP_POL_Pos)
#define SSP_POL                           SSP_POL_Msk                       /*!<POL(polarity)SSP serial clk polarity */
#define SSP_FRF_Pos                       (0)
#define SSP_FRF_Msk                       (0x3UL << SSP_FRF_Pos)
#define SSP_FRF                           SSP_FRF_Msk                       /*!<FRF(frame format) */


/*** Bit definition for [data] register(0x0000000c) ****/
#define SSP_DATA_Pos                      (0)
#define SSP_DATA_Msk                      (0xFFFFFFFFUL << SSP_DATA_Pos)
#define SSP_DATA                          SSP_DATA_Msk                      /*!<数据寄存器 */


/*** Bit definition for [fifoCtrl] register(0x00000010) ****/
#define SSP_TX_FIFO_THRES_Pos             (8)
#define SSP_TX_FIFO_THRES_Msk             (0xFUL << SSP_TX_FIFO_THRES_Pos)
#define SSP_TX_FIFO_THRES                 SSP_TX_FIFO_THRES_Msk             /*!<设置TX FIFO阈值 */
#define SSP_RX_FIFO_THRES_Pos             (4)
#define SSP_RX_FIFO_THRES_Msk             (0xFUL << SSP_RX_FIFO_THRES_Pos)
#define SSP_RX_FIFO_THRES                 SSP_RX_FIFO_THRES_Msk             /*!<设置RX FIFO阈值 */
#define SSP_TX_DMA_EN_Pos                 (3)
#define SSP_TX_DMA_EN_Msk                 (0x1UL << SSP_TX_DMA_EN_Pos)
#define SSP_TX_DMA_EN                     SSP_TX_DMA_EN_Msk                 /*!<TX DMA使能 */
#define SSP_RX_DMA_EN_Pos                 (2)
#define SSP_RX_DMA_EN_Msk                 (0x1UL << SSP_RX_DMA_EN_Pos)
#define SSP_RX_DMA_EN                     SSP_RX_DMA_EN_Msk                 /*!<RX DMA使能 */


/*** Bit definition for [fifoSr] register(0x00000014) ****/
#define SSP_TX_FIFO_CNTR_Pos              (10)
#define SSP_TX_FIFO_CNTR_Msk              (0x1FUL << SSP_TX_FIFO_CNTR_Pos)
#define SSP_TX_FIFO_CNTR                  SSP_TX_FIFO_CNTR_Msk              /*!<TX FIFO中剩余的空位 */
#define SSP_RX_FIFO_CNTR_Pos              (5)
#define SSP_RX_FIFO_CNTR_Msk              (0x1FUL << SSP_RX_FIFO_CNTR_Pos)
#define SSP_RX_FIFO_CNTR                  SSP_RX_FIFO_CNTR_Msk              /*!<RX FIFO中存在的数据 */
#define SSP_BUSY_Pos                      (4)
#define SSP_BUSY_Msk                      (0x1UL << SSP_BUSY_Pos)
#define SSP_BUSY                          SSP_BUSY_Msk                      /*!<BUSY */
#define SSP_TX_FIFO_EMPTY_Pos             (3)
#define SSP_TX_FIFO_EMPTY_Msk             (0x1UL << SSP_TX_FIFO_EMPTY_Pos)
#define SSP_TX_FIFO_EMPTY                 SSP_TX_FIFO_EMPTY_Msk             /*!<TX FIFO是否为空 */
#define SSP_RX_FIFO_FULL_Pos              (2)
#define SSP_RX_FIFO_FULL_Msk              (0x1UL << SSP_RX_FIFO_FULL_Pos)
#define SSP_RX_FIFO_FULL                  SSP_RX_FIFO_FULL_Msk              /*!<RX FIFO是否满 */
#define SSP_TX_BEYOND_THRES_Pos           (1)
#define SSP_TX_BEYOND_THRES_Msk           (0x1UL << SSP_TX_BEYOND_THRES_Pos)
#define SSP_TX_BEYOND_THRES               SSP_TX_BEYOND_THRES_Msk           /*!<TX FIFO中的数据是否超出设置的阈值 */
#define SSP_RX_BEYOND_THRES_Pos           (0)
#define SSP_RX_BEYOND_THRES_Msk           (0x1UL << SSP_RX_BEYOND_THRES_Pos)
#define SSP_RX_BEYOND_THRES               SSP_RX_BEYOND_THRES_Msk           /*!<RX FIFO中的数据是否超出设置的阈值 */


/*** Bit definition for [intrEn] register(0x00000018) ****/
#define SSP_MST_EOT_IE_Pos                (6)
#define SSP_MST_EOT_IE_Msk                (0x1UL << SSP_MST_EOT_IE_Pos)
#define SSP_MST_EOT_IE                    SSP_MST_EOT_IE_Msk                /*!<Master传输结束中断使能 */
#define SSP_TX_THRES_IE_Pos               (5)
#define SSP_TX_THRES_IE_Msk               (0x1UL << SSP_TX_THRES_IE_Pos)
#define SSP_TX_THRES_IE                   SSP_TX_THRES_IE_Msk               /*!<TX FIFO阈值中断使能 */
#define SSP_RX_THRES_IE_Pos               (4)
#define SSP_RX_THRES_IE_Msk               (0x1UL << SSP_RX_THRES_IE_Pos)
#define SSP_RX_THRES_IE                   SSP_RX_THRES_IE_Msk               /*!<RX FIFO阈值中断使能 */
#define SSP_TX_EMPTY_IE_Pos               (3)
#define SSP_TX_EMPTY_IE_Msk               (0x1UL << SSP_TX_EMPTY_IE_Pos)
#define SSP_TX_EMPTY_IE                   SSP_TX_EMPTY_IE_Msk               /*!<TX FIFO空中断使能 */
#define SSP_RX_FULL_IE_Pos                (2)
#define SSP_RX_FULL_IE_Msk                (0x1UL << SSP_RX_FULL_IE_Pos)
#define SSP_RX_FULL_IE                    SSP_RX_FULL_IE_Msk                /*!<RX FIFO满中断使能 */
#define SSP_TX_UNDERRUN_IE_Pos            (1)
#define SSP_TX_UNDERRUN_IE_Msk            (0x1UL << SSP_TX_UNDERRUN_IE_Pos)
#define SSP_TX_UNDERRUN_IE                SSP_TX_UNDERRUN_IE_Msk            /*!<TX UNDERRUN中断使能 */
#define SSP_RX_OVERRUN_IE_Pos             (0)
#define SSP_RX_OVERRUN_IE_Msk             (0x1UL << SSP_RX_OVERRUN_IE_Pos)
#define SSP_RX_OVERRUN_IE                 SSP_RX_OVERRUN_IE_Msk             /*!<RX OVERRUN中断使能 */


/*** Bit definition for [intrSr] register(0x0000001c) ****/
#define SSP_MST_EOT_INTR_Pos              (6)
#define SSP_MST_EOT_INTR_Msk              (0x1UL << SSP_MST_EOT_INTR_Pos)
#define SSP_MST_EOT_INTR                  SSP_MST_EOT_INTR_Msk              /*!<Master传输结束中断状态/清除 */
#define SSP_TX_THRES_INTR_Pos             (5)
#define SSP_TX_THRES_INTR_Msk             (0x1UL << SSP_TX_THRES_INTR_Pos)
#define SSP_TX_THRES_INTR                 SSP_TX_THRES_INTR_Msk             /*!<TX FIFO阈值中断状态/清除 */
#define SSP_RX_THRES_INTR_Pos             (4)
#define SSP_RX_THRES_INTR_Msk             (0x1UL << SSP_RX_THRES_INTR_Pos)
#define SSP_RX_THRES_INTR                 SSP_RX_THRES_INTR_Msk             /*!<RX FIFO阈值中断状态/清除 */
#define SSP_TX_EMPTY_INTR_Pos             (3)
#define SSP_TX_EMPTY_INTR_Msk             (0x1UL << SSP_TX_EMPTY_INTR_Pos)
#define SSP_TX_EMPTY_INTR                 SSP_TX_EMPTY_INTR_Msk             /*!<TX FIFO空中断状态/清除 */
#define SSP_RX_FULL_INTR_Pos              (2)
#define SSP_RX_FULL_INTR_Msk              (0x1UL << SSP_RX_FULL_INTR_Pos)
#define SSP_RX_FULL_INTR                  SSP_RX_FULL_INTR_Msk              /*!<RX FIFO满中断状态/清除 */
#define SSP_TX_UNDERRUN_INTR_Pos          (1)
#define SSP_TX_UNDERRUN_INTR_Msk          (0x1UL << SSP_TX_UNDERRUN_INTR_Pos)
#define SSP_TX_UNDERRUN_INTR              SSP_TX_UNDERRUN_INTR_Msk          /*!<TX UNDERRUN中断状态/清除 */
#define SSP_RX_OVERRUN_INTR_Pos           (0)
#define SSP_RX_OVERRUN_INTR_Msk           (0x1UL << SSP_RX_OVERRUN_INTR_Pos)
#define SSP_RX_OVERRUN_INTR               SSP_RX_OVERRUN_INTR_Msk           /*!<RX OVERRUN中断状态/清除 */


/*** Bit definition for [timing] register(0x00000020) ****/
#define SSP_T_CS_DESEL_Pos                (0)
#define SSP_T_CS_DESEL_Msk                (0xFUL << SSP_T_CS_DESEL_Pos)
#define SSP_T_CS_DESEL                    SSP_T_CS_DESEL_Msk                /*!<SSP Transfer Gap */




/**
 * @brief  REG_Ssp
 */
typedef struct {
    __IO uint32_t  version;                          /*!< Offset 0x00000000 */
    __IO uint32_t  comCtrl;                          /*!< Offset 0x00000004 */
    __IO uint32_t  fmtCtrl;                          /*!< Offset 0x00000008 */
    __IO uint32_t  data;                             /*!< Offset 0x0000000c */
    __IO uint32_t  fifoCtrl;                         /*!< Offset 0x00000010 */
    __IO uint32_t  fifoSr;                           /*!< Offset 0x00000014 */
    __IO uint32_t  intrEn;                           /*!< Offset 0x00000018 */
    __IO uint32_t  intrSr;                           /*!< Offset 0x0000001c */
    __IO uint32_t  timing;                           /*!< Offset 0x00000020 */
} REG_Ssp;

#define SSP_TIMEOUT             (100000)


#define SSP_RX_FIFO_DEPTH       (16)
#define SSP_TX_FIFO_DEPTH       (16)


static inline bool SSP_SspeCrossBackGet(REG_Ssp *regs)
{
    return (READ_BIT(regs->comCtrl, SSP_SSPE_BACK)) ? true : false;
}

static inline void SSP_MasterEnable(REG_Ssp *regs)
{
    CLEAR_BIT(regs->comCtrl, SSP_MS);
}

static inline void SSP_SlaveEnable(REG_Ssp *regs)
{
    SET_BIT(regs->comCtrl, SSP_MS);
}

static inline void SSP_Enable(REG_Ssp *regs)
{
    volatile uint32_t timeout = SSP_TIMEOUT;
    SET_BIT(regs->comCtrl, SSP_SSPE);
    while (SSP_SspeCrossBackGet(regs) == false){
        SET_BIT(regs->comCtrl, SSP_SSPE);
        OS_ASSERT(timeout--);
    }
}

static inline void SSP_Disable(REG_Ssp *regs)
{
    volatile uint32_t timeout = SSP_TIMEOUT;
    CLEAR_BIT(regs->comCtrl, SSP_SSPE);
    while (SSP_SspeCrossBackGet(regs) == true){
        CLEAR_BIT(regs->comCtrl, SSP_SSPE);
        OS_ASSERT(timeout--);
    }
}

static inline void SSP_LbmEnable(REG_Ssp *regs)
{
    SET_BIT(regs->comCtrl, SSP_LBM);
}

typedef enum{
    SSP_MOTOROLA_SPI_FORMAT = 0,
    SSP_TI_SSP_FORMAT,
    SSP_ISI_SPI_FORMAT,
}SSP_FrameFormat;

static inline void SSP_FrameFormatSet(REG_Ssp *regs, SSP_FrameFormat format)
{
    MODIFY_REG(regs->fmtCtrl, SSP_FRF, format);
}

typedef enum{
    SSP_POLARITY_LOW = 0,
    SSP_POLARITY_HIGH,
}SSP_Polarity;

static inline void SSP_PolaritySet(REG_Ssp *regs, SSP_Polarity pol)
{
    if(SSP_POLARITY_HIGH == pol){
        SET_BIT(regs->fmtCtrl, SSP_POL);
    } else {
        CLEAR_BIT(regs->fmtCtrl, SSP_POL);
    }
}

typedef enum {
    SSP_FIRST_PHASE = 0,
    SSP_SECOND_PHASE,
}SSP_Phase;

static inline void SSP_PhaseSet(REG_Ssp *regs, SSP_Phase pha)
{
    if(SSP_SECOND_PHASE == pha){
        SET_BIT(regs->fmtCtrl, SSP_PHA);
    } else {
        CLEAR_BIT(regs->fmtCtrl, SSP_PHA);
    }
}

typedef enum{
    SSP_DATA_WIDTH_8BIT = 7,
    SSP_DATA_WIDTH_16BIT = 15,
    SSP_DATA_WIDTH_32BIT = 31,
}SSP_DataWidth;

static inline void SSP_FrameSizeSet(REG_Ssp *regs, uint8_t width)
{
    if(width <= 4){
        width = 4;
    }

    MODIFY_REG(regs->fmtCtrl, SSP_DSS, (width - 1) << SSP_DSS_Pos);
}

static inline void SSP_WriteFifoData(REG_Ssp *regs, uint32_t data)
{
    WRITE_REG(regs->data, data);
}

static inline int32_t SSP_WriteFifo(REG_Ssp *regs, SSP_DataWidth width, void *buf, uint32_t num)
{
    if(num == 0){
        return -1;
    }

    switch (width){
        case SSP_DATA_WIDTH_8BIT:{
            uint8_t *dataWidth8 = (uint8_t *)buf;
            do{
                // SSP_PrintDebug("write fifo %d", *dataWidth8);
                WRITE_REG(regs->data, *(dataWidth8++));
            } while (--num != 0);
            break;
        }

        case SSP_DATA_WIDTH_16BIT:{
            uint16_t *dataWidth16 = (uint16_t *)buf;
            do{
                // SSP_PrintDebug("write fifo 0x%x", *dataWidth16);
                WRITE_REG(regs->data, *dataWidth16++);
            } while (--num != 0);
            break;
        }

        case SSP_DATA_WIDTH_32BIT:{
            uint32_t *dataWidth32 = (uint32_t *)buf;
            do{
                // SSP_PrintDebug("write fifo 0x%x", *dataWidth32);
                WRITE_REG(regs->data, *dataWidth32++);
            } while (--num != 0);
            break;
        }
            
        
        default:
            break;
    }

    return 0;
}

static uint32_t SSP_ReadFifoData(REG_Ssp *regs)
{
    return READ_REG(regs->data);
}

static inline int32_t SSP_ReadFifo(REG_Ssp *regs, SSP_DataWidth width, void *buf, uint32_t num)
{
    if(!buf || !num){
        // SSP_PrintError("SSP Buffer is NULL");
        return -1;
    }

    if(!READ_BIT(regs->fifoCtrl, SSP_RX_FIFO_CNTR)){
        // SSP_PrintError("SSP Fifo is NULL");
        return -1;
    }

    switch (width){
        case SSP_DATA_WIDTH_8BIT:{
            uint8_t *dataWidth8 = (uint8_t *)buf;
            do{
                *dataWidth8++ = READ_REG(regs->data);
                // SSP_PrintDebug("read fifo 0x%x", *(dataWidth8 - 1));
            } while (--num);
            break;
        }
        
        case SSP_DATA_WIDTH_16BIT:{
            uint16_t *dataWidth16 = (uint16_t *)buf;
            do{
                *dataWidth16++ = READ_REG(regs->data);
                // SSP_PrintDebug("read fifo 0x%x", *(dataWidth16 - 1));
            } while (--num);
            break;
        }

        case SSP_DATA_WIDTH_32BIT:{
            uint32_t *dataWidth32 = (uint32_t *)buf;
            do{
                *dataWidth32++ = READ_REG(regs->data);
                // SSP_PrintDebug("read fifo 0x%x", *(dataWidth32 - 1));
            } while (--num);
            break;
        }
            
        default:
            break;
    }

    return 0;
}

static inline int32_t SSP_WriteFifoDummy(REG_Ssp *regs, uint32_t num, uint32_t dummy)
{
    if(num == 0){
        return -1;
    }

    do{
        WRITE_REG(regs->data, dummy);
    } while (--num != 0);

    return 0;
}

static inline int32_t SSP_ReadFifoDummy(REG_Ssp *regs, uint32_t num)
{
    do
    {
        READ_REG(regs->data);
    } while (--num);

    return 0;
}

static inline void SSP_TxFifoThresSet(REG_Ssp *regs, uint8_t thres)
{
    MODIFY_REG(regs->fifoCtrl, SSP_TX_FIFO_THRES, (thres - 1) << SSP_TX_FIFO_THRES_Pos);
}

static inline void SSP_RxFifoThresSet(REG_Ssp *regs, uint8_t thres)
{
    MODIFY_REG(regs->fifoCtrl, SSP_RX_FIFO_THRES, (thres - 1) << SSP_RX_FIFO_THRES_Pos);
}

static inline void SSP_TxDmaEnable(REG_Ssp *regs)
{
    SET_BIT(regs->fifoCtrl, SSP_TX_DMA_EN);
}

static inline void SSP_TxDmaDisable(REG_Ssp *regs)
{
    CLEAR_BIT(regs->fifoCtrl, SSP_TX_DMA_EN);
}

static inline void SSP_RxDmaEnable(REG_Ssp *regs)
{
    SET_BIT(regs->fifoCtrl, SSP_RX_DMA_EN);
}

static inline void SSP_RxDmaDisable(REG_Ssp *regs)
{
    CLEAR_BIT(regs->fifoCtrl, SSP_RX_DMA_EN);
}

static inline uint8_t SSP_TxFifoVacancyGet(REG_Ssp *regs)
{
    return READ_BIT(regs->fifoSr, SSP_TX_FIFO_CNTR) >> SSP_TX_FIFO_CNTR_Pos;
}

static inline uint8_t SSP_RxFifoVacancyGet(REG_Ssp *regs)
{
    return SSP_RX_FIFO_DEPTH - (READ_BIT(regs->fifoSr, SSP_RX_FIFO_CNTR) >> SSP_RX_FIFO_CNTR_Pos);
}

static inline uint8_t SSP_RxFifoFillGet(REG_Ssp *regs)
{
    return READ_BIT(regs->fifoSr, SSP_RX_FIFO_CNTR) >> SSP_RX_FIFO_CNTR_Pos;
}

static inline bool SSP_IsBusy(REG_Ssp *regs)
{
    return READ_BIT(regs->fifoSr, SSP_BUSY) ? true : false;
}

static inline bool SSP_TxFifoIsEmpty(REG_Ssp *regs)
{
    return READ_BIT(regs->fifoSr, SSP_TX_FIFO_EMPTY) ? true : false;
}

static inline bool SSP_RxFifoIsFull(REG_Ssp *regs)
{
    return READ_BIT(regs->fifoSr, SSP_RX_FIFO_FULL) ? true : false;
}

static inline bool SSP_TxFifoIsBeyondThres(REG_Ssp *regs)
{
    return READ_BIT(regs->fifoSr, SSP_TX_BEYOND_THRES) ? true : false;
}

static inline bool SSP_RxFifoIsBeyondThres(REG_Ssp *regs)
{
    return READ_BIT(regs->fifoSr, SSP_RX_BEYOND_THRES) ? true : false;
}

static inline void SSP_InterruptRxEndEnable(REG_Ssp *regs)
{
    MODIFY_REG(regs->intrEn, SSP_MST_EOT_IE, 1);
}

static inline void SSP_InterruptEnableAll(REG_Ssp *regs)
{
    WRITE_REG(regs->intrEn, SSP_MST_EOT_IE | SSP_TX_THRES_IE | SSP_RX_THRES_IE | SSP_TX_EMPTY_IE | SSP_RX_FULL_IE | SSP_TX_UNDERRUN_IE | SSP_RX_OVERRUN_IE);
}

static inline void SSP_InterruptDisableAll(REG_Ssp *regs)
{
    WRITE_REG(regs->intrEn, 0);
    SET_BIT(regs->intrEn, SSP_MST_EOT_IE);
    // WRITE_REG(regs->intrEn, SSP_TX_UNDERRUN_IE | SSP_RX_OVERRUN_IE);
    // WRITE_REG(regs->intrEn, SSP_RX_OVERRUN_IE);
}

static inline void SSP_InterruptClearAll(REG_Ssp *regs)
{
    WRITE_REG(regs->intrSr, SSP_MST_EOT_INTR | SSP_TX_THRES_INTR | SSP_RX_THRES_INTR | SSP_TX_EMPTY_INTR | SSP_RX_FULL_INTR | SSP_TX_UNDERRUN_INTR | SSP_RX_OVERRUN_INTR);
}

static inline void SSP_InterruptClear(REG_Ssp *regs, uint32_t status)
{
    WRITE_REG(regs->intrSr, status);
}

static inline uint32_t SSP_InterruptStatusGet(REG_Ssp *regs)
{
    return READ_REG(regs->intrSr);
}


#endif