/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author      Notes
 * 2018/08/29     Bernard     first version
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h> /* statfs */
#include <dirent.h>

#include "dlmodule.h"
#include "dlelf.h"


#define LOG(fmt, ...)       printf("mkldmo: " fmt "\r\n", ##__VA_ARGS__)

static const char** g_symtabel;
static uint32_t     g_symcount;

static int32_t prvBuildSymtab(const char *buf, uint32_t len)
{
    const char *p = buf;
    while (p < buf + len) {
        if (p[0]) {
            g_symtabel = (const char**)realloc(g_symtabel, (g_symcount+1)*sizeof(const char*));
            if (NULL == g_symtabel)
                return -1;
            g_symtabel[g_symcount++] = p;
            p = p + strlen(p);
        } else {
            p++;
        }
    }
    return 0;
}

static uint8_t* prvLoadFile(const char *name, uint32_t *size)
{
    uint32_t    len;
    uint8_t     *buf = NULL;

    FILE *fp = fopen(name, "rb");
    if (fp == NULL) {
        LOG("File not exist: %s", name);
        goto __fail;
    }

    if (fseek(fp, 0, SEEK_END) == 0) {
        len = (uint32_t)ftell(fp);
        fseek(fp, 0, SEEK_SET);
    } else {
        LOG("Get file length fail: %s", name);
        goto __fail;
    }

    if (len == 0) {
        LOG("File length is zero: %s", name);
        goto __fail;
    }

    buf = (uint8_t*)malloc(len);
    if (buf == NULL) {
        LOG("Alloc memory fail");
        goto __fail;
    }

    *size = fread(buf, 1, len, fp);
    if (*size == len) {
        fclose(fp);
        return buf;
    }
    LOG("Read file data fail: %s", name);

__fail:
    if (fp) {
        fclose(fp);
    }
    if (buf) {
        free(buf);
    }
    return NULL;
}


int32_t dlmodule_link_check(const char* modfile, const char* symfile)
{
    uint32_t mod_len, sym_len;
    int32_t  ret = -1;
    uint8_t *symbol_ptr = NULL;
    uint8_t *module_ptr = NULL;
    dlmodule_t *module = NULL;

    module_ptr = prvLoadFile(modfile, &mod_len);
    if (NULL == module_ptr)
        goto __fail;

    symbol_ptr = prvLoadFile(symfile, &sym_len);
    if (NULL == symbol_ptr)
        goto __fail;

    // build symbol table
    if (prvBuildSymtab((const char *)symbol_ptr, sym_len)) {
        LOG("Module: build symbol table fail\n");
        goto __fail;
    }

    /* check ELF header */
    if (memcmp(elf_module->e_ident, RTMMAG, SELFMAG) != 0 &&
        memcmp(elf_module->e_ident, ELFMAG, SELFMAG) != 0) {
        LOG("Module: magic error\n");
        goto __fail;
    }

    /* check ELF class */
    if (elf_module->e_ident[EI_CLASS] != ELFCLASS32) {
        LOG("Module: ELF class error\n");
        goto __fail;
    }

    module = (dlmodule_t*)malloc(sizeof(dlmodule_t));
    if (NULL == module)
        goto __fail;
    memset(module, 0, sizeof(dlmodule_t));

    if (elf_module->e_type == ET_DYN) {
        ret = dlmodule_load_shared_object(module, module_ptr);
    }
    else {
        LOG("Module: unsupported elf type: %d\n", elf_module->e_type);
    }

__fail:
    if (module) {
        /* destory module */
        if (module->mem_space)
            free(module->mem_space);
        /* delete module object */
        free(module);
    }
    if (g_symtabel)
        free(g_symtabel);
    if (symbol_ptr)
        free(symbol_ptr);
    if (module_ptr)
        free(module_ptr);
    return ret;
}

int32_t dlmodule_symbol_find(const char *sym_str)
{
    /* find in kernel symbol table */
    for (uint32_t index = 0; index < g_symcount; index ++) {
        if (strcmp(g_symtabel[index], sym_str) == 0)
            return index+1;
    }
    return 0;
}

