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

#ifndef __SVC_EQUIPMENT_H__
#define __SVC_EQUIPMENT_H__

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define SVC_EQP_IMEI_SIZE (16)
/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
#define SVC_EQP_NCELL_MAX    6 //  支持的最大邻小区个数
/**
 * @brief 小区信息上报，对应API svc_eqp_read_cellinfo
 * @param str_oper，数字plmn,只需要5个字符1
 * @param tac,TrackingAreaCode
 * @param cell_id,小区ID
 * @param pci,小区pci
 * @param real_rsrp,小区rsrp -141dBm ~ -44dBm
 */
typedef struct
{
    char        str_oper[6];    //  数字plmn,只需要5个字符
    uint16_t    tac;            //  TrackingAreaCode
    uint32_t    cell_id;        //  小区ID
    uint16_t    pci;            //  小区pci
    int16_t     real_rsrp;      //  小区rsrp -141dBm ~ -44dBm
} SVC_EQP_CellInfo;

/**
 * @brief WIFISCAN结果，对应API svc_eqp_wifiscan_start的cb回调
 * @param mac_addr, mac 地址
 * @param rssi,信号强度
 */
#define SVC_EQP_WifiSCAN_MAC_LEN     20
#define SVC_EQP_WifiSCAN_SSID_LEN    40
typedef struct
{
    uint8_t channel;
    char mac_addr[SVC_EQP_WifiSCAN_MAC_LEN];  // mac 地址
    int32_t rssi;       // 信号强度
    char ssid[SVC_EQP_WifiSCAN_SSID_LEN];
}SVC_EQP_WifiScanInfo;

/**/
#define SVC_NCELL_NUM               8
typedef struct {
    uint32_t    earfcn;
    uint16_t    pci;
    int16_t     sRsrp;
}SVC_NCellInfo;

typedef struct {
    uint8_t         mcc[3];
    uint8_t         mnc[3];
    uint16_t        tac;

    uint32_t        earfcn;
    uint16_t        pci;
    int16_t         sRsrp;
    int16_t         sRsrq;
    int16_t         sinr;
    int8_t          nCellNum; // 邻区个数，即 nCell 中有效数据个数
    SVC_NCellInfo   nCell[SVC_NCELL_NUM];
}SVC_CellInfo;


/**
 * @brief 回调类型
 * @param[in]  scaninfo Scan结果
 * @param[in]  num  scan结果个数
 * &note 回调函数参考代码如下
void demo_wifiscancb(SVC_EQP_WifiScanInfo *scanInfo, uint32_t num)
{
 osPrintf("APP num:[%u]\r\n", num);
 for(int i = 0; i < num; i++)
 {
     osPrintf("APP channel:%u rssi:%d mac:[%s] ssid:[%s]\r\n",
             scanInfo[i].channel, scanInfo[i].rssi, scanInfo[i].mac_addr, scanInfo[i].ssid);
 }
}
 */
typedef void (*WifiScanCb) (SVC_EQP_WifiScanInfo *scaninfo, uint32_t num);
/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief 获取设备IMEI
 *
 * @param[out] p_imei 至少SVC_EQP_IMEI_SIZE大小空间，用于存放IMEI
 * @param[in] buf_len  描述p_imei空间大小
 *
 * @return  osOK：成功，其他值：失败
 * @note 实际调用 Api_GetImei，建议使用 Api_GetImei @ ap_ps_interface.h
 ************************************************************************************
*/
int svc_eqp_read_imei(char *p_imei, uint32_t buf_len);
/**
 ************************************************************************************
 * @brief 获取设备小区信息，发送AT+CMNCELL命令
 *
 * @param[in] arr_cellinfo 存放小区信息的数组
 * @param[in] arr_count  描述数组arr_cellinfo中 SVC_EQP_CellInfo 个数,最多输出arr_count个小区信息
 *
 * @return 0：AT命令发送成功 其他：失败
 * @note
 * 1.同步接口会阻塞任务，禁止在AT命令处理函数中调用，该接口会阻塞较长时间
 * 2.数组arr_cellinfo成员个数建议不要超过 SVC_EQP_NCELL_MAX
 * 3.arr_cellinfo中str_oper为空表示无效数据
 * 4.第一个是服务小区，其他是邻小区
 * 5.参考代码如下
 SVC_EQP_CellInfo ncell[SVC_EQP_NCELL_MAX] = {0};
 int i = 0;
 svc_eqp_read_cellinfo(ncell, sizeof(ncell)/sizeof(ncell[0]));
 for(i = 0; i < sizeof(ncell)/sizeof(ncell[0]); i++)
 {
     if(strlen(ncell[i].str_oper) != 0)
         osPrintf("oper[%s] tac[%u] cellid[%x] pci[%u] rsrp[%d]\r\n", ncell[i].str_oper, ncell[i].tac, ncell[i].cell_id, ncell[i].pci, ncell[i].real_rsrp);
 }
 ************************************************************************************
*/
int svc_eqp_read_cellinfo(SVC_EQP_CellInfo *arr_cellinfo, uint8_t arr_count);


/**
 ************************************************************************************
 * @brief 获取设备号
 *
 * @param[out] p_boardnum 用于存放Board number
 * @param[in] buf_len  描述p_boardnum空间大小
 *
 * @return  osOK：成功，其他值：失败
 * @note 实际调用 amt_NvItemRead，建议使用 amt_NvItemRead @ ap_ps_interface.h
 ************************************************************************************
*/
int svc_eqp_read_boardnum(char *p_boardnum, uint32_t buf_len);

/**
 ************************************************************************************
 * @brief 获取校准状态
 *
 * @return  true: 已校准，false: 未校准
 ************************************************************************************
*/
bool svc_eqp_read_calibration(void);

/**
 ************************************************************************************
 * @brief wifiscan 主动上报AT解析处理
 * @param[in] cmd AT命令内容
 * @param[in] cmdLen  AT命令内容长度
 *
 * @return 0：成功  其他：失败
 * @内部函数，用于处理WIFISCAN的AT上报
 ************************************************************************************
*/
int svc_eqp_wifiscannum_handler(char *cmd, uint32_t cmdLen);
/**
 ************************************************************************************
 * @brief wifiscan 启动，发送AT+WIFISCAN=<scantime>,<scannum>,<channel>命令
 *
 * @param[in] scantime 扫描超时时间 [2-255]s
 * @param[in] scannum  最大扫描个数 [1-20]
 * @param[in] channel  扫描通道 [1-13,255] 255表示全通道扫描
 * @param[in] cb       扫描成功回调函数
 *
 * @return 0：AT命令发送成功 其他：失败
 * @note   同步接口会阻塞任务，禁止在AT命令处理函数中调用
 *         扫描结果会通过cb回调函数上报
 ************************************************************************************
*/
int svc_eqp_wifiscan_start(uint8_t scantime, uint8_t scannum, uint8_t channel, WifiScanCb cb);

/**
 ************************************************************************************
 * @brief 获取小区信息
 *
 * @param[out] info 用于存放小区信息
 * @note 实际调用 api_GetNetInfo，建议使用 api_GetNetInfo @ ap_ps_interface.h
 * 对应参数意义，参考 api_GetNetInfo
 ************************************************************************************
*/
void svc_eqp_get_cellInfo(SVC_CellInfo *info);

#ifdef __cplusplus
}
#endif
#endif /* __SVC_EQUIPMENT_H__ */
