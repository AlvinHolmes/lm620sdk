/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file example_http.c
*
* @brief  http 示例文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <vfs.h>
#include <ota_user.h>
#include "os.h"
#include "lwip/sockets.h"
#include "http_application_api.h"
#include "nr_micro_shell.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define DEMO_HTTP_DBG_ON
#define DEMO_HTTP_ERR_ON
#ifdef DEMO_HTTP_DBG_ON
#define DEMO_HTTP_DBG_PRINTF         osPrintf
#else
#define DEMO_HTTP_DBG_PRINTF(...)
#endif

#ifdef DEMO_HTTP_ERR_ON
#define DEMO_HTTP_ERR_PRINTF         osPrintf
#else
#define DEMO_HTTP_ERR_PRINTF(...)
#endif

#define DEMO_HTTP_TASK_STACK_SIZE    (4096)
#define DEMO_HTTP_TASK_PRIO          (18)
#define DEMO_HTTP_RSP_BUF_LEN        (2048)
#define DEMO_HTTP_MAX_COUNT          (1)
#define DEMO_HTTP_MAX_PRINT_ONCE     (126)


#define DEMO_HTTP_URL_GET            "http://httpbin.org/get"
#define DEMO_HTTP_URL_POST           "http://httpbin.org/post"
#define DEMO_HTTP_URL_PUT            "http://httpbin.org/put"
//#define DEMO_HTTPS_URL_GET           "https://httpbin.org/get"
#define DEMO_HTTPS_URL_GET           "https://jbd-ota.oss-cn-beijing.aliyuncs.com/24sa02/img2/24.02.20.03.jbd.img"

#define DEMO_HTTPS_URL_POST          "https://httpbin.org/post"
#define DEMO_HTTPS_URL_PUT           "https://httpbin.org/put"
#define DEMO_HTTP_FILE_PATH          "/usr/"     /* 文件存放路径 */
#define DEMO_HTTP_FILE_NAME          "demo_http.txt"

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/

static void DEMO_HttpBufPrint(char *buf, const char *func)
{
    int i = 0;
    char tmpBuf[DEMO_HTTP_MAX_PRINT_ONCE + 1] = {0};
    if(NULL == buf)
    {
        return;
    }
    if(strlen(buf) > DEMO_HTTP_RSP_BUF_LEN)
    {
        DEMO_HTTP_ERR_PRINTF("%s, buf overflow\r\n", __FUNCTION__);
        return ;
    }
    i = strlen(buf) / (DEMO_HTTP_MAX_PRINT_ONCE + 1);
    DEMO_HTTP_DBG_PRINTF("%s, buf: \r\n", func);
    for(int j = 0; j <= i; j++)
    {
        memcpy(tmpBuf, buf + j * DEMO_HTTP_MAX_PRINT_ONCE, DEMO_HTTP_MAX_PRINT_ONCE);
        tmpBuf[DEMO_HTTP_MAX_PRINT_ONCE] = '\0';
        DEMO_HTTP_DBG_PRINTF("%s", tmpBuf);
        memset(tmpBuf, 0, strlen(tmpBuf));
    }
    DEMO_HTTP_DBG_PRINTF("\r\n");
    return;
}

static char* DEMO_HttpFilePathGet(char *fileName)
{
    char *filePath = OS_NULL;
    uint8_t filePathLen = 0;

    if(OS_NULL == fileName)
    {
        DEMO_HTTP_ERR_PRINTF("%s, filename is null\r\n", __FUNCTION__);
        return OS_NULL;
    }

    /* 获取 /usr/ 路径下的文件名 fileName */
    filePathLen = strlen(DEMO_HTTP_FILE_PATH) + strlen(fileName) + 1;
    filePath = osMalloc(filePathLen);
    if (filePath != NULL)
    {
        memset(filePath, 0, filePathLen);

        strcat(filePath, DEMO_HTTP_FILE_PATH);
        strcat(filePath, fileName);
    }
    return filePath;
}

