/*
 * Copyright (c) 2017 Simon Goldschmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt
 *
 */

#include <os.h>

#include "arch/sys_arch.h"
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "ip4_nat.h"
#include "net_api.h"

#if 0
/*
 * Initialize the network interface device
 *
 * @return the operation status, ERR_OK on OK, ERR_IF on error
 */
static err_t netif_device_init(struct netif *netif)
{
    struct eth_device *ethif = OS_NULL;

    ethif = (struct eth_device *)netif->state;
    if (ethif != OS_NULL)
    {
        os_device_t *device = OS_NULL;

        /* get device object */
        device = (os_device_t *)ethif;
        if (os_device_open(device) != OS_EOK)
        {
            return ERR_IF;
        }

        /* copy device flags to netif flags */
        netif->flags = ethif->flags;
        netif->mtu   = ETHERNET_MTU;

        /* set output */
        netif->output = etharp_output;

        return ERR_OK;
    }

    return ERR_IF;
}

static os_err_t __tcpip_init_done_callback(os_device_t *device, void *data)
{
    struct eth_device *ethif;
    ip4_addr_t ipaddr, netmask, gw;

    IP4_ADDR(&gw, 0,0,0,0);
    IP4_ADDR(&ipaddr, 0,0,0,0);
    IP4_ADDR(&netmask, 0,0,0,0);

    /* enter critical */
    os_schedule_lock();

    if (device->type == OS_DEVICE_TYPE_NETIF)
    {
        ethif = (struct eth_device *)device;

        /* leave critical */
        os_schedule_unlock();
        LOCK_TCPIP_CORE();

        netif_add(ethif->netif, &ipaddr, &netmask, &gw,
                    ethif, netif_device_init, tcpip_input);

        if (netif_default == OS_NULL)
            netif_set_default(ethif->netif);

#if LWIP_DHCP
        /* set interface up */
        netif_set_up(ethif->netif);
        /* if this interface uses DHCP, start the DHCP client */
        dhcp_start(ethif->netif);
#else
        /* set interface up */
        netif_set_up(ethif->netif);
#endif

        if (ethif->flags & ETHIF_LINK_PHYUP)
        {
            netif_set_link_up(ethif->netif);
        }

        UNLOCK_TCPIP_CORE();
        /* enter critical */
        os_schedule_lock();
    }

    /* leave critical */
    os_schedule_unlock();

    return OS_EOK;
}
#endif

/**
 * LwIP system initialization
 */

void lwip_set_default_netif(struct netif *pnetif)
{
    netif_default = pnetif;
    return;
}

#if 1
extern void lwip_socket_event_init(void);
int lwip_system_init(void)
{
    /* set default netif to NULL */
    netif_default = OS_NULL;

    OS_ASSERT(sizeof(sys_tick_t) == 8);  //  lwip中sys_tick_t默认要8字节, 检查是否宽度一致

#if LWIP_USING_NAT_IP4
    NAT_Ip4Init();
#endif

    lwip_socket_event_init();

    tcpip_init(OS_NULL, OS_NULL);

#ifdef OS_USING_PM
      lwip_sby_init();
#endif

    return 0;
}

#else
/*
 * Initialize the ethernetif layer and set network interface device up
 */
static void tcpip_init_done_callback(void *arg)
{
    osSemaphoreRelease((sys_sem_t)arg);
}

int        lwip_system_init(void)
{
    static bool_t s_init_ok = OS_FALSE;
    osStatus_t         rc        = osOK;
    sys_sem_t        done_sem  = OS_NULL;

    if (s_init_ok)
    {
        //os_kprintf("lwip system already init.\r\n");
        return 0;
    }

    /* set default netif to NULL */
    netif_default = OS_NULL;

    done_sem = osSemaphoreNew(OS_SEM_VALUE_MAX, 0, OS_NULL);
    if (done_sem == OS_NULL)
    {
        LWIP_ASSERT("Failed to create semaphore", 0);

        return -1;
    }

#if LWIP_USING_NAT_IP4
    NAT_Ip4Init();
#endif

    tcpip_init(tcpip_init_done_callback, (void*)done_sem);

    /* waiting for initialization done */
    if (osSemaphoreAcquire(done_sem, osWaitForever) != osOK)
    {
        osSemaphoreDelete(done_sem);

        return -1;
    }
    osSemaphoreDelete(done_sem);

    s_init_ok = OS_TRUE;

    return 0;
}
#endif

