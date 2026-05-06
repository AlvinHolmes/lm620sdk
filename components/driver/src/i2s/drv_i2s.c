
/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_i2s.c
 *
 * @brief       I2S驱动接口.
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
#include "os.h"
#include "os_hw.h"
#include "drv_dma.h"
#include "drv_i2s.h"
#include "i2s_core.h"

#if defined(OS_USING_PM)
#include "psm_sys.h"
#include "psm_wakelock.h"
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct 
{
    uint8_t *bufPtr[I2S_FRAME_BUF_NUM];
    uint16_t dataSize[I2S_FRAME_BUF_NUM];
    uint16_t defaultSize;
    uint8_t writeIndex;
    uint8_t readIndex;
    uint8_t count;
}I2S_RingBuf;

typedef struct 
{
    I2S_RingBuf ringBuf;
    DMA_ChHandle *dmaHandlePtr;
    DMA_ChCfg dmaCfg;
    uint8_t state;
    struct osSemaphore  *ctrlSem;
}I2S_Ctrl;

typedef struct
{
    I2S_Ctrl txCtrl;
    I2S_Ctrl rxCtrl;
    I2SCallBack cb;
    VoiceProcFunc voiceProc;
}I2S_Handle;

static I2S_Handle *g_i2sHandle = NULL;
#if defined(OS_USING_PM)
static WakeLock g_i2s_wakelock;
#endif

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

static int8_t I2S_BufInit(I2S_RingBuf *ringBuf, uint16_t bufSize)
{
    uint8_t bufIndex = 0;

    ringBuf->count = 0;
    ringBuf->readIndex = 0;
    ringBuf->writeIndex = 0;
    ringBuf->defaultSize = bufSize;

    for(bufIndex = 0; bufIndex < I2S_FRAME_BUF_NUM; bufIndex ++)
    {
        ringBuf->bufPtr[bufIndex] = osMallocAlign(ringBuf->defaultSize, OS_CACHE_LINE_SZ);
        OS_ASSERT(ringBuf->bufPtr[bufIndex]);
        memset(ringBuf->bufPtr[bufIndex], 0, ringBuf->defaultSize);
        ringBuf->dataSize[bufIndex] = bufSize;
    }

    return DRV_OK;
}

void I2S_RxRingBufInc(void)
{
    if(g_i2sHandle->rxCtrl.ringBuf.count < I2S_FRAME_BUF_NUM)
    {
        g_i2sHandle->rxCtrl.ringBuf.count ++; 
        if(g_i2sHandle->rxCtrl.ctrlSem)
            osSemaphoreRelease(g_i2sHandle->rxCtrl.ctrlSem); 
    }
}

static void  I2S_RxDmaCallback(OS_UNUSED void *para)
{
    if(g_i2sHandle->voiceProc.rx_proc_notify)
        g_i2sHandle->voiceProc.rx_proc_notify((uint32_t)g_i2sHandle->rxCtrl.ringBuf.bufPtr[g_i2sHandle->rxCtrl.ringBuf.writeIndex]);
    
    if(g_i2sHandle->rxCtrl.ringBuf.count < I2S_FRAME_BUF_NUM)
    { 
        g_i2sHandle->rxCtrl.ringBuf.writeIndex = (g_i2sHandle->rxCtrl.ringBuf.writeIndex + 1) % I2S_FRAME_BUF_NUM;

        if(g_i2sHandle->voiceProc.rx_proc_notify == NULL)
            I2S_RxRingBufInc();
    }

    if(g_i2sHandle->rxCtrl.ringBuf.count < I2S_FRAME_BUF_NUM)
        g_i2sHandle->rxCtrl.dmaCfg.DestAddr = (uint32_t)g_i2sHandle->rxCtrl.ringBuf.bufPtr[g_i2sHandle->rxCtrl.ringBuf.writeIndex];

    DMA_Start(g_i2sHandle->rxCtrl.dmaHandlePtr, &g_i2sHandle->rxCtrl.dmaCfg);
    g_i2sHandle->rxCtrl.state  = I2S_BUSY;

    if(g_i2sHandle->cb)
        g_i2sHandle->cb(I2S_RX_CB_EVENT);
}

