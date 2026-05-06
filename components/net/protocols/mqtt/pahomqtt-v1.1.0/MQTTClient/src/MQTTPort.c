/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        MQTTOneOS.c
 *
 * @brief       socket port file for mqtt
 *
 * @details
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 * 2020-12-29   OneOS Team      modify tls netowrk interface and add new mqtt net interface
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "MQTTPort.h"
#include "app_at_pub.h"

#define MQ_NET_TAG             "[MQ_NET]"

#define MQTT_TASK_STACK_SIZE   (4*1024)

#define U32_DIFF(a, b) (((a) >= (b)) ? ((a) - (b)) : (((a) + ((b) ^ 0xFFFFFFFF) + 1)))

extern void MQTT_SocketRecvEventCallback(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param);

/**
 ***********************************************************************************************************************
 * @brief           This function start a thread.
 *
 * @param[in]       thread        thread to create.
 * @param[in]       fn            function to run.
 * @param[in]       arg           arguments.
 *
 * @return          Return status.
 * @retval          TRUE          create success.
 * @retval          FALSE         create failed.
 ***********************************************************************************************************************
 */
int ThreadStart(Thread *thread, void (*fn)(void *), void *arg)
{
    osThreadId_t threadId;

    osThreadAttr_t attr = {"MQTTTask", osThreadDetached, NULL, 0U, NULL, MQTT_TASK_STACK_SIZE, osPriorityBelowNormal, 0U, 0U};

    threadId = osThreadNew(fn, arg, &attr);
    if (NULL != threadId)
    {
        *thread = threadId;
        return TRUE;
    }

    return FALSE;
}