void sys_init(void)
{ /* nothing on OneOS porting */
}

void lwip_sys_init(void)
{
    lwip_system_init();
}

/*
 * Create a new semaphore
 *
 * @return the operation status, ERR_OK on OK; others on error
 */
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    osSemaphoreId_t id;

    id = osSemaphoreNew(1U, count, NULL);
    if (id == OS_NULL)
        return ERR_MEM;
    else
    {
        *sem = id;
        return ERR_OK;
    }
}

/*
 * Deallocate a semaphore
 */
void sys_sem_free(sys_sem_t *sem)
{
    osSemaphoreDelete(*sem);
}

/*
 * Signal a semaphore
 */
void sys_sem_signal(sys_sem_t *sem)
{
    osSemaphoreRelease(*sem);
}

/*
 * Block the thread while waiting for the semaphore to be signaled
 *
 * @return If the timeout argument is non-zero, it will return the number of
 * milliseconds spent waiting for the semaphore to be signaled; If the semaphore
 * isn't signaled within the specified time, it will return SYS_ARCH_TIMEOUT; If
 * the thread doesn't wait for the semaphore, it will return zero
 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    osStatus_t ret  = osOK;
    u64_t    t    = 0;
    u32_t    tick = 0;

    /* get the begin tick */
    tick = osKernelGetTickCount();
    if (timeout == 0)
        t = osWaitForever;
    else
    {
        /* convert msecond to os tick */
        if (timeout < (1000 / OS_TICK_PER_SECOND))
            t = 1;
        else
            t = timeout / (1000 / OS_TICK_PER_SECOND);
    }

    ret = osSemaphoreAcquireRelaxed(*sem, t, osWaitForever);

    if (ret == osErrorTimeout)
        return SYS_ARCH_TIMEOUT;
    else
    {
        if (ret == osOK)
            ret = 1;
    }

    /* get elapse msecond */
    tick = osKernelGetTickCount() - tick;

    /* convert tick to msecond */
    tick = tick * (1000 / OS_TICK_PER_SECOND);
    if (tick == 0)
    {
        tick = 1;
    }

    return tick;
}

#ifndef sys_sem_valid
/** Check if a semaphore is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_sem_valid(sys_sem_t *sem)
{
 /*return (int)(*sem);*/
    if(OS_NULL != *sem)
        return 1;
    else
        return 0;
}
#endif

#ifndef sys_sem_set_invalid
/** Set a semaphore invalid so that sys_sem_valid returns 0
 */
void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = OS_NULL;
}
#endif

/* ====================== Mutex ====================== */

/** Create a new mutex
 * @param mutex pointer to the mutex to create
 * @return a new mutex
 */
err_t sys_mutex_new(sys_mutex_t *mutex)
{
    osMutexId_t id;
    osMutexAttr_t attr;

    memset(&attr, 0 ,sizeof(attr));
    attr.attr_bits = osMutexPrioInherit | osMutexRecursive;


    id = osMutexNew(&attr);
    if (id == OS_NULL)
        return ERR_MEM;
    else
    {
        *mutex = id;
        return ERR_OK;
    }
}

/** Lock a mutex
 * @param mutex the mutex to lock
 */
void sys_mutex_lock(sys_mutex_t *mutex)
{
	osMutexAcquire(*mutex, osWaitForever);
}

/** Unlock a mutex
 * @param mutex the mutex to unlock
 */
void sys_mutex_unlock(sys_mutex_t *mutex)
{
    osMutexRelease(*mutex);
}

