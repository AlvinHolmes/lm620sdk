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
#include <os.h>
#include <drv_common.h>
#include <drv_lp.h>
#include <drv_flash.h>
#include <dlmodule.h>
#include "nr_micro_shell.h"

#ifndef OS_USING_MODULE
#error "OS_USING_MODULE Not define"
#endif

#define BLOCK_SIZE          (4096)

#define FLAG_SIZE           (2*BLOCK_SIZE)

#define IMGS_SIZE(part)     ((PART_Size(part)-FLAG_SIZE)/2)

#define IMGA_ADDR           0
#define IMGB_ADDR           IMGS_SIZE

#define DLMO_PART_NAME      "dlmo"
/**
 *
 * dlmo partition 320KB, including two images & boot flag
 *
 * layout:
 * -------------------------------------------------
 * |  A         |      B         | flag0  | flag1  |
 * -------------------------------------------------
 *
 * flag = 8KB
 *
 * A = B = (320-8)/2 = 156KB
 *
 */

static int getBootImage(const char *name)
{
    //PART_Handle *part = PART_Find(name);

    // read flags

    // return image addr
    return (int)IMGA_ADDR;
}

int main(void)
{
    /** run module on flash part 'dlmo'   */
    int off = getBootImage(DLMO_PART_NAME);

    if (off >= 0) {
        osPrintf("Execute module image: %s[%Xh]\n", DLMO_PART_NAME, off);
        dlimage_exec(DLMO_PART_NAME, off, DLMO_PART_NAME " a1 a2 a3");
    } else {
        osPrintf("Not found valid image\n");
    }

    return 0;
}

static void (*g_msgCb)(const char*);

void dlmoSetMsgCallback(void (*pfn)(const char*))
{
    g_msgCb = pfn;
}
RTM_EXPORT(dlmoSetMsgCallback);

static void SHELL_dlmo(char argc, char **argv)
{
    if (g_msgCb && argc > 1) {
        g_msgCb(argv[1]);
    }
}
NR_SHELL_CMD_EXPORT(dlmo, SHELL_dlmo);



