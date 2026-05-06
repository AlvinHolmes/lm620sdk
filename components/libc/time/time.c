/**
 * @file time.c
 * @brief POSIX 时间函数实现
 * @version 0.1
 * @date 2025-01-19
 *
 * Copyright (c) 2006-2021, RT-Thread Development Team
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2026 南京创芯慧联技术有限公司. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>

#include "os.h"
#include "drv_rtc.h"

/* seconds per day */
#define SPD (24 * 60 * 60)

#if !defined(__weak)
#   define __weak __attribute__((weak))
#endif

/* days per month -- nonleap! */
static const short __spm[13] = {
    0,
    (31),
    (31 + 28),
    (31 + 28 + 31),
    (31 + 28 + 31 + 30),
    (31 + 28 + 31 + 30 + 31),
    (31 + 28 + 31 + 30 + 31 + 30),
    (31 + 28 + 31 + 30 + 31 + 30 + 31),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31),
};

static const char days[] = "Sun Mon Tue Wed Thu Fri Sat ";
static const char months[] = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ";

time_t timegm(struct tm *const t);

static int __isleap(int year)
{
    /* every fourth year is a leap year except for century years that are
     * not divisible by 400. */
    /*  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); */
    return (!(year % 4) && ((year % 100) || !(year % 400)));
}

static void num2str(char *c, int i)
{
    c[0] = i / 10 + '0';
    c[1] = i % 10 + '0';
}

/**
 * @brief 将时间戳转换为 GMT/UTC 时间
 */
struct tm *gmtime_r(const time_t *timep, struct tm *r)
{
    int i;
    time_t work_time;
    int work_days;
    int work_seconds;
    
    work_time = *timep;
    
    /* 计算天数（向下取整） */
    work_days = (int)(work_time / (24*60*60));
    if (work_time < 0 && work_time % (24*60*60) != 0)
    {
        work_days--;  /* 负数向下取整 */
    }
    
    /* 计算当天的秒数（确保非负：0-86399） */
    work_seconds = (int)(work_time % (24*60*60));
    if (work_seconds < 0)
    {
        work_seconds += 24*60*60;  /* 转换为正数 */
    }
    
    /* 计算时分秒 */
    r->tm_sec = work_seconds % 60;
    work_seconds /= 60;
    r->tm_min = work_seconds % 60;
    r->tm_hour = work_seconds / 60;
    
    /* 计算星期（1970-01-01 是星期四，wday=4） */
    r->tm_wday = (4 + work_days) % 7;
    if (r->tm_wday < 0)
    {
        r->tm_wday += 7;  /* 确保 0-6 */
    }
    
