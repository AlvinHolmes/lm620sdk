/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_spi.c
 *
 * @brief       SPI驱动接口.
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
#include <drv_pin.h>
#include <drv_spi.h>
#include <drv_soc.h>
#include "drv_spi_private.h"
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
// #define SPI_PrintDebug(fmt, ...) osPrintf("[%-5d]    %-15s %-25s :" fmt "\r\n", __LINE__, __FILE__, __func__, ##__VA_ARGS__)
#define SPI_PrintError(fmt, ...) osPrintf("[%-5d]    %-15s %-25s :" "\033[" "31m" fmt "\r\n" "\033[0m", __LINE__, __FILE__, __func__, ##__VA_ARGS__)
#define SPI_PrintDebug(fmt, ...)

#define SPI_DEF_TIMEOUT             (1000)
#define SPI_DEF_TX_WIDTH            (8)
#define SPI_DEF_TX_THRES            (8)
#define SPI_DEF_RX_THRES            (4)
#define SPI_DEF_TX_VALUE            (0xff)
#define SPI_CACHE_SIZE              (OS_CACHE_LINE_SZ)
#define SPI_MIN(a, b)               ((a) < (b) ? (a) : (b))
#define SPI_ALIGN_RIGHT(addr)       ((((uint32_t)(addr)) + (SPI_CACHE_SIZE) - 1) & ~((SPI_CACHE_SIZE) - 1))
#define SPI_ALIGN_LEFT(addr)        (((uint32_t)(addr)) & ~((SPI_CACHE_SIZE) - 1))
#define SPI_IS_ALIGNED(addr)        (!(((uint32_t)(addr)) & (SPI_CACHE_SIZE - 1)))

#define SPI_NEW_TIME                osTime_t
#define SPI_CURRENT_TIME            osGetTime()

static SPI_Handle *g_spiHdl = NULL;

static void SPI_BusNumberConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config & SPI_BUS_NUMBER_Msk)){
        return;
    }

    hdl->config &= ~SPI_BUS_NUMBER_Msk;
    hdl->config |= mask;

    SPI_PrintDebug("config bus number : %d", mask >> SPI_BUS_NUMBER_Pos);
}

static void SPI_ClockRequest(SPI_Handle *hdl)
{
    CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_SW_PCLK_EN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_SW_WCLK_EN, 1);

    SPI_PrintDebug("request clock");
}

static void SPI_ClockRelease(SPI_Handle *hdl)
{
    CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_SW_PCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_SW_WCLK_EN, 0);

    SPI_PrintDebug("release clock");
}

static void SPI_ClockSourceConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config & SPI_CLK_SRC_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_CLK_SRC_13M || mask == SPI_CLK_SRC_26M || mask == SPI_CLK_SRC_39M || mask == SPI_CLK_SRC_78M);

    hdl->config &= ~SPI_CLK_SRC_Msk;
    hdl->config |= mask;

    switch (mask){
        case SPI_CLK_SRC_13M:
            CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_WCLK_SEL, SSP0_WCLK_26M);
            SPI_PrintDebug("config clock source 13M");
            break;

        case SPI_CLK_SRC_26M:
            CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_WCLK_SEL, SSP0_WCLK_52M);
            SPI_PrintDebug("config clock source 26M");
            break;

        case SPI_CLK_SRC_39M:
            CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_WCLK_SEL, SSP0_WCLK_78M);
            SPI_PrintDebug("config clock source 39M");
            break;

        case SPI_CLK_SRC_78M:
            CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_WCLK_SEL, SSP0_WCLK_156M);
            SPI_PrintDebug("config clock source 78M");
            break;
    }
}

static void SPI_ClockDivConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config & SPI_CLK_DIV_Msk)){
        return;
    }

    OS_ASSERT(mask <= SPI_CLK_DIV(0xf));

    hdl->config &= ~SPI_CLK_DIV_Msk;
    hdl->config |= mask;

    CLK_SetPdcoreLspCrmRegs(CLK_GEN_SSP_DIV_PARAM, (mask >> SPI_CLK_DIV_Pos));

    SPI_PrintDebug("config div %d", mask >> SPI_CLK_DIV_Pos);
}

static void SPI_MSConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config & SPI_MS_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_MS_MODE_MASTER || mask == SPI_MS_MODE_SLAVE);

    hdl->config &= ~SPI_MS_MODE_Msk;
    hdl->config |= mask;

    if(mask == SPI_MS_MODE_MASTER){
        SPI_MasterEnable(hdl->regs);
        SPI_PrintDebug("config master mode");
    }else{
        SPI_SlaveEnable(hdl->regs);
        SPI_PrintDebug("config slave mode");
    }
}

static void SPI_PolPhaConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config & SPI_POLPHA_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_POLPHA_00 || mask == SPI_POLPHA_01 || mask == SPI_POLPHA_10 || mask == SPI_POLPHA_11);

    hdl->config &= ~SPI_POLPHA_Msk;
    hdl->config |= mask;

    switch (mask) {
        case SPI_POLPHA_00:
            SPI_PolaritySet(hdl->regs, SPI_POLARITY_LOW);
            SPI_PhaseSet(hdl->regs, SPI_FIRST_PHASE);
            SPI_PrintDebug("config pol pha 00");
            break;

        case SPI_POLPHA_01:
            SPI_PolaritySet(hdl->regs, SPI_POLARITY_LOW);
            SPI_PhaseSet(hdl->regs, SPI_SECOND_PHASE);
            SPI_PrintDebug("config pol pha 01");
            break;

        case SPI_POLPHA_10:
            SPI_PolaritySet(hdl->regs, SPI_POLARITY_HIGH);
            SPI_PhaseSet(hdl->regs, SPI_FIRST_PHASE);
            SPI_PrintDebug("config pol pha 10");
            break;

        case SPI_POLPHA_11:
            SPI_PolaritySet(hdl->regs, SPI_POLARITY_HIGH);
            SPI_PhaseSet(hdl->regs, SPI_SECOND_PHASE);
            SPI_PrintDebug("config pol pha 11");
            break;
    }
}

static void SPI_DataWidthConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config & SPI_DATA_BITS_Msk)){
        return;
    }

    OS_ASSERT(mask >= SPI_DATA_BITS(8) && mask <= SPI_DATA_BITS(16));

    hdl->config &= ~SPI_DATA_BITS_Msk;
    hdl->config |= mask;

    SPI_WordSizeSet(hdl->regs, mask >> SPI_DATA_BITS_Pos);

    SPI_PrintDebug("config data width %d", mask >> SPI_DATA_BITS_Pos);
}

static void SPI_TransModeConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config & SPI_TRANS_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_TRANS_MODE_POLL || mask == SPI_TRANS_MODE_INT || mask == SPI_TRANS_MODE_DAM || 
              mask == SPI_TRANS_MODE_DAM_POLL || mask == SPI_TRANS_MODE_POLL_DMA || mask == SPI_TRANS_MODE_HDLC);

    hdl->config &= ~SPI_TRANS_MODE_Msk;
    hdl->config |= mask;

    switch (mask) {
        case SPI_TRANS_MODE_POLL:
            SPI_TxRxDmaDisable(hdl->regs);
            SPI_AllIntrDisable(hdl->regs);
            SPI_PrintDebug("config trans mode poll");
            break;

        case SPI_TRANS_MODE_INT:
            SPI_TxRxDmaDisable(hdl->regs);
            SPI_AllIntrEnable(hdl->regs);
            SPI_PrintDebug("config trans mode int");
            break;

        case SPI_TRANS_MODE_DAM:
            SPI_TxRxDmaEnable(hdl->regs);
            SPI_FifoIntrDisable(hdl->regs);
            SPI_TransIntrEnable(hdl->regs);
            SPI_PrintDebug("config trans mode dma");
            break;
        
        case SPI_TRANS_MODE_DAM_POLL:
            SPI_TxRxDmaDisable(hdl->regs);
            SPI_AllIntrDisable(hdl->regs);
            SPI_PrintDebug("config trans mode dma poll");
            break;

        case SPI_TRANS_MODE_POLL_DMA:
            SPI_TxRxDmaEnable(hdl->regs);
            SPI_AllIntrDisable(hdl->regs);
            SPI_PrintDebug("config trans mode poll dma");
            break;
    }
}

static void SPI_CsModeConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config & SPI_CS_MDOE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_CS_MDOE_AUTO || mask == SPI_CS_MDOE_MANUAL);

    hdl->config &= ~SPI_CS_MDOE_Msk;
    hdl->config |= mask;

    if(mask == SPI_CS_MDOE_AUTO){
        // AUTO
        SPI_PrintDebug("config cs mode auto");
    }else {
        SPI_PrintDebug("config cs mode manual");
    }
}


static void SPI_TxThresConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config2 & SPI_TX_THRES_Msk)){
        return;
    }

    OS_ASSERT(mask >= SPI_TX_THRES_VALUE(1) && mask <= SPI_TX_THRES_VALUE(16));

    hdl->config2 &= ~SPI_TX_THRES_Msk;
    hdl->config2 |= mask;

    SPI_TxFifoThresSet(hdl->regs, mask >> SPI_TX_THRES_Pos);

    SPI_PrintDebug("config2 tx thres %d", mask >> SPI_TX_THRES_Pos);
}

static void SPI_RxThresConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config2 & SPI_RX_THRES_Msk)){
        return;
    }

    OS_ASSERT(mask >= SPI_RX_THRES_VALUE(1) && mask <= SPI_RX_THRES_VALUE(16));

    hdl->config2 &= ~SPI_RX_THRES_Msk;
    hdl->config2 |= mask;

    SPI_RxFifoThresSet(hdl->regs, mask >> SPI_RX_THRES_Pos);

    SPI_PrintDebug("config2 rx thres %d", mask >> SPI_RX_THRES_Pos);
}

