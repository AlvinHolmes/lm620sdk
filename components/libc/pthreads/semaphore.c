/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-10-26     Bernard      the first version
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <os.h>
#include "semaphore.h"
#include "pthread_internal.h"

static sem_t *posix_sem_list = OS_NULL;
static struct osSemaphore posix_sem_lock;
void posix_sem_system_init()
{
    osSemaphoreInit(&posix_sem_lock, 1, OS_SEM_VALUE_MAX, OS_IPC_FLAG_FIFO);
}

OS_INLINE void posix_sem_insert(sem_t *psem)
{
    psem->next = posix_sem_list;
    posix_sem_list = psem;
}

static void posix_sem_delete(sem_t *psem)
{
    sem_t *iter;
    if (posix_sem_list == psem)
    {
        posix_sem_list = psem->next;

        if(psem->sem && psem->sem->name)
            osFree((void *)psem->sem->name);
        osSemaphoreDelete(psem->sem);
        osFree(psem);

        return;
    }
    for (iter = posix_sem_list; iter->next != OS_NULL; iter = iter->next)
    {
        if (iter->next == psem)
        {
            /* delete this mq */
            if (psem->next != OS_NULL)
                iter->next = psem->next;
            else
                iter->next = OS_NULL;

            /* delete RT-Thread mqueue */
            if(psem->sem && psem->sem->name)
                osFree((void *)psem->sem->name);
            osSemaphoreDelete(psem->sem);
            osFree(psem);

            return ;
        }
    }
}

static sem_t *posix_sem_find(const char* name)
{
    sem_t *iter;
    osSem_t sem;

    for (iter = posix_sem_list; iter != OS_NULL; iter = iter->next)
    {
        sem = (osSem_t)iter->sem;

        if (strncmp(sem->name, name, OS_NAME_MAX) == 0)
        {
            return iter;
        }
    }

    return OS_NULL;
}

int sem_close(sem_t *sem)
{
    if (sem == OS_NULL)
    {
        osSetErrno(EINVAL);

        return -1;
    }

    /* lock posix semaphore list */
    osSemaphoreAcquire(&posix_sem_lock, osWaitForever);
    sem->refcount --;
    if (sem->refcount == 0)
    {
        /* delete from posix semaphore list */
        if (sem->unlinked)
            posix_sem_delete(sem);
        sem = OS_NULL;
    }
    osSemaphoreRelease(&posix_sem_lock);

    return 0;
}
RTM_EXPORT(sem_close);

int sem_destroy(sem_t *sem)
{
    osStatus_t result;

    if ((!sem) || !(sem->unamed))
    {
        osSetErrno(EINVAL);

        return -1;
    }

    /* lock posix semaphore list */
    osSemaphoreAcquire(&posix_sem_lock, osWaitForever);
    result = osSemaphoreAcquire(sem->sem, 0);
    if (result != osOK)
    {
        osSemaphoreRelease(&posix_sem_lock);
        osSetErrno(EBUSY);

        return -1;
    }

    /* destroy an unamed posix semaphore */
    posix_sem_delete(sem);
    osSemaphoreRelease(&posix_sem_lock);

    return 0;
}
RTM_EXPORT(sem_destroy);

int sem_unlink(const char *name)
{
    sem_t *psem;

    /* lock posix semaphore list */
    osSemaphoreAcquire(&posix_sem_lock, osWaitForever);
    psem = posix_sem_find(name);
    if (psem != OS_NULL)
    {
        psem->unlinked = 1;
        if (psem->refcount == 0)
        {
            /* remove this semaphore */
            posix_sem_delete(psem);
        }
        osSemaphoreRelease(&posix_sem_lock);

        return 0;
    }
    osSemaphoreRelease(&posix_sem_lock);

    /* no this entry */
    osSetErrno(ENOENT);

    return -1;
}
RTM_EXPORT(sem_unlink);

int sem_getvalue(sem_t *sem, int *sval)
{
    if (!sem || !sval)
    {
        osSetErrno(EINVAL);

        return -1;
    }
    *sval = sem->sem->value;

    return 0;
}
RTM_EXPORT(sem_getvalue);

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    char *name;
    static uint16_t psem_number = 0;
    osSemaphoreAttr_t attr = {OS_NULL, 0U, OS_NULL, 0U};

    if (sem == OS_NULL)
    {
        osSetErrno(EINVAL);

        return -1;
    }

    name = (char *)osMalloc(OS_NAME_MAX);
    if(name == OS_NULL)
    {
        osSetErrno(ENOMEM);
        return -1;
    }
    osSnprintf(name, OS_NAME_MAX, "psem%02d", psem_number++);
    attr.name = name;
    sem->sem = osSemaphoreNew(OS_SEM_VALUE_MAX, value, &attr);
    if (sem == OS_NULL)
    {
        osSetErrno(ENOMEM);

        return -1;
    }
    /* 清除prio，设置为fifo */
    OSOBJ_ATTR(sem->sem) &= ~OS_IPC_FLAG_PRIO;

    // osPrintf("sem_init name:%s, attr:%x\r\n", sem->sem->name, OSOBJ_ATTR(sem->sem));

    /* initialize posix semaphore */
    sem->refcount = 1;
    sem->unlinked = 0;
    sem->unamed = 1;
    /* lock posix semaphore list */
    osSemaphoreAcquire(&posix_sem_lock, osWaitForever);
    posix_sem_insert(sem);
    osSemaphoreRelease(&posix_sem_lock);

    return 0;
}
RTM_EXPORT(sem_init);

