/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file example_socket_client.c
*
* @brief  socket client 示例文件.
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

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define DEMO_SOCKET_CLIENT_DBG_ON
#define DEMO_SOCKET_CLIENT_ERR_ON
#ifdef DEMO_SOCKET_CLIENT_DBG_ON
#define DEMO_SOCKET_CLIENT_DBG_PRINTF               osPrintf
#else
#define DEMO_SOCKET_CLIENT_DBG_PRINTF(...)
#endif

#ifdef DEMO_SOCKET_CLIENT_ERR_ON
#define DEMO_SOCKET_CLIENT_ERR_PRINTF               osPrintf
#else
#define DEMO_SOCKET_CLIENT_ERR_PRINTF(...)
#endif

#define DEMO_SOCKET_CLIENT_SEND_LEN                  (128)

#define DEMO_SOCKET_CLIENT_TASK_STACK_SIZE           (1024 * 4)
#define DEMO_SOCKET_CLIENT_TASK_MAX_QUEUE_NUM        (32)
#define DEMO_SOCKET_CLIENT_TASK_PRIO                 (osPriorityAboveNormal3)
#define DEMO_SOCKET_CLIENT_MAX_PRINT_ONCE            (126)
#define DEMO_SOCKET_CLIENT_TCP_MSL                   (60000UL)
#define DEMO_SOCKET_CLIENT_HOST_NAME_LEN             (DNS_MAX_NAME_LENGTH)


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef struct
{
    int fd;
    uint8_t startFlag;
    uint8_t protoType;
    uint16_t remotePort;
    ip_addr_t remoteIp;
    char host[DEMO_SOCKET_CLIENT_HOST_NAME_LEN];
}DEMO_SocketClientParam;

typedef struct
{
    int fd;             /* fd */
    uint32_t event;     /* 回调事件 */
    uint32_t dataLen;   /* 数据长度 */
}DEMO_SocketClientTaskMessage;

typedef enum
{
    DEMO_SOCKET_CLIENT_OPTION_HELP = 0,         /* -h */
    DEMO_SOCKET_CLIENT_OPTION_CREATE,           /* create */
    DEMO_SOCKET_CLIENT_OPTION_SEND,             /* send data */
    DEMO_SOCKET_CLIENT_OPTION_STOP,             /* stop */
    DEMO_SOCKET_CLIENT_OPTION_MAX,
}DEMO_SocketClientOption;        /* demo socket client 测试类型 */

typedef enum
{
    DEMO_SOCKET_CLIENT_START = 1,
    DEMO_SOCKET_CLIENT_STOP,
}DEMO_SocketClientStartFlag;

typedef enum
{
    DEMO_SOCKET_CLIENT_TCP = 1,
    DEMO_SOCKET_CLIENT_UDP,
}DEMO_SocketClientProtoType;

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static DEMO_SocketClientParam g_ClientParam = {0};
static osThreadId_t g_ClientThreadId = NULL;
static osMessageQueueId_t g_ClientTaskMQ = NULL;

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

/* demo_socket_client help*/
static void DEMO_SocketClientHelp(void)
{
    osPrintf("usage: [command] [options] [remote_host] [remote_port]\r\n");

    osPrintf("       command:\r\n");
    osPrintf("              demo_socket_client            [demo socket client client start]\r\n");

    osPrintf("       options:\r\n");
    osPrintf("              -h                        [display help]\r\n");
    osPrintf("              tcp                       [tcp]\r\n");
    osPrintf("              udp                       [udp]\r\n");
    osPrintf("              stop                      [stop demo]\r\n");
    osPrintf("              send                      [send data]\r\n");

    osPrintf("        remote_host                     [remote host domain or remote ip address]\r\n");
    osPrintf("        remote_port                     [remote port: 0~65535]\r\n");

    return;
}

static uint8_t DEMO_SocketClientOptionGet(char **argv)
{
    int8_t i = -1;
    char *option[] ={"-h", "create", "send", "stop"};

    for(i = 0; i < DEMO_SOCKET_CLIENT_OPTION_MAX; i++)
    {
        if(!strcmp((argv[1]), option[i]))
        {
            break;
        }
    }

    return i;
}