static void SPI_TxDefValConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->config2 & SPI_TX_VALUE_Msk)){
        return;
    }

    hdl->config2 &= ~SPI_TX_VALUE_Msk;
    hdl->config2 |= mask;

    SPI_PrintDebug("config2 default value %x", mask >> SPI_TX_VALUE_Pos);
}

/**
 * @brief 管脚方向
 * 
 * @param hdl 
 * @param mask 
 */
static void SPI_BiDirModeConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_BIDIR_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_BIDIR_MODE_UNI_DIR || mask == SPI_BIDIR_MODE_BI_DIR);

    hdl->speConfig &= ~SPI_BIDIR_MODE_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_BIDIR_MODE_UNI_DIR){
        SPI_UnBiDirPinMode(hdl->regs);
        SPI_PrintDebug("spec config un-bi dir mode");
    }else{
        SPI_BiDirPinMode(hdl->regs);
        SPI_PrintDebug("spec config bi dir mode");
    }
}

/**
 * @brief 传输方向：只发  只收  收发同时
 * 
 * @param hdl 
 * @param mask 
 */
static void SPI_DirModeConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_DIR_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_DIR_MODE_TX_ONLY || mask == SPI_DIR_MODE_RX_ONLY || mask == SPI_DIR_MODE_TX_RX);

    hdl->speConfig &= ~SPI_DIR_MODE_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_DIR_MODE_TX_ONLY){
        SPI_RtxModeCfg(hdl->regs, SPI_TX_ONLY_MODE);
        SPI_PrintDebug("spec config tx only");
    }else if(mask == SPI_DIR_MODE_RX_ONLY){
        SPI_RtxModeCfg(hdl->regs, SPI_RX_ONLY_MODE);
        SPI_PrintDebug("spec config rx only");
    }else{
        SPI_RtxModeCfg(hdl->regs, SPI_RTX_BOTH);
        SPI_PrintDebug("spec config tx&rx both");
    }
}

static void SPI_HwFlowCtrlConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_HW_FLOWCTRL_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_HW_FLOWCTRL_EN || mask == SPI_HW_FLOWCTRL_DIS);

    hdl->speConfig &= ~SPI_HW_FLOWCTRL_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_HW_FLOWCTRL_EN){
        SPI_HardFlowCtrlEnable(hdl->regs);
        SPI_PrintDebug("spec config hw flowctrl enable");
    }else{
        SPI_HardFlowCtrlDisable(hdl->regs);
        SPI_PrintDebug("spec config hw flowctrl disable");
    }
}

static void SPI_HwFlowCtrlModeConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_HW_FLOWCTRL_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_HW_FLOWCTRL_MODE_SINGLE || mask == SPI_HW_FLOWCTRL_MODE_DUL);

    hdl->speConfig &= ~SPI_HW_FLOWCTRL_MODE_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_HW_FLOWCTRL_MODE_SINGLE){
        SPI_DevReadySignalSingleMode(hdl->regs);
        SPI_PrintDebug("spec config hw flowctrl single");
    }else{
        SPI_DevReadySignalDulMode(hdl->regs);
        SPI_PrintDebug("spec config hw flowctrl dul");
    }
}

static void SPI_SpeculatSpeedConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_SPECULAT_SPEED_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_SPECULAT_SPEED_EN || mask == SPI_SPECULAT_SPEED_DIS);

    hdl->speConfig &= ~SPI_SPECULAT_SPEED_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_SPECULAT_SPEED_EN){
        SPI_SpeculatEnable(hdl->regs);
        SPI_PrintDebug("spec config speculat enable");
    }else{
        SPI_SpeculatDisable(hdl->regs);
        SPI_PrintDebug("spec config speculat disable");
    }
}

static void SPI_ClockStyleConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_CLK_STYLE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_CLK_STYLE_IDLE_PROMPT || mask == SPI_CLK_STYLE_IDLE_DELAY);

    hdl->speConfig &= ~SPI_CLK_STYLE_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_CLK_STYLE_IDLE_PROMPT){
        SPI_ClkNormalMode(hdl->regs);
        SPI_PrintDebug("spec config clk style idle prompt");
    }else{
        SPI_ClkExtraMode(hdl->regs);
        SPI_PrintDebug("spec config clk style idle delay");
    }
}

static void SPI_RtxQtyModeConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_RX_QTY_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_RX_QTY_MODE_MANULE || mask == SPI_RX_QTY_MODE_AUTO);

    hdl->speConfig &= ~SPI_RX_QTY_MODE_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_RX_QTY_MODE_AUTO){
        SPI_RxLengthAutoEnable(hdl->regs);
        SPI_PrintDebug("spec config rx qty auto");
    }else{
        SPI_RxLengthAutoDisable(hdl->regs);
        SPI_PrintDebug("spec config rx qty manule");
    }
}

static void SPI_TxCrcAutoConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_TX_CRC16_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_TX_CRC16_MODE_AUTO || mask == SPI_TX_CRC16_MODE_MANULE);

    hdl->speConfig &= ~SPI_TX_CRC16_MODE_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_TX_CRC16_MODE_AUTO){
        SPI_TxCrc16Enalbe(hdl->regs);
        SPI_PrintDebug("spec config tx crc auto");
    }else{
        SPI_TxCrc16Disable(hdl->regs);
        SPI_PrintDebug("spec config tx crc manule");
    }
}

static void SPI_RxCrcAutoConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_RX_CRC16_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_RX_CRC16_MODE_AUTO || mask == SPI_RX_CRC16_MODE_MANULE);

    hdl->speConfig &= ~SPI_RX_CRC16_MODE_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_RX_CRC16_MODE_AUTO){
        SPI_RxCrc16Enalbe(hdl->regs);
        SPI_PrintDebug("spec config rx crc auto");
    }else{
        SPI_RxCrc16Disalbe(hdl->regs);
        SPI_PrintDebug("spec config rx crc manule");
    }
}

static void SPI_InputRefConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_INPUT_REF_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_INPUT_REF_MODE_EN || mask == SPI_INPUT_REF_MODE_DIS);

    hdl->speConfig &= ~SPI_INPUT_REF_MODE_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_INPUT_REF_MODE_EN){
        SPI_InReflectionEnable(hdl->regs);
        SPI_PrintDebug("spec config input ref enable");
    }else{
        SPI_InReflectionDisable(hdl->regs);
        SPI_PrintDebug("spec config input ref disable");
    }
}

static void SPI_OutputRefConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->speConfig & SPI_OUTPUT_REF_MODE_Msk)){
        return;
    }

    OS_ASSERT(mask == SPI_OUTPUT_REF_MODE_EN || mask == SPI_OUTPUT_REF_MODE_DIS);

    hdl->speConfig &= ~SPI_OUTPUT_REF_MODE_Msk;
    hdl->speConfig |= mask;

    if(mask == SPI_OUTPUT_REF_MODE_EN){
        SPI_OutReflectionEnable(hdl->regs);
        SPI_PrintDebug("spec config output ref enable");
    }else{
        SPI_OutReflectionDisable(hdl->regs);
        SPI_PrintDebug("spec config output ref enable");
    }
}

static void SPI_CrcOutXorValueConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->crc16Param & SPI_CRC_OUT_XOR_Msk)){
        return;
    }

    hdl->crc16Param &= ~SPI_CRC_OUT_XOR_Msk;
    hdl->crc16Param |= mask;

    SPI_CrcOutXorValSet(hdl->regs, mask >> SPI_CRC_OUT_XOR_Pos);
}

static void SPI_CrcInitValueConfig(SPI_Handle *hdl, uint32_t mask)
{
    if(mask == (hdl->crc16Param & SPI_CRC_INIT_VAL_Msk)){
        return;
    }

    hdl->crc16Param &= ~SPI_CRC_INIT_VAL_Msk;
    hdl->crc16Param |= mask;

    SPI_CrcOutXorValSet(hdl->regs, mask >> SPI_CRC_INIT_VAL_Pos);
}

static void SPI_PinMuxInit(void)
{

}

static void SPI_PinMuxDeInit(void)
{

}

void SPI_CsSet(SPI_Handle *hdl, uint8_t level)
{
    
}

void SPI_CommonCfg(SPI_Handle *hdl, uint32_t arg)
{
    uint32_t mask = arg & SPI_BUS_NUMBER_Msk;
    if(mask){
        SPI_BusNumberConfig(hdl, mask);
    }
    
    mask = arg & SPI_CLK_SRC_Msk;
    if(mask){
        SPI_ClockSourceConfig(hdl, mask);
    }

    mask = arg & SPI_CLK_DIV_Msk;
    if (mask) {
        SPI_ClockDivConfig(hdl, mask);
    }

    mask = arg & SPI_MS_MODE_Msk;
    if(mask){
        SPI_MSConfig(hdl, mask);
    }

    mask = arg & SPI_POLPHA_Msk;
    if(mask){
        SPI_PolPhaConfig(hdl, mask);
    }

    mask = arg & SPI_DATA_BITS_Msk;
    if(mask){
        SPI_DataWidthConfig(hdl, mask);
    }

    mask = arg & SPI_TRANS_MODE_Msk;
    if(mask){
        SPI_TransModeConfig(hdl, mask);
    }

    mask = arg & SPI_CS_MDOE_Msk;
    if(mask){
        SPI_CsModeConfig(hdl, mask);
    }
}

static void SPI_ValueCfg(SPI_Handle *hdl, uint32_t arg)
{
    uint32_t mask = arg & SPI_TX_THRES_Msk;
    if(mask){
        SPI_TxThresConfig(hdl, mask);
    }

    mask = arg & SPI_RX_THRES_Msk;
    if(mask){
        SPI_RxThresConfig(hdl, mask);
    }

    mask = arg & SPI_TX_VALUE_Msk;
    if(mask){
        SPI_TxDefValConfig(hdl, mask);
    }
}

