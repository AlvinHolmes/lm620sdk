/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file example_socket_server.c
*
* @brief  socket server 示例文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "os.h"
#include "lwip/api.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
static void DEMO_SocketServerClose(int *fd);
static void DEMO_SocketServerCloseAll(void);

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define DEMO_SOCKET_SERVER_DBG_ON
#define DEMO_SOCKET_SERVER_ERR_ON
#ifdef DEMO_SOCKET_SERVER_DBG_ON
#define DEMO_SOCKET_SERVER_DBG_PRINTF               osPrintf
#else
#define DEMO_SOCKET_SERVER_DBG_PRINTF(...)
#endif

#ifdef DEMO_SOCKET_SERVER_ERR_ON
#define DEMO_SOCKET_SERVER_ERR_PRINTF               osPrintf
#else
#define DEMO_SOCKET_SERVER_ERR_PRINTF(...)
#endif

#define DEMO_SOCKET_SERVER_SEND_LEN                  (128)

#define DEMO_SOCKET_SERVER_TASK_STACK_SIZE           (1024 * 4)
#define DEMO_SOCKET_SERVER_TASK_MAX_QUEUE_NUM        (32)
#define DEMO_SOCKET_SERVER_TASK_PRIO                 (osPriorityAboveNormal3)
#define DEMO_SOCKET_LISETN_TASK_STACK_SIZE           (2048)
#define DEMO_SOCKET_LISETN_TASK_PRIORITY             (osPriorityNormal3)

#define DEMO_SOCKET_SERVER_MAX_PRINT_ONCE            (126)
#define DEMO_SOCKET_SERVER_TCP_MSL                   (60000UL)
#define DEMO_SOCKET_SERVER_HOST_NAME_LEN             (DNS_MAX_NAME_LENGTH)

#define DEMO_SOCKET_SERVER_CONNECT_ID_MIN            (0)
#define DEMO_SOCKET_SERVER_CONNECT_ID_MAX            (15)
#define DEMO_SOCKET_SERVER_CONNECT_CNT               (DEMO_SOCKET_SERVER_CONNECT_ID_MAX - DEMO_SOCKET_SERVER_CONNECT_ID_MIN + 1) /* 可连接的 TCP 客户端的个数 */


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct
{
    uint8_t startFlag;
    uint8_t protoType;
    uint16_t localPort;
    ip_addr_t localIp;
    uint16_t remotePort;
    ip_addr_t remoteIp;
    char host[DEMO_SOCKET_SERVER_HOST_NAME_LEN];
}DEMO_SocketServerParam;

typedef struct
{
    int fd;             /* fd */
    uint32_t event;     /* 回调事件 */
    uint32_t dataLen;   /* 数据长度 */
}DEMO_SocketServerTaskMessage;

typedef enum
{
    DEMO_SOCKET_SERVER_OPTION_HELP = 0,         /* -h */
    DEMO_SOCKET_SERVER_OPTION_CREATE,           /* create */
    DEMO_SOCKET_SERVER_OPTION_SEND,             /* send data */
    DEMO_SOCKET_SERVER_OPTION_STOP,             /* stop */
    DEMO_SOCKET_SERVER_OPTION_MAX,
}DEMO_SocketServerOption;        /* demo socket server 测试类型 */

typedef enum
{
    DEMO_SOCKET_SERVER_START = 1,
    DEMO_SOCKET_SERVER_STOP,
}DEMO_SocketServerStartFlag;

typedef enum
{
    DEMO_SOCKET_SERVER_TCP = 1,
    DEMO_SOCKET_SERVER_UDP,
}DEMO_SocketServerProtoType;

typedef enum
{
    DEMO_SOCKET_SERVER_CLOSE_MODE_WATI_DATA = 0,      /* 等待发送缓存区数据发送完毕后，关闭 TCP 连接 */
    DEMO_SOCKET_SERVER_CLOSE_MODE_NOW,                /* 立即关闭不等待缓存区数据发送完毕 */
    DEMO_SOCKET_SERVER_CLOSE_MODE_WAIT_2MSL,          /* 等待 2MSL (Maximum Segment Lifetime, 最大分段) 后关闭 */
    DEMO_SOCKET_SERVER_CLOSE_MODE_SEND_RST,           /* 向服务器发送 RST 消息重置连接后关闭 */
}DEMO_SocketServerCloseMode; /* TCP/IP 关闭模式 */

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static DEMO_SocketServerParam g_ServerParam = {0};
static osThreadId_t g_ServerListenThreadId = NULL;
static osThreadId_t g_ServerThreadId = NULL;
static osMessageQueueId_t g_ServerTaskMQ = NULL;
static int g_ServerFd = -1;
static int g_ServerAcceptFd[DEMO_SOCKET_SERVER_CONNECT_CNT] = {-1};

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void dbg_hex_data(char *name, char *pdata, uint16_t len)
{
    uint32_t i;

    osPrintf("[%s],len[%d]\r\n", name, len);
    for(i = 0; i < len; i++)
    {
        if (i % 16 == 0)
        {
            osPrintf("%04x:  ", i);
        }
        osPrintf("%02x ", pdata[i]);

        if (((i + 1) % 8 == 0) && (((i + 1) % 16) != 0))
        {
            osPrintf(" ");
        }
        if(((i + 1) % 16) == 0)
        {
            osPrintf("\r\n");
        }
    }
    osPrintf("\r\n");

    return;
}

