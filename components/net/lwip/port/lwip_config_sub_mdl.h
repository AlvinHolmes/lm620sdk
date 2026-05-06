#ifndef __LWIP_CONFIG_SUB_MBL_H__
#define __LWIP_CONFIG_SUB_MBL_H__

/*  数传模块 LWIP NUM 定义 */

#define IP_FRAG                     1

/* NUM of LwIP */

#define LWIP_MEMP_NUM_NETCONN   14 ///Recommend:14   Reference:8
#define LWIP_PBUF_NUM           16
#ifdef USE_TOP_PPP
#define LWIP_PBUF_POOL_NUM      8
#else
#define LWIP_PBUF_POOL_NUM      6
#endif
#define LWIP_RAW_PCB_NUM        2  ///Recommend:2 Reference:4
#define LWIP_UDP_PCB_NUM        14 ///Recommend:14 Reference:4
#define LWIP_TCP_PCB_NUM        12 ///Recommend:12 Reference:4
#define LWIP_TCP_SEG_NUM        13 ///Recommend:13 Reference:40
#define MEMP_NUM_PBUF            12 ///Recommend:12   Reference:256
#define MEMP_NUM_ARP_QUEUE       2 ///Recommend:32   Reference:30
#define MEMP_NUM_FRAG_PBUF       25 ///Recommend:25   Reference:15
#define MEMP_NUM_ND6_QUEUE       15 ///Recommend:15   Reference:20
#define MEMP_NUM_NETBUF          24 ///Recommend:24   Reference:2
#define MEMP_NUM_NETDB           2  ///Recommend:2    Reference:1
#define MEMP_NUM_TCPIP_MSG_API   128 ///Recommend:24   Reference:8
#define MEMP_NUM_TCPIP_MSG_INPKT 64  ///Recommend:3    Reference:256
#define MEMP_NUM_TCP_PCB_LISTEN  2  ///Recommend:2    Reference:8
#define LWIP_ND6_NUM_NEIGHBORS   5  ///Recommend:5    Reference:10
#define LWIP_ND6_NUM_DESTINATIONS 5

/* end NUM of LwIP */

/* IPSec */
//#define LWIP_USING_IPSEC
/* end of IPSec */

#endif /* __LWIP_CONFIG_SUB_MBL_H__ */
