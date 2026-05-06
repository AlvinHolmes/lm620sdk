/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_lcd.c
 *
 * @brief       LCD驱动实现.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team         创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "drv_lcd.h"
#include "lcd_priv.h"
#include "chip_reg_base.h"
#include "psm_core.h"
#include "drv_pin.h"
#include <driver/src/soc/drv_clk.h>


#if LCD_USE_APB_MONITOR
#include <drv_apb_monitor.h>
static uint32_t g_lcdMonitorCounter = 100;
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define LCD_FLAG_DONE  (1)
#define LLI_COUNT (3)
#define DMA_MAX_TRANS_NUM  (60000)

#define LCD_FIFO_THRES          (LCD_FIFO_THRES_8)

#if (LCD_FIFO_THRES == LCD_FIFO_THRES_4)
#define LCD_DMA_BURST_LEN       (DMA_BLEN_4)
#elif (LCD_FIFO_THRES == LCD_FIFO_THRES_8)
#define LCD_DMA_BURST_LEN       (DMA_BLEN_8)
#elif (LCD_FIFO_THRES == LCD_FIFO_THRES_12)
#define LCD_DMA_BURST_LEN       (DMA_BLEN_12)
#elif (LCD_FIFO_THRES == LCD_FIFO_THRES_16)
#define LCD_DMA_BURST_LEN       (DMA_BLEN_16)
#else
#error "please define valid fifo thres"
#endif

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
#if defined(OS_USING_PM)
static inline void LCD_WakeLockInit(LCD_Handle *handle)
{
    PSM_WakelockInit(&handle->wakeLock, PSM_DEEP_SLEEP);
}

static inline void LCD_WakeLock(LCD_Handle *handle)
{
    PSM_WakeLock(&handle->wakeLock);
}

static inline void LCD_WakeUnLock(LCD_Handle *handle)
{
    PSM_WakeUnlock(&handle->wakeLock);
}
#endif

static void LCD_IrqHandler(int vector, void *param)
{
    LCD_Handle *handle = param;
    REG_Lcd *pRegs = handle->pRes->regBase;
    uint32_t    status = READ_REG(pRegs->transStatus);

    if (status & TRANS_INT)
    {
        SET_BIT(pRegs->transStatus, TRANS_INT);
        CLEAR_BIT(pRegs->operation, FIFO_INT_EN);
        CLEAR_BIT(pRegs->operation, DMA_EN);
        CLEAR_BIT(pRegs->operation, TRANS_INT_EN);
        handle->inProcess = false;

    #if defined(OS_USING_PM)
        LCD_WakeUnLock(handle);
    #endif

        if(handle->inTransData)
        {
            if(handle->SendCallback)
            {
                handle->SendCallback(handle->userData);
            }
        }
        else
        {
            osThreadFlagsSet(handle->transThread, LCD_FLAG_DONE);
            handle->transThread = NULL;
        }
    } 
    if(status&FIFO_INT)
    {
        CLEAR_BIT(pRegs->operation, FIFO_INT_EN);
    }

}


static void LCD_LliDmaStart(DMA_ChHandle *chHandle, DMA_LliDesc *pChCfg,uint32_t src_addr, uint32_t len)
{
    int32_t lli_count = 0;
    osDCacheCleanAndInvalidRange((void*)(src_addr), len);
    uint32_t trans_num;
    uint32_t remain_num;
    uint32_t loop;
    trans_num = len / DMA_MAX_TRANS_NUM;
    remain_num = len % DMA_MAX_TRANS_NUM;
    if (trans_num == 0)
    {
      pChCfg[ trans_num ].SrcAddr = src_addr;
      pChCfg[ trans_num ].Count   = len;
      pChCfg[ trans_num ].LLI     = 0;
      pChCfg[trans_num].Control.IrqMod =  DMA_IMOD_ALL_ENABLE;
      lli_count++;
    }
    else
    {
      for (loop = 0; loop < trans_num; loop ++)
      {
          pChCfg[ loop ].SrcAddr = src_addr + loop*DMA_MAX_TRANS_NUM;
          pChCfg[ loop ].Count   = DMA_MAX_TRANS_NUM;
          pChCfg[ loop ].LLI     = (uint32_t)(&pChCfg[ loop + 1 ]);
          lli_count++;
      }
      if(remain_num != 0)
      {
          pChCfg[ loop ].SrcAddr = src_addr + loop*DMA_MAX_TRANS_NUM;
          pChCfg[ loop ].Count   = remain_num;
          pChCfg[ loop ].LLI     = 0;
          pChCfg[loop].Control.IrqMod =  DMA_IMOD_ALL_ENABLE;
          lli_count++;
      }
      else
      {
          pChCfg[ loop - 1 ].LLI   = 0;
          pChCfg[ loop - 1 ].Control.IrqMod =  DMA_IMOD_ALL_ENABLE;
      }
    }
    DMA_LliStart(chHandle, pChCfg, lli_count);
}

