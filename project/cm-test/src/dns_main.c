/**
 * @file main.c
 * @brief DNS 测试项目 - 开机自动运行
 * @date 2026-02-12
 */

#include "lwip/sockets.h"
#include "cm_async_dns.h"
#include "cm_eloop.h"
#include "cm_sys.h"
#include "cm_modem_info.h"
#include "cm_modem.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>

/* 声明系统相关函数 */
extern int cm_eloop_init_default(void);

/* 删除手动定义的 cm_cereg_state_t，使用头文件中的定义 */

/* DNS解析回调 */
static void dns_callback(int req_id, cm_async_dns_event_e event, void *cb_param,
                         const char *host_name, const cm_async_dns_ip_addr_t *ip_addr)
{
    (void)req_id;
    (void)cb_param;

    if (event == CM_ASYNC_DNS_RESOLVE_OK) {
        char ip_str[128] = {0};

        if (ip_addr->type == CM_ASYNC_DNS_ADDRTYPE_IPV4) {
            inet_ntop(AF_INET, &(ip_addr->u_addr.sin_addr), ip_str, sizeof(ip_str));
        } else if (ip_addr->type == CM_ASYNC_DNS_ADDRTYPE_IPV6) {
            inet_ntop(AF_INET6, &(ip_addr->u_addr.sin6_addr), ip_str, sizeof(ip_str));
        }

        cm_printf("[DNS] DNS resolved: %s -> %s\n", host_name, ip_str);
    } else {
        cm_printf("[DNS] DNS resolution failed for %s\n", host_name);
    }
}

/* 等待联网成功 */
static void wait_for_network(void)
{
    cm_cereg_state_t cereg = {0};
    int retry_count = 0;
    const int max_retries = 120; // 最多等待2分钟

    cm_printf("[DNS] Waiting for network to be ready...\n");

    while (retry_count < max_retries) {
        cm_modem_get_cereg_state(&cereg);
        if (cereg.state == 1) {
            cm_printf("[DNS] Network is ready!\n");
            return;
        }
        cm_printf("[DNS] Waiting for network... (state=%d, retry=%d/%d)\n", cereg.state, retry_count + 1, max_retries);
        osDelay(1000); // 等待1秒
        retry_count++;
    }

    cm_printf("[DNS] Warning: Network timeout, proceeding anyway...\n");
}

/* 验证当前活动 CID 和本地地址 */
static void verify_local_addr(void)
{
    int cid;
    int ret;
    struct sockaddr_in addr4;
    struct sockaddr_in6 addr6;
    char ip4_str[64] = {0};
    char ip6_str[128] = {0};

    cid = cm_get_active_cid();
    cm_printf("[DNS] active cid = %d\n", cid);
    if (cid < 0) {
        cm_printf("[DNS] no active cid, skip local addr test\n");
        return;
    }

    ret = cm_get_ipv4v6_local_addr(cid, AF_INET, &addr4);
    if (ret == 0) {
        inet_ntop(AF_INET, &addr4.sin_addr, ip4_str, sizeof(ip4_str));
        cm_printf("[DNS] local ipv4 = %s\n", ip4_str);
    } else {
        cm_printf("[DNS] get local ipv4 failed, ret=%d\n", ret);
    }

    ret = cm_get_ipv4v6_local_addr(cid, AF_INET6, &addr6);
    if (ret == 0) {
        inet_ntop(AF_INET6, &addr6.sin6_addr, ip6_str, sizeof(ip6_str));
        cm_printf("[DNS] local ipv6 = %s\n", ip6_str);
    } else {
        cm_printf("[DNS] get local ipv6 failed, ret=%d\n", ret);
    }
}

/* 自动运行DNS测试 */
static void run_dns_test(void)
{
    cm_printf("[DNS] Starting DNS resolution test...\n");

    cm_async_dns_ip_addr_t ip_buf;
    memset(&ip_buf, 0, sizeof(ip_buf));

    int ret = cm_async_dns_request("www.baidu.com", CM_ASYNC_DNS_ADDRTYPE_IPV4,
                                   &ip_buf, dns_callback, NULL);

    if (ret > 0) {
        cm_printf("[DNS] DNS request sent, req_id=%d\n", ret);
    } else if (ret == 0) {
        cm_printf("[DNS] DNS resolved immediately\n");
        dns_callback(0, CM_ASYNC_DNS_RESOLVE_OK, NULL, "www.baidu.com", &ip_buf);
    } else {
        cm_printf("[DNS] DNS request failed, ret=%d\n", ret);
    }
}

int main(void)
{
    cm_eloop_init_default();

    /* 等待联网成功 */
    wait_for_network();

    /* 验证当前活动 CID 和本地地址 */
    verify_local_addr();

    /* 运行DNS测试 */
    run_dns_test();

    return 0;
}