/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file app_main.c
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
#include "rtmsym.h"
#include "vfs.h"
#include "ap_ps_interface.h"
#include "svc_equipment.h"
#include <nvm.h>

typedef struct {
	int a;
	int b;
	int c;
}T_sAPP;

int count, count_0 = 0, count_1 = 1;
static int scount, scount_0 = 0, scount_1 = 1;
static T_sAPP s_app[3] = {{.a = 1, .b = 1, .c = 1}};

void app_thread_entry_1(void *parameter)
{
	osMessageQueueId_t mq = (osMessageQueueId_t)parameter;
	T_sAPP msg = {0};

	while(1) {
		if (osMessageQueueGet(mq, &msg, 0, osWaitForever) == osOK) {
			osPrintf("[app] thread_1 osMessageQueueGet a = %d, b = %d, c = %d \r\n", msg.a, msg.b, msg.c);
		}
	}
}

void app_thread_entry_2(void *parameter)
{
	osMessageQueueId_t mq = (osMessageQueueId_t)parameter;
	T_sAPP msg = {0};

	msg.a = 1;
	msg.b = 2;
	msg.c = 3;

	while(1) {
		msg.a++;
		msg.b++;
		msg.c++;
		osMessageQueuePut(mq, (void *)&msg, 0, osWaitForever);
		osPrintf("[app] thread_2 osMessageQueuePut a = %d, b = %d, c = %d \r\n", msg.a, msg.b, msg.c);
		osThreadMsSleep(2000);
	}
}

void app_main(void *param)
{
	T_sRtmSymInfo * symtab = (T_sRtmSymInfo *)param;

	rtmsym_initialize(symtab); // 必须先初始化RTM 才能调用后面的函数
	osPrintf("[app] symtab start: 0x%08x, end: 0x%08x \r\n", (int)symtab->start, (int)symtab->end);

	osPrintf("[app] I am app version 1.0.0 \r\n");

	// TODO: scv & ps接口测试
	char imei[16] = {0};
	Api_GetImei(imei);
	osPrintf("[app] IMEI: %s \r\n", imei);

	char sn[32] = {0};
	svc_eqp_read_boardnum(sn, sizeof(sn));
	osPrintf("[app] Board Number: %s \r\n", sn);

	// TODO: NV接口测试
	uint8_t dummy = 0;
	NVM_Read(NV_ITEM_ADDR(customCfg.dummy), (uint8_t *)&dummy, sizeof(dummy));
	osPrintf("[app] nv dummy: %d \r\n", dummy);
	dummy = 100;
	NVM_Write(NV_ITEM_ADDR(customCfg.dummy), (uint8_t *)&dummy, sizeof(dummy));

	// TODO: 全局变量测试
	for (int i = 0; i < sizeof(s_app) / sizeof(s_app[0]); i++) {
		osPrintf("[app] s_app[%d].a = %d, b = %d, c = %d \r\n", i, s_app[i].a, s_app[i].b, s_app[i].c);
	}
	osPrintf("[app] count: %d, count_0: %d, count_1: %d \r\n", count, count_0, count_1);
	osPrintf("[app] scount: %d, scount_0: %d, scount_1: %d \r\n", scount, scount_0, scount_1);

	// TODO: 文件系统测试
	VFS_File * fp = VFS_OpenFile("/usr/test.txt", "wb+");
	if (fp == NULL) {
		osPrintf("[app] VFS_OpenFile error \r\n", errno);
	}
	OS_ASSERT(fp);

	if (VFS_WriteFile("1234567890", 1, strlen("1234567890"), fp) != strlen("1234567890")) {
		osPrintf("[app] VFS_WriteFile error, %d \r\n", errno);
	}

	VFS_SeekFile(fp, 0, VFS_SEEK_SET);

	char file_buf_read[128] = {0};

	if (VFS_ReadFile(file_buf_read, 1, 10, fp) < 0) {
		osPrintf("[app] VFS_ReadFile error, %d \r\n", errno);
	}
	osPrintf("[app] read file: %s \r\n", file_buf_read);

	VFS_CloseFile(fp);

	// TODO: 消息队列测试
	osMessageQueueId_t mq_id = NULL;
	mq_id = osMessageQueueNew(10,sizeof(T_sAPP),NULL);
	OS_ASSERT(mq_id);

	// TODO: 线程测试
    osThread_t tid_1;
    osThreadAttr_t attr_1 = {"app_1", osThreadDetached, NULL, 0U, NULL, 2048, 8, 0U, 0U};
    tid_1 = osThreadNew(app_thread_entry_1, (void *)mq_id, &attr_1);
    OS_ASSERT(tid_1);

	osThread_t tid_2;
	osThreadAttr_t attr_2 = {"app_2", osThreadDetached, NULL, 0U, NULL, 2048, 8, 0U, 0U};
	tid_2 = osThreadNew(app_thread_entry_2, (void *)mq_id, &attr_2);
	OS_ASSERT(tid_2);

	osThreadJoin(tid_1);
	osThreadJoin(tid_2);
}