static void  I2S_TxDmaCallback(OS_UNUSED void *para)
{
    g_i2sHandle->txCtrl.ringBuf.readIndex = (g_i2sHandle->txCtrl.ringBuf.readIndex + 1) % I2S_FRAME_BUF_NUM;
    g_i2sHandle->txCtrl.ringBuf.count --;

    if(g_i2sHandle->txCtrl.ringBuf.count != 0)
    {
        if(g_i2sHandle->voiceProc.tx_proc)
            g_i2sHandle->voiceProc.tx_proc(g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.readIndex], g_i2sHandle->txCtrl.ringBuf.dataSize[g_i2sHandle->txCtrl.ringBuf.readIndex]); 
    }
    else
    {
        g_i2sHandle->txCtrl.ringBuf.count ++; 
        g_i2sHandle->txCtrl.ringBuf.writeIndex = (g_i2sHandle->txCtrl.ringBuf.writeIndex + 1) % I2S_FRAME_BUF_NUM;
        memset(g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.readIndex], 0,  g_i2sHandle->txCtrl.ringBuf.defaultSize);

        if(g_i2sHandle->voiceProc.tx_proc)
            g_i2sHandle->voiceProc.tx_proc(g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.readIndex], g_i2sHandle->txCtrl.ringBuf.dataSize[g_i2sHandle->txCtrl.ringBuf.readIndex]);
   
        void* beginaddr_aligned = NULL;
        void* endaddr_aligned = NULL;
        beginaddr_aligned = (void *)OS_ALIGN_DOWN((uint32_t)g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.readIndex], 32);
        endaddr_aligned   = (void *)OS_ALIGN((uint32_t)g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.readIndex] + g_i2sHandle->txCtrl.ringBuf.dataSize[g_i2sHandle->txCtrl.ringBuf.readIndex], 32);
        osDCacheCleanRange(beginaddr_aligned, endaddr_aligned - beginaddr_aligned); 
    }

    g_i2sHandle->txCtrl.dmaCfg.SrcAddr = (uint32_t)g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.readIndex];
    g_i2sHandle->txCtrl.dmaCfg.Count = g_i2sHandle->txCtrl.ringBuf.dataSize[g_i2sHandle->txCtrl.ringBuf.readIndex];
    DMA_Start(g_i2sHandle->txCtrl.dmaHandlePtr, &g_i2sHandle->txCtrl.dmaCfg);
    g_i2sHandle->txCtrl.state  = I2S_BUSY;
    
    osSemaphoreRelease(g_i2sHandle->txCtrl.ctrlSem);
    if(g_i2sHandle->cb)
        g_i2sHandle->cb(I2S_TX_CB_EVENT);
}

