#include <os.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h> 
#include <mqueue.h>

#include "nr_micro_shell.h"

extern int clock_gettime(clockid_t clock_id, struct timespec *tp);

/* ============================================
 * demo_thread: 基本线程创建和测试
 * ============================================ */

static void *app_main(void *argv)
{
    int count = 0;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    while (1) {
        osPrintf("[App]: Hello OpenCPU, count=%d\r\n", count++);

        pthread_testcancel();

        usleep(200 * 1000);

        if (count >= 10) {
            osPrintf("[App]: Test finished, exiting...\r\n");
            break;
        }
    }

    return NULL;
}

static int demo_thread(void)
{
    pthread_t hello_ptid;
    int ret;

    osPrintf("[App]: Demo App Start\r\n");

    pthread_attr_t attr;
    struct sched_param param;
    param.sched_priority = 20;
    pthread_attr_init(&attr);
    pthread_attr_setschedparam(&attr, &param);

    ret = pthread_create(&hello_ptid, &attr, app_main, NULL);
    osPrintf("[App]: pthread_create returned: %d\r\n", ret);
    if (ret != 0) {
        osPrintf("[App]: pthread_create failed: %d\r\n", ret);
        return -1;
    }

    osPrintf("[App]: Waiting 1500ms before cancel...\r\n");
    usleep(1500 * 1000);

    osPrintf("[App]: Calling pthread_cancel...\r\n");
    ret = pthread_cancel(hello_ptid);
    osPrintf("[App]: pthread_cancel returned: %d\r\n", ret);

    osPrintf("[App]: Calling pthread_join...\r\n");
    ret = pthread_join(hello_ptid, NULL);
    osPrintf("[App]: pthread_join returned: %d\r\n", ret);

    osPrintf("[App]: Demo App End\r\n");

    return ret;
}

/* ============================================
 * demo_msgqueue: 消息队列测试
 * ============================================ */

#define MQ_NAME "/test_mq"
#define MSG_SIZE 128
#define MAX_MSG 10

static void *mq_sender(void *arg)
{
    mqd_t mq;
    char msg[MSG_SIZE];
    int count = 0;
    int ret;

    mq = mq_open(MQ_NAME, O_WRONLY);
    if (mq == NULL) {
        osPrintf("[MQ]: Sender mq_open failed, errno:%d\r\n", osGetErrno());
        return NULL;
    }

    osPrintf("[MQ]: Sender started\r\n");

    for (count = 0; count < 10; count++) {
        snprintf(msg, MSG_SIZE, "Message %d from sender", count);
        ret = mq_send(mq, msg, strlen(msg) + 1, 0);
        if (ret < 0) {
            osPrintf("[MQ]: mq_send failed, errno:%d\r\n", osGetErrno());
        } else {
            osPrintf("[MQ]: Sent: %s\r\n", msg);
        }
        usleep(100 * 1000);
    }

    struct timespec timeout;
    for (; count < 20; count++) {
        snprintf(msg, MSG_SIZE, "Message %d from sender", count);

        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 1;
        ret = mq_timedsend(mq, msg, strlen(msg) + 1, 0, &timeout);
        if (ret < 0) {
            osPrintf("[MQ]: mq_timedsend %d failed, errno:%d\r\n", count, osGetErrno());
        } else {
            osPrintf("[MQ]: Sent: %s\r\n", msg);
        }
        usleep(500 * 1000);
    }

    mq_close(mq);

    osPrintf("[MQ]: Sender finished\r\n");

    pthread_exit(NULL);

    return NULL;
}

