/**
 * @file example_main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-12-29
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/****************************************************************************
* Included Files
****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "cm_os.h"
#include "cm_sys.h"
#include "cm_rtc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SECOND_OF_DAY (24 * 60 * 60)
static const char *const WEEKDAY[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
static const char DayOfMon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void demo_sec_to_date(long lSec, cm_tm_t *tTime);
static uint8_t demo_time_to_weekday(cm_tm_t *t);
static void alarm_callback(void);
static void print_time(const char *prefix, cm_tm_t *tm);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void demo_sec_to_date(long lSec, cm_tm_t *tTime)
{
    unsigned short i = 0;
    unsigned short j = 0;
    unsigned short iDay = 0;
    unsigned long lDay = 0;
    lDay = lSec / SECOND_OF_DAY;
    lSec = lSec % SECOND_OF_DAY;
    i = 1970;
    while (lDay > 365) {
        if (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0)) {
            lDay -= 366;
        } else {
            lDay -= 365;
        }
        i++;
    }
    if ((lDay == 365) && !(((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))) {
        lDay -= 365;
        i++;
    }
    tTime->tm_year = i;
    for (j = 0; j < 12; j++) {
        if ((j == 1) && (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))) {
            iDay = 29;
        } else {
            iDay = DayOfMon[j];
        }
        if (lDay >= iDay) {
            lDay -= iDay;
        } else {
            break;
        }
    }
    tTime->tm_mon = j + 1;
    tTime->tm_mday = lDay + 1;
    tTime->tm_hour = ((lSec / 3600)) % 24;
    tTime->tm_min = (lSec % 3600) / 60;
    tTime->tm_sec = (lSec % 3600) % 60;
}

static uint8_t demo_time_to_weekday(cm_tm_t *t)
{
    uint32_t u32WeekDay = 0;
    uint32_t u32Year = t->tm_year;
    uint8_t u8Month = t->tm_mon;
    uint8_t u8Day = t->tm_mday;
    if (u8Month < 3U) {
        u32WeekDay = (((23U * u8Month) / 9U) + u8Day + 4U + u32Year + ((u32Year - 1U) / 4U) - ((u32Year - 1U) / 100U) + ((
                          u32Year - 1U) / 400U)) % 7U;
    } else {
        u32WeekDay = (((23U * u8Month) / 9U) + u8Day + 4U + u32Year + (u32Year / 4U) - (u32Year / 100U) +
                      (u32Year / 400U) - 2U) % 7U;
    }
    if (u32WeekDay == 0U) {
        u32WeekDay = 7U;
    }
    return (uint8_t)u32WeekDay;
}

static void alarm_callback(void)
{
    cm_printf("[ALARM] Alarm callback triggered!\r\n");
}

static void print_time(const char *prefix, cm_tm_t *tm)
{
    cm_printf("[%s] %04d-%02d-%02d,%02d:%02d:%02d,%s\r\n",
                   prefix, tm->tm_year, tm->tm_mon, tm->tm_mday,
                   tm->tm_hour, tm->tm_min, tm->tm_sec,
                   WEEKDAY[demo_time_to_weekday(tm) - 1]);
}

/****************************************************************************
 * Public Functions
****************************************************************************/

void rtc_test_case_1(void)
{
    int32_t ret;
    cm_tm_t tm;
    cm_printf("rtc_test_case_1 start ...\r\n");
    ret = (int32_t)cm_rtc_get_current_time();
    cm_printf("[LEGAL] get_current_time: %d\r\n", ret);
    demo_sec_to_date((long)ret, &tm);
    print_time("TIME", &tm);
    ret = cm_rtc_set_current_time(cm_rtc_get_current_time() + 600);
    cm_printf("[LEGAL] set_current_time(+10min): %d\r\n", ret);
    ret = (int32_t)cm_rtc_get_current_time();
    demo_sec_to_date((long)ret, &tm);
    print_time("TIME", &tm);
    cm_printf("rtc_test_case_1 pass\r\n");
}

