/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-10-26     Bernard      the first version
 */

#include <pthread.h>
#include "pthread_internal.h"
#include <errno.h>

int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    if (!attr)
        return EINVAL;

    return 0;
}
RTM_EXPORT(pthread_condattr_destroy);

int pthread_condattr_init(pthread_condattr_t *attr)
{
    if (!attr)
        return EINVAL;
    attr->is_initialized = PTHREAD_PROCESS_PRIVATE;

    return 0;
}
RTM_EXPORT(pthread_condattr_init);

int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared)
{
    if (!attr || !pshared)
        return EINVAL;

    *pshared = PTHREAD_PROCESS_PRIVATE;

    return 0;
}
RTM_EXPORT(pthread_condattr_getpshared);

int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
    if ((pshared != PTHREAD_PROCESS_PRIVATE) &&
        (pshared != PTHREAD_PROCESS_SHARED))
    {
        return EINVAL;
    }

    if (pshared != PTHREAD_PROCESS_PRIVATE)
        return ENOSYS;

    return 0;
}
RTM_EXPORT(pthread_condattr_setpshared);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    osStatus_t result;
    char *cond_name;
    static uint16_t cond_num = 0;
    _pthread_cond_t *cond_cb = OS_NULL;

    /* parameter check */
    if (cond == OS_NULL)
        return EINVAL;
    if ((attr != OS_NULL) && (attr->is_initialized != PTHREAD_PROCESS_PRIVATE))
        return EINVAL;

    cond_cb = (_pthread_cond_t *)osMalloc(sizeof(_pthread_cond_t));
    if(cond_cb == OS_NULL)
        return ENOMEM;

    cond_name = (char *)osMalloc(OS_NAME_MAX);
    if(cond_name == OS_NULL)
    {
        osFree(cond_cb);
        return ENOMEM;
    }
    osSnprintf(cond_name, OS_NAME_MAX, "cond%02d", cond_num++);

    if (attr == OS_NULL) /* use default value */
        cond_cb->attr.is_initialized = PTHREAD_PROCESS_PRIVATE;
    else
        cond_cb->attr.is_initialized  = attr->is_initialized ;

    result = osSemaphoreInit(&cond_cb->sem, 0, OS_SEM_VALUE_MAX, OS_IPC_FLAG_FIFO);
    if (result != osOK)
    {
        osFree(cond_name);
        osFree(cond_cb);
        return EINVAL;
    }

    /* set name */
    cond_cb->sem.name = cond_name;

    /* detach the object from system object container */
    osObjectDetach(&(cond_cb->sem.parent.parent));
    cond_cb->sem.parent.parent.type = osObject_Class_Semaphore;

    *cond = (pthread_cond_t)cond_cb;
    return 0;
}
RTM_EXPORT(pthread_cond_init);

int pthread_cond_destroy(pthread_cond_t *cond)
{
    osStatus_t result;
    _pthread_cond_t *ci = OS_NULL;

    if (cond == OS_NULL)
        return EINVAL;
    if (*cond == 0)
        return 0; /* which is not initialized */

    ci = (_pthread_cond_t *)(*cond);

    result = osSemaphoreAcquire(&(ci->sem), 0);
    if (result != osOK)
        return EBUSY;

    if(ci->sem.name)
        osFree((void *)ci->sem.name);

    osSemaphoreDelete(&(ci->sem));

    /* clean condition */
    osMemset(ci, 0, sizeof(pthread_cond_t));
    osFree(ci);
    *cond = 0;

    return 0;
}
RTM_EXPORT(pthread_cond_destroy);

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    osStatus_t result;
    _pthread_cond_t *ci = OS_NULL;

    if (cond == OS_NULL)
        return EINVAL;
    if (*cond == -1)
        pthread_cond_init(cond, OS_NULL);

    ci = (_pthread_cond_t *)(*cond);

    osCriticalEnter();
    while (1)
    {
        /* try to take condition semaphore */
        result = osSemaphoreAcquire(&(ci->sem), 0);
        if (result == osErrorTimeout)
        {
            /* it's timeout, release this semaphore */
            osSemaphoreRelease(&(ci->sem));
        }
        else if (result == osOK)
        {
            /* has taken this semaphore, release it */
            osSemaphoreRelease(&(ci->sem));
            break;
        }
        else
        {
            osCriticalExit();

            return EINVAL;
        }
    }
    osCriticalExit();

    return 0;
}
RTM_EXPORT(pthread_cond_broadcast);

int pthread_cond_signal(pthread_cond_t *cond)
{
    osStatus_t result;
    _pthread_cond_t *ci = OS_NULL;

    if (cond == OS_NULL)
        return EINVAL;
    if (*cond == 0)
        pthread_cond_init(cond, OS_NULL);

    ci = (_pthread_cond_t *)(*cond);

    result = osSemaphoreRelease(&(ci->sem));
    if (result == osOK)
        return 0;

    return 0;
}
RTM_EXPORT(pthread_cond_signal);

osStatus_t _pthread_cond_timedwait(pthread_cond_t  *cond,
                                 pthread_mutex_t *mutex,
                                 int32_t       timeout)
{
    osStatus_t result;
    _pthread_cond_t *ci = OS_NULL;
    _pthread_mutex_t *mi = OS_NULL;

    if (!cond || !mutex)
        return osError;
    /* check whether initialized */
    if (*cond == 0)
        pthread_cond_init(cond, OS_NULL);
    
    ci = (_pthread_cond_t *)(*cond);
    mi = (_pthread_mutex_t *)(*mutex);

    /* The mutex was not owned by the current thread at the time of the call. */
    if (mi->lock.owner != osThreadSelf())
        return osError;
    /* unlock a mutex failed */
    if (pthread_mutex_unlock(mutex) != 0)
        return osError;

    result = osSemaphoreAcquire(&(ci->sem), timeout);
    /* lock mutex again */
    pthread_mutex_lock(mutex);

    return result;
}
RTM_EXPORT(_pthread_cond_timedwait);

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    osStatus_t result;

    result = _pthread_cond_timedwait(cond, mutex, osWaitForever);
    if (result == osOK)
        return 0;

    return EINVAL;
}
RTM_EXPORT(pthread_cond_wait);

int pthread_cond_timedwait(pthread_cond_t        *cond,
                           pthread_mutex_t       *mutex,
                           const struct timespec *abstime)
{
    int timeout;
    osStatus_t result;

    timeout = timespec_diff_ms(abstime);
    result = _pthread_cond_timedwait(cond, mutex, timeout);
    if (result == osOK)
        return 0;
    if (result == osErrorTimeout)
        return ETIMEDOUT;

    return EINVAL;
}
RTM_EXPORT(pthread_cond_timedwait);