static void *mq_receiver(void *arg)
{
    mqd_t mq;
    char msg[MSG_SIZE];
    unsigned int prio;
    int ret;
    int count = 0;

    mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == NULL) {
        osPrintf("[MQ]: Receiver mq_open failed, errno:%d\r\n", osGetErrno());
        return NULL;
    }

    osPrintf("[MQ]: Receiver started\r\n");
    sleep(5);

    while (count < 10) {
        ret = mq_receive(mq, msg, MSG_SIZE, &prio);
        if (ret < 0) {
            osPrintf("[MQ]: mq_receive failed, errno:%d\r\n", osGetErrno());
            break;
        }
        osPrintf("[MQ]: Received: %s\r\n", msg);
        count++;
    }

    struct timespec timeout;
    while (count < 25) {
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 1;
        ret = mq_timedreceive(mq, msg, MSG_SIZE, &prio, &timeout);
        if (ret < 0) {
            osPrintf("[MQ]: mq_timedreceive failed, errno:%d\r\n", osGetErrno());
        } else {
            osPrintf("[MQ]: Received: %s\r\n", msg);
        }

        count++;
    }

    mq_close(mq);
    mq_unlink(MQ_NAME);

    osPrintf("[MQ]: Receiver finished\r\n");

    pthread_exit(NULL);

    return NULL;
}

static int demo_msgqueue(void)
{
    pthread_t sender_tid, receiver_tid;
    struct mq_attr attr;
    mqd_t mq;
    int ret;

    osPrintf("[MQ]: Demo MessageQueue Start\r\n");

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_curmsgs = 0;

    mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0, &attr);
    if (mq == NULL) {
        osPrintf("[MQ]: mq_open failed\r\n");
        return -1;
    }
    mq_close(mq);

    ret = pthread_create(&sender_tid, NULL, mq_sender, NULL);
    if (ret != 0) {
        osPrintf("[MQ]: pthread_create sender failed: %d\r\n", ret);
        return -1;
    }

    ret = pthread_create(&receiver_tid, NULL, mq_receiver, NULL);
    if (ret != 0) {
        osPrintf("[MQ]: pthread_create receiver failed: %d\r\n", ret);
        return -1;
    }

    pthread_join(sender_tid, NULL);
    pthread_join(receiver_tid, NULL);

    osPrintf("[MQ]: Demo MessageQueue End\r\n");

    return 0;
}

/* ============================================
 * demo_mutex: 互斥锁测试
 * ============================================ */

static pthread_mutex_t mutex;
static int shared_counter = 0;

static void *mutex_task(void *arg)
{
    int id = (int)arg;
    int i;
    int ret;

    osPrintf("[Mutex]: Task %d started\r\n", id);

    for (i = 0; i < 5; i++) {
        if(id == 1)
            pthread_mutex_lock(&mutex);
        else
        {
            struct timespec abs_time;
            clock_gettime(CLOCK_REALTIME, &abs_time);
            abs_time.tv_nsec += 50 * 1000 * 1000;
            if((ret = pthread_mutex_timedlock(&mutex, &abs_time)) == 0)
            {
                pthread_mutex_unlock(&mutex);
            }
            pthread_mutex_lock(&mutex);
            osPrintf("[Mutex]: Task %d: timedlock return %d\r\n", id, ret);
        }

        osPrintf("[Mutex]: Task %d: counter=%d\r\n", id, shared_counter);
        shared_counter++;
        usleep(100 * 1000);
        shared_counter--;
        osPrintf("[Mutex]: Task %d: counter=%d\r\n", id, shared_counter);

        pthread_mutex_unlock(&mutex);

        usleep(200 * 1000);
    }

    osPrintf("[Mutex]: Task %d finished\r\n", id);

    return NULL;
}

static int demo_mutex(void)
{
    pthread_t task1_tid, task2_tid;
    pthread_mutexattr_t mutex_attr;
    int ret;

    osPrintf("[Mutex]: Demo Mutex Start\r\n");

    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);

    ret = pthread_mutex_init(&mutex, &mutex_attr);
    if (ret != 0) {
        osPrintf("[Mutex]: pthread_mutex_init failed: %d\r\n", ret);
        return -1;
    }

    pthread_mutexattr_destroy(&mutex_attr);

    ret = pthread_create(&task1_tid, NULL, mutex_task, (void *)1);
    if (ret != 0) {
        osPrintf("[Mutex]: pthread_create task1 failed: %d\r\n", ret);
        pthread_mutex_destroy(&mutex);
        return -1;
    }

    ret = pthread_create(&task2_tid, NULL, mutex_task, (void *)2);
    if (ret != 0) {
        osPrintf("[Mutex]: pthread_create task2 failed: %d\r\n", ret);
        pthread_mutex_destroy(&mutex);
        return -1;
    }

    pthread_join(task1_tid, NULL);
    pthread_join(task2_tid, NULL);

    pthread_mutex_destroy(&mutex);

    osPrintf("[Mutex]: Demo Mutex End, final counter=%d\r\n", shared_counter);

    return 0;
}

