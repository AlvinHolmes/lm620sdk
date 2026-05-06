#include "os.h"
#include "os_hw.h"
#include "drv_spi_cam.h"
#include "drv_dma.h"
#include "spi_cam_core.h"
#include <stdio.h>
#include "os.h"
#include "drv_soc.h"

#define         MAX_DMA_LLI_NUM                 10
#define         SINGLE_DMA_TRANSFER_SIZE        (60 *1024 )

#define SPICAM_IMAGE_WIDTH(hdl)         ((hdl)->imageWidth)
#define SPICAM_IMAGE_HEIGHT(hdl)        ((hdl)->imageHeight)
#define SPICAM_LANE_NUM(hdl)            ((hdl)->wireNum)
#define SPICAM_MEMPOOL(hdl)             ((&(hdl)->pool))

// #define SPI_PrintDebug(fmt, ...) osPrintf("[%-5d]    %-15s %-25s :" fmt "\r\n", __LINE__, __FILE__, __func__, ##__VA_ARGS__)
#define SPI_PrintError(fmt, ...) osPrintf("[%-5d]    %-15s %-25s :" "\033[" "31m" fmt "\r\n" "\033[0m", __LINE__, __FILE__, __func__, ##__VA_ARGS__)
#define SPI_PrintDebug(fmt, ...)

static SPICAM_Handle *g_spiHandle = NULL;

static SPICAM_ImageBuffer *g_camUsingNode = NULL;


static inline void SPI_RequestCLock(void)
{
    //clock
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_CAM_SW_PCLK_EN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_CAM_SW_WCLK_EN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_CAM_WCLK_SEL, SPICAM_WCLK_156M);

    //mclk
    CLK_SetTopCrmRegs(CLK_CAM_MCLK_DIV_SEL, 0);     // 默认二分频，需要改为一分频
    CLK_SetTopCrmRegs(CLK_CAM_MCLK_EN, 1);
    CLK_SetTopCrmRegs(CLK_CAM_MCLK_SEL, CAM_MCLK_CLK_52M);
}

static inline void SPI_ReleaseClock(void)
{
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_CAM_SW_PCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_CAM_SW_WCLK_EN, 0);
    
    //mclk
    CLK_SetTopCrmRegs(CLK_CAM_MCLK_EN, 0);
}

static inline void SPI_SetSlaveMode(SpiReg *reg)
{
    uint32_t reg_val = READ_REG(reg->comCtrl);
    
    reg_val |= (SPI_MS_MASK);
    
    WRITE_REG(reg->comCtrl, reg_val);
}

typedef enum{
    SSP_MOTOROLA_SPI_FORMAT = 0,
    SSP_TI_SSP_FORMAT,
    SSP_ISI_SPI_FORMAT,
}SSP_FrameFormat;

static inline void SPI_SetFrameMode(SpiReg *reg, SSP_FrameFormat format)
{
    MODIFY_REG(reg->fmtCtrl, SPI_FRAME_FMT_MASK, format);
}

typedef enum{
    SSP_POLARITY_LOW = 0,
    SSP_POLARITY_HIGH,
}SSP_Polarity;

static inline void SSP_PolaritySet(SpiReg *regs, SSP_Polarity pol)
{
    if(SSP_POLARITY_HIGH == pol){
        SET_BIT(regs->fmtCtrl, SPI_POL_MASK);
    } else {
        CLEAR_BIT(regs->fmtCtrl, SPI_POL_MASK);
    }
}

typedef enum {
    SSP_FIRST_PHASE = 0,
    SSP_SECOND_PHASE,
}SSP_Phase;

static inline void SSP_PhaseSet(SpiReg *regs, SSP_Phase pha)
{
    if(SSP_SECOND_PHASE == pha){
        SET_BIT(regs->fmtCtrl, SPI_PHA_MASK);
    } else {
        CLEAR_BIT(regs->fmtCtrl, SPI_PHA_MASK);
    }
}

