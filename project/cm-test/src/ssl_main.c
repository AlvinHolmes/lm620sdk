/**
 * @file main.c
 * @brief SSL 测试项目 - 开机自动运行
 * @date 2026-02-12
 *
 * 注意：运行前请先配置以下参数：
 * - TEST_SSL_ADDR: SSL服务器地址
 * - TEST_SSL_PORT: SSL服务器端口
 * - TEST_CA_CERT: CA证书
 * - TEST_CLIENT_CERT: 客户端证书（双向验证时需要）
 * - TEST_CLIENT_KEY: 客户端密钥（双向验证时需要）
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "cm_sys.h"
#include "cm_ssl.h"
#include "cm_modem_info.h"
#include "cm_modem.h"

/* 声明系统相关函数 */
extern int cm_eloop_init_default(void);

/* =============== 配置参数 =============== */
static const char *TEST_SSL_ADDR = "111.45.11.5";       /* SSL服务器地址 (www.baidu.com) */
static const char *TEST_SSL_HOST = "www.baidu.com";    /* SSL服务器域名（用于 SNI） */
static const int TEST_SSL_PORT = 443;                   /* SSL服务器端口 */
static const int TEST_SSL_VERIFY = 0;                   /* 验证模式: 0=无验证, 1=单向验证, 2=双向验证 */
static const char *TEST_CA_CERT = "";                   /* CA证书 */
static const char *TEST_CLIENT_CERT = "";               /* 客户端证书 */
static const char *TEST_CLIENT_KEY = "";                /* 客户端密钥 */
/* ========================================= */

/* 等待联网成功 */
static void wait_for_network(void)
{
    cm_cereg_state_t cereg = {0};
    int retry_count = 0;
    const int max_retries = 120;

    cm_printf("[SSL] Waiting for network to be ready...\n");

    while (retry_count < max_retries) {
        cm_modem_get_cereg_state(&cereg);
        if (cereg.state == 1) {
            cm_printf("[SSL] Network is ready!\n");
            return;
        }
        cm_printf("[SSL] Waiting for network... (state=%d, retry=%d/%d)\n", cereg.state, retry_count + 1, max_retries);
        osDelay(1000);
        retry_count++;
    }

    cm_printf("[SSL] Warning: Network timeout, proceeding anyway...\n");
}

/* 列举支持的加密套件 */
static void list_cipher_suites(void)
{
    int *list = cm_ssl_list_cipher();
    int size = 0;

    cm_printf("[SSL] Supported cipher suites:\n");

    while (list[size] != 0) {
        size++;
    }

    cm_printf("[SSL] Total cipher suites: %d\n", size);
    cm_printf("[SSL] First 10 cipher suites: ");
    for (int i = 0; i < (size < 10 ? size : 10); i++) {
        cm_printf("0x%x ", list[i]);
    }
    cm_printf("\n");
}

/* 创建 TCP 连接 */
static int create_tcp_connection(void)
{
    int sock;
    struct sockaddr_in server_addr;

    /* 创建 socket */
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        cm_printf("[SSL] Failed to create socket: %d\n", errno);
        return -1;
    }

    cm_printf("[SSL] Socket created: %d\n", sock);

    /* 配置服务器地址 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(TEST_SSL_ADDR);
    server_addr.sin_port = htons(TEST_SSL_PORT);

    /* 连接服务器 */
    cm_printf("[SSL] Connecting to %s:%d...\n", TEST_SSL_ADDR, TEST_SSL_PORT);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cm_printf("[SSL] Connect failed: %d\n", errno);
        lwip_close(sock);
        return -1;
    }

    cm_printf("[SSL] TCP connected successfully!\n");
    return sock;
}

/* 配置 SSL 参数 */
static int configure_ssl_params(int ssl_id)
{
    uint16_t negotime = 60;
    uint8_t session = 1;
    uint8_t sni = 1;  /* 启用 SNI */
    uint8_t version = 255;
    uint16_t cipher_suite = 0x0000;
    uint8_t ignore_timestamp = 0;
    uint8_t ignore_verify = 0;
    int verify = TEST_SSL_VERIFY;

    /* 设置 SSL 参数 */
    if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_NEGOTIME, &negotime) < 0) {
        cm_printf("[SSL] Failed to set negotiate time\n");
        return -1;
    }

    if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_SESSION, &session) < 0) {
        cm_printf("[SSL] Failed to set session\n");
        return -1;
    }

    if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_SNI, &sni) < 0) {
        cm_printf("[SSL] Failed to set SNI\n");
        return -1;
    }

    if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_VERSION, &version) < 0) {
        cm_printf("[SSL] Failed to set version\n");
        return -1;
    }

    if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_CIPHER_SUITE, &cipher_suite) < 0) {
        cm_printf("[SSL] Failed to set cipher suite\n");
        return -1;
    }

    if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_IGNORESTAMP, &ignore_timestamp) < 0) {
        cm_printf("[SSL] Failed to set ignore timestamp\n");
        return -1;
    }

    if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_IGNOREVERIFY, &ignore_verify) < 0) {
        cm_printf("[SSL] Failed to set ignore verify\n");
        return -1;
    }

    /* 设置验证模式 */
    if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_VERIFY, &verify) < 0) {
        cm_printf("[SSL] Failed to set verify mode\n");
        return -1;
    }

    /* 设置证书 */
    if (strlen(TEST_CA_CERT) > 0) {
        if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_CA_CERT, (void *)TEST_CA_CERT) < 0) {
            cm_printf("[SSL] Failed to set CA cert\n");
            return -1;
        }
    }

    if (strlen(TEST_CLIENT_CERT) > 0) {
        if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_CLI_CERT, (void *)TEST_CLIENT_CERT) < 0) {
            cm_printf("[SSL] Failed to set client cert\n");
            return -1;
        }
    }

    if (strlen(TEST_CLIENT_KEY) > 0) {
        if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_CLI_KEY, (void *)TEST_CLIENT_KEY) < 0) {
            cm_printf("[SSL] Failed to set client key\n");
            return -1;
        }
    }

    cm_printf("[SSL] SSL parameters configured (verify mode: %d)\n", verify);
    return 0;
}

