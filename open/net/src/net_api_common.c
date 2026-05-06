/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file    net_api_common.c
 *
 * @brief: 网络对外接口.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-28     ict team          创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "net_api_common.h"


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/


/************************************************************************************
 *                                 局部函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
static uint8_t g_TcpdumpFlag = 0;


/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
static void NET_ApiIfInfoGet(struct netif *inp , NET_ApiIfInfo *IfInfo)
{
    IfInfo->cid = inp->cid;
    IfInfo->ip_flg = inp->ip_state;
    memcpy(IfInfo->ifName,inp->name,NET_API_IF_NAME_SIZE);
    ip4_addr_copy(IfInfo->ifIp4Addr,*ip_2_ip4(&(inp->ip_addr)));
    ip4_addr_copy(IfInfo->ifIp4Netmask,*ip_2_ip4(&(inp->netmask)));

    for(int i = 0; i < NET_API_IF_IP6_ADDR_NUM; i++)
    {
        ip6_addr_copy(IfInfo->ifIp6Addr[i],*ip_2_ip6(&(inp->ip6_addr[i])));
    }

    return;
}

/**
 ************************************************************************************
 * @brief           通过 CID 查找对应的网络接口信息
 *
 * @param[in]       cid         PDP 上下文 ID
 * @param[out]      *IfInfo     查找到的网络接口信息
 *
 * @return          ERR_OK: 成功查找接口信息; 其它: 查找接口信息失败
 ************************************************************************************
*/
int8_t NET_ApiIfInfoGetByCid(uint8_t cid, NET_ApiIfInfo *IfInfo)
{
    struct netif *inp = NULL;

    if(NULL == IfInfo)
    {
        MID_NET_PRINT_ERROR("%s, IfInfo is null\r\n", __FUNCTION__);
        return ERR_MEM;
    }

    inp = netif_get_by_cid(cid);
    if(NULL == inp) /* 没有找到 cid 对应的 netif */
    {
        MID_NET_PRINT_ERROR("%s, inp is null\r\n", __FUNCTION__);
        return ERR_RTE;
    }
    if(!netif_is_up(inp))
    {
        MID_NET_PRINT_ERROR("%s, neit not up\r\n", __FUNCTION__);
        return ERR_VAL;
    }

    NET_ApiIfInfoGet(inp, IfInfo);

    return ERR_OK;
}

/**
 ************************************************************************************
 * @brief           设置 CID 对应的数据抓包标志
 *
 * @param[in]       cid      PDP 上下文 ID (1-8)
 * @param[out]      flag     TCPDUMP 抓包的标志. 0 表示关闭; 1表示打开
 *
 * @return          ERR_OK: 设置成功; 其它: 设置失败
 ************************************************************************************
*/
int8_t NET_ApiTcpdumpFlagSetByCid(uint8_t cid, uint8_t flag)
{
    if(!((cid >= NET_CID_MIN) && (cid <= NET_CID_MAX)))
    {
        MID_NET_PRINT_ERROR("%s, err cid[%u]\r\n", __FUNCTION__, cid);
        return ERR_MEM;
    }

    if(NET_API_TCPDUMP_FLAG_CLOSE == flag)
    {
        g_TcpdumpFlag &= (~(1 << (cid - NET_CID_MIN)) & 0xFF);
    }
    else if(NET_API_TCPDUMP_FLAG_OPEN == flag)
    {
        g_TcpdumpFlag |= ((1 << (cid - NET_CID_MIN)) & 0xFF);
    }
    else
    {
        MID_NET_PRINT_ERROR("%s, err flag[%u]\r\n", __FUNCTION__, flag);
        return ERR_VAL;
    }

    return ERR_OK;
}

/**
 ************************************************************************************
 * @brief           获取 CID 对应的数据抓包标志
 *
 * @param[in]       cid      PDP 上下文 ID (1-8)
 *
 * @return          抓包标志. 0: 关闭; 1: 打开
 ************************************************************************************
*/
uint8_t NET_ApiTcpdumpFlagReadByCid(uint8_t cid)
{
    uint8_t flag = 0;

    if(!((cid >= NET_CID_MIN) && (cid <= NET_CID_MAX)))
    {
        MID_NET_PRINT_ERROR("%s, cid err[%d]\r\n", __FUNCTION__, cid);
        return flag;
    }

    flag = (g_TcpdumpFlag >> (cid - NET_CID_MIN)) & 0x01;
    return flag;
}

