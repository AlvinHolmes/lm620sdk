/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file main.c
*
* @brief  main函数入口文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/
#include <os.h>
#include <drv_reset.h>

int printPowerOnReason(void)
{
    T_Drv_PowerOn_Type pwrType = Drv_GetPowerOnState();

    if (pwrType == POWER_ON_STANDBY) {
        T_SbyOn_Reason reason = Sys_GetSbyOnReason();
        switch (reason)
        {
        case SBYON_RTC:
            osPrintf("sby POWER ON by rtc\r\n");
            break;
        case SBYON_POWER_KEY:
            osPrintf("sby POWER ON by key on\r\n");
            break;
        case SBYON_WAKEUP_KEY:
            osPrintf("sby POWER ON by wakeup key\r\n");
            break;
        case SBYON_RESET_KEY:
            osPrintf("sby POWER ON by reset key\r\n");
            break;
        default:
            osPrintf("sby POWER ON error: %d\r\n", reason);
            break;
        }
    } else if (pwrType == POWER_ON_NORMAL) {
        T_Sys_Reset_Reason reason = Sys_GetResetReason();
        switch (reason)
        {
        case SYS_UNKNOW_RSTN_FLAG:
        case SYS_POR5_RSTN_FLAG:
            osPrintf("POWER ON by key on/wakeup key\r\n");
            break;
        case SYS_EXT_RSTN_FLAG:
            osPrintf("POWER ON by reset key\r\n");
            break;
        case SYS_GLOBAL_SW_RSTN_FLAG:
            osPrintf("POWER ON by soft reset\r\n");
            break;
        case SYS_APSS_WDT_RSTN_FLAG:
        case SYS_CPSS_WDT_RSTN_FLAG:
            osPrintf("POWER ON by watch dog reset\r\n");
            break;
        default:
            osPrintf("POWER ON error: %d\r\n", reason);
            break;
        }

    }
    return 0;
}


int main(void)
{
	return 0;
}

