/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file    net_api_common.h
 *
 * @brief
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-28     ict team          创建
 ************************************************************************************
 */
#ifndef __NET_API_COMMON_H__
#define __NET_API_COMMON_H__

#include "lwip/ip4_addr.h"
#include "lwip/ip6_addr.h"
#include "lwip/netif.h"

/**
 * @addtogroup NetApi
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define NET_API_IF_NAME_SIZE          (2)
#define NET_API_IF_IP6_ADDR_NUM       (2)


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum{
    NET_API_IF_IP_NONE = 0,
    NET_API_IF_IP_4 = 1,
    NET_API_IF_IP_6 = 2,
    NET_API_IF_IP_4_6 = 3,
    NET_API_IF_IP_MAX,
}NET_ApiIfIpType;

typedef enum{
    NET_API_TCPDUMP_FLAG_CLOSE = 0, /* 抓包关闭 */
    NET_API_TCPDUMP_FLAG_OPEN = 1, /* 抓包打开 */
}NET_ApiTcpdumpFlag; /* CID 对应的抓包标志 */

typedef struct
{
    uint8_t  cid;
    uint8_t  ip_flg; /* IPv4 或 IPv6 地址是否获取到的状态标记, bit0 表示 ip4 地址, bit1 表示 ip6 地址. 值为 1 表示已获取到ip地址, 值为 0 表示未获取到ip地址 */
    char     ifName[NET_API_IF_NAME_SIZE];
    ip4_addr_t ifIp4Addr;
    ip4_addr_t ifIp4Netmask;
    ip6_addr_t ifIp6Addr[NET_API_IF_IP6_ADDR_NUM];
}NET_ApiIfInfo; /* 对外提供的网络接口信息 */


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief           通过 CID 查找对应的网络接口信息
 *
 * @param[in]       cid         PDP 上下文 ID
 * @param[out]      *IfInfo     查找到的网络接口信息
 *
 * @return          ERR_OK: 查找接口信息成功; 其它: 查找接口信息失败
 ************************************************************************************
*/
int8_t NET_ApiIfInfoGetByCid(uint8_t cid, NET_ApiIfInfo *IfInfo);

/**
 ************************************************************************************
 * @brief           设置 CID 对应的数据抓包标志
 *
 * @param[in]       cid      PDP 上下文 ID (1-8)
 * @param[out]      flag     TCPDUMP 抓包标志. 0 表示关闭; 1表示打开,参考 NET_ApiTcpdumpFlag
 *
 * @return          ERR_OK: 设置成功; 其它: 设置失败
 ************************************************************************************
*/
int8_t NET_ApiTcpdumpFlagSetByCid(uint8_t cid, uint8_t flag);

/**
 ************************************************************************************
 * @brief           获取 CID 对应的数据抓包标志
 *
 * @param[in]       cid      PDP 上下文 ID (1-8)
 *
 * @return          抓包标志. 0: 关闭; 1: 打开
 ************************************************************************************
*/
uint8_t NET_ApiTcpdumpFlagReadByCid(uint8_t cid);

#endif

/** @} */
