/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <os.h>
#include "mqueue.h"
#include "pthread_internal.h"

static mqd_t posix_mq_list = OS_NULL;
static struct osSemaphore posix_mq_lock;

/* 内部消息结构 */
struct mq_msg_internal {
    size_t len;           /* 实际消息长度 */
    char data[];          /* 柔性数组 */
};

void posix_mq_system_init()
{
    osSemaphoreInit(&posix_mq_lock, 1, OS_SEM_VALUE_MAX, OS_IPC_FLAG_FIFO);
}


OS_INLINE void posix_mq_insert(mqd_t pmq)
{
    pmq->next = posix_mq_list;
    posix_mq_list = pmq;
}

static void posix_mq_delete(mqd_t pmq)
{
    mqd_t iter;
    if (posix_mq_list == pmq)
    {
        posix_mq_list = pmq->next;

        if(pmq->mq && pmq->mq->name)
        {
            osFree((void *)pmq->mq->name);
            pmq->mq->name = OS_NULL;
        }
        osMessageQueueDelete(pmq->mq);
        osFree(pmq);

        return;
    }
    for (iter = posix_mq_list; iter->next != OS_NULL; iter = iter->next)
    {
        if (iter->next == pmq)
        {
            /* delete this mq */
            if (pmq->next != OS_NULL)
                iter->next = pmq->next;
            else
                iter->next = OS_NULL;

            if(pmq->mq && pmq->mq->name)
            {
                osFree((void *)pmq->mq->name);
                pmq->mq->name = OS_NULL;
            }

            /* delete RT-Thread mqueue */
            osMessageQueueDelete(pmq->mq);

            osFree(pmq);

            return ;
        }
    }
}

static mqd_t posix_mq_find(const char* name)
{
    mqd_t iter;
    osMq_t mq;

    for (iter = posix_mq_list; iter != OS_NULL; iter = iter->next)
    {
        mq = (osMq_t)(iter->mq);

        if (strncmp(mq->name, name, OS_NAME_MAX) == 0)
        {
            return iter;
        }
    }

    return OS_NULL;
}

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
    if ((mqdes == OS_NULL) || mqstat == OS_NULL)
    {
        osSetErrno(EBADF);

        return -1;
    }

    mqstat->mq_maxmsg = osMessageQueueGetCapacity(mqdes->mq);
    mqstat->mq_msgsize = mqdes->max_msgsize;
    mqstat->mq_curmsgs = osMessageQueueGetCount(mqdes->mq);
    mqstat->mq_flags = 0;

    return 0;
}
RTM_EXPORT(mq_getattr);

mqd_t mq_open(const char *name, int oflag, ...)
{
    mqd_t mqdes;
    va_list arg;
    mode_t mode;
    struct mq_attr *attr = OS_NULL;
    osMessageQueueAttr_t msgQueueAttr = {OS_NULL, 0U, OS_NULL, 0U, OS_NULL, 0U};

    /* lock posix mqueue list */
    osSemaphoreAcquire(&posix_mq_lock, osWaitForever);

    mqdes = OS_NULL;
    if (oflag & O_CREAT)
    {
        va_start(arg, oflag);
        mode = (mode_t)va_arg(arg, unsigned int);
        mode = mode;
        attr = (struct mq_attr *)va_arg(arg, struct mq_attr *);
        va_end(arg);

        if (oflag & O_EXCL)
        {
            if (posix_mq_find(name) != OS_NULL)
            {
                osSetErrno(EEXIST);
                goto __return;
            }
        }
        mqdes = (mqd_t) osMalloc (sizeof(struct mqdes));
        if (mqdes == OS_NULL)
        {
            osSetErrno(ENFILE);
            goto __return;
        }

        mqdes->max_msgsize = attr->mq_msgsize;

        /* create RT-Thread message queue */

        if(name)
        {
            msgQueueAttr.name = (char *)osMalloc(OS_NAME_MAX);
            if (msgQueueAttr.name == OS_NULL)
            {
                osSetErrno(ENOMEM);
                goto __return;
            }
            osStrncpy((char *)msgQueueAttr.name, name, OS_NAME_MAX);
            ((char *)msgQueueAttr.name)[OS_NAME_MAX-1] = '\0';
        }
        mqdes->mq = osMessageQueueNew(attr->mq_maxmsg, sizeof(void *), &msgQueueAttr);
        if (mqdes->mq == OS_NULL) /* create failed */
        {
            osSetErrno(ENFILE);
            goto __return;
        }
        /* 清除prio，设置为fifo */
        OSOBJ_ATTR(mqdes->mq) &= ~OS_IPC_FLAG_PRIO;

        /* initialize reference count */
        mqdes->refcount = 1;
        mqdes->unlinked = 0;

        /* insert mq to posix mq list */
        posix_mq_insert(mqdes);
    }
    else
    {
        /* find mqueue */
        mqdes = posix_mq_find(name);
        if (mqdes != OS_NULL)
        {
            mqdes->refcount ++; /* increase reference count */
        }
        else
        {
            osSetErrno(ENOENT);
            goto __return;
        }
    }
    osSemaphoreRelease(&posix_mq_lock);

    return mqdes;

__return:
    /* release lock */
    osSemaphoreRelease(&posix_mq_lock);

    /* release allocated memory */
    if (mqdes != OS_NULL)
    {
        if (mqdes->mq != OS_NULL)
        {
            /* delete RT-Thread message queue */
            osMessageQueueDelete(mqdes->mq);
        }
        osFree(mqdes);
    }

    if(msgQueueAttr.name != OS_NULL)
    {
        osFree((void *)msgQueueAttr.name);
    }
    return OS_NULL;
}
RTM_EXPORT(mq_open);

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio)
{
    osStatus_t result;
    struct mq_msg_internal *msg;
    size_t actual_len;

    if ((mqdes == OS_NULL) || (msg_ptr == OS_NULL))
    {
        osSetErrno(EINVAL);
        return -1;
    }

    result = osMessageQueueGet(mqdes->mq, &msg, 0, osWaitForever);
    if (result != osOK)
    {
        osSetErrno(EBADF);
        return -1;
    }

    actual_len = msg_len > msg->len ? msg->len : msg_len;
    osMemcpy(msg_ptr, msg->data, actual_len);
    osFree(msg);

    return actual_len;
}
RTM_EXPORT(mq_receive);