/* 从服务器上获取数据 */
static void DEMO_HttpGet(const char *url)
{     //1.初始化
    int ret = -1;
    http_client_t client = {0};
    http_client_data_t clientData = {0};
    char *buf = NULL;                               //buf用于存储服务器返回的响应数据。
    int bufLen = DEMO_HTTP_RSP_BUF_LEN;

    if(NULL == url)
    {
        DEMO_HTTP_ERR_PRINTF("%s: url is null\r\n", __FUNCTION__);
        return;
    }

    buf = osMalloc(bufLen);
    if(NULL == buf)
    {
        DEMO_HTTP_ERR_PRINTF("DEMO_HttpGet:buf malloc failed\r\n");
        return;
    }
    memset(buf, 0, bufLen);
    clientData.response_buf = buf;
    clientData.response_buf_len = bufLen;
            //2.建立连接
    ret = http_client_create(&client, url);
    if(!ret)
    {    //3.发起GET请求
        ret = http_client_head(&client, url, &clientData);
        DEMO_HTTP_DBG_PRINTF("http_client_head, ret [%d], response_code [%d]\r\n", ret, client.response_code);
        DEMO_HTTP_DBG_PRINTF("http_client_head total len [%d][%d][%d][%d]\r\n", clientData.content_range_len, \
                clientData.retrieve_len, clientData.response_content_len, clientData.content_block_len);
        DEMO_HttpBufPrint(buf, "http_client_head");
        clientData.range_start = 0;
        clientData.range_len = 1024;

        ret = http_client_get(&client, url, &clientData);
    }
            //4.打印响应的错误码、状态码、响应内容
    DEMO_HTTP_DBG_PRINTF("http_client_get, ret [%d], response_code [%d]\r\n", ret, client.response_code);
    DEMO_HTTP_DBG_PRINTF("http_client_get total len [%d][%d][%d][%d]\r\n", clientData.content_range_len, \
            clientData.retrieve_len, clientData.response_content_len, clientData.content_block_len);
    DEMO_HttpBufPrint(buf, "http_client_get");
            //5.断开与服务器的连接
    http_client_stop(&client);
    if(NULL != buf)
    {
        osFree(buf);
        buf = NULL;
    }
    return;
}

/* 向服务器上传送数据 */
static void DEMO_HttpPost(const char *url)
{     //1.初始化
    int ret = -1;
    char *contentType = "text/plain";
    char *postData = "http post ok!";
    http_client_t client = {0};
    http_client_data_t clientData = {0};
    char *buf = NULL;                                   //buf用于存储服务器返回的响应数据。
    int bufLen = DEMO_HTTP_RSP_BUF_LEN;

    if(NULL == url)
    {
        DEMO_HTTP_ERR_PRINTF("%s: url is null\r\n", __FUNCTION__);
        return;
    }

    buf = osMalloc(bufLen);
    if(NULL == buf)
    {
        DEMO_HTTP_ERR_PRINTF("DEMO_HttpPost:buf malloc failed\r\n");
        return;
    }
    memset(buf, 0, bufLen);

    clientData.response_buf = buf;
    clientData.response_buf_len = bufLen;
    clientData.post_buf = postData;
    clientData.post_buf_len = strlen(postData);
    clientData.post_content_type = contentType;
       //2.建立连接
    ret = http_client_create(&client, url);
    if(!ret)
    {//3.发起POST请求
        ret = http_client_post(&client, url, &clientData);
    }
        //4.打印响应的错误码、状态码、响应内容
    DEMO_HTTP_DBG_PRINTF("DEMO_HttpPost, ret [%d], response_code [%d]\r\n", ret, client.response_code);
    DEMO_HttpBufPrint(buf, __FUNCTION__);
        //5.断开与服务器的连接
    http_client_stop(&client);
    if(NULL != buf)
    {
        osFree(buf);
        buf = NULL;
    }
    return;
}

