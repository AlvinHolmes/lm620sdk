/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file
*
* @brief  MQTT 订阅和断开重连示例
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/
#include "mqtt_api.h"


/************************************************************************************
*                                 宏定义
************************************************************************************/
#ifdef MQTT_USING_TLS //依赖MQTT协议中的宏配置
#define MQTT_SERVER_ADDR  "broker.emqx.io" // "34.243.217.54" "35.172.255.228"
#define MQTT_SERVER_PORT  8883
static const char g_certificate[] =
    "-----BEGIN CERTIFICATE-----\r\n"                                      \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\r\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n" \
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\r\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n" \
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\r\n" \
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\r\n" \
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\r\n" \
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\r\n" \
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\r\n" \
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\r\n" \
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\r\n" \
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\r\n" \
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\r\n" \
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\r\n" \
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\r\n" \
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\r\n" \
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\r\n" \
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\r\n" \
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\r\n"                 \
    "-----END CERTIFICATE-----\r\n"
    ;
#else
#define MQTT_SERVER_ADDR  "broker.emqx.io"
#define MQTT_SERVER_PORT  1883
static const char *g_certificate = NULL;
#endif

#define COMMAND_TIMEOUT 30000

#define EX_MQTT_DEMO_USE_SLOG                   (0) // 0:使用osPrintf; 1:使用slogPrintf

#if EX_MQTT_DEMO_USE_SLOG
#define EX_MQTT_DEMO_PRINTF_DEBUG(format, ...)  slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_TEST, format, ##__VA_ARGS__)
#define EX_MQTT_DEMO_PRINTF_INFO(format, ...)   slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_TEST, format, ##__VA_ARGS__)
#define EX_MQTT_DEMO_PRINTF_WARN(format, ...)   slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_TEST, format, ##__VA_ARGS__)
#define EX_MQTT_DEMO_PRINTF_ERROR(format, ...)  slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_TEST, format, ##__VA_ARGS__)
#else
#define EX_MQTT_DEMO_PRINTF_DEBUG(format, ...)  osPrintf(format, ##__VA_ARGS__)
#define EX_MQTT_DEMO_PRINTF_INFO(format, ...)   osPrintf(format, ##__VA_ARGS__)
#define EX_MQTT_DEMO_PRINTF_WARN(format, ...)   osPrintf(format, ##__VA_ARGS__)
#define EX_MQTT_DEMO_PRINTF_ERROR(format, ...)  osPrintf(format, ##__VA_ARGS__)
#endif

#define MAX_CACHE_SIZE 10

#define MQTT_TASK_STACK_SIZE 4096

#define MAX_EVENT_COUNT 10


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
typedef struct
{
    uint32_t eventID;
    uint32_t dataLen;
    void* data;
} MQTT_DEMO_Event;

typedef enum
{
    MQTT_DEMO_EVENT_START_CONNECT = 0,
    MQTT_DEMO_EVENT_CONNECTED,
    MQTT_DEMO_EVENT_RECV,
    MQTT_DEMO_EVENT_KEEPALIVE_LOST,
    MQTT_DEMO_EVENT_SESSION_CLOSED,
} MQTT_DEMO_EventId;


typedef struct
{
    char *topic;
    uint32_t topicLen;
    uint8_t *data;
    uint32_t dataLen;
} MQTT_DEMO_TopicData;

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/

//线程句柄
static struct osThread *g_MQTT_Thread = NULL;
//消息队列名柄
static osMessageQueueId_t g_MQTT_QueueId = NULL;

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
//  创建队列
static osMessageQueueId_t DEMO_MQTT_MessageQueue_Create(uint32_t messageCount)
{
    osMessageQueueAttr_t msgQueueAttr = {"MQTTAPP", 0U, NULL, 0U, NULL, 0U};

    return osMessageQueueNew(messageCount, sizeof(MQTT_DEMO_Event), &msgQueueAttr);
}

//  删除队列
static osStatus_t DEMO_MQTT_MessageQueue_Delete(osMessageQueueId_t queue)
{
    return osMessageQueueDelete(queue);
}

//  从队列收消息
static osStatus_t DEMO_MQTT_MessageQueue_Recv(osMessageQueueId_t queue, MQTT_DEMO_Event *event)
{
    return osMessageQueueGet(queue, event, 0, osWaitForever);
}

//  向队列发送消息
static osStatus_t DEMO_MQTT_MessageQueue_Send(osMessageQueueId_t queue, MQTT_DEMO_Event event)
{
    return osMessageQueuePut(queue, &event, 0, 0);
}

