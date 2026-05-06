/**
 * @file example_main.c
 * @brief MQTT 测试项目 - 开机自动运行测试
 * @date 2026-02-15
 */

#include "cm_sys.h"
#include "cm_modem_info.h"
#include "cm_modem.h"
#include "cm_mqtt.h"
#include "cm_ssl.h"
#include "cm_os.h"
#include "cm_pm.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* 声明系统相关函数 */
extern int cm_eloop_init_default(void);

/* MQTT 配置参数 */
#define MQTT_USE_SSL    1
#define MQTT_SSL_ID     1
#define MQTT_SSL_VERIFY 0
#define MQTT_SSL_NEGOTIME 60

#define MQTT_SERVER      "airtest.openluat.com"
#define MQTT_PORT        (MQTT_USE_SSL ? 8883 : 1883)
#define MQTT_CLIENT_ID   "lm620_test_001"
#define MQTT_USERNAME    "user"
#define MQTT_PASSWORD    "password"
#define MQTT_TOPIC       "test/lm620"
#define MQTT_MESSAGE     "Hello from LM620 MQTT Client!"
#define MQTT_KEEPALIVE   120
#define MQTT_QOS         1

/* MQTT 客户句柄 */
static cm_mqtt_client_t *g_mqtt_client = NULL;

/* 测试步骤状态 */
typedef enum {
    TEST_STEP_IDLE = 0,
    TEST_STEP_INIT,
    TEST_STEP_CONNECT,
    TEST_STEP_WAIT_CONNECT,
    TEST_STEP_SUBSCRIBE,
    TEST_STEP_WAIT_SUBSCRIBE,
    TEST_STEP_PUBLISH,
    TEST_STEP_WAIT_PUBLISH,
    TEST_STEP_GET_TOPICS,
    TEST_STEP_DISCONNECT,
    TEST_STEP_WAIT_DISCONNECT,
    TEST_STEP_DESTROY,
    TEST_STEP_COMPLETED
} test_step_e;

static test_step_e g_test_step = TEST_STEP_IDLE;
static int g_test_result = 0;
static volatile int g_puback_received = 0;  /* 是否收到 PUBACK */

/****************************************************************************/
/* 回调函数 */
/****************************************************************************/

/**
 * @brief 连接确认回调
 */
static int mqtt_connack_cb(cm_mqtt_client_t* client, int session, cm_mqtt_conn_state_e conn_res)
{
    cm_printf("[MQTT] CONNACK: session=%d, state=%d\r\n", session, conn_res);

    if (conn_res == CM_MQTT_CONN_STATE_SUCCESS) {
        g_test_step = TEST_STEP_SUBSCRIBE;
        cm_printf("[MQTT] Connected successfully!\r\n");
    } else if (conn_res == CM_MQTT_CONN_STATE_CLIENT_SHUTDOWN) {
        cm_printf("[MQTT] Disconnected by client\r\n");
    } else if (conn_res == CM_MQTT_CONN_STATE_RECONNECTING) {
        cm_printf("[MQTT] Reconnecting...\r\n");
    } else {
        g_test_result = -1;
        cm_printf("[MQTT] Connect failed! state=%d\r\n", conn_res);
    }

    return 0;
}

/**
 * @brief 接收发布消息回调
 */
static int mqtt_publish_cb(cm_mqtt_client_t* client, unsigned short msgid, char* topic, 
                          int total_len, int payload_len, char* payload)
{
    cm_printf("[MQTT] RECEIVED: msgid=%d, topic=%s, len=%d\r\n", msgid, topic, payload_len);

    /* 限制打印长度 */
    int print_len = payload_len > 100 ? 100 : payload_len;
    cm_printf("[MQTT] DATA: %.*s\r\n", print_len, payload);

    return 0;
}

/**
 * @brief 发布确认回调
 */
static int mqtt_puback_cb(cm_mqtt_client_t* client, unsigned short msgid, char dup)
{
    cm_printf("[MQTT] PUBACK: msgid=%d, dup=%d\r\n", msgid, dup);
    g_puback_received = 1;
    return 0;
}

/**
 * @brief 订阅确认回调
 */