/* 向服务器指定位置上传送数据 */
static void DEMO_HttpPut(const char *url)
{     //1.初始化
    int ret = -1;
    char *contentType = "text/plain";
    char *putData = "http put ok!";
    http_client_t client = {0};
    http_client_data_t clientData = {0};
    char *buf = NULL;                                   //buf用于存储服务器返回的响应数据。
    int bufLen = DEMO_HTTP_RSP_BUF_LEN;

    if(NULL == url)
    {
        DEMO_HTTP_ERR_PRINTF("%s: url is null\r\n", __FUNCTION__);
        return;
    }

    buf = osMalloc(bufLen);
    if(NULL == buf)
    {
        DEMO_HTTP_ERR_PRINTF("DEMO_HttpPut:buf malloc failed\r\n");
        return;
    }
    memset(buf, 0, bufLen);

    clientData.response_buf = buf;
    clientData.response_buf_len = bufLen;
    clientData.post_buf = putData;
    clientData.post_buf_len = strlen(putData);
    clientData.post_content_type = contentType;
        //2.建立连接
    ret = http_client_create(&client, url);
    if(!ret)
    {//3.发起PUT请求
        ret = http_client_put(&client, url, &clientData);
    }
        //4.打印响应的错误码、状态码、响应内容
    DEMO_HTTP_DBG_PRINTF("DEMO_HttpPut, ret [%d], response_code [%d]\r\n", ret, client.response_code);
    DEMO_HttpBufPrint(buf, __FUNCTION__);
        //5.断开与服务器的连接
    http_client_stop(&client);
    if(NULL != buf)
    {
        osFree(buf);
        buf = NULL;
    }
    return;
}

/* 从服务器上获取文件 */
static void DEMO_HttpGetFile(const char *url)
{     //1.初始化
    int ret = -1;
    http_client_t client = {0};
    http_client_data_t clientData = {0};
    char *fileName = DEMO_HTTP_FILE_NAME;        //保存响应数据的文件名
    char *filePath = OS_NULL;

    if(NULL == url)
    {
        DEMO_HTTP_ERR_PRINTF("%s: url is null\r\n", __FUNCTION__);
        return;
    }
       //2.获取文件路径
    filePath = DEMO_HttpFilePathGet(fileName);
    if(NULL == filePath)
    {
        DEMO_HTTP_ERR_PRINTF("%s, filePath is null\r\n", __FUNCTION__);
        return;
    }
       //3.建立连接
    DEMO_HTTP_ERR_PRINTF("%s, %d, filePath is [%s]\r\n", __FUNCTION__, __LINE__, filePath);
    ret = http_client_create(&client, url);
    DEMO_HTTP_ERR_PRINTF("%s, %d, filePath is [%s]\r\n", __FUNCTION__, __LINE__, filePath);
    if(!ret)
    {//4.发起GetFile请求
        DEMO_HTTP_ERR_PRINTF("%s, %d, filePath is [%s]\r\n", __FUNCTION__, __LINE__, filePath);
        ret = http_client_get_file(&client, url, &clientData, (const char *)filePath);
    }
        //5.打印响应的错误码、状态码
    DEMO_HTTP_DBG_PRINTF("DEMO_HttpGetFile, ret [%d], response_code [%d]\r\n", ret, client.response_code);
        //6.断开与服务器的连接
    http_client_close(&client);
    if (NULL != filePath)
    {
        osFree(filePath);
        filePath = NULL;
    }
    return;
}

/* 向服务器上传送文件 */
static void DEMO_HttpPostFile(const char *url)
{     //1.初始化
    int ret = -1;
    http_client_t client = {0};
    http_client_data_t clientData = {0};
    char *buf = NULL;                           //用于接收服务器响应数据
    int bufLen = DEMO_HTTP_RSP_BUF_LEN;
    char *fileName = DEMO_HTTP_FILE_NAME;        //要发送的文件
    char *filePath = OS_NULL;

    if(NULL == url)
    {
        DEMO_HTTP_ERR_PRINTF("%s: url is null\r\n", __FUNCTION__);
        return;
    }

    buf = osMalloc(bufLen);
    if(NULL == buf)
    {
        DEMO_HTTP_ERR_PRINTF("%s, buf malloc failed\r\n", __FUNCTION__);
        return;
    }
    memset(buf, 0, bufLen);
        //2.获取文件路径
    filePath = DEMO_HttpFilePathGet(fileName);
    if(NULL == filePath)
    {
        DEMO_HTTP_ERR_PRINTF("%s, filePath is null\r\n", __FUNCTION__);
        return;
    }

    clientData.response_buf = buf;
    clientData.response_buf_len = bufLen;
        //3.建立连接
    ret = http_client_create(&client, url);
    if(!ret)
    {//4.发起PostFile请求
        ret = http_client_post_file(&client, url, &clientData, filePath, HTTP_POST, NULL);
    }
        //5.打印响应的错误码、状态码、响应内容
    DEMO_HTTP_DBG_PRINTF("DEMO_HttpPostFile, ret [%d], response_code [%d]\r\n", ret, client.response_code);
    DEMO_HttpBufPrint(buf, __FUNCTION__);
        //6.断开与服务器的连接
    http_client_close(&client);
    if(NULL != buf)
    {
        osFree(buf);
        buf = NULL;
    }
    if (NULL != filePath)
    {
        osFree(filePath);
        filePath = NULL;
    }
    return;
}