void ThreadDelete(Thread thread)
{
    if (NULL != thread)
    {
        osThreadTerminate(thread);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function initialize mutex.
 *
 * @param[in]       mutex        mutex to initialize.
 *
 ***********************************************************************************************************************
 */
void MutexInit(Mutex *mutex)
{
    osMutexInit(mutex, OS_IPC_FLAG_PRIO);
}

/**
 ***********************************************************************************************************************
 * @brief           This function deinitialize mutex.
 *
 * @param[in]       mutex        mutex to deinitialize.
 *
 ***********************************************************************************************************************
 */
void MutexDeInit(Mutex *mutex)
{
    osMutexRelease(mutex);
}

/**
 ***********************************************************************************************************************
 * @brief           This function take mutex.
 *
 * @param[in]       mutex        mutex to lock.
 *
 * @return          Return status.
 * @retval          TRUE         lock success.
 * @retval          FALSE        lock failed.
 ***********************************************************************************************************************
 */
int MutexLock(Mutex *mutex)
{
    osStatus_t result;

    result = osMutexAcquire(mutex, osWaitForever);
    if (result != osOK)
    {
        OS_ASSERT(0);
        return FALSE;
    }

    return TRUE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function release mutex.
 *
 * @param[in]       mutex        mutex to unlock.
 *
 * @return          Return status.
 * @retval          TRUE         unlock success.
 * @retval          FALSE        unlock failed.
 ***********************************************************************************************************************
 */
int MutexUnlock(Mutex *mutex)
{
    osStatus_t result;

    result = osMutexRelease(mutex);
    if (result != osOK)
    {
        OS_ASSERT(0);
        return FALSE;
    }

    return TRUE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function set timer callback.
 ***********************************************************************************************************************
 */
void TimerCallback(void *parameter)
{
    /*Becareful: do not include any debug printf code.*/
}

/**
 ***********************************************************************************************************************
 * @brief           This function set timer parameter.
 *
 * @param[in]       timer        timer to set.
 * @param[in]       xTicksToWait ticks to wait.
 *
 ***********************************************************************************************************************
 */
void TimerSetTimeOutState(Timer *timer, osTick_t xTicksToWait)
{
    int32_t     status;
    osTick_t    set_ticktowait = xTicksToWait;

    OS_ASSERT(NULL != timer->xTimeOut);

    osTimerStop(timer->xTimeOut);
    status = osTimerStart(timer->xTimeOut, set_ticktowait);
    OS_ASSERT(osOK == status);

    timer->xTicksToWait = xTicksToWait;
    timer->xTicksRecord = osTickGet();
}

/**
 ***********************************************************************************************************************
 * @brief           This function check whether timeout happened.
 *
 * @param[in]       timer        timer to check.
 *
 * @return          Return status.
 * @retval          TRUE         timeout.
 * @retval          FALSE        not timeout or error happened.
 ***********************************************************************************************************************
 */
int TimerCheckForTimeOut(Timer *timer)
{
    osTick_t tick_now;
    osTick_t tick_pre;
    osTick_t tick_diff;

    if (NULL == timer)
        return FALSE;

    if (osTimerIsRunning(timer->xTimeOut)) /*timer is not out*/
    {
        tick_now = osTickGet();
        tick_pre = timer->xTicksRecord;
        tick_diff = U32_DIFF(tick_now, tick_pre);
        timer->xTicksRecord = tick_now;

        if (tick_diff < timer->xTicksToWait)
        {
            timer->xTicksToWait -= tick_diff;
            return FALSE;
        }
        else
        {
            timer->xTicksToWait = 0;
            osTimerStop(timer->xTimeOut);
            return TRUE;
        }
    }
    else /*timer out happen*/
    {
        osTimerStop(timer->xTimeOut);
        timer->xTicksToWait = 0;
        return TRUE;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function set timer in milliseconds.
 *
 * @param[in]       timer        timer to check.
 * @param[in]       timeout_ms   time in milliseconds to set.
 ***********************************************************************************************************************
 */
void TimerCountdownMS(Timer *timer, unsigned int timeout_ms)
{
    osTick_t xTicksToWait;

    if (0 != timeout_ms)
    {
        xTicksToWait = osTickFromMs(timeout_ms); /* convert milliseconds to ticks */
        TimerSetTimeOutState(timer, xTicksToWait);  /* Record the time at which this function was entered. */
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function set timer in seconds.
 *
 * @param[in]       timer        timer to check.
 * @param[in]       timeout      time in seconds to set.
 ***********************************************************************************************************************
 */
void TimerCountdown(Timer *timer, unsigned int timeout)
{
    TimerCountdownMS(timer, timeout * 1000);
}

/**
 ***********************************************************************************************************************
 * @brief           This function returns time left in milliseconds.
 *
 * @param[in]       timer        timer to check.
 *
 * @return          time left.
 * @retval          int          milliseconds.
 ***********************************************************************************************************************
 */
int TimerLeftMS(Timer *timer)
{
    return (timer->xTicksToWait * (1000 / OS_TICK_PER_SECOND));
}

/**
 ***********************************************************************************************************************
 * @brief           This function check whether timeout happened.
 *
 * @param[in]       timer        timer to check.
 *
 * @return          Return status.
 * @retval          TRUE         timeout.
 * @retval          others       not timeout or error happened.
 ***********************************************************************************************************************
 */
char TimerIsExpired(Timer *timer)
{
    return TimerCheckForTimeOut(timer);
}

/**
 ***********************************************************************************************************************
 * @brief           This function initialize timer.
 *
 * @param[in]       timer        timer to check.
 ***********************************************************************************************************************
 */
void TimerInit(Timer *timer, TimerCallbackFunc timerCb)
{
    OS_ASSERT(NULL != timer);
    /*creat timer*/
    timer->xTimeOut = NULL;
    if (NULL == timerCb)
    {
        timerCb = TimerCallback;
    }
    timer->xTimeOut = osTimerNew(timerCb, osTimerOnce, timer, NULL);
    OS_ASSERT(NULL != timer->xTimeOut);

    timer->xTicksToWait = 0;
    timer->xTicksRecord = osTickGet();
}

/**
 ***********************************************************************************************************************
 * @brief           This function stop timer.
 *
 * @param[in]       timer        timer to check.
 ***********************************************************************************************************************
 */
void TimerStop(Timer *timer)
{
    OS_ASSERT(NULL != timer->xTimeOut);

    osTimerStop(timer->xTimeOut);
    timer->xTicksToWait = 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function release timer.
 *
 * @param[in]       timer        timer to check.
 ***********************************************************************************************************************
 */
void TimerRelease(Timer *timer)
{
    /*release timer*/
    if (NULL != timer->xTimeOut)
    {
        osTimerDelete(timer->xTimeOut);
    }

    timer->xTicksToWait = 0;
    timer->xTicksRecord = osTickGet();
    timer->xTimeOut = NULL;
}

#ifdef MQTT_USING_TLS
static uintptr_t ssl_establish(const char *host, uint16_t port, const char *ca_crt, uint32_t ca_crt_len)
{
    char        port_str[6] = {0};
    uintptr_t   tls_session_ptr = (uintptr_t)NULL;

    if (host == NULL || ca_crt == NULL)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"ssl input params are NULL, abort");
        return (uintptr_t)NULL;
    }

    if (!strlen(host) || (strlen(host) < 8))
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"ssl invalid host: '%s'(len=%d), abort", host, (int)strlen(host));
        return (uintptr_t)NULL;
    }

    sprintf(port_str, "%u", port);
    if (0 != mq_tls_network_mbedtls_establish(&tls_session_ptr, host, port_str, ca_crt, NULL, NULL, NULL))
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"ssl establish error!\r\n");
        return (uintptr_t)NULL;
    }

    return (uintptr_t)(tls_session_ptr);
}

static int mqtt_connect_ssl(Network *pNetwork)
{
    uintptr_t handle = 0;
    if (NULL == pNetwork)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"network is null");
        return -1;
    }

    handle = ssl_establish(pNetwork->pHostAddress,
                            pNetwork->port,
                            pNetwork->ca_crt,
                            pNetwork->ca_crt_len + 1);
    if (0 != handle)
    {
        pNetwork->handle = handle;
        /* 注册 LWIP 事件回调函数 */
        pNetwork->fd = ((mq_tls_session_t *)pNetwork->handle)->server_fd.fd;
        lwip_socket_register_callback(((mq_tls_session_t *)pNetwork->handle)->server_fd.fd,
            MQTT_SocketRecvEventCallback, NULL);
        MQTT_PRINT_INFO(MQ_NET_TAG"connect ssl success, fd: %d \r\n", pNetwork->fd);
        return 0;
    }
    else
    {
        /* The space has been freed */
        MQTT_PRINT_ERROR(MQ_NET_TAG"connect ssl error!\r\n");
        return -1;
    }
}

static int mqtt_disconnect_ssl(Network *pNetwork)
{
    if (NULL == pNetwork)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"ssl network is null");
        return -1;
    }

    if ((uintptr_t)(-1) == pNetwork->fd)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"ssl pNetwork->fd = -1");
        return -1;
    }

    lwip_socket_unregister_callback(pNetwork->fd);
    mq_tls_network_mbedtls_close((mq_tls_session_t **)&(pNetwork->handle));
    pNetwork->handle = (uintptr_t)-1; /* must have */
    pNetwork->fd = -1;
    MQTT_PRINT_INFO(MQ_NET_TAG"MbedTLS connection close success.\r\n");

    return 0;
}

