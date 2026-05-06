/**
 * @file example_main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-12-25
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/****************************************************************************
* Included Files
****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "cm_os.h"
#include "cm_sys.h"
#include "cm_fs.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TEST_FILE1 "/test1.bin"
#define TEST_FILE2 "/test2.bin"

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
****************************************************************************/

int main(void)
{
    char test_data[] = "test the world is wonderful!";
    char read_buf[32];
    cm_fs_system_info_t info;
    int32_t ret;
    int32_t file_size;
    cm_fs_t fd;

    cm_printf("CM FS test starts\n");

    /* 测试写入文件 */
    cm_printf("=== Test write file ===\n");
    fd = cm_fs_open(TEST_FILE1, CM_FS_WBPLUS);
    if (fd == NULL) {
        cm_printf("cm_fs_open(%s, CM_FS_WBPLUS) failed\n", TEST_FILE1);
        return 0;
    }
    cm_printf("cm_fs_open(%s, CM_FS_WBPLUS) success, fd=%p\n", TEST_FILE1, fd);

    ret = cm_fs_write(fd, test_data, strlen(test_data));
    cm_printf("cm_fs_write() written=%d bytes\n", ret);

    ret = cm_fs_sync(fd);
    cm_printf("cm_fs_sync() ret=%d\n", ret);

    ret = cm_fs_close(fd);
    cm_printf("cm_fs_close() ret=%d\n", ret);

    /* 测试读取文件 */
    cm_printf("\n=== Test read file ===\n");
    if (cm_fs_exist(TEST_FILE1)) {
        cm_printf("cm_fs_exist(%s) = true\n", TEST_FILE1);

        fd = cm_fs_open(TEST_FILE1, CM_FS_RB);
        if (fd == NULL) {
            cm_printf("cm_fs_open(%s, CM_FS_RB) failed\n", TEST_FILE1);
            return 0;
        }
        cm_printf("cm_fs_open(%s, CM_FS_RB) success, fd=%p\n", TEST_FILE1, fd);

        memset(read_buf, 0, sizeof(read_buf));
        ret = cm_fs_read(fd, read_buf, 32);
        cm_printf("cm_fs_read() read=%d bytes, data=%s\n", ret, read_buf);

        ret = cm_fs_close(fd);
        cm_printf("cm_fs_close() ret=%d\n", ret);
    } else {
        cm_printf("cm_fs_exist(%s) = false\n", TEST_FILE1);
    }

    /* 测试获取文件大小 */
    cm_printf("\n=== Test get file size ===\n");
    file_size = cm_fs_filesize(TEST_FILE1);
    cm_printf("cm_fs_filesize(%s) = %d bytes\n", TEST_FILE1, file_size);

    /* 测试 seek 和部分读取 */
    cm_printf("\n=== Test seek and partial read ===\n");
    fd = cm_fs_open(TEST_FILE1, CM_FS_RB);
    if (fd == NULL) {
        cm_printf("cm_fs_open(%s, CM_FS_RB) failed\n", TEST_FILE1);
        return 0;
    }

    ret = cm_fs_seek(fd, file_size - 10, CM_FS_SEEK_SET);
    if (ret < 0) {
        cm_printf("cm_fs_seek() failed\n");
        cm_fs_close(fd);
        return 0;
    }

    memset(read_buf, 0, sizeof(read_buf));
    ret = cm_fs_read(fd, read_buf, 10);
    cm_printf("cm_fs_read() last 10 bytes: %s\n", read_buf);

    ret = cm_fs_close(fd);
    cm_printf("cm_fs_close() ret=%d\n", ret);

    /* 测试获取文件系统信息 */
    cm_printf("\n=== Test get filesystem info ===\n");
    ret = cm_fs_getinfo(&info);
    if (ret == 0) {
        cm_printf("cm_fs_getinfo() success:\n");
        cm_printf("  total_size=%u bytes\n", info.total_size);
        cm_printf("  free_size=%u bytes\n", info.free_size);
    } else {
        cm_printf("cm_fs_getinfo() failed, ret=%d\n", ret);
    }

    /* 测试移动文件 */
    cm_printf("\n=== Test move file ===\n");
    if (cm_fs_exist(TEST_FILE1)) {
        if (cm_fs_exist(TEST_FILE2)) {
            ret = cm_fs_delete(TEST_FILE2);
            cm_printf("cm_fs_delete(%s) ret=%d\n", TEST_FILE2, ret);
        }

        ret = cm_fs_move(TEST_FILE1, TEST_FILE2);
        cm_printf("cm_fs_move(%s, %s) ret=%d\n", TEST_FILE1, TEST_FILE2, ret);

        if (cm_fs_exist(TEST_FILE1)) {
            cm_printf("cm_fs_move() failed: source still exists\n");
        } else {
            cm_printf("cm_fs_move() success: source moved\n");
        }
    }

    /* 测试删除文件 */
    cm_printf("\n=== Test delete file ===\n");
    if (cm_fs_exist(TEST_FILE2)) {
        ret = cm_fs_delete(TEST_FILE2);
        cm_printf("cm_fs_delete(%s) ret=%d\n", TEST_FILE2, ret);

        if (cm_fs_exist(TEST_FILE2)) {
            cm_printf("cm_fs_delete() failed: file still exists\n");
        } else {
            cm_printf("cm_fs_delete() success: file deleted\n");
        }
    }

    /* 测试异常情况 */
    cm_printf("\n=== Test error handling ===\n");
    ret = cm_fs_getinfo(NULL);
    if (ret != 0) {
        cm_printf("cm_fs_getinfo(NULL) correctly returned error\n");
    } else {
        cm_printf("cm_fs_getinfo(NULL) abnormal: should return error\n");
    }

    cm_printf("\nCM FS test ends\n");
    return 0;
}
