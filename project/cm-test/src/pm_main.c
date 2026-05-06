/**
 * @file example_main.c
 * @brief PM 测试用例
 * @date 2025-01-22
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
#include "cm_pm.h"
#include "cm_sys.h"
#include "cm_demo_common.h"
#include "cm_rtc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PM_TEST_EVENT_QUEUE_SIZE     8
#define PM_TEST_STANDBY_WAKEUP_COUNT 5
#define PM_TEST_RTC_WAKEUP_SECONDS   10
#define PM_TEST_ENTER_PSM            1
#define SECOND_OF_DAY                (24 * 60 * 60)

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef enum {
    PM_TEST_EVENT_POWERKEY_PRESS = 0,
    PM_TEST_EVENT_POWERKEY_RELEASE,
    PM_TEST_EVENT_PM_ENTER,
    PM_TEST_EVENT_PM_EXIT,
} pm_test_event_e;

typedef struct {
    pm_test_event_e event;
    uint32_t data;
} pm_test_msg_t;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void pm_test_event_callback(void);
static void pm_test_exit_callback(uint32_t reason);
static void pm_test_powerkey_callback(cm_powerkey_event_e event);
static void pm_test_task(void *arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static osMessageQueueId_t s_event_queue = NULL;
static volatile uint32_t s_key_press_count = 0;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void pm_sec_to_date(long lSec, cm_tm_t *tTime)
{
    unsigned short i, j, iDay;
    unsigned long lDay;
    static const unsigned char DayOfMon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

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

static void pm_test_event_callback(void)
{
    pm_test_msg_t msg = {.event = PM_TEST_EVENT_PM_ENTER, .data = 0};
    if (s_event_queue != NULL) {
        osMessageQueuePut(s_event_queue, &msg, 0, 0);
    }
}

static void pm_test_exit_callback(uint32_t reason)
{
    pm_test_msg_t msg = {.event = PM_TEST_EVENT_PM_EXIT, .data = reason};
    if (s_event_queue != NULL) {
        osMessageQueuePut(s_event_queue, &msg, 0, 0);
    }
}

static void pm_test_powerkey_callback(cm_powerkey_event_e event)
{
    pm_test_msg_t msg;
    if (CM_POWERKEY_EVENT_RELEASE == event) {
        msg.event = PM_TEST_EVENT_POWERKEY_RELEASE;
    } else {
        msg.event = PM_TEST_EVENT_POWERKEY_PRESS;
    }
    msg.data = 0;
    if (s_event_queue != NULL) {
        osMessageQueuePut(s_event_queue, &msg, 0, 0);
    }
}

static void pm_test_task(void *arg)
{
    (void)arg;
    pm_test_msg_t msg;
    uint64_t current_time;
    cm_tm_t alarm_time;

    while (1) {
        if (osMessageQueueGet(s_event_queue, &msg, NULL, osWaitForever) == osOK) {
            switch (msg.event) {
            case PM_TEST_EVENT_POWERKEY_PRESS:
                s_key_press_count++;
                cm_printf("[PM] powerkey PRESS (count=%u)\r\n", s_key_press_count);
                break;

            case PM_TEST_EVENT_POWERKEY_RELEASE:
                cm_printf("[PM] powerkey RELEASE\r\n");
                break;

            case PM_TEST_EVENT_PM_ENTER:
                cm_printf("[PM] entering low power mode\r\n");
                break;

            case PM_TEST_EVENT_PM_EXIT:
                cm_printf("[PM] exiting low power mode, reason=%lu\r\n", msg.data);
                break;

            default:
                break;
            }

            if (s_key_press_count >= PM_TEST_STANDBY_WAKEUP_COUNT) {
                cm_printf("[PM] Key count reached %u, setting RTC alarm for standby wakeup\r\n",
                          PM_TEST_STANDBY_WAKEUP_COUNT);

                current_time = cm_rtc_get_current_time();
                if (current_time > 0) {
                    pm_sec_to_date((long)(current_time + PM_TEST_RTC_WAKEUP_SECONDS), &alarm_time);

                    cm_rtc_enable_alarm(true);
                    if (cm_rtc_set_alarm(&alarm_time) == 0) {
                        cm_printf("[PM] RTC alarm set for %d seconds wakeup\r\n", PM_TEST_RTC_WAKEUP_SECONDS);
                    }

#if PM_TEST_ENTER_PSM
                    cm_printf("[PM] Entering standby via extension API...\r\n");
                    osDelay(2000);
                    cm_pm_work_unlock();
                    cm_pm_enter_psm();
#else
                    cm_printf("[PM] Poweroff path does not support RTC wakeup\r\n");
                    cm_printf("[PM] Entering true poweroff...\r\n");
                    osDelay(2000);
                    cm_pm_poweroff();
#endif
                } else {
                    cm_printf("[PM] Failed to get current time\r\n");
                }
                s_key_press_count = 0;
            }
        }
    }
}

/****************************************************************************
 * Public Functions
****************************************************************************/

int main(void)
{
    int reason;
    osThreadAttr_t task_attr = {0};

    cm_printf("[PM] Initializing...\r\n");

    s_event_queue = osMessageQueueNew(PM_TEST_EVENT_QUEUE_SIZE, sizeof(pm_test_msg_t), NULL);
    if (s_event_queue == NULL) {
        cm_printf("[PM] failed to create queue\r\n");
        return -1;
    }

    task_attr.name = "pm_test_task";
    task_attr.stack_size = 2048;
    task_attr.priority = osPriorityBelowNormal;
    if (osThreadNew(pm_test_task, NULL, &task_attr) == NULL) {
        cm_printf("[PM] failed to create task\r\n");
        return -1;
    }

    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = pm_test_event_callback;
    pm_cfg.cb_exit = pm_test_exit_callback;
    cm_pm_init(pm_cfg);
    cm_printf("[PM] Initialized\r\n");

    cm_printf("\r\n[PM] Getting power on reason...\r\n");
    reason = cm_pm_get_power_on_reason();
    cm_printf("[PM] power on reason=%d\r\n", reason);
    switch (reason) {
    case CM_PM_UNKNOWN:
        cm_printf("[PM]   -> UNKNOWN\r\n");
        break;
    case CM_PM_CHARG_POW_OFF:
        cm_printf("[PM]   -> CHARG_POW_OFF\r\n");
        break;
    case CM_PM_RD_PRO_MODE:
        cm_printf("[PM]   -> RD_PRO_MODE\r\n");
        break;
    case CM_PM_RTC_ALARM:
        cm_printf("[PM]   -> RTC_ALARM\r\n");
        break;
    case CM_PM_POWER_ON:
        cm_printf("[PM]   -> POWER_ON\r\n");
        break;
    case CM_PM_ERROR_RESET:
        cm_printf("[PM]   -> ERROR_RESET\r\n");
        break;
    default:
        cm_printf("[PM]   -> INVALID\r\n");
        break;
    }

    cm_printf("\r\n[PM] Enabling powerkey callback...\r\n");
    cm_pm_powerkey_regist_callback(pm_test_powerkey_callback);

    cm_printf("[PM] Press powerkey %d times to trigger RTC wakeup standby test\r\n", PM_TEST_STANDBY_WAKEUP_COUNT);
#if PM_TEST_ENTER_PSM
    cm_printf("[PM] Using cm_pm_enter_psm() for wake-capable standby\r\n");
#else
    cm_printf("[PM] Using cm_pm_poweroff() true poweroff path\r\n");
#endif

    return 0;
}