#endif // MQTT_USING_TLS

static int mqtt_connect_tcp(Network *pNetwork)
{
    int                 fd = 0;
    int                 retVal = -1;
    struct sockaddr_in  sAddr;
    struct hostent     *host_entry = NULL;
    long                socket_mode = 1; /* non-blocking */

    if (NULL == pNetwork)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"MQTT pNetwork is null\r\n");
        return -1;
    }

    if ((host_entry = gethostbyname(pNetwork->pHostAddress)) == NULL)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"dns parse error!\r\n");
        goto exit;
    }

    memset(&sAddr, 0, sizeof(struct sockaddr_in));
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(pNetwork->port);
    sAddr.sin_addr = *(struct in_addr *)host_entry->h_addr_list[0];

    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"socket create error!\r\n");
        goto exit;
    }

    if (pNetwork->pdpCid != 0)
    {
        if(osOK != APP_AT_BindCid(fd, pNetwork->pdpCid))
        {
            MQTT_PRINT_ERROR(MQ_NET_TAG"mqtt bind netif fail, fd:%d, pdpCid:%u!\r\n", fd, pNetwork->pdpCid);
            closesocket(fd);
            goto exit;
        }
    }

    if ((retVal = connect(fd, (struct sockaddr *)&sAddr, sizeof(sAddr))) < 0)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"socket connect error!\r\n");
        closesocket(fd);
        goto exit;
    }

    /* Set socket non-blocking mode. */
    if ((retVal = ioctlsocket(fd, FIONBIO, &socket_mode)) < 0 )
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"socket ioctlsocket error!\r\n");
        closesocket(fd);
        goto exit;
    }

    /* 注册 LWIP 事件回调函数 */
    pNetwork->fd = fd;
    if(0 != lwip_socket_register_callback(fd, MQTT_SocketRecvEventCallback, NULL))
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"socket register error!\r\n");
        closesocket(fd);
        pNetwork->fd = -1;
        goto exit;
    }
    MQTT_PRINT_INFO(MQ_NET_TAG"socket connect success, fd: %d \r\n", fd);
    pNetwork->handle = (uintptr_t)fd;

