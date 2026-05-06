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
#include "stdlib.h"
#include "os.h"
#include <nr_micro_shell.h>

#include "bus_mux_api.h"


#include "mux_proxy_api.h"
#include "mux_proxy_pub.h"
#include "mux_proxy_control.h"
#include "mux_proxy_at.h"
#include "mux_proxy_ip.h"
#include "mux_proxy_pass_through.h"
#include "at_api.h"
#include "net_api.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
extern BUS_Tunnel *SPI_DeviceSyncTunnelGet(void);
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
static BUS_MuxHandle *g_BusMuxHdl = NULL;

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void MuxProxy_Callback(void *param, BUS_MUX_EVENT event)
{
    switch (event) {
        case BUS_MUX_EVENT_LINKUP:
            MuxProxy_Control_MuxUp();
            break;

        case BUS_MUX_EVENT_LINKDOWN:
            MuxProxy_Control_MuxDown();
            break;
        
        default:
            break;
    }
}

static void MuxProxy_InitiateSPIAndBusMux(void)
{
    g_BusMuxHdl = BUS_MuxCreate("spimux", SPI_DeviceSyncTunnelGet(), MuxProxy_Callback, NULL); //   初始化bus mux

    OS_ASSERT(g_BusMuxHdl != NULL);
}

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
BUS_MuxChannel *MuxProxy_InsertBusMuxChannel(uint8_t id, const char *name, uint8_t priority, BUS_EventFunc callback, void *param)
{
    BUS_MuxChannel *chl = BUS_MuxInsert(g_BusMuxHdl, id, name, priority, callback, param);

    OS_ASSERT(chl != NULL);

    return chl;
}
void MuxProxy_SlaveInit(void)
{
    MuxProxy_Control_Init();
    MuxProxy_InitiateSPIAndBusMux();
    MuxProxy_Control_MuxInit();

    return;
}

//  用于复位Proxy模块，清理状态
void MuxProxy_Reset(void)
{
    return;
}


#define MUX_TEST_CMD_SHOWINFO 0
#define MUX_TEST_CMD_INIT 1
#define MUX_TEST_CMD_IP_CONNECT 2
#define MUX_TEST_CMD_IP_DISCONNECT 3
#define MUX_TEST_CMD_AT_SEND 4
#define MUX_TEST_CMD_IP_SEND 5
#define MUX_TEST_CMD_MUXTEST 6
#define MUX_TEST_CMD_IP_ACTIVE 7
#define MUX_TEST_CMD_IP_DEACTIVE 8
#define MUX_TEST_CMD_IP_PAUSE 9
#define MUX_TEST_CMD_IP_RESUME 10
#define MUX_TEST_CMD_IP_CLRSTATS 11

void sproxy_debug(char argc, char **argv)
{
    int i;
    int test_cmd = 0;
    if(argc == 1)
    {
        osPrintf("Master MUX Test Help\r\n");
        osPrintf("[%d] Show information\r\n", MUX_TEST_CMD_SHOWINFO);
        osPrintf("[%d] Slave Init\r\n", MUX_TEST_CMD_INIT);
        osPrintf("[%d] Send IP Connect\r\n", MUX_TEST_CMD_IP_CONNECT);
        osPrintf("[%d] Send IP Disconnect\r\n", MUX_TEST_CMD_IP_DISCONNECT);
        //osPrintf("[%d] AT [OK] Send Test\r\n", MUX_TEST_CMD_AT_SEND);
        //osPrintf("[%d] IP Send Test\r\n", MUX_TEST_CMD_IP_SEND);
        osPrintf("[%d] Send Mux Test\r\n", MUX_TEST_CMD_MUXTEST);
        osPrintf("[%d] Send IP Active\r\n", MUX_TEST_CMD_IP_ACTIVE);
        osPrintf("[%d] Send IP Deactive\r\n", MUX_TEST_CMD_IP_DEACTIVE);
        osPrintf("[%d] Send IP Pause\r\n", MUX_TEST_CMD_IP_PAUSE);
        osPrintf("[%d] Send IP Resume\r\n", MUX_TEST_CMD_IP_RESUME);
        osPrintf("[%d] Clear IP Statistic\r\n", MUX_TEST_CMD_IP_CLRSTATS);
    }
    else
    {
        for(i=0; i < argc; i++)
        {
            osPrintf("param%d [%s] ", i, argv[i]);
        }
        osPrintf("\r\n");
        test_cmd = atoi(argv[1]);

        if(test_cmd == MUX_TEST_CMD_SHOWINFO){
            MuxProxy_Control_ShellShow();
            MuxProxy_IP_ShowStatistic();
        }

        if(test_cmd == MUX_TEST_CMD_INIT){
            MuxProxy_SlaveInit();
        }

        if(test_cmd == MUX_TEST_CMD_IP_CONNECT){
            //uint8_t mac_addr[6] = {0x34, 0x97, 0xf6, 0x94, 0xea, 0x55};
            //uint32_t ip_addr = 0xc0a87201;// 192.168.114.1

            //MuxProxy_Control_IpConnect(ip_addr, mac_addr);
            char * urc = "\r\n+CMGIPDNS: 1,1,\"IP\",\"10.55.138.212\",\"0.0.0.0\",\"211.136.17.107\",\"211.136.20.203\"\r\n";
            AT_SendUnsolicited(0, urc, strlen(urc));
        }

        if(test_cmd == MUX_TEST_CMD_IP_DISCONNECT){
            //MuxProxy_Control_IpDisconnect();
            char * urc = "\r\n+CGEV: ME PDN DEACT 1\r\n";
            AT_SendUnsolicited(0, urc, strlen(urc));
        }

        if(test_cmd == MUX_TEST_CMD_MUXTEST)
        {
            osPrintf("Mux Test Send [%d]\r\n", MuxProxy_Control_MuxTest());
        }

        if(test_cmd == MUX_TEST_CMD_IP_ACTIVE)
        {
            osPrintf("Mux IP Active [%d]\r\n", NET_MgrPdpNetDevCtl(1,1));
        }

        if(test_cmd == MUX_TEST_CMD_IP_DEACTIVE)
        {
            osPrintf("Mux IP Deactive [%d]\r\n", NET_MgrPdpNetDevCtl(0,1));
        }

        if(test_cmd == MUX_TEST_CMD_IP_PAUSE)
        {
            MuxProxy_Control_IpPause();
        }

        if(test_cmd == MUX_TEST_CMD_IP_RESUME)
        {
            MuxProxy_Control_IpResume();
        }

        if(test_cmd == MUX_TEST_CMD_IP_CLRSTATS)
        {
            MuxProxy_IP_ClearStatistic();
        }
    }
    return;
}

NR_SHELL_CMD_EXPORT(sproxy, sproxy_debug);


