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

#ifndef __SRV_NETWORK_H__
#define __SRV_NETWORK_H__

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


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
/**
 * @brief cereg主动上报 对应消息 SVC_EVT_NETWORK_CEREG_IND
 * @param stat,网络状态 0 未注册，MT不搜索可注册的运营商网络,1 已注册，本地网络（非漫游）,2 未注册，MT尝试附着或搜索可注册的运营商网络,3 注册被拒,4 未知（例如：无E-UTRAN覆盖）,5 已注册，漫游
 * @param tac,字符串类型；16进制格式的2个字节，跟踪区编码，需要配置 AT+CEREG=2
 * @param ci,字符串类型；16进制格式的4个字节，小区识别码，需要配置 AT+CEREG=2
 * @param act,当前服务小区的接入技术 7 : EUTRAN，需要配置 AT+CEREG=2
 * @param subact,子制式 0 : TDD 1 : FDD，需要配置 AT+CEREG=2
 */
typedef struct
{
    uint8_t stat;   /* 网络状态
                       0 未注册，MT不搜索可注册的运营商网络
                       1 已注册，本地网络（非漫游）
                       2 未注册，MT尝试附着或搜索可注册的运营商网络
                       3 注册被拒
                       4 未知（例如：无E-UTRAN覆盖）
                       5 已注册，漫游
                    */
    char  tac[6];   /* 字符串类型；16进制格式的2个字节，跟踪区编码，需要配置 AT+CEREG=2 */
    char  ci[10];   /* 字符串类型；16进制格式的4个字节，小区识别码，需要配置 AT+CEREG=2*/
    uint8_t act;    /* 当前服务小区的接入技术 7 : EUTRAN，需要配置 AT+CEREG=2*/
    uint8_t subact; /* 子制式 0 : TDD 1 : FDD，需要配置 AT+CEREG=2 */
} SVC_NETWORK_CeregInfo;

/**
 * @brief cgev主动上报 对应消息 SVC_EVT_NETWORK_CGEV_IND
 * @param stat,PDP状态 0: 去激活      1:激活
 * @param cid,PDP上下文的CID值
 */
typedef struct
{
    uint8_t stat; //  0: 去激活      1:激活
    uint8_t cid;  //  PDP上下文的CID值
} SVC_NETWORK_CgevInfo;

/*cmgactstat主动上报，已废弃*/
/**
 * @brief +CMGACTSTAT 主动上报 对应消息 SVC_EVT_NETWORK_CMGACTSTAT_IND
 * @param stat,netif网卡状态 0: 去激活      1:激活
 * @param cid,PDP上下文的CID值
 */
typedef struct
{
    uint8_t stat; //  0: 去激活      1:激活
    uint8_t cid;  //  PDP上下文的CID值
} SVC_NETWORK_CmgactstatInfo;

/**
 * @brief +NETDEVCTL 主动上报，OpenCPU不需要使用 对应消息 SVC_EVT_NETWORK_NETDEVCTL_IND
 * @param stat,rndis状态 0: 去激活      1:激活
 * @param cid,PDP上下文的CID值
 */
typedef struct
{
    uint8_t stat; //  0: 去激活      1:激活
    uint8_t cid;  //  PDP上下文的CID值
} SVC_NETWORK_NetdevctlInfo;

/**
 * @brief +NETSTAT 主动上报 对应消息 SVC_EVT_NETWORK_NETSTAT_IND
 * @param stat,netif网卡状态 0: 去激活      1:激活
 * @param cid,PDP上下文的CID值
 */
typedef struct
{
    uint8_t stat; //  0: 去激活      1:激活
    uint8_t cid;  //  PDP上下文的CID值
} SVC_NETWORK_NetstatInfo;

/**
 * @brief 执行AT+COPS?结果上报 对应消息 SVC_EVT_NETWORK_READ_COPS_RESULT 对应API svc_network_read_cops
 * @param mode,0:自动搜网模式 1:手动搜网模式 2:注销网络 3:仅设置<format> 4:手动/自动搜网模式
 * @param format,0:长格式字母类型<oper>，最多16字符  1:短格式字母类型<oper>，最多8字符 2:数字类型<oper>
 * @param oper,运营商名称,格式由format确定
 * @param act,7 E-UTRAN
 */
