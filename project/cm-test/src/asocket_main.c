/**
 * @file main.c
 * @brief ASOCKET 测试项目 - 开机自动运行
 * @date 2026-02-12
 */

#include "lwip/sockets.h"
#include "cm_asocket.h"
#include "cm_eloop.h"
#include "cm_sys.h"
#include "cm_modem_info.h"
#include "cm_modem.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>

/* 声明系统相关函数 */
extern int cm_eloop_init_default(void);

/* 测试参数 */
#define TEST_SERVER_ADDR "120.24.78.22"  /* 百度IP作为测试服务器 */
#define TEST_SERVER_PORT 80
#define TEST_HTTP_GET     "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n"

/* 全局变量 */
static int test_sock = -1;

/* ASocket 事件回调 */
static void asocket_event_callback(int sock, cm_asocket_event_e event, void *user_param)
{
    (void)user_param;

    switch (event) {
        case CM_ASOCKET_EV_CONNECT_OK:
            cm_printf("[ASOCKET] Socket %d connected successfully\n", sock);
            /* 连接成功后发送HTTP GET请求 */
            int ret = cm_asocket_send(sock, TEST_HTTP_GET, strlen(TEST_HTTP_GET), 0);
            if (ret > 0) {
                cm_printf("[ASOCKET] Sent HTTP request, len=%d\n", ret);
            } else {
                cm_printf("[ASOCKET] Send failed, ret=%d\n", ret);
            }
            break;

        case CM_ASOCKET_EV_CONNECT_FAIL:
            cm_printf("[ASOCKET] Socket %d connection failed\n", sock);
            break;

        case CM_ASOCKET_EV_RECV_IND: {
            int recv_avail = 0;
            cm_asocket_ioctl(sock, FIONREAD, &recv_avail);

            uint8_t recv_buf[512] = {0};
            int ret = cm_asocket_recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);

            if (ret > 0) {
                recv_buf[ret] = '\0';
                cm_printf("[ASOCKET] Received %d bytes:\n%s\n", ret, recv_buf);
            } else {
                cm_printf("[ASOCKET] Recv error, errno=%d\n", errno);
            }
            break;
        }

        case CM_ASOCKET_EV_SEND_IND:
            cm_printf("[ASOCKET] Socket %d send indication\n", sock);
            break;

        case CM_ASOCKET_EV_ERROR_IND: {
            int sock_error = 0;
            socklen_t opt_len = sizeof(sock_error);
            cm_asocket_getsockopt(sock, SOL_SOCKET, SO_ERROR, &sock_error, &opt_len);
            cm_printf("[ASOCKET] Socket %d error: %d\n", sock, sock_error);
            break;
        }

        default:
            cm_printf("[ASOCKET] Unknown event: %d\n", event);
            break;
    }
}

/* 等待联网成功 */
static void wait_for_network(void)
{
    cm_cereg_state_t cereg = {0};
    int retry_count = 0;
    const int max_retries = 120;

    cm_printf("[ASOCKET] Waiting for network to be ready...\n");

    while (retry_count < max_retries) {
        cm_modem_get_cereg_state(&cereg);
        if (cereg.state == 1) {
            cm_printf("[ASOCKET] Network is ready!\n");
            return;
        }
        cm_printf("[ASOCKET] Waiting for network... (state=%d, retry=%d/%d)\n",
                  cereg.state, retry_count + 1, max_retries);
        osDelay(1000);
        retry_count++;
    }

    cm_printf("[ASOCKET] Warning: Network timeout, proceeding anyway...\n");
}

/* 测试 TCP 连接 */
static void test_tcp_connect(void)
{
    cm_printf("[ASOCKET] === Starting TCP Connect Test ===\n");

    /* 打开 TCP socket */
    test_sock = cm_asocket_open(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                                 asocket_event_callback, NULL);

    if (test_sock < 0) {
        cm_printf("[ASOCKET] Failed to create socket\n");
        return;
    }

    cm_printf("[ASOCKET] Created socket %d\n", test_sock);

    /* 准备服务器地址 */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(TEST_SERVER_ADDR);
    server_addr.sin_port = htons(TEST_SERVER_PORT);

    /* 连接服务器 */
    int ret = cm_asocket_connect(test_sock, (const struct sockaddr *)&server_addr,
                                 sizeof(server_addr));

    if (ret < 0) {
        if (errno == EINPROGRESS) {
            cm_printf("[ASOCKET] Connect in progress...\n");
        } else {
            cm_printf("[ASOCKET] Connect failed, errno=%d\n", errno);
        }
    } else if (ret == 0) {
        cm_printf("[ASOCKET] Connect succeeded immediately\n");
        /* 发送HTTP请求 */
        cm_asocket_send(test_sock, TEST_HTTP_GET, strlen(TEST_HTTP_GET), 0);
    }

    cm_printf("[ASOCKET] === TCP Connect Test Initiated ===\n\n");
}

/* 测试 Socket 关闭 */
static void test_socket_close(void)
{
    cm_printf("[ASOCKET] === Starting Socket Close Test ===\n");

    if (test_sock >= 0) {
        int ret = cm_asocket_close(test_sock);
        if (ret == 0) {
            cm_printf("[ASOCKET] Socket %d closed successfully\n", test_sock);
        } else {
            cm_printf("[ASOCKET] Failed to close socket %d, errno=%d\n", test_sock, errno);
        }
        test_sock = -1;
    } else {
        cm_printf("[ASOCKET] No socket to close\n");
    }

    cm_printf("[ASOCKET] === Socket Close Test Complete ===\n\n");
}

int main(void)
{
    cm_eloop_init_default();

    cm_printf("[ASOCKET] ================================\n");
    cm_printf("[ASOCKET] ASocket Test Starting...\n");
    cm_printf("[ASOCKET] ================================\n\n");

    /* 等待联网成功 */
    wait_for_network();

    /* 延迟一下确保网络稳定 */
    osDelay(2000);

    /* 测试 TCP 连接 */
    test_tcp_connect();

    /* 等待数据接收和连接完成 */
    osDelay(5000);

    /* 关闭 Socket */
    test_socket_close();

    cm_printf("[ASOCKET] ================================\n");
    cm_printf("[ASOCKET] ASocket Test Complete\n");
    cm_printf("[ASOCKET] ================================\n");

    return 0;
}