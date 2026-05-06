#ifndef __RWS_PORT_H__
#define __RWS_PORT_H__ 1
//  移植过程中新增文件
#include <stdio.h>
#include "cmsis_os2.h"
#include "os.h"

typedef uint8_t  rt_uint8_t;
typedef size_t   rt_size_t;

//  移植过程中新增文件适配开始
#define EWS_LOG_LEVEL_NONE      0
#define RWS_LOG_LEVEL_DBG       4
#define RWS_LOG_LEVEL_INFO      3
#define RWS_LOG_LEVEL_WARN      2
#define EWS_LOG_LEVEL_ERR       1

#define RWS_LOG_LEVEL           EWS_LOG_LEVEL_NONE

#if (RWS_LOG_LEVEL >= RWS_LOG_LEVEL_DBG)
    #define LOG_D(format, ...)    osPrintf(format"\r\n", ##__VA_ARGS__)
#else
    #define LOG_D(format, ...)
#endif

#if (RWS_LOG_LEVEL >= RWS_LOG_LEVEL_INFO)
    #define LOG_I(format, ...)    osPrintf(format"\r\n", ##__VA_ARGS__)
#else
    #define LOG_I(format, ...)
#endif

#if (RWS_LOG_LEVEL >= RWS_LOG_LEVEL_WARN)
    #define LOG_W(format, ...)    osPrintf(format"\r\n", ##__VA_ARGS__)
#else
    #define LOG_W(format, ...)
#endif

#if (RWS_LOG_LEVEL >= EWS_LOG_LEVEL_ERR)
    #define LOG_E(format, ...)    osPrintf(format"\r\n", ##__VA_ARGS__)
#else
    #define LOG_E(format, ...)
#endif

// end LOG define

#define rt_tick_get osKernelGetTickCount
#define pthread_exit(x)   osThreadExit()

//  移植过程中适配结束
#endif
