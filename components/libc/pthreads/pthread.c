/**
 *************************************************************************************
 * @copyright 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 *            保留所有权利。
 *
 * @file      pthread.c
 *
 * @brief     phtread接口实现.
 *
 * @revision  1.0
 *
 * @par 修改日志:
 * <table>
 * <tr><th>Date        <th>Version   <th>Author     <th>Description
 * <tr><td>2026-01-26  <td>1.0       <td>ICT Team   <td>初始版本
 * </table>
 ************************************************************************************
 */
#include <stdlib.h>
#include <errno.h>
#include <os.h>
#include <pthread.h>
#include "pthread_internal.h"

OS_DEFINE_SPINLOCK(pth_lock);
_pthread_data_t *pth_table[PTHREAD_NUM_MAX] = {NULL};

_pthread_data_t *_pthread_get_data(pthread_t thread)
{
    OS_DECLARE_SPINLOCK(pth_lock);
    _pthread_data_t *ptd;

    if (thread >= PTHREAD_NUM_MAX) return NULL;

    osHwSpinLock(&pth_lock);
    ptd = pth_table[thread];
    osHwSpinUnlock(&pth_lock);

    if (ptd && ptd->magic == PTHREAD_MAGIC) return ptd;

    return NULL;
}

pthread_t _pthread_data_get_pth(_pthread_data_t *ptd)
{
    int index;
    OS_DECLARE_SPINLOCK(pth_lock);

    osHwSpinLock(&pth_lock);
    for (index = 0; index < PTHREAD_NUM_MAX; index ++)
    {
        if (pth_table[index] == ptd) break;
    }
    osHwSpinUnlock(&pth_lock);

    return index;
}

pthread_t _pthread_data_create(void)
{
    int index;
    _pthread_data_t *ptd = NULL;
    OS_DECLARE_SPINLOCK(pth_lock);

    ptd = (_pthread_data_t*)osMalloc(sizeof(_pthread_data_t));
    if (!ptd) return PTHREAD_NUM_MAX;

    memset(ptd, 0x0, sizeof(_pthread_data_t));
    ptd->canceled = 0;
    ptd->cancelstate = PTHREAD_CANCEL_DISABLE;
    ptd->canceltype = PTHREAD_CANCEL_DEFERRED;
    ptd->magic = PTHREAD_MAGIC;

    osHwSpinLock(&pth_lock);
    for (index = 0; index < PTHREAD_NUM_MAX; index ++)
    {
        if (pth_table[index] == NULL)
        {
            pth_table[index] = ptd;
            break;
        }
    }
    osHwSpinUnlock(&pth_lock);

    /* full of pthreads, clean magic and release ptd */
    if (index == PTHREAD_NUM_MAX)
    {
        ptd->magic = 0x0;
        osFree(ptd);
    }

    return index;
}

void _pthread_data_destroy(pthread_t pth)
{
    OS_DECLARE_SPINLOCK(pth_lock);

    _pthread_data_t *ptd = _pthread_get_data(pth);
    if (ptd)
    {
        /* remove from pthread table */
        osHwSpinLock(&pth_lock);
        pth_table[pth] = NULL;
        osHwSpinUnlock(&pth_lock);

        /* delete joinable semaphore */
        // if (ptd->joinable_sem != OS_NULL)
        // {
        //     if (ptd->joinable_sem->name)
        //         osFree((void *)ptd->joinable_sem->name);

        //     osSemaphoreDetach(ptd->joinable_sem);
        //     osFree(ptd->joinable_sem);
        //     ptd->joinable_sem = OS_NULL;
        // }

        if (ptd->tid)
        {
            if (ptd->tid->name)
                osFree((void *)(ptd->tid->name));

            /* release thread resource */
            if (ptd->attr.stackaddr == OS_NULL && ptd->tid->stackAddr != OS_NULL)
            {
                /* release thread allocated stack */
                osFree(ptd->tid->stackAddr);
            }
            /* clean stack addr pointer */
            ptd->tid->stackAddr = OS_NULL;
        }

        /*
        * if this thread create the local thread data,
        * delete it
        */
        if (ptd->tls != OS_NULL) osFree(ptd->tls);
        osFree(ptd->tid);

        /* clean magic */
        ptd->magic = 0x0;

        /* free ptd */
        osFree(ptd);
    }
}

