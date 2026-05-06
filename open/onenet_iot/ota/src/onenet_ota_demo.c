/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief:OneNET OTA token
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-28     ict team          创建
 ************************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdlib.h>
#include <ota_user.h>
#include "os.h"
#include "nr_micro_shell.h"
#include "onenet_ota_app.h"


/************************************************************************************
 *                                 函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_DEMO_FIRMWARE_PATH                    "/usr/"
#define OTA_DEMO_FIRMWARE_NAME                    "ota.bin"


/************************************************************************************
 *                                 类型定义
 ************************************************************************************/


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/


/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/


/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/

static void ONENET_OTA_HttpDemoEntry(char argc, char **argv)
{
    if(argc > 9) /* 命令行参数个数超界, 不做处理, 返回 */
    {
        osPrintf("ota_http usage as follow:\r\n");
        osPrintf("    ota_http cfg <product_id> <dev_name> <et> <access_key> <method> <file_path> <file_name> \
                <f_version> <s_version> <dev_version>\r\n");
        osPrintf("    ota_http start/stop\r\n");
        return;
    }

    osPrintf("onenet ota version is v1.2.1 at %s/%s\r\n", __DATE__, __TIME__);
    if(!strcmp(argv[1], "cfg"))
    {
        OTA_AppParamInit();
        OTA_AppProductIdSet(argv[2]);
        OTA_AppDevNameSet(argv[3]);
        OTA_AppEtSet(atoi(argv[4]));
        OTA_AppAccessKeySet(argv[5]);
        OTA_AppKeyMethodSet(atoi(argv[6]));
        OTA_AppFilePathSet(argv[7]);
        OTA_AppFileNameSet(argv[8]);
        OTA_AppFVersionSet("V2.0");
        OTA_AppSVersionSet("V1.3");
        OTA_AppDevVersionSet("V1.2");
    }
    else if(!strcmp(argv[1], "start"))
    {
        OTA_AppStart(); /* HTTPS 时, workqueue 栈空间不够, 因此创建一个新任务 */
    }
    else if(!strcmp(argv[1], "stop"))
    {
        OTA_AppStop();
    }
    else
    {
        osPrintf("ota_http usage as follow:\r\n");
        osPrintf("    ota_http cfg <product_id> <dev_name> <et> <access_key> <method> <file_path> <file_name> \
                <f_version> <s_version> <dev_version>\r\n");
        osPrintf("    ota_http start/stop\r\n");
    }

    return;
}

/* 获取上次 OTA 升级的结果 */
static void ONENET_OTA_ReadDemoEntry(char argc, char **argv)
{
    int32_t ret = -1;

    ret = OTA_GetResult();
    if(OTA_ERROR_OK == ret)
    {
        osPrintf("onenet ota upgrade succ\r\n");
    }
    else
    {
        osPrintf("onenet ota upgrade fail[%d]\r\n", ret);
    }

    return;
}

static void ONENET_OTA_UpgradeDemoEntry(char argc, char **argv)
{
    int32_t ret = -1;
    char *pathName = NULL;

    pathName = OTA_AppPathNameGet(OTA_DEMO_FIRMWARE_PATH, OTA_DEMO_FIRMWARE_NAME);
    if(NULL == pathName)
    {
        osPrintf("ota demo pathName is null\r\n");
        return;
    }
    osPrintf("ota demo pathName is [%s]\r\n", pathName);

    ret = OTA_CheckPackage(pathName, NULL);
    osPrintf("ota demo upgrade CheckPackage ret[%d]\r\n", ret);
    if (ret == OTA_ERROR_OK)
    {
        osThreadMsSleep(500);
        osPrintf("ota demo upgrade start\r\n");
        ret = OTA_StartUpgrade(pathName);
        if(OTA_ERROR_OK == ret)
        {
            osThreadMsSleep(500);
            //do reboot, and should never return
            osShutdown(OS_REBOOT, OS_REBOOT_FOTA);
            osThreadSuspend(osThreadSelf());
            OS_ASSERT(0);
        }
    }
    else
    {
        osPrintf("ota demo upgrade check fail[%d], stop\r\n", ret);
    }

    osFree(pathName);
    pathName = NULL;

    return;
}

NR_SHELL_CMD_EXPORT(ota_http, ONENET_OTA_HttpDemoEntry);
NR_SHELL_CMD_EXPORT(ota_read, ONENET_OTA_ReadDemoEntry);
NR_SHELL_CMD_EXPORT(ota_upgrade, ONENET_OTA_UpgradeDemoEntry);


#ifdef __cplusplus
}
#endif