typedef struct
{
    uint8_t mode; // 0~4 0:自动搜网模式 1:手动搜网模式 2:注销网络 3:仅设置<format> 4:手动/自动搜网模式
    uint8_t format; // 0~2 0:长格式字母类型<oper>，最多16字符  1:短格式字母类型<oper>，最多8字符 2:数字类型<oper>
    char oper[20];  //运营商名称,格式由format确定
    uint8_t act; // 7 E-UTRAN
    //uint8_t subact; // 子制式   : 0 TDD  1 FDD
} SVC_NETWORK_CopsReadInfo;

/**
 * @brief +CTZE 或 +CTZEU 主动上报 对应消息 SVC_EVT_NETWORK_TIME_IND 需要配置AT+CTZR=2或AT+CTZR=3
 * @param  sec,   秒
 * @param  min,   分
 * @param  hour, 时
 * @param  mday, 天
 * @param  mon,   月
 * @param  year, 年
 * @param  zone, 时区
 */
typedef struct
{
    int sec;  // 秒
    int min;  // 分
    int hour; // 时
    int mday; // 天
    int mon;  // 月
    int year; // 年
    int zone; // 时区
} SVC_NETWORK_DateTime;

/**
 * @brief 网络注册状态
 * @param
 */
typedef enum {
    SVC_NETWORK_STATUS_NOREGNOSEARCH = 0, // 未注册 未搜索网络
    SVC_NETWORK_STATUS_REGHPLMN,          // 已注册 本地网络
    SVC_NETWORK_STATUS_NOREGBUTSEARCH,    // 未注册 尝试搜索网络
    SVC_NETWORK_STATUS_REGDENIED,         // 未注册 注册被拒绝
    SVC_NETWORK_STATUS_UNKNOWN,           // 未注册 状态未知
    SVC_NETWORK_STATUS_REGROAMING         // 已注册 漫游网络
} SVC_NetworkStatus;

/**
 * @brief PDP激活状态
 * @param
 */
typedef enum {
    SVC_NETWORK_PDP_INACTIVE = 0, // 未激活
    SVC_NETWORK_PDP_ACTIVE        // 已激活
} SVC_NetworkPdpStatus;


// 网络信号强度相关信息
typedef struct
{
    int16_t rssi;
    int16_t rsrp;
    int16_t sinr;
    int16_t rsrq;
} SVC_NETWORK_QCSQInfo;

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief    配置CFUN值 发送AT+CFUN=<fun>命令
 *
 * @param[in] cfun的配置值
 *
 * @return 0：AT命令发送成功 其他：失败
 * @note 配置结果通过 SVC_EVT_NETWORK_SET_CFUN_RESULT 消息上报
 ************************************************************************************
*/
int svc_network_set_cfun(uint8_t fun);
/**
 ************************************************************************************
 * @brief    配置CEREG   发送AT+CEREG=<n>命令
 *
 * @param[in] CEREG n值
 *
 * @return 0：AT命令发送成功 其他：失败
 * @note 配置结果通过 SVC_EVT_NETWORK_SET_CEREG_RESULT 消息上报
 ************************************************************************************
*/
int svc_network_set_cereg(uint8_t n);

/**
 ************************************************************************************
 * @brief    读取CSQ值 发送AT+CSQ命令
 *
 * @param[in] none
 *
 * @return 0：AT命令发送成功 其他：失败
 * @note CSQ值通过 SVC_EVT_NETWORK_READ_CSQ_RESULT 消息上报
 ************************************************************************************
*/
int svc_network_read_csq(void);

