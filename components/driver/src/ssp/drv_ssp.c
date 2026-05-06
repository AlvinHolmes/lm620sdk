/**
 ***********************************************************************************************************************
 * Copyright (c) 2022, Nanjing Innochip Technology Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        drv_ssp.c
 *
 * @brief       Implementation of ssp function.
 *
 * @par History:
 * <table>
 * <tr><th>Date                <th> Author              <th> Notes
 * <tr><td>2022-04-06    <td> ICT Team           <td> First version
 * </table>
 *
 */

#include <drv_ssp.h>
#include <string.h>
#include <os_def.h>
#include <os_hw.h>
#include <os.h>
#include <os_debug.h>
#include "drv_ssp_private.h"

#define SSP_MIN(a, b)           ((a) < (b) ? (a) : (b))

#define CACHE_ALIGN_DOWN(size, align) ((size) & ~((align)-1))

#define SSP_ALIGN_RIGHT(addr)       ((((uint32_t)(addr)) + (32) - 1) & ~((32) - 1))
#define SSP_ALIGN_LEFT(addr)        (((uint32_t)(addr)) & ~((32) - 1))
#define SSP_IS_ALIGNED(addr)        (!(((uint32_t)(addr)) & (32 - 1)))

#define SSP_RX_FIFO_THRES_DEF       (4)
#define SSP_RX_DMA_THRES            (DMA_BLEN_4)

#define SSP_TX_FIFO_THRES_DEF       (8)
#define SSP_TX_DMA_THRES            (DMA_BLEN_8)


#define SPI_NEW_TIME                osTime_t
#define SPI_CURRENT_TIME            osGetTime()

/*********************************************************************************************************/

static int32_t SSP_CsPinMuxInit(SSP_Handle *hdl)
{
#ifdef SSP_USE_PINMUX

#endif
    return 0;
}

static int32_t SSP_CsPinDeInit(SSP_Handle *hdl)
{
#ifdef SSP_USE_PINMUX
    
#endif
    return 0;
}

static int32_t SSP_PinMuxDeInit(SSP_Handle *hdl)
{
#ifdef SSP_USE_PINMUX
    
#endif
    return 0;
}

static int32_t SSP_ClkSrcRequest(SSP_Handle *hdl, uint32_t freq)
{    
    SSP_Clock *clock    = &hdl->clock;
    uint8_t *ctrl       = &hdl->ctrl;

    if((*ctrl & SSP_CLOCK_REQ) &&(SSP_CLK_SRC_13M != freq || SSP_CLK_SRC_26M != freq || SSP_CLK_SRC_39M != freq)){
        return -1;
    }

#ifdef SSP_USE_ASIC
    if (SSP_CLK_SRC_13M == freq){
        
        SSP_PrintDebug("Request Freq 13M");
    } else if(SSP_CLK_SRC_26M == freq){
        
        SSP_PrintDebug("Request Freq 26M");
    } else if(SSP_CLK_SRC_39M == freq){
        
        SSP_PrintDebug("Request Freq 39M");
    } else if(SSP_CLK_SRC_78M == freq){
        
        SSP_PrintDebug("Request Freq 78M");
    }
    clock->freq = freq;
#else
    clock->freq = SSP_CLK_SRC_13M;
#endif

    *ctrl |= SSP_CLOCK_REQ;
    return 0;
}

static int32_t SSP_ClkSrcCfg(SSP_Handle *hdl, uint32_t freq)
{    
    SSP_Clock *clock    = &hdl->clock;
    uint8_t *ctrl       = &hdl->ctrl;

    if(!(*ctrl & SSP_CLOCK_REQ) &&(SSP_CLK_SRC_13M != freq || SSP_CLK_SRC_26M != freq)){
        return -1;
    }

#ifdef SSP_USE_ASIC
    if (SSP_CLK_SRC_13M == freq){
        SSP_PrintDebug("Config Freq 13M");
    } else{
        SSP_PrintDebug("Config Freq 26M");
    }
    clock->freq = freq;
#else
    clock->freq = SSP_CLK_SRC_13M;
#endif

    return 0;
}

static int32_t SSP_ClkSrcRelease(SSP_Handle *hdl)
{
    SSP_Clock *clock    = &hdl->clock;
    uint8_t *ctrl       = &hdl->ctrl;

    if((*ctrl & SSP_CLOCK_REQ) == 0){
        return 0;
    }

#ifdef SSP_USE_ASIC
    ict_clk_release(clock->pclk);
    ict_clk_release(clock->wclk);    
#endif

    clock->freq = 0;
    *ctrl &= ~(SSP_CLOCK_REQ);
    return 0;
}

#ifdef SSP_USE_DMA

static void SSP_DmaTxCallback(void *para)
{    
    SSP_Handle *hdl     = (SSP_Handle *)para;
    SSP_Msg *txMsg      = &hdl->trans.txMsg;
    SSP_Msg *rxMsg      = &hdl->trans.rxMsg;
    SSP_Info *info      = &hdl->info;
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;

    txMsg->flags = 1;

    if(!rxMsg->buf || rxMsg->flags == 1){
        while(SSP_IsBusy(regs));            // slave模式下，如果发送的长度小于fifo深度，会导致dma搬完但是spi还未启动，卡死在这里
        SSP_InterruptDisableAll(regs);
        SSP_Disable(regs);
        SSP_InterruptClearAll(regs);
        SSP_PrintDebug("SSP TX DMA Transmit end");
        info->status.busy = 0;
        if(info->cbEvent){
            info->cbEvent(info->cbDate);
        }
    }
}

static void SSP_DmaRxCallback(void *para)
{
    SSP_Handle *hdl     = (SSP_Handle *)para;
    SSP_Msg *txMsg      = &hdl->trans.txMsg;
    SSP_Msg *rxMsg      = &hdl->trans.rxMsg;
    SSP_Info *info      = &hdl->info;
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;

    rxMsg->flags = 1;

    if(!txMsg->buf || txMsg->flags == 1){
        while(SSP_IsBusy(regs));
        SSP_InterruptDisableAll(regs);
        SSP_Disable(regs);
        SSP_InterruptClearAll(regs);
        SSP_PrintDebug("SSP RX DMA Transmit end");
        info->status.busy = 0;
        if(info->cbEvent){
            info->cbEvent(info->cbDate);
        }
    }
}