static inline void SPI_LineNumSet(SpiReg *reg, uint8_t num)
{
    uint32_t reg_val = READ_REG(reg->fmtCtrl);

    reg_val |= (SPI_CAM_MODE_MASK);     // camera mode
    reg_val &= (~SPI_LANE_NUM_MASK);    // data line num
    reg_val |= (num << SPI_LANE_NUM_POS);
    WRITE_REG(reg->fmtCtrl, reg_val);
}

static inline void SPI_FifoThresSet(SpiReg *reg, uint8_t thres)
{
    uint32_t reg_val = READ_REG(reg->fifoCtrl);
    reg_val &= (~SPI_TX_FIFO_THRES_MASK);
    reg_val &= (~SPI_RX_FIFO_THRES_MASK);
    reg_val |= (thres << SPI_TX_FIFO_THRES_POS);
    reg_val |= (thres << SPI_RX_FIFO_THRES_POS);
    //dma en
    reg_val |= (SPI_TX_DMA_EN_MASK);
    reg_val |= (SPI_RX_DMA_EN_MASK);
    
    WRITE_REG(reg->fifoCtrl, reg_val);
}

static inline void SPI_ResetFifo(SpiReg *reg)
{
    uint32_t reg_val = READ_REG(reg->syncCode);
    reg_val |= (SPI_RST_CAM_FIFO_MASK );
    WRITE_REG(reg->syncCode, reg_val);
}

static inline void SPI_PacketSizeSet(SpiReg *reg, uint32_t size)
{
    WRITE_REG(reg->packetSize, size);
}

static inline void SPI_SetSyncCode(SpiReg *reg, uint8_t code, uint8_t sampleMode)
{
    uint32_t reg_val = READ_REG(reg->syncCode);
    reg_val &= (~SPI_SYNC_CODE_MASK);
    reg_val |= (code << SPI_SYNC_CODE_POS);

    if(code == CAMERA_MODE_MTK)
    {
        reg_val &= ~(0xFFFFFF << SPI_ID_SOF_POS);
        reg_val |= (MTK_START << SPI_ID_SOF_POS);
        reg_val |= (MTK_DATA_PACKET << SPI_ID_SOL_POS);
        reg_val |= (MTK_END << SPI_ID_EOF_POS);
    }
    else
    {
        reg_val &= ~(0xFFFFFF << SPI_ID_SOF_POS);
        reg_val |= (BT656_START << SPI_ID_SOF_POS);
        reg_val |= (BT656_LINE_START << SPI_ID_SOL_POS);
        reg_val |= (BT656_END << SPI_ID_EOF_POS);
    }

    reg_val &= (~SPI_SAMPLE_MODE_MASK);
    reg_val |= (sampleMode << SPI_SAMPLE_MODE_POS);

    WRITE_REG(reg->syncCode, reg_val);
}

static void SPICAM_MemDeInit(SPICAM_Handle *hdl)
{
    SPICAM_ImageBuffer *node;

    // OS_ASSERT(SPICAM_MEMPOOL(hdl)->remain == SPICAM_MEMPOOL(hdl)->depth - 1);   // 有一个节点被控制器长期持有,需要减1

    // SPI_PrintDebug("spicam mempool remain %d", SPICAM_MEMPOOL(hdl)->remain);

    // uint32_t num = osListLen(&SPICAM_MEMPOOL(hdl)->free) + osListLen(&SPICAM_MEMPOOL(hdl)->used);

    for(uint8_t j = 0; j < SPICAM_MEMPOOL(hdl)->depth; j++) {
        node = &SPICAM_MEMPOOL(hdl)->array[j];
        osFree(node->data);
        osFree(node->dmaCfg);
    }

    osFree(SPICAM_MEMPOOL(hdl)->array);
}