/* demo_socket_server help*/
static void DEMO_SocketServerHelp(void)
{
    osPrintf("usage: [command] [options] [local_ip] [local_port]\r\n");

    osPrintf("       command:\r\n");
    osPrintf("              demo_socket_server            [demo socket server client start]\r\n");

    osPrintf("       options:\r\n");
    osPrintf("              -h                        [display help]\r\n");
    osPrintf("              tcp                       [tcp]\r\n");
    osPrintf("              udp                       [udp]\r\n");
    osPrintf("              stop                      [stop demo]\r\n");
    osPrintf("              send                      [send data]\r\n");

    osPrintf("        local_ip                        [local ip address]\r\n");
    osPrintf("        local_port                      [local port: 0~65535]\r\n");

    return;
}

static uint8_t DEMO_SocketServerOptionGet(char **argv)
{
    int8_t i = -1;
    char *option[] ={"-h", "create", "send", "stop"};

    for(i = 0; i < DEMO_SOCKET_SERVER_OPTION_MAX; i++)
    {
        if(!strcmp((argv[1]), option[i]))
        {
            break;
        }
    }

    return i;
}

static int8_t DEMO_SocketServerFreeConnectIdFind(void)
{
    int8_t i = -1;
    uint8_t connectId =0;
    for(connectId = DEMO_SOCKET_SERVER_CONNECT_ID_MIN; connectId <= DEMO_SOCKET_SERVER_CONNECT_ID_MAX; connectId++)
    {
        if(g_ServerAcceptFd[connectId] < 0)
        {
            i = connectId;
            break;
        }
    }
    return i;
}

static int8_t DEMO_SocketServerAccepFdFind(int fd)
{
    int8_t i = -1;
    uint8_t connectId =0;

    for(connectId = DEMO_SOCKET_SERVER_CONNECT_ID_MIN; connectId <= DEMO_SOCKET_SERVER_CONNECT_ID_MAX; connectId++)
    {
        if((g_ServerAcceptFd[connectId] == fd) && (g_ServerFd != fd))
        {
            i = connectId;
            break;
        }
    }

    return i;
}

static int* DEMO_SocketServerFdFind(int fd)
{
    int *ret = NULL;
    uint8_t connectId =0;

    if(g_ServerFd == fd)
    {
        return &g_ServerFd;
    }

    for(connectId = DEMO_SOCKET_SERVER_CONNECT_ID_MIN; connectId <= DEMO_SOCKET_SERVER_CONNECT_ID_MAX; connectId++)
    {
        if(g_ServerAcceptFd[connectId] == fd)
        {
            ret = &g_ServerAcceptFd[connectId];
            break;
        }
    }
    return ret;
}

/* 设置 TCP 关闭模式 */
static int32_t DEMO_SocketServerSetSockoptLinger(int fd, uint8_t closeMode)
{
    int8_t ret = -1;
    struct linger soLinger;

    switch(closeMode)
    {
        case DEMO_SOCKET_SERVER_CLOSE_MODE_WATI_DATA:
            soLinger.l_onoff = 0;
            soLinger.l_linger = 0;
            break;

        case DEMO_SOCKET_SERVER_CLOSE_MODE_NOW:
        case DEMO_SOCKET_SERVER_CLOSE_MODE_SEND_RST:
            soLinger.l_onoff = 1;
            soLinger.l_linger = 0;
            break;

        case DEMO_SOCKET_SERVER_CLOSE_MODE_WAIT_2MSL:
            soLinger.l_onoff = 1;
            soLinger.l_linger = 2 * DEMO_SOCKET_SERVER_TCP_MSL;
            break;

        default:
            break;
    }

    ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, &soLinger, sizeof(soLinger));
    if(ret != 0)
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("set socket option linger fail [%d]\r\n", ret);
        return osError;
    }

    return osOK;
}

static osStatus_t DEMO_SocketServerDataQueuePut(int fd, unsigned int event, int len)
{
    DEMO_SocketServerTaskMessage msg;
    osStatus_t ret = osError;

    OS_ASSERT(g_ServerTaskMQ != NULL);

    msg.fd = fd;
    msg.event = event;
    msg.dataLen = len;

    ret = osMessageQueuePut(g_ServerTaskMQ, &msg, 0, 0);

    return ret;
}

static void DEMO_SocketServerLwipEventCallback(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param)
{
    int32_t ret = -1;
    DEMO_SOCKET_SERVER_ERR_PRINTF("lwipEventCallback: fd [%d], event [%d], len [%d]\r\n", fd, event, len);
    ret = DEMO_SocketServerDataQueuePut(fd, event, len);
    if(osOK != ret)
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("recv from lwip queue put fail\r\n");
        return;
    }

    return;
}