static int32_t LCD_Send(LCD_Handle *handle, uint8_t cmd, uint8_t *buf, uint32_t len, bool data)
{
    REG_Lcd *pRegs = handle->pRes->regBase;

    OS_ASSERT(handle->inProcess == false);

    handle->inProcess = true;
    handle->inTransData = data;

    //lcd ctrl
    WRITE_REG(pRegs->transLen, len);
    WRITE_REG(pRegs->operation, WR_MEM | DMA_EN | TRANS_INT_EN | 
                               (LCD_FIFO_THRES << FIFO_THRES_Pos));  // write memory, dma enable

    CLEAR_BIT(pRegs->operation, FIFO_INT_EN);
    WRITE_REG(pRegs->dcsCmd, cmd);

    handle->transLen = len;

#if defined(OS_USING_PM)
    LCD_WakeLock(handle);
#endif

    //start dma
    LCD_LliDmaStart(handle->transDma, handle->transDmaCfg,(uint32_t)buf, len);
    // start
    WRITE_REG(pRegs->start, START);

    return 0;  //no wait, no use dma interrupt, use TRANS_INT interrupt.
}

/**
 *************************************************************************************
 * @brief           读LCD参数
 *
 * @param[in]       handle                     LCD控制器句柄
 * @param[in]       cmd                        读命令
 * @param[in]       buf                        读取参数的存放地址
 * @param[in]       len                        读取参数长度
 *
 * @return          读取结果.
 * @retval          ==0                        读取成功
 * @note            阻塞方式，直到读取成功后函数返回；
 *************************************************************************************
 */
int32_t LCD_ReadParam(LCD_Handle *handle, uint8_t cmd, uint8_t *buf, uint8_t len)
{
    REG_Lcd *pRegs = handle->pRes->regBase;
    int index;

    OS_ASSERT(handle->inProcess == false);

    if(len >= 8)
    {
        return DRV_ERR_PARAMETER;
    }

    handle->inProcess = true;
    handle->inTransData = false;
    
    WRITE_REG(pRegs->transLen, len);  
    WRITE_REG(pRegs->operation, LCD_RW | TRANS_INT_EN);  // read register, no fifo, generate trans int en interrupt.
    WRITE_REG(pRegs->dcsCmd, cmd);

    handle->transThread = osThreadSelf();
    
    CLK_SetPdcoreLspCrmRegs(CLK_SPI_LCD_WCLK_SEL, SPILCD_WCLK_26M);

#if defined(OS_USING_PM)
    LCD_WakeLock(handle);
#endif

    WRITE_REG(pRegs->start, START);

    //wait complete
    osThreadFlagsWait(LCD_FLAG_DONE, osFlagsWaitAny, osWaitForever);

    CLK_SetPdcoreLspCrmRegs(CLK_SPI_LCD_WCLK_SEL, SPILCD_WCLK_104M);
    
    for(index = 0; index < len; index++)
    {
        buf[index] = READ_REG(pRegs->rdValue[index]);
    }
    
    return 0;
}