int8_t I2S_Initialize(I2S_BusCfg *cfg, I2SCallBack cb, VoiceProcFunc *voiceProc)
{
    if(cfg == NULL || cfg->frameSize == 0 || (cfg->frameSize % I2S_FIFO_THRESHOLD) != 0)
        return DRV_ERR_PARAMETER;

    if(!g_i2sHandle)
    {
        g_i2sHandle = osMalloc(sizeof(I2S_Handle));
        OS_ASSERT(g_i2sHandle);

        memset(g_i2sHandle, 0, sizeof(I2S_Handle));

        //I2S Hardware Init
        I2S_CoreHandle *coreHandle = osMalloc(sizeof(I2S_CoreHandle));
        coreHandle->cfg = osMalloc(sizeof(I2S_BusCfg));
        osMemcpy(coreHandle->cfg, cfg, sizeof(I2S_BusCfg));

        if(I2S_Config(coreHandle) != DRV_OK)
            return DRV_ERR; 
 
        if(cfg->workMode & I2S_PLAY_MODE)
        {    
            g_i2sHandle->txCtrl.state = I2S_IDLE;
            I2S_BufInit(&g_i2sHandle->txCtrl.ringBuf, cfg->frameSize);
            g_i2sHandle->txCtrl.dmaHandlePtr = DMA_Request(DMA_REQ_I2S_TXF);
            g_i2sHandle->txCtrl.dmaCfg.Count = g_i2sHandle->txCtrl.ringBuf.defaultSize;
            g_i2sHandle->txCtrl.dmaCfg.DestAddr = I2S_GetDataFifoAddr();

            g_i2sHandle->txCtrl.dmaCfg.Control.BurstReqMod   = DMA_RMOD_DEV;
            g_i2sHandle->txCtrl.dmaCfg.Control.DestMod       = DMA_AMOD_FIFO;
            g_i2sHandle->txCtrl.dmaCfg.Control.SrcMod        = DMA_AMOD_RAM;
            g_i2sHandle->txCtrl.dmaCfg.Control.SrcBurstSize  = DMA_BSIZE_32BIT;
            g_i2sHandle->txCtrl.dmaCfg.Control.DestBurstSize = DMA_BSIZE_32BIT;
            g_i2sHandle->txCtrl.dmaCfg.Control.IrqMod        = DMA_IMOD_ALL_ENABLE;
            g_i2sHandle->txCtrl.dmaCfg.Control.IntSel        = DMA_INT1;
            g_i2sHandle->txCtrl.dmaCfg.Control.SrcBurstLen   = DMA_BLEN_8;
            g_i2sHandle->txCtrl.dmaHandlePtr->callback       = I2S_TxDmaCallback;
        }

        if(cfg->workMode & I2S_RECORD_MODE)
        {
            g_i2sHandle->rxCtrl.state = I2S_IDLE;
            I2S_BufInit(&g_i2sHandle->rxCtrl.ringBuf, cfg->frameSize);  
            g_i2sHandle->rxCtrl.dmaHandlePtr = DMA_Request(DMA_REQ_I2S_RXF0);
            g_i2sHandle->rxCtrl.dmaCfg.Count = g_i2sHandle->rxCtrl.ringBuf.defaultSize;  
            g_i2sHandle->rxCtrl.dmaCfg.SrcAddr = I2S_GetDataFifoAddr();

            g_i2sHandle->rxCtrl.dmaCfg.Control.BurstReqMod   = DMA_RMOD_DEV;
            g_i2sHandle->rxCtrl.dmaCfg.Control.DestMod       = DMA_AMOD_RAM;
            g_i2sHandle->rxCtrl.dmaCfg.Control.SrcMod        = DMA_AMOD_FIFO;
            g_i2sHandle->rxCtrl.dmaCfg.Control.SrcBurstSize  = DMA_BSIZE_32BIT;
            g_i2sHandle->rxCtrl.dmaCfg.Control.DestBurstSize = DMA_BSIZE_32BIT;
            g_i2sHandle->rxCtrl.dmaCfg.Control.IrqMod        = DMA_IMOD_ALL_ENABLE;
            g_i2sHandle->rxCtrl.dmaCfg.Control.IntSel        = DMA_INT1;
            g_i2sHandle->rxCtrl.dmaCfg.Control.SrcBurstLen   = DMA_BLEN_8;
            g_i2sHandle->rxCtrl.dmaHandlePtr->callback       = I2S_RxDmaCallback;
        }

        g_i2sHandle->cb = cb; 

        if(voiceProc)
        {
            g_i2sHandle->voiceProc.init = voiceProc->init;
            g_i2sHandle->voiceProc.deinit = voiceProc->deinit;
            g_i2sHandle->voiceProc.rx_proc_notify = voiceProc->rx_proc_notify;
            g_i2sHandle->voiceProc.tx_proc = voiceProc->tx_proc;
        }

        if(g_i2sHandle->voiceProc.init)
        {
            g_i2sHandle->voiceProc.init(cfg->sampleRate, 3, 20);
        }

#if defined(OS_USING_PM)
        PSM_Peri26MDmaChannelRegister(DMA_REQ_I2S_RXF0);
        PSM_Peri26MDmaChannelRegister(DMA_REQ_I2S_TXF);
        PSM_WakelockInit(&g_i2s_wakelock, PSM_DEEP_SLEEP);
#endif

    }
    
    return DRV_OK;     
}