exit:
    return retVal;
}

static int mqtt_disconnect_tcp(Network *pNetwork)
{
    if (NULL == pNetwork)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"network is null");
        return -1;
    }

    if ((uintptr_t)(-1) == pNetwork->handle)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"MQTT_Network->handle = -1");
        return -1;
    }

    lwip_socket_unregister_callback(pNetwork->handle);
    closesocket(pNetwork->handle);
    pNetwork->handle = (uintptr_t)(-1);
    pNetwork->fd = -1;
    MQTT_PRINT_INFO(MQ_NET_TAG"TCP connection close success.\r\n");

    return 0;
}

static int MQTT_net_connect(Network *pNetwork)
{
    int ret = 0;
#ifdef MQTT_USING_TLS
    if (NULL != pNetwork->ca_crt)
    {
        ret = mqtt_connect_ssl(pNetwork);
        return ret;
    }
#endif
    if (NULL == pNetwork->ca_crt)
    {
        ret = mqtt_connect_tcp(pNetwork);
    }
    else
    {
        ret = -1;
        MQTT_PRINT_ERROR(MQ_NET_TAG"no method match!");
    }

    return ret;
}

static int MQTT_net_disconnect(Network *pNetwork)
{
    int ret = 0;
#ifdef MQTT_USING_TLS
    if (NULL != pNetwork->ca_crt)
    {
        ret = mqtt_disconnect_ssl(pNetwork);
        return ret;
    }
#endif
    if (NULL == pNetwork->ca_crt)
    {
        ret = mqtt_disconnect_tcp(pNetwork);
    }
    else
    {
        ret = -1;
        MQTT_PRINT_ERROR(MQ_NET_TAG"no method match!");
    }

    return ret;
}