//主题订阅回调函数
static void messageArrived(MQTT_Handle handle, MQTT_MessageData *data)
{
    osStatus_t status = osStatusReserved;
    //打印消息内容
    EX_MQTT_DEMO_PRINTF_INFO("MQTT arrived topic [%u][%.*s]\r\n",
                            data->topicName->lenstring.len, data->topicName->lenstring.len, data->topicName->lenstring.data);
    EX_MQTT_DEMO_PRINTF_INFO("MQTT arrived data [%u][%.*s]\r\n",
                            data->message->payloadlen, data->message->payloadlen, data->message->payload);

    MQTT_DEMO_Event sendevent = {0};
    // memory layout |struct TopicData| topic memory | topic data memory |
    uint32_t event_dataLen = sizeof(MQTT_DEMO_TopicData) +  (data->topicName->lenstring.len + 1) + (data->message->payloadlen);
    MQTT_DEMO_TopicData *topicdata = NULL;

    sendevent.eventID = MQTT_DEMO_EVENT_RECV;

    sendevent.data = osCalloc(1, event_dataLen);
    if (sendevent.data != NULL)
    {
        topicdata = sendevent.data;
        // copy topic
        topicdata->topic = (char *)(topicdata+1);
        memcpy(topicdata->topic, data->topicName->lenstring.data, data->topicName->lenstring.len);
        topicdata->topicLen = data->topicName->lenstring.len;
        // copy topic data
        topicdata->data = (uint8_t *)topicdata->topic + topicdata->topicLen + 1;
        topicdata->dataLen = data->message->payloadlen;
        osMemcpy(topicdata->data, data->message->payload, data->message->payloadlen);
    }

    status = DEMO_MQTT_MessageQueue_Send(g_MQTT_QueueId, sendevent);
    EX_MQTT_DEMO_PRINTF_INFO("Message arrived Event Send[%d]\r\n", status);
    if (status != osOK && sendevent.data != NULL)
    {
        osFree(sendevent.data);
    }
}

//默认主题订阅回调函数
static void defaultMessageArrived(MQTT_Handle handle, MQTT_MessageData *data)
{
    EX_MQTT_DEMO_PRINTF_INFO("MQTT default arrived topic [%u][%.*s]\r\n",
                            data->topicName->lenstring.len, data->topicName->lenstring.len, data->topicName->lenstring.data);
    EX_MQTT_DEMO_PRINTF_INFO("MQTT default arrived data [%u][%.*s]\r\n",
                            data->message->payloadlen, data->message->payloadlen, data->message->payload);
}

static void MQTT_EventNotifyFunc(MQTT_Handle handle, uint32_t eventId, void* eventParam)
{
    MQTT_DEMO_Event sendevent = {0};
    osStatus_t status = osStatusReserved;

    if (eventId == MQTT_NOTIFY_EVENT_CONNECT)
    {
        sendevent.eventID = MQTT_DEMO_EVENT_CONNECTED;

        status = DEMO_MQTT_MessageQueue_Send(g_MQTT_QueueId, sendevent);
    }

    if (eventId == MQTT_NOTIFY_EVENT_LOST_KEEPALIVE)
    {
        sendevent.eventID = MQTT_DEMO_EVENT_KEEPALIVE_LOST;

        status = DEMO_MQTT_MessageQueue_Send(g_MQTT_QueueId, sendevent);
    }

    if (eventId == MQTT_NOTIFY_EVENT_CLOSE_SESSION)
    {
        sendevent.eventID = MQTT_DEMO_EVENT_SESSION_CLOSED;

        status = DEMO_MQTT_MessageQueue_Send(g_MQTT_QueueId, sendevent);
    }

    EX_MQTT_DEMO_PRINTF_INFO("MQTT Notify Event[%u] Send[%d]\r\n", eventId, status);
}