void I2S_UnInitialize(void)
{
    uint8_t bufIndex = 0;

    if(g_i2sHandle)
    {
        if(g_i2sHandle->voiceProc.deinit)
        {
            g_i2sHandle->voiceProc.deinit();
        }

        if(g_i2sHandle->rxCtrl.dmaHandlePtr)
            DMA_Release(g_i2sHandle->rxCtrl.dmaHandlePtr);
        
        if(g_i2sHandle->txCtrl.dmaHandlePtr)
            DMA_Release(g_i2sHandle->txCtrl.dmaHandlePtr);
        
        g_i2sHandle->rxCtrl.dmaHandlePtr = NULL;
        g_i2sHandle->txCtrl.dmaHandlePtr = NULL;
        I2S_DeConfig();

        for(bufIndex = 0; bufIndex < I2S_FRAME_BUF_NUM; bufIndex ++)
        {
            if(g_i2sHandle->rxCtrl.ringBuf.bufPtr[bufIndex])
            {
                osFree(g_i2sHandle->rxCtrl.ringBuf.bufPtr[bufIndex]);
                g_i2sHandle->rxCtrl.ringBuf.bufPtr[bufIndex] = NULL;
            }

            if(g_i2sHandle->txCtrl.ringBuf.bufPtr[bufIndex])
            {
                osFree(g_i2sHandle->txCtrl.ringBuf.bufPtr[bufIndex]);
                g_i2sHandle->txCtrl.ringBuf.bufPtr[bufIndex] = NULL;
            }
                
        }
        osFree(g_i2sHandle);
        g_i2sHandle = NULL;
    }

#if defined(OS_USING_PM)
    PSM_Peri26MDmaChannelUnregister(DMA_REQ_I2S_RXF0);
    PSM_Peri26MDmaChannelUnregister(DMA_REQ_I2S_TXF);
#endif
}


int32_t I2S_BusWrite(const uint8_t *buf, uint32_t size, uint32_t timeOut)
{
    void* beginaddr_aligned = NULL;
    void* endaddr_aligned = NULL;

    if(buf == NULL || size == 0 )
        return DRV_ERR_PARAMETER;
    
    if(osSemaphoreAcquire(g_i2sHandle->txCtrl.ctrlSem, timeOut) != 0)
        return DRV_ERR;

    if(g_i2sHandle->txCtrl.ringBuf.count >= I2S_FRAME_BUF_NUM)
        return DRV_ERR_FULL;

    if(size > g_i2sHandle->txCtrl.ringBuf.defaultSize)
        size = g_i2sHandle->txCtrl.ringBuf.defaultSize;

    osMemset(g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.writeIndex], 0, g_i2sHandle->txCtrl.ringBuf.defaultSize);
    osMemcpy(g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.writeIndex], buf, size);  
    g_i2sHandle->txCtrl.ringBuf.dataSize[g_i2sHandle->txCtrl.ringBuf.writeIndex] = size;

    beginaddr_aligned = (void *)OS_ALIGN_DOWN((uint32_t)g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.writeIndex], 32);
    endaddr_aligned   = (void *)OS_ALIGN((uint32_t)g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.writeIndex] + size, 32);
    osDCacheCleanRange(beginaddr_aligned, endaddr_aligned - beginaddr_aligned); 

    ubase_t level = osInterruptDisable();
    g_i2sHandle->txCtrl.ringBuf.count ++; 
    g_i2sHandle->txCtrl.ringBuf.writeIndex = (g_i2sHandle->txCtrl.ringBuf.writeIndex + 1) % I2S_FRAME_BUF_NUM;
    osInterruptEnable(level);

    if(g_i2sHandle->txCtrl.state == I2S_IDLE)
    {
        g_i2sHandle->txCtrl.dmaCfg.SrcAddr = (uint32_t)g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.readIndex];
        g_i2sHandle->txCtrl.dmaCfg.Count = size;
        DMA_Start(g_i2sHandle->txCtrl.dmaHandlePtr, &g_i2sHandle->txCtrl.dmaCfg);
        g_i2sHandle->txCtrl.state  = I2S_BUSY;
    }
    return size;
}

