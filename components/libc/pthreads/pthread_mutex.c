/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-10-26     Bernard      the first version
 */

#include <os.h>
#include <pthread.h>
#include <pthread_internal.h>
#include <errno.h>

#define  MUTEXATTR_SHARED_MASK 0x0010
#define  MUTEXATTR_TYPE_MASK   0x000f


int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (attr)
    {
        attr->is_initialized = PTHREAD_PROCESS_PRIVATE;

        return 0;
    }

    return EINVAL;
}
RTM_EXPORT(pthread_mutexattr_init);

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    if (attr)
    {
        attr->is_initialized = -1;

        return 0;
    }

    return EINVAL;
}
RTM_EXPORT(pthread_mutexattr_destroy);

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    if (attr && type)
    {
        int  atype = (attr->is_initialized & MUTEXATTR_TYPE_MASK);

        if (atype >= PTHREAD_MUTEX_NORMAL && atype <= PTHREAD_MUTEX_ERRORCHECK)
        {
            *type = atype;

            return 0;
        }
    }

    return EINVAL;
}
RTM_EXPORT(pthread_mutexattr_gettype);

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (attr && type >= PTHREAD_MUTEX_NORMAL && type <= PTHREAD_MUTEX_ERRORCHECK)
    {
        attr->is_initialized = (attr->is_initialized & ~MUTEXATTR_TYPE_MASK) | type;

        return 0;
    }

    return EINVAL;
}
RTM_EXPORT(pthread_mutexattr_settype);

int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
    if (!attr)
        return EINVAL;

    switch (pshared)
    {
    case PTHREAD_PROCESS_PRIVATE:
        attr->is_initialized &= ~MUTEXATTR_SHARED_MASK;
        return 0;

    case PTHREAD_PROCESS_SHARED:
        attr->is_initialized |= MUTEXATTR_SHARED_MASK;
        return 0;
    }

    return EINVAL;
}
RTM_EXPORT(pthread_mutexattr_setpshared);