//接收命令并处理的函数
static void MQTT_DEMO_Task_Entry(void *param)
{
    MQTT_Handle MQTT_handle = NULL;
    int quit = 0;
    MQTTClient_ConfigureParams configparam = {0};

    // //为上下文分配空间
    EX_MQTT_DEMO_PRINTF_INFO("MQTT Demo Task start!\r\n");

    //初始化连接参数
    char addr[] = MQTT_SERVER_ADDR;
    int  port   = MQTT_SERVER_PORT;

    MQTT_ConnectOption connectData = MQTT_CONNECTDATA_INITIALIZER;
    connectData.MQTTVersion = 4; /*3 = 3.1 4 = 3.1.1*/
    connectData.keepAliveInterval = 120;
    connectData.cleansession = 0;
    connectData.clientID.cstring = "MQTT_DEMO123";
    connectData.username.cstring = "username";
    connectData.password.cstring = "password";
    connectData.willFlag = 1; //支持遗嘱消息
    connectData.will.qos = 1;
    connectData.will.retained = 0;
    connectData.will.topicName.cstring = "last/will";
    connectData.will.message.cstring = "Clientl disconnected unexpectedly.";
    connectData.host = addr;
    connectData.port = port;
    connectData.ca_crt = g_certificate;

    uint32_t        sendbufLen = 512;
    uint32_t        readbufLen = 512;
    int             rc = 0;
    unsigned char   *sendbuf = osMalloc(sendbufLen);
    unsigned char   *readbuf = osMalloc(readbufLen);
    const char      *topicFilters[1] = { "/MQTT_TEST" };
    int             QoSs[1] = { MQTT_QOS1 };
    osStatus_t sendstatus = osStatusReserved;
    MQTT_DEMO_Event sendevent = {0};

    OS_ASSERT(sendbuf != NULL && readbuf != NULL);
    //osMemset(context, 0, sizeof(MqttDemoContext));
    osMemset(sendbuf, 0, sendbufLen);
    osMemset(readbuf, 0, readbufLen);

    //创建MQTT客户端
    rc = MQTT_CreateClient(&MQTT_handle,
                    COMMAND_TIMEOUT,
                    sendbuf,
                    sendbufLen,
                    readbuf,
                    readbufLen,
                    NULL);
    if (0 != rc)
    {
        EX_MQTT_DEMO_PRINTF_ERROR("MQTT create client failed, check param\r\n");
        goto exit;
    }
    EX_MQTT_DEMO_PRINTF_INFO("MQTT create client sucess\r\n");

    MQTT_ClientSetEventNotify(MQTT_handle, MQTT_EventNotifyFunc);
    MQTT_ClientSetDefaultMessageHandler(MQTT_handle, defaultMessageArrived);
    MQTT_ClientGetConfigure(MQTT_handle, &configparam);
    configparam.reconnEnable = 0;
    MQTT_ClientSetConfigure(MQTT_handle, configparam);


    g_MQTT_QueueId = DEMO_MQTT_MessageQueue_Create(MAX_EVENT_COUNT);

    if(g_MQTT_QueueId == NULL)
    {
        goto exit;
    }
    else
    {
        EX_MQTT_DEMO_PRINTF_INFO("MQTT create message queue sucess\r\n");
    }

    memset(&sendevent,0, sizeof(sendevent));
    do
    {
        sendevent.eventID = MQTT_DEMO_EVENT_START_CONNECT;

        sendstatus = DEMO_MQTT_MessageQueue_Send(g_MQTT_QueueId, sendevent);
        EX_MQTT_DEMO_PRINTF_INFO("MQTT send sendstatus %d\r\n", sendstatus);
    } while ( sendstatus != osOK);

    while(quit == 0)
    {
        MQTT_DEMO_Event event = {0};
        if(DEMO_MQTT_MessageQueue_Recv(g_MQTT_QueueId, &event) == osOK )
        {
            //   从消息队列获取消息
            switch (event.eventID)
            {
                case MQTT_DEMO_EVENT_START_CONNECT :
                    EX_MQTT_DEMO_PRINTF_INFO("MQTT Connected start\r\n");
                    rc = 0;
                    do
                    {
                        //osTick_t now = osTickGet();
                        if(rc != 0)
                            osDelay(5000);
                        rc = MQTT_Connect(MQTT_handle, &connectData);
                        EX_MQTT_DEMO_PRINTF_ERROR("MQTT connect is %d\r\n", rc);
                        //EX_MQTT_DEMO_PRINTF_ERROR("MQTT connect  return code from MQTT connect is %d  %llu %llu\r\n", rc, now, osTickGet());
                    } while(rc != 0);
                    EX_MQTT_DEMO_PRINTF_INFO("MQTT Connected Done %d\r\n", MQTT_IsConnected(MQTT_handle));
                    break;

                case MQTT_DEMO_EVENT_CONNECTED :
                    //SUBSCRIBE 订阅主题
                    if ((rc = MQTT_Subscribe(MQTT_handle, 1, topicFilters, QoSs, messageArrived)) != 0)
                    {
                        EX_MQTT_DEMO_PRINTF_ERROR("MQTT subscribe failed, return code from MQTT subscribe is %d\r\n", rc);
                        goto exit;
                    }
                    EX_MQTT_DEMO_PRINTF_INFO("MQTT Subscribed\r\n");
                    break;

                case MQTT_DEMO_EVENT_RECV :
                    if (event.data != NULL)
                    {
                        MQTT_DEMO_TopicData *topicdata = event.data;
                        EX_MQTT_DEMO_PRINTF_INFO("MQTT Event recv topic [%u][%.*s]\r\n", topicdata->topicLen, topicdata->topicLen, topicdata->topic);
                        EX_MQTT_DEMO_PRINTF_INFO("MQTT Event recv data [%u][%.*s]\r\n", topicdata->dataLen, topicdata->dataLen, topicdata->data);
                        if(topicdata->dataLen >= 4 && memcmp(topicdata->data, "QUIT", 4) == 0) // 订阅到QUIT内容退出线程
                        {
                            quit = 1;
                            EX_MQTT_DEMO_PRINTF_INFO("MQTT Event Quit\r\n");
                        }
                        osFree(event.data);
                    }
                    else
                    {
                        EX_MQTT_DEMO_PRINTF_INFO("MQTT Event recv data[NULL]\r\n");
                    }
                    break;

                case MQTT_DEMO_EVENT_KEEPALIVE_LOST :
                case MQTT_DEMO_EVENT_SESSION_CLOSED :
                    EX_MQTT_DEMO_PRINTF_INFO("MQTT DEMO Close proc %d\r\n", MQTT_IsConnected(MQTT_handle));
                    if (MQTT_IsConnected(MQTT_handle) == OS_TRUE)
                    {
                        EX_MQTT_DEMO_PRINTF_INFO("MQTT DEMO disconnect proc already connected\r\n");
                        //MQTT_Disconnect(MQTT_handle);
                    }
                    else
                    {
                        EX_MQTT_DEMO_PRINTF_INFO("MQTT DEMO disconnect proc\r\n");
                        memset(&sendevent,0, sizeof(sendevent));
                        sendevent.eventID = MQTT_DEMO_EVENT_START_CONNECT;
                        DEMO_MQTT_MessageQueue_Send(g_MQTT_QueueId, sendevent);
                    }
                    break;

                default:
                    break;

            }
        }
        else
        {
            EX_MQTT_DEMO_PRINTF_ERROR("MQTT Queue Get Msg Err\r\n");
        }
    }


    //UNSUBSCRIBE 取消订阅
    if ((rc = MQTT_Unsubscribe(MQTT_handle, 1, topicFilters)) != 0)
    {
        EX_MQTT_DEMO_PRINTF_ERROR("MQTT unsubscribe failed, return code from MQTT unsubscribe is %d\r\n", rc);
        goto exit;
    }
    EX_MQTT_DEMO_PRINTF_INFO("MQTT Unsubscribed\r\n");

exit:
    //DISCONNECT 主动断开连接
    if(NULL != MQTT_handle)
    {
        if (MQTT_IsConnected(MQTT_handle) == OS_TRUE)
        {
            MQTT_Disconnect(MQTT_handle);
        }

        //清空初始化
        MQTT_DestroyClient(&MQTT_handle);
        MQTT_handle = NULL;
    }

    //释放申请的资源
    if (sendbuf)
    {
        osFree(sendbuf);
        sendbuf = NULL;
    }
    if (readbuf)
    {
        osFree(readbuf);
        readbuf = NULL;
    }

    if (g_MQTT_QueueId != NULL)
    {
        DEMO_MQTT_MessageQueue_Delete(g_MQTT_QueueId);
        g_MQTT_QueueId = NULL;
    }

    EX_MQTT_DEMO_PRINTF_INFO("MQTT sample Reconn terminate.\r\n");
    g_MQTT_Thread = NULL;
    osThreadTerminate(osThreadSelf());
    return;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
static void MQTT_DEMO_Entry(char argc, char **argv)
{
    EX_MQTT_DEMO_PRINTF_DEBUG("ExMqttDemo start, argc:%d!\r\n", argc);
    osThreadAttr_t attr = {"MQTT_recon_demo", osThreadDetached, NULL, 0U, NULL, MQTT_TASK_STACK_SIZE, osPriorityBelowNormal1, 0U, 0U};

    if(g_MQTT_Thread == NULL)
    {
        g_MQTT_Thread = osThreadNew(MQTT_DEMO_Task_Entry, NULL, &attr);
        if(g_MQTT_Thread == NULL)
        {
            EX_MQTT_DEMO_PRINTF_ERROR("MQTT failed to creat task.\r\n");
            return ;
        }
    }
    else
    {
        EX_MQTT_DEMO_PRINTF_DEBUG("ExMqttDemo start already\r\n");
    }
}
#if 0
int main(void)
{
    MQTT_DEMO_Entry(0, NULL);
}
#endif
//SHELL命令
#ifdef OS_USING_SHELL
#include "nr_micro_shell.h"
NR_SHELL_CMD_EXPORT(demo_mqtt_recon, MQTT_DEMO_Entry); // "start mqtt sample"
#endif