sem_t *sem_open(const char *name, int oflag, ...)
{
    sem_t* sem;
    va_list arg;
    mode_t mode;
    unsigned int value;

    sem = OS_NULL;
    osSemaphoreAttr_t attr = {OS_NULL, 0U, OS_NULL, 0U};

    /* lock posix semaphore list */
    osSemaphoreAcquire(&posix_sem_lock, osWaitForever);
    if (oflag & O_CREAT)
    {
        va_start(arg, oflag);
        mode = (mode_t) va_arg( arg, unsigned int); mode = mode;
        value = va_arg( arg, unsigned int);
        va_end(arg);

        if (oflag & O_EXCL)
        {
            if (posix_sem_find(name) != OS_NULL)
            {
                osSetErrno(EEXIST);
                goto __return;
            }
        }
        sem = (sem_t*) osMalloc (sizeof(struct posix_sem));
        if (sem == OS_NULL)
        {
            osSetErrno(ENFILE);
            goto __return;
        }

        if(name)
        {
            attr.name = (char *)osMalloc(OS_NAME_MAX);
            if (attr.name == OS_NULL)
            {
                osSetErrno(ENOMEM);
                goto __return;
            }
            strncpy((char *)attr.name, name, OS_NAME_MAX);
            ((char *)attr.name)[OS_NAME_MAX-1] = '\0';
        }

        /* create RT-Thread semaphore */
        sem->sem = osSemaphoreNew(OS_SEM_VALUE_MAX, value, &attr);
        if (sem->sem == OS_NULL) /* create failed */
        {
            osSetErrno(ENFILE);
            goto __return;
        }
        /* 清除prio，设置为fifo */
        OSOBJ_ATTR(sem->sem) &= ~OS_IPC_FLAG_PRIO;

        // osPrintf("sem_open name:%s, attr:%x\r\n", sem->sem->name, OSOBJ_ATTR(sem->sem));

        /* initialize reference count */
        sem->refcount = 1;
        sem->unlinked = 0;
        sem->unamed = 0;

        /* insert semaphore to posix semaphore list */
        posix_sem_insert(sem);
    }
    else
    {
        /* find semaphore */
        sem = posix_sem_find(name);
        if (sem != OS_NULL)
        {
            sem->refcount ++; /* increase reference count */
        }
        else
        {
            osSetErrno(ENOENT);
            goto __return;
        }
    }
    osSemaphoreRelease(&posix_sem_lock);

    return sem;

__return:
    /* release lock */
    osSemaphoreRelease(&posix_sem_lock);

    /* release allocated memory */
    if (sem != OS_NULL)
    {
        /* delete RT-Thread semaphore */
        if (sem->sem != OS_NULL)
            osSemaphoreDelete(sem->sem);
        osFree(sem);
    }

    if(attr.name != OS_NULL)
    {
        osFree((void *)attr.name);
    }

    return OS_NULL;
}
RTM_EXPORT(sem_open);

int sem_post(sem_t *sem)
{
    osStatus_t result;

    if (!sem)
    {
        osSetErrno(EINVAL);

        return -1;
    }

    result = osSemaphoreRelease(sem->sem);
    if (result == osOK)
        return 0;

    osSetErrno(EINVAL);

    return -1;
}
RTM_EXPORT(sem_post);

int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
{
    osStatus_t result;
    uint32_t timeout;

    if (!sem || !abs_timeout)
        return EINVAL;

    /* calculate os tick */
    timeout = timespec_diff_ms(abs_timeout);

    result = osSemaphoreAcquire(sem->sem, timeout);
    if (result == osErrorTimeout)
    {
        osSetErrno(ETIMEDOUT);

        return -1;
    }
    if (result == osOK)
        return 0;

    osSetErrno(EINTR);

    return -1;
}
RTM_EXPORT(sem_timedwait);

int sem_trywait(sem_t *sem)
{
    osStatus_t result;

    if (!sem)
    {
        osSetErrno(EINVAL);

        return -1;
    }

    result = osSemaphoreAcquire(sem->sem, 0);
    if (result == osErrorResource)
    {
        osSetErrno(EAGAIN);

        return -1;
    }
    if (result == osOK)
        return 0;

    osSetErrno(EINTR);

    return -1;
}
RTM_EXPORT(sem_trywait);

int sem_wait(sem_t *sem)
{
    osStatus_t result;

    if (!sem)
    {
        osSetErrno(EINVAL);

        return -1;
    }

    result = osSemaphoreAcquire(sem->sem, osWaitForever);
    if (result == osOK)
        return 0;

    osSetErrno(EINTR);

    return -1;
}
RTM_EXPORT(sem_wait);