int pthread_system_init(void)
{
    /* initialize key area */
    // pthread_key_system_init();
    /* initialize posix mqueue */
    posix_mq_system_init();
    /* initialize posix semaphore */
    posix_sem_system_init();

    return 0;
}
INIT_COMPONENT_EXPORT(pthread_system_init, OS_INIT_SUBLEVEL_MIDDLE);

static void _pthread_destroy(_pthread_data_t *ptd)
{
    pthread_t pth = _pthread_data_get_pth(ptd);
    if (pth != PTHREAD_NUM_MAX)
    {
        _pthread_data_destroy(pth);
    }

    return;
}

static void _pthread_cleanup(osThread_t tid)
{
    _pthread_data_t *ptd;

    /* get pthread data from user data of thread */
    ptd = (_pthread_data_t *)tid->userData;
    OS_ASSERT(ptd != OS_NULL);

    /* clear cleanup function */
    tid->cleanup = OS_NULL;
    if (ptd->attr.detachstate == PTHREAD_CREATE_JOINABLE)
    {
        // osSemaphoreRelease(ptd->joinable_sem);
    }
    else
    {
        /* release pthread resource */
        _pthread_destroy(ptd);
    }
}

static void pthread_entry_stub(void *parameter)
{
    void *value;
    _pthread_data_t *ptd;

    ptd = (_pthread_data_t *)parameter;

    /* execute pthread entry */
    value = ptd->thread_entry(ptd->thread_parameter);
    /* set value */
    ptd->return_value = value;
}

int pthread_create(pthread_t            *pid,
                   const pthread_attr_t *attr,
                   void *(*start)(void *), void *parameter)
{
    int ret = 0;
    void *stack;
    char *name = OS_NULL;
    static uint16_t pthread_number = 0;

    pthread_t pth_id;
    _pthread_data_t *ptd;

    /* in posix pthread, defualt mode should be joinable */
    uint16_t attrFlags = osThreadJoinable;

    /* pid shall be provided */
    OS_ASSERT(pid != OS_NULL);

    /* allocate posix thread data */
    pth_id = _pthread_data_create();
    if (pth_id == PTHREAD_NUM_MAX)
    {
        ret = ENOMEM;
        goto __exit;
    }
    /* get pthread data */
    ptd = _pthread_get_data(pth_id);

    if (attr != OS_NULL)
    {
        if(attr->is_initialized != 1)
        {
            ret = EINVAL;
            goto __exit;
        }
        ptd->attr = *attr;
    }
    else
    {
        /* use default attribute */
        pthread_attr_init(&ptd->attr);
    }

    name = (char *)osMalloc(OS_NAME_MAX);
    if(name == OS_NULL)
    {
        ret = ENOMEM;
        goto __exit;
    }

    osSnprintf(name, OS_NAME_MAX, "pth%02d", pthread_number ++);

    /* pthread is a static thread object */
    ptd->tid = (osThread_t) osMalloc(sizeof(struct osThread));
    if (ptd->tid == OS_NULL)
    {
        ret = ENOMEM;
        goto __exit;
    }
    memset(ptd->tid, 0, sizeof(struct osThread));

    if (ptd->attr.detachstate == PTHREAD_CREATE_JOINABLE)
    {
        // ptd->joinable_sem = osMalloc(sizeof(struct osSemaphore));
        // if (ptd->joinable_sem == OS_NULL)
        // {
        //     ret = ENOMEM;
        //     goto __exit;
        // }
        // ptd->joinable_sem->name = osCalloc(1, OS_NAME_MAX);
        // if (ptd->joinable_sem->name == OS_NULL)
        // {
        //     ret = ENOMEM;
        //     goto __exit;
        // }
        // osSnprintf((char *)ptd->joinable_sem->name, sizeof(ptd->joinable_sem->name), "%s", name);
        // osSemaphoreInit(ptd->joinable_sem, 0, OS_SEM_VALUE_MAX, OS_IPC_FLAG_FIFO);
    }
    else
    {
        attrFlags = osThreadDetached;
        // ptd->joinable_sem = OS_NULL;
    }

    /* set parameter */
    ptd->thread_entry = start;
    ptd->thread_parameter = parameter;

    /* stack */
    if (ptd->attr.stackaddr == 0)
    {
        stack = (void *)osMalloc(ptd->attr.stacksize);
    }
    else
    {
        stack = (void *)(ptd->attr.stackaddr);
    }

    if (stack == OS_NULL)
    {
        ret = ENOMEM;
        goto __exit;
    }

    /* initial this pthread to system */
    if (osThreadInit(ptd->tid, name, pthread_entry_stub, ptd,
                       stack, ptd->attr.stacksize,
                       ptd->attr.schedparam.sched_priority, 5) != osOK)
    {
        ret = EINVAL;
        goto __exit;
    }
    OSOBJ_ATTR(ptd->tid) = attrFlags;

    /* name has been stored by os thread obj, reset this to avoid double free */
    name = OS_NULL;

    /* set pthread id */
    *pid = pth_id;

    /* set pthread cleanup function and ptd data */
    // ptd->tid->cleanup = _pthread_cleanup;
    ptd->tid->userData = (uint32_t)ptd;

    /* start thread */
    if (osThreadStart(ptd->tid) == osOK)
        return 0;

    /* start thread failed */
    // osThreadDetach(ptd->tid);
    osThreadDeinit(ptd->tid);
    ret = EINVAL;

__exit:
    if(name)
        osFree(name);

    if (pth_id != PTHREAD_NUM_MAX)
        _pthread_data_destroy(pth_id);
    return ret;
}
RTM_EXPORT(pthread_create);

