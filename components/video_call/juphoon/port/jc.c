#include "stddef.h"
#include <stdint.h>
#include "os.h"

#include "cmsis_os2.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "jc.h"
#ifdef USE_SVC
#include "svc_settings.h"
#endif
#include "video_call.h"


// UDP: 上行和下行的语音 和 视频
// TCP：控制报文加 下行的视频，考虑到IOT 设备缓存小，服务器事先缓冲排序，按照TCP下发

//#define JC_DEBUG_PRINTF
#ifdef JC_DEBUG_PRINTF
    #define jc_printf osPrintf
#else
    #define jc_printf(...)
#endif

struct net_addr* jc_freeaddr(struct net_addr* addr, unsigned num)
{
    for (; addr && num > 0; --num)
    {
        struct net_addr *a = (struct net_addr *)addr;
        addr = addr->next;
        jc_free(a, sizeof(struct net_addr));
    }

    return addr;
}

const char* jc_addr2str(const struct net_addr* addr)
{
    if (NULL != addr)
    {
        if ((addr->sa.sa_family) == AF_INET)
        {
            return inet_ntoa(((struct sockaddr_in *)&(addr->sa))->sin_addr);
        }
        else
        {
            return inet6_ntoa(((struct sockaddr_in6 *)(&addr->sa))->sin6_addr);
        }
    }
    else
    {
        return NULL;
    }
}

static ip_addr_t dns_ip_addr;
static osSemaphoreId_t dns_sem = NULL;
static void _dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
    if(ipaddr)
    {
        memcpy(&dns_ip_addr, ipaddr, sizeof(ip_addr_t));
    }

    if(dns_sem)  
    {
        osSemaphoreRelease(dns_sem);
    }   
}

struct net_addr* jc_getaddrs(const char* host, unsigned short dft_port, unsigned enables)
{
    struct net_addr *addr = NULL;
    struct addrinfo hint;
    char   port[10] = { 0 };

    jc_printf("before host: %s, dft_port: %d enables:%d\r\n", host, dft_port, enables);

    char url[64] = {0};
    char urlIp[32] = {0};
    char urlPort[8] = {0};
    memcpy(url, host, strlen(host));

#if 1
    if(enables == 7)
    {
        uint8_t len = strlen(url);
        for(uint8_t i = 0; i < len; i++)
        {
            if(url[i] == ':')
            {
                memcpy(urlIp, url, i);
                memcpy(urlPort, &url[i+1], len -i);
            }
        }
        dft_port = atoi(urlPort);
    }
    else
    {
        memcpy(urlIp, url, strlen(url));
    }

    char *tmpHost = urlIp;

    jc_printf("%s  host: %s , dft_port: %d\r\n", __FUNCTION__, tmpHost, dft_port);
    itoa(dft_port, port, 10);
    memset(&hint, 0, sizeof(struct addrinfo));
#endif

    ip_addr_t dns_ipaddr;
    err_t dns_ret = -1;

    if(!dns_sem)
    {
        dns_sem = osMalloc(sizeof(struct osSemaphore));
        osSemaphoreInit(dns_sem,  0, 1, OS_IPC_FLAG_FIFO);
    }

    while(1)
    {
        if(osSemaphoreAcquire(dns_sem, osNoWait) != osOK)
            break;  
    }

    dns_ret = dns_gethostbyname(tmpHost, &dns_ipaddr, _dns_found, NULL);
 
    if(dns_ret == ERR_OK)
    {
        memcpy(&dns_ip_addr, &dns_ipaddr, sizeof(ip_addr_t)); 
    }
    else if(dns_ret == ERR_INPROGRESS)
    {
        osSemaphoreAcquire(dns_sem, osTickFromMillisecond(20000));
    }

    osPrintf("dns result: %x \r\n", dns_ip_addr.u_addr.ip4.addr);
    addr = jc_malloc(sizeof(struct net_addr));
    OS_ASSERT(NULL != addr);
    addr->next = NULL;

   //ipaddr_aton(tmpHost, &ipAddr);

    struct sockaddr_in *tmpAddr = (struct sockaddr_in *)(&(addr->sa));
    tmpAddr->sin_family = AF_INET;
    tmpAddr->sin_port = htons(dft_port);
    tmpAddr->sin_addr.s_addr = dns_ip_addr.u_addr.ip4.addr;

    return addr;
}

struct net_sock* jc_connecthost(struct net_addr* addr)
{
    int ret = -1;
    int fd = -1;