/**
 *************************************************************************************
 * @brief           写LCD参数
 *
 * @param[in]       handle                     LCD控制器句柄
 * @param[in]       cmd                        写命令
 * @param[in]       buf                        写参数的存放地址
 * @param[in]       len                        写参数长度
 *
 * @return          写入结果.
 * @retval          ==0                        写入成功
 * @note            阻塞方式，直到写入成功后函数返回；
 *************************************************************************************
 */
int32_t LCD_WriteParam(LCD_Handle *handle, uint8_t cmd, uint8_t *buf, uint8_t len)
{
    REG_Lcd *pRegs = handle->pRes->regBase;
    int index;

    OS_ASSERT(handle->inProcess == false);
    OS_ASSERT(handle->transThread == NULL);


    //发送伽马数据
    if(len > 8)
    {
        handle->transThread = osThreadSelf();
        LCD_Send(handle, cmd, buf, len, false);
        
        //wait complete
        osThreadFlagsWait(LCD_FLAG_DONE, osFlagsWaitAny, osWaitForever);
    }
    else
    {
        handle->inProcess = true;
        handle->inTransData = false;
        for(index = 0; index < len; index++)
        {
            WRITE_REG(pRegs->wrParam[index], buf[index]);
        }

        WRITE_REG(pRegs->transLen, len);
        WRITE_REG(pRegs->operation, TRANS_INT_EN);  // write register, no fifo, generate trans int en interrupt.
        WRITE_REG(pRegs->dcsCmd, cmd);

        handle->transThread = osThreadSelf();

    #if defined(OS_USING_PM)
        LCD_WakeLock(handle);
    #endif
        
        WRITE_REG(pRegs->start, START);
        
        //wait complete
        osThreadFlagsWait(LCD_FLAG_DONE, osFlagsWaitAny, osWaitForever);
    
    }

    return 0;
}

/**
 *************************************************************************************
 * @brief           发送LCD屏显数据，非阻塞方式
 *
 * @param[in]       handle                     LCD控制器句柄
 * @param[in]       cmd                        发数据命令
 * @param[in]       buf                        数据的存放地址
 * @param[in]       len                        数据长度
 *
 * @return          写入结果.
 * @retval          ==0                        写入成功
 * @note            非阻塞方式，未等发送，函数立即返回
 *                  发送完成后，通过handle->SendCallback通知用户
 *************************************************************************************
 */
int32_t LCD_SendData(LCD_Handle *handle, uint8_t cmd, uint8_t *buf, uint32_t len)
{
#if LCD_USE_APB_MONITOR
    g_lcdMonitorCounter = 0;

    APBMON_Cfg cfg = {0};
    APBMON_Result cnt = {0};
    REG_Lcd *pRegs = handle->pRes->regBase;

    cfg.start_addr = (uint32_t)&pRegs->txFifo;
    cfg.end_addr = (uint32_t)&pRegs->txFifo;
    cfg.timer_val = 0xFFFFF;
    cfg.int_addr = 0;
    cfg.int_offset = 0;
    cfg.int_cnt = 10;
    cfg.int_type = 0;
    cfg.int_enble = 0;
    cfg.domain = 1;
    cfg.cb = NULL;
    APBMON_Start(&cfg);
#endif

    LCD_Send(handle, cmd, buf, len, true);
    return 0;
}
/**
 *************************************************************************************
 * @brief           查询发送数据长度
 *
 * @param[in]       handle                     LCD控制器句柄
 *
 * @return          DMA搬运已发送数据长度.
 * @retval          >=0    已发送数据长度
 *************************************************************************************
 */
int32_t LCD_GetSendCount(LCD_Handle *handle)
{
    return DMA_GetCount(handle->transDma);
}

/**
 *************************************************************************************
 * @brief           配置LCD控制器
 *
 * @param[in]       handle                     LCD控制器句柄
 * @param[in]       ctrl                       配置控制字
 * @param[in]       val                        对应的配置值
 *
 * @return          配置结果.
 * @retval          ==0                        配置成功
 *************************************************************************************
 */
