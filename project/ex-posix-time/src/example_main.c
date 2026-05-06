/**
 * @file example_main.c
 * @brief POSIX 时间功能测试示例
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
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "nr_micro_shell.h"

#define TEST_COUNT 3

static void test_gettimeofday(void)
{
    struct timeval tv;
    char *curr_time;

    osPrintf("\n--- Test gettimeofday ---\r\n");

    gettimeofday(&tv, NULL);
    curr_time = ctime(&tv.tv_sec);
    osPrintf("current real system time to local time is %s\r\n", curr_time);
    osPrintf("timeval: tv_sec=%lld, tv_usec=%ld\r\n", tv.tv_sec, tv.tv_usec);
}

static void test_gmtime(void)
{
    struct timeval tv;
    struct tm *tm = NULL;

    osPrintf("\n--- Test gmtime ---\r\n");

    gettimeofday(&tv, NULL);
    tm = gmtime(&tv.tv_sec);
    osPrintf("UTC time is %d-%d-%d, %d:%d:%d\r\n", (tm->tm_year) + 1900, (tm->tm_mon) + 1, (tm->tm_mday), (tm->tm_hour), (tm->tm_min), (tm->tm_sec));
}

static void test_localtime(void)
{
    struct timeval tv;
    struct tm *tm = NULL;

    osPrintf("\n--- Test localtime ---\r\n");

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    osPrintf("local time is %d-%d-%d, %d:%d:%d\r\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

static void test_asctime(void)
{
    struct timeval tv;
    struct tm *tm = NULL;
    char *curr_time;

    osPrintf("\n--- Test asctime ---\r\n");

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    curr_time = asctime(tm);
    osPrintf("current local time is %s\r\n", curr_time);
}

static void test_mktime(void)
{
    struct timeval tv;
    struct tm tm2;
    struct tm *tm = NULL;
    time_t timestamp_by_mktime = 0;
    time_t timestamp_by_time = 0;
    char *curr_time;

    osPrintf("\n--- Test mktime ---\r\n");

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    memcpy(&tm2, tm, sizeof(tm2));
    timestamp_by_mktime = mktime(&tm2);
    osPrintf("timestamp by mktime() = %lld\r\n", timestamp_by_mktime);
    curr_time = ctime(&timestamp_by_mktime);
    osPrintf("timestamp to local time is %s\r\n", curr_time);

    timestamp_by_time = time(NULL);
    osPrintf("timestamp by time() = %lld\r\n", timestamp_by_time);
    curr_time = ctime(&timestamp_by_time);
    osPrintf("timestamp to local time is %s\r\n", curr_time);
}

static void test_clock_settime(void)
{
    struct timespec tp;
    tp.tv_sec = 1769593926;
    tp.tv_nsec = 0;

    osPrintf("\n--- Test clock_settime ---\r\n");

    if (clock_settime(CLOCK_REALTIME, &tp) == 0) {
        osPrintf("clock_settime: tv_sec=%lld, tv_nsec=%ld\r\n", tp.tv_sec, tp.tv_nsec);
    } else {
        osPrintf("clock_settime failed\r\n");
    }
}

static void test_clock_gettime(void)
{
    struct timespec tp;

    osPrintf("\n--- Test clock_gettime ---\r\n");

    if (clock_gettime(CLOCK_REALTIME, &tp) == 0) {
        osPrintf("CLOCK_REALTIME: tv_sec=%lld, tv_nsec=%ld\r\n", tp.tv_sec, tp.tv_nsec);
    } else {
        osPrintf("clock_gettime failed\r\n");
    }
}

static void test_clock_getres(void)
{
    struct timespec res;

    osPrintf("\n--- Test clock_getres ---\r\n");

    if (clock_getres(CLOCK_REALTIME, &res) == 0) {
        osPrintf("CLOCK_REALTIME resolution: tv_sec=%lld, tv_nsec=%ld\r\n", res.tv_sec, res.tv_nsec);
    } else {
        osPrintf("clock_getres failed\r\n");
    }
}

static void test_sleep(void)
{
    osPrintf("\n--- Test sleep ---\r\n");
    osPrintf("sleeping for 2 seconds...\r\n");
    sleep(2);
    osPrintf("sleep completed\r\n");
}

static void test_usleep(void)
{
    osPrintf("\n--- Test usleep ---\r\n");
    osPrintf("sleeping for 500000 microseconds (0.5 seconds)...\r\n");
    usleep(500000);
    osPrintf("usleep completed\r\n");
}

static void test_nanosleep(void)
{
    struct timespec req, rem;

    osPrintf("\n--- Test nanosleep ---\r\n");
    req.tv_sec = 0;
    req.tv_nsec = 500000000L;
    osPrintf("sleeping for 500000000 nanoseconds (0.5 seconds)...\r\n");
    nanosleep(&req, &rem);
    osPrintf("nanosleep completed\r\n");
}

static void test_settimeofday(void)
{
    struct timeval tv;
    struct tm *tm = NULL;
    struct timezone tz;
    osPrintf("\n--- Test settimeofday ---\r\n");

    tv.tv_sec = 10 * 60 * 60;
    tv.tv_usec = 0;

    tz.tz_minuteswest = 8 * 60;
    osPrintf("setting time to 10:00:00 GMT, tz -8...\r\n");
    if (settimeofday(&tv, &tz) == 0) {
        osPrintf("settimeofday success\r\n");
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);
        osPrintf("new local time: %d-%d-%d, %d:%d:%d\r\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    } else {
        osPrintf("settimeofday failed\r\n");
    }

    tz.tz_minuteswest = -8 * 60;
    osPrintf("restore tz to +8...\r\n");
    if (settimeofday(&tv, &tz) == 0) {
        osPrintf("settimeofday success\r\n");
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);
        osPrintf("new local time: %d-%d-%d, %d:%d:%d\r\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    } else {
        osPrintf("settimeofday failed\r\n");
    }
}

static void demo_time_all(char argc, char **argv)
{
    int ret = 0;
    int count;

    osPrintf("\r\n");
    osPrintf("========================================\r\n");
    osPrintf("  POSIX Time Function Test\r\n");
    osPrintf("========================================\r\n");

    for (count = 0; count < TEST_COUNT; count++) {
        osPrintf("\n========== Test Round %d ==========\r\n", count + 1);

        test_gettimeofday();
        sleep(2);

        test_gmtime();
        sleep(2);

        test_localtime();
        sleep(2);

        test_asctime();
        sleep(2);

        test_mktime();
        sleep(2);

        test_clock_settime();
        sleep(2);
        usleep(200 * 1000);

        test_clock_gettime();
        sleep(2);
        
        test_clock_getres();
        sleep(2);

        test_settimeofday();
        test_sleep();
        test_usleep();
        test_nanosleep();
        
        if (count < TEST_COUNT - 1) {
            osPrintf("\nWaiting 3 seconds before next round...\r\n");
            sleep(3);
        }
    }

    osPrintf("\n========================================\r\n");
    if (ret == 0) {
        osPrintf("=== Demo Test PASSED ===\r\n");
    } else {
        osPrintf("=== Demo Test FAILED ===\r\n");
    }
    osPrintf("========================================\n\r\n");

    return;
}

NR_SHELL_CMD_EXPORT(posix_time, demo_time_all);

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
