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
#ifndef __FOTA_H__
#define __FOTA_H__


/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdint.h>
#include <os.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
extern int DownloadFotaFile(const char * url); // 下载OTA文件
extern int InitFota(void); // 初始化环境
extern int CheckFotaFileFormat(void); // 检查OTA文件格式是否合法
extern int GetFotaFileVersion(char * old_version, int old_size, char * new_version, int new_size); // 获取OTA版本号
extern int CheckFotaFileSigned(void); // 检查OTA文件签名
extern int StartupFota(void); // 开始升级
extern int GetFotaResult(void); // 获取升级结果

#ifdef __cplusplus
}
#endif
#endif//__FOTA_H__