void SPI_SpecCfg(SPI_Handle *hdl, uint32_t arg)
{
    uint32_t mask = arg & SPI_BIDIR_MODE_Msk;
    if(mask){
        SPI_BiDirModeConfig(hdl, mask);
    }

    mask = arg & SPI_DIR_MODE_Msk;
    if(mask){
        SPI_DirModeConfig(hdl, mask);
    }

    mask = arg & SPI_HW_FLOWCTRL_Msk;
    if(mask){
        SPI_HwFlowCtrlConfig(hdl, mask);
    }

    mask = arg & SPI_HW_FLOWCTRL_MODE_Msk;
    if(mask){
        SPI_HwFlowCtrlModeConfig(hdl, mask);
    }

    mask = arg & SPI_SPECULAT_SPEED_Msk;
    if(mask){
        SPI_SpeculatSpeedConfig(hdl, mask);
    }

    mask = arg & SPI_CLK_STYLE_Msk;
    if(mask){
        SPI_ClockStyleConfig(hdl, mask);
    }

    mask = arg & SPI_RX_QTY_MODE_Msk;
    if(mask){
        SPI_RtxQtyModeConfig(hdl, mask);
    }

    mask = arg & SPI_TX_CRC16_MODE_Msk;
    if(mask){
        SPI_TxCrcAutoConfig(hdl, mask);
    }

    mask = arg & SPI_RX_CRC16_MODE_Msk;
    if(mask){
        SPI_RxCrcAutoConfig(hdl, mask);
    }

    mask = arg & SPI_INPUT_REF_MODE_Msk;
    if(mask){
        SPI_InputRefConfig(hdl, mask);
    }

    mask = arg & SPI_OUTPUT_REF_MODE_Msk;
    if(mask){
        SPI_OutputRefConfig(hdl, mask);
    }
}

static void SPI_CrcParamCfg(SPI_Handle *hdl, uint32_t arg)
{
    SPI_CrcOutXorValueConfig(hdl, arg & SPI_CRC_OUT_XOR_Msk);
    SPI_CrcInitValueConfig(hdl, arg & SPI_CRC_INIT_VAL_Msk);
}



static void SPI_MasterWriteOnlyMsg(SPI_Handle *hdl)
{
    uint8_t len;
    SPI_Msg *msg = &hdl->trans.txMsg;
    uint8_t vacancy;

    uint32_t remain = msg->len - msg->cnt;
    if(!remain){
        return;
    }

    vacancy = SPI_TxVacancyGet(hdl->regs);
    if(!vacancy){
        // SPI_PrintError("spi no tx vacancy");
        return;
    }

    len = SPI_MIN(vacancy, remain);
    
    uint8_t *dataWidth8   = (uint8_t *)msg->buf + msg->cnt;
    for(uint32_t i = 0; i < len; i++){
        SPI_WriteFifo(hdl->regs, *(dataWidth8 + i));
    }

    msg->cnt += len;

    return;
}

/**
 * @brief SLAVE TX ONLY 模式下发送数据
 * 
 * @param hdl 
 */
static uint32_t SPI_SlaveWriteOnlyMsg(SPI_Handle *hdl)
{
    uint8_t len;
    SPI_Msg *msg = &hdl->trans.txMsg;
    uint8_t vacancy;

    uint32_t remain = msg->len - msg->cnt;
    if(!remain){
        return 0;
    }

    vacancy = SPI_TxVacancyGet(hdl->regs);
    if(!vacancy){
        // SPI_PrintError("spi no tx vacancy");
        return 0;
    }

    len = SPI_MIN(vacancy, remain);
    if(!len){
        return 0;
    }
    
    uint8_t *dataWidth8   = (uint8_t *)msg->buf + msg->cnt;
    for(uint32_t i = 0; i < len; i++){
        SPI_WriteFifo(hdl->regs, *(dataWidth8 + i));
    }

    msg->cnt += len;

    return len;
}

/**
 * @brief SLAVE RX ONLY 模式下读取数据
 * 
 * 如果实际读取的长度，超过指定的长度，则会丢弃
 * 
 * @param hdl 
 */
static void SPI_SlaveReadOnlyMsg(SPI_Handle *hdl)
{
    uint8_t len;
    SPI_Msg *msg = &hdl->trans.rxMsg;
    uint8_t valid = SPI_RxValidGet(hdl->regs);
    if(!valid){
        // SPI_PrintError("spi no rx valid data");
        return;
    }

    // 接收到指定长度后，其他数据丢弃
    if(msg->len <= msg->cnt){
        // msg->flags |= SPI_RX_MSG_OVERRUN;
        // SPI_PrintError("spi rx buffer overrun");
        for(uint32_t i = 0; i < valid; i++){
            SPI_ReadFifo(hdl->regs);
        }
        msg->cnt += valid;
        return;
    }

    len = SPI_MIN(valid, msg->len - msg->cnt);
    
    uint8_t *dataWidth8   = (uint8_t *)msg->buf + msg->cnt;
    for(uint32_t i = 0; i < len; i++){
        *(dataWidth8 + i) = SPI_ReadFifo(hdl->regs);
    }

    msg->cnt += len;

    return;
}

static void SPI_MasterReadOnlyMsg(SPI_Handle *hdl)
{
    uint8_t len;
    SPI_Msg *msg = &hdl->trans.rxMsg;
    uint8_t valid = SPI_RxValidGet(hdl->regs);
    if(!valid){
        return;
    }

    // 接收到指定长度后，其他数据丢弃
    if(msg->len <= msg->cnt){
        hdl->trans.err |= SPI_MSG_MST_RX_OVERRUN;
        for(uint32_t i = 0; i < valid; i++){
            SPI_ReadFifo(hdl->regs);
        }
        msg->cnt += valid;
        return;
    }
#if 0
    else if(((msg->len) == msg->cnt) && ((hdl->speConfig & SPI_RX_CRC16_MODE_Msk) == SPI_RX_CRC16_MODE_AUTO)){
        uint8_t tmp = SPI_ReadFifo(hdl->regs);
        hdl->trans.rxMsg.crcRecv &= ~(0xff);
        hdl->trans.rxMsg.crcRecv |= tmp;
        msg->cnt += 1;
    }else if(((msg->len + 1) == msg->cnt) && ((hdl->speConfig & SPI_RX_CRC16_MODE_Msk) == SPI_RX_CRC16_MODE_AUTO)){
        uint8_t tmp = SPI_ReadFifo(hdl->regs);
        hdl->trans.rxMsg.crcRecv &= ~(0xff00);
        hdl->trans.rxMsg.crcRecv |= (uint16_t)tmp << 8;
        msg->cnt += 1;
    }
#endif
    len = SPI_MIN(valid, msg->len - msg->cnt);
    
    uint8_t *dataWidth8   = (uint8_t *)msg->buf + msg->cnt;
    for(uint32_t i = 0; i < len; i++){
        *(dataWidth8 + i) = SPI_ReadFifo(hdl->regs);
    }

    msg->cnt += len;

    return;
}

static int32_t SPI_WriteMsg(SPI_Handle *hdl)
{
    return DRV_OK;
}

static int32_t SPI_ReadMsg(SPI_Handle *hdl)
{
    return DRV_OK;
}

static void SPI_XferEndFunc(SPI_Handle *hdl)
{
    if((hdl->speConfig & SPI_RX_CRC16_MODE_Msk) == SPI_RX_CRC16_MODE_AUTO){
        hdl->trans.rxMsg.crc = SPI_CrcRxStaGet(hdl->regs);
        hdl->trans.rxMsg.crc0 = SPI_RxCrcHist0(hdl->regs);
        hdl->trans.rxMsg.crc1 = SPI_RxCrcHist1(hdl->regs);
    }
    if((hdl->speConfig & SPI_TX_CRC16_MODE_Msk) == SPI_TX_CRC16_MODE_AUTO){
        hdl->trans.txMsg.crc = SPI_CrcTxStaGet(hdl->regs);
    }

    hdl->status.busy = 0;

    if(hdl->cbEvent) {
        if(hdl->trans.err) {
            hdl->cbEvent(hdl->userData, SPI_EVENT_DATA_LOST);
        }
        else {
            hdl->cbEvent(hdl->userData, SPI_EVENT_TRANSFER_COMPLETE);
        }
    }
        

    // hdl->status.busy = 0;
}