    /* 计算年份和年内天数 */
    i = 1970;
    if (work_days >= 0)
    {
        /* 1970年及以后 */
        while (1)
        {
            int days_in_year = __isleap(i) ? 366 : 365;
            if (work_days >= days_in_year)
            {
                work_days -= days_in_year;
                i++;
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        /* 1970年之前 */
        while (work_days < 0)
        {
            i--;
            int days_in_year = __isleap(i) ? 366 : 365;
            work_days += days_in_year;
        }
    }
    
    r->tm_year = i - 1900;
    r->tm_yday = work_days;
    
    /* 计算月份和日期 */
    r->tm_mday = 1;
    
    /* 处理闰年的特殊情况 */
    if (__isleap(i) && (work_days > 58))
    {
        if (work_days == 59)
        {
            r->tm_mday = 2;  /* 2月29日 */
        }
        work_days -= 1;  /* 跳过2月29日进行后续计算 */
    }
    
    /* 查找月份 */
    for (i = 11; i > 0 && (__spm[i] > work_days); --i)
        ;
    
    r->tm_mon = i;
    r->tm_mday += work_days - __spm[i];
    r->tm_isdst = 0;

    return r;
}

struct tm *gmtime(const time_t *t)
{
    static struct tm tmp;
    return gmtime_r(t, &tmp);
}

/**
 * @brief 将UTC时间戳转换为本地时间
 */
struct tm* localtime_r(const time_t* t, struct tm* r)
{
    time_t tmp;
    int8_t timezone = 0;
    RTC_GetZone(&timezone);
    tmp = *t + ((timezone / 4) * 3600);
    return gmtime_r(&tmp, r);
}

struct tm* localtime(const time_t* t)
{
    static struct tm tmp;
    return localtime_r(t, &tmp);
}
/**
 * @brief 将本地时间 tm 结构体转换为UTC时间戳
 */
time_t mktime(struct tm *t)
{
    time_t timestamp;
    int8_t timezone = 0;
    RTC_GetZone(&timezone);
    timestamp = timegm(t);
    timestamp = timestamp - ((timezone / 4) * 3600);
    return timestamp;
}

/**
 * @brief 将 tm 结构体转换为字符串
 */
char *asctime_r(const struct tm *t, char *buf)
{
    if ((int)strlen(days) <= (t->tm_wday << 2) || (int)strlen(months) <= (t->tm_mon << 2)) {
        *(int *)buf = *(int *)days;
        *(int *)(buf + 4) = *(int *)months;
        num2str(buf + 8, t->tm_mday);
        if (buf[8] == '0') {
            buf[8] = ' ';
        }
        buf[10] = ' ';
        num2str(buf + 11, t->tm_hour);
        buf[13] = ':';
        num2str(buf + 14, t->tm_min);
        buf[16] = ':';
        num2str(buf + 17, t->tm_sec);
        buf[19] = ' ';
        num2str(buf + 20, 2000 / 100);
        num2str(buf + 22, 2000 % 100);
        buf[24] = '\n';
        buf[25] = '\0';
        return buf;
    }
    *(int *)buf = *(int *)(days + (t->tm_wday << 2));
    *(int *)(buf + 4) = *(int *)(months + (t->tm_mon << 2));
    num2str(buf + 8, t->tm_mday);
    if (buf[8] == '0') {
        buf[8] = ' ';
    }
    buf[10] = ' ';
    num2str(buf + 11, t->tm_hour);
    buf[13] = ':';
    num2str(buf + 14, t->tm_min);
    buf[16] = ':';
    num2str(buf + 17, t->tm_sec);
    buf[19] = ' ';
    num2str(buf + 20, (t->tm_year + 1900) / 100);
    num2str(buf + 22, (t->tm_year + 1900) % 100);
    buf[24] = '\n';
    buf[25] = '\0';
    return buf;
}

char *asctime(const struct tm *timeptr)
{
    static char buf[26];
    return asctime_r(timeptr, buf);
}

/**
 * @brief 将时间戳转换为本地时间字符串
 */
char *ctime_r(const time_t *tim_p, char *result)
{
    struct tm tm;
    return asctime_r(localtime_r(tim_p, &tm), result);
}

char *ctime(const time_t *tim_p)
{
    return asctime(localtime(tim_p));
}

__weak clock_t clock(void)
{
    return osKernelGetTickCount();
}


time_t timegm(struct tm *const t)
{
    time_t day;
    time_t i;
    time_t years = (time_t)(t->tm_year) - 70;
    if (t->tm_sec > 60) {
        t->tm_min += t->tm_sec / 60;
        t->tm_sec %= 60;
    }
    if (t->tm_min > 60) {
        t->tm_hour += t->tm_min / 60;
        t->tm_min %= 60;
    }
    if (t->tm_hour > 24) {
        t->tm_mday += t->tm_hour / 24;
        t->tm_hour %= 24;
    }
    if (t->tm_mon > 12) {
        t->tm_year += t->tm_mon / 12;
        t->tm_mon %= 12;
    }
    while (t->tm_mday > __spm[1 + t->tm_mon]) {
        if ((t->tm_mon == 1) && __isleap(t->tm_year + 1900)) {
            --t->tm_mday;
        }
        t->tm_mday -= __spm[t->tm_mon];
        ++t->tm_mon;
        if (t->tm_mon > 11) {
            t->tm_mon = 0;
            ++t->tm_year;
        }
    }
    if (t->tm_year < 70) {
        return (time_t) -1;
    }
    day = years * 365 + (years + 1) / 4;
    if (years >= 131) {
        years -= 131;
        years /= 100;
        day -= (years >> 2) * 3 + 1;
        if ((years &= 3) == 3) {
            years--;
        }
        day -= years;
    }
    day += t->tm_yday = __spm[t->tm_mon] + t->tm_mday - 1 + (__isleap(t->tm_year + 1900) & (t->tm_mon > 1));
    i = 7;
    t->tm_wday = (day + 4) % i;
    i = 24;
    day *= i;
    i = 60;
    return ((day + t->tm_hour) * i + t->tm_min) * i + t->tm_sec;
}

/* int gettimeofday(struct timeval *tv, struct timezone *tz) 见 components\kernel\src\kservice.c */

/**
 * @brief 设置系统时间
 */
int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
    RTC_Time rtcTime;
    int8_t zone = 0;

    if (tv == NULL) {
        return -1;
    }
    if (tz != NULL) {
        zone = -(tz->tz_minuteswest / 15);
        RTC_SetZone(zone);
    } else {
        RTC_GetZone(&zone);
    }

    /* In this system, we consistently set RTC to local time */
    rtcTime = tv->tv_sec + (zone * 15 * 60);
    rtcTime = rtcTime < 0 ? 0 : rtcTime;
    RTC_SetTime(rtcTime);
    return 0;
}

/* inherent in the toolchain */
/*
double     difftime (time_t _time2, time_t _time1);
size_t     strftime (char *__restrict _s,
                 size_t _maxsize, const char *__restrict _fmt,
                 const struct tm *__restrict _t);
 */


time_t time(time_t *t)
{
    time_t time_now = 0;
    int8_t zone = 0;

    RTC_GetZone(&zone);
    time_now = RTC_GetTime() - (zone * 15 * 60);

    /* if t is not NULL, write timestamp to *t */
    if (t != OS_NULL)
    {
        *t = time_now;
    }

    return time_now;
}