static int32_t SSP_TxDmaLliInit(SSP_Handle *hdl)
{
    SSP_DmaInfo *dma    = &hdl->dma;
    DMA_LliDesc *cfg    = &dma->txLliCfg[0];
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;
    SSP_Info *info      = &hdl->info;

    SSP_TxFifoThresSet(regs, SSP_TX_FIFO_THRES_DEF);
    SSP_RxFifoThresSet(regs, SSP_RX_FIFO_THRES_DEF);

    dma->txBurstLen = SSP_TX_DMA_THRES;
    dma->rxBurstLen = SSP_RX_DMA_THRES;

    dma->txHdl              = DMA_Request(hdl->res->txLogicID);
    if (!dma->txHdl){
        SSP_PrintError("SPI TX DMA Request Error\r\n");
        OS_ASSERT(0);
    }

    dma->txHdl->callback    = SSP_DmaTxCallback;
    dma->txHdl->para        = (void *)hdl;

    for(uint32_t i = 0; i < SSP_LLI_NUM_MAX; i++) {
        (cfg + i)->DestAddr             = (uint32_t)(&regs->data);

        (cfg + i)->ZYpara               = 0;
        (cfg + i)->SrcZYstep            = 0;
        (cfg + i)->LLI                  = 0;

        (cfg + i)->Control.Enable       = 1;                        // 是否需要只配置最后一个为1
        (cfg + i)->Control.BurstReqMod  = DMA_RMOD_DEV;
        (cfg + i)->Control.DestMod      = DMA_AMOD_FIFO;
        (cfg + i)->Control.SrcMod       = DMA_AMOD_RAM;
        (cfg + i)->Control.IrqMod       = DMA_IMOD_ALL_ENABLE;      // 是否需要只配置最后一个为all，其他为只处理error中断
        (cfg + i)->Control.IntSel       = DMA_INT1;
        (cfg + i)->Control.SrcBurstLen  = dma->txBurstLen;

        switch (info->control & SSP_DATA_BITS_Msk){
            case SSP_DATA_BITS(8):
                (cfg + i)->Control.SrcBurstSize   = DMA_BSIZE_8BIT;
                (cfg + i)->Control.DestBurstSize  = DMA_BSIZE_8BIT;
                SSP_PrintDebug("SSP tx DMA Burst Size 8bit");
                break;
                
            case SSP_DATA_BITS(16):
                (cfg + i)->Control.SrcBurstSize   = DMA_BSIZE_16BIT;
                (cfg + i)->Control.DestBurstSize  = DMA_BSIZE_16BIT;
                SSP_PrintDebug("SSP tx DMA Burst Size 16bit");
                break;

            case SSP_DATA_BITS(32):
                (cfg + i)->Control.SrcBurstSize   = DMA_BSIZE_32BIT;
                (cfg + i)->Control.DestBurstSize  = DMA_BSIZE_32BIT;
                SSP_PrintDebug("SSP tx DMA Burst Size 32bit");
                break;
        
            default:
                SSP_PrintError("do not support this dma msg width");
                break;
        }
    }

    DMA_ChCfg *cfg2      = &dma->txCfg;

    cfg2->DestAddr               = (uint32_t)(&regs->data);
    
    cfg2->Control.Enable         = 1;
    cfg2->Control.BurstReqMod    = DMA_RMOD_DEV;
    cfg2->Control.DestMod        = DMA_AMOD_FIFO;
    cfg2->Control.SrcMod         = DMA_AMOD_RAM;
    cfg2->Control.IrqMod         = DMA_IMOD_ALL_ENABLE;
    cfg2->Control.IntSel         = DMA_INT1;
    cfg2->Control.SrcBurstLen    = dma->txBurstLen;

    switch (info->control & SSP_DATA_BITS_Msk) {
        case SSP_DATA_BITS(8):
            cfg2->Control.SrcBurstSize   = DMA_BSIZE_8BIT;
            cfg2->Control.DestBurstSize  = DMA_BSIZE_8BIT;
            SSP_PrintDebug("set tx dma burst size 8 bit\r\n");
            break;
        
        case SSP_DATA_BITS(16):
            cfg2->Control.SrcBurstSize   = DMA_BSIZE_16BIT;
            cfg2->Control.DestBurstSize  = DMA_BSIZE_16BIT;
            SSP_PrintDebug("set tx dma burst size 16 bit\r\n");
            break;

        case SSP_DATA_BITS(32):
            cfg2->Control.SrcBurstSize   = DMA_BSIZE_32BIT;
            cfg2->Control.DestBurstSize  = DMA_BSIZE_32BIT;
            SSP_PrintDebug("SSP DMA Burst Size 32bit");
            break;
        
        default:
            SSP_PrintError("dma do not support this tx burst size\r\n");
            OS_ASSERT(0);
            break;
    }

    return DRV_OK;
}