int32_t I2S_BusRead(uint8_t *buf, uint32_t size, uint32_t timeOut)
{
    void  *beginaddr_aligned;
    void  *endaddr_aligned;

    if(buf == NULL || size == 0)
        return DRV_ERR;    

    if(osSemaphoreAcquire(g_i2sHandle->rxCtrl.ctrlSem, timeOut) != 0)
        return DRV_ERR;
    
    if(size > g_i2sHandle->rxCtrl.ringBuf.defaultSize)
        size = g_i2sHandle->rxCtrl.ringBuf.defaultSize;
    
    if(g_i2sHandle->rxCtrl.ringBuf.count == 0)
        return DRV_ERR_EMPTY;
    
    beginaddr_aligned = (void *)OS_ALIGN_DOWN((uint32_t)g_i2sHandle->rxCtrl.ringBuf.bufPtr[g_i2sHandle->rxCtrl.ringBuf.readIndex], 32);
    endaddr_aligned   = (void *)OS_ALIGN((uint32_t)g_i2sHandle->rxCtrl.ringBuf.bufPtr[g_i2sHandle->rxCtrl.ringBuf.readIndex] + size, 32);
    osDCacheCleanAndInvalidRange(beginaddr_aligned, endaddr_aligned - beginaddr_aligned);

    osMemcpy(buf, g_i2sHandle->rxCtrl.ringBuf.bufPtr[g_i2sHandle->rxCtrl.ringBuf.readIndex], size);

    ubase_t level = osInterruptDisable();
    g_i2sHandle->rxCtrl.ringBuf.readIndex = (g_i2sHandle->rxCtrl.ringBuf.readIndex + 1) % I2S_FRAME_BUF_NUM;
    g_i2sHandle->rxCtrl.ringBuf.count --;
    osInterruptEnable(level);

    return size;
}

int8_t I2S_BusStart(uint8_t mode)
{
    void  *beginaddr_aligned;
    void  *endaddr_aligned;

#if defined(OS_USING_PM)
    PSM_WakeLock(&g_i2s_wakelock);
#endif

    if(mode & I2S_PLAY_MODE)
    {
        g_i2sHandle->txCtrl.state = I2S_IDLE;
        g_i2sHandle->txCtrl.ringBuf.count = 0;
        g_i2sHandle->txCtrl.ringBuf.readIndex = 0;
        g_i2sHandle->txCtrl.ringBuf.writeIndex = 0;

        I2S_TxEnable(true);

        if(g_i2sHandle->txCtrl.ctrlSem == NULL)
        {
            g_i2sHandle->txCtrl.ctrlSem = osMalloc(sizeof(struct osSemaphore));
            osSemaphoreInit(g_i2sHandle->txCtrl.ctrlSem, I2S_FRAME_BUF_NUM - 1, I2S_FRAME_BUF_NUM, OS_IPC_FLAG_FIFO);
        }

        beginaddr_aligned = (void *)OS_ALIGN_DOWN((uint32_t)g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.writeIndex], OS_CACHE_LINE_SZ);
        endaddr_aligned   = (void *)OS_ALIGN((uint32_t)g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.writeIndex] + 
                                                        g_i2sHandle->txCtrl.ringBuf.defaultSize, OS_CACHE_LINE_SZ);
        osDCacheCleanAndInvalidRange(beginaddr_aligned, endaddr_aligned - beginaddr_aligned);

        g_i2sHandle->txCtrl.ringBuf.count ++; 
        g_i2sHandle->txCtrl.ringBuf.writeIndex = (g_i2sHandle->txCtrl.ringBuf.writeIndex + 1) % I2S_FRAME_BUF_NUM;

        g_i2sHandle->txCtrl.dmaCfg.SrcAddr = (uint32_t)g_i2sHandle->txCtrl.ringBuf.bufPtr[g_i2sHandle->txCtrl.ringBuf.readIndex];
        g_i2sHandle->txCtrl.dmaCfg.Count = g_i2sHandle->txCtrl.ringBuf.defaultSize;
        DMA_Start(g_i2sHandle->txCtrl.dmaHandlePtr, &g_i2sHandle->txCtrl.dmaCfg);
        g_i2sHandle->txCtrl.state  = I2S_BUSY;
        
    }
 
    if(mode & I2S_RECORD_MODE)
    {
        g_i2sHandle->rxCtrl.state = I2S_IDLE;
        g_i2sHandle->rxCtrl.ringBuf.count = 0;
        g_i2sHandle->rxCtrl.ringBuf.readIndex = 0;
        g_i2sHandle->rxCtrl.ringBuf.writeIndex = 0;

        I2S_RxEnable(true);
        if(g_i2sHandle->rxCtrl.ctrlSem == NULL)
        {
            g_i2sHandle->rxCtrl.ctrlSem = osMalloc(sizeof(struct osSemaphore));
            osSemaphoreInit(g_i2sHandle->rxCtrl.ctrlSem, 0, I2S_FRAME_BUF_NUM, OS_IPC_FLAG_FIFO);
        }

        beginaddr_aligned = (void *)OS_ALIGN_DOWN((uint32_t)g_i2sHandle->rxCtrl.ringBuf.bufPtr[g_i2sHandle->rxCtrl.ringBuf.writeIndex], 32);
        endaddr_aligned   = (void *)OS_ALIGN((uint32_t)g_i2sHandle->rxCtrl.ringBuf.bufPtr[g_i2sHandle->rxCtrl.ringBuf.writeIndex] + 
                                                        g_i2sHandle->rxCtrl.ringBuf.defaultSize, 32);
        osDCacheCleanAndInvalidRange(beginaddr_aligned, endaddr_aligned - beginaddr_aligned);

        g_i2sHandle->rxCtrl.dmaCfg.DestAddr = (uint32_t)g_i2sHandle->rxCtrl.ringBuf.bufPtr[g_i2sHandle->rxCtrl.ringBuf.writeIndex];
        DMA_Start(g_i2sHandle->rxCtrl.dmaHandlePtr, &g_i2sHandle->rxCtrl.dmaCfg);
        g_i2sHandle->rxCtrl.state  = I2S_BUSY;
    }

    return DRV_OK;
}