static int32_t DEMO_SocketServerDataRecvFromLwip(int fd, uint32_t len)
{
    int16_t recvSize = 0;
    char *recvBuf = NULL;
    socklen_t fromLen = sizeof(struct sockaddr);
    struct sockaddr remoteSockAddr;

    recvBuf = osMalloc(len);
    if(NULL == recvBuf)
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("normal memory alloc fail\r\n");
        return osError;
    }

    memset(&remoteSockAddr, 0, sizeof(struct sockaddr));

    /* NETCONN_DONTBLOCK 表示以非阻塞方式接收.
      如果想以阻塞的方式接收, 需要使用 lwip_fcntl 设置为阻塞, 同时将 lwip_recvfrom 函数的第四个参数 flags 设置为 0.
      UDP 作为服务器时, 需要采用 lwip_recvfrom 把客户端 IP 地址和端口号记录下来, 否则 sendto 的时候不指定客户端 IP 地址和端口号, 数据将发不出去*/
    recvSize = lwip_recvfrom(fd, recvBuf, len, NETCONN_DONTBLOCK, &remoteSockAddr, &fromLen);
    if(recvSize <= 0)
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("server recv from lwip fail, recvSize[%d]\r\n", recvSize);
        if(NULL != recvBuf)
        {
            osFree(recvBuf);
            recvBuf = NULL;
        }
        return osErrorTimeout;
    }

    /* 收到客户端发过来的数据时, 将远端 IP 地址和端口号保存, 以便 UDP 发送数据能成功发送到客户端 */
    DEMO_SOCKET_SERVER_ERR_PRINTF("server recv from lwip sa_family[%d]\r\n", remoteSockAddr.sa_family);
    if(AF_INET == remoteSockAddr.sa_family)
    {
        struct sockaddr_in remoteSockAddr4;
        memset(&remoteSockAddr4, 0, sizeof(struct sockaddr_in));
        memcpy(&remoteSockAddr4, &remoteSockAddr, sizeof(struct sockaddr_in));

        g_ServerParam.remotePort = ntohs(remoteSockAddr4.sin_port);
     //   g_ServerParam.remoteIp.u_addr.ip4.addr = remoteSockAddr4.sin_addr.s_addr;
        memcpy(ip_2_ip4(&g_ServerParam.remoteIp),&(remoteSockAddr4.sin_addr), sizeof(struct sockaddr_in));
        DEMO_SOCKET_SERVER_DBG_PRINTF("recv: remote_port[%d], remote_ip [%s]\r\n", htons(remoteSockAddr4.sin_port), \
                        inet_ntoa(remoteSockAddr4.sin_addr.s_addr));
    }
    else if(AF_INET6 == remoteSockAddr.sa_family)
    {
        struct sockaddr_in6 remoteSockAddr6;
        memset(&remoteSockAddr6, 0, sizeof(struct sockaddr_in6));
        memcpy(&remoteSockAddr6, &remoteSockAddr, sizeof(struct sockaddr_in6));

        g_ServerParam.remotePort = ntohs(remoteSockAddr6.sin6_port);
        memcpy(ip_2_ip6(&g_ServerParam.remoteIp),&(remoteSockAddr6.sin6_addr), sizeof(struct in6_addr));
        DEMO_SOCKET_SERVER_DBG_PRINTF("recv6: remote_port[%d], remote_ip [%s]\r\n", htons(remoteSockAddr6.sin6_port), \
                        inet6_ntoa(remoteSockAddr6.sin6_addr.s6_addr));
    }

    /* lwip_fcntl 设置为阻塞 */
    /*
    int flags = 0;
    flags = lwip_fcntl(fd, F_GETFL, 0);
    lwip_fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    */

    DEMO_SOCKET_SERVER_DBG_PRINTF("server rrecv from lwip ok, recvSize[%d]\r\n", recvSize);
    dbg_hex_data("DEMO_SocketServerDataRecvFromLwip", recvBuf, recvSize);
    if(NULL != recvBuf)
    {
        osFree(recvBuf);
        recvBuf = NULL;
    }

    return osOK;
}