static int32_t SPI_TxDmaSingleInit(SSP_Handle *hdl)
{
    SSP_DmaInfo *dma    = &hdl->dma;
    DMA_ChCfg *cfg      = &dma->txCfg;
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;
    SSP_Info *info      = &hdl->info;

    dma->txBurstLen = SSP_TX_DMA_THRES;
    dma->rxBurstLen = SSP_RX_DMA_THRES;

    SSP_TxFifoThresSet(regs, SSP_TX_FIFO_THRES_DEF);
    SSP_RxFifoThresSet(regs, SSP_RX_FIFO_THRES_DEF);

    dma->txHdl = DMA_Request(hdl->res->txLogicID);
    if (!dma->txHdl){
        SSP_PrintError("SSP TX DMA Request Error\r\n");
        OS_ASSERT(0);
    }
    dma->txHdl->callback        = SSP_DmaTxCallback;
    dma->txHdl->para            = (void *)hdl;

    cfg->DestAddr               = (uint32_t)(&regs->data);
    
    cfg->Control.Enable         = 1;
    cfg->Control.BurstReqMod    = DMA_RMOD_DEV;
    cfg->Control.DestMod        = DMA_AMOD_FIFO;
    cfg->Control.SrcMod         = DMA_AMOD_RAM;
    cfg->Control.IrqMod         = DMA_IMOD_ALL_ENABLE;
    cfg->Control.IntSel         = DMA_INT1;
    cfg->Control.SrcBurstLen    = dma->txBurstLen;

    switch (info->control & SSP_DATA_BITS_Msk) {
        case SSP_DATA_BITS(8):
            cfg->Control.SrcBurstSize   = DMA_BSIZE_8BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_8BIT;
            SSP_PrintDebug("set tx dma burst size 8 bit\r\n");
            break;
        
        case SSP_DATA_BITS(16):
            cfg->Control.SrcBurstSize   = DMA_BSIZE_16BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_16BIT;
            SSP_PrintDebug("set tx dma burst size 16 bit\r\n");
            break;

        case SSP_DATA_BITS(32):
            cfg->Control.SrcBurstSize   = DMA_BSIZE_32BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_32BIT;
            SSP_PrintDebug("SSP DMA Burst Size 32bit");
            break;
        
        default:
            SSP_PrintError("dma do not support this tx burst size\r\n");
            OS_ASSERT(0);
            break;
    }

    return DRV_OK;
}

static int32_t SSP_RxDmaInit(SSP_Handle *hdl)
{
    SSP_DmaInfo *dma    = &hdl->dma;
    DMA_ChCfg *cfg      = &dma->rxCfg;
    SSP_Info *info      = &hdl->info;
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;

    dma->txBurstLen = SSP_TX_DMA_THRES;
    dma->rxBurstLen = SSP_RX_DMA_THRES;

    dma->rxHdl = DMA_Request(hdl->res->rxLogicID);
    if (!dma->rxHdl){
        SSP_PrintError("SPI RX DMA Request Error\r\n");
        OS_ASSERT(0);
    }
    dma->rxHdl->callback        = SSP_DmaRxCallback;
    dma->rxHdl->para            = (void *)hdl;

    cfg->SrcAddr                = (uint32_t)(&regs->data);

    cfg->Control.Enable         = 1;
    cfg->Control.BurstReqMod    = DMA_RMOD_DEV;
    cfg->Control.SrcMod         = DMA_AMOD_FIFO;
    cfg->Control.DestMod        = DMA_AMOD_RAM;
    cfg->Control.IrqMod         = DMA_IMOD_ALL_ENABLE;
    cfg->Control.IntSel         = DMA_INT1;
    cfg->Control.SrcBurstLen    = dma->rxBurstLen;

    switch (info->control & SSP_DATA_BITS_Msk){
        case SSP_DATA_BITS(8):{
            cfg->Control.SrcBurstSize   = DMA_BSIZE_8BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_8BIT;
            SSP_PrintDebug("SSP DMA Burst Size 8bit");
            break;
        }
            
        case SSP_DATA_BITS(16):{
            cfg->Control.SrcBurstSize   = DMA_BSIZE_16BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_16BIT;
            SSP_PrintDebug("SSP DMA Burst Size 16bit");
            break;
        }

        case SSP_DATA_BITS(32):{
            cfg->Control.SrcBurstSize   = DMA_BSIZE_32BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_32BIT;
            SSP_PrintDebug("SSP DMA Burst Size 32bit");
            break;
        }
        
        default:{
            SSP_PrintError("do not support this dma msg width");
            break;
        }
    }

    return DRV_OK;
}

static void SSP_TxDmaDeInit(SSP_Handle *hdl)
{
    if(!hdl->dma.txHdl){
        SSP_PrintError("SPI TX Dma Handle is NULL");
        OS_ASSERT(0);
    }

    DMA_Release(hdl->dma.txHdl);
    // hdl->dma.txHdl = NULL;
}

static void SSP_RxDmaDeInit(SSP_Handle *hdl)
{
    if(!hdl->dma.rxHdl){
        SSP_PrintError("SPI RX Dma Handle is NULL");
        OS_ASSERT(0);
    }

    DMA_Release(hdl->dma.rxHdl);
    // hdl->dma.rxHdl = NULL;
}

static int32_t SSP_DmaInit(SSP_Handle *hdl)
{
    // SPI_TxDmaSingleInit(hdl);
    SSP_TxDmaLliInit(hdl);
    SSP_RxDmaInit(hdl);
    return 0;
}

static int32_t SSP_DmaLliInit(SSP_Handle *hdl)
{
    SSP_TxDmaLliInit(hdl);
    SSP_RxDmaInit(hdl);
    return 0;
}

static void SSP_DmaDeInit(SSP_Handle *hdl)
{
    SSP_TxDmaDeInit(hdl);
    SSP_RxDmaDeInit(hdl);
}

int32_t SSP_TxDmaStart(SSP_Handle *hdl)
{
    DMA_ChCfg *cfg  = &hdl->dma.txCfg;

    cfg->SrcAddr    = (uint32_t)hdl->trans.txMsg.buf;
    cfg->Count      = hdl->trans.txMsg.len;

    osDCacheCleanRange((void *)(cfg->SrcAddr), cfg->Count);

    return DMA_Start(hdl->dma.txHdl, cfg);
}

static int32_t SSP_TxLliDmaStart(SSP_Handle *hdl)
{
    DMA_LliDesc *cfg    = &hdl->dma.txLliCfg[0];
    SSP_Msg *msg        = hdl->trans.txMsgs;
    uint32_t addr1, addr2;

    for (uint32_t i = 0; i < hdl->trans.txNum; i++) {
        (cfg + i)->SrcAddr = (uint32_t)msg[i].buf;
        (cfg + i)->Count   = msg[i].len;
        (cfg + i)->Control.IrqMod = DMA_IMOD_ALL_DISABLE;

        if(SSP_IS_ALIGNED(msg[i].buf)) {
            osDCacheCleanRange((void *)msg[i].buf, msg[i].len);
        }
        else {
            addr1 = SSP_ALIGN_LEFT(msg[i].buf);
            addr2 = SSP_ALIGN_RIGHT(msg[i].buf + msg[i].len);
            osDCacheCleanRange((void *)addr1, addr2 - addr1);
        }
    }
    (cfg + hdl->trans.txNum - 1)->Control.Enable = 1;
    (cfg + hdl->trans.txNum - 1)->Control.IrqMod = DMA_IMOD_ALL_ENABLE;

    return DMA_LliStart(hdl->dma.txHdl, cfg, hdl->trans.txNum);
}

