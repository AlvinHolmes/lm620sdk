/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief:OneNET OTA token
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-28     ict team          创建
 ************************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/sockets.h"
#include "onenet_ota_pub.h"


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_PUB_ON
 #ifdef OTA_PUB_ON
#define OTA_PUB_DBG_PRINTF                         OTA_PUB_PRINT_DEBUG
#else
#define OTA_PUB_DBG_PRINTF(...)
#endif
#define OTA_PUB_ERR_PRINTF                         OTA_PUB_PRINT_ERROR


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static osMutex_t g_OtaPubMutex = NULL;  /* 互斥锁 */


/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/


/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
/* 释放内存 */
void OTA_PubBufFree(char *buf)
{
     if(NULL != buf)
     {
         osFree(buf);
     }
     return;
}

AT_SslContextEx* OTA_PubSslMbedtlsEstablish(const char *host, uint16_t port, uint8_t cid, uint8_t sslCtxId, uint8_t connFlag)
{
    return AT_SslMbedtlsEstablish(host, port, cid, sslCtxId, connFlag);
}

 /* 设置套接字属性( TCP 关闭方式属性参数 linger) setsockopt */
int32_t OTA_PubSetSockoptLinger(int fd, uint8_t closeMode)
{
    int32_t ret = -1;
    struct linger soLinger;

    switch(closeMode)
    {
        case OTA_PUB_SOCKET_CLOSE_WATI_DATA:
            soLinger.l_onoff = 0;
            soLinger.l_linger = 0;
            break;

        case OTA_PUB_SOCKET_CLOSE_NOW:
        case OTA_PUB_SOCKET_CLOSE_SEND_RST:
            soLinger.l_onoff = 1;
            soLinger.l_linger = 0;
            break;

        case OTA_PUB_SOCKET_CLOSE_WAIT_2MSL:
            soLinger.l_onoff = 1;
            soLinger.l_linger = 2 * TCP_MSL;
            break;

        default:
            break;
    }

    ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, &soLinger, sizeof(soLinger));
    if(ret != 0)
    {
        OTA_PUB_ERR_PRINTF("set socket option linger fail [%d]\r\n", ret);
        return osError;
    }

    return osOK;
}

/* 启动定时器 */
int32_t OTA_PubTimerStartRelaxed(struct osTimer **timer, void (*cb)(void *parameter), uint32_t ms, uint32_t relaxed_ms, void *parameter, uint8_t flag)
{
    osStatus_t ret;

    if(NULL != *timer)
    {
        return osError;
    }

    *timer = (struct osTimer *)osMalloc(sizeof(struct osTimer));
    if (*timer == NULL)
    {
        OTA_PUB_ERR_PRINTF("%s mem alloc fail\r\n", __func__);
        return osErrorTimeout;
    }
    osTimerInit(*timer, cb, parameter, 0xffff, flag);

    ret = osTimerStartRelaxedMs(*timer, ms, relaxed_ms);
    if(osOK != ret)
    {
        OTA_PUB_ERR_PRINTF("timer start fail %d\r\n", ret);
        osTimerDelete(*timer);
        osFree(*timer);
        *timer = NULL;
        return osErrorResource;
    }

    return osOK;
}

/* 停止定时器 */
void OTA_PubTimerStop(struct osTimer **timer)
{
    if(NULL != *timer)
    {
        osTimerStop(*timer);    /* stop timer */
        osTimerDelete(*timer);  /* delete timer */
        osFree(*timer);
        *timer = NULL;
    }

    return;
}

void OTA_PubMutexLock(void)
{
    osMutexAcquire(g_OtaPubMutex, osWaitForever);
    return;
}

void OTA_PubMutexUnlock(void)
{
    osMutexRelease(g_OtaPubMutex);
    return;
}

/* 创建互斥锁 */
int32_t OTA_PubMutexCreate(void)
{
    osMutexAttr_t attr;

    memset(&attr, 0 ,sizeof(attr));
    attr.attr_bits = osMutexPrioInherit | osMutexRecursive;

    if(NULL == g_OtaPubMutex)
    {
        g_OtaPubMutex = (osMutex_t)osMutexNew(&attr);
        OS_ASSERT(g_OtaPubMutex != NULL);
    }
    else
    {
        OTA_PUB_ERR_PRINTF("g_OtaPubMutex already create, fail\r\n");
        return osError;
    }

    return osOK;
}

/* 释放互斥锁 */
void OTA_PubMutexDelete(void)
{
    if(NULL != g_OtaPubMutex)
    {
        osMutexDelete(g_OtaPubMutex);
        g_OtaPubMutex = NULL;
    }

    return;
}

#ifdef __cplusplus
}
#endif

