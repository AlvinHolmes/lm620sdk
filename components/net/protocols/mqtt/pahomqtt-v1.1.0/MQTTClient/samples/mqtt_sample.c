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
 * \@file        MQTTOneOS.c
 *
 * \@brief       socket port file for mqtt
 *
 * \@details
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os.h>
#include <os_log.h>
#include "mqtt_api.h"
#include "certificate.h"

#define MQ_SAMPLE_TAG   "[MQ_SAMPLE]"

#ifdef MQTT_USING_TLS
#define TLS_SERVER_ADDR  "121.89.166.244"      /* private ali cloud mqtts server */
#define TLS_SERVER_PORT  8883
// #define TLS_SERVER_ADDR  "139.186.193.211"
// #define TLS_SERVER_PORT  8883
#else
#define TCP_SERVER_ADDR  "broker.emqx.io"
#define TCP_SERVER_PORT  1883
#endif

#define COMMAND_TIMEOUT 30000

typedef struct
{
    MQTT_Handle handle;
} mqtt_context_t;

struct osThread *mqtt_sample_task[2] = {NULL, NULL};

/**
 ***********************************************************************************************************************
 * @brief           This function prints arrived message.
 *
 * @param[in]       data            message info.
 ***********************************************************************************************************************
 */
void messageArrived(MQTT_Handle handle, MQTT_MessageData *data)
{
    LOG_I("Message arrived on topic %.*s: %.*s\r\n",
               data->topicName->lenstring.len,
               data->topicName->lenstring.data,
               data->message->payloadlen,
               data->message->payload);
}

static void mqtt_sample_task_func1(void *arg)
{
    /* connect to TCP_SERVER_ADDR, subscribe to a topic, send and receive messages regularly every 1 sec */
    uint32_t sendbufLen = 512;
    uint32_t readbufLen = 512;
    mqtt_context_t *mqtt_context = osMalloc(sizeof(mqtt_context_t));
    unsigned char   *sendbuf = osMalloc(sendbufLen);
    unsigned char   *readbuf = osMalloc(readbufLen);
    int             rc = 0;
    int             count = 0;
    const char* topicFilters[2] = {"mqtest1", "mqtest2"};
    int QoSs[2] = {MQTT_QOS1, MQTT_QOS2};


#ifdef MQTT_USING_TLS
    char addr[] = TLS_SERVER_ADDR;
    int port = TLS_SERVER_PORT;
#else
    char addr[] = TCP_SERVER_ADDR;
    int port = TCP_SERVER_PORT;
#endif

    OS_ASSERT(mqtt_context != NULL && sendbuf != NULL && readbuf != NULL);
    if (mqtt_context == NULL || sendbuf == NULL || readbuf == NULL)
    {
        if (mqtt_context)
        {
            osFree(mqtt_context);
        }
        if (sendbuf)
        {
            osFree(sendbuf);
        }
        if (readbuf)
        {
            osFree(readbuf);
        }

        return;
    }
    osMemset(mqtt_context, 0, sizeof(mqtt_context_t));
    osMemset(sendbuf, 0, sendbufLen);
    osMemset(readbuf, 0 , readbufLen);
    MQTT_ConnectOption connectData = MQTT_CONNECTDATA_INITIALIZER;

    rc = MQTT_CreateClient(&mqtt_context->handle,
                   COMMAND_TIMEOUT,
                   sendbuf,
                   sendbufLen,
                   readbuf,
                   readbufLen,
                   NULL);
    if (0 != rc)
    {
        LOG_E(MQ_SAMPLE_TAG"MQTT_CreateClient failed, check param\r\n");
        goto exit;
    }
    LOG_I(MQ_SAMPLE_TAG"MQTT_CreateClient sucess\r\n");

    connectData.MQTTVersion = 4; /*3 = 3.1 4 = 3.1.1*/
    connectData.keepAliveInterval = 60;
    connectData.cleansession = 1;
    connectData.willFlag = 0;
    connectData.clientID.cstring = "MQTTClient1";
    connectData.username.cstring = "";
    connectData.password.cstring = "";
    connectData.host = addr;
    connectData.port = port;
    connectData.ca_crt = g_certificate;

    if ((rc = MQTT_Connect(mqtt_context->handle, &connectData)) != 0)
    {
        LOG_E(MQ_SAMPLE_TAG"MQTT connect failed, return code from MQTT connect is %d\r\n", rc);
        goto exit;
    }
    LOG_I(MQ_SAMPLE_TAG"MQTT Connected\r\n");

    if ((rc = MQTT_Subscribe(mqtt_context->handle, 2, topicFilters, QoSs, messageArrived)) != 0)
    {
        LOG_E(MQ_SAMPLE_TAG"MQTT subscribe failed, return code from MQTT subscribe is %d\r\n", rc);
        goto exit;
    }
    LOG_I(MQ_SAMPLE_TAG"MQTT Subscribed\r\n");

    while (count<100)
    {
        ++count;
        MQTT_Message message;
        char payload[128];

        message.qos = count % 3;
        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d", count);
        message.payloadlen = strlen(payload);

        if ((rc = MQTT_Publish(mqtt_context->handle, "mqtest2", &message)) != 0)
        {
            LOG_E(MQ_SAMPLE_TAG"MQTT publish failed, return code from MQTT publish is %d\r\n", rc);
            continue;
            // goto exit;
        }
        LOG_I(MQ_SAMPLE_TAG"MQTT publish ok!\r\n");
        osThreadMsSleep(100);
    }

    if ((rc = MQTT_Unsubscribe(mqtt_context->handle, 2, topicFilters)) != 0)
    {
        LOG_E(MQ_SAMPLE_TAG"MQTT unsubscribe failed, return code from MQTT unsubscribe is %d\r\n", rc);
        goto exit;
    }
    LOG_I(MQ_SAMPLE_TAG"MQTT Unsubscribed\r\n");

exit:
    if (MQTT_IsConnected(mqtt_context->handle) == OS_TRUE)
        MQTT_Disconnect(mqtt_context->handle);

    MQTT_DestroyClient(&mqtt_context->handle);

    if (mqtt_context)
    {
        osFree(mqtt_context);
    }
    if (sendbuf)
    {
        osFree(sendbuf);
    }
    if (readbuf)
    {
        osFree(readbuf);
    }

    LOG_I(MQ_SAMPLE_TAG"MQTT sample1 terminate.\r\n");
}

