/**
 * @file main.c
 * @brief FTP 测试项目 - 开机自动运行
 * @date 2026-02-12
 *
 * 测试前请先修改下方的测试配置参数
 */

#include "cm_sys.h"
#include "cm_modem_info.h"
#include "cm_ftp.h"
#include "cm_modem.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* 声明系统相关函数 */
extern int cm_eloop_init_default(void);

/****************************************************************************
 * 测试配置 - 请修改以下参数
 ****************************************************************************/

#define TEST_SERVER     "airtest.openluat.com"  /* FTP服务器地址 */
#define TEST_PORT       21                       /* FTP端口 */
#define TEST_USERNAME   "luat"                   /* 用户名 */
#define TEST_PASSWORD   "123456"                 /* 密码 */

#define TEST_DIR_NAME       "ftp_test_dir"
#define TEST_FILE_NAME      "ftp_test_file.txt"
#define TEST_FILE_SIZE      256
#define TEST_DATA_OFFSET    100

/****************************************************************************
 * 辅助函数
 ****************************************************************************/

static void print_separator(const char *title)
{
    cm_printf("\n");
    cm_printf("========================================\n");
    cm_printf(" %s\n", title);
    cm_printf("========================================\n");
}

static void print_result(const char *test_name, int result)
{
    if (result >= 0) {
        cm_printf("[PASS] %s\n", test_name);
    } else {
        cm_printf("[FAIL] %s (err=%d)\n", test_name, result);
    }
}

static void print_file_data(cm_ftp_file_data_t *file_data)
{
    cm_printf("  - Name: %s\n", file_data->file_name);
    cm_printf("    Attr: %s\n", file_data->file_attr ? "File" : "Directory");
    if (file_data->file_attr) {
        cm_printf("    Size: %u bytes\n", file_data->file_size);
    }
}

/****************************************************************************
 * 测试用例
 ****************************************************************************/

/**
 * @brief 测试目录操作
 */
static int test_directory_operations(int32_t handle)
{
    char current_dir[256] = {0};

    print_separator("Test: Directory Operations");

    /* 获取当前目录 */
    if (cm_ftp_get_current_dir(handle, current_dir) == 0) {
        cm_printf("当前目录: %s\n", current_dir);
        print_result("cm_ftp_get_current_dir", 0);
    } else {
        print_result("cm_ftp_get_current_dir", -1);
    }

    /* 创建测试目录 */
    print_result("cm_ftp_create_dir", cm_ftp_create_dir(handle, TEST_DIR_NAME));

    /* 切换到测试目录 */
    print_result("cm_ftp_set_current_dir", cm_ftp_set_current_dir(handle, TEST_DIR_NAME));

    /* 验证目录切换 */
    memset(current_dir, 0, sizeof(current_dir));
    if (cm_ftp_get_current_dir(handle, current_dir) == 0) {
        cm_printf("切换后目录: %s\n", current_dir);
        print_result("验证目录切换", strstr(current_dir, TEST_DIR_NAME) ? 0 : -1);
    }

    /* 返回上级目录 */
    print_result("cm_ftp_set_current_dir (..)", cm_ftp_set_current_dir(handle, ".."));

    /* 删除测试目录 */
    print_result("cm_ftp_remove_dir", cm_ftp_remove_dir(handle, TEST_DIR_NAME));

    return 0;
}

/**
 * @brief 测试文件上传
 */
static int test_file_upload(int32_t handle)
{
    uint8_t test_data[TEST_FILE_SIZE];
    int i;

    print_separator("Test: File Upload");

    /* 生成测试数据 */
    for (i = 0; i < TEST_FILE_SIZE; i++) {
        test_data[i] = (uint8_t)('A' + (i % 26));
    }
    test_data[TEST_FILE_SIZE - 1] = '\0';

    cm_printf("上传测试文件: %s (%d bytes)\n", TEST_FILE_NAME, TEST_FILE_SIZE);

    /* 上传文件 (覆盖模式) */
    print_result("cm_ftp_put_file (STOR)",
                 cm_ftp_put_file(handle, 0, TEST_FILE_NAME, test_data, TEST_FILE_SIZE));

    return 0;
}

/**
 * @brief 测试文件下载
 */
static int test_file_download(int32_t handle)
{
    uint8_t recv_data[TEST_FILE_SIZE + 1] = {0};
    int32_t recv_len;
    int32_t file_size;

    print_separator("Test: File Download");

    /* 获取文件大小 */
    file_size = cm_ftp_get_file_size(handle, TEST_FILE_NAME);
    print_result("cm_ftp_get_file_size", file_size > 0 ? 0 : -1);
    cm_printf("文件大小: %d bytes\n", (int)file_size);

    /* 下载完整文件 */
    recv_len = cm_ftp_get_file(handle, 0, TEST_FILE_NAME, 0, recv_data, TEST_FILE_SIZE);
    print_result("cm_ftp_get_file (full)", recv_len > 0 ? 0 : -1);
    cm_printf("接收到 %d 字节\n", (int)recv_len);
    if (recv_len > 0) {
        recv_data[recv_len] = '\0';
        cm_printf("数据预览: %.50s...\n", (char *)recv_data);
    }

    return 0;
}

/**
 * @brief 测试断点续传
 */