static int mqtt_suback_cb(cm_mqtt_client_t* client, unsigned short msgid, int count, int qos[])
{
    cm_printf("[MQTT] SUBACK: msgid=%d, count=%d\r\n", msgid, count);

    /* 检查连接状态是否仍然正常 */
    int state = cm_mqtt_client_get_state(client);
    cm_printf("[MQTT] Client state after SUBACK: %d\r\n", state);

    if (state == CM_MQTT_STATE_CONNECTED) {
        g_test_step = TEST_STEP_WAIT_SUBSCRIBE;  /* 等待一段时间确保状态稳定 */
    } else {
        cm_printf("[MQTT] WARNING: Connection lost after SUBACK\r\n");
        g_test_result = -1;
        g_test_step = TEST_STEP_DISCONNECT;
    }

    return 0;
}

/**
 * @brief 取消订阅确认回调
 */
static int mqtt_unsuback_cb(cm_mqtt_client_t* client, unsigned short msgid)
{
    cm_printf("[MQTT] UNSUBACK: msgid=%d\r\n", msgid);
    return 0;
}

/**
 * @brief Ping 响应回调
 */
static int mqtt_pingresp_cb(cm_mqtt_client_t* client, int ret)
{
    cm_printf("[MQTT] PINGRESP: ret=%d\r\n", ret);
    return 0;
}

/**
 * @brief 超时回调
 */
static int mqtt_timeout_cb(cm_mqtt_client_t* client, unsigned short msgid)
{
    cm_printf("[MQTT] TIMEOUT: msgid=%d\r\n", msgid);
    g_test_result = -1;

    /* 检查连接状态 */
    int state = cm_mqtt_client_get_state(client);
    cm_printf("[MQTT] Client state after timeout: %d\r\n", state);

    return 0;
}

/****************************************************************************/
/* 测试函数 */
/****************************************************************************/

/**
 * @brief 初始化 MQTT 客户端
 */
static int mqtt_test_init(void)
{
    cm_printf("[MQTT] Step 1: Initializing client...\r\n");

    g_mqtt_client = cm_mqtt_client_create();
    if (g_mqtt_client == NULL) {
        cm_printf("[MQTT] ERROR: Failed to create client\r\n");
        return -1;
    }

    /* 设置回调函数 */
    cm_mqtt_client_cb_t callbacks = {0};
    callbacks.connack_cb = mqtt_connack_cb;
    callbacks.publish_cb = mqtt_publish_cb;
    callbacks.puback_cb = mqtt_puback_cb;
    callbacks.pubrec_cb = NULL;
    callbacks.pubcomp_cb = NULL;
    callbacks.suback_cb = mqtt_suback_cb;
    callbacks.unsuback_cb = mqtt_unsuback_cb;
    callbacks.pingresp_cb = mqtt_pingresp_cb;
    callbacks.timeout_cb = mqtt_timeout_cb;

    cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_EVENT, &callbacks);

    /* 设置参数 */
    int version = 4;
    int pkt_timeout = 10;
    int reconn_times = 3;
    int reconn_cycle = 20;
    int reconn_mode = 0;
    int retry_times = 3;
    int ping_cycle = 60;
    int dns_priority = 0;

    cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_VERSION, &version);
    cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_PKT_TIMEOUT, &pkt_timeout);
    cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_RETRY_TIMES, &retry_times);
    cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_RECONN_MODE, &reconn_mode);
    cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_RECONN_TIMES, &reconn_times);
    cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_RECONN_CYCLE, &reconn_cycle);
    cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_PING_CYCLE, &ping_cycle);
    cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_DNS_PRIORITY, &dns_priority);

#if MQTT_USE_SSL
    {
        int ssl_enable = 1;
        int ssl_id = MQTT_SSL_ID;
        uint8_t verify = MQTT_SSL_VERIFY;
        uint16_t negotime = MQTT_SSL_NEGOTIME;

        if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_VERIFY, &verify) < 0) {
            cm_printf("[MQTT] ERROR: Failed to set SSL verify\r\n");
            return -1;
        }
        if (cm_ssl_setopt(ssl_id, CM_SSL_PARAM_NEGOTIME, &negotime) < 0) {
            cm_printf("[MQTT] ERROR: Failed to set SSL negotime\r\n");
            return -1;
        }
        cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_SSL_ID, &ssl_id);
        cm_mqtt_client_set_opt(g_mqtt_client, CM_MQTT_OPT_SSL_ENABLE, &ssl_enable);
        cm_printf("[MQTT] MQTTS enabled: ssl_id=%d, verify=%d, negotime=%d\r\n",
                  ssl_id, verify, negotime);
    }
#endif

    cm_printf("[MQTT] Client initialized successfully\r\n");
    return 0;
}