static void mqtt_sample_task_func2(void *arg)
{
    /* connect to TCP_SERVER_ADDR, subscribe to a topic, send and receive messages regularly every 1 sec */
    uint32_t sendbufLen = 512;
    uint32_t readbufLen = 512;
    mqtt_context_t *mqtt_context = osMalloc(sizeof(mqtt_context_t));
    unsigned char   *sendbuf = osMalloc(sendbufLen);
    unsigned char   *readbuf = osMalloc(readbufLen);
    int             rc = 0;
    int             count = 0;
    const char* topicFilters[2] = {"mqtest2", "mqtest3"};
    int QoSs[2] = {MQTT_QOS0, MQTT_QOS1};


#ifdef MQTT_USING_TLS
    char addr[] = TLS_SERVER_ADDR;
    int port = TLS_SERVER_PORT;
#else
    char addr[] = TCP_SERVER_ADDR;
    int port = TCP_SERVER_PORT;
#endif

    OS_ASSERT(mqtt_context != NULL && sendbuf != NULL && readbuf != NULL);
    if (mqtt_context == NULL || sendbuf == NULL || readbuf == NULL)
    {
        if (mqtt_context)
        {
            osFree(mqtt_context);
        }
        if (sendbuf)
        {
            osFree(sendbuf);
        }
        if (readbuf)
        {
            osFree(readbuf);
        }

        return;
    }
    osMemset(mqtt_context, 0, sizeof(mqtt_context_t));
    osMemset(sendbuf, 0, sendbufLen);
    osMemset(readbuf, 0 , readbufLen);
    MQTT_ConnectOption connectData = MQTT_CONNECTDATA_INITIALIZER;

    rc = MQTT_CreateClient(&mqtt_context->handle,
                   COMMAND_TIMEOUT,
                   sendbuf,
                   sendbufLen,
                   readbuf,
                   readbufLen,
                   NULL);
    if (0 != rc)
    {
        LOG_E(MQ_SAMPLE_TAG"MQTT create client failed, check param\r\n");
        goto exit;
    }
    LOG_I(MQ_SAMPLE_TAG"MQTT create client sucess\r\n");

    connectData.MQTTVersion = 4; /*3 = 3.1 4 = 3.1.1*/
    connectData.keepAliveInterval = 60;
    connectData.cleansession = 1;
    connectData.willFlag = 0;
    connectData.clientID.cstring = "MQTTClient2";
    connectData.username.cstring = "";
    connectData.password.cstring = "";
    connectData.host = addr;
    connectData.port = port;
    connectData.ca_crt = g_certificate;

    if ((rc = MQTT_Connect(mqtt_context->handle, &connectData)) != 0)
    {
        LOG_E(MQ_SAMPLE_TAG"MQTT connect failed, return code from MQTT connect is %d\r\n", rc);
        goto exit;
    }
    LOG_I(MQ_SAMPLE_TAG"MQTT Connected\r\n");

    if ((rc = MQTT_Subscribe(mqtt_context->handle, 2, topicFilters, QoSs, messageArrived)) != 0)
    {
        LOG_E(MQ_SAMPLE_TAG"MQTT subscribe failed, return code from MQTT subscribe is %d\r\n", rc);
        goto exit;
    }
    LOG_I(MQ_SAMPLE_TAG"MQTT Subscribed\r\n");

    while (count<100)
    {
        ++count;
        MQTT_Message message;
        char payload[128];

        message.qos = count % 3;
        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d", count);
        message.payloadlen = strlen(payload);

        LOG_I(MQ_SAMPLE_TAG"MQTT publish ...!\r\n");
        if ((rc = MQTT_Publish(mqtt_context->handle, "mqtest3", &message)) != 0)
        {
            LOG_E(MQ_SAMPLE_TAG"MQTT publish failed, return code from MQTT publish is %d\r\n", rc);
            continue;
            // goto exit;
        }
        LOG_I(MQ_SAMPLE_TAG"MQTT publish ok!\r\n");
        osThreadMsSleep(100);
    }

    if ((rc = MQTT_Unsubscribe(mqtt_context->handle, 2, topicFilters)) != 0)
    {
        LOG_E(MQ_SAMPLE_TAG"MQTT unsubscribe failed, return code from MQTT unsubscribe is %d\r\n", rc);
        goto exit;
    }
    LOG_I(MQ_SAMPLE_TAG"MQTT Unsubscribed\r\n");

exit:
    if (MQTT_IsConnected(mqtt_context->handle) == OS_TRUE)
        MQTT_Disconnect(mqtt_context->handle);

    MQTT_DestroyClient(&mqtt_context->handle);

    if (mqtt_context)
    {
        osFree(mqtt_context);
    }
    if (sendbuf)
    {
        osFree(sendbuf);
    }
    if (readbuf)
    {
        osFree(readbuf);
    }

    LOG_I(MQ_SAMPLE_TAG"MQTT sample2 terminate.\r\n");
}

