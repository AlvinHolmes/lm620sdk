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
 * 2023-08-28     ict team          创建
 ************************************************************************************
 */

#ifndef __ONENET_OTA_PUB_H__
#define __ONENET_OTA_PUB_H__

#include "slog_print.h"
#include "app_at_ssl.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_PubSocketCallbackRegister                 lwip_socket_register_callback
#define OTA_PubSocketCallbackUnregister               lwip_socket_unregister_callback
#define OTA_PubTimerStart(timer, cb, ms, parameter, flag)  OTA_PubTimerStartRelaxed(timer, cb, ms, osWaitForever, parameter, flag)

#if 0
#define OTA_PUB_PRINT(format, ...)        slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_AT, format"\r\n", ##__VA_ARGS__)
#define OTA_PUB_PRINT_DEBUG(format, ...)  slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_AT, format, ##__VA_ARGS__)
#define OTA_PUB_PRINT_INFO(format, ...)   slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_AT, format, ##__VA_ARGS__)
#define OTA_PUB_PRINT_WARN(format, ...)   slogPrintf(SLOG_LEVEL_WARN, SLOG_PRINT_SUBMDL_AT, format, ##__VA_ARGS__)
#define OTA_PUB_PRINT_ERROR(format, ...)  slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_AT, format, ##__VA_ARGS__)
#else
#define OTA_PUB_PRINT(format, ...)        osPrintf(format"\r\n", ##__VA_ARGS__)
#define OTA_PUB_PRINT_DEBUG(format, ...)  osPrintf(format, ##__VA_ARGS__)
#define OTA_PUB_PRINT_INFO(format, ...)   osPrintf(format, ##__VA_ARGS__)
#define OTA_PUB_PRINT_WARN(format, ...)   osPrintf(format, ##__VA_ARGS__)
#define OTA_PUB_PRINT_ERROR(format, ...)  osPrintf(format, ##__VA_ARGS__)
#endif


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum
{
    OTA_PUB_SOCKET_CLOSE_WATI_DATA = 0,      /* 等待发送缓存区数据发送完毕后，关闭 TCP 连接 */
    OTA_PUB_SOCKET_CLOSE_NOW,                /* 立即关闭不等待缓存区数据发送完毕 */
    OTA_PUB_SOCKET_CLOSE_WAIT_2MSL,          /* 等待 2MSL (Maximum Segment Lifetime, 最大分段) 后关闭 */
    OTA_PUB_SOCKET_CLOSE_SEND_RST,           /* 向服务器发送 RST 消息重置连接后关闭 */
}OTA_PubSocketCloseMode; /* TCP/IP 关闭模式 */


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
void OTA_PubBufFree(char *buf);
AT_SslContextEx* OTA_PubSslMbedtlsEstablish(const char *host, uint16_t port, uint8_t cid, uint8_t sslCtxId, uint8_t connFlag);
int32_t OTA_PubSetSockoptLinger(int fd, uint8_t closeMode);
int32_t OTA_PubTimerStartRelaxed(struct osTimer **timer, void (*cb)(void *parameter), uint32_t ms, uint32_t relaxed_ms, void *parameter, uint8_t flag);
void OTA_PubTimerStop(struct osTimer **timer);
void OTA_PubMutexLock(void);
void OTA_PubMutexUnlock(void);
int32_t OTA_PubMutexCreate(void);
void OTA_PubMutexDelete(void);


#ifdef __cplusplus
}
#endif
#endif