static int32_t SPI_SlavePollXfer(SPI_Handle *hdl)
{
    SPI_NEW_TIME timeout = SPI_CURRENT_TIME + hdl->timeout;
    uint32_t mask = hdl->speConfig & SPI_DIR_MODE_Msk;
    bool end;
    int32_t ret = DRV_ERR;

    SPI_TxRxFifoClear(hdl->regs);
    SPI_AllIntrClear(hdl->regs);

    // 重新启动，避免CS跳变导致提前终止
restart:
    end = false;

    if(mask == SPI_DIR_MODE_TX_ONLY || mask == SPI_DIR_MODE_TX_RX){
        SPI_TxLengthSet(hdl->regs, hdl->trans.txMsg.len - 1);
        SPI_SlaveWriteOnlyMsg(hdl);
    }
    
    SPI_Start(hdl->regs);

    if(hdl->extFunc) {
        hdl->extFunc(hdl->extParam);
    }

    do{
        if(!SPI_IsBusy(hdl->regs)){
            SPI_PrintDebug("spi is idle");
            end = true;
        }
        switch (mask){
            case SPI_DIR_MODE_TX_ONLY:
                if(SPI_SlaveWriteOnlyMsg(hdl)) {
                    timeout = SPI_CURRENT_TIME + hdl->timeout;
                }
                if(end){
                    if(SPI_TxValidyGet(hdl->regs)){
                        goto restart;
                    }
                    // 未全部发送完
                    if(hdl->trans.txMsg.len != hdl->trans.txMsg.cnt){
                        hdl->trans.err |= SPI_MSG_SLV_TX_UNFINISH;
                        ret = DRV_ERR;
                        goto success;
                    }
                    ret = DRV_OK;
                    goto success;
                }
                break;
            case SPI_DIR_MODE_RX_ONLY:
                SPI_SlaveReadOnlyMsg(hdl);
                if(end){
                    if(!hdl->trans.rxMsg.cnt){
                        SPI_PrintError("restart\r\n");
                        goto restart;
                    }
                    if(hdl->trans.rxMsg.len < hdl->trans.rxMsg.cnt){
                        hdl->trans.err |= SPI_MSG_SLV_RX_OVERRUN;
                        ret = DRV_ERR;
                        goto success;
                    }
                    ret = DRV_OK;
                    goto success;
                }
                break;
            case SPI_DIR_MODE_TX_RX:
                // SPI_WriteMsg(hdl);
                break;
        
            default:
                break;
        }
    } while(SPI_CURRENT_TIME < timeout);

    SPI_PrintError("trans poll timeout");
    ret = DRV_ERR_TIMEOUT;
    return ret;

success:
    SPI_XferEndFunc(hdl);

    return ret;
}

static int32_t SPI_MasterPollXfer(SPI_Handle *hdl)
{
    SPI_NEW_TIME timeout = SPI_CURRENT_TIME + hdl->timeout;
    uint32_t mask = hdl->speConfig & SPI_DIR_MODE_Msk;
    bool end = false;
    int32_t ret = DRV_ERR;
    SPI_PrintDebug("start master poll transfer");

    SPI_TxRxFifoClear(hdl->regs);
    SPI_AllIntrClear(hdl->regs);

    if(mask == SPI_DIR_MODE_TX_ONLY || mask == SPI_DIR_MODE_TX_RX){
        SPI_TxLengthSet(hdl->regs, hdl->trans.txMsg.len - 1);
        SPI_MasterWriteOnlyMsg(hdl);
    }

    if(mask == SPI_DIR_MODE_RX_ONLY || mask == SPI_DIR_MODE_TX_RX){
        if((hdl->speConfig & SPI_RX_QTY_MODE_Msk) == SPI_RX_QTY_MODE_MANULE){
            SPI_RxLengthRegSet(hdl->regs, hdl->trans.rxMsg.len - 1);

            if((hdl->speConfig & SPI_RX_CRC16_MODE_Msk) == SPI_RX_CRC16_MODE_AUTO)
                hdl->trans.rxMsg.len += 2;
        }
    }

    SPI_Start(hdl->regs);

    do{
        if(!SPI_IsBusy(hdl->regs)){
            SPI_PrintDebug("spi is idle");
            end = true;
        }
        switch (mask){
            case SPI_DIR_MODE_TX_ONLY:
                SPI_MasterWriteOnlyMsg(hdl);
                if(end){
                    ret = DRV_OK;
                    goto success;
                }
                break;
            case SPI_DIR_MODE_RX_ONLY:
                SPI_MasterReadOnlyMsg(hdl);
                if(end){
                    ret = DRV_OK;
                    goto success;
                }
                break;
            case SPI_DIR_MODE_TX_RX:
                SPI_MasterWriteOnlyMsg(hdl);
                SPI_MasterReadOnlyMsg(hdl);
                if(end){
                    ret = DRV_OK;
                    goto success;
                }
                break;
        
            default:
                break;
        }
    } while(SPI_CURRENT_TIME < timeout);

    SPI_PrintDebug("------------timeout---------");
    ret = DRV_ERR_TIMEOUT;
    return ret;

success:
    SPI_XferEndFunc(hdl);

    return ret;
}

#ifdef SPI_USE_DMA
static void SPI_RxDmaCb(void *para)
{
    ubase_t level = osInterruptDisable();
    SPI_Handle *hdl = (SPI_Handle *)para;
    if(!hdl->status.busy){
        osInterruptEnable(level);
        SPI_PrintDebug("do not need run dma interrupt server");
        return;
    }
    hdl->status.busy = 0;

    // if((hdl->speConfig & SPI_DIR_MODE_Msk) != SPI_DIR_MODE_TX_ONLY) {
        // while(SPI_IsBusy(hdl->regs));           // 等待FIFO数据搬完
    // }

    osInterruptEnable(level);

    SPI_XferEndFunc(hdl);
}

static int32_t SPI_TxDmaRequest(SPI_Handle *hdl)
{
    SPI_DmaInfo *dma = &hdl->dma;

    if(hdl->ctrl & SPI_TX_DMA_INIT){
        return DRV_OK;
    }

    dma->txHdl = DMA_Request(hdl->res->txLogicID);
    if (!dma->txHdl){
        SPI_PrintError("SPI TX DMA Request Error\r\n");
        OS_ASSERT(0);
    }

    hdl->ctrl |= SPI_TX_DMA_INIT;

    return DRV_OK;
}

#ifdef SPI_USE_TX_LLI
static int32_t SPI_TxDmaLliInit(SPI_Handle *hdl)
{
    SPI_DmaInfo *dma    = &hdl->dma;
    DMA_LliDesc *cfg    = &dma->txLliCfg[0];

    dma->txHdl->callback    = NULL;
    dma->txHdl->para        = (void *)hdl;

    for(uint32_t i = 0; i < SPI_TX_LLI_NUM_MAX; i++) {
        (cfg + i)->DestAddr             = (uint32_t)(&hdl->regs->data);

        (cfg + i)->ZYpara               = 0;
        (cfg + i)->SrcZYstep            = 0;
        (cfg + i)->LLI                  = 0;

        (cfg + i)->Control.Enable       = 1;                        // 是否需要只配置最后一个为1
        (cfg + i)->Control.BurstReqMod  = DMA_RMOD_DEV;
        (cfg + i)->Control.DestMod      = DMA_AMOD_FIFO;
        (cfg + i)->Control.SrcMod       = DMA_AMOD_RAM;
        (cfg + i)->Control.IrqMod       = DMA_IMOD_ALL_ENABLE;      // 是否需要只配置最后一个为all，其他为只处理error中断
        (cfg + i)->Control.IntSel       = DMA_INT1;
        (cfg + i)->Control.SrcBurstLen  = ((hdl->config2 & SPI_TX_THRES_Msk) >> SPI_TX_THRES_Pos) - 1;

        switch (hdl->config & SPI_DATA_BITS_Msk) {
            case SPI_DATA_BITS(8):
                (cfg + i)->Control.SrcBurstSize   = DMA_BSIZE_8BIT;
                (cfg + i)->Control.DestBurstSize  = DMA_BSIZE_8BIT;
                SPI_PrintDebug("set tx dma burst size 8 bit");
                break;
            
            case SPI_DATA_BITS(16):
                (cfg + i)->Control.SrcBurstSize   = DMA_BSIZE_16BIT;
                (cfg + i)->Control.DestBurstSize  = DMA_BSIZE_16BIT;
                SPI_PrintDebug("set tx dma burst size 16 bit");
                break;
            
            default:
                SPI_PrintError("dma do not support this tx burst size %d", (hdl->config & SPI_DATA_BITS_Msk) >> SPI_DATA_BITS_Pos);
                OS_ASSERT(0);
                break;
        }
    }

    return DRV_OK;
}
#endif

static int32_t SPI_TxDmaSingleInit(SPI_Handle *hdl)
{
    SPI_DmaInfo *dma    = &hdl->dma;
    DMA_ChCfg *cfg      = &dma->txCfg;

    dma->txHdl->callback        = NULL;
    dma->txHdl->para            = (void *)hdl;

    cfg->DestAddr               = (uint32_t)(&hdl->regs->data);
    
    cfg->Control.Enable         = 1;
    cfg->Control.BurstReqMod    = DMA_RMOD_DEV;
    cfg->Control.DestMod        = DMA_AMOD_FIFO;
    cfg->Control.SrcMod         = DMA_AMOD_RAM;
    cfg->Control.IrqMod         = DMA_IMOD_ALL_ENABLE;
    cfg->Control.IntSel         = DMA_INT1;
    cfg->Control.SrcBurstLen    = ((hdl->config2 & SPI_TX_THRES_Msk) >> SPI_TX_THRES_Pos) - 1;

    switch (hdl->config & SPI_DATA_BITS_Msk) {
        case SPI_DATA_BITS(8):
            cfg->Control.SrcBurstSize   = DMA_BSIZE_8BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_8BIT;
            SPI_PrintDebug("set tx dma burst size 8 bit\r\n");
            break;
        
        case SPI_DATA_BITS(16):
            cfg->Control.SrcBurstSize   = DMA_BSIZE_16BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_16BIT;
            SPI_PrintDebug("set tx dma burst size 16 bit\r\n");
            break;
        
        default:
            SPI_PrintError("dma do not support this tx burst size %d\r\n", (hdl->config & SPI_DATA_BITS_Msk) >> SPI_DATA_BITS_Pos);
            OS_ASSERT(0);
            break;
    }

    return DRV_OK;
}