#ifdef MQTT_USING_TLS
#define MQTT_TASK_STACK_SIZE 10240 /* MQTT using tls, MQTT thread stack size need larger than 6K */
#else
#define MQTT_TASK_STACK_SIZE 4096
#endif

/**
 ***********************************************************************************************************************
 * @brief           This function start mqtt sample.
 ***********************************************************************************************************************
 */
void mqtts_sample_start(char argc, char **argv)
{
    osThreadAttr_t attr = {NULL, osThreadDetached, NULL, 0U, NULL, MQTT_TASK_STACK_SIZE, osPriorityNormal, 0U, 0U};
    mqtt_sample_task[0] = osThreadNew(mqtt_sample_task_func1, NULL, &attr);

    if (OS_NULL == mqtt_sample_task[0])
    {
        LOG_E(MQ_SAMPLE_TAG"mqtt echo thread1 create failed\r\n");
        OS_ASSERT(OS_NULL != mqtt_sample_task[0]);
    }

    mqtt_sample_task[1] = osThreadNew(mqtt_sample_task_func2, NULL, &attr);

    if (OS_NULL == mqtt_sample_task[1])
    {
        LOG_E(MQ_SAMPLE_TAG"mqtt echo thread2 create failed\r\n");
        OS_ASSERT(OS_NULL != mqtt_sample_task[1]);
    }
}

#ifdef OS_USING_SHELL
#include "nr_micro_shell.h"
NR_SHELL_CMD_EXPORT(mqtt_sample, mqtts_sample_start); // "start mqtt sample"
#endif
