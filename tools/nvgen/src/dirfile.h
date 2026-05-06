/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        dir_file.h
 *
 * @brief       目录文件操作接口.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-25     ict team          创建
 ************************************************************************************
 */

#ifndef _DIR_FILE_OPERATIONS_H_
#define _DIR_FILE_OPERATIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

int NVGEN_CreateDirs(const char *path);
int NVGEN_ReadFile(const char* file, void *buf, unsigned int len);
int NVGEN_WriteNewFile(const char* file, const void *buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* _DIR_FILE_OPERATIONS_H_ */