int32_t SSP_RxDmaStart(SSP_Handle *hdl)
{
    DMA_ChCfg *cfg = &hdl->dma.rxCfg;
    SSP_Msg *msg = &hdl->trans.rxMsg;
    int32_t ret;
    uint32_t addr1, addr2;

    cfg->DestAddr = (uint32_t)msg->buf;
    cfg->Count    = msg->len;

    if(SSP_IS_ALIGNED(msg->buf)) {
        osDCacheInvalidRange((void *)msg->buf, msg->len);
    }
    else {
        addr1 = SSP_ALIGN_LEFT(msg->buf);
        addr2 = SSP_ALIGN_RIGHT(msg->buf + msg->len);
        osDCacheInvalidRange((void *)addr1, addr2 - addr1);
    }

    ret = DMA_Start(hdl->dma.rxHdl, cfg);

    return ret;
}
#endif

/*********************************************************************************************************/

int32_t SSP_WriteMsg(SSP_Handle *hdl)
{
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;
    SSP_Transfer *trans = &hdl->trans;
    SSP_Msg *txMsg      = &trans->txMsg;
    SSP_Msg *rxMsg      = &trans->rxMsg;
    SSP_Info *info      = &hdl->info;

    // slave模式下不使用dma会卡死在这里
    while(SSP_TxFifoVacancyGet(regs) != SSP_TX_FIFO_DEPTH);

    uint8_t txVacancy = SSP_TxFifoVacancyGet(regs);
    uint8_t rxVacancy = SSP_RxFifoVacancyGet(regs);

    SSP_PrintDebug("Tx fifo vacancy : %d, rx fifo vacancy :%d", txVacancy, rxVacancy);

    uint8_t vacancy = SSP_MIN(txVacancy, rxVacancy);
    uint32_t remain;
    uint8_t txLen;

    if(!vacancy){
        SSP_PrintError("SSP has null vacancy");
        return -1;
    }

    if(!txMsg->buf){
        txLen = rxMsg->len - rxMsg->cnt;
        txLen = SSP_MIN(vacancy, txLen);
        SSP_WriteFifoDummy(regs, txLen, info->defVal);
        SSP_PrintDebug("SSP Write dummy");
        return 0;
    }

    remain = txMsg->len - txMsg->cnt;

    if(!remain) {
        SSP_PrintDebug("SSP tx buffer remain 0");
        return -1;
    }

    txLen = SSP_MIN(vacancy, remain);

    SSP_PrintDebug("SSP Will Send %d bytes Msg", txLen);

    switch (info->control & SSP_DATA_BITS_Msk){
        case SSP_DATA_BITS(8):{
            uint8_t *dataWidth8   = (uint8_t *)txMsg->buf + txMsg->cnt;
            for(int i = 0; i < txLen; i++){
                SSP_WriteFifoData(regs, *(dataWidth8 + i));
            }
            break;
        }
            
        case SSP_DATA_BITS(16):{
            uint16_t *dataWidth16   = (uint16_t *)txMsg->buf + txMsg->cnt;
            for(int i = 0; i < txLen; i++){
                SSP_WriteFifoData(regs, *(dataWidth16 + i));
            }
            break;
        }

        case SSP_DATA_BITS(32):{
            uint32_t *dataWidth32   = (uint32_t *)txMsg->buf + txMsg->cnt;
            for(int i = 0; i < txLen; i++){
                SSP_WriteFifoData(regs, *(dataWidth32 + i));
            }
            break;
        }
        
        default:{
            SSP_PrintError("do not support this write msg width");
            uint8_t *dataWidth8   = (uint8_t *)txMsg->buf + txMsg->cnt;
            for(int i = 0; i < txLen; i++){
                SSP_WriteFifoData(regs, *(dataWidth8 + i));
            }
            break;
        }
    }

    txMsg->cnt += txLen;

    return 0;
}

int32_t SSP_ReadMsg(SSP_Handle *hdl)
{
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;
    SSP_Transfer *trans = &hdl->trans;
    SSP_Msg *rxMsg      = &trans->rxMsg;
    SSP_Info *info      = &hdl->info;

    if(rxMsg->len == rxMsg->cnt && rxMsg->len != 0){
        SSP_PrintError("SSP has no data to read");
        return -1;
    }

    uint8_t rxLen = SSP_RxFifoFillGet(regs);
    if(!rxLen){
        SSP_PrintError("rx fifo is null");
        return -1;
    }

    SSP_PrintDebug("SSP Will Recv %d bytes", rxLen);

    if(!rxMsg->buf){
        SSP_ReadFifoDummy(regs, rxLen);
        SSP_PrintDebug("read dummy");
        return 0;
    }

    switch (info->control & SSP_DATA_BITS_Msk) {
        case SSP_DATA_BITS(8):{
            uint8_t *dataWidth8 = (uint8_t *)rxMsg->buf + rxMsg->cnt;
            SSP_PrintDebug("read 8bit data");
            for(int i = 0; i < rxLen; i++){
                *(dataWidth8 + i) = SSP_ReadFifoData(regs);
            }
            break;
        }

        case SSP_DATA_BITS(16):{
            uint16_t *dataWidth16 = (uint16_t *)rxMsg->buf + rxMsg->cnt;
            SSP_PrintDebug("read 16bit data");
            for(int i = 0; i < rxLen; i++){
                *(dataWidth16 + i) = SSP_ReadFifoData(regs);
            }
            break;
        }

        case SSP_DATA_BITS(32):{
            uint32_t *dataWidth32 = (uint32_t *)rxMsg->buf + rxMsg->cnt;
            SSP_PrintDebug("read 32bit data");
            for(int i = 0; i < rxLen; i++){
                *(dataWidth32 + i) = SSP_ReadFifoData(regs);
            }
            break;
        }
        
        default:{
            SSP_PrintError("do not support this read msg width");
            uint8_t *dataWidth8 = (uint8_t *)rxMsg->buf + rxMsg->cnt;
            for(int i = 0; i < rxLen; i++){
                *(dataWidth8 + i) = SSP_ReadFifoData(regs);
            }
            break;
        }
    }
    rxMsg->cnt += rxLen;

    return 0;
}