    /* 创建套接字 socket */
    if ((addr->sa.sa_family) == AF_INET)
    {
        fd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    }
    else
    {
        fd = lwip_socket(AF_INET6, SOCK_STREAM, IPPROTO_IP);
    }

    if(fd < 0)
    {
        jc_printf("jc_connecthost socket creat fail [%d]\r\n",fd);
        goto connecthost_exit;
    }

    /* 设置套接字属性(超时时间) */
    struct timeval optVal;
    optVal.tv_sec   = 2;
    optVal.tv_usec  = 0;

    /* 设置发送的超时时间 */
    if (lwip_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &optVal, sizeof(optVal)) != 0)
    {
        jc_printf("jc_connecthost set socket option send timeout fail\r\n");
        goto connecthost_exit;
    }

    /* 设置接收的超时时间 */
    if (lwip_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &optVal, sizeof(optVal)) != 0)
    {
        jc_printf("jc_connecthost set socket option recv timeout fail\r\n");
        goto connecthost_exit;
    }

    /*设置关闭套接字模式*/
    struct linger soLinger;
    soLinger.l_linger = 0;
    soLinger.l_onoff = 1;
    if (lwip_setsockopt(fd, SOL_SOCKET, SO_LINGER, &soLinger, sizeof(soLinger)) != 0)
    {
        jc_printf("jc_connecthost set socket option close mode fail\r\n");
        goto connecthost_exit;
    }

    /* 绑定套接字 bind */
    if ((addr->sa.sa_family) == AF_INET) /* IPV4 */
    {
        struct sockaddr_in localAddr;
        memset(&localAddr, 0, sizeof(struct sockaddr_in));

        /* 本端地址信息, 用于绑定 bind 套接字 */
        localAddr.sin_family = AF_INET;
        if(lwip_bind(fd, (struct sockaddr *)&localAddr, sizeof(struct sockaddr)) < 0)
        {
            jc_printf("jc_connecthost socket bind4 fail\r\n");
            goto connecthost_exit;
        }
    }
    else
    {
        struct sockaddr_in6 localAddr6;
        memset(&localAddr6, 0, sizeof(struct sockaddr_in6));

        /* 本端地址信息, 用于绑定 bind 套接字 */
        localAddr6.sin6_family = AF_INET6;

        if(lwip_bind(fd, (struct sockaddr *)&localAddr6, sizeof(struct sockaddr)) < 0)
        {
            jc_printf("jc_connecthost socket bind6 fail\r\n");
            goto connecthost_exit;
        }
    }

    /* 连接 connect */
    /* 设置 connect 为非阻塞, TCP 时有效 */
    // flags = lwip_fcntl(fd, F_GETFL, 0);
    // lwip_fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    if ((addr->sa.sa_family) == AF_INET) /* IPV4 */
    {
        struct sockaddr_in remoteSockAddr;
        memset(&remoteSockAddr, 0, sizeof(struct sockaddr_in));

        /* 用于在 connect 时将远端IP地址和远端端口号赋给 TCP/UDP 的PCB */
        remoteSockAddr.sin_family = AF_INET;
        memcpy(&(remoteSockAddr.sin_addr), &(((struct sockaddr_in *)(&addr->sa))->sin_addr),sizeof(struct in_addr));
        remoteSockAddr.sin_port = (((struct sockaddr_in *)(&addr->sa))->sin_port);

        jc_printf("%s ip: %x, port: %d fd: %d\r\n",__FUNCTION__, remoteSockAddr.sin_addr.s_addr, remoteSockAddr.sin_port, (int)fd);
        ret = lwip_connect(fd, (struct sockaddr *)&remoteSockAddr, sizeof(struct sockaddr));
        if(ret < 0)
        {
            jc_printf("IPV4 lwip_connect failed %d \r\n", ret);
            goto connecthost_exit;
        }
    }
    else /* IPV6 */
    {
        struct sockaddr_in6 remoteSockAddr6;
        memset(&remoteSockAddr6, 0, sizeof(struct sockaddr_in6));

        /* 用于在 connect 时将远端IP地址和远端端口号赋给 TCP/UDP 的PCB */
        remoteSockAddr6.sin6_family = AF_INET6;
        memcpy(&(remoteSockAddr6.sin6_addr),&(((struct sockaddr_in6 *)(&addr->sa))->sin6_addr),sizeof(struct in6_addr));
        remoteSockAddr6.sin6_port = ((struct sockaddr_in6 *)(&addr->sa))->sin6_port;

        ret = lwip_connect(fd, (struct sockaddr *)&remoteSockAddr6, sizeof(struct sockaddr));
        if(ret < 0)
        {
            jc_printf("IPV6 lwip_connect failed %d \r\n", ret);
            goto connecthost_exit;
        }
    }

    return (struct net_sock*)fd;