static void DEMO_SocketClientClose(int *fd)
{
    int32_t ret = -1;

    if(*fd >= 0)
    {
        lwip_socket_unregister_callback(*fd); /* 注销 LWIP 事件回调 */
        ret = lwip_close(*fd); /* 关闭 TCP/IP 连接 */
        if (ret < 0)
        {
            DEMO_SOCKET_CLIENT_ERR_PRINTF("DEMO_SocketClientClose fail, fd[%d]\r\n", *fd);
            return;
        }
        else
        {
            DEMO_SOCKET_CLIENT_ERR_PRINTF("DEMO_SocketClientClose ok, fd[%d]\r\n", *fd);
            *fd = -1;
        }
    }

    return;
}

static osStatus_t DEMO_SocketClientDataQueuePut(int fd, unsigned int event, int len)
{
    DEMO_SocketClientTaskMessage msg;
    osStatus_t ret = osError;

    OS_ASSERT(g_ClientTaskMQ != NULL);

    msg.fd = fd;
    msg.event = event;
    msg.dataLen = len;

    ret = osMessageQueuePut(g_ClientTaskMQ, &msg, 0, 0);

    return ret;
}

/*事件回调: 从 LWIP 收到的数据*/
static void DEMO_SocketClientLwipEventCallback(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param)
{
    int32_t ret = -1;
    DEMO_SOCKET_CLIENT_ERR_PRINTF("lwipEventCallback: fd [%d], event [%d], len [%d]\r\n", fd, event, len);
    ret = DEMO_SocketClientDataQueuePut(fd, event, len);
    if(osOK != ret)
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("client recv from lwip queue put fail\r\n");
        return;
    }

    return;
}

static int32_t DEMO_SocketClientDataRecvFromLwip(int fd, uint32_t len)
{
    int16_t recvSize = 0;
    char *recvBuf = NULL;

    recvBuf = osMalloc(len);
    if(NULL == recvBuf)
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("normal memory alloc fail\r\n");
        return osError;
    }

    /* NETCONN_DONTBLOCK 表示以非阻塞方式接收.
      如果想以阻塞的方式接收, 需要使用 lwip_fcntl 设置为阻塞, 同时将 lwip_recv 函数的第四个参数 flags 设置为 0*/
    recvSize = lwip_recv(fd, recvBuf, len, NETCONN_DONTBLOCK);
    if(recvSize <= 0)
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("client recv from lwip fail, recvSize[%d]\r\n", recvSize);
        if(NULL != recvBuf)
        {
            osFree(recvBuf);
            recvBuf = NULL;
        }
        return osErrorTimeout;
    }

    /* lwip_fcntl 设置为阻塞 */
    /*
    int flags = 0;
    flags = lwip_fcntl(fd, F_GETFL, 0);
    lwip_fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    */

    DEMO_SOCKET_CLIENT_DBG_PRINTF("client recv from lwip ok, recvSize[%d]\r\n", recvSize);
    dbg_hex_data("DEMO_SocketClientDataRecvFromLwip", recvBuf, recvSize);
    if(NULL != recvBuf)
    {
        osFree(recvBuf);
        recvBuf = NULL;
    }

    return osOK;
}