#ifdef SSP_USE_IRQ
static void SSP_InterruptServer(int id, void* param)
{
    SSP_Handle *hdl     = (SSP_Handle *)param;
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;
    SSP_Transfer *trans = &hdl->trans;
    SSP_Msg *txMsg      = &trans->txMsg;
    SSP_Msg *rxMsg      = &trans->rxMsg;
    SSP_Info *info      = &hdl->info;

    uint32_t status = SSP_InterruptStatusGet(regs);
    SSP_PrintDebug("SSP Status : 0x%x, len:%d, wr:%d, rd:%d", status, txMsg->len ? txMsg->len : rxMsg->len, txMsg->cnt, rxMsg->cnt);

    SSP_InterruptClear(regs, status);

    if(status & (SSP_TX_UNDERRUN_INTR | SSP_RX_OVERRUN_INTR)) {
        SSP_PrintError("SSP Status = 0x%x\r\n", status);

        if(status & SSP_TX_UNDERRUN_INTR) {
            SSP_PrintError("SSP_TX_UNDERRUN_INTR\r\n");
        }

        if(status & SSP_RX_OVERRUN_INTR) {
            SSP_PrintError("SSP_RX_OVERRUN_INTR\r\n");
        }
        OS_ASSERT(0);
    }

    // if(txMsg->len == txMsg->cnt && rxMsg->len == rxMsg->cnt){
    if(status & (SSP_MST_EOT_INTR)) {
        while(SSP_IsBusy(regs));
        SSP_InterruptDisableAll(regs);
        SSP_Disable(regs);
        info->status.busy = 0;
        SSP_PrintDebug("========== SSP Trans Done ===========\r\n");
        if(info->cbEvent){
            info->cbEvent(info->cbDate);
        }
        return;
    }

    if(status & (SSP_MST_EOT_INTR | SSP_TX_THRES_INTR | SSP_RX_THRES_INTR | SSP_TX_EMPTY_INTR | SSP_RX_FULL_INTR | SSP_TX_UNDERRUN_INTR | SSP_RX_OVERRUN_INTR)){
        if(status & (SSP_TX_UNDERRUN_INTR | SSP_RX_OVERRUN_INTR)){
            SSP_PrintError("SSP lose bytes");
            SSP_InterruptDisableAll(regs);
            SSP_Disable(regs);
            SSP_InterruptClearAll(regs);
            info->status.busy = 0;
            SSP_PrintDebug("========== SSP Trans Done ===========\r\n");
            if(info->cbEvent){
                info->cbEvent(info->cbDate);
            }
            return;
        }
        SSP_WriteMsg(hdl);
        SSP_ReadMsg(hdl);
    }
}
#endif

static int32_t SSP_IntrInstall(SSP_Handle *hdl)
{
#ifdef SSP_USE_IRQ
    SSP_IrqInfo *irq = &hdl->irq;
    osInterruptConfig(irq->number, irq->priorityMax, irq->level);
    osInterruptInstall(irq->number, SSP_InterruptServer, hdl);
    osInterruptUnmask(irq->number);
#endif
    return 0;
}

static int32_t SSP_IntrUnInstall(SSP_Handle *hdl)
{
#ifdef SSP_USE_IRQ
    SSP_IrqInfo *irq = &hdl->irq;
    osInterruptUninstall(irq->number);
#endif
    return 0;
}

int32_t SSP_Initialize(SSP_Handle *hdl)
{
    uint8_t *ctrl       = &hdl->ctrl;

    hdl->regs = (REG_Ssp *)hdl->res->regBase;
    *ctrl = 0;

#ifdef SSP_USE_IRQ
    SSP_IrqInfo *irq                = &hdl->irq;
    irq->number      = OS_EXT_IRQ_TO_IRQ(hdl->res->intNum);
    irq->priorityMax = 1;
    irq->level       = IRQ_HIGH_LEVEL;
#endif

    *ctrl |= SSP_INIT;

    return DRV_OK;
}

int32_t SSP_Uninitialize(SSP_Handle *hdl)
{
    uint8_t *ctrl       = &hdl->ctrl;

    SSP_PinMuxDeInit(hdl);

#ifdef SSP_USE_DMA
    SSP_DmaDeInit(hdl);
#endif

    *ctrl &= ~(SSP_INIT);

    return DRV_OK;
}

int32_t SSP_PowerControl(SSP_Handle *hdl, DRV_POWER_STATE state)
{
    uint8_t *ctrl       = &hdl->ctrl;

    switch (state){
        case DRV_POWER_OFF:
            if((*ctrl & SSP_POWER) == 0){
                return DRV_OK;
            }
            SSP_IntrUnInstall(hdl);
            SSP_ClkSrcRelease(hdl);
            *ctrl &= ~(SSP_POWER);
            break;

        case DRV_POWER_LOW:
            return DRV_ERR_UNSUPPORTED;
            break;

        case DRV_POWER_FULL:
            if((*ctrl & SSP_INIT) == 0){
                return DRV_ERR;
            }
            if(*ctrl & SSP_POWER){
                return DRV_OK;
            }
            SSP_IntrInstall(hdl);
            if((hdl->info.control & SSP_FREQ_Msk) == SSP_FREQ_13M)
                SSP_ClkSrcRequest(hdl, SSP_CLK_SRC_13M);
            else if((hdl->info.control & SSP_FREQ_Msk) == SSP_FREQ_26M)
                SSP_ClkSrcRequest(hdl, SSP_CLK_SRC_26M);
            else if((hdl->info.control & SSP_FREQ_Msk) == SSP_FREQ_39M)
                SSP_ClkSrcRequest(hdl, SSP_CLK_SRC_39M);
            else if((hdl->info.control & SSP_FREQ_Msk) == SSP_FREQ_78M)
                SSP_ClkSrcRequest(hdl, SSP_CLK_SRC_78M);
            else{
                SSP_PrintError("SSP Do not Support this Freq");
            }
            *ctrl |= SSP_POWER;
            break;
    }

    return DRV_OK;
}