int pthread_detach(pthread_t thread)
{
    int ret = 0;
    _pthread_data_t *ptd = _pthread_get_data(thread);

    osCriticalEnter();
    if (ptd->attr.detachstate == PTHREAD_CREATE_DETACHED)
    {
        /* The implementation has detected that the value specified by thread does not refer
         * to a joinable thread.
         */
        ret = EINVAL;
        goto __exit;
    }

    if ((ptd->tid->stat & OS_THREAD_STAT_MASK) == OS_THREAD_CLOSE)
    {
        // /* this defunct pthread is not handled by idle */
        // if (osSemaphoreAcquire(ptd->joinable_sem, 0) != osOK)
        // {
        //     osSemaphoreRelease(ptd->joinable_sem);

        //     /* change to detach state */
        //     ptd->attr.detachstate = PTHREAD_CREATE_DETACHED;

        //     /* delete joinable semaphore */
        //     if (ptd->joinable_sem != OS_NULL)
        //     {
        //         if (ptd->joinable_sem->name)
        //             osFree((void *)ptd->joinable_sem->name);

        //         osSemaphoreDetach(ptd->joinable_sem);
        //         osFree(ptd->joinable_sem);
        //         ptd->joinable_sem = OS_NULL;
        //     }
        // }
        // else
        // {
            /* destroy this pthread */
            _pthread_destroy(ptd);
        // }

        goto __exit;
    }
    else
    {
        /* change to detach state */
        ptd->attr.detachstate = PTHREAD_CREATE_DETACHED;

        /* delete joinable semaphore */
        // if (ptd->joinable_sem != OS_NULL)
        // {
        //     if (ptd->joinable_sem->name)
        //         osFree((void *)ptd->joinable_sem->name);

        //     osSemaphoreDetach(ptd->joinable_sem);
        //     osFree(ptd->joinable_sem);
        //     ptd->joinable_sem = OS_NULL;
        // }
    }

__exit:
    osCriticalExit();
    return ret;
}
RTM_EXPORT(pthread_detach);

int pthread_join(pthread_t thread, void **value_ptr)
{
    _pthread_data_t *ptd;
    // osStatus_t result;

    ptd = _pthread_get_data(thread);
    if (ptd && ptd->tid == osThreadSelf())
    {
        /* join self */
        return EDEADLK;
    }

    if (ptd->attr.detachstate == PTHREAD_CREATE_DETACHED)
        return EINVAL; /* join on a detached pthread */

    osThreadJoin(ptd->tid);
    // result = osSemaphoreAcquire(ptd->joinable_sem, osWaitForever);

    /* get return value */
    if (value_ptr != OS_NULL)
        *value_ptr = ptd->return_value;

    /* destroy this pthread */
    _pthread_destroy(ptd);

    return 0;
}
RTM_EXPORT(pthread_join);