#ifdef MQTT_USING_TLS
static int mqtt_read_ssl(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    osTick_t    xTicksToWait = 0;
    Timer       xTimeOut = {0, 0, NULL};
    int         recvLen = 0;
    int         rc = 0;
    int         xMsToWait = 0;

    if(0 == timeout_ms) /* timeoutMs 为0表示非阻塞 */
    {
        timeout_ms = 10; /* 由于0会导致timer配置失败，以及select接口一直等，所以这里置超时时间为 10 毫秒 */
    }

    xTicksToWait = osTickFromMs(timeout_ms); /* convert milliseconds to ticks */
    xMsToWait = timeout_ms;

    TimerInit(&xTimeOut, TimerCallback);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */

    static int net_status = 0;

    mq_tls_session_t *session_ptr = (mq_tls_session_t *)pNetwork->handle;

    do
    {
        xTicksToWait = xTimeOut.xTicksToWait;
        xMsToWait = xTicksToWait * (1000 / OS_TICK_PER_SECOND);

        mbedtls_ssl_conf_read_timeout(&(session_ptr->config), xMsToWait);
        rc = mq_tls_network_mbedtls_read((mq_tls_session_t *)pNetwork->handle,
                                         (unsigned char *)(buffer + recvLen),
                                         len - recvLen);
        if (rc > 0)
        {
            recvLen += rc;
            net_status = 0;
        }
        else if (rc == 0)
        {
            /* if ret is 0 and net_status is -2, indicate the connection is closed during last call */
            if (net_status == -2)
                recvLen = net_status;
            break;
        }
        else
        {
            if (MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == rc)
            {
            #ifdef MBEDTLS_ERROR_C
                char err_str[33];
                mbedtls_strerror(rc, err_str, sizeof(err_str));
                MQTT_PRINT_ERROR(MQ_NET_TAG"ssl recv error: code = %d, err_str = '%s'", rc, err_str);
            #endif
                net_status = -2; /* connection is closed */
                recvLen = net_status;
                break;
            }
            else if ((MBEDTLS_ERR_SSL_TIMEOUT == rc)
                     || (MBEDTLS_ERR_SSL_CONN_EOF == rc)
                     || (MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED == rc)
                     || (MBEDTLS_ERR_SSL_NON_FATAL == rc))
           {
                /* read already complete */
                /* if call mbedtls_ssl_read again, it will return 0 (means EOF) */
            }
            else
            {
            #ifdef MBEDTLS_ERROR_C
                char err_str[33];
                mbedtls_strerror(rc, err_str, sizeof(err_str));
                MQTT_PRINT_ERROR(MQ_NET_TAG"ssl recv error: code = %d, err_str = '%s'", rc, err_str);
            #endif
                net_status = -1;
                recvLen = net_status;
                break;
            }
        }

    } while ((recvLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));

    TimerRelease(&xTimeOut);

    return recvLen;
}

static int mqtt_write_ssl(Network *pNetwork, unsigned char *buffer, uint32_t len, uint32_t timeout_ms)
{
    osTick_t    xTicksToWait = osTickFromMs(timeout_ms); /* convert milliseconds to ticks */
    Timer       xTimeOut = {0, 0, NULL};
    int         sentLen = 0;
    int         rc = 0;

    TimerInit(&xTimeOut, TimerCallback);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */

    do
    {
        xTicksToWait = xTimeOut.xTicksToWait;
        rc = mq_tls_network_mbedtls_write((mq_tls_session_t *)pNetwork->handle,
                                          (const unsigned char *)(buffer + sentLen),
                                          len - sentLen);
        if (rc > 0)
        {
            sentLen += rc;
        }
        else if (rc < 0)
        {
            if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                MQTT_PRINT_ERROR(MQ_NET_TAG"mbedtls_ssl_write returned -0x%x", -rc);
                sentLen = rc;
                break;
            }
        }
    } while ((sentLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));

    TimerRelease(&xTimeOut);

    return sentLen;
}

#endif // MQTT_USING_TLS

static int mqtt_read_tcp(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    uintptr_t       fd = pNetwork->handle;
    osTick_t        xTicksToWait = osTickFromMs(timeout_ms); /* convert milliseconds to ticks. */
    Timer           xTimeOut = {0, 0, NULL};
    int             recvLen = 0;
    int             ret;
    struct timeval  tv;
    fd_set          sets;

    tv.tv_sec  = 0;
    tv.tv_usec = timeout_ms * 1000;

    TimerInit(&xTimeOut, TimerCallback);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */

    do
    {
        xTicksToWait = xTimeOut.xTicksToWait;
        tv.tv_sec = 0;
        tv.tv_usec = xTicksToWait * (1000 / OS_TICK_PER_SECOND) * 1000;

        FD_ZERO(&sets);
        FD_SET(fd, &sets);

        ret = select(fd + 1, &sets, NULL, NULL, &tv);
        if (ret > 0)
        {
            ret = recv(fd, buffer + recvLen, len - recvLen, 0);
            if (ret > 0)
            {
                recvLen += ret;
            }
            else if (0 == ret)
            {
                MQTT_PRINT_ERROR(MQ_NET_TAG"connection close\r\n");
                recvLen = -1;
                break;
            }
            else
            {
                if (EINTR == errno)
                {
                    MQTT_PRINT_ERROR(MQ_NET_TAG"EINTR be caught");
                    continue;
                }
                // MQTT_PRINT_ERROR(MQ_NET_TAG"recv fail");
                recvLen = -1;
                break;
            }
        }
        else if (0 == ret)
        {
            break;
        }
        else
        {
            MQTT_PRINT_ERROR(MQ_NET_TAG"select-recv fail");
            recvLen = -1;
            break;
        }
    } while ((recvLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));

    TimerRelease(&xTimeOut);

    return recvLen;
}