/**
 * @brief 连接 MQTT 服务器
 */
static int mqtt_test_connect(void)
{
    cm_printf("[MQTT] Step 2: Connecting to %s:%d (%s)\r\n",
              MQTT_SERVER, MQTT_PORT, MQTT_USE_SSL ? "MQTTS" : "MQTT");

    cm_mqtt_connect_options_t conn_opts = {
        .hostport = MQTT_PORT,
        .hostname = MQTT_SERVER,
        .clientid = MQTT_CLIENT_ID,
        .username = MQTT_USERNAME,
        .password = MQTT_PASSWORD,
        .keepalive = MQTT_KEEPALIVE,
        .will_topic = NULL,
        .will_message = NULL,
        .will_message_len = 0,
        .will_flag = 0,
        .clean_session = 1,
    };

    int ret = cm_mqtt_client_connect(g_mqtt_client, &conn_opts);
    if (ret != 0) {
        cm_printf("[MQTT] ERROR: Connect failed, ret=%d\r\n", ret);
        return -1;
    }

    cm_printf("[MQTT] Connect request sent\r\n");
    return 0;
}

/**
 * @brief 订阅主题
 */
static int mqtt_test_subscribe(void)
{
    cm_printf("[MQTT] Step 3: Subscribing to topic: %s\r\n", MQTT_TOPIC);

    const char *topic = MQTT_TOPIC;
    char qos = MQTT_QOS;

    int ret = cm_mqtt_client_subscribe(g_mqtt_client, &topic, &qos, 1);
    if (ret < 0) {
        cm_printf("[MQTT] ERROR: Subscribe failed, ret=%d\r\n", ret);
        return -1;
    }

    cm_printf("[MQTT] Subscribe request sent\r\n");
    return 0;
}

/**
 * @brief 发布消息
 */
static int mqtt_test_publish(void)
{
    cm_printf("[MQTT] Step 4: Publishing message...\r\n");

    /* 检查连接状态 */
    int state = cm_mqtt_client_get_state(g_mqtt_client);
    if (state != CM_MQTT_STATE_CONNECTED) {
        cm_printf("[MQTT] ERROR: Not connected (state=%d), cannot publish\r\n", state);
        return -1;
    }

    const char *message = MQTT_MESSAGE;
    int msg_len = strlen(message);
    char flags = CM_MQTT_QOS_1;

    cm_printf("[MQTT] Publishing to topic: %s, len: %d, flags: 0x%02X\r\n", MQTT_TOPIC, msg_len, flags);

    int ret = cm_mqtt_client_publish(g_mqtt_client, MQTT_TOPIC, message, msg_len, flags);
    if (ret <= 0) {
        cm_printf("[MQTT] ERROR: Publish failed, ret=%d (", ret);
        switch (ret) {
            case CM_MQTT_RET_NOT_CONNECT:
                cm_printf("NOT_CONNECT)\r\n");
                break;
            case CM_MQTT_RET_STATE_ERR:
                cm_printf("STATE_ERR)\r\n");
                break;
            case CM_MQTT_RET_INVALID_PARAM:
                cm_printf("INVALID_PARAM)\r\n");
                break;
            default:
                cm_printf("unknown)\r\n");
                break;
        }
        return -1;
    }

    cm_printf("[MQTT] Publish request sent, len=%d\r\n", ret);
    return 0;
}

/**
 * @brief 获取已订阅的主题
 */
static void mqtt_test_get_topics(void)
{
    cm_printf("[MQTT] Step 5: Getting subscribed topics...\r\n");

    linklist_t *list = cm_mqtt_client_get_sub_topics(g_mqtt_client);
    if (list == NULL || list->count == 0) {
        cm_printf("[MQTT] No subscribed topics\r\n");
        return;
    }

    char topic_buf[256] = {0};
    int buf_len = 0;
    int count = 0;
    linklist_element_t *element = NULL;
    cm_mqtt_topic_t *topic_msg = NULL;

    while ((element = linklist_next_element(list, &element)) != NULL) {
        topic_msg = (cm_mqtt_topic_t *)element->content;

        if (topic_msg->state != CM_MQTT_TOPIC_SUBSCRIBED) {
            continue;
        }

        count++;
        int topic_len = topic_msg->topic_len;
        if (buf_len + topic_len + 10 < sizeof(topic_buf)) {
            memcpy(topic_buf + buf_len, topic_msg->topic, topic_len);
            buf_len += topic_len;
            buf_len += snprintf(topic_buf + buf_len, sizeof(topic_buf) - buf_len, 
                             " (QoS%d), ", topic_msg->qos);
        }
    }

    if (count > 0) {
        topic_buf[buf_len - 2] = '\0';  /* Remove last ", " */
        cm_printf("[MQTT] Subscribed topics: %s\r\n", topic_buf);
    }
}