int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio)
{
    osStatus_t result;
    struct mq_msg_internal *msg;

    if ((mqdes == OS_NULL) || (msg_ptr == OS_NULL))
    {
        osSetErrno(EINVAL);
        return -1;
    }

    /* 检查消息长度是否超过限制 */
    if (msg_len > mqdes->max_msgsize)
    {
        osSetErrno(EMSGSIZE);
        return -1;
    }

    /* 分配消息内存：头部 + 数据 */
    msg = (struct mq_msg_internal *)osMalloc(sizeof(struct mq_msg_internal) + msg_len);
    if (msg == OS_NULL) {
        osSetErrno(ENOMEM);
        return -1;
    }

    msg->len = msg_len;
    osMemcpy(msg->data, msg_ptr, msg_len);

    result = osMessageQueuePut(mqdes->mq, &msg, 0, 0);
    if (result == osOK)
        return 0;

    osSetErrno(EBADF);
    osFree(msg);

    return -1;
}
RTM_EXPORT(mq_send);

ssize_t mq_timedreceive(mqd_t                  mqdes,
                        char                  *msg_ptr,
                        size_t                 msg_len,
                        unsigned              *msg_prio,
                        const struct timespec *abs_timeout)
{
    uint32_t timeout;
    osStatus_t result;
    size_t actual_len;
    struct mq_msg_internal *msg;

    /* parameters check */
    if ((mqdes == OS_NULL) || (msg_ptr == OS_NULL) || (abs_timeout == OS_NULL))
    {
        osSetErrno(EINVAL);
        return -1;
    }

    timeout = timespec_diff_ms(abs_timeout);

    result = osMessageQueueGet(mqdes->mq, &msg, 0, timeout);
    if (result == osOK)
    {
        actual_len = msg_len > msg->len ? msg->len : msg_len;
        osMemcpy(msg_ptr, msg->data, actual_len);
        osFree(msg);
        
        return actual_len;
    }
    else if(result == osErrorTimeout)
    {
        osSetErrno(ETIMEDOUT);
        return -1;
    }

    osSetErrno(EBADMSG);
    return -1;
}
RTM_EXPORT(mq_timedreceive);

int mq_timedsend(mqd_t                  mqdes,
                 const char            *msg_ptr,
                 size_t                 msg_len,
                 unsigned               msg_prio,
                 const struct timespec *abs_timeout)
{
    uint32_t timeout;
    osStatus_t result;
    struct mq_msg_internal *msg;

    if (mqdes == OS_NULL || msg_ptr == OS_NULL || abs_timeout == OS_NULL)
    {
        osSetErrno(EBADF);
        return -1;
    }

    /* 检查消息长度是否超过限制 */
    if (msg_len > mqdes->max_msgsize)
    {
        osSetErrno(EMSGSIZE);
        return -1;
    }

    timeout = timespec_diff_ms(abs_timeout);

    /* 分配消息内存：头部 + 数据 */
    msg = (struct mq_msg_internal *)osMalloc(sizeof(struct mq_msg_internal) + msg_len);
    if (msg == OS_NULL) {
        osSetErrno(ENOMEM);
        return -1;
    }

    msg->len = msg_len;
    osMemcpy(msg->data, msg_ptr, msg_len);

    /* 发送消息指针到队列 */
    result = osMessageQueuePut(mqdes->mq, &msg, 0, timeout);
    if (result == osOK)
    {
        return 0;
    }
    else if(result == osErrorTimeout)
    {
        osSetErrno(ETIMEDOUT);
        osFree(msg);
        return -1;
    }

    osSetErrno(EBADF);
    osFree(msg);

    return -1;
}
RTM_EXPORT(mq_timedsend);

int mq_close(mqd_t mqdes)
{
    if (mqdes == OS_NULL)
    {
        osSetErrno(EINVAL);
        return -1;
    }

    /* lock posix mqueue list */
    osSemaphoreAcquire(&posix_mq_lock, osWaitForever);
    mqdes->refcount --;
    if (mqdes->refcount == 0)
    {
        /* delete from posix mqueue list */
        if (mqdes->unlinked)
            posix_mq_delete(mqdes);
    }
    osSemaphoreRelease(&posix_mq_lock);

    return 0;
}
RTM_EXPORT(mq_close);

int mq_unlink(const char *name)
{
    mqd_t pmq;

    /* lock posix mqueue list */
    osSemaphoreAcquire(&posix_mq_lock, osWaitForever);
    pmq = posix_mq_find(name);
    if (pmq != OS_NULL)
    {
        pmq->unlinked = 1;
        if (pmq->refcount == 0)
        {
            /* remove this mqueue */
            posix_mq_delete(pmq);
        }
        osSemaphoreRelease(&posix_mq_lock);

        return 0;
    }
    osSemaphoreRelease(&posix_mq_lock);

    /* no this entry */
    osSetErrno(ENOENT);

    return -1;
}
RTM_EXPORT(mq_unlink);