/**
 ************************************************************************************
 * @brief    配置PDP激活或去激活 发送AT+CGACT=<stat>,<cid>命令
 *
 * @param[in] stat   0 去激活  1 激活
 * @param[in] cid    PDP上下文的CID值
 *
 * @return 0：AT命令发送成功 其他：失败
 * @note 配置结果通过 SVC_EVT_NETWORK_SET_CGACT_RESULT 消息上报
 ************************************************************************************
*/
int svc_network_set_cgact(uint8_t stat, uint8_t cid);

/**
 ************************************************************************************
 * @brief    PDP数据传输激活或去激活 发送AT+CMGACT=<stat>,<cid>命令
 *
 * @param[in] stat   0 去激活  1 激活
 * @param[in] cid     PDP上下文的CID值
 *
 * @return 0：AT命令发送成功 其他：失败
 * @note  2210不使用，已废弃
    配置结果通过 SVC_EVT_NETWORK_SET_CMGACT_RESULT 消息上报
 ************************************************************************************
*/
int svc_network_set_cmgact(uint8_t stat, uint8_t cid);

/**
 ************************************************************************************
 * @brief    eth数据传输激活或去激活 发送AT+NETDEVCTL=<enable>,<cid>命令
 *
 * @param[in] enable   0 去激活  1 激活
 * @param[in] cid     PDP上下文的CID值
 *
 * @return 0：AT命令发送成功 其他：失败
 * @note 配置结果通过 SVC_EVT_NETWORK_SET_NETDEVCTL_RESULT 消息上报
 ************************************************************************************
*/
int svc_network_set_netdevctl(uint8_t enable, uint8_t cid);

/**
 ************************************************************************************
 * @brief    读取COPS的值 发送AT+COPS?命令
 *
 * @param[in] none
 *
 * @return 0：AT命令发送成功 其他：失败
 * @note 结果通过 SVC_EVT_NETWORK_READ_COPS_RESULT 消息上报
 ************************************************************************************
*/
int svc_network_read_cops(void);

/**
 ************************************************************************************
 * @brief	 读取 CEREG 的 stat 值
 *
 * @param[in]
 * @param[out] status
 *
 * @return osOK：成功，其他值：失败
 * @note
 ************************************************************************************
*/
int svc_network_read_status(SVC_NetworkStatus * status);

/**
 ************************************************************************************
 * @brief	 通过 AT+CGCONTRDP=cid 读取 PDP 激活状态
 *
 * @param[in] cid
 * @param[out] status
 *
 * @return osOK：成功，其他值：失败
 * @note
 ************************************************************************************
*/
int svc_network_read_pdp_status(uint8_t cid, SVC_NetworkPdpStatus * status);

/**
 ************************************************************************************
 * @brief    配置CTZR   发送+CTZR=<reporting>命令
 *
 * @param[in] reporting值
 * <reporting>
 * 0 禁用时区更改报告
 * 1 启用时区更改报告，报告格式为+CTZV:<tz>
 * 2 启用时区更改报告，报告格式为+CTZE:<tz>,<dst>,[<time>]
 * 3 启用时区更改报告，报告格式为+CTZEU:<tz>,<dst>,[<utime>]
 *
 * @return 0：AT命令执行成功 其他：失败
 * @note 同步接口，会阻塞直到命令返回
 *       配置影响时区上报格式，只有2和3才可能有时间上报
 ************************************************************************************
*/
int svc_network_set_ctzr_sync(uint8_t reporting);

/**
 ************************************************************************************
 * @brief    配置QCSQ
 *
 * @param[in] QCSQ enble值, 0:关闭+QCSQ主动上报，1:开启+QCSQ主动上报
 * @param[in] delta 门限值, 变化超过门限时，会有+QCSQ上报，0表示不配置该参数
 *
 * @return 0：AT命令执行成功 其他：失败
 * @note 同步接口，会阻塞直到命令返回
 * delta    为0时发送命令 AT+QCSQ=<enable>
 * delta    为非0时发送命令 AT+QCSQ=<enable>,<delta>
 ************************************************************************************
*/
int svc_network_set_qcsq_sync(uint8_t enable, uint8_t delta);

#ifdef __cplusplus
}
#endif
#endif /* __SRV_NETWORK_H__ */
