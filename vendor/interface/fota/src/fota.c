/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-01     ict team          创建
  ************************************************************************************
  */

 /************************************************************************************
  *                                 头文件
  ************************************************************************************/
#include <os.h>
#include <stdio.h>
#include "http_application_api.h"
#include "ota_user.h"
#include "fota.h"


#define ZLOG_TARGET   "fota"
#include "zlog.h"
/************************************************************************************
 *                                 内部函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define HTTP_GET_HEADER_LEN                 (512)
#define HTTP_GET_RSP_DATA_LEN               (1024 * 8)
#define HTTP_GET_LEN_PER_PKG                (1024 * 4)
#define HTTP_GET_RETRY_COUNT_MAX            (3)     /* OTA HTTP(S) 升级下载重传最大次数 */

#define HTTP_CONNECT_KEEPALIVE              "connection: keep-alive"

#define HTTP_RESPONSE_CODE_OK                200
#define HTTP_RESPONSE_CODE_CREATE            201
#define HTTP_RESPONSE_CODE_ACCEPTED          202
#define HTTP_RESPONSE_CODE_NON_AUTHORITATIVE 203
#define HTTP_RESPONSE_CODE_NO_CONTENT        204
#define HTTP_RESPONSE_CODE_RESET_CONTENT     205
#define HTTP_RESPONSE_CODE_PARTIAL_CONTENT   206
#define HTTP_RESPONSE_CODE_MULTI_STATUS      207

#define OTA_FILE_NAME "/usr/ota.img"

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static OTA_PackageHeadzone ota_header = {0};

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void PrintLongString(const char * string, int len)
{
    for(int i = 0;i < len;i++)
    {
        ZLOGN("%c", string[i]);
    }
    ZLOGN("\r\n");
}

static FILE * OpenOtaFile(const char * name)
{
    return fopen(name, "wb");
}

static int WriteOtaFile(FILE * fp, const char * data, int len)
{
    if(fp == NULL || data == NULL || len < 0)
    {
        return -1;
    }

    return fwrite(data, 1, len, fp);
}

static int CloseOtaFile(FILE * fp)
{
    return fclose(fp);
}

static int HttpGetRequestDownloadFotaFile(FILE * fp, const char * url)
{
    int ret = -1;
    http_client_t client = {0};
    http_client_data_t client_data = {0};
    int ota_total_len = 0;
    int ota_download_len = -1;
    int retry = 0;
    int result = 0;

    if(fp == NULL|| url == NULL)
    {
        ZLOGE("arg is null \r\n");
        return -1;
    }
    client.header = osMalloc(HTTP_GET_HEADER_LEN);
    client_data.response_buf = osMalloc(HTTP_GET_RSP_DATA_LEN);
    if(client.header == NULL || client_data.response_buf == NULL)
    {
        ZLOGE("malloc failed \r\n");
        return -2;
    }
    memset(client.header, 0, HTTP_GET_HEADER_LEN);
    client_data.response_buf_len = HTTP_GET_RSP_DATA_LEN;
    memset(client_data.response_buf, 0, client_data.response_buf_len);

    osSnprintf(client.header, HTTP_GET_HEADER_LEN - 1, "%s\r\n", HTTP_CONNECT_KEEPALIVE);

    ret = http_client_create(&client, url);
    if(ret != HTTP_SUCCESS)
    {
        osFree(client_data.response_buf);
        osFree(client.header);
        ZLOGE("http_client_create failed, return: %d \r\n", ret);
        return -3;
    }

    client_data.range_start = 0;
    client_data.range_len = HTTP_GET_LEN_PER_PKG;

    ZLOGD("http_client_get start\r\n");
    do{
        retry++;
        ret = http_client_get(&client, url, &client_data);
        if(ret != HTTP_SUCCESS)
        {
            ZLOGE("http_client_get failed, return: %d \r\n", ret);
            if(retry < HTTP_GET_RETRY_COUNT_MAX)
            {
                continue;
            }
            else
            {
                result = -4;
                break;
            }
        }
        retry = 0;
        ZLOGD("http_client_get response_code [%d]\r\n", client.response_code);
        ZLOGD("http_client_get total len [%d][%d][%d][%d] \r\n", client_data.content_range_len, client_data.retrieve_len, client_data.response_content_len, client_data.content_block_len);
        //ZLOGI("client_data.response_buf: "), PrintLongString(client_data.response_buf, client_data.response_content_len);
        if(client.response_code != HTTP_RESPONSE_CODE_OK && client.response_code != HTTP_RESPONSE_CODE_PARTIAL_CONTENT)
        {
            ZLOGE("http get request refuse \r\n");
            result = -5;
            break;
        }

        if(ota_total_len == 0)
        {
            ota_total_len = client_data.content_range_len;
            ota_download_len = 0;
        }
        ota_download_len += client_data.response_content_len;
        ZLOGD("ota downloading..., total: %d, offset: %d, len: %d \r\n", ota_total_len, ota_download_len, client_data.response_content_len);

        // TODO: 写入文件 client_data.response_buf client_data.response_content_len
        if(WriteOtaFile(fp, client_data.response_buf, client_data.response_content_len) != client_data.response_content_len)
        {
            ZLOGE("write file fail \r\n");
            result = -6;
            break;
        }

        client_data.range_start = ota_download_len;
        client_data.range_len = ota_total_len - ota_download_len < HTTP_GET_LEN_PER_PKG? ota_total_len - ota_download_len : HTTP_GET_LEN_PER_PKG;
    }while(ota_download_len < ota_total_len);
    ZLOGD("http_client_get end \r\n");

    http_client_stop(&client);
    osFree(client_data.response_buf);
    osFree(client.header);

    return result;
}