static void DEMO_SocketServerTaskProcess(DEMO_SocketServerTaskMessage msg)
{
    int *findFd = NULL;
    int fd = msg.fd;
    uint32_t event = msg.event;
    uint32_t dataLen = msg.dataLen;

    findFd = DEMO_SocketServerFdFind(fd);
    if(NULL == findFd)
    {
        DEMO_SOCKET_SERVER_DBG_PRINTF("DEMO_SocketServerTaskProcess: fd[%d] not find\r\n", fd);
        return;
    }
    DEMO_SOCKET_SERVER_DBG_PRINTF("DEMO_SocketServerTaskProcess:fd[%d][%d], event[%d], dataLen[%d]\r\n", fd, *findFd, event, dataLen);
    switch(event)
    {
        case SOCKET_TCP_EVENT_ACCEPT:  /* 作为 TCP 服务器, 已接收到 TCP 客户端的连接 */
            break;

        case SOCKET_TCP_EVENT_SENT:  /* TCP 时, 已发送数据 */
            break;

        case SOCKET_TCP_EVENT_RECV:  /* 接收到的数据 */
            if(DEMO_SOCKET_SERVER_TCP == g_ServerParam.protoType)
            {
                if(0 == dataLen) /* 收到对端的 TCP 关闭请求 */
                {
                    DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServer tcp server recv close, fd[%d][%d]\r\n", fd, *findFd);
                    DEMO_SocketServerClose(findFd);
                }
                else /* 收到 TCP 数据 */
                {
                    DEMO_SocketServerDataRecvFromLwip(*findFd, dataLen);
                }
            }
            else if(DEMO_SOCKET_SERVER_UDP == g_ServerParam.protoType) /* 收到 UDP 数据 */
            {
                DEMO_SocketServerDataRecvFromLwip(*findFd, dataLen);
            }
            break;

        case SOCKET_TCP_EVENT_CONNECTED:  /* 作为 TCP 客户端, 连接成功 */
            DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServer tcp server connect ok, fd[%d][%d]\r\n", fd, *findFd);
            break;

        case SOCKET_TCP_EVENT_POLL:
            break;

        case SOCKET_TCP_EVENT_ERR:  /* TCP 错误提示, 如: 收到对端 RST */
            DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServer tcp server recv rst, fd[%d][%d]\r\n", fd, *findFd);
            DEMO_SocketServerClose(findFd);
            break;

        case SOCKET_NETIF_EVENT_DOWN:  /* netif down (如: PDP 去激活了) */
            DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServer tcp server netif down, fd[%d]\r\n", *findFd);
            DEMO_SocketServerClose(findFd);
            break;

        default:
            DEMO_SOCKET_SERVER_ERR_PRINTF("lwipEventCallback:server event out of range [%d]\r\n", event);
            break;
    }

    return;
}

static void DEMO_SocketServerTaskEntry(void *param)
{
    while(1)
    {
        DEMO_SocketServerTaskMessage msg = {0};
        if(osMessageQueueGet(g_ServerTaskMQ, &msg, 0, osWaitForever) == osOK)
        {
            DEMO_SOCKET_SERVER_DBG_PRINTF("DEMO_SocketServerTaskEntry, msgFd [%d]\r\n", msg.fd);
            DEMO_SocketServerTaskProcess(msg);
        }
        else
        {
            continue;
        }
    }
    return;
}

static void DEMO_SocketServerParamSet(char **argv)
{
    memset(&g_ServerParam, 0, sizeof(DEMO_SocketServerParam));
    g_ServerParam.startFlag = DEMO_SOCKET_SERVER_START;
    if(!strcmp((argv[2]), "tcp"))
    {
        g_ServerParam.protoType = DEMO_SOCKET_SERVER_TCP;
    }
    else if(!strcmp((argv[2]), "udp"))
    {
        g_ServerParam.protoType = DEMO_SOCKET_SERVER_UDP;
    }

    strncpy(g_ServerParam.host, argv[3], sizeof(g_ServerParam.host) - 1);
    g_ServerParam.host[sizeof(g_ServerParam.host) - 1] = '\0';
    g_ServerParam.localPort = atoi(argv[4]);
    DEMO_SOCKET_SERVER_DBG_PRINTF("server param set, protoType[%s], host[%s], localPort[%d]\r\n", \
                                argv[1], g_ServerParam.host, g_ServerParam.localPort);

    return;
}

static void DEMO_SocketServerListenTask(void *parameter)
{
    fd_set serverReadFds = {0}; /* TCP 监听是否有数据来到的读文件描述符集合, 用于 select */
    fd_set readfds = {0};
    struct timeval timeout;
    socklen_t sinSize = sizeof(struct sockaddr_in);
    struct sockaddr_in clientAddr;
    int8_t acceptId = -1;
    int acceptFd = -1;

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    /* 先清空, 以免出现随机值 */
    FD_ZERO(&serverReadFds);
    FD_SET(g_ServerFd, &serverReadFds);

    while (1)
    {
        readfds = serverReadFds; /* 每次都用初始 FD_SET 的文件描述符集合 */

        if (select(g_ServerFd + 1, &readfds, OS_NULL, OS_NULL, &timeout) == 0)
        {
            continue;
        }

        acceptId = DEMO_SocketServerFreeConnectIdFind();
        if(acceptId == -1)
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("tcp listen task client incoming full\r\n");
            acceptFd = lwip_accept(g_ServerFd, (struct sockaddr *)&clientAddr, &sinSize);
            DEMO_SocketServerClose(&acceptFd); /* 关闭 TCP/IP 连接 */
            DEMO_SOCKET_SERVER_DBG_PRINTF("incoming full, clear fd\r\n");
            continue;
        }

        g_ServerAcceptFd[acceptId] = lwip_accept(g_ServerFd, (struct sockaddr *)&clientAddr, &sinSize);
        if(g_ServerAcceptFd[acceptId] < 0)
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket accept fail, acceptId[%d][%d]\r\n", acceptId, g_ServerAcceptFd[acceptId]);
            g_ServerListenThreadId = NULL;
            return;
        }
        DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket accept ok, acceptFd[%d]\r\n", g_ServerAcceptFd[acceptId]);

        lwip_socket_register_callback(g_ServerAcceptFd[acceptId], DEMO_SocketServerLwipEventCallback, NULL);
        if(osOK != DEMO_SocketServerSetSockoptLinger(g_ServerAcceptFd[acceptId], DEMO_SOCKET_SERVER_CLOSE_MODE_NOW)) /* 设置为立即关闭属性, 客户端关闭 TCP 连接时,可以立即使用同一个本地端口号建立 TCP连接 */
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("set socket option linger fail\r\n");
            DEMO_SocketServerCloseAll();
            return;
        }
    }

    return;
}

