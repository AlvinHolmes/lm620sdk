/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file   import.c
*
* @brief  KERNEL API 导入.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/
#include <os.h>
#include "rtmsym.h"
#include "vfs.h"

RTM_IMPORT_FUNC(int *,_osErrno,(void), ());
RTM_IMPORT_FUNC(int32_t, osVsnprintf,(char *buf, size_t size, const char *fmt, va_list args), (buf, size, fmt, args));
RTM_IMPORT_FUNC(void, osPuts, (const char *str), (str));

// RTM_IMPORT_FUNC(void, osPrintf, (const char *fmt, ...), (fmt))  // 可变参数列表无法通过函数指针传递 此函数实现不可用
void osPrintf(const char *fmt, ...)
{
    va_list args;
    size_t length = 0;
    char rt_log_buf[OS_CONSOLEBUF_SIZE] = {0};

    va_start(args, fmt);

    length = osVsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > 0 && length  < sizeof(rt_log_buf)) {
        osPuts(rt_log_buf);
    }

    va_end(args);
}
RTM_IMPORT_FUNC(void, osAssertHandler,(const char *file, uint32_t line), (file, line));

RTM_IMPORT_FUNC(osThreadId_t, osThreadNew, (osThreadFunc_t func, void *argument, const osThreadAttr_t *attr), (func, argument, attr));
RTM_IMPORT_FUNC(osStatus_t, osThreadSleep, (uint32_t tick), (tick));
RTM_IMPORT_FUNC(osStatus_t, osThreadJoin, (osThreadId_t thread_id), (thread_id));

RTM_IMPORT_FUNC(osStatus_t, osMessageQueueGet, (osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout), (mq_id, msg_ptr, msg_prio, timeout));
RTM_IMPORT_FUNC(osStatus_t, osMessageQueuePut, (osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout), (mq_id, msg_ptr, msg_prio, timeout));
RTM_IMPORT_FUNC(osMessageQueueId_t, osMessageQueueNew, (uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr), (msg_count, msg_size, attr));

RTM_IMPORT_FUNC(VFS_File*, VFS_OpenFile, (const char *path, const char *mode), (path, mode));
RTM_IMPORT_FUNC(size_t, VFS_ReadFile, (void *buf, size_t size, size_t count, VFS_File *fp), (buf, size, count, fp));
RTM_IMPORT_FUNC(size_t, VFS_WriteFile, (const void *buf, size_t size, size_t count, VFS_File *fp), (buf, size, count, fp));
RTM_IMPORT_FUNC(int, VFS_CloseFile, (VFS_File *fp), (fp));
RTM_IMPORT_FUNC(int, VFS_SeekFile, (VFS_File *fp, long offset, int whence), (fp, offset, whence));

RTM_IMPORT_FUNC(int32_t, NVM_Read, (uint32_t nvItem, uint8_t *nvData, uint32_t len), (nvItem, nvData, len));
RTM_IMPORT_FUNC(int32_t, NVM_Write, (uint32_t nvItem, uint8_t *nvData, uint32_t len), (nvItem, nvData, len));

RTM_IMPORT_FUNC(int32_t, Api_GetImei, (char * pImeiPtr), (pImeiPtr))
RTM_IMPORT_FUNC(int, svc_eqp_read_boardnum, (char *p_boardnum, uint32_t buf_len), (p_boardnum, buf_len))

