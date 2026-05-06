/**
 * @version   V1.0
 * @date      2020-08-01
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 */


#ifndef HTTP_CLIENT_WRAPPER_H
#define HTTP_CLIENT_WRAPPER_H

#include "http_client.h"
//#include "oneos_config.h"
//#include <os_util.h>


//#include <dlog.h>
//#include "os_log.h"


#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#endif

#define HTTP_TAG "HTTP_TAG"

/* HTTP DEBUG level */
#define HTTP_DBG_ERROR           0
#define HTTP_DBG_WARNING         1
#define HTTP_DBG_INFO            2
#define HTTP_DBG_LOG             3

#define HTTP_DBG_LEVEL           HTTP_DBG_ERROR

#define os_malloc                     osMalloc
#define os_free                       osFree
#define HTTP_CLIENT_DBG_PRINTF        MID_NET_PRINT_INFO
#define HTTP_CLIENT_ERR_PRINTF        MID_NET_PRINT_ERROR

#if 0
#if (HTTP_DBG_LEVEL >= HTTP_DBG_LOG)
#define LOG_D(tag, format, ...)       osPrintf("HTTP_D: "format"\r\n", ##__VA_ARGS__)
#else
#define LOG_D(...)
#endif

#if (HTTP_DBG_LEVEL >= HTTP_DBG_INFO)
#define LOG_I(tag, format, ...)       osPrintf("HTTP_I: "format"\r\n", ##__VA_ARGS__)
#else
#define LOG_I(...)
#endif

#if (HTTP_DBG_LEVEL >= HTTP_DBG_WARNING)
#define LOG_W(tag, format, ...)       osPrintf("HTTP_W: "format"\r\n", ##__VA_ARGS__)
#else
#define LOG_W(...)
#endif

#if (HTTP_DBG_LEVEL >= HTTP_DBG_ERROR)
#define LOG_E(tag, format, ...)       osPrintf("HTTP_E: "format"\r\n", ##__VA_ARGS__)
#else
#define LOG_E(...)
#endif
#else
#if (HTTP_DBG_LEVEL >= HTTP_DBG_LOG)
#define LOG_D(tag, format, ...)       slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_MID_NET, format, ##__VA_ARGS__)
#else
#define LOG_D(...)
#endif

#if (HTTP_DBG_LEVEL >= HTTP_DBG_INFO)
#define LOG_I(tag, format, ...)       slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_MID_NET, format, ##__VA_ARGS__)
#else
#define LOG_I(...)
#endif

#if (HTTP_DBG_LEVEL >= HTTP_DBG_WARNING)
#define LOG_W(tag, format, ...)       slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_MID_NET, format, ##__VA_ARGS__)
#else
#define LOG_W(...)
#endif

#if (HTTP_DBG_LEVEL >= HTTP_DBG_ERROR)
#define LOG_E(tag, format, ...)       slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_MID_NET, format, ##__VA_ARGS__)
#else
#define LOG_E(...)
#endif
#endif


int http_tcp_conn_wrapper(http_client_t *client, const char *host);
int http_tcp_conn_wrapper_extend(http_client_t *client, const char *host, uint8_t pdp_id);
int http_tcp_close_wrapper(http_client_t *client);
int http_tcp_send_wrapper(http_client_t *client, const char *data, int length);
int http_tcp_recv_wrapper(http_client_t *client, char *buf, int buflen, int timeout_ms, int *p_read_len);
int http_tcp_recv_wrapper_nonblock(http_client_t *client, char *buf, int recv_len, int *p_read_len);

#ifdef CONFIG_HTTP_SECURE
int http_ssl_conn_wrapper(http_client_t *client, const char *host);
int http_ssl_conn_wrapper_extend(http_client_t *client, const char *host, uint8_t pdp_id);
int http_ssl_close_wrapper(http_client_t *client);
int http_ssl_send_wrapper(http_client_t *client, const char *data, size_t length);
int http_ssl_recv_wrapper(http_client_t *client, char *buf, int buflen, int timeout_ms, int *p_read_len);
int http_ssl_recv_wrapper_nonblock(http_client_t *client, char *buf, int recv_len, int *p_read_len);
#endif

#endif