static void DEMO_SocketClientTaskProcess(DEMO_SocketClientTaskMessage msg)
{
    int fd = msg.fd;
    uint32_t event = msg.event;
    uint32_t dataLen = msg.dataLen;

    DEMO_SOCKET_CLIENT_DBG_PRINTF("DEMO_SocketClientTaskProcess:fd[%d], event[%d], dataLen[%d]\r\n", fd, event, dataLen);
    switch(event)
    {
        case SOCKET_TCP_EVENT_ACCEPT:  /* 作为 TCP 服务器, 已接收到 TCP 客户端的连接 */
            break;

        case SOCKET_TCP_EVENT_SENT:  /* TCP 时, 已发送数据 */
            break;

        case SOCKET_TCP_EVENT_RECV:  /* 接收到的数据 */
            if(DEMO_SOCKET_CLIENT_TCP == g_ClientParam.protoType)
            {
                if(0 == dataLen) /* 收到对端的 TCP 关闭请求 */
                {
                    DEMO_SOCKET_CLIENT_ERR_PRINTF("DEMO_SocketClient tcp client recv close, fd[%d]\r\n", fd);
                    DEMO_SocketClientClose(&fd);
                }
                else /* 收到 TCP 数据 */
                {
                    DEMO_SocketClientDataRecvFromLwip(fd, dataLen);
                }
            }
            else if(DEMO_SOCKET_CLIENT_UDP == g_ClientParam.protoType) /* 收到 UDP 数据 */
            {
                DEMO_SocketClientDataRecvFromLwip(fd, dataLen);
            }
            break;

        case SOCKET_TCP_EVENT_CONNECTED:  /* 作为 TCP 客户端, 连接成功 */
            DEMO_SOCKET_CLIENT_ERR_PRINTF("DEMO_SocketClient tcp client connect ok, fd[%d]\r\n", fd);
            break;

        case SOCKET_TCP_EVENT_POLL:
            break;

        case SOCKET_TCP_EVENT_ERR:  /* TCP 错误提示, 如: 收到对端 RST */
        case SOCKET_NETIF_EVENT_DOWN: /* netif down (如: PDP 去激活了) */
            DEMO_SOCKET_CLIENT_ERR_PRINTF("DEMO_SocketClient tcp client recv rst, fd[%d]\r\n", fd);
            DEMO_SocketClientClose(&fd);
            break;

        default:
            DEMO_SOCKET_CLIENT_ERR_PRINTF("lwipEventCallback: client event out of range [%d]\r\n", event);
            break;
    }

    return;
}

static void DEMO_SocketClientTaskEntry(void *param)
{
    while(1)
    {
        DEMO_SocketClientTaskMessage msg = {0};
        if(osMessageQueueGet(g_ClientTaskMQ, &msg, 0, osWaitForever) == osOK)
        {
            DEMO_SOCKET_CLIENT_DBG_PRINTF("DEMO_SocketClientTaskEntry, msgFd [%d]\r\n", msg.fd);
            DEMO_SocketClientTaskProcess(msg);
        }
        else
        {
            continue;
        }
    }
    return;
}

static void DEMO_SocketClientParamSet(char **argv)
{
    memset(&g_ClientParam, 0, sizeof(DEMO_SocketClientParam));
    g_ClientParam.startFlag = DEMO_SOCKET_CLIENT_START;
    if(!strcmp((argv[2]), "tcp"))
    {
        g_ClientParam.protoType = DEMO_SOCKET_CLIENT_TCP;
    }
    else if(!strcmp((argv[2]), "udp"))
    {
        g_ClientParam.protoType = DEMO_SOCKET_CLIENT_UDP;
    }

    strncpy(g_ClientParam.host, argv[3], sizeof(g_ClientParam.host) - 1);
    g_ClientParam.host[sizeof(g_ClientParam.host) - 1] = '\0';
    g_ClientParam.remotePort = atoi(argv[4]);
    DEMO_SOCKET_CLIENT_DBG_PRINTF("client param set, protoType[%s], host[%s], remotePort[%d]\r\n", \
                                argv[1], g_ClientParam.host, g_ClientParam.remotePort);


    return;
}