pthread_t pthread_self (void)
{
    osThread_t tid;
    _pthread_data_t *ptd;

    tid = osThreadSelf();
    if (tid == NULL) return PTHREAD_NUM_MAX;

    /* get pthread data from user data of thread */
    ptd = (_pthread_data_t *)osThreadSelf()->userData;
    OS_ASSERT(ptd != OS_NULL);

    return _pthread_data_get_pth(ptd);
}
RTM_EXPORT(pthread_self);

void pthread_exit(void *value)
{
    _pthread_data_t *ptd;
    _pthread_cleanup_t *cleanup;
    // extern _pthread_key_data_t _thread_keys[PTHREAD_KEY_MAX];

    if (osThreadSelf() == OS_NULL) exit(0);

    /* get pthread data from user data of thread */
    ptd = (_pthread_data_t *)osThreadSelf()->userData;

    osCriticalEnter();
    /* disable cancel */
    ptd->cancelstate = PTHREAD_CANCEL_DISABLE;
    /* set return value */
    ptd->return_value = value;
    osCriticalExit();

    /* invoke pushed cleanup */
    while (ptd->cleanup != OS_NULL)
    {
        cleanup = ptd->cleanup;
        ptd->cleanup = cleanup->next;

        cleanup->cleanup_func(cleanup->parameter);
        /* release this cleanup function */
        osFree(cleanup);
    }

    /* destruct thread local key */
    // if (ptd->tls != OS_NULL)
    // {
    //     void *data;
    //     uint32_t index;

    //     for (index = 0; index < PTHREAD_KEY_MAX; index ++)
    //     {
    //         if (_thread_keys[index].is_used)
    //         {
    //             data = ptd->tls[index];
    //             if (data)
    //                 _thread_keys[index].destructor(data);
    //         }
    //     }

    //     /* release tls area */
    //     osFree(ptd->tls);
    //     ptd->tls = OS_NULL;
    // }

    /* detach thread */
    // ret = osThreadDetach(ptd->tid);
    // osPrintf("pthread_exit osThreadDetach ret:%d\r\n", ret);

    // osThreadCleanup(ptd->tid);

    // apply with osThreadDeinit
    osThreadDeinit(ptd->tid);

    osSchedule();

    exit(0);
}
RTM_EXPORT(pthread_exit);

int pthread_kill(pthread_t thread, int sig)
{
#ifdef OS_USING_SIGNALS
    _pthread_data_t *ptd;

    ptd = _pthread_get_data(thread);
    if (ptd)
    {
        return osThreadKill(ptd->tid, sig);
    }

    return EINVAL;
#else
    return ENOSYS;
#endif
}
RTM_EXPORT(pthread_kill);

void _pthread_cleanup_pop(struct _pthread_cleanup_context *_context, int execute)
{
    _pthread_data_t *ptd;
    _pthread_cleanup_t *cleanup;

    if (osThreadSelf() == NULL) return;

    /* get pthread data from user data of thread */
    ptd = (_pthread_data_t *)osThreadSelf()->userData;
    OS_ASSERT(ptd != OS_NULL);

    if (execute)
    {
        osCriticalEnter();
        cleanup = ptd->cleanup;
        if (cleanup)
            ptd->cleanup = cleanup->next;
        osCriticalExit();

        if (cleanup)
        {
            cleanup->cleanup_func(cleanup->parameter);

            osFree(cleanup);
        }
    }
}

