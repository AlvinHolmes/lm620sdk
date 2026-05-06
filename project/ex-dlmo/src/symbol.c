/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file   symbol.c
*
* @brief  内核API接口文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/
#include <os.h>
#include <dlmodule.h>
#include <nvm.h>
#include <vfs.h>

/**
 * C libary APIs
 */
RTM_EXPORT(osGetErrno);
RTM_EXPORT(osSetErrno);

RTM_EXPORT(malloc);
RTM_EXPORT(free);
RTM_EXPORT(realloc);
RTM_EXPORT(calloc);

RTM_EXPORT(osMemset);
RTM_EXPORT(osMemcmp);
RTM_EXPORT(osMemcpy);
RTM_EXPORT(osMemmove);

RTM_EXPORT(osStrlen);
RTM_EXPORT(osStrstr);
RTM_EXPORT(osStrncpy);
RTM_EXPORT(osStrncmp);
RTM_EXPORT(osStrcmp);

RTM_EXPORT(osVsnprintf);
RTM_EXPORT(osSnprintf);
RTM_EXPORT(osVsprintf);
RTM_EXPORT(osSprintf);
RTM_EXPORT(osPrintf);

RTM_EXPORT(osAssertHandler);


/**
 * Interrupt APIs
 */
RTM_EXPORT(osInterruptDisable);
RTM_EXPORT(osInterruptEnable);

/**
 * Tick APIs
 */
RTM_EXPORT(osTickGet);
RTM_EXPORT(osTickFromMillisecond);
RTM_EXPORT(osTickFromMsRelaxed);

/**
 * Kernel APIs
 */
RTM_EXPORT(osCriticalEnter);
RTM_EXPORT(osCriticalExit);

/**
 * Thread APIs
 */
RTM_EXPORT(osThreadNew);
RTM_EXPORT(osThreadTerminate);
RTM_EXPORT(osThreadStart);
RTM_EXPORT(osThreadSleep);
RTM_EXPORT(osThreadSleepRelaxed);
RTM_EXPORT(osThreadGetId);
RTM_EXPORT(osThreadSuspend);
RTM_EXPORT(osThreadResume);
RTM_EXPORT(osThreadSetPriority);
RTM_EXPORT(osThreadJoin);
RTM_EXPORT(osThreadFlagsSet);
RTM_EXPORT(osThreadFlagsClear);
RTM_EXPORT(osThreadFlagsWait);

/**
 * Semaphore APIs
 */
RTM_EXPORT(osSemaphoreNew);
RTM_EXPORT(osSemaphoreDelete);
RTM_EXPORT(osSemaphoreAcquire);
RTM_EXPORT(osSemaphoreAcquireRelaxed);
RTM_EXPORT(osSemaphoreRelease);

/**
 * Mutex APIs
 */
RTM_EXPORT(osMutexNew);
RTM_EXPORT(osMutexDelete);
RTM_EXPORT(osMutexAcquire);
RTM_EXPORT(osMutexRelease);

/**
 * Event APIs
 */
RTM_EXPORT(osEventFlagsNew);
RTM_EXPORT(osEventFlagsDelete);
RTM_EXPORT(osEventFlagsClear);
RTM_EXPORT(osEventFlagsWait);

/**
 * Mailbox APIs
 */
RTM_EXPORT(osMbCreate);
RTM_EXPORT(osMbDestory);
RTM_EXPORT(osMbSend);
RTM_EXPORT(osMbSendRelaxed);
RTM_EXPORT(osMbUrgent);
RTM_EXPORT(osMbRecv);
RTM_EXPORT(osMbRecvRelaxed);

/**
 * Message queue APIs
 */
RTM_EXPORT(osMessageQueueNew);
RTM_EXPORT(osMessageQueueDelete);
RTM_EXPORT(osMessageQueuePut);
RTM_EXPORT(osMessageQueuePutRelaxed);
RTM_EXPORT(osMessageQueueUrgent);
RTM_EXPORT(osMessageQueueGet);
RTM_EXPORT(osMessageQueueGetRelaxed);

/**
 * Timer APIs
 */
RTM_EXPORT(osTimerNew);
RTM_EXPORT(osTimerDelete);
RTM_EXPORT(osTimerStart);
RTM_EXPORT(osTimerStartRelaxed);
RTM_EXPORT(osTimerStop);


/**
 * File system APIs
 */
RTM_EXPORT(VFS_OpenFile);
RTM_EXPORT(VFS_CloseFile);
RTM_EXPORT(VFS_ReadFile);
RTM_EXPORT(VFS_WriteFile);
RTM_EXPORT(VFS_SeekFile);
RTM_EXPORT(VFS_SyncFile);
RTM_EXPORT(VFS_GetFileSize);

RTM_EXPORT(VFS_MakeDir);
RTM_EXPORT(VFS_OpenDir);
RTM_EXPORT(VFS_CloseDir);
RTM_EXPORT(VFS_ReadDir);
RTM_EXPORT(VFS_RmDir);
RTM_EXPORT(VFS_Rename);
RTM_EXPORT(VFS_Unlink);
RTM_EXPORT(VFS_Access);
RTM_EXPORT(VFS_Stat);
RTM_EXPORT(VFS_StatFSUser);

/**
 * NV APIs
 */
RTM_EXPORT(NVM_Read);
RTM_EXPORT(NVM_Write);