int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared)
{
    if (!attr || !pshared)
        return EINVAL;

    *pshared = (attr->is_initialized & MUTEXATTR_SHARED_MASK) ? PTHREAD_PROCESS_SHARED
                                               : PTHREAD_PROCESS_PRIVATE;
    return 0;
}
RTM_EXPORT(pthread_mutexattr_getpshared);

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    osStatus_t result;
    char *name;
    static uint16_t pthread_mutex_number = 0;
    _pthread_mutex_t *mutex_cb = OS_NULL;

    if (!mutex)
        return EINVAL;

    mutex_cb = (_pthread_mutex_t *)osMalloc(sizeof(_pthread_mutex_t));
    if(mutex_cb == OS_NULL)
        return ENOMEM;

    /* build mutex name */
    name = (char *)osMalloc(OS_NAME_MAX);
    if(name == OS_NULL)
        return ENOMEM;

    osSnprintf(name, OS_NAME_MAX, "pmtx%02d", pthread_mutex_number ++);

    if (attr == OS_NULL)
        mutex_cb->attr.is_initialized = PTHREAD_PROCESS_PRIVATE;
    else
        mutex_cb->attr.is_initialized = attr->is_initialized;

    /* init mutex lock */
    result = osMutexInit(&(mutex_cb->lock), OS_IPC_FLAG_FIFO);
    if (result != osOK)
        return EINVAL;

    /* set name */
    mutex_cb->lock.name = name;

    /* detach the object from system object container */
    osObjectDetach(&(mutex_cb->lock.parent.parent));
    mutex_cb->lock.parent.parent.type = osObject_Class_Mutex;

    *mutex = (pthread_mutex_t)mutex_cb;
    return 0;
}
RTM_EXPORT(pthread_mutex_init);

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    _pthread_mutex_t *mi = OS_NULL;

    if (!mutex || *mutex == 0)
        return EINVAL;

    mi = (_pthread_mutex_t *)(*mutex);

    /* it's busy */
    if (mi->lock.owner != OS_NULL)
        return EBUSY;

    if(mi->lock.name)
        osFree((void *)mi->lock.name);

    osMutexDelete(&mi->lock);
    osMemset(mi, 0, sizeof(pthread_mutex_t));
    osFree(mi);
    *mutex = 0;

    return 0;
}
RTM_EXPORT(pthread_mutex_destroy);

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    int mtype;
    osStatus_t result;
    _pthread_mutex_t *mi = OS_NULL;

    if (!mutex)
        return EINVAL;

    if (*mutex == 0)
    {
        /* init mutex */
        pthread_mutex_init(mutex, OS_NULL);
    }

    mi = (_pthread_mutex_t *)(*mutex);

    mtype = mi->attr.is_initialized & MUTEXATTR_TYPE_MASK;
    osCriticalEnter();
    if (mi->lock.owner == osThreadSelf() &&
        mtype != PTHREAD_MUTEX_RECURSIVE)
    {
        osCriticalExit();

        return EDEADLK;
    }
    osCriticalExit();

    result = osMutexAcquire(&(mi->lock), osWaitForever);
    if (result == osOK)
        return 0;

    return EINVAL;
}
RTM_EXPORT(pthread_mutex_lock);

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    osStatus_t result;
    _pthread_mutex_t *mi = OS_NULL;

    if (!mutex)
        return EINVAL;
    if (*mutex == 0)
    {
        /* init mutex */
        pthread_mutex_init(mutex, OS_NULL);
    }

    mi = (_pthread_mutex_t *)(*mutex);

    if (mi->lock.owner != osThreadSelf())
    {
        int mtype;
        mtype = mi->attr.is_initialized & MUTEXATTR_TYPE_MASK;

        /* error check, return EPERM */
        if (mtype == PTHREAD_MUTEX_ERRORCHECK)
            return EPERM;

        /* no thread waiting on this mutex */
        if (mi->lock.owner == OS_NULL)
            return 0;
    }

    result = osMutexRelease(&(mi->lock));
    if (result == osOK)
        return 0;

    return EINVAL;
}
RTM_EXPORT(pthread_mutex_unlock);

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    osStatus_t result;
    int mtype;
    _pthread_mutex_t *mi = OS_NULL;

    if (!mutex)
        return EINVAL;
    if (*mutex == 0)
    {
        /* init mutex */
        pthread_mutex_init(mutex, OS_NULL);
    }

    mi = (_pthread_mutex_t *)(*mutex);

    mtype = mi->attr.is_initialized & MUTEXATTR_TYPE_MASK;
    osCriticalEnter();
    if (mi->lock.owner == osThreadSelf() &&
        mtype != PTHREAD_MUTEX_RECURSIVE)
    {
        osCriticalExit();

        return EDEADLK;
    }
    osCriticalExit();

    result = osMutexAcquire(&(mi->lock), 0);
    if (result == osOK) return 0;

    return EBUSY;
}
RTM_EXPORT(pthread_mutex_trylock);

int	pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
    osStatus_t result;
    int mtype;
    _pthread_mutex_t *mi = OS_NULL;
    int timeout;

    if (!mutex || !abstime)
        return EINVAL;
    if (*mutex == 0)
    {
        /* init mutex */
        pthread_mutex_init(mutex, OS_NULL);
    }

    mi = (_pthread_mutex_t *)(*mutex);

    mtype = mi->attr.is_initialized & MUTEXATTR_TYPE_MASK;
    osCriticalEnter();
    if (mi->lock.owner == osThreadSelf() &&
        mtype != PTHREAD_MUTEX_RECURSIVE)
    {
        osCriticalExit();

        return EDEADLK;
    }
    osCriticalExit();

    timeout = timespec_diff_ms(abstime);

    result = osMutexAcquire(&(mi->lock), timeout);
    if (result == osOK) return 0;

    return EBUSY;
}
RTM_EXPORT(pthread_mutex_timedlock);