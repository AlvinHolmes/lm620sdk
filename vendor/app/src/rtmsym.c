/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file
*
* @brief
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/
#include <os.h>
#include "rtmsym.h"

static T_sRtmSymInfo g_symtab = {0};
// void * g_osPrintf_symbol = NULL;

int rtmsym_initialize(const T_sRtmSymInfo * symtab)
{
    if (symtab == NULL || symtab->start == NULL || symtab->end == NULL) {
        return -1;
    }

    g_symtab.start = symtab->start;
    g_symtab.end = symtab->end;

    // g_osPrintf_symbol = rtmsym_find("osPrintf");
    // if (g_osPrintf_symbol == NULL) {
    //     return -2;
    // }

    return 0;
}

void * rtmsym_find(const char * symbol)
{
    void * p_addr = NULL;
    const struct osModuleSymtab * p = NULL;

    for ( p = g_symtab.start; p < g_symtab.end; p++ ) {
        if (strcmp(p->name, symbol) == 0) {
            p_addr = p->addr;
            break;
        }
    }

    return p_addr;
}