connecthost_exit:
    lwip_close(fd);
    fd = -1;

    return (struct net_sock*)fd;
}

struct net_sock* jc_dgramsocket(struct net_addr* addr)
{
    int fd = -1;

    /* 创建套接字 socket */
    if ((addr->sa.sa_family) == AF_INET)
    {
        fd = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        jc_printf("%s fd: %d\r\n", __FUNCTION__, fd);
    }
    else
    {
        fd = lwip_socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
    }

    if(fd < 0)
    {
        jc_printf("jc_dgramsocket socket creat fail [%d]\r\n",fd);
        goto dgramsocket_exit;
    }

    /* 设置套接字属性(超时时间) */
    struct timeval optVal;
    optVal.tv_sec   = 2;
    optVal.tv_usec  = 0;

    /* 设置发送的超时时间 */
    if (lwip_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &optVal, sizeof(optVal)) != 0)
    {
        jc_printf("jc_dgramsocket set socket option send timeout fail\r\n");
        goto dgramsocket_exit;
    }

    /* 设置接收的超时时间 */
    if (lwip_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &optVal, sizeof(optVal)) != 0)
    {
        jc_printf("jc_dgramsocket set socket option recv timeout fail\r\n");
        goto dgramsocket_exit;
    }

    /* 绑定套接字 bind */
    if ((addr->sa.sa_family) == AF_INET) /* IPV4 */
    {
        struct sockaddr_in localAddr;
        memset(&localAddr, 0, sizeof(struct sockaddr_in));

        /* 本端地址信息, 用于绑定 bind 套接字 */
        localAddr.sin_family = AF_INET;

        if(lwip_bind(fd, (struct sockaddr *)&localAddr, sizeof(struct sockaddr)) < 0)
        {
            jc_printf("jc_dgramsocket socket bind4 fail\r\n");
            goto dgramsocket_exit;
        }
    }
    else
    {
        struct sockaddr_in6 localAddr6;
        memset(&localAddr6, 0, sizeof(struct sockaddr_in6));

        /* 本端地址信息, 用于绑定 bind 套接字 */
        localAddr6.sin6_family = AF_INET6;

        if(lwip_bind(fd, (struct sockaddr *)&localAddr6, sizeof(struct sockaddr)) < 0)
        {
            jc_printf("jc_dgramsocket socket bind6 fail\r\n");
            goto dgramsocket_exit;
        }
    }

    return (struct net_sock *)fd;

dgramsocket_exit:
    lwip_close(fd);
    fd = -1;

    return (struct net_sock *)fd;
}

int jc_sendto(struct net_sock* sockfd, void* data, int len, struct net_addr* addr)
{
    int sendSize = -1;   /* sendto 发送完后返回值表示的实际发送的数据长度 */
    socklen_t tolen = 0;
    struct sockaddr *to = NULL;

    if (addr)
    {
        to = &addr->sa;
        if ((addr->sa.sa_family) == AF_INET)
        {
            tolen = sizeof(struct sockaddr_in);
        }
        else
        {
            tolen = sizeof(struct sockaddr_in6);
        }
    }
    

    sendSize = lwip_sendto((int)sockfd, data, len, 8, to, tolen);
    if (sendSize < 0)
    {
        jc_printf("data send fail [%d]\r\n", sendSize);
    }
    else
    {
        jc_printf("data send ok [%d], len: %d\r\n", sendSize, len);
    }
    

    return sendSize;
}

struct net_sock* jc_shutdown(struct net_sock* sockfd, struct net_addr* addr)
{
    // int ret;
    // struct net_addr* nextAddr = NULL;
    // if (addr)
    // {
    //     nextAddr = addr->next;
    // }

    lwip_shutdown((int)sockfd, SHUT_RDWR);
    jc_printf("shutdown ret\r\n");

    return sockfd;
}

void jc_closesocket(struct net_sock* sockfd)
{
    int ret = 0;

    if((int)sockfd >= 0)
    {
        ret = lwip_close((int)sockfd);
    }

    if (ret < 0)
    {
        jc_printf("jc_closesocket fail\r\n");
        return;
    }
    jc_printf("jc_closesocket ok\r\n");

    return;
}

