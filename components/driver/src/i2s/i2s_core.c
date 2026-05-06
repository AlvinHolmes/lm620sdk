#include "i2s_core.h"
#include "drv_soc.h"
#include "os.h"

static I2S_CoreHandle *g_i2sCoreHandle = NULL;

static void I2S_ClockConfig(void)
{
    uint32_t configCodecClk = 0;
    uint32_t divInter = 0;
    uint64_t divFloat = 0;

    CLK_SetPdcoreLspCrmRegs(CLK_I2S_SW_WCLK_EN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_SW_PCLK_EN, 1);
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_WCLK_SEL_HL, 1);

    CLK_SetTopCrmRegs(CLK_AU_MCLK_DIV_SEL, 0);                  // top crm 不分频
    CLK_SetTopCrmRegs(CLK_AU_MCLK_SEL, AU_MCLK_SEL_WID);        // codec clk 作为 au_mclk 输出
    CLK_SetTopCrmRegs(CLK_AU_MCLK_EN, 1);                       // au_mclk 输出使能

    WRITE_U32(LSP_CRM_ADDRBASE + (CLK_I2S_FRAC_DIV1 >> 16), 0);
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_FRAC_DIV2, 0);

    if(g_i2sCoreHandle->cfg->timing == TIMING_I2S)
    {
        I2S_TimingCfg *i2sTimingCfg = (I2S_TimingCfg *)g_i2sCoreHandle->cfg->timingCfg;
        configCodecClk = g_i2sCoreHandle->cfg->sampleRate * g_i2sCoreHandle->cfg->fs;
        divInter = g_i2sCoreHandle->cfg->fs / (i2sTimingCfg->dataCycle * 2);

    }       
    else if(g_i2sCoreHandle->cfg->timing == TIMING_PCM)
    {
        PCM_TimingCfg *pcmTimingCfg = (PCM_TimingCfg *)g_i2sCoreHandle->cfg->timingCfg;
        configCodecClk = g_i2sCoreHandle->cfg->sampleRate * g_i2sCoreHandle->cfg->fs; 
        divInter = g_i2sCoreHandle->cfg->fs / (pcmTimingCfg->slotCycle * pcmTimingCfg->slotNum);
    }

    if(divInter == 1)
    {
        configCodecClk = configCodecClk * 2;
        divInter += 1;
    }
    //整数分频器
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_WCLK_DIV, divInter - 1);

    //小数分频器
    g_i2sCoreHandle->workClk = MAIN_CLOCK;
    divInter = (g_i2sCoreHandle->workClk / configCodecClk);
    divFloat = (uint64_t)(g_i2sCoreHandle->workClk % configCodecClk) * 100000 / configCodecClk;  // 保留 5 位
    divFloat = (uint16_t)(divFloat * 0xFFFF / 100000);          // 0xFFFF 最大分母
    WRITE_U32(LSP_CRM_ADDRBASE + (CLK_I2S_FRAC_DIV1 >> 16), divFloat | (0xFFFF << 16));
    //CLK_SetPdcoreLspCrmRegs(CLK_I2S_FRAC_DIV1, divFloat | (100 << 16));
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_FRAC_DIV2, (divInter) | (1 << 16));
}

static void I2S_ModuleEnable(bool enable)
{
    if(enable)
        WRITE_REG(g_i2sCoreHandle->reg->processCtrl, READ_REG(g_i2sCoreHandle->reg->processCtrl) | I2S_PROCESS_CTRL_EN_MASK);
    else
        WRITE_REG(g_i2sCoreHandle->reg->processCtrl, READ_REG(g_i2sCoreHandle->reg->processCtrl) & ~I2S_PROCESS_CTRL_EN_MASK);
}