/** Delete a semaphore
 * @param mutex the mutex to delete
 */
void sys_mutex_free(sys_mutex_t *mutex)
{
    osMutexDelete(*mutex);
}

#ifndef sys_mutex_valid
/** Check if a mutex is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mutex_valid(sys_mutex_t *mutex)
{
    /*return (int)(*mutex);*/
    if(OS_NULL != *mutex)
        return 1;
    else
        return 0;
}
#endif

#ifndef sys_mutex_set_invalid
/** Set a mutex invalid so that sys_mutex_valid returns 0
 */
void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    *mutex = OS_NULL;
}
#endif

/* ====================== Mailbox ====================== */

/*
 * Create an empty mailbox for maximum "size" elements
 *
 * @return the operation status, ERR_OK on OK; others on error
 */
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    osMb_t            tmpmbox;

    tmpmbox = osMbCreate(size, OS_IPC_FLAG_FIFO);
    if (tmpmbox != OS_NULL)
    {
        *mbox = tmpmbox;
        return ERR_OK;
    }

    return ERR_MEM;
}

/*
 * Deallocate a mailbox
 */
void sys_mbox_free(sys_mbox_t *mbox)
{
    osMbDestory(*mbox);
}

/** Post a message to an mbox - may not fail
 * -> blocks if full, only used from tasks not from ISR
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    (void)osMbSend(*mbox, (ubase_t)msg, osWaitForever);
}

/*
 * Try to post the "msg" to the mailbox
 *
 * @return return ERR_OK if the "msg" is posted, ERR_MEM if the mailbox is full
 */
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    if (osMbSend(*mbox, (ubase_t)msg, osNoWait) == osOK)
        return ERR_OK;

    return ERR_MEM;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
    return sys_mbox_trypost(mbox, msg);
}

/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return time (in milliseconds) waited for a message, may be 0 if not waited
           or SYS_ARCH_TIMEOUT on timeout
 *         The returned time has to be accurate to prevent timer jitter!
 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    osStatus_t ret;
    u64_t    t;
    u32_t    tick;

    /* get the begin tick */
    tick = osKernelGetTickCount();

    if (timeout == 0)
        t = osWaitForever;
    else
    {
        /* convirt msecond to os tick */
        if (timeout < (1000 / OS_TICK_PER_SECOND))
            t = 1;
        else
            t = timeout / (1000 / OS_TICK_PER_SECOND);
    }

    ret = osMbRecvRelaxed(*mbox, (ubase_t*)msg, t, osWaitForever);
    if (ret != osOK)
    {
        return SYS_ARCH_TIMEOUT;
    }

    /* get elapse msecond */
    tick = osKernelGetTickCount() - tick;

    /* convert tick to msecond */
    tick = tick * (1000 / OS_TICK_PER_SECOND);
    if (tick == 0)
        tick = 1;

    return tick;
}

/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return 0 (milliseconds) if a message has been received
 *         or SYS_MBOX_EMPTY if the mailbox is empty
 */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    int ret;

    ret = osMbRecv(*mbox, (ubase_t*)msg, osNoWait);

    if (ret == osErrorTimeout || ret == osErrorResource)
        return SYS_ARCH_TIMEOUT;
    else
    {
        if (ret == osOK)
            ret = 1;
    }

    return ret;
}

#ifndef sys_mbox_valid
/** Check if an mbox is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mbox_valid(sys_mbox_t *mbox)
{
   /* return (int)(*mbox);*/
    if(OS_NULL != *mbox)
        return 1;
    else
        return 0;
}
#endif

#ifndef sys_mbox_set_invalid
/** Set an mbox invalid so that sys_mbox_valid returns 0
 */
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = OS_NULL;
}
#endif

/* ====================== System ====================== */

/*
TODO:
 * Start a new thread named "name" with priority "prio" that will begin
 * its execution in the function "thread()". The "arg" argument will be
 * passed as an argument to the thread() function
 */
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    osThreadId_t id;
    osThreadAttr_t attr = {name, osThreadDetached, NULL, 0U, NULL, stacksize, prio, 0U, 0U};

    /* create thread */
    id = osThreadNew(thread, arg, &attr);
    OS_ASSERT(id != OS_NULL);

    return id;
}