/* ============================================
 * demo_sem: 信号量测试
 * ============================================ */

static sem_t sem;
static volatile int sem_test_done = 0;

static void *sem_wait_task(void *arg)
{
    int ret;
    struct timespec abs_tim;

    osPrintf("[Sem]: Wait task started\r\n");

    while (!sem_test_done) {
        /* 测试 sem_trywait */
        ret = sem_trywait(&sem);
        if (ret == 0) {
            osPrintf("[Sem]: sem_trywait get a semaphore.\r\n");
        } else {
            if (errno != EAGAIN) {
                osPrintf("[Sem]: sem_trywait ERROR.\r\n");
            }
        }

        /* 测试 sem_timedwait (1 秒超时) */
        clock_gettime(CLOCK_REALTIME, &abs_tim);
        abs_tim.tv_sec += 1;

        ret = sem_timedwait(&sem, &abs_tim);
        if (ret == 0) {
            osPrintf("[Sem]: sem_timedwait get a semaphore.\r\n");
        } else {
            if (errno != ETIMEDOUT) {
                osPrintf("[Sem]: sem_timedwait ERROR.\r\n");
            }
        }

        /* 测试 sem_wait (阻塞) */
        ret = sem_wait(&sem);
        if (ret == 0) {
            osPrintf("[Sem]: sem_wait get a semaphore.\r\n");
        } else {
            osPrintf("[Sem]: sem_wait ERROR %d.\r\n", errno);
            pthread_exit(NULL);
        }
    }

    osPrintf("[Sem]: Wait task finished\r\n");

    return NULL;
}

static void *sem_post_task(void *arg)
{
    int ret;
    int times = 0;
    int times_all = (int)arg;

    osPrintf("[Sem]: Post task started\r\n");

    while (1) {
        sleep(2);
        times++;
        if (times <= times_all) {
            ret = sem_post(&sem);
            if (ret == 0) {
                osPrintf("[Sem]: post a semaphore.\r\n");
            }
        } else {
            osPrintf("[Sem]: all semaphore post ok, test end.\r\n");
            /* 设置结束标志，让 Wait 任务能够退出循环 */
            sem_test_done = 1;
            /* 额外发送一次信号量，让 Wait 任务从 sem_wait 返回 */
            sem_post(&sem);
            pthread_exit(NULL);
        }
    }

    return NULL;
}

static int demo_sem(void)
{
    pthread_t wait_tid, post_tid;
    int value;
    int ret;

    osPrintf("[Sem]: Demo Semaphore Start\r\n");

    /* 初始化信号量，初始值为 3 */
    ret = sem_init(&sem, 0, 3);
    if (ret != 0) {
        osPrintf("[Sem]: sem_init error!!!\r\n");
        return -1;
    }

    /* 验证初始值 */
    ret = sem_getvalue(&sem, &value);
    if (ret == 0) {
        if (value == 3) {
            osPrintf("[Sem]: sem_init ok.\r\n");
        } else {
            osPrintf("[Sem]: sem_getvalue error %d.\r\n", value);
        }
    }

    ret = pthread_create(&wait_tid, NULL, sem_wait_task, NULL);
    if (ret != 0) {
        osPrintf("[Sem]: pthread_create wait task failed: %d\r\n", ret);
        sem_destroy(&sem);
        return -1;
    }

    ret = pthread_create(&post_tid, NULL, sem_post_task, (void *)10);
    if (ret != 0) {
        osPrintf("[Sem]: pthread_create post task failed: %d\r\n", ret);
        sem_destroy(&sem);
        return -1;
    }

    pthread_join(wait_tid, NULL);
    pthread_join(post_tid, NULL);

    sem_destroy(&sem);

    osPrintf("[Sem]: Demo Semaphore End\r\n");

    return 0;
}

