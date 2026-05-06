
#include "wdt_core.h"
#include "drv_soc.h"

uint8_t WDT_Initlize(WDT_CoreHandle *handle)
{
    handle->reg = (WDT_Reg *)g_WDT_Res[0].regBase;  

    CLK_SetCoreCsrReg(CLK_WDT_PCLK_EN, 1);
    CLK_SetCoreCsrReg(CLK_WDT_WCLK_EN, 1);
    CLK_SetCoreCsrReg(CLK_WDT_CLK_SEL, PWM_TIMER_CLK_32K);
    CLK_SetCoreCsrReg(CLK_WDT_CLK_DIV, 0);
    return DRV_OK;
}

uint8_t WDT_UnInitlize(WDT_CoreHandle *handle)
{
    handle->reg = (WDT_Reg *)g_WDT_Res[0].regBase;

    WRITE_REG(handle->reg->startReg,  WDT_WRT_KEY << WDT_WRT_KEY_POS);

    CLK_SetCoreCsrReg(CLK_WDT_PCLK_EN, 0);
    CLK_SetCoreCsrReg(CLK_WDT_WCLK_EN, 0);

    return DRV_OK;
}

__IRAM_CODE_PSM_RE uint8_t WDT_Start(WDT_CoreHandle *handle)
{
    WRITE_REG(handle->reg->startReg,  WDT_START_MASK | (WDT_WRT_KEY << WDT_WRT_KEY_POS));
    return DRV_OK;
}

__IRAM_CODE_PSM_RE uint8_t WDT_Stop(WDT_CoreHandle *handle)
{
    WRITE_REG(handle->reg->startReg,  WDT_WRT_KEY << WDT_WRT_KEY_POS);
    return DRV_OK;
}

__IRAM_CODE_PSM_RE uint8_t WDT_Config(WDT_CoreHandle *handle)
{
    uint32_t regVal = 0;

    //set div
    regVal = (WDT_CLK_DIV - 1) | (WDT_WRT_KEY << WDT_WRT_KEY_POS);
    WRITE_REG(handle->reg->cfgReg, regVal);
#ifdef WDT_FPGA
    //set load
    WRITE_REG(handle->reg->loadValReg,  handle->userData.timeOutPeriod * 1000 | (WDT_WRT_KEY << WDT_WRT_KEY_POS));
    //set compare
    WRITE_REG(handle->reg->compareValReg,  WDT_CMP_VALUE * 1000 | (WDT_WRT_KEY << WDT_WRT_KEY_POS));
#else
        //set load
    WRITE_REG(handle->reg->loadValReg,  handle->userData.timeOutPeriod | (WDT_WRT_KEY << WDT_WRT_KEY_POS));
    //set compare
    WRITE_REG(handle->reg->compareValReg,  WDT_CMP_VALUE | (WDT_WRT_KEY << WDT_WRT_KEY_POS));
#endif

    //refresh
    regVal = READ_REG(handle->reg->setEnReg);
    regVal &= 0x0000FFFF;
    regVal = (regVal ^ WDT_SET_EN_MASK) & WDT_SET_EN_MASK;
    WRITE_REG(handle->reg->setEnReg, regVal | (WDT_WRT_KEY << WDT_WRT_KEY_POS));
    //check ack
    while((READ_REG(handle->reg->setEnReg) & WDT_SET_EN_MASK) != ((READ_REG(handle->reg->staReg) >> 4) & WDT_SET_EN_MASK));
    
    return DRV_OK;
}


uint32_t WDT_CountGet(WDT_CoreHandle *handle)
{
    return READ_REG(handle->reg->currentValReg);
}

uint32_t WDT_LoadGet(WDT_CoreHandle *handle)
{
    return READ_REG(handle->reg->loadValReg);
}

uint32_t WDT_TimeoutGet(WDT_CoreHandle *handle)
{
    return READ_REG(handle->reg->compareValReg);
}