static void SPICAM_MemInit(SPICAM_Handle *hdl)
{
    DMA_LliDesc  *dma;
    OS_ASSERT(SPICAM_MEMPOOL(hdl)->depth);

    SPICAM_MEMPOOL(hdl)->array = (SPICAM_ImageBuffer *)osMalloc(sizeof(SPICAM_ImageBuffer) * SPICAM_MEMPOOL(hdl)->depth);
    OS_ASSERT(SPICAM_MEMPOOL(hdl)->array != NULL);

    SPI_PrintDebug("malloc image node : 0x%x", SPICAM_MEMPOOL(hdl)->array);

    osListInit(&SPICAM_MEMPOOL(hdl)->free);
    osListInit(&SPICAM_MEMPOOL(hdl)->used);

    hdl->rxCtrl.userLLiNum = SPICAM_IMAGE_HEIGHT(hdl) * SPICAM_IMAGE_WIDTH(hdl) * 2 / SINGLE_DMA_TRANSFER_SIZE;
    if(SPICAM_IMAGE_HEIGHT(hdl) * SPICAM_IMAGE_WIDTH(hdl) * 2 % SINGLE_DMA_TRANSFER_SIZE != 0) {
        hdl->rxCtrl.userLLiNum += 1;
    }

    for(uint8_t j = 0; j < SPICAM_MEMPOOL(hdl)->depth; j++) {
        SPICAM_ImageBuffer *node = &SPICAM_MEMPOOL(hdl)->array[j];
        osListInit(&node->node);
        memset(node->name, 0, 15);
        sprintf(node->name, "img:%d", j);
        node->len = SPICAM_IMAGE_HEIGHT(hdl) * SPICAM_IMAGE_WIDTH(hdl) * 2 + 0;
        node->data = osMallocAlign(node->len, OS_CACHE_LINE_SZ);
        SPI_PrintDebug("malloc image buffer : 0x%x len : %d", node->data, node->len);
        OS_ASSERT(node->data != NULL);
        osDCacheInvalidRange(node->data, node->len);
        node->dmaParam = node;
        osListInsertBefore(&SPICAM_MEMPOOL(hdl)->free, &node->node);

        node->dmaCfg = (DMA_LliDesc *)osMallocAlign(sizeof(DMA_LliDesc) * hdl->rxCtrl.userLLiNum, OS_CACHE_LINE_SZ);
        SPI_PrintDebug("malloc dma config : 0x%x", node->dmaCfg);
        OS_ASSERT(node->dmaCfg != NULL);
        // node->dmaCfg = descList + sizeof(DMA_LliDesc) * hdl->rxCtrl.userLLiNum * j;
        dma = node->dmaCfg;

        for(uint8_t i = 0; i < hdl->rxCtrl.userLLiNum; i++)
        {
            dma[i].Control.BurstReqMod   = DMA_RMOD_DEV;
            dma[i].Control.DestMod       = DMA_AMOD_RAM ;
            dma[i].Control.SrcMod        = DMA_AMOD_FIFO ;
            dma[i].Control.SrcBurstSize  = DMA_BSIZE_32BIT;
            dma[i].Control.DestBurstSize = DMA_BSIZE_32BIT;

            dma[i].Control.IrqMod = DMA_IMOD_ALL_DISABLE;
            if(i == (hdl->rxCtrl.userLLiNum -1) )
                dma[i].Control.IrqMod        = DMA_IMOD_ALL_ENABLE;

            dma[i].Control.IntSel        = DMA_INT1;
            dma[i].Control.SrcBurstLen   = DMA_BLEN_8;
            dma[i].Control.Enable        = 1;

            dma[i].DestAddr = (uint32_t)node->data + i * SINGLE_DMA_TRANSFER_SIZE;
            dma[i].SrcAddr = SPI_GetDataFifoAddr(hdl->regs);

            if ((node->len) >= (uint32_t)((i + 1) * SINGLE_DMA_TRANSFER_SIZE))
                dma[i].Count = SINGLE_DMA_TRANSFER_SIZE;
            else
                dma[i].Count = (node->len) - i * SINGLE_DMA_TRANSFER_SIZE;

            // SPI_PrintDebug("len = %d", dma[i].Count);
        }

        osDCacheCleanAndInvalidRange(node->data, node->len);
        osDCacheCleanRange(node->dmaCfg, sizeof(DMA_LliDesc) * hdl->rxCtrl.userLLiNum);
    }

    SPICAM_MEMPOOL(hdl)->remain = SPICAM_MEMPOOL(hdl)->depth;
    SPICAM_MEMPOOL(hdl)->max = 0;
    SPI_PrintDebug("spi cam mempool init");
}