/* SSL 连接 */
static int ssl_connect(cm_ssl_ctx_t **ssl_ctx, int sock)
{
    int ssl_id = 1;
    int ret;

    cm_printf("[SSL] Starting SSL handshake...\n");

    /* 配置 SSL 参数 */
    if (configure_ssl_params(ssl_id) < 0) {
        return -1;
    }

    /* 执行 SSL 握手（带 SNI） */
    ret = cm_ssl_conn_with_host((void **)ssl_ctx, ssl_id, sock, 0, (char *)TEST_SSL_HOST);
    if (ret < 0) {
        cm_printf("[SSL] SSL handshake failed!\n");
        return -1;
    }

    cm_printf("[SSL] SSL handshake successful!\n");
    return 0;
}

/* 发送数据 */
static int ssl_send(cm_ssl_ctx_t *ssl_ctx, const char *data, int len)
{
    int total_sent = 0;
    int ret;

    cm_printf("[SSL] Sending data: %s\n", data);

    while (total_sent < len) {
        ret = cm_ssl_write(ssl_ctx, (void *)(data + total_sent), len - total_sent);
        if (ret < 0) {
            cm_printf("[SSL] SSL write failed!\n");
            return -1;
        }
        total_sent += ret;
    }

    cm_printf("[SSL] Data sent successfully (%d bytes)\n", total_sent);
    return total_sent;
}

/* 接收数据 */
static int ssl_recv(cm_ssl_ctx_t *ssl_ctx, uint8_t *buf, int buf_size)
{
    int ret;

    cm_printf("[SSL] Waiting for response...\n");

    ret = cm_ssl_read(ssl_ctx, buf, buf_size);
    if (ret < 0) {
        cm_printf("[SSL] SSL read failed!\n");
        return -1;
    }

    cm_printf("[SSL] Data received (%d bytes)\n", ret);
    return ret;
}

/* SSL 测试主流程 */
static void run_ssl_test(void)
{
    cm_ssl_ctx_t *ssl_ctx = NULL;
    int sock = -1;
    uint8_t recv_buf[256];

    cm_printf("[SSL] ==================== SSL Test Started ====================\n");

    /* 1. 列举加密套件 */
    list_cipher_suites();
    osDelay(1000);

    /* 2. 创建 TCP 连接 */
    sock = create_tcp_connection();
    if (sock < 0) {
        cm_printf("[SSL] Test failed: TCP connection error\n");
        return;
    }
    osDelay(1000);

    /* 3. SSL 握手 */
    if (ssl_connect(&ssl_ctx, sock) < 0) {
        cm_printf("[SSL] Test failed: SSL handshake error\n");
        lwip_close(sock);
        return;
    }
    osDelay(1000);

    /* 4. 发送数据 */
    const char *test_msg = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n";
    if (ssl_send(ssl_ctx, test_msg, strlen(test_msg)) < 0) {
        cm_printf("[SSL] Test failed: SSL send error\n");
        cm_ssl_close((void **)&ssl_ctx);
        lwip_close(sock);
        return;
    }
    osDelay(2000);

    /* 5. 接收数据 */
    memset(recv_buf, 0, sizeof(recv_buf));
    int recv_len = ssl_recv(ssl_ctx, recv_buf, sizeof(recv_buf) - 1);
    if (recv_len > 0) {
        cm_printf("[SSL] Received data: %s\n", recv_buf);
    }

    /* 6. 关闭连接 */
    cm_printf("[SSL] Closing SSL connection...\n");
    cm_ssl_close((void **)&ssl_ctx);
    lwip_close(sock);
    cm_printf("[SSL] Connection closed\n");

    cm_printf("[SSL] ==================== SSL Test Completed ====================\n");
}

int main(void)
{
    cm_eloop_init_default();

    /* 等待联网成功 */
    wait_for_network();

    /* 运行SSL测试 */
    run_ssl_test();

    /* 保持运行 */
    while (1) {
        osDelay(1000);
    }

    return 0;
}