/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
int DownloadFotaFile(const char * url)
{
    FILE * fp = NULL;

    fp = OpenOtaFile(OTA_FILE_NAME);
    if(fp == NULL)
    {
        ZLOGE("open %s fail \r\n", OTA_FILE_NAME);
        return -1;
    }

    if(HttpGetRequestDownloadFotaFile(fp, url) != 0)
    {
        ZLOGE("ota file download fail \r\n");
        CloseOtaFile(fp);
        return -2;
    }
    CloseOtaFile(fp);

    return 0;
}

int InitFota(void)
{
    int ret = 0;

    if((ret = OTA_Init()) != OTA_ERROR_OK)
    {
        ZLOGE("OTA_Init fail, %d \r\n", ret);
        return -1;
    }

    return 0;
}

int CheckFotaFileFormat(void)
{
    int ret = 0;

    if((ret = OTA_CheckPackage(OTA_FILE_NAME, &ota_header)) != OTA_ERROR_OK)
    {
        ZLOGE("OTA_CheckPackage fail, %d \r\n", ret);
        return -1;
    }

    return 0;
}

int GetFotaFileVersion(char * old_version, int old_size, char * new_version, int new_size)
{
    if(old_version == NULL || new_version == NULL)
    {
        return -1;
    }
    osStrncpy(old_version, ota_header.oldVersion, old_size);
    osStrncpy(new_version, ota_header.newVersion, new_size);

    return 0;
}

int CheckFotaFileSigned(void)
{
    int ret = 0;

    if((ret = OTA_CheckSignedPackage(OTA_FILE_NAME)) != OTA_ERROR_OK)
    {
        ZLOGE("OTA_CheckSignedPackage fail, %d \r\n", ret);
        return -1;
    }

    return 0;
}

int StartupFota(void)
{
    int ret = 0;

    if((ret = OTA_StartUpgrade(OTA_FILE_NAME)) != OTA_ERROR_OK)
    {
        ZLOGE("OTA_StartUpgrade fail, %d \r\n", ret);
        return -1;
    }

    ZLOGW("startup ota, will reboot...  \r\n");
    osShutdown(OS_REBOOT, OS_REBOOT_FOTA);

    return 0;
}

int GetFotaResult(void)
{
    return OTA_GetResult();
}


#ifdef OS_USING_SHELL
#include "nr_micro_shell.h"

static void cmd_ota(void *argument)
{
    if(DownloadFotaFile((char *)argument) != 0)
    {
        shell_printf("OTA file download fail \r\n");
        return;
    }

    if(InitFota() != 0)
    {
        shell_printf("InitFota fail \r\n");
        return;
    }
    if(CheckFotaFileFormat() != 0)
    {
        shell_printf("CheckFotaFileFormat fail \r\n");
        return;
    }

    shell_printf("OTA head: %c%c%c%c \r\n", ota_header.head[0], ota_header.head[1], ota_header.head[2], ota_header.head[3]);
    shell_printf("OTA ver: %02x%02x \r\n", ota_header.ver[0], ota_header.ver[1]);
    shell_printf("OTA sver: %02x \r\n", ota_header.sver);
    shell_printf("OTA count: %02x \r\n", ota_header.count);
    shell_printf("OTA size: %u \r\n", ota_header.size);
    shell_printf("OTA obufLen: %u \r\n", ota_header.obufLen);
    shell_printf("OTA nbufLen: %u \r\n", ota_header.nbufLen);
    shell_printf("OTA dbufLen: %u \r\n", ota_header.dbufLen);
    shell_printf("OTA old version: %s \r\n", ota_header.oldVersion);
    shell_printf("OTA new version: %s \r\n", ota_header.newVersion);

    if(CheckFotaFileSigned() != 0)
    {
        shell_printf("CheckFotaFileSigned fail \r\n");
        return;
    }

    shell_printf("OTA startup, will reboot... \r\n");
    if(StartupFota() != 0)
    {
        shell_printf("StartupFota fail \r\n");
        return;
    }
}

void http_ota(char argc, char **argv)
{
    osThreadId_t pid = 0;
    osThreadAttr_t attr = {"http_ota", osThreadDetached, NULL, 0U, NULL, 4096, osPriorityLow3, 0U, 0U};

    if(argc < 2)
    {
        shell_printf("http_ota url [path] \r\n");
        return;
    }

    pid = osThreadNew(cmd_ota, argv[1], &attr);
    OS_ASSERT(pid != NULL);

    return;
}
NR_SHELL_CMD_EXPORT(http_ota, http_ota);
#endif