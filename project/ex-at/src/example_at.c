/**
 *************************************************************************************
 * 版权所有 (C) 2024, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2024-10-22     ict team          创建
  ************************************************************************************
  */

 /************************************************************************************
  *                                 头文件
  ************************************************************************************/
#include <stdbool.h>
#include "os.h"
#include "nr_micro_shell.h"
#include "at_api.h"
#include "at_parser.h"
#include "at_client_api.h"


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

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
/* AT命令添加示例 Begin */
void AT_CMD_DemoSet(uint8_t channelId, uint8_t *cmd, uint16_t cmdLen)
{
    uint8_t mode = 0;
    at_errno_t err;
    char *rsp;
    err = at_parser_parse_cmd((char *)cmd, cmdLen, "%hhu", &mode);

    if(err == AT_ERRNO_NO_ERROR && mode == 1)
    {
        rsp = "\r\n+DEMO: SET 1\r\n";

        AT_SendResponse(channelId, rsp, strlen(rsp));
        AT_SendResponseOK(channelId);
    }
    else
    {
        AT_SendResponseCMEError(channelId, CME_ERR_UNVALID_PARAM);
    }
}
void AT_CMD_DemoRead(uint8_t channelId)
{
    char *rsp = "\r\n+DEMO: READ\r\n";

    AT_SendResponse(channelId, rsp, strlen(rsp));
    AT_SendResponseOK(channelId);
}
void AT_CMD_DemoTest(uint8_t channelId)
{
    char *rsp = "\r\n+DEMO: TEST\r\n";

    AT_SendResponse(channelId, rsp, strlen(rsp));
    AT_SendResponseOK(channelId);
}
void AT_CMD_DemoExec(uint8_t channelId)
{
    char *rsp = "\r\n+DEMO: EXEC\r\n";

    AT_SendResponse(channelId, rsp, strlen(rsp));
    AT_SendResponseOK(channelId);
}

AT_CMD_TABLE_SECTION_USER  const AT_CommandItem g_atCmdTableDemo[] =
{
    {"+DEMO",       AT_CMD_DemoSet,             AT_CMD_DemoRead,         AT_CMD_DemoTest,           AT_CMD_DemoExec},
};


void AT_SendExampleResponse(uint8_t channelId, char *rsp, uint32_t rspLen, void *userdata)
{
	osPrintf("AT_SendExample response : [%s]!\r\n", rsp);
}

/* AT命令添加示例 End */

/* Open-CPU AT命令发送示例 Begin */
void AT_SendExample(void)
{
	int32_t ret;

    char *urc1 = "\r\n+DEMO: 666\r\n";
	//   广播方式发送主动上报
    ret = AT_SendUnsolicited(0, urc1, strlen(urc1));
	if (ret == osOK)
	{
		osPrintf("AT_SendExample broadcast URC [%s] successfully!\r\n", urc1);
	}
	//   指定通道发送主动上报
    char *urc2 = "\r\n+DEMO: 888\r\n";
	ret = AT_SendUnsolicited(AT_CHANNEL_ID_UART, urc2, strlen(urc2));
	if (ret == osOK)
	{
		osPrintf("AT_SendExample UART send URC [%s] successfully!\r\n", urc2);
	}

	char *cmd = "AT+DEMO=1\r\n";
	//   阻塞方式发送，直到收到AT命令最终响应才退出
	ret = AT_Client_SendCommand(cmd, strlen(cmd), true, AT_SendExampleResponse, NULL);
	if (ret == osOK)
	{
		osPrintf("AT_SendExample block send [%s] successfully!\r\n", cmd);
	}

	//   直接发送，不处理命令响应
	ret = AT_Client_SendCommandWaitResult("ATE0\r\n");
	if (ret == osOK)
	{
		osPrintf("AT_SendExample executed [ATE0] successfully!\r\n");
	}

	//   非阻塞方式发送，命令发送即退出
	ret = AT_Client_SendCommand(cmd, strlen(cmd), false, AT_SendExampleResponse, NULL);
	if (ret == osOK)
	{
		osPrintf("AT_SendExample noeblock send [%s] successfully!\r\n", cmd);
	}

	return;
}

/* Open-CPU AT命令发送示例 End */
/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
static void example_entry(char argc, char **argv)
{
	AT_SendExample();
}

/* 注册SHELL命令 */
NR_SHELL_CMD_EXPORT(at_example, example_entry);
