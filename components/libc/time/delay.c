/**
 * @file delay.c
 * @brief POSIX delay 接口实现
 * @version 0.1
 * @date 2025-01-17
 *
 * Copyright (c) 2025 南京创芯慧联技术有限公司. All rights reserved.
 *
 */

#include <stdio.h>

#include <time.h>

#include <os.h>

void os_hw_us_delay(unsigned int us)
{
    osUsDelay(us);
}

void msleep(unsigned int msecs)
{
    osDelay(msecs);
}

void ssleep(unsigned int seconds)
{
    msleep(seconds * 1000);
}

void mdelay(unsigned long msecs)
{
    os_hw_us_delay(msecs * 1000);
}

void udelay(unsigned long usecs)
{
    os_hw_us_delay(usecs);
}

void ndelay(unsigned long nsecs)
{
    os_hw_us_delay(1);
}

unsigned int sleep(unsigned int seconds)
{
    if (osThreadGetId() != NULL) {
        ssleep(seconds);
    } else {
        while (seconds > 0) {
            udelay(1000000u);
            seconds--;
        }
    }
    return 0;
}

int usleep(useconds_t usec)
{
    if (osThreadGetId() != NULL) {
        msleep(usec / 1000u);
    } else {
        udelay(usec / 1000u);
    }
    udelay(usec % 1000u);
    return 0;
}

/**
 * @brief 睡眠指定纳秒数
 */
int nanosleep(const struct timespec *req, struct timespec *rem)
{
    uint32_t ticks;
    uint64_t totalUsec;
    if (req == NULL) {
        return -1;
    }
    totalUsec = (uint64_t)req->tv_sec * USEC_PER_SEC + (uint64_t)req->tv_nsec / 1000UL;
    ticks = osTickFromUs((uint32_t)totalUsec);
    if (ticks == 0 && totalUsec > 0) {
        ticks = 1;
    }
    osDelay(ticks);
    if (rem != NULL) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }
    return 0;
}
