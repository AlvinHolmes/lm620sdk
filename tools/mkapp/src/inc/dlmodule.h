/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/08/11     Bernard      the first version
 */

#ifndef RT_DL_MODULE_H__
#define RT_DL_MODULE_H__
#include <stdint.h>

typedef struct {
    /* VMA base address for the first LOAD segment */
    uint32_t    vstart_addr;
    void*       mem_space;  /* memory space */
    uint32_t    mem_size;   /* sizeof memory space */
}dlmodule_t;

int32_t dlmodule_link_check(const char* modfile, const char* symfile);
int32_t dlmodule_symbol_find(const char *sym_str);

#endif
