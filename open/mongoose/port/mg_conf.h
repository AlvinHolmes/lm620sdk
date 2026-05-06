/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        mg_conf.h
 *
 * @brief       Mongoose 配置文件.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-04-21     ict team          创建
 ************************************************************************************
 */


#ifndef _MG_CONF_H_
#define _MG_CONF_H_

/*====================
  ARCH & PLAT
 *====================*/
#define MG_ARCH                     MG_ARCH_CUSTOM

#define MG_ENABLE_ASSERT            1

#define MG_ENABLE_CUSTOM_ASSERT     1
#if MG_ENABLE_CUSTOM_ASSERT
#define MG_CUSTOM_ASSERT_INCLUDE    "os_debug.h"
#define MG_CUSTOM_ASSERT(x)         OS_ASSERT((x))
#endif

#define MG_ENABLE_CUSTOM_MILLIS     1

#define MG_ENABLE_CUSTOM_RANDOM     0

/*====================
  Memory allocator
 *====================*/
#define MG_ENABLE_CUSTOM_MEMORY     1
#if MG_ENABLE_CUSTOM_MEMORY
#define MG_CUSTOM_MEMORY_INCLUDE    "os.h"
#define MG_CUSTOM_MALLOC(s)         osMalloc((s))
#define MG_CUSTOM_CALLOC(c, s)      osCalloc((c), (s))
#define MG_CUSTOM_FREE(p)           do { if ((p) != NULL) osFree((p)); } while(0)
#endif

/*====================
  Net
 *====================*/
#define MG_ENABLE_TCPIP             0  // Mongoose built-in network stack
#define MG_ENABLE_FREERTOS_TCP      0  // Amazon FreeRTOS-TCP network stack
#define MG_ENABLE_RL                0  // ARM MDK network stack
#define MG_ENABLE_LWIP              1

#define MG_ENABLE_POLL              0
#define MG_ENABLE_EPOLL             0

#define MG_ENABLE_IPV6              0
#define MG_IPV6_V6ONLY              0  // IPv6 socket binds only to V6, not V4 address

#define MG_TLS                      MG_TLS_NONE

#define MG_IO_SIZE                  1460

#define MG_SOCK_LISTEN_BACKLOG_SIZE 3

/*====================
  HTTP & WebServer
 *====================*/
#define MG_ENABLE_SSI               1
#if MG_ENABLE_SSI
#define MG_MAX_SSI_DEPTH            5
#define MG_SSI_BUFSIZ               1024
#endif

#define MG_MAX_HTTP_HEADERS         30

#define MG_HTTP_INDEX               "index.html"

#define MG_JSON_MAX_DEPTH           30

#define MG_ENABLE_DIRLIST           0

/*====================
  FileSystem
 *====================*/
#define MG_ENABLE_FILE              1

#define MG_ENABLE_FATFS             0
#define MG_ENABLE_PACKED_FS         1

#define MG_PATH_MAX                 128

#define MG_FILE_CUSTOM_FGETC        1
#define MG_FILE_CUSTOM_STAT         1
/*====================
 * Log
 *====================*/
#define MG_ENABLE_LOG               1
#define MG_ENABLE_CUSTOM_LOG        1

#define MG_BUILD_LOG_NONE           0
#define MG_BUILD_LOG_ERROR          1
#define MG_BUILD_lOG_INFO           2
#define MG_BUILD_LOG_DEBUG          3
#define MG_BUILD_LOG_VERBOSE        4

#define MG_BUILD_LOG_LEVEL          MG_BUILD_LOG_DEBUG

/*====================
 * Misc
 *====================*/
#define MG_ENABLE_MD5               0

#define MG_DEVICE                   MG_DEVICE_CUSTOM

#define MG_OTA                      MG_OTA_CUSTOM

#endif  // End of _MG_CONF_H_