static int mqtt_read_tcp_nonblock(Network *pNetwork, unsigned char *buffer, int len)
{
    uintptr_t       fd = pNetwork->handle;
    int             recvLen = 0;
    socklen_t       fromLen = sizeof(struct sockaddr);
    struct sockaddr remoteSockAddr;

    memset(&remoteSockAddr, 0, sizeof(struct sockaddr_in));
    if(0 != lwip_socket_addr_get(fd, &remoteSockAddr))
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"data recv get addr fail\r\n");
        return recvLen;
    }

    recvLen = lwip_recvfrom(fd, buffer, len, NETCONN_DONTBLOCK, &remoteSockAddr, &fromLen);

    return recvLen;
}

static int mqtt_write_tcp(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    uintptr_t       fd = pNetwork->handle;
    osTick_t        xTicksToWait = osTickFromMs(timeout_ms); /* convert milliseconds to ticks. */
    Timer           xTimeOut = {0, 0, NULL};
    int             sentLen = 0;
    int             ret;
    struct timeval  tv;
    fd_set          sets;

    ret        = 1;
    tv.tv_sec  = 0;
    tv.tv_usec = timeout_ms * 1000;

    TimerInit(&xTimeOut, TimerCallback);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */

    do
    {
        if (0 != xTicksToWait)
        {
            xTicksToWait = xTimeOut.xTicksToWait;
            tv.tv_sec = 0;
            tv.tv_usec = xTicksToWait * (1000 / OS_TICK_PER_SECOND) * 1000;
            FD_ZERO(&sets);
            FD_SET(fd, &sets);

            tv.tv_sec  = xTicksToWait / 1000;
            tv.tv_usec = (xTicksToWait % 1000) * 1000;

            ret = select(fd + 1, NULL, &sets, NULL, &tv);
            if (ret > 0)
            {
                if (0 == FD_ISSET(fd, &sets))
                {
                    MQTT_PRINT_ERROR(MQ_NET_TAG"Should NOT arrive");
                    /* If timeout in next loop, it will not sent any data */
                    ret = 0;
                    continue;
                }
            }
            else if (0 == ret)
            {
                MQTT_PRINT_ERROR(MQ_NET_TAG"select-write timeout %d", timeout_ms);
                sentLen = -1;
                break;
            }
            else
            {
                if (EINTR == errno)
                {
                    MQTT_PRINT_ERROR(MQ_NET_TAG"EINTR be caught");
                    continue;
                }

                MQTT_PRINT_ERROR(MQ_NET_TAG"select-write fail");
                sentLen = -1;
                break;
            }
        }

        if (ret > 0)
        {
            ret = send(fd, buffer + sentLen, len - sentLen, 0);
            if (ret > 0)
            {
                sentLen += ret;
            }
            else if (0 == ret)
            {
                MQTT_PRINT_ERROR(MQ_NET_TAG"No data be sent");
            }
            else
            {
                if (EINTR == errno)
                {
                    MQTT_PRINT_ERROR(MQ_NET_TAG"EINTR be caught");
                    continue;
                }

                MQTT_PRINT_ERROR(MQ_NET_TAG"send fail");
                sentLen = -1;
                break;
            }
        }
    }while ((sentLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));
    TimerRelease(&xTimeOut);

    return sentLen;
}