static void DEMO_SocketServerCreate(void)
{
    int fd = -1;
    int32_t ret = -1;
    uint8_t domainType = 0; /* socket domain 采用数据包还是流 */

    /* 将字符串形式的 IP 地址(第一个参数 host)转换得到十六进制形式的 IP 地址(第二个参数). */
    ipaddr_aton(g_ServerParam.host, &g_ServerParam.localIp);

    if(DEMO_SOCKET_SERVER_TCP == g_ServerParam.protoType)
    {
        domainType = SOCK_STREAM;
    }
    else if(DEMO_SOCKET_SERVER_UDP == g_ServerParam.protoType)
    {
        domainType = SOCK_DGRAM;
    }

    /* 创建套接字 socket */
    if (IP_IS_V4(&g_ServerParam.localIp)) /* IPv4 */
    {
        fd = lwip_socket(AF_INET, domainType, IPPROTO_IP);
    }
    else /* IPv6 */
    {
        fd = lwip_socket(AF_INET6, domainType, IPPROTO_IP);
    }
    if(fd < 0)
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServer create fail, fd[%d]\r\n", fd);
        return;
    }
    g_ServerFd = fd;
    DEMO_SOCKET_SERVER_DBG_PRINTF("DEMO_SocketServer create ok, fd[%d]\r\n", fd);

    /* 注册 LWIP 事件回调函数 */
    if(0 != lwip_socket_register_callback(g_ServerFd, DEMO_SocketServerLwipEventCallback, NULL))
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServer register callback fail\r\n");
        lwip_socket_unregister_callback(g_ServerFd); /* 注销 LWIP 事件回调 */
        lwip_close(g_ServerFd); /* 关闭 socket */
        g_ServerFd = -1;
        return;
    }

    /* 绑定套接字 bind */
    if(IP_IS_V4(&g_ServerParam.localIp)) /* IPv4 */
    {
        struct sockaddr_in localSockAddr;
        memset(&localSockAddr, 0, sizeof(struct sockaddr_in));

        /* 本端地址信息, 用于绑定 bind 套接字 */
        localSockAddr.sin_family = AF_INET;
        localSockAddr.sin_port = htons(g_ServerParam.localPort);

        /*本地 ip 地址可不设置, 默认就是"0.0.0.0"; 也可指定设置成"0.0.0.0"; 但不能设置成IP4_ADDR_ANY(0)，因为0经 inet_addr 之后地址会变成255.255.255.255
        如果本地 ip 地址设置为0.0.0.0, 那么在 sendto 走到 udp_sendto_if 时,   会将 netif 的 ip 地址赋给发送源地址, 因此在 bind 的时候可指定本地 ip 地址为0.0.0.0 */
        //localAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
        if(lwip_bind(g_ServerFd, (struct sockaddr *)&localSockAddr, sizeof(struct sockaddr)) < 0)
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("socket bind4 fail\r\n");
            lwip_socket_unregister_callback(g_ServerFd); /* 注销 LWIP 事件回调 */
            lwip_close(g_ServerFd); /* 关闭 TCP/IP 连接 */
            g_ServerFd = -1;
            return;
        }
    }
    else /* IPv6 */
    {
        struct sockaddr_in6 localSockAddr6;
        memset(&localSockAddr6, 0, sizeof(struct sockaddr_in6));

        /* 本端地址信息, 用于绑定 bind 套接字 */
        localSockAddr6.sin6_family = AF_INET6;
        localSockAddr6.sin6_port = htons(g_ServerParam.localPort);

        //ipaddr_aton(IP6_ADDR_ANY, &ip6Addr); /* 本地ipv6地址可不设置, 如果设置成 IP6_ADDR_ANY, 会得到意想不到的地址 */
        //memcpy(localAddr6.sin6_addr.s6_addr, ip_2_ip6(&ip6Addr),sizeof(localAddr6.sin6_addr.s6_addr));
        if(lwip_bind(g_ServerFd, (struct sockaddr *)&localSockAddr6, sizeof(struct sockaddr)) < 0)
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("socket bind6 fail\r\n");
            lwip_socket_unregister_callback(g_ServerFd); /* 注销 LWIP 事件回调 */
            lwip_close(g_ServerFd); /* 关闭 TCP/IP 连接 */
            g_ServerFd = -1;
            return;
        }
    }
    DEMO_SOCKET_SERVER_DBG_PRINTF("DEMO_SocketServerBind ok\r\n");