int8_t I2S_BusStop(uint8_t mode)
{
    if(!g_i2sHandle)
        return DRV_ERR_EMPTY;
        
    if(mode & I2S_PLAY_MODE)
    {
        DMA_Stop(g_i2sHandle->txCtrl.dmaHandlePtr);
        I2S_TxEnable(false);

        g_i2sHandle->txCtrl.state = I2S_IDLE;
        g_i2sHandle->txCtrl.ringBuf.count = 0;
        g_i2sHandle->txCtrl.ringBuf.readIndex = 0;
        g_i2sHandle->txCtrl.ringBuf.writeIndex = 0;

        if(g_i2sHandle->txCtrl.ctrlSem)
        {
            osSemaphoreDetach(g_i2sHandle->txCtrl.ctrlSem);
            osFree(g_i2sHandle->txCtrl.ctrlSem);
            g_i2sHandle->txCtrl.ctrlSem = NULL;
        }
        
    }

    if(mode & I2S_RECORD_MODE)
    {
        DMA_Stop(g_i2sHandle->rxCtrl.dmaHandlePtr);
        I2S_RxEnable(false);

        g_i2sHandle->rxCtrl.state = I2S_IDLE;
        g_i2sHandle->rxCtrl.ringBuf.count = 0;
        g_i2sHandle->rxCtrl.ringBuf.readIndex = 0;
        g_i2sHandle->rxCtrl.ringBuf.writeIndex = 0;

        if(g_i2sHandle->rxCtrl.ctrlSem)
        {
            osSemaphoreDetach(g_i2sHandle->rxCtrl.ctrlSem);
            osFree(g_i2sHandle->rxCtrl.ctrlSem);
            g_i2sHandle->rxCtrl.ctrlSem = NULL;
        }
    }

#if defined(OS_USING_PM)
    PSM_WakeUnlock(&g_i2s_wakelock);
#endif

    return DRV_OK;
}

uint8_t I2S_BusGetAvaliableBufCount(uint8_t mode)
{
    if(mode & I2S_PLAY_MODE)
        return (I2S_FRAME_BUF_NUM - g_i2sHandle->txCtrl.ringBuf.count);
    else
        return g_i2sHandle->rxCtrl.ringBuf.count;
}

uint8_t I2S_BusGetState(uint8_t mode)
{
    if(mode & I2S_PLAY_MODE)
        return g_i2sHandle->txCtrl.state;
    else
        return g_i2sHandle->rxCtrl.state;
}