static void I2S_DmaConfig(void)
{
    uint32_t reg_val = 0;

    reg_val = READ_REG(g_i2sCoreHandle->reg->fifoCtrl);
    
    if(g_i2sCoreHandle->cfg->workMode & I2S_PLAY_MODE)
    {
        reg_val &= (~I2S_FIFO_TX_THRES_MASK);
        reg_val |= ((I2S_FIFO_THRESHOLD - 1) << I2S_FIFO_TX_THRES_POS);
        reg_val |= I2S_FIFO_TX_DMA_EN_MASK;
    }

    if(g_i2sCoreHandle->cfg->workMode & I2S_RECORD_MODE)
    {
        reg_val &= (~I2S_FIFO_RX0_THRES_MASK);
        reg_val |= ((I2S_FIFO_THRESHOLD - 1) << I2S_FIFO_RX0_THRES_POS);
        reg_val |= I2S_FIFO_RX0_DMA_EN_MASK;
    }   

    WRITE_REG(g_i2sCoreHandle->reg->fifoCtrl, reg_val);
}

void I2S_TxEnable(bool enable)
{
    if(enable)
        WRITE_REG(g_i2sCoreHandle->reg->processCtrl, READ_REG(g_i2sCoreHandle->reg->processCtrl) | I2S_PROCESS_CTRL_TX_EN_MASK);
    else
        WRITE_REG(g_i2sCoreHandle->reg->processCtrl, READ_REG(g_i2sCoreHandle->reg->processCtrl) & ~I2S_PROCESS_CTRL_TX_EN_MASK);
}

void I2S_RxEnable(bool enable)
{
    if(enable)
        WRITE_REG(g_i2sCoreHandle->reg->processCtrl, READ_REG(g_i2sCoreHandle->reg->processCtrl) | I2S_PROCESS_CTRL_RX_EN_MASK);
    else
        WRITE_REG(g_i2sCoreHandle->reg->processCtrl, READ_REG(g_i2sCoreHandle->reg->processCtrl) & ~I2S_PROCESS_CTRL_RX_EN_MASK);   
}

