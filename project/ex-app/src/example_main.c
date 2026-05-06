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
#include "appload.h"

#ifndef OS_USING_MODULE
#error "OS_USING_MODULE Not define"
#endif

extern int8_t appimg_exec(const char* part, uint32_t offset,void *param);

#define BLOCK_SIZE          (4096)

#define FLAG_SIZE           (2*BLOCK_SIZE)

#define IMGS_SIZE(part)     ((PART_Size(part)-FLAG_SIZE)/2)

#define IMGA_ADDR           0
#define IMGB_ADDR           IMGS_SIZE

#define APP_PART_NAME      "app"

typedef struct {
    struct osModuleSymtab * start;
    struct osModuleSymtab * end;
}T_sRtmSymInfo;


/**
 *
 * app partition 240KB, including two images & boot flag
 *
 * layout:
 * -------------------------------------------------
 * |  A         |      B         | flag0  | flag1  |
 * -------------------------------------------------
 *
 * flag = 8KB
 *
 * A = B = (240-8)/2 = 116KB
 *
 */

static int getBootImage(const char *name)
{
    //PART_Handle *part = PART_Find(name);

    // read flags

    // return image addr
    return (int)IMGA_ADDR;
}

extern uint32_t __rtmsymtab_start;
extern uint32_t __rtmsymtab_end;
T_sRtmSymInfo symtab = {0}; // 必须使用全局变量

int main(void)
{
    osPrintf("[kernel] I am kernel v1 \r\n");
    /** run module on flash part 'app'   */
    int off = getBootImage(APP_PART_NAME);

    symtab.start = (struct osModuleSymtab *)&__rtmsymtab_start;
    symtab.end = (struct osModuleSymtab *)&__rtmsymtab_end;

    osPrintf("[kernel] symtab start: 0x%08x, end: 0x%08x \r\n", symtab.start, symtab.end);

    if (off >= 0) {
        osPrintf("[kernel] Execute app image: %s[%Xh]\r\n", APP_PART_NAME, off);
        appimg_exec(APP_PART_NAME, off, (void *)&symtab);
    } else {
        osPrintf("[kernel] Not found valid image\r\n");
    }


    return 0;
}