static void DEMO_SocketClientCreate(void)
{
    int32_t ret = -1;
    int fd = -1;
    uint8_t domainType = 0; /* socket domain 采用数据包还是流 */

    /* ipaddr_aton 函数功能:
      1. 返回值如果为非 0,  说明第一个参数 host 是 IP 地址, 无需进行 DNS 解析. 此时可将第一个参数 host (字符串形式的 IP 地址)转换得到第二个参数(十六进制形式的 IP 地址).
      2. 返回值如果为 0, 说明第一个参数 host 是域名地址, 需要进行 DNS 解析.*/
    if(!ipaddr_aton(g_ClientParam.host, &g_ClientParam.remoteIp))
    {
        struct hostent *hostAddr = NULL; /* 域名解析得到的远端主机地址信息 */
        if((hostAddr = gethostbyname(g_ClientParam.host)) == NULL) /* 域名解析失败 */
        {
            DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client gethostbyname fail\r\n");
            return;
        }
        memcpy(&g_ClientParam.remoteIp, hostAddr->h_addr, hostAddr->h_length);
    }

    if(DEMO_SOCKET_CLIENT_TCP == g_ClientParam.protoType)
    {
        domainType = SOCK_STREAM;
    }
    else if(DEMO_SOCKET_CLIENT_UDP == g_ClientParam.protoType)
    {
        domainType = SOCK_DGRAM;
    }

    /* 创建套接字 socket */
    if (IP_IS_V4(&g_ClientParam.remoteIp)) /* IPv4 */
    {
        fd = lwip_socket(AF_INET, domainType, IPPROTO_IP);
    }
    else /* IPv6 */
    {
        fd = lwip_socket(AF_INET6, domainType, IPPROTO_IP);
    }
    if(fd < 0)
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("DEMO_SocketClient create fail [%d]\r\n",fd);
        return;
    }
    g_ClientParam.fd = fd;
    DEMO_SOCKET_CLIENT_DBG_PRINTF("DEMO_SocketClient create ok, fd[%d]\r\n", fd);

    /* 注册 LWIP 事件回调函数 */
    if(0 != lwip_socket_register_callback(g_ClientParam.fd, DEMO_SocketClientLwipEventCallback, NULL))
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("DEMO_SocketClient register callback fail\r\n");
        lwip_close(g_ClientParam.fd); /* 关闭 socket */
        g_ClientParam.fd = -1;
        return;
    }

    /* 使用 lwip_fcntl 设置 connect、send、recv 为非阻塞. 如果不设置, 默认为阻塞(connect、send、recv 都默认为阻塞).
    1. 如果没有使用 lwip_fcntl 设置非阻塞, connect 将使用默认的阻塞方式.
    2. 如果使用 lwip_fcntl 设置了非阻塞, 对 send 和 recv 也会生效. 此时在 send 和 recv 时即便将 flags 设置为阻塞(flags 填 0 即可), 也会以非阻塞的方式进行 send 和 recv.
    3. 如果想在 send 和 recv 时使用阻塞, 不能使用 lwip_fcntl 设置非阻塞, 如果使用 lwip_fcntl 设置为了非阻塞, 需要使用 lwip_fcntl 设置回阻塞, 并且在 send 和 recv 时将 flags 设置为阻塞(flags 填 0 即可) */
    int flags = 0;
    flags = lwip_fcntl(g_ClientParam.fd, F_GETFL, 0); /* 先获取当前标志, 再追加非阻塞标志 */
    lwip_fcntl(g_ClientParam.fd, F_SETFL, flags | O_NONBLOCK);

    /* lwip_fcntl 设置为阻塞 */
    /*
    int flags = 0;
    flags = lwip_fcntl(g_ClientParam.fd, F_GETFL, 0);
    lwip_fcntl(g_ClientParam.fd, F_SETFL, flags & ~O_NONBLOCK);
    */

    /* 建立连接 */
    if (IP_IS_V4(&g_ClientParam.remoteIp)) /* IPV4 */
    {
        struct sockaddr_in remoteSockAddr;
        memset(&remoteSockAddr, 0, sizeof(struct sockaddr_in));

        /* 用于在 connect 时将远端IP地址和远端端口号赋给 TCP/UDP 的PCB */
        remoteSockAddr.sin_family = AF_INET;
        memcpy(&(remoteSockAddr.sin_addr),ip_2_ip4(&(g_ClientParam.remoteIp)),sizeof(struct in_addr));
        remoteSockAddr.sin_port = htons(g_ClientParam.remotePort);

        DEMO_SOCKET_CLIENT_DBG_PRINTF("fd=[%d], connect port[%d], ip[%s]\r\n", g_ClientParam.fd, \
                    ntohs(remoteSockAddr.sin_port), inet_ntoa(remoteSockAddr.sin_addr.s_addr));
        ret = lwip_connect(g_ClientParam.fd, (struct sockaddr *)&remoteSockAddr, sizeof(struct sockaddr));
    }
    else /* IPV6 */
    {
        struct sockaddr_in6 remoteSockAddr6;
        memset(&remoteSockAddr6, 0, sizeof(struct sockaddr_in6));

        /* 用于在 connect 时将远端IP地址和远端端口号赋给 TCP/UDP 的PCB */
        remoteSockAddr6.sin6_family = AF_INET6;
        memcpy(&(remoteSockAddr6.sin6_addr),ip_2_ip6(&(g_ClientParam.remoteIp)),sizeof(struct in6_addr));
        remoteSockAddr6.sin6_port = htons(g_ClientParam.remotePort);

        ret = lwip_connect(g_ClientParam.fd, (struct sockaddr *)&remoteSockAddr6, sizeof(struct sockaddr));
    }

    if(ret < 0)
    {
         /* TCP 非阻塞进行 connect 时, 会立刻返回 -1, 不影响连接过程, 连接结果通过 LWIP 回调上报.
           如果没有设置 TCP connect 为非阻塞, 当 ret 值返回小于 0 时, 此处不用判断是否 UDP 连接, 即 TCP 和 UDP 都要关闭连接*/
        if(DEMO_SOCKET_CLIENT_UDP == g_ClientParam.protoType)
        {
            DEMO_SOCKET_CLIENT_ERR_PRINTF("connect fail [%d]\r\n", ret);
            lwip_socket_unregister_callback(g_ClientParam.fd); /* 注销 LWIP 事件回调 */
            lwip_close(g_ClientParam.fd); /* 关闭 TCP/IP 连接 */
            g_ClientParam.fd = -1;
            return;
        }
    }
    DEMO_SOCKET_CLIENT_DBG_PRINTF("DEMO_SocketClient connect ok, fd[%d]\r\n", fd);

    return;
}

