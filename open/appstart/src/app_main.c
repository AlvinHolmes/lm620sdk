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
#include "drv_reset.h"
#include "platcfg.h"
#include "app_pub.h"
#include "app_urc_ring_indicate.h"

/************************************************************************************
 *                                 外部函数声明
 ************************************************************************************/
int APP_Uart_AT(void);
int APP_URC_Monitor(void);
void MuxProxy_SlaveInit(void);
void APP_AT_Function_Init(void);
void APP_4G_Start(void);
void APP_4G_Init(void);

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define BOOTYPE_HISTORY_COUNT       (6)
#define BOOTYPE_ITEM_LEN            (6)
#define BOOTYPE_ITEM_FORMAT         "%02d_%02d;"
#define BOOTYPE_FILE_PATH           "/usr/boot.his"

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void APP_RecordBootType(void);
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief           应用入口
 *
 * @param[in]       none
 *
 * @return
 ************************************************************************************
*/
int APP_Init(void)
{
    osPrintf("APP_Init Start\r\n");
    APP_4G_Start();
    osPrintf("APP_Init End\r\n");

#if BOOTYPE_HISTORY_COUNT
    APP_RecordBootType();
#endif
    return 0;
}
INIT_APP_EXPORT(APP_Init, OS_INIT_SUBLEVEL_LOW);

/**
 ************************************************************************************
 * @brief           AT应用入口
 *
 * @param[in]       none
 *
 * @return
 * @note 注册AT设备，注册AT主动上报监控
 ************************************************************************************
*/
int APP_AT_Init(void)
{
    osPrintf("APP_AT_Init Start\r\n");
    APP_URC_Monitor(); //  处理主动上报的AT命令，获取MODEM状态
    APP_AT_Function_Init(); //  AT命令功能模块
//#ifdef USE_TOP_FPGA
#ifdef USE_TOP_SPIMUX
//#ifndef CONFIG_USE_FLASH2
    extern bool_t g_Flash2_Enable;
    if(PlatCfg_GetMuxFlag() && (g_Flash2_Enable == OS_FALSE)) {
        MuxProxy_SlaveInit(); //  MUX 和 MUX AT通道
    }
//#endif
#endif
    APP_URC_RI_Initiate();
    APP_Uart_AT(); //  UART AT通道
    APP_4G_Init();
    osPrintf("APP_AT_Init End\r\n");

    return 0;
}
INIT_COMPONENT_EXPORT(APP_AT_Init, OS_INIT_SUBLEVEL_LOW);


#if BOOTYPE_HISTORY_COUNT
#include "nr_micro_shell.h"
#include "vfs.h"

static void APP_RecordBootType(void)
{
    VFS_File    *fp;
    char        *p;
    char        buf[BOOTYPE_HISTORY_COUNT * BOOTYPE_ITEM_LEN + 1] = {0};

    fp = VFS_OpenFile(BOOTYPE_FILE_PATH, "rb+");
    if (fp != OS_NULL) {
        VFS_ReadFile(buf, 1, sizeof(buf)-1, fp);
        if (strlen(buf) > (BOOTYPE_HISTORY_COUNT-1) * BOOTYPE_ITEM_LEN) {
            osMemmove(buf, buf + BOOTYPE_ITEM_LEN, (BOOTYPE_HISTORY_COUNT-1) * BOOTYPE_ITEM_LEN);
            p = buf + (BOOTYPE_HISTORY_COUNT-1) * BOOTYPE_ITEM_LEN;
        } else {
            p = buf + strlen(buf);
        }
    } else {
        fp = VFS_OpenFile(BOOTYPE_FILE_PATH, "wb");
        p = buf;
    }

    T_Drv_PowerOn_Type  pwrType = Drv_GetPowerOnState();
    uint8_t             reason = 0;
    if (pwrType == POWER_ON_STANDBY) {
        reason = Sys_GetSbyOnReason();
    } else if (pwrType == POWER_ON_NORMAL || pwrType == POWER_ON_WDT) {
        reason = Sys_GetResetReason();
    }
    osSnprintf(p, BOOTYPE_ITEM_LEN+1, BOOTYPE_ITEM_FORMAT, pwrType, reason);

    if (fp) {
        VFS_SeekFile(fp, 0, VFS_SEEK_SET);
        VFS_WriteFile(buf, 1, strlen(buf), fp);
        VFS_CloseFile(fp);
    }

    osPrintf("History boot: %s\r\n", buf);
}

static void SHELL_boothis(char argc, char **argv)
{
    VFS_File    *fp;
    char        buf[BOOTYPE_HISTORY_COUNT * BOOTYPE_ITEM_LEN + 1] = {0};

    fp = VFS_OpenFile(BOOTYPE_FILE_PATH, "rb");
    if (fp != OS_NULL) {
        VFS_ReadFile(buf, 1, sizeof(buf)-1, fp);
        VFS_CloseFile(fp);
        osPrintf("History boot: %s\r\n", buf);
    }
}

NR_SHELL_CMD_EXPORT(boothis, SHELL_boothis);

int32_t APP_GetBootHistory(char* buf, uint32_t len)
{
    VFS_File    *fp;
    osMemset(buf, 0, len);
    fp = VFS_OpenFile(BOOTYPE_FILE_PATH, "rb");
    if (fp != OS_NULL) {
        VFS_ReadFile(buf, 1, len-1, fp);
        VFS_CloseFile(fp);
    }
    return 0;
}

#else

int32_t APP_GetBootHistory(char* buf, uint32_t len)
{
    return -1;
}

#endif