/* in fact as SYS_ARCH_PROTECT defined in cc.h */
/* sys_arch_protect and sys_arch_unprotect will not be used*/
#if 0
sys_prot_t sys_arch_protect(void)
{
    os_base_t level;

    /* disable interrupt */
    level = os_irq_lock();

    return level;
}

void sys_arch_unprotect(sys_prot_t pval)
{
    /* enable interrupt */
    os_irq_unlock(pval);

    return;
}
#endif

void sys_arch_assert(const char *file, int line)
{
    osPrintf("\r\nAssertion: %d, %s, thread %s\r\n", line, file, osThreadGetName(osThreadSelf()));
    OS_ASSERT(0);
}

u32_t sys_jiffies(void)
{
    return osKernelGetTickCount() * (1000 / OS_TICK_PER_SECOND);
}

u64_t sys_jiffies64(void)
{
    return osTickGet() * (1000 / OS_TICK_PER_SECOND);
}

u32_t sys_now(void)
{
    return osKernelGetTickCount() * (1000 / OS_TICK_PER_SECOND);
}

#if LWIP_USING_WAKEUP_TIMER
static void lwip_sys_wakeup_timer_callback(void *argument)
{
    LWIP_DEBUGF(WAKEUP_TIMER_DEBUG, ("lwip timer wakeup at %llu\n", sys_jiffies64()));
    ;
}

/**
 ************************************************************************************
 * @brief           创建唤醒定时器
 *
 * @param[in]       callback 回调函数
 * @param[in]       argument 用户参数
 *
 * @return   sys_timer_t         唤醒定时器句柄
 * @note  单次定时器，主要用于唤醒系统
 ************************************************************************************
*/
sys_timer_t sys_wakeup_timer_new(sys_timer_callback_t callback, void *argument)
{
    LWIP_DEBUGF(WAKEUP_TIMER_DEBUG, ("lwip timer sys_timer_new\n"));
    if(callback == NULL)
    {
        callback = lwip_sys_wakeup_timer_callback;
    }
    return osTimerNew(callback, osTimerOnce, argument, NULL);
}

/**
 ************************************************************************************
 * @brief           删除唤醒定时器
 *
 * @param[in]       timer         唤醒定时器句柄
 *
 * @return
 * @note  tags用于debug日志输出
 ************************************************************************************
*/
void sys_wakeup_timer_free(sys_timer_t timer,int tags)
{
    LWIP_DEBUGF(WAKEUP_TIMER_DEBUG, ("lwip timer sys_timer_free tmr %p %d\n", timer, tags));
    if(timer != NULL)
    {
        (void)osTimerStop(timer);
        (void)osTimerDelete(timer);
    }
}

/**
 ************************************************************************************
 * @brief           启动唤醒定时器
 *
 * @param[in]       timer         唤醒定时器句柄
 * @param[in]       timeout_ms    唤醒定时器时长
 *
 * @return none
 * @note  tags 和 desc用于debug日志输出
 ************************************************************************************
*/
void sys_wakeup_timer_start(sys_timer_t timer, uint32_t timeout_ms, int tags, char* desc)
{
    if(timer != NULL)
    {
        (void)osTimerStart(timer, osTickFromMs(timeout_ms));
    }

    LWIP_DEBUGF(WAKEUP_TIMER_DEBUG, ("lwip timer time %u timeout at %llu [%d][%s]\n", timeout_ms, sys_jiffies64() + timeout_ms, tags, desc == NULL ? "" : desc));
}

/**
 ************************************************************************************
 * @brief           停止唤醒定时器
 *
 * @param[in]       timer         唤醒定时器句柄
 *
 * @return none
 ************************************************************************************
*/
void sys_wakeup_timer_stop(sys_timer_t timer)
{
    if(timer != NULL)
    {
        osTimerStop(timer);
    }
}