#if 0 /* UDP 使用 connect, 可以将远端端口号和远端 IP 地址绑定到 UDP PCB 上, 可以使用 send(或 sendto) 给客户端主动先发送数据 */
    uint16_t remotePort = 0; /* 根据实际需要的远端端口号自行设置 */
    ip_addr_t remoteIp = {0}; /* 根据实际需要的远端 IP 地址自行设置 */
    if(DEMO_SOCKET_SERVER_UDP == g_ServerParam.protoType)
    {
        if (IP_IS_V4(&remoteIp)) /* IPV4 */
        {
            struct sockaddr_in remoteSockAddr;
            memset(&remoteSockAddr, 0, sizeof(struct sockaddr_in));

            /* 用于在 connect 时将远端IP地址和远端端口号赋给 TCP/UDP 的PCB */
            remoteSockAddr.sin_family = AF_INET;
            memcpy(&(remoteSockAddr.sin_addr), ip_2_ip4(&remoteIp), sizeof(struct in_addr));
            remoteSockAddr.sin_port = htons(remotePort);

            DEMO_SOCKET_SERVER_DBG_PRINTF("fd=[%d], connect port[%d], ip[%s]\r\n", g_ServerFd, \
                        ntohs(remoteSockAddr.sin_port), inet_ntoa(remoteSockAddr.sin_addr.s_addr));
            ret = lwip_connect(g_ServerFd, (struct sockaddr *)&remoteSockAddr, sizeof(struct sockaddr));
        }
        else /* IPV6 */
        {
            struct sockaddr_in6 remoteSockAddr6;
            memset(&remoteSockAddr6, 0, sizeof(struct sockaddr_in6));

            /* 用于在 connect 时将远端IP地址和远端端口号赋给 TCP/UDP 的PCB */
            remoteSockAddr6.sin6_family = AF_INET6;
            memcpy(&(remoteSockAddr6.sin6_addr), ip_2_ip6(&remoteIp), sizeof(struct in6_addr));
            remoteSockAddr6.sin6_port = htons(remotePort);

            ret = lwip_connect(g_ServerFd, (struct sockaddr *)&remoteSockAddr6, sizeof(struct sockaddr));
        }
        if(ret < 0)
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("connect fail [%d]\r\n", ret);
            lwip_socket_unregister_callback(g_ServerFd); /* 注销 LWIP 事件回调 */
            lwip_close(g_ServerFd); /* 关闭 TCP/IP 连接 */
            g_ServerFd = -1;
            return;
        }
    }
#endif

    if(DEMO_SOCKET_SERVER_TCP == g_ServerParam.protoType)
    {
        /* TCP 监听 */
        ret = lwip_listen(g_ServerFd, DEMO_SOCKET_SERVER_CONNECT_CNT);
        if(0 != ret)
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServerListen fail\r\n");
            lwip_socket_unregister_callback(g_ServerFd); /* 注销 LWIP 事件回调 */
            lwip_close(g_ServerFd); /* 关闭 TCP/IP 连接 */
            g_ServerFd = -1;
            return;
        }
        DEMO_SOCKET_SERVER_DBG_PRINTF("DEMO_SocketServer listen ok, fd[%d]\r\n", fd);

        /* 可以在 LWIP 事件回调函数中或创建任务的方式对接收到的客户端连接进行监听处理. 此处以创建任务为例说明 accept 的处理方法, 在LWIP 事件回调函数中有 UDP 对接收到的客户端连接的处理例子 */
        if (g_ServerListenThreadId == NULL)
        {
            osThreadAttr_t attr = {"demoSockListen", osThreadDetached, NULL, 0U, NULL, DEMO_SOCKET_LISETN_TASK_STACK_SIZE,
                                    DEMO_SOCKET_LISETN_TASK_PRIORITY, 0U, 0U};
            g_ServerListenThreadId = osThreadNew(DEMO_SocketServerListenTask, NULL, &attr);
            if (OS_NULL == g_ServerListenThreadId)
            {
                DEMO_SOCKET_SERVER_ERR_PRINTF("listen task create failed.\r\n");
                lwip_socket_unregister_callback(g_ServerFd); /* 注销 LWIP 事件回调 */
                lwip_close(g_ServerFd); /* 关闭 TCP/IP 连接 */
                g_ServerFd = -1;
                return;
            }
            DEMO_SOCKET_SERVER_DBG_PRINTF("listen task start ok.\r\n");
        }
    }

    return;
}

