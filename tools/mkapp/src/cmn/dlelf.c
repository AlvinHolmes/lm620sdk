/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author      Notes
 * 2018/08/29     Bernard     first version
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dlmodule.h"
#include "dlelf.h"

#define LOG(fmt, ...)       printf("mkldmo: " fmt "\r\n", ##__VA_ARGS__)

typedef int bool_t;      /**< boolean type */

/* boolean type definitions */
#define OS_TRUE                         1               /**< boolean true  */
#define OS_FALSE                        0               /**< boolean fails */

int32_t dlmodule_load_shared_object(dlmodule_t* module, void *module_ptr)
{
    bool_t linked   = OS_FALSE;
    uint32_t index, module_size = 0;
    Elf32_Addr vstart_addr, vend_addr;
    bool_t has_vstart;

    if (memcmp(elf_module->e_ident, RTMMAG, SELFMAG) == 0)
    {
        /* rtmlinker finished */
        linked = OS_TRUE;
    }

    /* get the ELF image size */
    has_vstart = OS_FALSE;
    vstart_addr = vend_addr = 0;
    for (index = 0; index < elf_module->e_phnum; index++)
    {
        if (phdr[index].p_type != PT_LOAD)
            continue;

        if (phdr[index].p_memsz < phdr[index].p_filesz)
        {
            LOG("invalid elf: segment %d: p_memsz: %d, p_filesz: %d\n",
                       index, phdr[index].p_memsz, phdr[index].p_filesz);
            return -1;
        }
        if (!has_vstart)
        {
            vstart_addr = phdr[index].p_vaddr;
            vend_addr = phdr[index].p_vaddr + phdr[index].p_memsz;
            has_vstart = OS_TRUE;
            if (vend_addr < vstart_addr)
            {
                LOG("invalid elf: segment %d: p_vaddr: %d, p_memsz: %d\n",
                           index, phdr[index].p_vaddr, phdr[index].p_memsz);
                return -1;
            }
        }
        else
        {
            if (phdr[index].p_vaddr < vend_addr)
            {
                LOG("invalid elf: segment should be sorted and not overlapped\n");
                return -1;
            }
            if (phdr[index].p_vaddr > vend_addr + 16)
            {
                /* There should not be too much padding in the object files. */
                //LOG("warning: too much padding before segment %d", index);
            }

            vend_addr = phdr[index].p_vaddr + phdr[index].p_memsz;
            if (vend_addr < phdr[index].p_vaddr)
            {
                LOG("invalid elf: "
                           "segment %d address overflow\n", index);
                return -1;
            }
        }
    }

    module_size = vend_addr - vstart_addr;
    //LOG("module size: %d, vstart_addr: 0x%p", module_size, vstart_addr);
    if (module_size == 0)
    {
        LOG("Module size error: %d, vstart_addr: 0x%p\n", module_size, vstart_addr);
        return -1;
    }

    module->vstart_addr = vstart_addr;

    /* allocate module space */
    module->mem_space = malloc(module_size);
    if (module->mem_space == NULL)
    {
        LOG("Module: allocate space failed.\n");
        return -1;
    }
    module->mem_size = module_size;

    /* zero all space */
    memset(module->mem_space, 0, module_size);
    for (index = 0; index < elf_module->e_phnum; index++)
    {
        if (phdr[index].p_type == PT_LOAD)
        {
            memcpy(module->mem_space + phdr[index].p_vaddr - vstart_addr,
                      (uint8_t *)elf_module + phdr[index].p_offset,
                      phdr[index].p_filesz);
        }
    }


    /* handle relocation section */
    for (index = 0; index < elf_module->e_shnum; index ++)
    {
        uint32_t i, nr_reloc;
        Elf32_Sym *symtab;
        uint8_t *strtab;
        Elf32_Rel *rel = NULL;
        Elf32_Rela *rela = NULL;
        bool_t unsolved = OS_FALSE;

        if (!IS_REL(shdr[index]) && !IS_RELA(shdr[index]))
            continue;

        /* get relocate item */
        if (IS_REL(shdr[index]))
        {
            rel = (Elf32_Rel *)((uint8_t *)module_ptr + shdr[index].sh_offset);
            nr_reloc = (uint32_t)(shdr[index].sh_size / sizeof(Elf32_Rel));
        }
        else
        {
            rela = (Elf32_Rela *)((uint8_t *)module_ptr + shdr[index].sh_offset);
            nr_reloc = (uint32_t)(shdr[index].sh_size / sizeof(Elf32_Rela));
        }

        /* locate .rel.plt and .rel.dyn section */
        symtab = (Elf32_Sym *)((uint8_t *)module_ptr +
                               shdr[shdr[index].sh_link].sh_offset);
        strtab = (uint8_t *)module_ptr +
                 shdr[shdr[shdr[index].sh_link].sh_link].sh_offset;

        /* relocate every items */
        for (i = 0; i < nr_reloc; i ++)
        {
            Elf32_Sym *sym;
            //Elf32_Sword addend = 0;

            if (rela != NULL) {
                rel = (Elf32_Rel *)rela;
                //addend = rela->r_addend;
            }

            sym = &symtab[ELF32_R_SYM(rel->r_info)];

            if ((sym->st_shndx != SHT_NULL) ||(ELF_ST_BIND(sym->st_info) == STB_LOCAL))
            {
                //Elf32_Addr addr;

                //addr = (Elf32_Addr)(module->mem_space + sym->st_value - vstart_addr);
                //dlmodule_relocate(module, rel, addr, addend);
            }
            else if (!linked)
            {
                Elf32_Addr addr;

                /* need to resolve symbol in kernel symbol table */
                addr = dlmodule_symbol_find((const char *)(strtab + sym->st_name));
                if (addr == 0)
                {
                    printf("\r\n");
                    LOG("Module: can't find < %s > in kernel symbol table\r\n", strtab + sym->st_name);
                    unsolved = OS_TRUE;
                }
                else
                {
                    //dlmodule_relocate(module, rel, addr, addend);
                }
            }
            if (rela != NULL)
            {
                rela ++;
            }
            else
            {
                rel ++;
            }
        }

        if (unsolved)
            return -1;
    }

    return 0;
}