static int32_t SPI_RxDmaInit(SPI_Handle *hdl)
{
    SPI_DmaInfo *dma    = &hdl->dma;
    DMA_ChCfg *cfg      = &dma->rxCfg;

    if(hdl->ctrl & SPI_RX_DMA_INIT){
        return DRV_OK;
    }

    dma->rxHdl = DMA_Request(hdl->res->rxLogicID);
    if (!dma->rxHdl){
        SPI_PrintError("SPI RX DMA Request Error\r\n");
        OS_ASSERT(0);
    }
    dma->rxHdl->callback        = NULL;
    dma->rxHdl->para            = (void *)hdl;

    cfg->SrcAddr                = (uint32_t)(&hdl->regs->data);

    cfg->Control.Enable         = 1;
    cfg->Control.BurstReqMod    = DMA_RMOD_DEV;
    cfg->Control.SrcMod         = DMA_AMOD_FIFO;
    cfg->Control.DestMod        = DMA_AMOD_RAM;
    cfg->Control.IrqMod         = DMA_IMOD_ALL_ENABLE;
    cfg->Control.IntSel         = DMA_INT1;
    cfg->Control.SrcBurstLen    = ((hdl->config2 & SPI_RX_THRES_Msk) >> SPI_RX_THRES_Pos) - 1;

    switch (hdl->config & SPI_DATA_BITS_Msk) {
        case SPI_DATA_BITS(8):
            cfg->Control.SrcBurstSize   = DMA_BSIZE_8BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_8BIT;
            SPI_PrintDebug("set rx dma burst size 8 bit\r\n");
            break;
        
        case SPI_DATA_BITS(16):
            cfg->Control.SrcBurstSize   = DMA_BSIZE_16BIT;
            cfg->Control.DestBurstSize  = DMA_BSIZE_16BIT;
            SPI_PrintDebug("set rx dma burst size 16 bit\r\n");
            break;
        
        default:
            SPI_PrintError("dma do not support this rx burst size %d\r\n", (hdl->config & SPI_DATA_BITS_Msk) >> SPI_DATA_BITS_Pos);
            OS_ASSERT(0);
            break;
    }

    hdl->ctrl |= SPI_RX_DMA_INIT;

    return DRV_OK;
}

static void SPI_TxDmaDeInit(SPI_Handle *hdl)
{
    if(!(hdl->ctrl & SPI_TX_DMA_INIT)){
        return;
    }

    if(!hdl->dma.txHdl){
        SPI_PrintError("SPI TX Dma Handle is NULL");
        OS_ASSERT(0);
    }

    DMA_Release(hdl->dma.txHdl);
    // hdl->dma.txHdl = NULL;

    hdl->ctrl &= ~SPI_TX_DMA_INIT;
}

static void SPI_RxDmaDeInit(SPI_Handle *hdl)
{
    if(!(hdl->ctrl & SPI_RX_DMA_INIT)){
        return;
    }

    if(!hdl->dma.rxHdl){
        SPI_PrintError("SPI RX Dma Handle is NULL");
        OS_ASSERT(0);
    }

    DMA_Release(hdl->dma.rxHdl);

    // hdl->dma.rxHdl = NULL;

    hdl->ctrl &= ~SPI_RX_DMA_INIT;
}

static int32_t SPI_TxDmaStart(SPI_Handle *hdl)
{
    DMA_ChCfg *cfg  = &hdl->dma.txCfg;

    cfg->SrcAddr    = (uint32_t)hdl->trans.txMsg.buf;
    cfg->Count      = hdl->trans.txMsg.len;

    osDCacheCleanRange((void *)(cfg->SrcAddr), cfg->Count);

    return DMA_Start(hdl->dma.txHdl, cfg);
}

#ifdef SPI_USE_TX_LLI
static int32_t SPI_TxLliDmaStart(SPI_Handle *hdl)
{
    DMA_LliDesc *cfg    = &hdl->dma.txLliCfg[0];
    SPI_MsgArray *msg   = &hdl->trans.txMsgs;
    uint32_t addr1, addr2;

    for (uint32_t i = 0; i < msg->num; i++) {
        (cfg + i)->SrcAddr = (uint32_t)msg->buf[i];
        (cfg + i)->Count   = msg->len[i];
        (cfg + i)->Control.IrqMod = DMA_IMOD_ALL_DISABLE;

        if(SPI_IS_ALIGNED(msg->buf[i])) {
            osDCacheCleanRange((void *)msg->buf[i], msg->len[i]);
        }
        else {
            addr1 = SPI_ALIGN_LEFT(msg->buf[i]);
            addr2 = SPI_ALIGN_RIGHT(msg->buf[i] + msg->len[i]);
            osDCacheCleanRange((void *)addr1, addr2 - addr1);
        }
    }
    (cfg + msg->num - 1)->Control.Enable = 1;
    (cfg + msg->num - 1)->Control.IrqMod = DMA_IMOD_ALL_ENABLE;

    return DMA_LliStart(hdl->dma.txHdl, cfg, msg->num);
}
#endif

static int32_t SPI_RxDmaStart(SPI_Handle *hdl)
{
    DMA_ChCfg *cfg = &hdl->dma.rxCfg;
    SPI_Msg *msg = &hdl->trans.rxMsg;
    int32_t ret;
    uint32_t addr1, addr2;

    cfg->DestAddr = (uint32_t)msg->buf;
    cfg->Count    = msg->len;

    if(SPI_IS_ALIGNED(msg->buf)) {
        osDCacheInvalidRange((void *)msg->buf, msg->len);
    }
    else {
        addr1 = SPI_ALIGN_LEFT(msg->buf);
        addr2 = SPI_ALIGN_RIGHT(msg->buf + msg->len);
        osDCacheInvalidRange((void *)addr1, addr2 - addr1);
    }

    // ret = DMA_Start(hdl->dma.rxHdl, cfg);
    // osDCacheInvalidRange((void *)(msg->buf), msg->len);       // 需要测试下，是否可以先start，再invalid
    ret = DMA_Start(hdl->dma.rxHdl, cfg);

    return ret;
}

static int32_t SPI_DmaXfer(SPI_Handle *hdl)
{
    // osTick_t tick = osTickGet() + hdl->timeout;
    uint32_t mask = hdl->speConfig & SPI_DIR_MODE_Msk;
    // bool end = false;
    int32_t ret = DRV_ERR;
    SPI_PrintDebug("start master dma transfer");

    // SPI_TxRxDmaDisable(hdl->regs);
    SPI_TxRxFifoClear(hdl->regs);
    SPI_AllIntrClear(hdl->regs);
    SPI_TxRxDmaEnable(hdl->regs);
    // SPI_AllIntrClear(hdl->regs);

    if(mask == SPI_DIR_MODE_TX_ONLY || mask == SPI_DIR_MODE_TX_RX){
        if(hdl->trans.ctrl & SPI_TRANS_CTRL_MULTI_MSG) {
            #ifdef SPI_USE_TX_LLI
                SPI_TxLengthSet(hdl->regs, hdl->trans.txMsgs.total - 1);
                SPI_PrintDebug("lli set tx len = %d", hdl->trans.txMsgs.total);
            #else
                SPI_PrintError("Need Enable SPI_USE_TX_LLI First\r\n");
                OS_ASSERT(0);
            #endif
        }
        else {
            SPI_TxLengthSet(hdl->regs, hdl->trans.txMsg.len - 1);
            SPI_PrintDebug("single set tx len = %d", hdl->trans.txMsg.len);
        }
        // SPI_MasterWriteOnlyMsg(hdl);     // dma模式下是否可以不提前写入fifo
    }

#if 1
    // master模式需要设置接收
    if((hdl->config & SPI_MS_MODE_Msk) == SPI_MS_MODE_MASTER) {
        if(mask == SPI_DIR_MODE_RX_ONLY || mask == SPI_DIR_MODE_TX_RX){
            if((hdl->speConfig & SPI_RX_QTY_MODE_Msk) == SPI_RX_QTY_MODE_MANULE){
                SPI_RxLengthRegSet(hdl->regs, hdl->trans.rxMsg.len - 1);

                if((hdl->speConfig & SPI_RX_CRC16_MODE_Msk) == SPI_RX_CRC16_MODE_AUTO)
                    hdl->trans.rxMsg.len += 2;
            }
        }
    }
#else
    if(mask == SPI_DIR_MODE_RX_ONLY || mask == SPI_DIR_MODE_TX_RX){
        if((hdl->speConfig & SPI_RX_QTY_MODE_Msk) == SPI_RX_QTY_MODE_MANULE){
            SPI_RxLengthRegSet(hdl->regs, hdl->trans.rxMsg.len - 1);

            if((hdl->speConfig & SPI_RX_CRC16_MODE_Msk) == SPI_RX_CRC16_MODE_AUTO)
                hdl->trans.rxMsg.len += 2;
        }
    }
#endif

    // 配置DMA，接着启动DMA
    switch (mask){
        case SPI_DIR_MODE_TX_ONLY:
            if(hdl->trans.ctrl & SPI_TRANS_CTRL_MULTI_MSG) {
                #ifdef SPI_USE_TX_LLI
                    SPI_PrintDebug("start tx lli dma");
                    ret = SPI_TxLliDmaStart(hdl);
                #else
                    SPI_PrintError("Need Enable SPI_USE_TX_LLI First\r\n");
                    OS_ASSERT(0);
                #endif
            }
            else {
                SPI_PrintDebug("start tx single dma");
                ret = SPI_TxDmaStart(hdl);
            }
            break;

        case SPI_DIR_MODE_RX_ONLY:
            ret = SPI_RxDmaStart(hdl);
            break;

        case SPI_DIR_MODE_TX_RX:
            if(hdl->trans.ctrl & SPI_TRANS_CTRL_MULTI_MSG) {
                #ifdef SPI_USE_TX_LLI
                    ret = SPI_TxLliDmaStart(hdl) + SPI_RxDmaStart(hdl);
                #else
                    SPI_PrintError("Need Enable SPI_USE_TX_LLI First\r\n");
                    OS_ASSERT(0);
                #endif
            }
            else {
                ret = SPI_TxDmaStart(hdl) + SPI_RxDmaStart(hdl);
            }
            break;
    
        default:
            break;
    }

    if(ret) {
        DMA_Stop(hdl->dma.txHdl);
        DMA_Stop(hdl->dma.rxHdl);
        return ret;
    }

    if(mask == SPI_DIR_MODE_TX_ONLY || mask == SPI_DIR_MODE_TX_RX){
        while (!SPI_TxValidyGet(hdl->regs)) {
            SPI_PrintDebug("wait fifo not empty...");
        }
    }
    
    SPI_Start(hdl->regs);

    return ret;
}
#endif