static SPICAM_ImageBuffer *SPICAM_MemFreeListGet(SPICAM_Handle *hdl)
{
    SPICAM_ImageBuffer *node;

    unsigned long level = osInterruptDisable();

    if(osListIsEmpty(&SPICAM_MEMPOOL(hdl)->free)) {
        osInterruptEnable(level);
        return NULL;
        // 如果没有空闲节点，则从已使用链表中取最后一个节点
        // node = osContainerOf(SPICAM_MEMPOOL(hdl)->used.prev, SPICAM_ImageBuffer, node);
        // osPrintf("free pool full\r\n");
    }
    else {
        node = osListFirstEntry(&SPICAM_MEMPOOL(hdl)->free, SPICAM_ImageBuffer, node);

        SPICAM_MEMPOOL(hdl)->remain--;
        if(SPICAM_MEMPOOL(hdl)->max < SPICAM_MEMPOOL(hdl)->depth - SPICAM_MEMPOOL(hdl)->remain) {
            SPICAM_MEMPOOL(hdl)->max = SPICAM_MEMPOOL(hdl)->depth - SPICAM_MEMPOOL(hdl)->remain;
        }
    }

    osListRemove(&node->node);

    // osPrintf("get free node : %s\r\n", node->name);

    osInterruptEnable(level);

    return node;
}

static SPICAM_ImageBuffer *SPICAM_MemBuf2Node(SPICAM_Handle *hdl, uint8_t *addr)
{
    SPICAM_ImageBuffer *node = NULL;

    for(uint32_t i = 0; i < SPICAM_MEMPOOL(hdl)->depth; i++) {
        node = &SPICAM_MEMPOOL(hdl)->array[i];
        if(node->data == addr) {
            break;
        }
    }

    OS_ASSERT(node);

    return node;
}

static void SPICAM_ImagePut(SPICAM_ImageBuffer *node)
{
    osDCacheInvalidRange(node->data, node->len);

    SPICAM_Handle *hdl = g_spiHandle;

    unsigned long level = osInterruptDisable();
    osListInsertBefore(&SPICAM_MEMPOOL(hdl)->free, &node->node);
    SPICAM_MEMPOOL(hdl)->remain++;

    // osPrintf("put free node : %s\r\n", node->name);

    osInterruptEnable(level);
}

void SPICAM_ImageFree(SPICAM_Handle *hdl, uint8_t *addr)
{
    SPICAM_ImageBuffer *node = SPICAM_MemBuf2Node(hdl, addr);

    SPICAM_ImagePut(node);

    if(SPICAM_MEMPOOL(hdl)->depth == 1 && !(hdl->ctrl & SPICAM_RUNNING)){
        SPICAM_Start(hdl);
    }
}

uint8_t *SPICAM_ImageCapture(SPICAM_Handle *hdl)
{
    unsigned long level = osInterruptDisable();

    // if(!(hdl->ctrl & SPICAM_RUNNING)){
    //     osInterruptEnable(level);
    //     return NULL;
    // }

    if(osListIsEmpty(&SPICAM_MEMPOOL(hdl)->used)) {
        osInterruptEnable(level);
        return NULL;
    }

    SPICAM_ImageBuffer *node = osListFirstEntry(&SPICAM_MEMPOOL(hdl)->used, SPICAM_ImageBuffer, node);
    osListRemove(&node->node);
    osInterruptEnable(level);

    return node->data;
}

static void SPICAM_MemUseListPut(SPICAM_ImageBuffer *node)
{
    SPICAM_Handle *hdl = g_spiHandle;

    unsigned long level = osInterruptDisable();
    osListInsertBefore(&SPICAM_MEMPOOL(hdl)->used, &node->node);
    // osPrintf("put used node : %s\r\n", node->name);
    osInterruptEnable(level);
}

