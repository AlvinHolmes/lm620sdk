/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-10-26     Bernard      the first version
 */

#ifndef __PTHREAD_INTERNAL_H__
#define __PTHREAD_INTERNAL_H__

#include <os.h>
#include <pthread.h>
#include <time.h>

struct _pthread_mutex
{
    pthread_mutexattr_t attr;
    struct osMutex lock;
};
typedef struct _pthread_mutex _pthread_mutex_t;

struct _pthread_cond
{
    pthread_condattr_t attr;
    struct osSemaphore sem;
};
typedef struct _pthread_cond _pthread_cond_t;


struct _pthread_cleanup
{
    void (*cleanup_func)(void *parameter);
    void *parameter;

    struct _pthread_cleanup *next;
};
typedef struct _pthread_cleanup _pthread_cleanup_t;

struct _pthread_key_data
{
    int is_used;
    void (*destructor)(void *parameter);
};
typedef struct _pthread_key_data _pthread_key_data_t;

#ifndef PTHREAD_NUM_MAX
#define PTHREAD_NUM_MAX 32
#endif

#define PTHREAD_MAGIC   0x70746873
struct _pthread_data
{
    uint32_t magic;
    pthread_attr_t attr;
    osThread_t tid;

    void* (*thread_entry)(void *parameter);
    void *thread_parameter;

    /* return value */
    void *return_value;

    /* semaphore for joinable thread */
    // osSem_t joinable_sem;  /* not necessary */

    /* cancel state and type */
    uint8_t cancelstate;
    volatile uint8_t canceltype;
    volatile uint8_t canceled;

    _pthread_cleanup_t *cleanup;
    void** tls; /* thread-local storage area */
};
typedef struct _pthread_data _pthread_data_t;

_pthread_data_t *_pthread_get_data(pthread_t thread);

void posix_mq_system_init(void);
void posix_sem_system_init(void);

extern int clock_gettime(clockid_t clock_id, struct timespec *tp);

/* 计算 timespec 的差值（ms） */
static inline uint32_t timespec_diff_ms(const struct timespec *abstime)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    int64_t diff_sec = abstime->tv_sec - now.tv_sec;
    int64_t diff_nsec = abstime->tv_nsec - now.tv_nsec;

    if (diff_sec < 0) {
        return 0;
    }
    if ((diff_sec == 0) && (diff_nsec < 0)) {
        return 0;
    }

    return (uint32_t)(diff_sec * 1000 + diff_nsec / 1000000);
}

#endif
