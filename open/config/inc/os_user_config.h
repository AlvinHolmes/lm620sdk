/**
 *************************************************************************************
 * @copyright 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 *            保留所有权利。
 *
 * @file      os_user_config.h
 *
 * @brief     os模块用户配置头文件.
 *
 * @revision  1.0
 *
 * @par 修改日志:
 * <table>
 * <tr><th>Date        <th>Version   <th>Author     <th>Description
 * <tr><td>2024-06-18  <td>1.0       <td>ICT Team   <td>初始版本
 * </table>
 ************************************************************************************
 */

#ifndef __OS_USER_CONFIG_H__
#define __OS_USER_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

extern uint16_t g_osTimerThreadStackSize;
extern uint16_t g_osWorkQueueThreadStackSize;

#ifdef __cplusplus
}
#endif

#endif

