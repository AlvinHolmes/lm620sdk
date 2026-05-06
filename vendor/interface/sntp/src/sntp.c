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
#include <os_workqueue.h>
#include "lwip/api.h"
#include "lwip/apps/sntp.h"
#include "lwip/sockets.h"
#include "drv_rtc.h"


#define ZLOG_TARGET   "sntp"
#include "zlog.h"
/************************************************************************************
 *                                 内部函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define SNTP_SERVER "cn.ntp.org.cn"
#define SNTP_SERVER_ALIYUN "ntp.aliyun.com"
#define SNTP_TIMEOUT_DEFAULT 30
#define SNTP_FAIL_PERIOD 30000
#define SNTP_SUCCESS_PERIOD 3600000
#define ONE_ZONE_SECOND 900   // 15min

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static struct osWork work_sntp = {0};

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void SntpReplyCallback(uint8_t user_id, int32_t reply_time)
{
    RTC_Time local_timestamp = 0;
    int8_t zone = 0;
    struct RTC_Date local = {0};

    ZLOGD("sntp_timestamp: %d \r\n", reply_time);

    if(reply_time > 0)
    {
        RTC_GetZone(&zone);
        local_timestamp = reply_time + zone * ONE_ZONE_SECOND;
        RTC_SetTime(local_timestamp);

        local = RTC_LocalTime(&local_timestamp);
        ZLOGD("RTC_SetTime timestamp: %lld, %d-%02d-%02d %02d:%02d:%02d \r\n", local_timestamp, local.tm_year + 1900, local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);
    }
}

static int SntpStart(void)
{
    struct netif * netif = NULL;

    if(sntp_enabled())
    {
        sntp_stop_single();
    }
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, SNTP_SERVER_ALIYUN);
    netif = netif_get_by_cid(1);
    if(netif == NULL)
    {
        ZLOGE("pdp is not active \r\n");
        return -1;
    }
    if(netif_is_up(netif) == 0)
    {
        ZLOGE("net is not up \r\n");
        return -2;
    }

    if(sntp_init_single(0, LWIP_IANA_PORT_SNTP, SNTP_TIMEOUT_DEFAULT, SntpReplyCallback, netif) != ERR_OK)
    {
        ZLOGE("sntp_init_single fail \r\n");
        return -3;
    }

    return 0;
}

static void sntp_work_func(void * data)
{
    if(SntpStart() == 0)
    {
        osWorkSubmit(&work_sntp, SNTP_SUCCESS_PERIOD);
    }
    else
    {
        osWorkSubmit(&work_sntp, SNTP_FAIL_PERIOD);
    }
}

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
int main_sntp(void)
{
    ZLOGD("sntp running \r\n");

    osWorkInit(&work_sntp, sntp_work_func, NULL, OS_TRUE);

    osWorkSubmit(&work_sntp, 5000);

    return 0;
}
INIT_APP_EXPORT(main_sntp, OS_INIT_SUBLEVEL_LOW);