static int32_t SPI_DoXfer(SPI_Handle *hdl)
{
    int32_t ret = DRV_ERR;

    if((hdl->config & SPI_TRANS_MODE_Msk) == SPI_TRANS_MODE_DAM) {
        if(DMA_GetStatus(hdl->dma.txHdl)){
            SPI_PrintDebug("tx dma is busy, stop it\r\n");
            DMA_Stop(hdl->dma.txHdl);
        }
        if(DMA_GetStatus(hdl->dma.rxHdl)){
            SPI_PrintDebug("rx dma is busy, stop it\r\n");
            DMA_Stop(hdl->dma.rxHdl);
        }
    }

    // SPI_TxRxFifoClear(hdl->regs);
    // SPI_AllIntrClear(hdl->regs);

    switch (hdl->config & SPI_TRANS_MODE_Msk){
        case SPI_TRANS_MODE_POLL:
            if((hdl->config & SPI_MS_MODE_Msk) == SPI_MS_MODE_MASTER)
                ret = SPI_MasterPollXfer(hdl);
            else
                ret = SPI_SlavePollXfer(hdl);
            break;
        case SPI_TRANS_MODE_INT:
            break;
        case SPI_TRANS_MODE_DAM:
            ret = SPI_DmaXfer(hdl);
            break;
        case SPI_TRANS_MODE_DAM_POLL:
            break;
        case SPI_TRANS_MODE_POLL_DMA:
            break;
    
        default:
            break;
    }

    return ret;
}

static void SPI_InterruptServer(int id, void *param)
{
    SPI_Handle *hdl = (SPI_Handle *)param;
    uint32_t status = hdl->regs->intrSr2;

    SPI_Intr2Clr(hdl->regs, status);

    if(status & (SPI_RXF_OVERRUN_INTR | SPI_TXF_UNDERRUN_INTR)){
        if(status & SPI_RXF_OVERRUN_INTR) {
            // SPI_PrintError("status = 0x%x, tx dma status = 0x%x, rx dma status = 0x%x", status, DMA_GetStatus(hdl->dma.txHdl), DMA_GetStatus(hdl->dma.rxHdl));
            // SPI_PrintError("SPI_RXF_OVERRUN_INTR\r\n");
            // OS_ASSERT(0);
            hdl->trans.err |= SPI_MSG_SLV_RX_OVERRUN;
            // osPrintf("overrun:%d\r\n", ++retrans);
        }

        if(status & SPI_TXF_UNDERRUN_INTR) {
            // SPI_PrintError("status = 0x%x, tx dma status = 0x%x, rx dma status = 0x%x", status, DMA_GetStatus(hdl->dma.txHdl), DMA_GetStatus(hdl->dma.rxHdl));
            // SPI_PrintError("SPI_TXF_UNDERRUN_INTR\r\n");
            // OS_ASSERT(0);
            hdl->trans.err |= SPI_MSG_SLV_TX_UNFINISH;
            // osPrintf("underrun:%d\r\n", ++retrans);
        }
        
    }

    // Master End / Slave End
    if (status & (SPI_MST_DONE_INTR | SPI_SLV_DONE_INTR)){
        ubase_t level = osInterruptDisable();

        if(!hdl->status.busy){
            osInterruptEnable(level);
            SPI_PrintDebug("do not need run spi interrupt server");
            return;
        }
        hdl->status.busy = 0;

        if(!hdl->capabilities.hdlc && (hdl->config & SPI_TRANS_MODE_Msk) == SPI_TRANS_MODE_DAM){
            // if((hdl->speConfig & SPI_DIR_MODE_Msk) == SPI_DIR_MODE_RX_ONLY){
            if(DMA_GetStatus(hdl->dma.txHdl)){
                DMA_Stop(hdl->dma.txHdl);
            }

            if(DMA_GetStatus(hdl->dma.rxHdl)){
                DMA_Stop(hdl->dma.rxHdl);
            }

            hdl->trans.rxMsg.cnt = hdl->trans.rxMsg.len - DMA_GetCount(hdl->dma.rxHdl);
            SPI_MasterReadOnlyMsg(hdl);

            SPI_TxRxDmaDisable(hdl->regs);

            // if((hdl->speConfig & SPI_DIR_MODE_Msk) != SPI_DIR_MODE_TX_ONLY){
            //     while (hdl->trans.rxMsg.len - DMA_GetCount(hdl->dma.rxHdl) != SPI_SlaveRxCnt(hdl->regs));
            //     DMA_Stop(hdl->dma.rxHdl);
            // }
        }

        osInterruptEnable(level);
        SPI_XferEndFunc(hdl);
    }
}

static void SPI_IntrInit(SPI_Handle *hdl)
{
    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum), 2, IRQ_HIGH_LEVEL);
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum), SPI_InterruptServer, hdl);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum));
}

static void SPI_IntrDeInit(SPI_Handle *hdl)
{
    osInterruptMask(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum));
    osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(hdl->res->intNum));
}

int32_t SPI_Initialize(SPI_Handle *hdl)
{
    if(hdl->ctrl & SPI_INIT){
        return DRV_OK;
    }

    g_spiHdl = hdl;

    hdl->config = 0;
    hdl->speConfig = 0;
    hdl->ctrl = 0;
    hdl->regs = (REG_Spi *)hdl->res->regBase;

    uint32_t config = /*SPI_BUS_NUMBER(busNumber) | */
                      /*SPI_CLK_SRC_78M |
                      SPI_CLK_DIV(0) |
                      SPI_MS_MODE_SLAVE |*/
                      SPI_POLPHA_00 |
                      SPI_DATA_BITS(SPI_DEF_TX_WIDTH) |
                      SPI_TRANS_MODE_DAM |
                      SPI_CS_MDOE_MANUAL;
    
    SPI_ClockRequest(hdl);

    SPI_SpeculatEnable(hdl->regs);

    SPI_CommonCfg(hdl, config);

    config = SPI_BIDIR_MODE_UNI_DIR | SPI_DIR_MODE_TX_RX;
    SPI_SpecCfg(hdl, config);

    config = SPI_TX_THRES_VALUE(SPI_DEF_TX_THRES) | SPI_RX_THRES_VALUE(SPI_DEF_RX_THRES) | SPI_TX_VALUE(SPI_DEF_TX_VALUE);
    SPI_ValueCfg(hdl, config);

#ifdef SPI_USE_IRQ
    SSP_IrqInfo *irq                = &hdl->irq;
    irq->priorityMax = 2;
    irq->level = IRQ_HIGH_LEVEL;
    hdl->irq.number = hdl->res->intNum;
#endif

    // SPI_IntrInit(hdl);
    
    SPI_PinMuxInit();

    hdl->ctrl |= SPI_INIT;

    return DRV_OK;
}

int32_t SPI_Uninitialize(SPI_Handle *hdl)
{
    if(!(hdl->ctrl & SPI_INIT)){
        return DRV_OK;
    }

    g_spiHdl = NULL;

    hdl->ctrl &= ~SPI_INIT;

    SPI_IntrDeInit(hdl);

    return DRV_OK;
}

int32_t SPI_PowerControl(SPI_Handle *hdl, DRV_POWER_STATE state)
{
    switch (state){
        case DRV_POWER_FULL:
            if(!(hdl->ctrl & SPI_INIT)){
                SPI_PrintError("Please Init SPI First...");
                return DRV_ERR;
            }

            if(hdl->ctrl & SPI_POWER){
                return DRV_OK;
            }

            SPI_ClockRequest(hdl);
        #ifdef SPI_USE_DMA
            if(!hdl->capabilities.hdlc){
                if((hdl->config & SPI_TRANS_MODE_Msk) == SPI_TRANS_MODE_DAM) {
                    SPI_TxDmaRequest(hdl);
                #ifdef SPI_USE_TX_LLI
                    SPI_TxDmaLliInit(hdl);
                #endif
                    SPI_TxDmaSingleInit(hdl);
                
                    SPI_RxDmaInit(hdl);
                    SPI_IntrInit(hdl);
                }
                else if((hdl->config & SPI_TRANS_MODE_Msk) == SPI_TRANS_MODE_POLL){
                    // do nothing
                }
                else {
                    SPI_IntrInit(hdl);
                }
            }
            
        #endif

            hdl->ctrl |= SPI_POWER;
            break;

        case DRV_POWER_LOW:
            return DRV_ERR_UNSUPPORTED;

        case DRV_POWER_OFF:
            if(!(hdl->ctrl & SPI_POWER)){
                return DRV_OK;
            }

        #ifdef SPI_USE_DMA
            if(!hdl->capabilities.hdlc){
                if((hdl->config & SPI_TRANS_MODE_Msk) == SPI_TRANS_MODE_DAM) {
                    if(hdl->dma.txHdl && DMA_GetStatus(hdl->dma.txHdl)){
                        SPI_PrintError("tx dma is busy, stop it\r\n");
                        DMA_Stop(hdl->dma.txHdl);
                    }
                    if(hdl->dma.rxHdl && DMA_GetStatus(hdl->dma.rxHdl)){
                        SPI_PrintError("rx dma is busy, stop it\r\n");
                        DMA_Stop(hdl->dma.rxHdl);
                    }
                }
                SPI_TxDmaDeInit(hdl);
                SPI_RxDmaDeInit(hdl);
            }
        #endif

            SPI_ClockRelease(hdl);

            hdl->ctrl &= ~(SPI_POWER);
            break;
        
        default:
            break;
    }
    

    return DRV_OK;
}