void LCD_Control(LCD_Handle *handle, LCD_ControlW ctrl, uint32_t val)
{
    REG_Lcd *pRegs = handle->pRes->regBase;

    if(ctrl == LCD_SPI_BIDIR)
    {
        MODIFY_REG(pRegs->protocol, SPI_BIDIR_Msk, val << SPI_BIDIR_Pos);
    }
    else if(ctrl == LCD_SPI_DCX)
    {
        MODIFY_REG(pRegs->protocol, DBI_DCX_Msk, val << DBI_DCX_Pos);
    }
    else if(ctrl == LCD_SAMP_SEL)
    {
        MODIFY_REG(pRegs->protocol, SAMP_SEL_Msk, val << SAMP_SEL_Pos);
    }
    else if(ctrl == LCD_READ_DMY_CYC)
    {
        MODIFY_REG(pRegs->protocol, DMY_CYC_Msk, val << DMY_CYC_Pos);
    }
    else if(ctrl == LCD_RGB565_SWAP)
    {
        WRITE_REG(pRegs->debug, val);
    }
}

/**
 *************************************************************************************
 * @brief           配置LCD模拟器
 *
 * @param[in]       handle                     LCD控制器句柄
 * @param[in]       format                     RGB格式
 * @param[in]       width                      屏宽
 * @param[in]       height                     屏高
 *
 * @return          无
 *************************************************************************************
 */
void LCD_SetEmu(LCD_Handle *handle, LCD_EmuRgbFormat format, uint16_t width, uint16_t height)
{
    REG_Lcd *pRegs = handle->pRes->regBase;

    WRITE_REG(pRegs->emuFormat, format);
    WRITE_REG(pRegs->emuSize, ((uint32_t)width << WIDTH_Pos) | ((uint32_t)height << HEIGHT_Pos));

    osDelay(200);
}

/**
 ************************************************************************************
 * @brief           初始化LCD控制器
 *
 * @param[in]       handle                     LCD控制器句柄
 *
 * @return          初始化结果.
 * @retval          ==0                        初始化成功
 ************************************************************************************
 */
#if LCD_USE_APB_MONITOR
static void LCD_DmaCallback(void *param)
{
    if(++g_lcdMonitorCounter == 3) {
        APBMON_Cfg cfg = {0};
        APBMON_Result cnt = {0};

        cnt.domain = 1;
        APBMON_CntGet(&cnt);

        cfg.domain = 1;
        APBMON_Stop(&cfg);

        osPrintf("wr_num=%d, rd_num=%d, total=%d\r\n", cnt.wr_num, cnt.rd_num, cnt.total_num);
    }
}
#endif

int32_t LCD_Initialize(LCD_Handle *handle)
{
    REG_Lcd *pRegs = handle->pRes->regBase;

    OS_ASSERT(handle->inProcess == false);

    handle->transDma = DMA_Request(handle->pRes->dmaReq);
    OS_ASSERT(handle->transDma);

#if LCD_USE_APB_MONITOR
    handle->transDma->callback = LCD_DmaCallback;
    handle->transDma->para = NULL;
#endif

#if defined(OS_USING_PM)
    LCD_WakeLockInit(handle);
#endif

    //由于DMA传输长度有限制，采用链表形式
    handle->transDmaCfg = (DMA_LliDesc *)osMallocAlign(LLI_COUNT*sizeof(DMA_LliDesc),OS_CACHE_LINE_SZ);
    OS_ASSERT(handle->transDmaCfg);

    for(int i = 0; i < LLI_COUNT;i++)
    {
        handle->transDmaCfg[ i ].DestAddr = (uint32_t)&pRegs->txFifo;
        handle->transDmaCfg[ i ].Control.BurstReqMod = DMA_RMOD_DEV;
        handle->transDmaCfg[ i ].Control.SrcMod = DMA_AMOD_RAM;
        handle->transDmaCfg[ i ].Control.DestMod = DMA_AMOD_FIFO;
        handle->transDmaCfg[ i ].Control.IrqMod = DMA_IMOD_ALL_DISABLE;
        handle->transDmaCfg[ i ].Control.IntSel = DMA_INT1;
        handle->transDmaCfg[ i ].Control.SrcBurstSize = DMA_BSIZE_32BIT;
        handle->transDmaCfg[ i ].Control.DestBurstSize = DMA_BSIZE_32BIT;
        handle->transDmaCfg[ i ].Control.SrcBurstLen = LCD_DMA_BURST_LEN;
        handle->transDmaCfg[ i ].Control.Enable      = 1;
    }

    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(handle->pRes->intNum), IRQ_PRI_NORMAL, IRQ_HIGH_LEVEL);
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(handle->pRes->intNum), LCD_IrqHandler, handle);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(handle->pRes->intNum));

    handle->inProcess = false;
    
    return 0;
}