static void SPICAM_RxDmaCallback(OS_UNUSED void *para)
{
    SPICAM_ImageBuffer *node = (SPICAM_ImageBuffer *)para;
    SPICAM_ImageBuffer *newNode;
    
    if(g_spiHandle->err) {
        // osPrintf("rx overrun interrupt \r\n");
        SPICAM_Stop(g_spiHandle);
        g_spiHandle->err = false;
        g_spiHandle->errNumber++;
        SPICAM_ImagePut(node);
        SPICAM_Start(g_spiHandle);
    }
    else {
        newNode = SPICAM_MemFreeListGet(g_spiHandle);
        if (newNode)
        {
            SPICAM_MemUseListPut(node);
            node = newNode;
            g_camUsingNode = newNode;
            g_spiHandle->rxCtrl.dmaHandlePtr->para = newNode->dmaParam;
            // osDCacheInvalidRange(node->data, node->len);
        }
        else if(SPICAM_MEMPOOL(g_spiHandle)->depth == 1) {
            SPICAM_Stop(g_spiHandle);
            SPICAM_MemUseListPut(node);
            if(g_spiHandle->cbEvent)
                g_spiHandle->cbEvent(g_spiHandle->userData, SPI_RX_EVENT);
            return;
        }

        DMA_LliStart(g_spiHandle->rxCtrl.dmaHandlePtr, node->dmaCfg, g_spiHandle->rxCtrl.userLLiNum);
        
        if(g_spiHandle->cbEvent)
            g_spiHandle->cbEvent(g_spiHandle->userData, SPI_RX_EVENT);
    }
}

static void SPICAM_InterruptServer(OS_UNUSED int vector, OS_UNUSED void *param)
{
    SPICAM_Handle *hdl = (SPICAM_Handle *)param;

    if(SPI_IsCamRxOverrunInterrupt(hdl->regs)) 
    {
        osPrintf("rx overrun interrupt \r\n");
        SPI_EnableRxOverrunInterrupt(hdl->regs, false);
        SPI_ClrCamRxOverrunInterrupt(hdl->regs);
        hdl->err = true;
    }

    SPI_ClrCamAllInterrupt(hdl->regs);
}

static void SPICAM_DmaInit(SPICAM_Handle *hdl)
{
    hdl->rxCtrl.dmaHandlePtr = DMA_Request(DMA_REQ_SPI_CAM_RX);

    SPICAM_MemInit(hdl);

    hdl->rxCtrl.dmaHandlePtr->callback = SPICAM_RxDmaCallback;
}

static void SPI_PreConfig(SpiReg *reg)
{
    SPI_Disable(reg);//disable spi
    
    SPI_SetSlaveMode(reg);// master or slave

    
    SPI_SetFrameMode(reg, SSP_MOTOROLA_SPI_FORMAT);// fmt

    SSP_PolaritySet(reg, SSP_POLARITY_LOW);
    SSP_PhaseSet(reg, SSP_FIRST_PHASE);

    //fifo thres
    SPI_FifoThresSet(reg, 7);
    
    SPI_SetSyncCode(reg, CAMERA_MODE_MTK, SAMPLE_IMAGE_DATA);
    
    SPI_ResetFifo(reg);
}

int32_t SPICAM_Initialize(SPICAM_Handle *hdl)
{
    g_spiHandle = hdl;

    hdl->regs = hdl->res->regBase;
    hdl->ctrl = 0;

    SPI_RequestCLock();

    SPI_PreConfig(hdl->regs); 

    osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum));
    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum), 1, IRQ_HIGH_LEVEL);
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum), SPICAM_InterruptServer,  hdl);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum));

    hdl->ctrl |= SPICAM_INIT;

    return DRV_OK;
}

int8_t SPICAM_UnInitialize(SPICAM_Handle *hdl)
{
    SPI_Disable(hdl->regs);
    SPI_ReleaseClock();

    osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum));
    osInterruptMask(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum));

    hdl->ctrl &= ~SPICAM_INIT;

    return DRV_OK;
}