static void DEMO_SocketServerClose(int *fd)
{
    int32_t ret = -1;

    if(*fd >= 0)
    {
        lwip_socket_unregister_callback(*fd); /* 注销 LWIP 事件回调 */
        ret = lwip_close(*fd); /* 关闭 TCP/IP 连接 */
        if (ret < 0)
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServerClose fail, fd[%d] \r\n", *fd);
            return;
        }
        else
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServerClose ok, fd[%d]\r\n", *fd);
            *fd = -1;
        }
    }
    else
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("DEMO_SocketServerClose no need, fd[%d]\r\n", *fd);
    }

    return;
}

static void DEMO_SocketServerCloseAll(void)
{
    uint8_t i = -1;

    /* 作为 TCP 服务器时, 关闭服务器时, 需要先关闭每个 TCP 客户端的连接 */
    for(i = DEMO_SOCKET_SERVER_CONNECT_ID_MIN; i <= DEMO_SOCKET_SERVER_CONNECT_ID_MAX; i++)
    {
        DEMO_SocketServerClose(&g_ServerAcceptFd[i]);
    }

    DEMO_SocketServerClose(&g_ServerFd);
    DEMO_SOCKET_SERVER_DBG_PRINTF("DEMO_SocketServerCloseAll ok \r\n");

    return;
}

static int32_t DEMO_SocketServerTaskStart(void)
{
    osThreadAttr_t attr = {"demoSockServer", osThreadDetached, NULL, 0U, NULL, DEMO_SOCKET_SERVER_TASK_STACK_SIZE,
                            DEMO_SOCKET_SERVER_TASK_PRIO, 0U, 0U};
    osMessageQueueAttr_t msgQueueAttr = {"sock_server_msg", 0U, NULL, 0U, NULL, 0U};

    if(NULL == g_ServerTaskMQ)
    {
        g_ServerTaskMQ = osMessageQueueNew(DEMO_SOCKET_SERVER_TASK_MAX_QUEUE_NUM, sizeof(DEMO_SocketServerTaskMessage), &msgQueueAttr);
        if(NULL == g_ServerTaskMQ)
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server queue create fail\r\n");
            return osError;
        }
    }

    if(NULL == g_ServerThreadId)
    {
        g_ServerThreadId = osThreadNew(DEMO_SocketServerTaskEntry, NULL, &attr);
        if(NULL == g_ServerThreadId)
        {
            DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server thread create fail\r\n");
            if(g_ServerTaskMQ != NULL)
            {
                osMessageQueueDelete(g_ServerTaskMQ);
                g_ServerTaskMQ = NULL;
            }
            return osErrorTimeout;
        }
    }

    return osOK;
}

static void DEMO_SocketServerTaskStop(void)
{
   if(g_ServerThreadId != NULL)
   {
       osThreadTerminate(g_ServerThreadId);
       g_ServerThreadId = NULL;
   }

   if(g_ServerTaskMQ != NULL)
   {
       osMessageQueueDelete(g_ServerTaskMQ);
       g_ServerTaskMQ = NULL;
   }

   return;
}

