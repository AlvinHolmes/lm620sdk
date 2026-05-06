/**
 * @file example_main.c
 * @brief POSIX 定时器功能测试示例
 * @version 0.1
 * @date 2025-01-19
 *
 * Copyright (c) 2025 南京创芯慧联技术有限公司. All rights reserved.
 *
 */

#include <os.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "nr_micro_shell.h"

static int timer_count = 0;

static void timer_call_back(union sigval sv)
{
    osPrintf("[Timer]: timer is expired, say hello!\r\n");
    osPrintf("[Timer]: param sv.sival_int = %d.\r\n", sv.sival_int);
    timer_count++;
}


static int timer_main(void)
{
    struct sigevent evp;
    int retcode;
    struct itimerspec its;
    timer_t timer_id;

    osPrintf("===============================================\r\n");
    osPrintf("POSIX Timer Test Start\r\n");

    memset(&evp, 0, sizeof(struct sigevent));
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = timer_call_back;
    evp.sigev_value.sival_int = 1234;

    retcode = timer_create(CLOCK_REALTIME, &evp, &timer_id);
    if (retcode == 0) {
        osPrintf("[Timer]: create timer = %ld ok.\r\n", (long)timer_id);
    } else {
        osPrintf("[Timer]: create timer failed!\r\n");
        return -1;
    }

    memset(&its, 0, sizeof(struct itimerspec));
    its.it_value.tv_sec = 5;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 2;
    its.it_interval.tv_nsec = 0;

    retcode = timer_settime(timer_id, 0, &its, NULL);
    if (retcode == 0) {
        osPrintf("[Timer]: set timer ok!\r\n");
    } else {
        osPrintf("[Timer]: set timer failed!\r\n");
        return -1;
    }

    sleep(2);

    retcode = timer_gettime(timer_id, &its);
    if (retcode == 0) {
        osPrintf("[Timer]: get timer info val_sec = %lld, interval = %lld\r\n",
                         (long long)its.it_value.tv_sec, (long long)its.it_interval.tv_sec);
    }

    sleep(30);

    retcode = timer_delete(timer_id);
    osPrintf("[Timer]: delete the timer, test end. timer_count = %d\r\n", timer_count);

    osPrintf("===============================================\r\n");
    return 0;
}

static void demo_timer_all(char argc, char **argv)
{
    int ret = 0;

    ret = timer_main();

    if (ret == 0) {
        osPrintf("\n=== Demo Timer Test PASSED ===\r\n");
    } else {
        osPrintf("\n=== Demo Timer Test FAILED ===\r\n");
    }

    return;	
}

NR_SHELL_CMD_EXPORT(posix_timer, demo_timer_all);

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