static int32_t SSP_XferPrepare(SSP_Handle *hdl)
{
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;
    SSP_Transfer *trans = &hdl->trans;
    SSP_Msg *txMsg      = &trans->txMsg;
    SSP_Msg *rxMsg      = &trans->rxMsg;
    SSP_Info *info      = &hdl->info;

    // 先读空
    SSP_ReadFifoDummy(regs, SSP_RX_FIFO_DEPTH);

    SSP_PrintDebug("0x%x", READ_REG(regs->comCtrl));
    SSP_PrintDebug("0x%x", READ_REG(regs->fmtCtrl));
    SSP_PrintDebug("0x%x", READ_REG(regs->fifoCtrl));
    SSP_PrintDebug("0x%x", READ_REG(regs->fifoSr));
    SSP_PrintDebug("0x%x", READ_REG(regs->intrEn));
    SSP_PrintDebug("0x%x", READ_REG(regs->intrSr));

    if(trans->txNum > 1) {
        switch (hdl->info.control & SSP_TRANS_MODE_Msk) {
            case SSP_TRANS_DAM_MODE:
            #ifdef SSP_USE_DMA
                SSP_TxDmaStart(hdl);
                SSP_RxDmaStart(hdl);
                SSP_Enable(regs);
                // SSP_InterruptEnableAll(regs);
            #else
                SSP_PrintError("Error : Please Define SSP_USE_DMA Macro");
                OS_ASSERT(0);
            #endif
                break;
            
            default:
                SSP_PrintError("Please select poll/intr/dma mode");
                return -1;
        }

        return 0;
    }

    switch (hdl->info.control & SSP_TRANS_MODE_Msk)
    {
    case SSP_TRANS_POLL_MODE:
        SSP_Enable(regs);
        while(txMsg->len != txMsg->cnt || rxMsg->len != rxMsg->cnt){
            SSP_WriteMsg(hdl);
            SSP_ReadMsg(hdl);
            // SSP_PrintDebug("%d %d", txMsg->cnt, rxMsg->cnt);
        }
        volatile uint32_t timeout = SSP_TIMEOUT;
        while(SSP_IsBusy(regs)){
            OS_ASSERT(timeout--);
        }
        SSP_Disable(regs);
        info->status.busy = 0;
        if(info->cbEvent){
            info->cbEvent(info->cbDate);
        }
        break;
    case SSP_TRANS_INT_MODE:
    #ifdef SSP_USE_IRQ
        SSP_Enable(regs);
        SSP_InterruptEnableAll(regs);
        break;
    #else
        SSP_PrintError("Error : Please Define SSP_USE_IRQ Macro");
        OS_ASSERT(0);
    #endif
    case SSP_TRANS_DAM_MODE:
    #ifdef SSP_USE_DMA
        SSP_TxDmaStart(hdl);
        SSP_RxDmaStart(hdl);
        SSP_Enable(regs);
    #else
        SSP_PrintError("Error : Please Define SSP_USE_DMA Macro");
        OS_ASSERT(0);
    #endif
        break;
    default:
        SSP_PrintError("Please select poll/intr/dma mode");
        return -1;
    }

    return 0;
}

static void SSP_DmaAutoSel(SSP_Handle *hdl, uint32_t control)
{
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;
    SSP_Info *info      = &hdl->info;

    if((info->control & SSP_TRANS_MODE_Msk) == (control & SSP_TRANS_MODE_Msk)){
        return;
    }

    info->control &= ~(SSP_TRANS_MODE_Msk);

    switch (control & SSP_TRANS_MODE_Msk){
        case SSP_TRANS_POLL_MODE:
            info->control |= SSP_TRANS_POLL_MODE;
            SSP_InterruptDisableAll(regs);
            SSP_TxDmaDisable(regs);
            SSP_RxDmaDisable(regs);
            SSP_PrintDebug("Cnotrol Func Set Poll trans mode");
            break;
        case SSP_TRANS_INT_MODE:
        #ifdef SSP_USE_IRQ
            info->control |= SSP_TRANS_INT_MODE;
            SSP_TxDmaDisable(regs);
            SSP_RxDmaDisable(regs);
            SSP_PrintDebug("Cnotrol Func Set Intr trans mode");
            break;
        #else
            SSP_PrintError("Error : Please Define SSP_USE_IRQ Macro");
            OS_ASSERT(0);
        #endif
        case SSP_TRANS_DAM_MODE:
        #ifdef SSP_USE_DMA
            info->control |= SSP_TRANS_DAM_MODE;
            SSP_InterruptDisableAll(regs);
            SSP_TxDmaEnable(regs);
            SSP_RxDmaEnable(regs);
            SSP_PrintDebug("Cnotrol Func Set DMA trans mode");
            break;
        #else
            SSP_PrintError("Error : Please Define SSP_USE_DMA Macro");
            OS_ASSERT(0);
        #endif
    
    #if (SSP_TRANS_POLL_MODE != 0)
        default:
            SSP_InterruptDisableAll(regs);
            SSP_TxDmaDisable(regs);
            SSP_RxDmaDisable(regs);
            info->control &= ~(SSP_TRANS_MODE_Msk);
            info->control |= SSP_TRANS_POLL_MODE;
            SSP_PrintDebug("Cnotrol Func Set Poll trans mode(default)");
            break;
    #endif
    }
}

// static uint8_t tmptxdata[4500];
int32_t SSP_TransmitReceive(SSP_Handle *hdl, const void *txData, uint32_t txLen, void *rxData, uint32_t rxLen)
{
    SSP_Transfer *trans = &hdl->trans;
    SSP_Msg *txMsg      = &trans->txMsg;
    SSP_Msg *rxMsg      = &trans->rxMsg;
    SSP_Info *info      = &hdl->info;

    if(!txData && !rxData){
        SSP_PrintError("SSP Transmit Data NULL");
        return -1;
    }

    if(SSP_IsBusy(hdl->regs)){
        SSP_PrintError("SSP Is Busy");
        return -1;
    }

    info->status.busy = 1;

    memset(trans, 0, sizeof(SSP_Transfer));

    txMsg->buf   = (uint8_t*)txData;
    txMsg->len   = txLen;
    trans->txNum = 1;

    rxMsg->buf   = (uint8_t*)rxData;
    rxMsg->len   = rxLen;

    return SSP_XferPrepare(hdl);
}