/**
 ************************************************************************************
 * @brief           去初始化LCD控制器
 *
 * @param[in]       handle                     LCD控制器句柄
 *
 * @return          无.
 ************************************************************************************
 */
void LCD_Uninitialize(LCD_Handle *handle)
{
    OS_ASSERT(handle->inProcess == false);
    
    osInterruptMask(OS_EXT_IRQ_TO_IRQ(handle->pRes->intNum));
    osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(handle->pRes->intNum));

    osFree(handle->transDmaCfg);
    DMA_Release(handle->transDma);
    
    return ;
}

/**
 ************************************************************************************
 * @brief           LCD控制器上下电，时钟开关等
 *
 * @param[in]       handle                     LCD控制器句柄
 * @param[in]       state                      功耗模式
 *
 * @return          配置结果.
 * @retval          ==0                        配置成功
 *************************************************************************************
 */
int32_t LCD_PowerControl(LCD_Handle *handle, DRV_POWER_STATE state)
{
    if(state == DRV_POWER_OFF)
    {
        //关闭PCLK和WCLK
        CLK_SetPdcoreLspCrmRegs(CLK_SPI_LCD_SW_PCLK_EN, 1);
        CLK_SetPdcoreLspCrmRegs(CLK_SPI_LCD_SW_WCLK_EN, 1);
    }
    else if(state == DRV_POWER_FULL)
    {
        //打开PCLK和WCLK
        CLK_SetPdcoreLspCrmRegs(CLK_SPI_LCD_SW_PCLK_EN, 1);
        CLK_SetPdcoreLspCrmRegs(CLK_SPI_LCD_SW_WCLK_EN, 1);
        CLK_SetPdcoreLspCrmRegs(CLK_SPI_LCD_WCLK_SEL, SPILCD_WCLK_104M);
    }
    return 0;
}

/**
 ************************************************************************************
 * @brief           LCD睡眠唤醒恢复接口
 *
 * @param[in]       pRegs                     某个LCD控制器入参
 * @param[in]       mode                      睡眠模式
 * @param[in]       saveAddr                  睡眠数据保存地址
 *
 * @return          保存数据长度.
 * @retval          >=                        数据长度
 *************************************************************************************
 */

#ifdef OS_USING_PM

static int LCD_Suspend(void *param, PSM_Mode mode, uint32_t *saveAddr)
{
    uint32_t *pSaveAddr = saveAddr;
    REG_Lcd *pRegs = param;
    
    if(mode == PSM_DEEP_SLEEP)
    {
        *pSaveAddr++ = READ_REG(pRegs->protocol);
    }

    return (uint32_t)pSaveAddr - (uint32_t)saveAddr;
}

static int LCD_Resume(void *param, PSM_Mode mode,uint32_t *saveAddr)
{
    uint32_t *pSaveAddr = saveAddr;
    REG_Lcd *pRegs = param;

    if (mode == PSM_DEEP_SLEEP)
    {
      WRITE_REG(pRegs->protocol, *pSaveAddr++);
    }
    
    return (uint32_t)pSaveAddr - (uint32_t)saveAddr;
}

static PSM_DpmOps g_LcdDpmOps = {
    .PsmSuspendNoirq = LCD_Suspend,
    .PsmResumeNoirq = LCD_Resume,
};

PSM_DPM_INFO_DEFINE(lcd, g_LcdDpmOps, (void*)BASE_SPI_LCD, PSM_LEVEL_LOW);
#endif