int32_t SPI_Transfer(SPI_Handle *hdl, uint8_t *txData, uint32_t txLen, uint8_t *rxData, uint32_t rxLen, uint32_t timeout)
{
    if(hdl->capabilities.hdlc){
        hdl->status.busy = 1;
        SPI_TxLengthSet(hdl->regs, txLen - 1);
        SPI_Start(hdl->regs);
        return DRV_OK;
    }

    SPI_Msg *txMsg = &hdl->trans.txMsg;
    SPI_Msg *rxMsg = &hdl->trans.rxMsg;

    OS_ASSERT((txData && txLen) || !txData);
    OS_ASSERT((rxData && rxLen) || !rxData);

    if(!txData && !rxData){
        SPI_PrintError("SPI Transmit Data NULL");
        return DRV_ERR;
    }

    if(SPI_IsBusy(hdl->regs)){
        SPI_PrintError("SSP In Busy");
        return DRV_ERR;
    }

    hdl->status.val = 0;// 清空状态

    hdl->timeout = timeout;
    hdl->status.busy = 1;

    memset(&hdl->trans, 0, sizeof(hdl->trans));

    // 清空收发消息
    if(txData){
        txMsg->buf = txData;
        txMsg->len = txLen;
    }

    if(rxData){
        rxMsg->buf = rxData;
        rxMsg->len = rxLen;
    }

    hdl->trans.ctrl &= (~SPI_TRANS_CTRL_MULTI_MSG);

    return SPI_DoXfer(hdl);
}

static int32_t SPI_LliTransfer(SPI_Handle *hdl, SPI_MsgArray *txArray, uint8_t *rxData, uint32_t rxLen, uint32_t timeout)
{
    if(hdl->capabilities.hdlc){
        return DRV_ERR;
    }

    SPI_MsgArray *txMsgs = &hdl->trans.txMsgs;
    SPI_Msg *rxMsg       = &hdl->trans.rxMsg;
    uint32_t total       = 0;

    OS_ASSERT(txArray->num <= SPI_TX_LLI_NUM_MAX);

    for(uint32_t i = 0; i < txArray->num; i++) {
        total += txArray->len[i];
        OS_ASSERT(txArray->buf[i] != NULL);
    }

    OS_ASSERT(txArray->total == total);
    OS_ASSERT((rxData && rxLen) || !rxData);

    // if(!txData && !rxData){
    //     SPI_PrintError("SPI Transmit Data NULL");
    //     return DRV_ERR;
    // }

    if(SPI_IsBusy(hdl->regs)){
        SPI_PrintError("SSP In Busy");
        return DRV_ERR;
    }

    hdl->status.val = 0;// 清空状态

    hdl->timeout = timeout;
    hdl->status.busy = 1;

    // 清空收发消息
    memset(&hdl->trans, 0, sizeof(hdl->trans));

    memcpy(txMsgs, txArray, sizeof(SPI_MsgArray));
    txMsgs->cnt = 0;
    txMsgs->crc = 0;

    if(rxData){
        rxMsg->buf = rxData;
        rxMsg->len = rxLen;
    }

    hdl->trans.ctrl |= SPI_TRANS_CTRL_MULTI_MSG;

    return SPI_DoXfer(hdl);
}

int32_t SPI_Control(SPI_Handle *hdl, uint32_t control, uint32_t arg)
{
    switch (control){
        case SPI_CONFIG_COMMON:
            SPI_CommonCfg(hdl, arg);
            break;

        case SPI_CONFIG_SPEC:
            SPI_SpecCfg(hdl, arg);
            break;

        case SPI_CONFIG_TRANS:
            SPI_CrcParamCfg(hdl, arg);
            break;

        case SPI_CONFIG_VAL:
            SPI_ValueCfg(hdl, arg);
        
        default:
            return DRV_ERR_UNSUPPORTED;
    }

    return DRV_OK;
}

SPI_STATUS SPI_GetStatus(SPI_Handle *hdl)
{
    return hdl->status;
}

uint16_t SPI_GetRxCrc16(SPI_Handle *hdl)
{
    if((hdl->speConfig & SPI_RX_CRC16_MODE_Msk) != SPI_RX_CRC16_MODE_AUTO){
        return 0;
    }

    return hdl->trans.rxMsg.crc;
}

uint16_t SPI_GetTxCrc16(SPI_Handle *hdl)
{
    if((hdl->speConfig & SPI_TX_CRC16_MODE_Msk) != SPI_TX_CRC16_MODE_AUTO){
        hdl->trans.txMsg.crc = SPI_CrcTxStaGet(hdl->regs);
    }

    return hdl->trans.txMsg.crc;
}

uint32_t SPI_SendLengthActual(SPI_Handle *hdl)
{
    return hdl->trans.txMsg.cnt;
}

uint32_t SPI_RecvLengthActual(SPI_Handle *hdl)
{
    return hdl->trans.rxMsg.cnt;
}

int32_t SPI_MasterSend(SPI_Handle *hdl, uint8_t *data, uint32_t len, bool crc, uint32_t timeout)
{
    uint32_t cfg = SPI_DIR_MODE_TX_ONLY;
    crc ? (cfg |= SPI_TX_CRC16_MODE_AUTO) : (cfg |= SPI_TX_CRC16_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_MASTER);
    SPI_SpecCfg(hdl, cfg);

    return SPI_Transfer(hdl, data, len, NULL, 0, timeout);
}

int32_t SPI_SlaveSend(SPI_Handle *hdl, uint8_t *data, uint32_t len, bool crc, uint32_t timeout)
{
    uint32_t cfg = SPI_DIR_MODE_TX_ONLY;
    crc ? (cfg |= SPI_TX_CRC16_MODE_AUTO) : (cfg |= SPI_TX_CRC16_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_SLAVE);
    SPI_SpecCfg(hdl, cfg);

    return SPI_Transfer(hdl, data, len, NULL, 0, timeout);
}

int32_t SPI_SlaveSendExtFunc(SPI_Handle *hdl, uint8_t *data, uint32_t len, void (*extFunc)(void *), void *extParam, bool crc, uint32_t timeout)
{
    int ret;
    uint32_t cfg = SPI_DIR_MODE_TX_ONLY;
    crc ? (cfg |= SPI_TX_CRC16_MODE_AUTO) : (cfg |= SPI_TX_CRC16_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_SLAVE);
    SPI_SpecCfg(hdl, cfg);

    hdl->extFunc = extFunc;
    hdl->extParam = extParam;

    ret = SPI_Transfer(hdl, data, len, NULL, 0, timeout);

    hdl->extFunc = NULL;
    hdl->extParam = NULL;

    return ret;
}

// 如果使能了CRC，需要保证data中有额外的两个字节存放CRC
// 如果使能硬解长度域，需要保证buffer足够大
int32_t SPI_MasterRecv(SPI_Handle *hdl, uint8_t *data, uint32_t len, bool crc, bool qty, uint32_t timeout)
{
    uint32_t cfg = SPI_DIR_MODE_RX_ONLY;
    crc ? (cfg |= SPI_RX_CRC16_MODE_AUTO) : (cfg |= SPI_RX_CRC16_MODE_MANULE);
    qty ? (cfg |= SPI_RX_QTY_MODE_AUTO)   : (cfg |= SPI_RX_QTY_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_MASTER);
    SPI_SpecCfg(hdl, cfg);

    return SPI_Transfer(hdl, NULL, 0, data, len, timeout);
}

/**
 * @brief Slave接收
 * 
 * @param hdl 
 * @param data 
 * @param len           允许最大的接收长度，超过将丢弃
 * @param timeout 
 * @return int32_t 
 */
int32_t SPI_SlaveRecv(SPI_Handle *hdl, uint8_t *data, uint32_t len, bool crc, bool qty, uint32_t timeout)
{
    uint32_t cfg = SPI_DIR_MODE_RX_ONLY;
    crc ? (cfg |= SPI_RX_CRC16_MODE_AUTO) : (cfg |= SPI_RX_CRC16_MODE_MANULE);
    qty ? (cfg |= SPI_RX_QTY_MODE_AUTO)   : (cfg |= SPI_RX_QTY_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_SLAVE);
    SPI_SpecCfg(hdl, cfg);

    return SPI_Transfer(hdl, NULL, 0, data, len, timeout);
}

int32_t SPI_SlaveRecvExcFunc(SPI_Handle *hdl, uint8_t *data, uint32_t len, void (*extFunc)(void *), void *extParam, bool crc, bool qty, uint32_t timeout)
{
    int ret;
    uint32_t cfg = SPI_DIR_MODE_RX_ONLY;
    crc ? (cfg |= SPI_RX_CRC16_MODE_AUTO) : (cfg |= SPI_RX_CRC16_MODE_MANULE);
    qty ? (cfg |= SPI_RX_QTY_MODE_AUTO)   : (cfg |= SPI_RX_QTY_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_SLAVE);
    SPI_SpecCfg(hdl, cfg);

    hdl->extFunc = extFunc;
    hdl->extParam = extParam;

    ret = SPI_Transfer(hdl, NULL, 0, data, len, timeout);

    hdl->extFunc = NULL;
    hdl->extParam = NULL;

    return ret;
}