/**
 * @brief 断开连接
 */
static int mqtt_test_disconnect(void)
{
    cm_printf("[MQTT] Step 6: Disconnecting...\r\n");

    int ret = cm_mqtt_client_disconnect(g_mqtt_client);
    if (ret != 0) {
        cm_printf("[MQTT] ERROR: Disconnect failed, ret=%d\r\n", ret);
        return -1;
    }

    cm_printf("[MQTT] Disconnect request sent\r\n");
    return 0;
}

/**
 * @brief 销毁客户端
 */
static void mqtt_test_destroy(void)
{
    cm_printf("[MQTT] Step 7: Destroying client...\r\n");

    if (g_mqtt_client != NULL) {
        /* 等待一段时间，确保所有异步操作完成 */
        cm_printf("[MQTT] Waiting for async operations to complete...\r\n");
        osDelay(300);

        cm_mqtt_client_destroy(g_mqtt_client);
        g_mqtt_client = NULL;

        /* 再次等待，确保资源完全释放 */
        cm_printf("[MQTT] Waiting for resources to be released...\r\n");
        osDelay(300);
    }

    cm_printf("[MQTT] Client destroyed\r\n");
}

/**
 * @brief 测试任务循环
 */
static void mqtt_test_task(void)
{
    static int wait_count = 0;

    switch (g_test_step) {
        case TEST_STEP_IDLE:
            /* 等待网络就绪后开始测试 */
            break;

        case TEST_STEP_INIT:
            if (mqtt_test_init() == 0) {
                g_test_step = TEST_STEP_CONNECT;
            } else {
                g_test_result = -1;
                g_test_step = TEST_STEP_DESTROY;
            }
            break;

        case TEST_STEP_CONNECT:
            if (mqtt_test_connect() == 0) {
                g_test_step = TEST_STEP_WAIT_CONNECT;
                wait_count = 0;
            } else {
                g_test_result = -1;
                g_test_step = TEST_STEP_DESTROY;
            }
            break;

        case TEST_STEP_WAIT_CONNECT:
            wait_count++;
            if (wait_count > 10) {  /* 等待 10 秒 */
                cm_printf("[MQTT] ERROR: Connect timeout\r\n");
                g_test_result = -1;
                g_test_step = TEST_STEP_DISCONNECT;
            }
            break;

        case TEST_STEP_SUBSCRIBE:
            if (mqtt_test_subscribe() == 0) {
                g_test_step = TEST_STEP_WAIT_SUBSCRIBE;
                wait_count = 0;
            } else {
                g_test_result = -1;
                g_test_step = TEST_STEP_DISCONNECT;
            }
            break;

        case TEST_STEP_WAIT_SUBSCRIBE:
            wait_count++;
            /* 等待 3 秒让连接状态稳定 */
            if (wait_count >= 30) {  /* 30 * 100ms = 3 秒 */
                /* 再次检查连接状态 */
                int state = cm_mqtt_client_get_state(g_mqtt_client);
                cm_printf("[MQTT] Checking connection state before publish: %d\r\n", state);

                if (state == CM_MQTT_STATE_CONNECTED) {
                    g_test_step = TEST_STEP_PUBLISH;
                } else {
                    cm_printf("[MQTT] ERROR: Connection lost, cannot publish\r\n");
                    g_test_result = -1;
                    g_test_step = TEST_STEP_DISCONNECT;
                }
            }
            break;

        case TEST_STEP_PUBLISH:
            g_puback_received = 0;  /* 重置 PUBACK 标志 */
            if (mqtt_test_publish() == 0) {
                g_test_step = TEST_STEP_WAIT_PUBLISH;
                wait_count = 0;
            } else {
                g_test_result = -1;
                g_test_step = TEST_STEP_DISCONNECT;
            }
            break;

        case TEST_STEP_WAIT_PUBLISH:
            wait_count++;
            if (g_puback_received) {
                /* 收到了 PUBACK，继续执行 */
                g_test_step = TEST_STEP_GET_TOPICS;
            } else if (wait_count > 5) {  /* 等待 5 秒 */
                cm_printf("[MQTT] ERROR: Publish timeout\r\n");
                g_test_result = -1;
                g_test_step = TEST_STEP_GET_TOPICS;
            }
            break;

        case TEST_STEP_GET_TOPICS:
            mqtt_test_get_topics();
            g_test_step = TEST_STEP_DISCONNECT;
            break;

        case TEST_STEP_DISCONNECT:
            if (mqtt_test_disconnect() == 0) {
                g_test_step = TEST_STEP_WAIT_DISCONNECT;
                wait_count = 0;
            } else {
                g_test_step = TEST_STEP_DESTROY;
            }
            break;

        case TEST_STEP_WAIT_DISCONNECT:
            wait_count++;
            if (wait_count > 3) {
                g_test_step = TEST_STEP_DESTROY;
            }
            break;

        case TEST_STEP_DESTROY:
            mqtt_test_destroy();
            g_test_step = TEST_STEP_COMPLETED;
            break;

        case TEST_STEP_COMPLETED:
            cm_printf("\r\n[MQTT] ============================================\r\n");
            if (g_test_result == 0) {
                cm_printf("[MQTT] TEST COMPLETED SUCCESSFULLY!\r\n");
            } else {
                cm_printf("[MQTT] TEST FAILED!\r\n");
            }
            cm_printf("[MQTT] ============================================\r\n");
            break;

        default:
            /* 定期检查连接状态 */
            if (g_mqtt_client != NULL) {
                int state = cm_mqtt_client_get_state(g_mqtt_client);
                static int last_state = -1;
                if (state != last_state) {
                    cm_printf("[MQTT] State changed: %d -> %d\r\n", last_state, state);
                    last_state = state;

                    /* 如果在测试过程中连接意外断开 */
                    if (state == CM_MQTT_STATE_DISCONNECTED &&
                        g_test_step != TEST_STEP_IDLE &&
                        g_test_step != TEST_STEP_DESTROY &&
                        g_test_step != TEST_STEP_COMPLETED) {
                        cm_printf("[MQTT] WARNING: Unexpected disconnection!\r\n");
                        g_test_result = -1;
                        g_test_step = TEST_STEP_DESTROY;
                    }
                }
            }
            break;
    }
}