static void DEMO_HttpTask(void *parameter)
{
    uint16_t count = 0;
    uint8_t isHttp = (uint8_t)(uintptr_t)(parameter);
    DEMO_HTTP_DBG_PRINTF("Demo http task isHttp[%d]\r\n", isHttp);
    while(count++ < DEMO_HTTP_MAX_COUNT)
    {
        DEMO_HttpGet(isHttp ? DEMO_HTTP_URL_GET : DEMO_HTTPS_URL_GET);
        DEMO_HttpPost(isHttp ? DEMO_HTTP_URL_POST : DEMO_HTTPS_URL_POST);
        DEMO_HttpPut(isHttp ? DEMO_HTTP_URL_PUT : DEMO_HTTPS_URL_PUT);
        DEMO_HttpGetFile(isHttp ? DEMO_HTTP_URL_GET : DEMO_HTTPS_URL_GET);
        DEMO_HttpPostFile(isHttp ? DEMO_HTTP_URL_POST : DEMO_HTTPS_URL_POST);
        osDelay(200);
    }
    DEMO_HTTP_DBG_PRINTF("Demo http task exit\r\n");
    return;
}

static void DEMO_HttpTaskStart(uint8_t isHttp)
{
    DEMO_HTTP_DBG_PRINTF("%s, isHttp[%d]\r\n", __FUNCTION__, isHttp);
    osThreadId_t DemoHttpTask = NULL;
    osThreadAttr_t attr = {"demo_http", osThreadDetached, NULL, 0U, NULL, DEMO_HTTP_TASK_STACK_SIZE,
                            DEMO_HTTP_TASK_PRIO, 0U, 0U};
    DemoHttpTask = osThreadNew(DEMO_HttpTask, (void *)(uintptr_t)isHttp, &attr);
    if (OS_NULL != DemoHttpTask)
    {
        DEMO_HTTP_DBG_PRINTF("%s ok.\r\n", __FUNCTION__);
    }
    else
    {
        DEMO_HTTP_ERR_PRINTF("%s failed\r\n", __FUNCTION__);
    }
    osThreadJoin(DemoHttpTask);
    DemoHttpTask = NULL;
    return;
}

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
static void DEMO_HttpEntry(char argc, char **argv)
{
    if(argc != 2)
    {
        DEMO_HTTP_ERR_PRINTF("Input format:example_http + [num] \r\nFor example:example_http 1 \r\n");
        DEMO_HTTP_ERR_PRINTF("num:\r\n1:http\r\n0:https\r\n");
    }
    else
    {
        uint8_t ishttp = atoi(argv[1]);
        DEMO_HttpTaskStart(ishttp);
    }
}


#define DEMO_HTTP_DBG_PRINTF         osPrintf
#define DEMO_HTTP_ERR_PRINTF         osPrintf

#define DEMO_HTTP_GET_TASK_STACK_SIZE           (1024 * 8)
#define DEMO_HTTP_GET_TASK_PRIO                 (18)
#define DEMO_HTTP_GET_RSP_BUF_LEN               (1024 * 8)
#define DEMO_HTTP_GET_LEN_PER_PKG               (1024 * 4)
#define DEMO_HTTP_GET_RETRY_COUNT_MAX           (3)     /* OTA HTTP(S) 升级下载重传最大次数 */
#define DEMO_HTTP_GET_MAX_PRINT_ONCE     (126)
#define DEMO_HTTP_CONNECT_KEEPALIVE             "Connection: keep-alive"
#define DEMO_HTTP_SAVE_FILE_NAME                "/usr/http_bin"

#define DEMO_HTTP_GET_URL_GET                   "http://jbd-ota.oss-cn-beijing.aliyuncs.com/24sa02/img2/24.02.20.03.jbd.img"
//#define DEMO_HTTP_GET_URL_GET                   "http://jbd-ota.oss-cn-beijing.aliyuncs.com/24sa02/24.02.20.03.jbd.txt"
#define DEMO_HTTPS_GET_URL_GET                  "https://jbd-ota.oss-cn-beijing.aliyuncs.com/24sa02/img2/24.02.20.03.jbd.img"