static int MQTT_net_read(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    int ret = 0;
#ifdef MQTT_USING_TLS
    if (NULL != pNetwork->ca_crt)
    {
        ret = mqtt_read_ssl(pNetwork, buffer, len, timeout_ms);
        ret = (ret <= 0) ? 0 : ret;
        return ret;
    }
#endif
    if (NULL == pNetwork->ca_crt)
    {
        if (0 == timeout_ms)
        {
            ret = mqtt_read_tcp_nonblock(pNetwork, buffer, len);
            ret = (ret <= 0) ? 0 : ret;
        }
        else
        {
            ret = mqtt_read_tcp(pNetwork, buffer, len, timeout_ms);
            ret = (ret <= 0) ? 0 : ret;
        }
    }
    else
    {
        ret = -1;
        MQTT_PRINT_ERROR(MQ_NET_TAG"no method match!");
    }

    return ret;
}

static int MQTT_net_write(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    int ret = 0;
#ifdef MQTT_USING_TLS
    if (NULL != pNetwork->ca_crt)
    {
        ret = mqtt_write_ssl(pNetwork, buffer, len, timeout_ms);
        return ret;
    }
#endif
    if (NULL == pNetwork->ca_crt)
    {
        ret = mqtt_write_tcp(pNetwork, buffer, len, timeout_ms);
    }
    else
    {
        ret = -1;
        MQTT_PRINT_ERROR(MQ_NET_TAG"no method match!");
    }

    return ret;
}

int MQTTNetworkSetParam(Network *pNetwork, const char *host, uint16_t port, const char *ca_crt)
{
    if (!pNetwork || !host)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"set parameter error! pNetwork=%p, host = %p", pNetwork, host);
        return -1;
    }
    pNetwork->pHostAddress = host;
    pNetwork->port = port;
    pNetwork->ca_crt = ca_crt;

    if (NULL == ca_crt)
    {
        pNetwork->ca_crt_len = 0;
    }
    else
    {
        pNetwork->ca_crt_len = strlen(ca_crt);
    }

    return 0;
}
/**
 ***********************************************************************************************************************
 * @brief           This function initialize mqtt network.
 *
 * @param[in]       pNetwork    struct Network pointer.
 * @param[in]       host        url or ip.
 * @param[in]       port        port.
 * @param[in]       ca_crt      certificate.
 *
 * @return          Return status.
 * @retval          0           success.
 * @retval          -1          failed.
 ***********************************************************************************************************************
 */
int MQTTNetworkInit(Network *pNetwork, const char *host, uint16_t port, const char *ca_crt)
{
    if (!pNetwork || !host)
    {
        MQTT_PRINT_ERROR(MQ_NET_TAG"parameter error! pNetwork=%p, host = %p", pNetwork, host);
        return -1;
    }

    if( MQTTNetworkSetParam(pNetwork, host, port, ca_crt) == 0)
    {
        pNetwork->fd = -1;
        pNetwork->pdpCid = 0;
        pNetwork->handle = (uintptr_t)(-1);
        pNetwork->mqttread = MQTT_net_read;
        pNetwork->mqttwrite = MQTT_net_write;
        pNetwork->disconnect = MQTT_net_disconnect;
        pNetwork->connect = MQTT_net_connect;
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function connect mqtt network.
 *
 * @param[in]       pNetwork    struct Network pointer.
 *
 * @return          Return status.
 * @retval          0           success.
 * @retval          -1          failed.
 ***********************************************************************************************************************
 */
int MQTTNetworkConnect(Network *pNetwork)
{
    if (NULL == pNetwork)
        return -1;

    return MQTT_net_connect(pNetwork);
}

/**
 ***********************************************************************************************************************
 * @brief           This function disconnect mqtt network.
 *
 * @param[in]       pNetwork    struct Network pointer.
 *
 * @return          void
 ***********************************************************************************************************************
 */
void MQTTNetworkDisconnect(Network *pNetwork)
{
    if (NULL == pNetwork)
        return;

    if (NULL != pNetwork->disconnect)
    {
        pNetwork->disconnect(pNetwork);
    }
    else
    {
        MQTT_net_disconnect(pNetwork);
    }
}
