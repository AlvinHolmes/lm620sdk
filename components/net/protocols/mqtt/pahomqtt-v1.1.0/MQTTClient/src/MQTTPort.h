/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        MQTTPort.h
 *
 * \@brief       socket port file for mqtt
 *
 * \@details
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 * 2020-12-29   OneOS Team      modify tls netowrk interface and add new mqtt net interface
 ***********************************************************************************************************************
 */

#if !defined(MQTTPORT_H)
#define MQTTPORT_H

#include <stdint.h>
#include <os.h>
#include <os_log.h>
#include "slog_print.h"
#include "mqtt_api.h"

#ifdef MQTT_USING_TLS
#include "mbedtls/error.h"
#include "mq_tls_mbedtls.h"
#endif // MQTT_USING_TLS

#ifdef USING_EMULATOR
#define MQTT_PRINT_DEBUG(format, ...)   osPrintf("[DEBUG]"format, ##__VA_ARGS__)
#define MQTT_PRINT_INFO(format, ...)    osPrintf("[INFO]"format, ##__VA_ARGS__)
#define MQTT_PRINT_WARN(format, ...)    osPrintf("[WARN]"format, ##__VA_ARGS__)
#define MQTT_PRINT_ERROR(format, ...)   osPrintf("[ERROR]"format, ##__VA_ARGS__)
#else
#define MQTT_PRINT_DEBUG(format, ...)   slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_MID_MQTT, format, ##__VA_ARGS__)
#define MQTT_PRINT_INFO(format, ...)    slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_MID_MQTT, format, ##__VA_ARGS__)
#define MQTT_PRINT_WARN(format, ...)    slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_MID_MQTT, format, ##__VA_ARGS__)
#define MQTT_PRINT_ERROR(format, ...)   slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_MID_MQTT, format, ##__VA_ARGS__)
#endif

/* MQTT return value definitions */
#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

typedef struct osThread *Thread;
typedef struct osMutex Mutex;

typedef struct Timer
{
    osTick_t xTicksToWait; /* tick wait setting for this timer */
    osTick_t xTicksRecord; /* record the tick value */
    struct osTimer *xTimeOut;   /* ct_timer ref */
} Timer;

/**
 * Timer callback function.
 */
typedef void (*TimerCallbackFunc) (void *argument);
void TimerInit(Timer *, TimerCallbackFunc);
void TimerStop(Timer *);
void TimerRelease(Timer *);
char TimerIsExpired(Timer *);
void TimerCountdownMS(Timer *, unsigned int);
void TimerCountdown(Timer *, unsigned int);
int TimerLeftMS(Timer *);

void MutexInit(Mutex *);
void MutexDeInit(Mutex *mutex);
int MutexLock(Mutex *);
int MutexUnlock(Mutex *);

int ThreadStart(Thread *, void (*fn)(void *), void *arg);
void ThreadDelete(Thread);

int MQTTNetworkSetParam(Network *pNetwork, const char *host, uint16_t port, const char *ca_crt);
int MQTTNetworkInit(Network *pNetwork, const char *host, uint16_t port, const char *ca_crt);
int MQTTNetworkConnect(Network *pNetwork);
void MQTTNetworkDisconnect(Network *pNetwork);
#endif
