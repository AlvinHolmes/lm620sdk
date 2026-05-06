/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file main.c
*
* @brief  main函数入口文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/
#include <stdio.h>
#include <os.h>
#include <vfs.h>

#define MSG_LEN         64

extern void dlmoSetMsgCallback(void (*pfn)(const char*));

static osMessageQueueId_t g_mq;

static void dlmoSendMsg(const char *msg)
{
    if (g_mq) {
        osMessageQueuePut(g_mq, msg, 0, osWaitForever);
    }
}

int main(int argc, char *argv[])
{
    char buf[MSG_LEN+1];

    if (argc) {
        for (int i = 0; i < argc; i++) {
            osPrintf("%s ", argv[i]);
        }
        osPrintf("\n");
    }

    g_mq = osMessageQueueNew(5, MSG_LEN, OS_NULL);
    if (g_mq == OS_NULL)
        return -1;

    dlmoSetMsgCallback(dlmoSendMsg);
    while (1) {
        osMessageQueueGet(g_mq, buf, 0, osWaitForever);
        buf[MSG_LEN] = 0;
        osPrintf("Recv msg:%s\n", buf);
        if (osStrcmp(buf, "exit") == 0)
            break;
    }
    dlmoSetMsgCallback(OS_NULL);

    osMessageQueueDelete(g_mq);
    g_mq = OS_NULL;

    osPrintf("Exit module\n");
    return 0;
}