static int32_t DEMO_SocketClientTaskStart(void)
{
    osThreadAttr_t attr = {"demoSockClient", osThreadDetached, NULL, 0U, NULL, DEMO_SOCKET_CLIENT_TASK_STACK_SIZE,
                            DEMO_SOCKET_CLIENT_TASK_PRIO, 0U, 0U};
    osMessageQueueAttr_t msgQueueAttr = {"sock_client_msg", 0U, NULL, 0U, NULL, 0U};

    if(NULL == g_ClientTaskMQ)
    {
        g_ClientTaskMQ = osMessageQueueNew(DEMO_SOCKET_CLIENT_TASK_MAX_QUEUE_NUM, sizeof(DEMO_SocketClientTaskMessage), &msgQueueAttr);
        if(NULL == g_ClientTaskMQ)
        {
            DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client queue create fail\r\n");
            return osError;
        }
    }

    if(NULL == g_ClientThreadId)
    {
        g_ClientThreadId = osThreadNew(DEMO_SocketClientTaskEntry, NULL, &attr);
        if(NULL == g_ClientThreadId)
        {
            DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client thread create fail\r\n");
            if(g_ClientTaskMQ != NULL)
            {
                osMessageQueueDelete(g_ClientTaskMQ);
                g_ClientTaskMQ = NULL;
            }
            return osErrorTimeout;
        }
    }

    return osOK;
}

static void DEMO_SocketClientTaskStop(void)
{
   if(g_ClientThreadId != NULL)
   {
       osThreadTerminate(g_ClientThreadId);
       g_ClientThreadId = NULL;
   }

   if(g_ClientTaskMQ != NULL)
   {
       osMessageQueueDelete(g_ClientTaskMQ);
       g_ClientTaskMQ = NULL;
   }

    return;
}

