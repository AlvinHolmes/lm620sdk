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


typedef struct {
    osTimerId_t os_timer;
    struct sigevent evp;
    struct itimerspec value;
    clockid_t clockid;
    uint32_t flags;
    int overrun;
} posix_timer_t;

#define POSIX_TIMER_FLAG_ACTIVE     (1 << 0)     /* 定时器已激活 */
#define POSIX_TIMER_FLAG_PERIODIC   (1 << 1)     /* 周期性定时器 */

static void posix_timer_callback(void *arg)
{
    posix_timer_t *timer = (posix_timer_t *)arg;
    osTick_t periodic_tick;

    if(timer == OS_NULL)
        return;

    if((timer->flags & POSIX_TIMER_FLAG_PERIODIC) && 
       (timer->value.it_interval.tv_sec > 0 || timer->value.it_interval.tv_nsec > 0)) {

        if(timer->os_timer == OS_NULL)
            return;
        
        periodic_tick = (uint32_t)osTickFromUs((timer->value.it_interval.tv_sec * USEC_PER_SEC) + (timer->value.it_interval.tv_nsec / NSEC_PER_USEC));

        osTimerStop(timer->os_timer);
        osTimerControl(timer->os_timer, OS_TIMER_CTRL_SET_PERIODIC, NULL);
        osTimerStart(timer->os_timer, periodic_tick);
    }

    switch(timer->evp.sigev_notify) {
        case SIGEV_NONE:
            break;

        case SIGEV_SIGNAL:
            #ifdef OS_USING_SIGNALS
            {
                /* todo */
            }
            #endif
            break;

        case SIGEV_THREAD:
            if(timer->evp.sigev_notify_function != OS_NULL) {
                timer->evp.sigev_notify_function(timer->evp.sigev_value);
            }
            break;

        default:
            break;
    }

    if(!(timer->flags & POSIX_TIMER_FLAG_PERIODIC)) {
        timer->flags &= ~POSIX_TIMER_FLAG_ACTIVE;
    }
}

int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid)
{
    posix_timer_t *timer;

    if (clockid != CLOCK_REALTIME) {
        osSetErrno(EINVAL);
        return -1;
    }
    if (evp == OS_NULL || timerid == OS_NULL) {
        osSetErrno(EINVAL);
        return -1;
    }
    // if (evp->sigev_notify != SIGEV_THREAD) {
    //     return -1;
    // }
    timer = (posix_timer_t *)malloc(sizeof(posix_timer_t));
    if (timer == OS_NULL) {
        osSetErrno(ENOMEM);
        return -1;
    }
    memset(timer, 0, sizeof(posix_timer_t));

    timer->clockid = clockid;
    if(evp) {
        osMemcpy(&timer->evp, evp, sizeof(struct sigevent));
    } else {
        timer->evp.sigev_notify = SIGEV_SIGNAL;
        timer->evp.sigev_signo = SIGALRM;
    }
    
    timer->evp = *evp;
    timer->os_timer = osTimerNew(posix_timer_callback, osTimerOnce, timer, NULL);
    if (timer->os_timer == OS_NULL) {
        osSetErrno(EAGAIN);
        osFree(timer);
        return -1;
    }
    *timerid = (timer_t)timer;
    return 0;
}

int timer_delete(timer_t timerid)
{
    posix_timer_t *timer = (posix_timer_t *)timerid;
    if (timer == OS_NULL || timer->os_timer == OS_NULL) {
        osSetErrno(EINVAL);
        return -1;
    }
    if (timer->flags & POSIX_TIMER_FLAG_ACTIVE) {
        osTimerStop(timer->os_timer);
    }
    osTimerDelete(timer->os_timer);
    free(timer);
    return 0;
}