int jc_timedrecv(unsigned ms, struct net_sock* tcpfd, struct net_sock* udpfd)
{
    int maxfd;
    int readStatus = 0;
    int ret;
    struct timeval tv;
    fd_set fs_r;

    if (NULL == tcpfd)
    {
        jc_printf("tcpfd is NULL\r\n");
        return -1;
    }

    FD_ZERO(&fs_r);
    FD_SET((int)tcpfd, &fs_r);
    maxfd = (int)tcpfd;

    if (NULL != udpfd)
    {
        FD_SET((int)udpfd, &fs_r);
        maxfd = ((int)tcpfd > (int)udpfd) ? (int)tcpfd : (int)udpfd;
    }

    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms - (tv.tv_sec * 1000)) * 1000;
    ret = lwip_select(maxfd + 1, &fs_r, NULL, NULL, &tv);

    readStatus = 0;
    if (ret > 0)
    {
        if (FD_ISSET((int)tcpfd, &fs_r))
        {
            readStatus |= (1<<0);
            jc_printf("%s tcpfd recv socket data tcpfd:%d\r\n", __FUNCTION__, (int)tcpfd);
        }

        if (FD_ISSET((int)udpfd, &fs_r))
        {
            readStatus |= (1<<1);
            jc_printf("udpfd ready\r\n");
        }
    }
    else if (ret == 0)
    {
        readStatus = 0;
        jc_printf("select timeout\r\n");
    }
    else
    {
        readStatus = -1;
        jc_printf("select error\r\n");
    }

    return readStatus;
}

int jc_recv(struct net_sock* sockfd, void* buf, int buflen)
{
    int ret = -1;

    ret = lwip_recv((int)sockfd, buf, buflen, 8);
   // osPrintf("%d, fd : %d, ret : %d \r\n", (uint32_t)osTickGet(), (int)sockfd, ret);
    if (ret < 0)
        ret = 0;

    return ret;
}


void jc_log(char *fmt, ...)
{
    va_list args;
    size_t length;
    char rt_log_buf[OS_CONSOLEBUF_SIZE];
    osOutputFunc_t output;

    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = osVsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > OS_CONSOLEBUF_SIZE - 1)
        length = OS_CONSOLEBUF_SIZE - 1;

    output = SHELL_GetOutputFunc();
    if (output == OS_NULL)
    {
        osConsoleOutput(rt_log_buf);
    }
    else
    {
        output(rt_log_buf);
    }
    va_end(args);
}

//返回芯片名称
const char*jc_chip_uid(void)
{
    return "LM620";
}

//返回当前CPU空闲百分比,不支持返回0
unsigned jc_cpu_idle(void)
{
    return 0;
}

//返回当前内存剩余KB,不支持返回0
unsigned jc_mem_size(void)
{
    return 0;
}

//返回递增的时间截,毫秒值
unsigned jc_time(void)
{
    return osMsFromTick(osTickGet());
}

//动态分配堆地址
void* jc_malloc(unsigned size)
{
   return osMallocAlign(size, OS_CACHE_LINE_SZ);
}

//释放动态堆地址
void jc_free(void* ptr, unsigned size)
{
    OS_ASSERT(ptr);
    osFree(ptr);
}

unsigned JRTC_TASK_STACK_KB = 16; /*< 上述创建的线程栈大小,与平台相关 */
void* jc_task_create(const char* name, void (*fn)(void*), void *arg)
{
    osThreadAttr_t attr = {name, osThreadDetached, NULL, 0U, NULL, JRTC_TASK_STACK_KB * 1024, osPriorityISR - 12, 0U, 0U};
    osThreadId_t task_handle = osThreadNew(fn, arg, &attr);
    return task_handle;
}

/** 等待线程结束
 * jrtc_close() 内部使用该函数回收线程
 */
void jc_task_join(void *task)
{
    // os_task_msleep(3000);
    // os_task_destroy(task);
}

void *jc_sem_alloc(void)
{
    void *sem = osMalloc(sizeof(struct osSemaphore));
    osSemaphoreInit(sem,  0, 1, OS_IPC_FLAG_FIFO);
    return sem;
}

void jc_sem_post(void *sem)
{
    if(sem)
        osSemaphoreRelease(sem);
}

void jc_sem_wait(void *sem, int ms)
{
    if(sem)
        osSemaphoreAcquire(sem, osTickFromMillisecond(ms));
}

void jc_sem_free(void *sem)
{
    if(sem)
    {
        osSemaphoreDetach(sem);
        osFree(sem);
    }
}