/****************************************************************************/
/* 主函数 */
/****************************************************************************/

/**
 * @brief 等待网络就绪
 */
static void wait_for_network(void)
{
    cm_cereg_state_t cereg = {0};
    int retry_count = 0;
    const int max_retries = 120;  /* 最多等待 2 分钟 */

    cm_printf("[MQTT] Waiting for network to be ready...\r\n");

    while (retry_count < max_retries) {
        cm_modem_get_cereg_state(&cereg);
        if (cereg.state == 1) {
            cm_printf("[MQTT] Network is ready!\r\n");
            break;
        }
        cm_printf("[MQTT] Waiting for network... (state=%d, retry=%d/%d)\r\n", 
                 cereg.state, retry_count + 1, max_retries);
        osDelay(1000);
        retry_count++;
    }

    if (retry_count >= max_retries) {
        cm_printf("[MQTT] Warning: Network timeout, proceeding anyway...\r\n");
    }
}

int main(void)
{
    /* 避免睡眠模式导致输出异常 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_work_lock();

    cm_eloop_init_default();

    /* 等待网络就绪 */
    wait_for_network();

    /* 启动 MQTT 测试 */
    g_test_step = TEST_STEP_INIT;

    cm_printf("\r\n[MQTT] ============================================\r\n");
    cm_printf("[MQTT] Starting %s Test\r\n", MQTT_USE_SSL ? "MQTTS" : "MQTT");
    cm_printf("[MQTT] Server: %s:%d\r\n", MQTT_SERVER, MQTT_PORT);
    cm_printf("[MQTT] Client ID: %s\r\n", MQTT_CLIENT_ID);
    cm_printf("[MQTT] Topic: %s\r\n", MQTT_TOPIC);
    cm_printf("[MQTT] ============================================\r\n\r\n");

    /* 主循环 */
    while (1) {
        mqtt_test_task();
        osDelay(1000);  /* 100ms 轮询周期 */

        /* 测试完成或失败时退出循环 */
        if (g_test_step == TEST_STEP_COMPLETED) {
            break;
        }
    }

    cm_printf("[MQTT] Test program exiting...\r\n");

    return g_test_result;  /* 返回测试结果：0=成功，-1=失败 */
}