int32_t SPICAM_PowerControl(SPICAM_Handle *hdl, DRV_POWER_STATE state)
{
    switch (state){
        case DRV_POWER_FULL:
            if(!(hdl->ctrl & SPICAM_INIT)){
                SPI_PrintError("Please Init SPI First...\r\n");
                return DRV_ERR;
            }

            if(hdl->ctrl & SPICAM_POWER){
                return DRV_OK;
            }

            SPI_RequestCLock();
        
            SPICAM_DmaInit(hdl);

            hdl->ctrl |= SPICAM_POWER;
            break;

        case DRV_POWER_LOW:
            return DRV_ERR_UNSUPPORTED;

        case DRV_POWER_OFF:
            if(!(hdl->ctrl & SPICAM_POWER)){
                return DRV_OK;
            }
            DMA_Stop(hdl->rxCtrl.dmaHandlePtr);
            DMA_Release(hdl->rxCtrl.dmaHandlePtr);

            SPI_Disable(hdl->regs);
            
            SPICAM_MemDeInit(hdl);

            SPI_ReleaseClock();

            hdl->ctrl &= ~(SPICAM_POWER);
            break;
        
        default:
            break;
    }
    

    return DRV_OK;
}

int32_t SPICAM_Control(SPICAM_Handle *hdl, uint32_t control, uint32_t arg)
{
    switch (control){
        case SPICAM_CONFIG_FRAME_WIDTH:
            SPICAM_IMAGE_WIDTH(hdl) = arg;
            break;

        case SPICAM_CONFIG_FREME_HEIGHT:
            SPICAM_IMAGE_HEIGHT(hdl) = arg;
            SPI_PacketSizeSet(hdl->regs, SPICAM_IMAGE_HEIGHT(hdl) * 2);
            break;

        case SPICAM_CONFIG_FRAME_DEPTH:
            SPICAM_MEMPOOL(hdl)->depth = arg;
            break;

        case SPICAM_CONFIG_LINE_NUM:
            SPICAM_LANE_NUM(hdl) = arg;
            SPI_LineNumSet(hdl->regs, arg);
            break;
        
        default:
            return DRV_ERR_UNSUPPORTED;
    }

    return DRV_OK;
}

void SPICAM_Start(SPICAM_Handle *hdl)
{
    unsigned long level;

    if(hdl->ctrl & SPICAM_RUNNING){
        return;
    }
    SPI_Disable(hdl->regs);

    SPI_ClrCamAllInterrupt(hdl->regs);

    SPI_RstCamFifo(hdl->regs);

    SPI_EnableRxOverrunInterrupt(hdl->regs, true);

    SPICAM_ImageBuffer *node = SPICAM_MemFreeListGet(hdl);
    if (node)
    {
        // osDCacheInvalidRange(node->data, node->len);

        hdl->rxCtrl.dmaHandlePtr->para = node->dmaParam;
        DMA_LliStart(hdl->rxCtrl.dmaHandlePtr, node->dmaCfg, hdl->rxCtrl.userLLiNum);
        // osPrintf("cam get node : %s\r\n", node->name);
    }

    level = osInterruptDisable();

    hdl->ctrl |= SPICAM_RUNNING;

    osInterruptEnable(level);

    SPI_Enable(hdl->regs);
}

void SPICAM_Stop(SPICAM_Handle *hdl)
{
    unsigned long level;

    if(!(hdl->ctrl & SPICAM_RUNNING)){
        return;
    }

    DMA_Stop(hdl->rxCtrl.dmaHandlePtr);

    SPI_Disable(hdl->regs);

    SPI_RstCamFifo(hdl->regs);
    SPI_EnableCamSOFInterrupt(hdl->regs, false);
    SPI_EnableRxOverrunInterrupt(hdl->regs, false);

    SPI_ClrCamAllInterrupt(hdl->regs);

    level = osInterruptDisable();

    hdl->ctrl &= ~(SPICAM_RUNNING);

    osInterruptEnable(level);
}

uint32_t SpiCam_ErrNumber(void)
{
    uint32_t ret = g_spiHandle->errNumber;
    g_spiHandle->errNumber = 0;
    return ret;
}

