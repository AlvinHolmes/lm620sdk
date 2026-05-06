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
#include <os.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>

#define DEFAULT_STACK_SIZE  2048
#define DEFAULT_PRIORITY    (OS_THREAD_PRIORITY_MAX/2 + OS_THREAD_PRIORITY_MAX/4)

const pthread_attr_t pthread_default_attr =
{
    1,
    0,                          /* stack base */
    DEFAULT_STACK_SIZE,         /* stack size */
    0,
    PTHREAD_INHERIT_SCHED,      /* Inherit parent prio/policy */
    SCHED_FIFO,                 /* scheduler policy */
    {
        DEFAULT_PRIORITY,       /* scheduler priority */
    },
    PTHREAD_CREATE_JOINABLE,    /* detach state */
};

int pthread_attr_init(pthread_attr_t *attr)
{
    OS_ASSERT(attr != OS_NULL);

    *attr = pthread_default_attr;

    return 0;
}
RTM_EXPORT(pthread_attr_init);

int pthread_attr_destroy(pthread_attr_t *attr)
{
    OS_ASSERT(attr != OS_NULL);

    memset(attr, 0, sizeof(pthread_attr_t));

    return 0;
}
RTM_EXPORT(pthread_attr_destroy);

int pthread_attr_setdetachstate(pthread_attr_t *attr, int state)
{
    OS_ASSERT(attr != OS_NULL);

    if (state != PTHREAD_CREATE_JOINABLE && state != PTHREAD_CREATE_DETACHED)
        return EINVAL;

    attr->detachstate = state;

    return 0;
}
RTM_EXPORT(pthread_attr_setdetachstate);

int pthread_attr_getdetachstate(pthread_attr_t const *attr, int *state)
{
    OS_ASSERT(attr != OS_NULL);

    *state = (int)attr->detachstate;

    return 0;
}
RTM_EXPORT(pthread_attr_getdetachstate);

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    OS_ASSERT(attr != OS_NULL);

    attr->schedpolicy = policy;

    return 0;
}
RTM_EXPORT(pthread_attr_setschedpolicy);

int pthread_attr_getschedpolicy(pthread_attr_t const *attr, int *policy)
{
    OS_ASSERT(attr != OS_NULL);

    *policy = (int)attr->schedpolicy;

    return 0;
}
RTM_EXPORT(pthread_attr_getschedpolicy);

int pthread_attr_setschedparam(pthread_attr_t           *attr,
                               struct sched_param const *param)
{
    OS_ASSERT(attr != OS_NULL);
    OS_ASSERT(param != OS_NULL);

    attr->schedparam.sched_priority = param->sched_priority;

    return 0;
}
RTM_EXPORT(pthread_attr_setschedparam);

int pthread_attr_getschedparam(pthread_attr_t const *attr,
                               struct sched_param   *param)
{
    OS_ASSERT(attr != OS_NULL);
    OS_ASSERT(param != OS_NULL);

    param->sched_priority = attr->schedparam.sched_priority;

    return 0;
}
RTM_EXPORT(pthread_attr_getschedparam);

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stack_size)
{
    OS_ASSERT(attr != OS_NULL);

    attr->stacksize = stack_size;

    return 0;
}
RTM_EXPORT(pthread_attr_setstacksize);

int pthread_attr_getstacksize(pthread_attr_t const *attr, size_t *stack_size)
{
    OS_ASSERT(attr != OS_NULL);

    *stack_size = attr->stacksize;

    return 0;
}
RTM_EXPORT(pthread_attr_getstacksize);

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stack_addr)
{
    OS_ASSERT(attr != OS_NULL);

    return EOPNOTSUPP;
}
RTM_EXPORT(pthread_attr_setstackaddr);

int pthread_attr_getstackaddr(pthread_attr_t const *attr, void **stack_addr)
{
    OS_ASSERT(attr != OS_NULL);

    return EOPNOTSUPP;
}
RTM_EXPORT(pthread_attr_getstackaddr);

int pthread_attr_setstack(pthread_attr_t *attr,
                          void           *stack_base,
                          size_t          stack_size)
{
    OS_ASSERT(attr != OS_NULL);

    attr->stackaddr = stack_base;
    attr->stacksize = OS_ALIGN_DOWN(stack_size, OS_ALIGN_SIZE);

    return 0;
}
RTM_EXPORT(pthread_attr_setstack);

int pthread_attr_getstack(pthread_attr_t const *attr,
                          void                **stack_base,
                          size_t               *stack_size)
{
    OS_ASSERT(attr != OS_NULL);

    *stack_base = attr->stackaddr;
    *stack_size = attr->stacksize;

    return 0;
}
RTM_EXPORT(pthread_attr_getstack);