
#ifndef __WDT_CORE_H_
#define __WDT_CORE_H_

#include "drv_wdt.h"

#define         WDT_WRT_KEY         (0x1234)

#ifdef WDT_FPGA
#define         WDT_CLK_DIV         (26000)
#else
#define         WDT_CLK_DIV         (32768)
#endif


#define         WDT_WRT_KEY_POS             (16)
#define         WDT_WRT_KEY_MASK            (0xffff << WDT_WRT_KEY_POS)
#define         WDT_START_MASK              (1)
#define         WDT_SET_EN_MASK             (0x3F)
#define         WDT_SET_EN_CFG_POS          (4)
#define         WDT_SET_EN_LOAD_POS         (2)    
#define         WDT_SET_EN_INT_POS          (0)    
#define         WDT_STA_INT_ACK             (4)
#define         WDT_STA_CNT_ACK             (6)
#define         WDT_STA_CFG_ACK             (8)


typedef struct 
{
    __IO uint32_t verReg;
    __IO uint32_t cfgReg;
    __IO uint32_t loadValReg;
    __IO uint32_t currentValReg;
    __IO uint32_t staReg;
    __IO uint32_t compareValReg;
    __IO uint32_t setEnReg;
    __IO uint32_t startReg;
}WDT_Reg;

typedef struct 
{
    WDT_Handle userData;
    WDT_Reg *reg;
}WDT_CoreHandle;

/**
 * @brief 看门狗初始化
 * @param handle 操作句柄
 * @return uint8_t
 */
uint8_t WDT_Initlize(WDT_CoreHandle *handle);

/**
 * @brief 看门狗去初始化
 * @param handle 操作句柄
 * @return uint8_t
 */
uint8_t WDT_UnInitlize(WDT_CoreHandle *handle);

/**
 * @brief 看门狗启动
 * @param handle 操作句柄
 * @return uint8_t
 */
uint8_t WDT_Start(WDT_CoreHandle *handle);

/**
 * @brief 看门狗配置
 * @param handle 操作句柄
 * @return uint8_t
 */
uint8_t WDT_Config(WDT_CoreHandle *handle);

/**
 * @brief 看门狗停止
 * @param handle 操作句柄
 * @return uint8_t
 */
uint8_t WDT_Stop(WDT_CoreHandle *handle);

/**
 * @brief 看门狗当前计数值
 * @param handle 操作句柄
 * @return uint32_t
 */
uint32_t WDT_CountGet(WDT_CoreHandle *handle);
uint32_t WDT_LoadGet(WDT_CoreHandle *handle);
uint32_t WDT_TimeoutGet(WDT_CoreHandle *handle);

#endif