typedef struct
{
    uint8_t retryCnt;        /* 升级下载重传次数 */
    uint32_t offset;         /* 升级版本的文件下载偏移量 */
    uint32_t totalSize;      /* 升级版本的文件大小 */
    int rangeStart;          /**< range start position in request header. */
    int rangeLen;            /**< range length in request header. */
}DEMO_HttpParam; /* ONENET OTA 升级参数信息 */

DEMO_HttpParam g_HttpParam = {0};

static void DEMO_HttpGetBufPrint(char *buf, int len, const char *func)
{
    int i = 0;
    char tmpBuf[DEMO_HTTP_GET_MAX_PRINT_ONCE + 1] = {0};
    if(NULL == buf)
    {
        return;
    }
    if(len > DEMO_HTTP_GET_RSP_BUF_LEN)
    {
        DEMO_HTTP_ERR_PRINTF("%s, buf overflow\r\n", __FUNCTION__);
        return ;
    }
    i = len / (DEMO_HTTP_GET_MAX_PRINT_ONCE + 1);
    DEMO_HTTP_DBG_PRINTF("%s, buf: \r\n", func);
    for(int j = 0; j <= i; j++)
    {
        memcpy(tmpBuf, buf + j * DEMO_HTTP_GET_MAX_PRINT_ONCE, DEMO_HTTP_GET_MAX_PRINT_ONCE);
        tmpBuf[DEMO_HTTP_GET_MAX_PRINT_ONCE] = '\0';
        DEMO_HTTP_DBG_PRINTF("%s", tmpBuf);
        memset(tmpBuf, 0, strlen(tmpBuf));
    }
    DEMO_HTTP_DBG_PRINTF("\r\n");
    return;
}

static void DEMO_HttpAutoSetKeepalive(int fd)
{
    int keepaliveEnable = 1;
    int keepidle = 5;
    int keepinterval = 5;
    int keepcount = 3;

    DEMO_HTTP_ERR_PRINTF("AutoSetKeepalive, fd[%d][%d]\r\n", fd, keepaliveEnable);
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepaliveEnable, sizeof(keepaliveEnable));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepinterval, sizeof(keepinterval));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &keepcount, sizeof(keepcount));
    DEMO_HTTP_ERR_PRINTF("AutoSetKeepalive ok [%d][%d][%d][%d]\r\n", keepaliveEnable, keepidle, keepinterval, keepcount);

    return;
}

/* 封装 HTTP client 头部信息 */
static int DEMO_HttpHeaderFill(http_client_t *client)
{
    int len = strlen(DEMO_HTTP_CONNECT_KEEPALIVE) + 3;
    if(NULL == client->header)
    {
        client->header = osMalloc(len);
        if(NULL == client->header)
        {
            DEMO_HTTP_ERR_PRINTF("ota header malloc fail\r\n");
            return -1;
        }
    }
    memset(client->header, 0, len);

    osSnprintf(client->header, len, "%s\r\n", DEMO_HTTP_CONNECT_KEEPALIVE);

    return 0;
}

/* 保存下载到的升级包*/
int32_t DEMO_HttpSaveFirmware(const void *buf, uint32_t len, char *pathName)
{
    VFS_File *fpPkg = NULL;
    size_t ret = 0;

    DEMO_HTTP_DBG_PRINTF("ota upgrade save pathName is [%s]\r\n", pathName);
    fpPkg = VFS_OpenFile(pathName, "ab");
    if(OS_NULL == fpPkg)
    {
        DEMO_HTTP_ERR_PRINTF("ota upgrade save open file fail\r\n");
        return -1;
    }

    ret = VFS_WriteFile(buf, 1, len, fpPkg);
    if(ret != len)
    {
        DEMO_HTTP_ERR_PRINTF("ota upgrade write file fail\r\n");
        return -2;
    }
    VFS_SyncFile(fpPkg);

    VFS_CloseFile(fpPkg);
    fpPkg = NULL;

    return 0;
}