static void DEMO_SocketClientDataSend(void)
{
    int ret = 0;
    char sendBuf[DEMO_SOCKET_CLIENT_SEND_LEN] = {0};

    if(DEMO_SOCKET_CLIENT_STOP == g_ClientParam.startFlag)
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client not start, send fail\r\n");
        return;
    }

    if(-1 == g_ClientParam.fd)
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client not create, send fail\r\n");
        return;
    }

    for(int i = 0; i < sizeof(sendBuf); i++)
    {
        sendBuf[i] = i + 1;
    }

    /* MSG_DONTWAIT 表示以非阻塞方式发送.
      如果想以阻塞的方式发送, 需要使用 lwip_fcntl 设置为阻塞, 同时将 lwip_send 函数的第四个参数 flags 设置为 0*/
    ret = lwip_send(g_ClientParam.fd, sendBuf, sizeof(sendBuf), MSG_DONTWAIT);
    if (ret > 0)
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("SendtoLwip ok, sendSize[%d]\r\n", ret);
    }
    else
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("SendtoLwip fail, sendSize[%d]\r\n", ret);
    }

    /* lwip_fcntl 设置为阻塞 */
    /*
    int flags = 0;
    flags = lwip_fcntl(g_ClientParam.fd, F_GETFL, 0);
    lwip_fcntl(g_ClientParam.fd, F_SETFL, flags & ~O_NONBLOCK);
    */

    return;
}

static void DEMO_SocketClientEntry(char argc, char **argv)
{
    int8_t option = 0;

    if(1 == argc)
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client param count err\r\n");
        DEMO_SocketClientHelp();
        return;
    }

    option = DEMO_SocketClientOptionGet(argv);
    if((option < 0) || (option >= DEMO_SOCKET_CLIENT_OPTION_MAX))
    {
        DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client type out of range, please input again. you can see help\r\n");
        return;
    }

    switch(option)
    {
        case DEMO_SOCKET_CLIENT_OPTION_HELP:
            DEMO_SocketClientHelp(); /* demo_socket_client -h */
            break;

        case DEMO_SOCKET_CLIENT_OPTION_CREATE:
            if(5 != argc)
            {
                DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client create count err\r\n");
                DEMO_SocketClientHelp();
                return;
            }
            if(DEMO_SOCKET_CLIENT_START == g_ClientParam.startFlag)
            {
                DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client is running, please stop first\r\n");
                return;
            }
            DEMO_SocketClientParamSet(argv);
            if(DEMO_SocketClientTaskStart())
            {
                DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client task start fail\r\n");
                return;
            }
            DEMO_SocketClientCreate();
            break;

        case DEMO_SOCKET_CLIENT_OPTION_SEND:
            if(2 != argc)
            {
                DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client send count err\r\n");
                DEMO_SocketClientHelp();
                return;
            }
            DEMO_SocketClientDataSend();
            break;

        case DEMO_SOCKET_CLIENT_OPTION_STOP:
            if(2 != argc)
            {
                DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client stop count err\r\n");
                DEMO_SocketClientHelp();
                return;
            }
            g_ClientParam.startFlag = DEMO_SOCKET_CLIENT_STOP;
            DEMO_SocketClientClose(&g_ClientParam.fd);
            memset(&g_ClientParam, 0, sizeof(DEMO_SocketClientParam));
            g_ClientParam.fd = -1;
            DEMO_SocketClientTaskStop();
            break;

        default:
            DEMO_SOCKET_CLIENT_ERR_PRINTF("demo socket client paramer error, please input again. you can see help\r\n");
            break;
    }

    return;
}

/* 注册SHELL命令 */
/** example:
1. demo_socket_client create tcp xxx.xxx.xxx.xxx   xxx    创建 TCP 客户端连接
2. demo_socket_client create udp xxx.xxx.xxx.xxx   xxx   创建 UDP 客户端连接
3. demo_socket_client send    发送数据
4. demo_socket_client stop    关闭 TCP 客户端连接
*/
#include "nr_micro_shell.h"
NR_SHELL_CMD_EXPORT(demo_socket_client,  DEMO_SocketClientEntry);