int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value)
{
    posix_timer_t *timer = (posix_timer_t *)timerid;
    uint32_t initial_tick;

    if (timer == OS_NULL || timer->os_timer == OS_NULL || new_value == OS_NULL) {
        osSetErrno(EINVAL);
        return -1;
    }
    if (old_value != OS_NULL) {
        osMemcpy(old_value, &timer->value, sizeof(struct itimerspec));
    }

    if (timer->flags & POSIX_TIMER_FLAG_ACTIVE) {
        osTimerStop(timer->os_timer);
        timer->flags &= POSIX_TIMER_FLAG_ACTIVE;
    }

    if ((new_value->it_value.tv_sec == 0) && (new_value->it_value.tv_nsec == 0)) {
        osMemset(&timer->value, 0, sizeof(struct itimerspec));
        return 0;
    }

    osMemcpy(&timer->value, new_value, sizeof(struct itimerspec));

    if(flags & TIMER_ABSTIME) {
        struct timespec now;
        struct timespec relative;

        clock_gettime(timer->clockid, &now);

        relative.tv_sec = new_value->it_value.tv_sec - now.tv_sec;
        relative.tv_nsec = new_value->it_value.tv_nsec - now.tv_nsec;

        if(relative.tv_nsec < 0) {
            relative.tv_sec--;
            relative.tv_nsec += 1000000000L;
        }

        if(relative.tv_sec < 0) {
            relative.tv_sec = 0;
            relative.tv_nsec = 1;
        }

        initial_tick = (uint32_t)osTickFromUs((relative.tv_sec * USEC_PER_SEC) + (relative.tv_nsec / NSEC_PER_USEC));
    } else {
        initial_tick = (uint32_t)osTickFromUs((new_value->it_value.tv_sec * USEC_PER_SEC) + (new_value->it_value.tv_nsec / NSEC_PER_USEC));
    }

    if(new_value->it_interval.tv_sec > 0 || new_value->it_interval.tv_nsec > 0) {

        /* periodic timer handling:
         * since os timer only support A timeout value, we use the following strategy:
         * 1. first use one-shot mode with timerout = it_value;
         * 2. after detecting a periodic timer in cb func, switch to periodic mode;
         * 3. user it_interval as timeout value;
         */
        timer->flags |= POSIX_TIMER_FLAG_PERIODIC;
        osTimerControl(timer->os_timer, OS_TIMER_CTRL_SET_ONESHOT, NULL);
    } else {
        timer->flags &= ~POSIX_TIMER_FLAG_PERIODIC;
        osTimerControl(timer->os_timer, OS_TIMER_CTRL_SET_ONESHOT, NULL);
    }

    if (osTimerStart(timer->os_timer, initial_tick) == osOK) {
        timer->flags |= POSIX_TIMER_FLAG_ACTIVE;
        timer->overrun = 0;
        return 0;
    }

    osSetErrno(EINVAL);
    return -1;
}

int timer_gettime(timer_t timerid, struct itimerspec *value)
{
    posix_timer_t *timer = (posix_timer_t *)timerid;
    uint64_t remain_ms;

    if (timer == OS_NULL || timer->os_timer == OS_NULL || value == OS_NULL) {
        osSetErrno(EINVAL);
        return -1;
    }

    if (timer->flags & POSIX_TIMER_FLAG_ACTIVE) {
        remain_ms = osTimerGetRemainMs(timer->os_timer);
        value->it_value.tv_sec = remain_ms / MSEC_PER_SEC;
        value->it_value.tv_nsec = (remain_ms % MSEC_PER_SEC) * NSEC_PER_MSEC;
    } else {
        value->it_value.tv_sec = 0;
        value->it_value.tv_nsec = 0;
    }
    return 0;
}

/**
 * @brief 获取定时器超限次数
 */
int timer_getoverrun(timer_t timerid)
{
    posix_timer_t *timer = (posix_timer_t *)timerid;
    if (timer == OS_NULL || timer->os_timer == OS_NULL) {
        osSetErrno(EINVAL);
        return -1;
    }

    return timer->overrun;
}