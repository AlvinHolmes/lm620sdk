#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "os.h"
#include "drv_rtc.h"


int clock_getres(clockid_t clock_id, struct timespec *res)
{
    if (res == OS_NULL) {
        osSetErrno(EINVAL);
        return -1;
    }
    switch (clock_id) {
    case CLOCK_REALTIME:
        res->tv_sec = 0;
        res->tv_nsec = NSEC_PER_SEC / OS_TICK_PER_SECOND;
        break;
    default:
        osSetErrno(EINVAL);
        return -1;
    }
    return 0;
}

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    struct timeval tv;

    if (tp == OS_NULL) {
        osSetErrno(EINVAL);
        return -1;
    }
    if (clock_id != CLOCK_REALTIME) {
        osSetErrno(EINVAL);
        return -1;
    }
    gettimeofday(&tv, NULL);
    tp->tv_sec = tv.tv_sec;
    tp->tv_nsec = tv.tv_usec * NSEC_PER_USEC;
    return 0;
}


int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
    RTC_Time rtcTime;
    int8_t zone = 0;

    if (tp == OS_NULL || tp->tv_sec < 0 || tp->tv_nsec < 0 || tp->tv_nsec > NSEC_PER_SEC ) {
        osSetErrno(EINVAL);
        return -1;
    }
    if (clock_id != CLOCK_REALTIME) {
        osSetErrno(EPERM);
        return -1;
    }

    RTC_GetZone(&zone);
    /* In this system, we consistently set RTC to local time */
    rtcTime = tp->tv_sec + (zone * 15 * 60);
    rtcTime = rtcTime < 0 ? 0 : rtcTime;

    RTC_SetTime(rtcTime);

    return 0;
}