/**
 ************************************************************************************
 * @brief           获取唤醒定时器剩余的时间
 *
 * @param[in]       timer         唤醒定时器句柄
 * @param[out]      timeout_ms    唤醒定时器剩余时间ms
 *
 * @return none
 ************************************************************************************
*/
void sys_wakeup_timer_remain(sys_timer_t timer, uint32_t *timeout_ms)
{
    if(timer != NULL)
    {
        *timeout_ms = (uint32_t)osTimerGetRemainMs(timer);
    }
}
#endif

#if MEM_OVERFLOW_CHECK || MEMP_OVERFLOW_CHECK
/**
 * Check if a mep element was victim of an overflow or underflow
 * (e.g. the restricted area after/before it has been altered)
 *
 * @param p the mem element to check
 * @param size allocated size of the element
 * @param descr1 description of the element source shown on error
 * @param descr2 description of the element source shown on error
 */
void mem_overflow_check_raw(void *p, size_t size, const char *descr1, const char *descr2)
{
#if MEM_SANITY_REGION_AFTER_ALIGNED || MEM_SANITY_REGION_BEFORE_ALIGNED
    u16_t k;
    u8_t *m;

#if MEM_SANITY_REGION_AFTER_ALIGNED > 0
    m = (u8_t *)p + size;
    for (k = 0; k < MEM_SANITY_REGION_AFTER_ALIGNED; k++)
    {
        if (m[k] != 0xcd)
        {
            char errstr[128];
            snprintf(errstr, sizeof(errstr), "detected mem overflow in %s%s", descr1, descr2);
            LWIP_ASSERT(errstr, 0);
        }
    }
#endif /* MEM_SANITY_REGION_AFTER_ALIGNED > 0 */

#if MEM_SANITY_REGION_BEFORE_ALIGNED > 0
    m = (u8_t *)p - MEM_SANITY_REGION_BEFORE_ALIGNED;
    for (k = 0; k < MEM_SANITY_REGION_BEFORE_ALIGNED; k++)
    {
        if (m[k] != 0xcd)
        {
            char errstr[128];
            snprintf(errstr, sizeof(errstr), "detected mem underflow in %s%s", descr1, descr2);
            LWIP_ASSERT(errstr, 0);
        }
    }
#endif /* MEM_SANITY_REGION_BEFORE_ALIGNED > 0 */
#else
    LWIP_UNUSED_ARG(p);
    LWIP_UNUSED_ARG(desc);
    LWIP_UNUSED_ARG(descr);
#endif
}

/**
 * Initialize the restricted area of a mem element.
 */
void mem_overflow_init_raw(void *p, size_t size)
{
#if MEM_SANITY_REGION_BEFORE_ALIGNED > 0 || MEM_SANITY_REGION_AFTER_ALIGNED > 0
    u8_t *m;
#if MEM_SANITY_REGION_BEFORE_ALIGNED > 0
    m = (u8_t *)p - MEM_SANITY_REGION_BEFORE_ALIGNED;
    memset(m, 0xcd, MEM_SANITY_REGION_BEFORE_ALIGNED);
#endif
#if MEM_SANITY_REGION_AFTER_ALIGNED > 0
    m = (u8_t *)p + size;
    memset(m, 0xcd, MEM_SANITY_REGION_AFTER_ALIGNED);
#endif
#else  /* MEM_SANITY_REGION_BEFORE_ALIGNED > 0 ||                                                                      \
          MEM_SANITY_REGION_AFTER_ALIGNED > 0 */
    LWIP_UNUSED_ARG(p);
    LWIP_UNUSED_ARG(desc);
#endif /* MEM_SANITY_REGION_BEFORE_ALIGNED > 0 ||                                                                      \
          MEM_SANITY_REGION_AFTER_ALIGNED > 0 */
}
#endif /* MEM_OVERFLOW_CHECK || MEMP_OVERFLOW_CHECK */

