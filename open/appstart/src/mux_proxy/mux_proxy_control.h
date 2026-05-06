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



#ifndef __MUX_PROXY_CONTROL_H__
#define __MUX_PROXY_CONTROL_H__


/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

typedef struct
{
    uint32_t cmdId; //命令必须内容, 参考MUX_ProxyControlCommand
    uint32_t dataLen; //命令必须内容,命令数据长度
} MuxProxy_CommandInfoHeader;

typedef struct
{
    MuxProxy_CommandInfoHeader header; //消息头必须内容
    uint8_t msgData[]; //消息可选内容
} MuxProxy_CommandInfo;

#define MUX_CONTROL_COMMAND_INFO_HEADER_SIZE (sizeof(MuxProxy_CommandInfoHeader))

typedef enum
{
    MUX_PROXY_CTRLCMD_TEST   = 0,         //   通路测试命令, 收到后打印
    MUX_PROXY_CTRLCMD_QUERY  = 1,         //   收到后回复 MUX_PROXY_CTRLCMD_ANSWER
    MUX_PROXY_CTRLCMD_ANSWER,             //   对 MUX_PROXY_CTRLCMD_QUERY 的响应
    MUX_PROXY_CTRLCMD_IP_CONNECT,         //   IP网络连接,对应PDP激活,和RNDIS connect
    MUX_PROXY_CTRLCMD_IP_DISCONNECT,      //   IP网络断开,对应PDP去激活,和RNDIS disconnect
    MUX_PROXY_CTRLCMD_IP_PAUSE,           //   暂停网络数据传输,用于流控暂停数据
    MUX_PROXY_CTRLCMD_IP_RESUME,          //   恢复网络数据传输,用于流控恢复数据
    MUX_PROXY_CTRLCMD_MAC_CONNECT,        //   MAC断开连接,对应RNDIS枚举完成
    MUX_PROXY_CTRLCMD_MAC_DISCONNECT,     //   MAC断开连接,对应RNDIS禁用
    MUX_PROXY_CTRLCMD_MAC_REMOVE,         //   MAC断开连接,对应RNDIS拨出
    MUX_PROXY_CTRLCMD_CHL_STATUS,         //   通道状态发布,通知对端通道的OPEN/CLOSE, 暂不使用
    MUX_PROXY_CTRLCMD_MAX
} MuxProxy_CommandId; //MUX控制命令, 用于跟对端通信

typedef enum
{
    MUX_PROXY_CTRLMSG_BASE = MUX_PROXY_CTRLCMD_MAX,
    MUX_PROXY_CTRLMSG_MUX_CONNECT,        //   MUX协议连接消息
    MUX_PROXY_CTRLMSG_MUX_DISCONNECT,     //   MUX协议断开连接消息
} MuxProxy_TaskMessageId; //MUX任务消息, 不能用于跟对端通信

typedef struct
{
    uint8_t mac_addr[MUX_PROXY_MAC_ADDR_LEN];  //   MAC地址
    uint8_t ip_type;                           //   4:IPV4   6:IPV6
    uint32_t ip_addr[4];                       //   IP地址兼容V4和V6,当时V4时用ip_addr[0],网络字节序
} MuxProxy_GateWayInfo; //   网关信息

typedef struct
{
    uint16_t channel_status; //   按bit使用,协议定义最多16个通道, 每个bit位对应一个通道,比如bit2对应2通道, 0表示关闭,1表示打开
} MuxProxy_ChannelStatusInfo;

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
int32_t MuxProxy_Control_Init(void);
int8_t MuxProxy_Control_MuxInit(void);
void MuxProxy_Control_MuxCleanChannel(void);
void MuxProxy_Control_MuxRestartChannel(void);
int32_t MuxProxy_Control_MuxTest(void);
int32_t MuxProxy_Control_MuxUp(void);
int32_t MuxProxy_Control_MuxDown(void);
void MuxProxy_Control_ShellShow(void);

#ifdef __cplusplus
}
#endif
#endif//__MUX_PROXY_CONTROL_H__


