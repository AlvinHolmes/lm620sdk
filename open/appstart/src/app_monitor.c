/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-01     ict team          创建
  ************************************************************************************
  */

 /************************************************************************************
  *                                 头文件
  ************************************************************************************/
#include "os.h"
#include "drv_adc.h"
#include "slog_print.h"

/************************************************************************************
 *                                 外部函数声明
 ************************************************************************************/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

#define APP_BAT_AVERAGE_NUM  (10)
#define APP_TEMP_AVERAGE_NUM  (10)

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static uint8_t g_batteryLevel;
static uint32_t g_batteryLastAvgAdcVal;
static uint32_t g_batteryAdcValSum;
static uint32_t g_batteryVoltage;
static uint32_t g_batteryLastLevelVoltage;

const static int g_batteryLevelPercentTable[11] =     {3377, 3485, 3562, 3618, 3681, 3762, 3845, 3904, 3979, 4015, 4140};
                                                      /* 135   130   125   120   115  110   100     90    80    50    15 */
const static int g_batteryLevelPercentTableInCharge[11] = {3512, 3615, 3687, 3738, 3796, 3872, 3945, 3994, 4059, 4065, 4155};

static int32_t g_tempPaVal;
static int32_t g_tempXoVal;
static int32_t g_tempRfVal;

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void prvBatteryInit(void);
static void prvTempSensorInit(void);

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

static int toPercentage(int voltage, const int *table)
{
	int i = 0;

	if(voltage < table[0])
		return 0;

	for(i = 0; i < 11; i++)
    {
        if(voltage < table[i])
             return i*10 - ((10UL * (int)(table[i] - voltage)) / (int)(table[i] - table[i-1]));
	}

	return 100;
}

static int toVoltage(int per, const int *table)
{
	int index;

	if(per < 1)
		return table[0];
	if(per > 100)
		return table[10];

    index = (per - 1) / 10;

	return table[index] + (((table[index + 1] - table[index]) * (per - (index * 10))) / 10);
}

bool_t prvGetChargeStatus(void)
{
    return false; // need todo
}

static void prvBatteryTimer(void)
{
    int32_t val;
    uint32_t newVal;
    int level;
    static uint8_t preInCharge;
    static osTick_t lastTick;

    if(g_batteryAdcValSum == 0)
    {
        prvBatteryInit();
        return;
    }
    
    val = ADC_ReadVoltData(ADC_CH_VBAT);
    if(val < 0)
    {
        slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_APP, "Battery read voltage error\r\n");
        return ;
    }
    
    g_batteryVoltage = val;
    
    if(prvGetChargeStatus())
    {
        if(preInCharge == 0)
        {
            preInCharge = 1;
            g_batteryLastAvgAdcVal = toVoltage(g_batteryLevel, g_batteryLevelPercentTableInCharge);
            g_batteryAdcValSum = g_batteryLastAvgAdcVal * APP_BAT_AVERAGE_NUM;
        }

        g_batteryAdcValSum = g_batteryAdcValSum - g_batteryLastAvgAdcVal + val;
        newVal = g_batteryAdcValSum / APP_BAT_AVERAGE_NUM;
        g_batteryLastAvgAdcVal = newVal;
    
        level = toPercentage(newVal, g_batteryLevelPercentTableInCharge);
        if(g_batteryLevel > 90)   //最后10%，按照1分钟，充电1%
        {
            if(osMsFromTick(osTickGet() - lastTick) >= (1000 * 60))
            {
                if(g_batteryLevel == 99)
                {
                    if((g_batteryLastAvgAdcVal >= g_batteryLastLevelVoltage) && ((g_batteryLastAvgAdcVal - g_batteryLastLevelVoltage) < 5))
                    {
                        g_batteryLevel = 100;
                    }
                    else
                    {
                        g_batteryLastLevelVoltage = g_batteryLastAvgAdcVal;
                        lastTick = osTickGet();
                    }
                }
                else
                {
                    if(level > g_batteryLevel)
                    {
                        g_batteryLevel++;
                        g_batteryLastLevelVoltage = g_batteryLastAvgAdcVal;
                        lastTick = osTickGet();
                    }
                }
            }
        }
        else if(g_batteryLevel > 50)   //恒压充电40分钟，充电50%
        {
            if(osMsFromTick(osTickGet() - lastTick) >= (1000 * 48))
            {
                if(level > g_batteryLevel)
                {
                    g_batteryLevel++;
                    g_batteryLastLevelVoltage = g_batteryLastAvgAdcVal;
                    lastTick = osTickGet();
                }
            }
        }
        else      //恒流充电25分钟，充电50%
        {
            if(osMsFromTick(osTickGet() - lastTick) >= (1000 * 30))
            {
                if(level > g_batteryLevel)
                {
                    g_batteryLevel++;
                    g_batteryLastLevelVoltage = g_batteryLastAvgAdcVal;
                    lastTick = osTickGet();
                }
            }
        }

    }
    else
    {
        if(preInCharge == 1)
        {
            preInCharge = 0;
            g_batteryLastAvgAdcVal = toVoltage(g_batteryLevel, g_batteryLevelPercentTable);
            g_batteryAdcValSum = g_batteryLastAvgAdcVal * APP_BAT_AVERAGE_NUM;
        }

        g_batteryAdcValSum = g_batteryAdcValSum - g_batteryLastAvgAdcVal + val;
        newVal = g_batteryAdcValSum / APP_BAT_AVERAGE_NUM;
        g_batteryLastAvgAdcVal = newVal;

        level = toPercentage(newVal, g_batteryLevelPercentTable);
        
        if(osMsFromTick(osTickGet() - lastTick) >= (1000 * 30))  // 1分钟，最多耗电2%
        {
            if(g_batteryLevel > level)
            {
                g_batteryLevel--;
                g_batteryLastLevelVoltage = g_batteryLastAvgAdcVal;
                lastTick = osTickGet();
            }
        }
    }

    slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_APP, "Battery voltage %d, avg %d, level %d, avg %d\r\n", g_batteryVoltage, g_batteryLastAvgAdcVal, level, g_batteryLevel);
}