/* ============================================
 * demo_cond: 条件变量定时器测试
 * ============================================ */

static pthread_cond_t cond;
static pthread_mutex_t mutex;
static int flag = 0;

static void *cond_wait_task(void *arg)
{
    struct timespec abstime;
    int ret;

    osPrintf("[Cond]: Wait task started\r\n");

    pthread_mutex_lock(&mutex);

    while (flag == 0) {
        osPrintf("[Cond]: Waiting for condition...\r\n");

        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += 5;

        ret = pthread_cond_timedwait(&cond, &mutex, &abstime);
        if (ret == ETIMEDOUT) {
            osPrintf("[Cond]: Wait timeout\r\n");
            break;
        } else if (ret != 0) {
            osPrintf("[Cond]: Wait error: %d\r\n", ret);
            break;
        }

        osPrintf("[Cond]: Condition signaled\r\n");
        break;
    }

    pthread_mutex_unlock(&mutex);

    osPrintf("[Cond]: Wait task finished\r\n");

    return NULL;
}

static void *cond_signal_task(void *arg)
{
    int count = 0;

    osPrintf("[Cond]: Signal task started\r\n");

    while (count < 3) {
        sleep(2);
        count++;

        pthread_mutex_lock(&mutex);

        flag = 1;

        osPrintf("[Cond]: Sending signal, count=%d\r\n", count);

        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    }

    osPrintf("[Cond]: Signal task finished\r\n");

    return NULL;
}

static int demo_cond(void)
{
    pthread_t wait_tid, signal_tid;
    int ret;

    osPrintf("[Cond]: Demo Condition Timer Start\r\n");

    ret = pthread_mutex_init(&mutex, NULL);
    if (ret != 0) {
        osPrintf("[Cond]: pthread_mutex_init failed: %d\r\n", ret);
        return -1;
    }

    ret = pthread_cond_init(&cond, NULL);
    if (ret != 0) {
        osPrintf("[Cond]: pthread_cond_init failed: %d\r\n", ret);
        pthread_mutex_destroy(&mutex);
        return -1;
    }

    ret = pthread_create(&wait_tid, NULL, cond_wait_task, NULL);
    if (ret != 0) {
        osPrintf("[Cond]: pthread_create wait task failed: %d\r\n", ret);
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
        return -1;
    }

    ret = pthread_create(&signal_tid, NULL, cond_signal_task, NULL);
    if (ret != 0) {
        osPrintf("[Cond]: pthread_create signal task failed: %d\r\n", ret);
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
        return -1;
    }

    pthread_join(wait_tid, NULL);
    pthread_join(signal_tid, NULL);

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);

    osPrintf("[Cond]: Demo Condition Timer End\r\n");

    return 0;
}

static void demo_os_all(char argc, char **argv)
{
    int ret = 0;

    ret = demo_thread();
    ret += demo_msgqueue();
    ret += demo_mutex();
    ret += demo_sem();
    ret += demo_cond();

    if (ret == 0) {
        osPrintf("\n=== Demo Test PASSED ===\r\n");
    } else {
        osPrintf("\n=== Demo Test FAILED ===\r\n");
    }

    return;
}


NR_SHELL_CMD_EXPORT(posix_os, demo_os_all);


#if defined(_CPU_AP)
#include "psm_wakelock.h"
WakeLock lock = {0};
int WAKELOCK_LOCK(void)
{
    PSM_WakelockInit(&lock, PSM_DEEP_SLEEP);
    PSM_WakeLock(&lock);
    return 0;
}
INIT_APP_EXPORT(WAKELOCK_LOCK, OS_INIT_SUBLEVEL_LOW);
#endif