uint32_t SSP_ActualRxLen(SSP_Handle *hdl)
{
    SSP_Msg *rxMsg = &hdl->trans.rxMsg;

    if((hdl->info.control & SSP_TRANS_MODE_Msk) == SSP_TRANS_DAM_MODE) {
    #ifdef SSP_USE_DMA
        SPI_NEW_TIME timeout = SPI_CURRENT_TIME + SSP_TIMEOUT;
        int32_t ret = -1;
        // volatile uint32_t timeout = SSP_TIMEOUT;
        // while(SSP_IsBusy(hdl->regs)){

        //     // if(timeout == 0) {
        //     //     os_kprintf("0x%x \r\n", READ_REG(hdl->regs->comCtrl));
        //     //     os_kprintf("0x%x \r\n", READ_REG(hdl->regs->fmtCtrl));
        //     //     os_kprintf("0x%x \r\n", READ_REG(hdl->regs->fifoCtrl));
        //     //     os_kprintf("0x%x \r\n", READ_REG(hdl->regs->fifoSr));
        //     //     os_kprintf("0x%x \r\n", READ_REG(hdl->regs->intrEn));
        //     //     os_kprintf("0x%x \r\n", READ_REG(hdl->regs->intrSr));
        //     // }
        //     OS_ASSERT(timeout--);   // 超时后需要做复位处理，否则从机将一直处于busy状态
        // }

        do{
            if(!SSP_IsBusy(hdl->regs)) {
                ret = 0;
                break;
            }
        }while (SPI_CURRENT_TIME < timeout);

        if(ret) {
            OS_ASSERT(0);
        }

        // osDCacheInvalidRange(rxMsg->buf, rxMsg->len);
        rxMsg->cnt = rxMsg->len -  DMA_GetCount(hdl->dma.rxHdl);
        SSP_ReadMsg(hdl);
    #endif
    }
    
    return rxMsg->cnt;
}

int32_t SSP_MultiTransfer(SSP_Handle *hdl, SSP_Msg *msgs, uint32_t num, uint8_t *rxData, uint32_t rxLen)
{
    SSP_Transfer *trans = &hdl->trans;
    SSP_Msg *rxMsg      = &trans->rxMsg;
    SSP_Info *info      = &hdl->info;

    if(!num) {
        SSP_PrintError("SSP Tx Num Is Zero");
    }

    for(uint32_t i = 0; i < num; i++) {
        if(!msgs[i].buf) {
            SSP_PrintError("SSP Tx Data[%d] NULL", i);
            return -1;
        }
    }

    if(!rxData){
        SSP_PrintError("SSP Rx Data NULL");
        return -1;
    }

    if(SSP_IsBusy(hdl->regs)){
        SSP_PrintError("SSP Is Busy");
        return -1;
    }

    info->status.busy = 1;

    memset(trans, 0, sizeof(SSP_Transfer));

    trans->txMsgs = msgs;
    trans->txNum = num;

    rxMsg->buf = rxData;
    rxMsg->len = rxLen;

    SSP_PrintDebug("set rx len %d", rxLen);

    return SSP_XferPrepare(hdl);
}

