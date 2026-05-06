/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file main.c
*
* @brief  main函数入口文件.
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
#ifdef MQTT_USING_TLS  //依赖MQTT协议中的宏配置
#define MQTT_SERVER_ADDR  "broker.emqx.io"
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
#define MQTT_TASK_STACK_SIZE 4096*2
#else
#define MQTT_SERVER_ADDR  "broker.emqx.io"
#define MQTT_SERVER_PORT  1883
static const char *g_certificate = NULL;
#define MQTT_TASK_STACK_SIZE 1024*4
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
/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
typedef struct
{
    MQTT_Handle handle;
} MqttDemoContext;

//线程句柄
static struct osThread *g_MqttThread = NULL;

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
//回调函数
static void messageArrived(MQTT_Handle handle, MQTT_MessageData *data)
{
    //打印消息内容
    EX_MQTT_DEMO_PRINTF_INFO("Message arrived on topic [%.*s][%s]\r\n",data->topicName->lenstring.len, data->topicName->lenstring.data, data->message->payload);
}

//接收命令并处理的函数
static void DemoHandlerTask(void *param)
{
    //创建上下文
    MqttDemoContext context;
    context.handle = NULL;

    EX_MQTT_DEMO_PRINTF_INFO("MQTT Demo Start!\n");
    // //为上下文分配空间
    // context.handle = osMalloc(sizeof(MQTTClient));
    // if(NULL == context)
    // {
    //     EX_MQTT_DEMO_PRINTF_ERROR("context malloc falied!\n");
    //     return ;
    // }

    //初始化连接参数
    char addr[] = MQTT_SERVER_ADDR;   //broker
    int  port   = MQTT_SERVER_PORT;

    MQTT_ConnectOption connectData = MQTT_CONNECTDATA_INITIALIZER;
    connectData.MQTTVersion = 4; /*3 = 3.1 4 = 3.1.1*/
    connectData.keepAliveInterval = 60;
    connectData.cleansession = 1;
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
    int             count = 0;
    int             rc = 0;
    unsigned char   *sendbuf = osMalloc(sendbufLen);
    unsigned char   *readbuf = osMalloc(readbufLen);
    const char      *topicFilters[1] = { "/MQTT_TEST" };
    int             QoSs[1] = { MQTT_QOS1 };



    OS_ASSERT(sendbuf != NULL && readbuf != NULL);
    //osMemset(context, 0, sizeof(MqttDemoContext));
    osMemset(sendbuf, 0, sendbufLen);
    osMemset(readbuf, 0, readbufLen);

    //创建MQTT客户端
    rc = MQTT_CreateClient(&context.handle,
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


    //CONNECT 与服务器建立连接
    if ((rc = MQTT_Connect(context.handle, &connectData)) != 0)
    {
        EX_MQTT_DEMO_PRINTF_ERROR("MQTT connect failed, return code from MQTT connect is %d\r\n", rc);
        goto exit;
    }
    EX_MQTT_DEMO_PRINTF_INFO("MQTT Connected\r\n");

    //SUBSCRIBE 订阅主题
    if ((rc = MQTT_Subscribe(context.handle, 1, topicFilters, QoSs, messageArrived)) != 0)
    {
        EX_MQTT_DEMO_PRINTF_ERROR("MQTT subscribe failed, return code from MQTT subscribe is %d\r\n", rc);
        goto exit;
    }
    EX_MQTT_DEMO_PRINTF_INFO("MQTT Subscribed\r\n");

    //循环发布20次消息
    while (count<20)
    {
        ++count;
        MQTT_Message message;
        char payload[128];

        message.qos = 0;
        message.retained = 0;
        message.payload = payload;          //如果创建异步发送，message.payload需要是全局内存，本示例是同步发送，所以可以使用局部内存
        osSnprintf(payload, sizeof(payload), "message number %d", count);
        message.payloadlen = strlen(payload);

        //PUBLISH 发布消息
        if ((rc = MQTT_Publish(context.handle, "/MQTT_TEST", &message)) != 0)
        {
            EX_MQTT_DEMO_PRINTF_ERROR("MQTT publish failed, return code from MQTT publish is %d\r\n", rc);
            continue;
        }
        EX_MQTT_DEMO_PRINTF_INFO("MQTT publish ok!\r\n");
        osThreadMsSleep(1000);
    }

    //UNSUBSCRIBE 取消订阅
    if ((rc = MQTT_Unsubscribe(context.handle, 1, topicFilters)) != 0)
    {
        EX_MQTT_DEMO_PRINTF_ERROR("MQTT unsubscribe failed, return code from MQTT unsubscribe is %d\r\n", rc);
        goto exit;
    }
    EX_MQTT_DEMO_PRINTF_INFO("MQTT Unsubscribed\r\n");

exit:
    //DISCONNECT 主动断开连接
    if(NULL != context.handle)
    {
        if (MQTT_IsConnected(context.handle) == OS_TRUE)
        {
            MQTT_Disconnect(context.handle);
        }

        //清空初始化
        MQTT_DestroyClient(&context.handle);
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

    EX_MQTT_DEMO_PRINTF_INFO("MQTT sample1 terminate.\r\n");
    g_MqttThread = NULL;
    osThreadTerminate(osThreadSelf());
    return;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
static void ExMqttDemo(char argc, char **argv)
{
    EX_MQTT_DEMO_PRINTF_DEBUG("ExMqttDemo start, argc:%d!\r\n", argc);
    osThreadAttr_t attr = {"MQTT_demo", osThreadDetached, NULL, 0U, NULL, MQTT_TASK_STACK_SIZE, osPriorityNormal, 0U, 0U};

    if(g_MqttThread == NULL)
    {
        g_MqttThread = osThreadNew(DemoHandlerTask, NULL, &attr);
        if(g_MqttThread == NULL)
        {
            EX_MQTT_DEMO_PRINTF_ERROR("failed to creat receiver task.\n");
            return ;
        }
    }
}

//SHELL命令
#ifdef OS_USING_SHELL
#include "nr_micro_shell.h"
NR_SHELL_CMD_EXPORT(mqtt_demo, ExMqttDemo); // "start mqtt sample"
#endif