int32_t SPI_MasterTransfer(SPI_Handle *hdl, uint8_t *txData, uint32_t txLen, bool txCrc,
                                            uint8_t *rxData, uint32_t rxLen, bool rxCrc, bool rxQty, 
                                            uint32_t timeout)
{
    uint32_t cfg = SPI_DIR_MODE_TX_RX;

    txCrc ? (cfg |= SPI_TX_CRC16_MODE_AUTO) : (cfg |= SPI_TX_CRC16_MODE_MANULE);
    rxCrc ? (cfg |= SPI_RX_CRC16_MODE_AUTO) : (cfg |= SPI_RX_CRC16_MODE_MANULE);
    rxQty ? (cfg |= SPI_RX_QTY_MODE_AUTO)   : (cfg |= SPI_RX_QTY_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_MASTER);
    SPI_SpecCfg(hdl, cfg);

    return SPI_Transfer(hdl, txData, txLen, rxData, rxLen, timeout);
}

int32_t SPI_SlaveTransfer(SPI_Handle *hdl, uint8_t *txData, uint32_t txLen, bool txCrc,
                                            uint8_t *rxData, uint32_t rxLen, bool rxCrc, bool rxQty, 
                                            uint32_t timeout)
{
    uint32_t cfg = SPI_DIR_MODE_TX_RX;

    txCrc ? (cfg |= SPI_TX_CRC16_MODE_AUTO) : (cfg |= SPI_TX_CRC16_MODE_MANULE);
    rxCrc ? (cfg |= SPI_RX_CRC16_MODE_AUTO) : (cfg |= SPI_RX_CRC16_MODE_MANULE);
    rxQty ? (cfg |= SPI_RX_QTY_MODE_AUTO)   : (cfg |= SPI_RX_QTY_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_SLAVE);
    SPI_SpecCfg(hdl, cfg);

    return SPI_Transfer(hdl, txData, txLen, rxData, rxLen, timeout);
}

int32_t SPI_MasterLliSend(SPI_Handle *hdl, SPI_MsgArray *txArray, bool crc, uint32_t timeout)
{
    uint32_t cfg = SPI_DIR_MODE_TX_ONLY;
    crc ? (cfg |= SPI_TX_CRC16_MODE_AUTO) : (cfg |= SPI_TX_CRC16_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_MASTER);
    SPI_SpecCfg(hdl, cfg);

    return SPI_LliTransfer(hdl, txArray, NULL, 0, timeout);
}

int32_t SPI_MasterLliTransfer(SPI_Handle *hdl, SPI_MsgArray *txArray, bool txCrc,
                                               uint8_t *rxData, uint32_t rxLen, bool rxCrc, bool rxQty, 
                                               uint32_t timeout)
{
    uint32_t cfg = SPI_DIR_MODE_TX_RX;

    txCrc ? (cfg |= SPI_TX_CRC16_MODE_AUTO) : (cfg |= SPI_TX_CRC16_MODE_MANULE);
    rxCrc ? (cfg |= SPI_RX_CRC16_MODE_AUTO) : (cfg |= SPI_RX_CRC16_MODE_MANULE);
    rxQty ? (cfg |= SPI_RX_QTY_MODE_AUTO)   : (cfg |= SPI_RX_QTY_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_MASTER);
    SPI_SpecCfg(hdl, cfg);

    return SPI_LliTransfer(hdl, txArray, rxData, rxLen, timeout);
}

int32_t SPI_SlaveLliTransfer(SPI_Handle *hdl, SPI_MsgArray *txArray, bool txCrc,
                                              uint8_t *rxData, uint32_t rxLen, bool rxCrc, bool rxQty, 
                                              uint32_t timeout)
{
    uint32_t cfg = SPI_DIR_MODE_TX_RX;

    txCrc ? (cfg |= SPI_TX_CRC16_MODE_AUTO) : (cfg |= SPI_TX_CRC16_MODE_MANULE);
    rxCrc ? (cfg |= SPI_RX_CRC16_MODE_AUTO) : (cfg |= SPI_RX_CRC16_MODE_MANULE);
    rxQty ? (cfg |= SPI_RX_QTY_MODE_AUTO)   : (cfg |= SPI_RX_QTY_MODE_MANULE);

    SPI_CommonCfg(hdl, SPI_MS_MODE_SLAVE);
    SPI_SpecCfg(hdl, cfg);

    return SPI_LliTransfer(hdl, txArray, rxData, rxLen, timeout);
}

uint32_t SPI_SlaveRxCntGet(SPI_Handle *hdl)
{
    uint32_t len, addr1, addr2;//, tmp1;
    // int32_t ret          = -1;
    uint32_t actual      = SPI_SlaveRxCnt(hdl->regs);
    // SPI_NEW_TIME timeout = SPI_CURRENT_TIME + hdl->timeout;
    // return actual;
    return hdl->trans.rxMsg.cnt;

    if((hdl->speConfig & SPI_DIR_MODE_Msk) != SPI_DIR_MODE_TX_ONLY){
        // 如果硬解长度域，必须用这种方法判断
        if(SPI_RX_QTY_MODE_AUTO == (hdl->speConfig & SPI_RX_QTY_MODE_Msk)) {
            if(SPI_RX_CRC16_MODE_AUTO == (hdl->speConfig & SPI_RX_CRC16_MODE_Msk)) {
                len = *(uint16_t *)(hdl->trans.rxMsg.buf + 2) + 6;
            }
            else {
                len = *(uint16_t *)(hdl->trans.rxMsg.buf + 2) + 4;
            }

            // 未对齐的需要丢弃，说明传输出现了异常
            if((len % SPI_DEF_RX_THRES) || (actual % SPI_DEF_RX_THRES)) {
                DMA_Stop(hdl->dma.rxHdl);
                // hdl->trans.rxMsg.cnt = hdl->trans.rxMsg.len - DMA_GetCount(hdl->dma.rxHdl);
                // SPI_ReadFifo(hdl);
                // SPI_PrintError("len= %d, actual = %d", len, actual);
                return 0;
            }

            if(len > actual) {
                len = actual;
            }
        }
        else {
            len = actual;
        }
        // do {
        //     tmp1 = hdl->trans.rxMsg.len - DMA_GetCount(hdl->dma.rxHdl);
        //     if (tmp1 == len /*|| (tmp1 == len - 2)*/) {
        //         ret = 0;
        //         break;
        //     }
        // } while (SPI_CURRENT_TIME < timeout);

        // // 必须强制死机
        // if(ret) {
        //     SPI_PrintError("Wait Timeout, Need Check!!!");
        //     SPI_PrintError("%d %d %d %d %d %d", 
        //                     hdl->trans.rxMsg.len, 
        //                     DMA_GetCount(hdl->dma.rxHdl), 
        //                     len, 
        //                     actual, 
        //                     *(uint16_t *)(hdl->trans.rxMsg.buf + 2), 
        //                     SPI_RxValidGet(hdl->regs));
        //     // PriPrintHex("rx", 32, hdl->trans.rxMsg.buf, 32);
        //     // OS_ASSERT(0);
        // }

        // DMA_Stop(hdl->dma.rxHdl);

        // hdl->trans.rxMsg.cnt = hdl->trans.rxMsg.len - DMA_GetCount(hdl->dma.rxHdl);
        // SPI_ReadMsg(hdl);

        // 防止dma搬运了fifo残留数据，需要再次invalid
        if(SPI_IS_ALIGNED(hdl->trans.rxMsg.buf)) {
            osDCacheInvalidRange((void *)hdl->trans.rxMsg.buf, len);
        }
        else {
            addr1 = SPI_ALIGN_LEFT(hdl->trans.rxMsg.buf);
            addr2 = SPI_ALIGN_RIGHT(hdl->trans.rxMsg.buf + len);
            osDCacheInvalidRange((void *)addr1, addr2 - addr1);
        }

        return len;
    }

    return actual;
}


#ifdef OS_USING_PM
#include <drv_psm_sys.h>

static int SPI_Suspend(void *param, PSM_Mode mode,uint32_t *save_addr)
{
    int i = 0;
    if(mode == PSM_DEEP_SLEEP && g_spiHdl) {
        *(save_addr + (i++)) = READ_REG(g_spiHdl->regs->protocol);
        *(save_addr + (i++)) = READ_REG(g_spiHdl->regs->rxCtrl);
        *(save_addr + (i++)) = READ_REG(g_spiHdl->regs->crcCtrl0);
        *(save_addr + (i++)) = READ_REG(g_spiHdl->regs->crcCtrl1);
        *(save_addr + (i++)) = READ_REG(g_spiHdl->regs->fifoCsr);
        *(save_addr + (i++)) = READ_REG(g_spiHdl->regs->intrEn);
        *(save_addr + (i++)) = READ_REG(g_spiHdl->regs->intrEn2);
    }
    return i * 4;
}

static int SPI_Resume(void *param, PSM_Mode mode,uint32_t *save_addr)
{
    int i = 0;
    if(mode == PSM_DEEP_SLEEP && g_spiHdl) {
        WRITE_REG(g_spiHdl->regs->protocol, *(save_addr + (i++)));
        WRITE_REG(g_spiHdl->regs->rxCtrl, *(save_addr + (i++)));
        WRITE_REG(g_spiHdl->regs->crcCtrl0, *(save_addr + (i++)));
        WRITE_REG(g_spiHdl->regs->crcCtrl1, *(save_addr + (i++)));
        WRITE_REG(g_spiHdl->regs->fifoCsr, *(save_addr + (i++)));
        WRITE_REG(g_spiHdl->regs->intrEn, *(save_addr + (i++)));
        WRITE_REG(g_spiHdl->regs->intrEn2, *(save_addr + (i++)));
    }
    return i * 4;
}

static PSM_DpmOps g_spi_dpmops = {
    .PsmSuspendNoirq = SPI_Suspend,
    .PsmResumeNoirq = SPI_Resume,
};

PSM_DPM_INFO_DEFINE(spi, g_spi_dpmops, OS_NULL, PSM_LEVEL_HIGH);

#endif