static void DEMO_HttpGetFileProcess(const char *url)
{     //1.初始化
    int ret = -1;
    http_client_t client = {0};
    http_client_data_t clientData = {0};
    char *buf = NULL;                               //buf用于存储服务器返回的响应数据。
    int rspBufLen = DEMO_HTTP_GET_RSP_BUF_LEN;
    int dlLen = -1;
    char *pathName = DEMO_HTTP_SAVE_FILE_NAME;

    if(NULL == url)
    {
        DEMO_HTTP_ERR_PRINTF("%s: url is null\r\n", __FUNCTION__);
        return;
    }

    buf = osMalloc(rspBufLen);
    if(NULL == buf)
    {
        DEMO_HTTP_ERR_PRINTF("DEMO_HttpGet:buf malloc failed\r\n");
        return;
    }
    memset(buf, 0, rspBufLen);
    clientData.response_buf = buf;
    clientData.response_buf_len = rspBufLen;

    if(0 != DEMO_HttpHeaderFill(&client))
    {
        DEMO_HTTP_ERR_PRINTF("DEMO_HttpHeaderFill failed\r\n");
        return;
    }

            //2.建立连接
    ret = http_client_create(&client, url);
    if(!ret)
    {
        clientData.range_start = 0;
        clientData.range_len = DEMO_HTTP_GET_LEN_PER_PKG;
        memset(&g_HttpParam, 0, sizeof(DEMO_HttpParam));
        //DEMO_HttpAutoSetKeepalive(client.socket);
        //while(1)
        {
            ;
        }
        // 3.发送get
        DEMO_HTTP_DBG_PRINTF("http_client_get start\r\n");
        ret = http_client_get(&client, url, &clientData);
                //4.打印响应的错误码、状态码、响应内容
        DEMO_HTTP_DBG_PRINTF("http_client_get, ret [%d], response_code [%d]\r\n", ret, client.response_code);
        DEMO_HTTP_DBG_PRINTF("http_client_get total len [%d][%d][%d][%d]\r\n", clientData.content_range_len, \
                clientData.retrieve_len, clientData.response_content_len, clientData.content_block_len);
        DEMO_HttpGetBufPrint(clientData.response_buf, clientData.response_content_len, "http_client_get");

        ret = DEMO_HttpSaveFirmware(clientData.response_buf, clientData.response_content_len, pathName);
        if (0 != ret)
        {
            DEMO_HTTP_ERR_PRINTF("Save first package data fail[%d]\r\n", ret);
            if(NULL != client.header)
            {
                osFree(client.header);
                client.header = NULL;
            }
            if(NULL != buf)
            {
                osFree(buf);
                buf = NULL;
            }
            return;
        }

        g_HttpParam.totalSize = clientData.content_range_len;
        dlLen = clientData.response_content_len;
        g_HttpParam.offset += dlLen;
        while(g_HttpParam.offset < g_HttpParam.totalSize)
        {
            g_HttpParam.retryCnt = 0;
            dlLen = g_HttpParam.totalSize - g_HttpParam.offset;
            dlLen = dlLen < DEMO_HTTP_GET_LEN_PER_PKG ? dlLen : DEMO_HTTP_GET_LEN_PER_PKG;
            clientData.range_start = g_HttpParam.offset;
            clientData.range_len = dlLen;
            DEMO_HTTP_DBG_PRINTF("download totalSize[%u], offset[%d], dlLen[%d]\r\n", \
                g_HttpParam.totalSize, g_HttpParam.offset, dlLen);
            DEMO_HTTP_DBG_PRINTF("has download byte[%u]\r\n\r\n", g_HttpParam.offset);
            while(g_HttpParam.retryCnt <= DEMO_HTTP_GET_RETRY_COUNT_MAX)
            {
                ret = http_client_get(&client, url, &clientData);
                if(0 != ret)
                {
                    DEMO_HTTP_ERR_PRINTF("ota donwload retry count [%d], ret[%d]\r\n", g_HttpParam.retryCnt, ret);
                    if (g_HttpParam.retryCnt > DEMO_HTTP_GET_RETRY_COUNT_MAX)
                    {
                        DEMO_HTTP_ERR_PRINTF("ota donwload reach retry count [%d]\r\n", g_HttpParam.retryCnt);
                        break;
                    }
                    else
                    {
                        g_HttpParam.retryCnt += 1;
                    }
                }
                else
                {
                    DEMO_HTTP_DBG_PRINTF("http_client_get, ret [%d], response_code [%d]\r\n", ret, client.response_code);
                    DEMO_HTTP_DBG_PRINTF("http_client_get total len [%d][%d][%d][%d]\r\n", clientData.content_range_len, \
                            clientData.retrieve_len, clientData.response_content_len, clientData.content_block_len);
                    DEMO_HttpGetBufPrint(clientData.response_buf, clientData.response_content_len, "http_client_get");
                    /* 保存文件数据 */
                    ret = DEMO_HttpSaveFirmware(clientData.response_buf, dlLen, pathName);
                    if (0 == ret)
                    {
                        DEMO_HTTP_ERR_PRINTF("Save package data ok[%d]\r\n", ret);
                        /* 保存下载进度 */
                        g_HttpParam.offset += dlLen;
                    }
                    else
                    {
                        DEMO_HTTP_ERR_PRINTF("Save package data fail[%d]\r\n", ret);
                    }
                    break;
                }
            }

            if (g_HttpParam.retryCnt > DEMO_HTTP_GET_RETRY_COUNT_MAX)
            {
                DEMO_HTTP_ERR_PRINTF("ota reach retry count [%d]\r\n", g_HttpParam.retryCnt);
                g_HttpParam.retryCnt = 0;
                break;
            }
        }

        if(g_HttpParam.offset == g_HttpParam.totalSize)
        {
            DEMO_HTTP_DBG_PRINTF("\r\nota upgrade pkg succ, upgrade it\r\n\r\n");
            //TODO: 下载完成，开始升级
        }
        else
        {
            DEMO_HTTP_ERR_PRINTF("ota donwload offset[%d] greater than totalSize[%d]\r\n", g_HttpParam.offset, g_HttpParam.totalSize);
        }
    }
    http_client_stop(&client);

    if(NULL != client.header)
    {
        osFree(client.header);
        client.header = NULL;
    }

    if(NULL != buf)
    {
        osFree(buf);
        buf = NULL;
    }

    DEMO_HTTP_DBG_PRINTF("Demo http task exit\r\n");
    return;
}

