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

/************************************************************************************
*                                 头文件
************************************************************************************/

#include "os.h"
#include "bus_mux_api.h"
#include "mux_proxy_api.h"
#include "mux_proxy_pub.h"
#include "mux_proxy_pass_through.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static BUS_MuxChannel *g_MuxProxy_passthrough_MuxChl = NULL;

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/


int32_t MuxProxy_PassThrough_Write(uint8_t *data, uint32_t len, uint32_t timeout)
{
    BUS_MuxErrCode err = BUS_MuxSendPool(g_MuxProxy_passthrough_MuxChl, data, len, timeout);
    if(err != BUSMUX_OK) {
        return osErrorTimeout;
    }

    return osOK;
}

int32_t MuxProxy_PassThrough_Read(uint8_t *data, uint32_t depth, uint32_t *lenOut, uint32_t timeout)
{
    BUS_EventMsg msg = {0};

    BUS_MuxErrCode err = BUS_MuxReadPool(g_MuxProxy_passthrough_MuxChl, &msg, data, depth, timeout);
    if(err != BUSMUX_OK) {
        *lenOut = 0;
        return osErrorTimeout;
    }

    *lenOut = depth < msg.len[0] ? depth : msg.len[0];
    memcpy(data, msg.data[0], *lenOut);

    return osOK;
}

int8_t MuxProxy_PassThrough_MuxInit(void)
{
    APP_MUX_PRINT_INFO("MuxProxy_PassThrough_MuxInit\r\n");

    if(g_MuxProxy_passthrough_MuxChl == NULL) {
        g_MuxProxy_passthrough_MuxChl = MuxProxy_InsertBusMuxChannel(MUX_PROXY_CHANNEL_PASS_THROUGH, "p-thro", MUX_PROXY_RAMDUMP_BUSMUX_CHANNEL_PRIORITY,
                                NULL, NULL);
    }
    return osOK;
}