int32_t SSP_Control(SSP_Handle *hdl, uint32_t control, uint32_t arg)
{
    REG_Ssp *regs       = (REG_Ssp *)hdl->regs;
    SSP_Info *info      = &hdl->info;

    info->control = control;

    switch (control & SSP_FREQ_Msk){
        case SSP_FREQ_13M:
            // SSP_ClkSrcCfg(hdl, SSP_CLK_SRC_13M);
            SSP_PrintDebug("Control Func Set Freq 13M");
            break;
        
        case SSP_FREQ_26M:
            // SSP_ClkSrcCfg(hdl, SSP_CLK_SRC_26M);
            SSP_PrintDebug("Control Func Set Freq 26M");
            break;

        case SSP_FREQ_39M:
            SSP_PrintDebug("Control Func Set Freq 39M");
            break;

        case SSP_FREQ_78M:
            SSP_PrintDebug("Control Func Set Freq 78M");
            break;
    
    #if (SSP_FREQ_13M != 0)
        default:
            info->control &= ~(SSP_FREQ_Msk);
            info->control |= SSP_FREQ_13M;
            SSP_PrintDebug("Control Func Set Freq 13M(default)");
            break;
    #endif
    }

    switch (control & SSP_MS_MODE_Msk){
        case SSP_MODE_MASTER:
            SSP_MasterEnable(regs);
            SSP_PrintDebug("Control Func Set Master Mode");
            break;

        case SSP_MODE_SLAVE:
            SSP_PrintDebug("Control Func Set Slave Mode");
            SSP_SlaveEnable(regs);
            break;

    #if (SSP_MODE_MASTER != 0)
        default:
            SSP_MasterEnable(regs);
            info->control &= ~(SSP_MS_MODE_Msk);
            info->control |= SSP_MODE_MASTER;
            SSP_PrintDebug("Control Func Set Slave Mode(default)");
            break;
    #endif
    }

    switch (control & SSP_POLPHA_Msk) {
        case SSP_CPOL0_CPHA0:
            SSP_PolaritySet(regs, SSP_POLARITY_LOW);
            SSP_PhaseSet(regs, SSP_FIRST_PHASE);
            SSP_PrintDebug("Control Func Set CPOCPH 00 Mode");
            break;

        case SSP_CPOL0_CPHA1:
            SSP_PolaritySet(regs, SSP_POLARITY_LOW);
            SSP_PhaseSet(regs, SSP_SECOND_PHASE);
            SSP_PrintDebug("Control Func Set CPOCPH 01 Mode");
            break;

        case SSP_CPOL1_CPHA0:
            SSP_PolaritySet(regs, SSP_POLARITY_HIGH);
            SSP_PhaseSet(regs, SSP_FIRST_PHASE);
            SSP_PrintDebug("Control Func Set CPOCPH 10 Mode");
            break;

        case SSP_CPOL1_CPHA1:
            SSP_PolaritySet(regs, SSP_POLARITY_HIGH);
            SSP_PhaseSet(regs, SSP_SECOND_PHASE);
            SSP_PrintDebug("Control Func Set CPOCPH 11 Mode");
            break;

    #if (SSP_CPOL0_CPHA0 != 0)
        default:
            SSP_PolaritySet(regs, SSP_POLARITY_LOW);
            SSP_PhaseSet(regs, SSP_FIRST_PHASE);
            info->control &= ~(SSP_POLPHA_Msk);
            info->control |= SSP_CPOL0_CPHA0;
            SSP_PrintDebug("Control Func Set CPOCPH 00 Mode(default)");
            break;
    #endif
    }

    switch (control & SSP_FRAME_FORMAT_Msk){
        case SSP_MOTOROLA_SPI:
            SSP_FrameFormatSet(regs, SSP_MOTOROLA_SPI_FORMAT);
            SSP_PrintDebug("Control Func Set Motorola SPI Mode");
            break;

        case SSP_TI_SSP:
            SSP_FrameFormatSet(regs, SSP_TI_SSP_FORMAT);
            SSP_PrintDebug("Control Func Set TI SSP Mode");
            break;

        case SSP_ISI_SPI:
            SSP_FrameFormatSet(regs, SSP_ISI_SPI_FORMAT);
            SSP_PrintDebug("Control Func Set ISI SPI Mode");
            break;

    #if (SSP_MOTOROLA_SPI != 0)
        default:
            SSP_FrameFormatSet(regs, SSP_MOTOROLA_SPI_FORMAT);
            info->control &= ~(SSP_FRAME_FORMAT_Msk);
            info->control |= SSP_MOTOROLA_SPI;
            SSP_PrintDebug("Control Func Set Motorola SPI Mode(default)");
            break;
    #endif
    }

    switch (control & SSP_DATA_BITS_Msk){
        case SSP_DATA_BITS(8U):
            SSP_FrameSizeSet(regs, 8);
            SSP_PrintDebug("Control Func Set Data width 8 bits");
            break;

        case SSP_DATA_BITS(16U):
            SSP_FrameSizeSet(regs, 16);
            SSP_PrintDebug("Control Func Set Data width 16 bits");
            break;

        case SSP_DATA_BITS(32U):
            SSP_FrameSizeSet(regs, 32);
            SSP_PrintDebug("Control Func Set Data width 32 bits");
            break;
    
    #if (SSP_DATA_BITS(8U) != 0)
        default:
            SSP_FrameSizeSet(regs, 8);
            info->control &= ~(SSP_DATA_BITS_Msk);
            info->control |= SSP_DATA_BITS(8U);
            SSP_PrintDebug("Control Func Set Data width 8 bits(default)");
            break;
    #endif
    }

    switch (control & SSP_TRANS_MODE_Msk){
        case SSP_TRANS_POLL_MODE:
            SSP_InterruptDisableAll(regs);
            SSP_TxDmaDisable(regs);
            SSP_RxDmaDisable(regs);
            SSP_PrintDebug("Cnotrol Func Set Poll trans mode");
            break;
        case SSP_TRANS_INT_MODE:
        #ifdef SSP_USE_IRQ
            SSP_TxDmaDisable(regs);
            SSP_RxDmaDisable(regs);
            SSP_PrintDebug("Cnotrol Func Set Intr trans mode");
            break;
        #else
            SSP_PrintError("Error : Please Define SSP_USE_IRQ Macro");
            OS_ASSERT(0);
        #endif
        case SSP_TRANS_DAM_MODE:
        #ifdef SSP_USE_DMA
            SSP_InterruptDisableAll(regs);
            SSP_TxDmaEnable(regs);
            SSP_RxDmaEnable(regs);
            SSP_DmaInit(hdl);
            SSP_PrintDebug("Cnotrol Func Set DMA trans mode");
            break;
        #else
            SSP_PrintError("Error : Please Define SSP_USE_DMA Macro");
            OS_ASSERT(0);
        #endif
    
    #if (SSP_TRANS_POLL_MODE != 0)
        default:
            SSP_InterruptDisableAll(regs);
            SSP_TxDmaDisable(regs);
            SSP_RxDmaDisable(regs);
            info->control &= ~(SSP_TRANS_MODE_Msk);
            info->control |= SSP_TRANS_POLL_MODE;
            SSP_PrintDebug("Cnotrol Func Set Poll trans mode(default)");
            break;
    #endif
    }

    switch (control & SSP_CS_MDOE_Msk){
        case SSP_CS_AUTO:
            SSP_CsPinDeInit(hdl);
            break;
        case SSP_CS_MANUAL:
            SSP_CsPinMuxInit(hdl);
            break;
        
    #if (SSP_CS_AUTO != 0)
        default:
            SSP_CsPinDeInit(hdl);
            info->control &= ~(SSP_CS_MDOE_Msk);
            info->control |= SSP_CS_AUTO;
            break;
    #endif
    }

    return DRV_OK;
}

SSP_STATUS SSP_GetStatus(SSP_Handle *hdl)
{
    return hdl->info.status;
}

uint32_t SSP_GetDataCount(SSP_Handle *hdl)
{
    return 0;
}

void SSP_Stop(SSP_Handle *hdl)
{
    if((hdl->info.control & SSP_TRANS_MODE_Msk) == SSP_TRANS_DAM_MODE){
    #ifdef SSP_USE_DMA
        // 这里存在一个问题，强制停止DMA，可能刚取出fifo的数据，还未写入ram，就被停掉
        SSP_PrintDebug("stop dma");
        DMA_Release(hdl->dma.txHdl);
        DMA_Release(hdl->dma.rxHdl);
    #endif
    }
}

void SSP_WaitIdle(SSP_Handle *hdl)
{
    while(SSP_IsBusy(hdl->regs));
}