static void DEMO_HttpGetFileTask(void *parameter)
{
    uint8_t isHttp = (uint8_t)(uintptr_t)(parameter);
    DEMO_HTTP_DBG_PRINTF("Demo http task isHttp[%d]\r\n", isHttp);
    DEMO_HttpGetFileProcess(isHttp ? DEMO_HTTP_GET_URL_GET : DEMO_HTTPS_GET_URL_GET);
    DEMO_HTTP_DBG_PRINTF("Demo http task exit\r\n");
    return;
}

static void DEMO_HttpGetFileTaskStart(uint8_t isHttp)
{
    DEMO_HTTP_DBG_PRINTF("%s, isHttp[%d]\r\n", __FUNCTION__, isHttp);
    osThreadId_t DemoHttpTask = NULL;
    osThreadAttr_t attr = {"demo_http", osThreadDetached, NULL, 0U, NULL, DEMO_HTTP_GET_TASK_STACK_SIZE,
                            DEMO_HTTP_GET_TASK_PRIO, 0U, 0U};
    DemoHttpTask = osThreadNew(DEMO_HttpGetFileTask, (void *)(uintptr_t)isHttp, &attr);
    if (OS_NULL != DemoHttpTask)
    {
        DEMO_HTTP_DBG_PRINTF("%s ok.\r\n", __FUNCTION__);
    }
    else
    {
        DEMO_HTTP_ERR_PRINTF("%s failed\r\n", __FUNCTION__);
    }
    osThreadJoin(DemoHttpTask);
    DemoHttpTask = NULL;
    return;
}

static void DEMO_HttpGetFileEntry(char argc, char **argv)
{
    if(argc != 2)
    {
        return;
    }
    else
    {
        uint8_t ishttp = atoi(argv[1]);
        DEMO_HttpGetFileTaskStart(ishttp);
    }
}

/* 注册SHELL命令 */
NR_SHELL_CMD_EXPORT(demo_http, DEMO_HttpEntry);
NR_SHELL_CMD_EXPORT(demo_http_get_file, DEMO_HttpGetFileEntry); /* jia bai da demo */