static void DEMO_SocketServerDataSend(uint8_t acceptId)
{
    int ret = 0;
    int fd = -1;
    char sendBuf[DEMO_SOCKET_SERVER_SEND_LEN] = {0};

    if(DEMO_SOCKET_SERVER_STOP == g_ServerParam.startFlag)
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server not start, send fail\r\n");
        return;
    }


    for(int i = 0; i < sizeof(sendBuf); i++)
    {
        sendBuf[i] = i + 1;
    }

    if(DEMO_SOCKET_SERVER_TCP == g_ServerParam.protoType)
    {
        fd = g_ServerAcceptFd[acceptId];
    }
    else if(DEMO_SOCKET_SERVER_UDP == g_ServerParam.protoType)
    {
        fd = g_ServerFd;
    }

    if(-1 == fd)
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server not create, send fail\r\n");
        return;
    }

    if(IP_IS_V4(&g_ServerParam.remoteIp)) /* IPv4 */
    {
        struct sockaddr_in remoteSockAddr;
        memset(&remoteSockAddr, 0, sizeof(struct sockaddr_in));

        /* 本端地址信息, 用于绑定 bind 套接字 */
        remoteSockAddr.sin_family = AF_INET;
        remoteSockAddr.sin_port = htons(g_ServerParam.remotePort);
        memcpy(&(remoteSockAddr.sin_addr), ip_2_ip4(&(g_ServerParam.remoteIp)), sizeof(struct in_addr));

        /* MSG_DONTWAIT 表示以非阻塞方式发送.
          如果想以阻塞的方式发送, 需要使用 lwip_fcntl 设置为阻塞, 同时将 lwip_send 函数的第四个参数 flags 设置为 0*/
        DEMO_SOCKET_SERVER_DBG_PRINTF("demo socket server send, acceptId[%d], fd[%d]\r\n", acceptId, fd);
        DEMO_SOCKET_SERVER_DBG_PRINTF("remote_port[%d],sendto_ip [%s], send_len[%d]\r\n", htons(remoteSockAddr.sin_port), \
                        inet_ntoa(remoteSockAddr.sin_addr.s_addr), sizeof(sendBuf));
        /* 采用 sendto 的方式发送数据, 因为 UDP 服务器时要指定发送的客户端 IP 地址和端口号 */
        ret = lwip_sendto(fd, sendBuf, sizeof(sendBuf), MSG_DONTWAIT, (struct sockaddr *)&remoteSockAddr, sizeof(struct sockaddr));
    }
    else /* IPv6 */
    {
        struct sockaddr_in6 remoteSockAddr6;
        memset(&remoteSockAddr6, 0, sizeof(struct sockaddr_in6));

        /* 本端地址信息, 用于绑定 bind 套接字 */
        remoteSockAddr6.sin6_family = AF_INET6;
        remoteSockAddr6.sin6_port = htons(g_ServerParam.localPort);
        memcpy(&(remoteSockAddr6.sin6_addr), ip_2_ip6(&(g_ServerParam.remoteIp)), sizeof(struct in6_addr));

        /* MSG_DONTWAIT 表示以非阻塞方式发送.
          如果想以阻塞的方式发送, 需要使用 lwip_fcntl 设置为阻塞, 同时将 lwip_send 函数的第四个参数 flags 设置为 0*/
        DEMO_SOCKET_SERVER_DBG_PRINTF("demo socket server send6, acceptId[%d], fd[%d]\r\n", acceptId, fd);
        DEMO_SOCKET_SERVER_DBG_PRINTF("remote_port6[%d],sendto_ip [%s], send_len[%d]\r\n", htons(remoteSockAddr6.sin6_port), \
                        inet6_ntoa(remoteSockAddr6.sin6_addr.s6_addr), sizeof(sendBuf));
        /* 采用 sendto 的方式发送数据, 因为 UDP 服务器时要指定发送的客户端 IP 地址和端口号 */
        ret = lwip_sendto(fd, sendBuf, sizeof(sendBuf), MSG_DONTWAIT, (struct sockaddr *)&remoteSockAddr6, sizeof(struct sockaddr_in6));
    }

    if (ret > 0)
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("SendtoLwip ok, sendSize[%d]\r\n", ret);
    }
    else
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("SendtoLwip fail, sendSize[%d]\r\n", ret);
    }

    /* lwip_fcntl 设置为阻塞 */
    /*
    int flags = 0;
    flags = lwip_fcntl(fd, F_GETFL, 0);
    lwip_fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    */

    return;
}

static void DEMO_SocketServerEntry(char argc, char **argv)
{
    int8_t option = 0;

    if(1 == argc)
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server param count err\r\n");
        DEMO_SocketServerHelp();
        return;
    }

    option = DEMO_SocketServerOptionGet(argv);
    if((option < 0) || (option >= DEMO_SOCKET_SERVER_OPTION_MAX))
    {
        DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server type out of range, please input again. you can see help\r\n");
        return;
    }

    switch(option)
    {
        case DEMO_SOCKET_SERVER_OPTION_HELP:
            DEMO_SocketServerHelp(); /* demo_socket_server -h */
            break;

        case DEMO_SOCKET_SERVER_OPTION_CREATE:
            if(5 != argc)
            {
                DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server create count err\r\n");
                DEMO_SocketServerHelp();
                return;
            }
            if(DEMO_SOCKET_SERVER_START == g_ServerParam.startFlag)
            {
                DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server is running, please stop first\r\n");
                return;
            }
            memset(g_ServerAcceptFd, -1, sizeof(g_ServerAcceptFd));
            DEMO_SocketServerParamSet(argv);
            if(DEMO_SocketServerTaskStart())
            {
                DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server task start fail\r\n");
                return;
            }
            DEMO_SocketServerCreate();
            break;

        case DEMO_SOCKET_SERVER_OPTION_SEND:
            if(3 != argc)
            {
                DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server send count err\r\n");
                DEMO_SocketServerHelp();
                return;
            }
            uint8_t acceptId = atoi(argv[2]);
            DEMO_SocketServerDataSend(acceptId);
            break;

        case DEMO_SOCKET_SERVER_OPTION_STOP:
            if(2 != argc)
            {
                DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server stop count err\r\n");
                DEMO_SocketServerHelp();
                return;
            }
            g_ServerParam.startFlag = DEMO_SOCKET_SERVER_STOP;
            DEMO_SocketServerCloseAll();
            memset(&g_ServerParam, 0, sizeof(DEMO_SocketServerParam));
            g_ServerFd = -1;
            DEMO_SocketServerTaskStop();
            break;

        default:
            DEMO_SOCKET_SERVER_ERR_PRINTF("demo socket server paramer error, please input again. you can see help\r\n");
            break;
    }

    return;
}

/* 注册SHELL命令 */
/** example:
1. demo_socket_server create tcp 0.0.0.0   xxx    创建 TCP 服务器
2. demo_socket_server create udp 0.0.0.0   xxx   创建 UDP 服务器
3. demo_socket_server send    发送数据
4. demo_socket_server stop    关闭服务器
*/
#include "nr_micro_shell.h"
NR_SHELL_CMD_EXPORT(demo_socket_server, DEMO_SocketServerEntry);

