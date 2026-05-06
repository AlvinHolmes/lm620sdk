/**
 * @file example_main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-12-26
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

#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

// #include "cm_os.h"
#include "cm_sys.h"
#include "cm_sim.h"
#include "cm_modem_info.h"
#include "cm_modem.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TARGET_IP_ADDR "223.5.5.5"
#define TARGET_PORT     80

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
    int sockfd;
    struct sockaddr_in server_addr;
    ip_addr_t target_ip;

    cm_printf("--- LwIP Connectivity Check Start ---\n");

#if 0
    /* 延迟一定时间等待网络链接 */
    osDelay(20 * 1000);
#else
    cm_cereg_state_t cereg = {
        .state = 0xff,
    };
    do {
        osDelay(1000);
        cm_printf("Waiting for network to be ready...\n");
        cm_modem_get_cereg_state(&cereg);
    } while (cereg.state != 1);
#endif

    if (!ipaddr_aton(TARGET_IP_ADDR, &target_ip)) {
        cm_printf("Error: Invalid IP address format: %s\n", TARGET_IP_ADDR);
        return -1;
    }

    sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cm_printf("Error: Failed to create socket.\n");
        return -1;
    }
    cm_printf("Socket created successfully, fd=%d\n", sockfd);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TARGET_PORT);

    memcpy(&(server_addr.sin_addr), ip_2_ip4(&target_ip), sizeof(struct in_addr));

    cm_printf("Attempting to connect to %s:%d...\n", TARGET_IP_ADDR, TARGET_PORT);
    if (lwip_connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
        cm_printf("SUCCESS: Connected to the internet!\n");
        cm_printf("--- LwIP Connectivity Check End ---\n");
        lwip_close(sockfd);
        return 0;
    } else {
        cm_printf("FAILURE: Failed to connect to the internet.\n");
        cm_printf("--- LwIP Connectivity Check End ---\n");
        lwip_close(sockfd);
        return -1;
    }
    return 0;
}