void _pthread_cleanup_push(struct _pthread_cleanup_context *_context, void (*routine)(void *), void *arg)
{
    _pthread_data_t *ptd;
    _pthread_cleanup_t *cleanup;

    if (osThreadSelf() == NULL) return;

    /* get pthread data from user data of thread */
    ptd = (_pthread_data_t *)osThreadSelf()->userData;
    OS_ASSERT(ptd != OS_NULL);

    cleanup = (_pthread_cleanup_t *)osMalloc(sizeof(_pthread_cleanup_t));
    if (cleanup != OS_NULL)
    {
        cleanup->cleanup_func = routine;
        cleanup->parameter = arg;

        osCriticalEnter();
        cleanup->next = ptd->cleanup;
        ptd->cleanup = cleanup;
        osCriticalExit();
    }
}

/*
 * According to IEEE Std 1003.1, 2004 Edition , following pthreads
 * interface support cancellation point:
 * mq_receive()
 * mq_send()
 * mq_timedreceive()
 * mq_timedsend()
 * msgrcv()
 * msgsnd()
 * msync()
 * pthread_cond_timedwait()
 * pthread_cond_wait()
 * pthread_join()
 * pthread_testcancel()
 * sem_timedwait()
 * sem_wait()
 *
 * A cancellation point may also occur when a thread is
 * executing the following functions:
 * pthread_rwlock_rdlock()
 * pthread_rwlock_timedrdlock()
 * pthread_rwlock_timedwrlock()
 * pthread_rwlock_wrlock()
 *
 * The pthread_cancel(), pthread_setcancelstate(), and pthread_setcanceltype()
 * functions are defined to be async-cancel safe.
 */

int pthread_setcancelstate(int state, int *oldstate)
{
    _pthread_data_t *ptd;

    if (osThreadSelf() == NULL) return EINVAL;

    /* get pthread data from user data of thread */
    ptd = (_pthread_data_t *)osThreadSelf()->userData;
    OS_ASSERT(ptd != OS_NULL);

    if ((state == PTHREAD_CANCEL_ENABLE) || (state == PTHREAD_CANCEL_DISABLE))
    {
        if (oldstate)
            *oldstate = ptd->cancelstate;
        ptd->cancelstate = state;

        return 0;
    }

    return EINVAL;
}
RTM_EXPORT(pthread_setcancelstate);

int pthread_setcanceltype(int type, int *oldtype)
{
    _pthread_data_t *ptd;

    if (osThreadSelf() == NULL) return EINVAL;

    /* get pthread data from user data of thread */
    ptd = (_pthread_data_t *)osThreadSelf()->userData;
    OS_ASSERT(ptd != OS_NULL);

    if ((type != PTHREAD_CANCEL_DEFERRED) && (type != PTHREAD_CANCEL_ASYNCHRONOUS))
        return EINVAL;

    if (oldtype)
        *oldtype = ptd->canceltype;
    ptd->canceltype = type;

    return 0;
}
RTM_EXPORT(pthread_setcanceltype);

void pthread_testcancel(void)
{
    int cancel = 0;
    _pthread_data_t *ptd;

    if (osThreadSelf() == OS_NULL) return;

    /* get pthread data from user data of thread */
    ptd = (_pthread_data_t *)osThreadSelf()->userData;
    OS_ASSERT(ptd != OS_NULL);

    if (ptd->cancelstate == PTHREAD_CANCEL_ENABLE)
        cancel = ptd->canceled;
    if (cancel)
        pthread_exit((void *)PTHREAD_CANCELED);
}
RTM_EXPORT(pthread_testcancel);

int pthread_cancel(pthread_t thread)
{
    _pthread_data_t *ptd;

    /* get posix thread data */
    ptd = _pthread_get_data(thread);
    OS_ASSERT(ptd != OS_NULL);

    /* cancel self */
    if (ptd->tid == osThreadSelf())
        return 0;

    /* set canceled */
    if (ptd->cancelstate == PTHREAD_CANCEL_ENABLE)
    {
        ptd->canceled = 1;
        if (ptd->canceltype == PTHREAD_CANCEL_ASYNCHRONOUS)
        {
            /*
             * to detach thread.
             * this thread will be removed from scheduler list
             * and because there is a cleanup function in the
             * thread (pthread_cleanup), it will move to defunct
             * thread list and wait for handling in idle thread.
             */
            // osThreadDetach(ptd->tid);

            osThreadDeinit(ptd->tid);
        }
    }

    return 0;
}
RTM_EXPORT(pthread_cancel);