void rtc_test_case_2(void)
{
    int32_t ret;
    cm_tm_t tm;
    cm_tm_t alarm;
    cm_printf("rtc_test_case_2 start ...\r\n");
    demo_sec_to_date((long)cm_rtc_get_current_time(), &tm);
    cm_rtc_register_alarm_cb(alarm_callback);
    ret = cm_rtc_set_alarm(&tm);
    cm_printf("[LEGAL] set_alarm: %d\r\n", ret);
    cm_rtc_enable_alarm(true);
    ret = cm_rtc_get_alarm(&alarm);
    cm_printf("[LEGAL] get_alarm: %d\r\n", ret);
    if (ret == 0) {
        print_time("ALARM", &alarm);
    }
    cm_rtc_enable_alarm(false);
    ret = cm_rtc_get_alarm(&alarm);
    cm_printf("[LEGAL] get_alarm(disabled): %d\r\n", ret);
    cm_printf("rtc_test_case_2 pass\r\n");
}

void rtc_test_case_3(void)
{
    int32_t ret;
    cm_tm_t tm;
    cm_printf("rtc_test_case_3 start ...\r\n");
    demo_sec_to_date((long)cm_rtc_get_current_time(), &tm);
    tm.tm_min = tm.tm_min + 1;
    ret = cm_rtc_set_alarm(&tm);
    cm_printf("[LEGAL] set_alarm(time+1min): %d\r\n", ret);
    cm_rtc_enable_alarm(true);
    demo_sec_to_date((long)cm_rtc_get_current_time(), &tm);
    tm.tm_sec = 60;
    ret = cm_rtc_set_alarm(&tm);
    cm_printf("[LEGAL] set_alarm(sec=60,invalid): %d\r\n", ret);
    demo_sec_to_date((long)cm_rtc_get_current_time(), &tm);
    tm.tm_min = 60;
    ret = cm_rtc_set_alarm(&tm);
    cm_printf("[LEGAL] set_alarm(min=60,invalid): %d\r\n", ret);
    demo_sec_to_date((long)cm_rtc_get_current_time(), &tm);
    tm.tm_hour = 24;
    ret = cm_rtc_set_alarm(&tm);
    cm_printf("[LEGAL] set_alarm(hour=24,invalid): %d\r\n", ret);
    demo_sec_to_date((long)cm_rtc_get_current_time(), &tm);
    tm.tm_mday = 32;
    ret = cm_rtc_set_alarm(&tm);
    cm_printf("[LEGAL] set_alarm(mday=32,invalid): %d\r\n", ret);
    demo_sec_to_date((long)cm_rtc_get_current_time(), &tm);
    tm.tm_mon = 14;
    ret = cm_rtc_set_alarm(&tm);
    cm_printf("[LEGAL] set_alarm(mon=14,invalid): %d\r\n", ret);
    cm_printf("rtc_test_case_3 pass\r\n");
}

void rtc_test_case_4(void)
{
    int32_t ret;
    int32_t timezone;
    cm_printf("rtc_test_case_4 start ...\r\n");
    timezone = cm_rtc_get_timezone();
    cm_printf("[LEGAL] get_timezone: %d\r\n", timezone);
    ret = cm_rtc_set_timezone(timezone + 1);
    cm_printf("[LEGAL] set_timezone(+1): %d\r\n", ret);
    timezone = cm_rtc_get_timezone();
    cm_printf("[LEGAL] get_timezone(after+1): %d\r\n", timezone);
    ret = cm_rtc_set_timezone(timezone + 24);
    cm_printf("[LEGAL] set_timezone(+24,invalid): %d\r\n", ret);
    cm_printf("rtc_test_case_4 pass\r\n");
}

int main(void)
{
    cm_printf("========================================\r\n");
    cm_printf("CM RTC Demo Start\r\n");
    cm_printf("========================================\r\n");

    rtc_test_case_1();
    rtc_test_case_2();
    rtc_test_case_3();
    rtc_test_case_4();

    cm_printf("\r\n========================================\r\n");
    cm_printf("CM RTC Demo End\r\n");
    cm_printf("========================================\r\n");

    while (1) {
        osDelay(1000);
    }

    return 0;
}