static int test_resume_download(int32_t handle)
{
    uint8_t recv_data[TEST_FILE_SIZE + 1] = {0};
    int32_t recv_len;

    print_separator("Test: Resume Download (offset)");

    /* 从偏移量下载 */
    cm_printf("从偏移量 %d 开始下载\n", TEST_DATA_OFFSET);
    recv_len = cm_ftp_get_file(handle, 0, TEST_FILE_NAME, TEST_DATA_OFFSET,
                               recv_data, TEST_FILE_SIZE - TEST_DATA_OFFSET);
    print_result("cm_ftp_get_file (with offset)", recv_len > 0 ? 0 : -1);
    cm_printf("接收到 %d 字节\n", (int)recv_len);
    if (recv_len > 0) {
        recv_data[recv_len] = '\0';
        cm_printf("数据预览: %.50s...\n", (char *)recv_data);
    }

    return 0;
}

/**
 * @brief 测试文件查找
 */
static int test_file_find(int32_t handle)
{
    cm_ftp_file_data_t file_data;
    int32_t find_fd;
    int file_count = 0;

    print_separator("Test: File Find (List Directory)");

    /* 开始查找 */
    find_fd = cm_ftp_find_first(handle, ".", &file_data);
    if (find_fd >= 0) {
        print_result("cm_ftp_find_first", 0);
        print_file_data(&file_data);
        file_count++;

        /* 继续查找 */
        while (cm_ftp_find_next(handle, find_fd, &file_data) >= 0) {
            print_file_data(&file_data);
            file_count++;
        }
        print_result("cm_ftp_find_next (completed)", 0);

        /* 关闭查找 */
        print_result("cm_ftp_find_close", cm_ftp_find_close(handle, find_fd));
    } else {
        print_result("cm_ftp_find_first", -1);
    }

    cm_printf("共找到 %d 个文件/目录\n", file_count);

    return 0;
}

/**
 * @brief 测试文件追加
 */
static int test_file_append(int32_t handle)
{
    uint8_t append_data[] = " [APPENDED DATA]";

    print_separator("Test: File Append");

    /* 追加数据 */
    print_result("cm_ftp_put_file (APPE)",
                 cm_ftp_put_file(handle, 1, TEST_FILE_NAME, append_data, sizeof(append_data) - 1));

    /* 验证追加后的文件大小 */
    int32_t new_size = cm_ftp_get_file_size(handle, TEST_FILE_NAME);
    cm_printf("追加后文件大小: %d bytes\n", (int)new_size);

    return 0;
}

/**
 * @brief 测试文件重命名
 */
static int test_file_rename(int32_t handle)
{
    const char *new_name = "ftp_test_file_renamed.txt";

    print_separator("Test: File Rename");

    /* 重命名文件 */
    print_result("cm_ftp_rename_file",
                 cm_ftp_rename_file(handle, TEST_FILE_NAME, new_name));

    /* 重命名回来 */
    print_result("cm_ftp_rename_file (restore)",
                 cm_ftp_rename_file(handle, new_name, TEST_FILE_NAME));

    return 0;
}

/**
 * @brief 测试清理
 */
static int test_cleanup(int32_t handle)
{
    print_separator("Test: Cleanup");

    /* 删除测试文件 */
    print_result("cm_ftp_delete_file", cm_ftp_delete_file(handle, TEST_FILE_NAME));

    return 0;
}

/****************************************************************************
 * 网络等待
 ****************************************************************************/

static void wait_for_network(void)
{
    cm_cereg_state_t cereg = {0};
    int retry_count = 0;
    const int max_retries = 120;

    cm_printf("[FTP] Waiting for network to be ready...\n");

    while (retry_count < max_retries) {
        cm_modem_get_cereg_state(&cereg);
        if (cereg.state == 1) {
            cm_printf("[FTP] Network is ready!\n");
            return;
        }
        cm_printf("[FTP] Waiting for network... (state=%d, retry=%d/%d)\n",
                  cereg.state, retry_count + 1, max_retries);
        osDelay(1000);
        retry_count++;
    }

    cm_printf("[FTP] Warning: Network timeout, proceeding anyway...\n");
}

/****************************************************************************
 * 主测试函数
 ****************************************************************************/

static void ftp_test_main(void)
{
    cm_ftp_config_t config = {0};
    int32_t handle;

    print_separator("FTP Client Test Suite");
    cm_printf("服务器: %s:%d\n", TEST_SERVER, TEST_PORT);
    cm_printf("用户名: %s\n", TEST_USERNAME);

    /* 配置连接参数 */
    config.url = (uint8_t *)TEST_SERVER;
    config.port = TEST_PORT;
    config.username = (uint8_t *)TEST_USERNAME;
    config.passwd = (uint8_t *)TEST_PASSWORD;
    config.data_type = 0;  /* Binary */
    config.rsptimeout = 10000;

    /* 连接服务器 */
    handle = cm_ftp_open(&config);
    if (handle < 0) {
        cm_printf("\n[ERROR] 无法连接到FTP服务器\n");
        return;
    }

    cm_printf("\n连接成功，开始测试...\n");

    /* 运行测试用例 */
    test_directory_operations(handle);
    test_file_upload(handle);
    test_file_download(handle);
    test_resume_download(handle);
    test_file_append(handle);
    test_file_find(handle);
    test_file_rename(handle);
    test_cleanup(handle);

    /* 断开连接 */
    print_separator("Disconnect");
    print_result("cm_ftp_close", cm_ftp_close(handle));

    print_separator("Test Summary");
    cm_printf("所有测试完成\n");
}

/****************************************************************************
 * 主函数
 ****************************************************************************/

int main(void)
{
    cm_eloop_init_default();

    cm_printf("\n");
    cm_printf("========================================\n");
    cm_printf(" FTP Test Project\n");
    cm_printf(" Date: %s %s\n", __DATE__, __TIME__);
    cm_printf("========================================\n");

    /* 等待联网成功 */
    wait_for_network();

    /* 延迟2秒确保网络稳定 */
    osDelay(2000);

    /* 运行FTP测试 */
    ftp_test_main();

    return 0;
}