int8_t I2S_Config(I2S_CoreHandle *coreHandle)
{
    uint32_t reg_val = 0;

    if(!g_i2sCoreHandle)
    {
        g_i2sCoreHandle = coreHandle;
        g_i2sCoreHandle->reg = (I2S_Reg *)(g_I2S_Res[0].regBase);
    }
    I2S_ClockConfig();
    I2S_ModuleEnable(false);
    //master or slave mode
    if(!g_i2sCoreHandle->cfg->isSlave)
        reg_val |= I2S_TIMING_MASTER_MODE_MASK;
    //loop back disable
    reg_val &= (~I2S_TIMING_LOOP_BACK_MASK);
    // pha
    reg_val &= ~I2S_TIMING_PHA_SEL_MASK;
    //timing
    reg_val &= (~I2S_TIMING_TIMING_SEL_MASK);
    if(g_i2sCoreHandle->cfg->timing == TIMING_PCM)
    {
        reg_val |= I2S_TIMING_TIMING_SEL_MASK;
    }
    //fsync
    reg_val &= (~I2S_TIMING_LONG_FSYNC_MASK);
    //extra cycle
    reg_val &= (~I2S_TIMING_EXTRA_CYCLE_MASK);
    //msb
    reg_val &= (~I2S_TIMING_ALIGN_MODE_MASK);
    reg_val |= (1 << I2S_TIMING_ALIGN_MODE_POS);    // default msb

    //lane num
    reg_val &= ~I2S_TIMING_LANE_NUM_MASK;

    if(g_i2sCoreHandle->cfg->timing == TIMING_PCM)
    {
        PCM_TimingCfg *pcmCfg = (PCM_TimingCfg *)g_i2sCoreHandle->cfg->timingCfg;
        //chnl num
        reg_val &= (~I2S_TIMING_CHNL_NUM_MASK);
        reg_val |= ((pcmCfg->slotNum - 1) << I2S_TIMING_CHNL_NUM_POS);
        //TS cfg
        reg_val &= (~I2S_TIMING_TS_CFG_MASK);
        reg_val |= ((pcmCfg->slotNum - 1) << I2S_TIMING_TS_CFG_POS);
        //TS width
        reg_val &= (~I2S_TIMING_TS_WIDTH_MASK);
        reg_val |= ((pcmCfg->slotCycle - 1) << I2S_TIMING_TS_WIDTH_POS);
        //data size
        reg_val &= (~I2S_TIMING_DATA_SIZE_MASK);
        reg_val |= ((pcmCfg->dataBits - 1) << I2S_TIMING_DATA_SIZE_POS);   

        switch (pcmCfg->transTiming)
        {
        case TIMING_PCM_MSB_JUSTIF:
            reg_val &= (~I2S_TIMING_ALIGN_MODE_MASK);
            reg_val |= (1 << I2S_TIMING_ALIGN_MODE_POS);
            break;
        case TIMING_PCM_LSB_JUSTIF:
            reg_val &= (~I2S_TIMING_ALIGN_MODE_MASK);
            reg_val |= (2 << I2S_TIMING_ALIGN_MODE_POS);
            break;
        case TIMING_PCM_STANDARD:
            reg_val &= (~I2S_TIMING_ALIGN_MODE_MASK);
            break;
        default:
            break;
        }
    }
    else
    {
        I2S_TimingCfg *i2sCfg = (I2S_TimingCfg *)g_i2sCoreHandle->cfg->timingCfg;
        reg_val &= (~I2S_TIMING_CHNL_NUM_MASK);
        reg_val &= (~I2S_TIMING_TS_CFG_MASK);
        reg_val |= ((i2sCfg->chnl) << I2S_TIMING_TS_CFG_POS);

        if(i2sCfg->chnl == I2S_DOUBLE_CHNL)
        {
            reg_val |= (1 << I2S_TIMING_CHNL_NUM_POS);
        }
        //TS width
        reg_val &= (~I2S_TIMING_TS_WIDTH_MASK);
        reg_val |= ((i2sCfg->dataCycle - 1) << I2S_TIMING_TS_WIDTH_POS);
        //data size
        reg_val &= (~I2S_TIMING_DATA_SIZE_MASK);
        reg_val |= ((i2sCfg->dataBits - 1) << I2S_TIMING_DATA_SIZE_POS);

        switch (i2sCfg->transTiming)
        {
        case TIMING_I2S_MSB_JUSTIF:
            reg_val &= (~I2S_TIMING_ALIGN_MODE_MASK);
            reg_val |= (1 << I2S_TIMING_ALIGN_MODE_POS);
            break;
        case TIMING_I2S_LSB_JUSTIF:
            reg_val &= (~I2S_TIMING_ALIGN_MODE_MASK);
            reg_val |= (2 << I2S_TIMING_ALIGN_MODE_POS);
            break;
        case TIMING_I2S_STANDARD:
            reg_val &= (~I2S_TIMING_ALIGN_MODE_MASK);
            break;
        case TIMING_I2S_NORMAL:
            reg_val &= (~I2S_TIMING_ALIGN_MODE_MASK);
            reg_val |= (3 << I2S_TIMING_ALIGN_MODE_POS);
            break;
        default:
            break;
        }
    }

    reg_val &= (~I2S_TIMING_SMJZ_MODE_MASK);

    WRITE_REG(g_i2sCoreHandle->reg->timingCtrl, reg_val);
    I2S_ModuleEnable(true);

    for(volatile uint8_t i = 0; i < 10; i++);

    if(READ_REG(g_i2sCoreHandle->reg->timingCtrl) & I2S_TIMING_FORMAT_ERROR_MASK)
        return DRV_ERR;
    
    I2S_DmaConfig();
    return DRV_OK;
}

void I2S_DeConfig(void)
{
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_SW_WCLK_EN, 0);
    CLK_SetPdcoreLspCrmRegs(CLK_I2S_SW_PCLK_EN, 0);
    CLK_SetTopCrmRegs(CLK_AU_MCLK_EN, 0);   
    if(g_i2sCoreHandle)
    {
        if(g_i2sCoreHandle->cfg)
        {
            osFree(g_i2sCoreHandle->cfg);
        }
        osFree(g_i2sCoreHandle);
        g_i2sCoreHandle = NULL;
    }
}

uint32_t I2S_GetDataFifoAddr(void)
{
    return (uint32_t)&(g_i2sCoreHandle->reg->data);
}