OS_WEAK
void mem_init(void)
{ /* nothing on OneOS porting */
}

void *mem_calloc(mem_size_t count, mem_size_t size)
{
    return calloc(count, size);
}

void *mem_trim(void *mem, mem_size_t size)
{
    LWIP_UNUSED_ARG(mem);
    LWIP_UNUSED_ARG(size);
    return mem;
}

void *mem_malloc(mem_size_t size)
{
    return malloc(size);
}

void mem_free(void *mem)
{
    free(mem);
}

#if 0//def LWIP_USING_PPP
u32_t sio_read(sio_fd_t fd, u8_t *buf, u32_t size)
{
    u32_t len;

    OS_ASSERT(fd != OS_NULL);

    len = os_device_read_nonblock((os_device_t *)fd, 0, buf, size);
    if (len <= 0)
        return 0;

    return len;
}

u32_t sio_write(sio_fd_t fd, u8_t *buf, u32_t size)
{
    OS_ASSERT(fd != OS_NULL);

    return os_device_write_nonblock((os_device_t *)fd, 0, buf, size);
}

void sio_read_abort(sio_fd_t fd)
{
    os_kprintf("read_abort\r\n");
}

void ppp_trace(int level, const char *format, ...)
{
    va_list     args;

    va_start(args, format);
    os_kprintf(format, args);
    va_end(args);

    return;
}
#endif

#if 0
/*
 * export bsd socket symbol for CMCC IOT Application Module
 */
#if LWIP_SOCKET
#include <lwip/sockets.h>
EXPORT_SYMBOL(lwip_accept);
EXPORT_SYMBOL(lwip_bind);
EXPORT_SYMBOL(lwip_shutdown);
EXPORT_SYMBOL(lwip_getpeername);
EXPORT_SYMBOL(lwip_getsockname);
EXPORT_SYMBOL(lwip_getsockopt);
EXPORT_SYMBOL(lwip_setsockopt);
EXPORT_SYMBOL(lwip_close);
EXPORT_SYMBOL(lwip_connect);
EXPORT_SYMBOL(lwip_listen);
EXPORT_SYMBOL(lwip_recv);
EXPORT_SYMBOL(lwip_read);
EXPORT_SYMBOL(lwip_recvfrom);
EXPORT_SYMBOL(lwip_send);
EXPORT_SYMBOL(lwip_sendto);
EXPORT_SYMBOL(lwip_socket);
EXPORT_SYMBOL(lwip_write);
EXPORT_SYMBOL(lwip_select);
EXPORT_SYMBOL(lwip_ioctl);
EXPORT_SYMBOL(lwip_fcntl);

EXPORT_SYMBOL(lwip_htons);
EXPORT_SYMBOL(lwip_htonl);

#if LWIP_DNS
#include <lwip/netdb.h>
EXPORT_SYMBOL(lwip_gethostbyname);
EXPORT_SYMBOL(lwip_gethostbyname_r);
EXPORT_SYMBOL(lwip_freeaddrinfo);
EXPORT_SYMBOL(lwip_getaddrinfo);
#endif

#endif

#if LWIP_DHCP
#include <lwip/dhcp.h>
EXPORT_SYMBOL(dhcp_start);
EXPORT_SYMBOL(dhcp_renew);
EXPORT_SYMBOL(dhcp_stop);
#endif

#if LWIP_NETIF_API
#include <lwip/netifapi.h>
EXPORT_SYMBOL(netifapi_netif_set_addr);
#endif

#if LWIP_NETIF_LINK_CALLBACK
EXPORT_SYMBOL(netif_set_link_callback);
#endif

#if LWIP_NETIF_STATUS_CALLBACK
EXPORT_SYMBOL(netif_set_status_callback);
#endif

EXPORT_SYMBOL(netif_find);
EXPORT_SYMBOL(netif_set_addr);
EXPORT_SYMBOL(netif_set_ipaddr);
EXPORT_SYMBOL(netif_set_gw);
EXPORT_SYMBOL(netif_set_netmask);

#endif