static void prvBatteryInit(void)
{
    int32_t val;

    val = ADC_ReadVoltData(ADC_CH_VBAT);

    if(val < 0)
    {
        slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_APP, "Battery init voltage error\r\n");
        return ;
    }
    
    g_batteryLastAvgAdcVal = val;
    g_batteryAdcValSum = g_batteryLastAvgAdcVal * APP_BAT_AVERAGE_NUM;
    
    if(prvGetChargeStatus())
    {
        g_batteryLevel = toPercentage(g_batteryLastAvgAdcVal,g_batteryLevelPercentTableInCharge);
    }
    else
    {
        g_batteryLevel = toPercentage(g_batteryLastAvgAdcVal, g_batteryLevelPercentTable);
    }    
}

static void prvTempSensorTimer(void)
{
    int32_t val;

    if(ADC_ReadTemp(ADC_CH_PA_TEMP, &val) == 0)
    {
        g_tempPaVal = val;
    }

    if(ADC_ReadTemp(ADC_CH_XO_TEMP, &val) == 0)
    {
        g_tempXoVal = val;
    }

    if(ADC_ReadTemp(ADC_CH_RF_TEMP, &val) == 0)
    {
        g_tempRfVal = val;
    }

    slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_APP, "Temp PA %d, XO %d, RF %d\r\n", g_tempPaVal, g_tempXoVal, g_tempRfVal);
}

void SysMonitorThread(void *argument)
{
    while(!ADC_IsReady())
    {
        osThreadSleepRelaxed(osTickFromMs(1000), osTickFromMs(3000));  //等待adc
        slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_APP, "Wait adc ready ...");
    }
    
    prvBatteryInit();
    
    while(1)
    {
        osThreadSleepRelaxed(osTickFromMs(5000), osTickFromMs(3000));  // 睡眠5S为周期，最大延时3s
        prvBatteryTimer();
        prvTempSensorTimer();

        if(g_batteryLevel <= 5)
        {
            slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_APP, "System is in low bettery, need poweroff");
            osShutdown(OS_SHUTDOWN, OS_SHUTDOWN_NORMAL);
        }
    }
}

/**
 ************************************************************************************
 * @brief           系统监控应用入口
 *
 * @param[in]       none
 *
 * @return
 ************************************************************************************
*/
#ifdef CONFIG_USE_SYSMON
int SysMonitorInit(void)
{
    osPrintf("SysMonitor Init\r\n");
    
    osThreadAttr_t attr = {"SysMonitor", osThreadDetached, NULL, 0U, NULL, 1024, osPriorityLow, 0U, 0U};

	osThreadNew(SysMonitorThread, NULL, &attr);

    return 0;
}
INIT_APP_EXPORT(SysMonitorInit, OS_INIT_SUBLEVEL_HIGH);
#endif

