/*
 * Copyright (c) 2019 Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/******************************************************************************
 * @file     ap-cpu.ld
 * @brief    GNU Linker Script for AP CPU
 * @version  V1.0.0
 * @date     9. May 2023
 ******************************************************************************/
OUTPUT_ARCH( "riscv" )

ENTRY( __start )

MEMORY
{
    INTENAL_CODE   (wxa!ri) : ORIGIN = 0x00008000, LENGTH = 16K
    INTENAL_DATA   (wxa!ri) : ORIGIN = 0x00010000, LENGTH = 16K

    IRAM0_PSM_RE   (wxa!ri) : ORIGIN = 0xC0200800, LENGTH = 10K
    IRAM2          (wxa!ri) : ORIGIN = 0xC026C800, LENGTH = 78K

    XIP            (wxa!ri) : ORIGIN = AP_XIP_START, LENGTH = 1024K
    DRAM           (wxa!ri) : ORIGIN = 0x800FE800, LENGTH = 2562K  
	DRAM_APP       (wxa!ri) : ORIGIN = 0x80380000, LENGTH = 512K
}

PROVIDE(__app_ram_start = ORIGIN(DRAM_APP));
PROVIDE(__app_ram_end = ORIGIN(DRAM_APP) + LENGTH(DRAM_APP));

SECTIONS
{
  __stack_size  = DEFINED(__stack_size)  ? __stack_size   : 2K;
  __malloc_min_size  = 512K;

  .itcm           :   ALIGN(0x10)
  {
    PROVIDE(__tcm_code_start = .);
    *(.vtable)
    KEEP (*(SORT_NONE(.init)))
    PROVIDE(__tcm_text_start = .);
    *(.itcm .itcm.*)
    PROVIDE( _ilm_lma = . );
    PROVIDE( _ilm = . );
    . = ALIGN(0x10);
    PROVIDE(__tcm_code_end = .);
  } >INTENAL_CODE

  .dtcm  : ALIGN(0x10)
  {
    PROVIDE(__tcm_data_start = .);
    . = ALIGN(0x10);
    PROVIDE(__tcm_rodata_start = .);
    *(.dtcm.rodata)
    . = ALIGN(0x10);
    PROVIDE(__tcm_srdata_start = .);
    *(.dtcm.srdata)
    . = ALIGN(0x10);
    PROVIDE( __stack_start = . );
    . = . + __stack_size;
    PROVIDE( __stack_end = . );
    . = ALIGN(0x10);
    PROVIDE(__tcm_srdata_end = .);
    *(.dtcm.rtdata)
    PROVIDE(__tcm_data_end = .);
  } >INTENAL_DATA

  .iram0_psm_re : ALIGN(0x10)
  {
    *(.iram.psm_recode*)
    *libdrv.a:drv_apm*.o (.text .text.* .rodata .rodata.*)
    *libgcc.a:save-restore.o (.text .text.* .rodata .rodata.*)
    *(.iram.psm_redata*)
  } >IRAM0_PSM_RE

  .iram2 : ALIGN(0x10)
  {
    __iram_start = .;
    *(.iram.sharedata*)
    PROVIDE(__iram_code_start = .);
    *(.iram.code*)
    *libbusmux.a:*.o (.text .text.*)
    drv_spi[!n]*.o (.text .text.*)
    mux_proxy_ip*.o (.text .text.*)
    drv_edcp*.o (.text .text.*)
    *(.sramtext.profile .sramtext.profile*)
    PROVIDE(__iram_rodata_start = .);
    *(.iram.rodata*)
    *libbusmux.a:*.o (.rodata .rodata.*)
    drv_spi[!n]*.o (.rodata .rodata.*)
    mux_proxy_ip*.o (.rodata .rodata.*)
    drv_edcp*.o (.rodata .rodata.*)
    . = ALIGN(0x10);
    PROVIDE(__iram_srdata_start = .);
    *(.iram.srdata*)
    . = ALIGN(0x10);
    PROVIDE(__iram_srdata_end = .);
    *(.iram.rtdata*)
    __iram_end = .;
  } >IRAM2

#if AP_XIP_ON
  .psram : ALIGN(0x10)
  {
    PROVIDE(__psram_code_start = .);
    /* text & rodata of libgcc.a */
    *libgcc.a:_c[lt]z*.o (.text .text.* .rodata .rodata.*)
    *libgcc.a:_*[vd]di3.o (.text .text.* .rodata .rodata.*)
    /* text & rodata of flash dirver & OS */
    drv_flash*.o (.text .text.* .rodata .rodata.*)
    flash_ids*.o (.text .text.* .rodata .rodata.*)
    xipsfc*.o (.text .text.* .rodata .rodata.*)
    spisfc*.o (.text .text.* .rodata .rodata.*)
    drv_dma*.o (.text .text.* .rodata .rodata.*)
    drv_uart*.o (.text .text.* .rodata .rodata.*)
    *libos.a:(.text .text.* .rodata .rodata.*)
    *libcpu.a:(.text .text.* .rodata .rodata.*)
#if PS_PARTITIAL
    *libps.a:zuc*.o (.text .text.* .rodata .rodata.*)
    *libps.a:snow3g*.o (.text .text.* .rodata .rodata.*)
    *libps.a:aes*.o (.text .text.* .rodata .rodata.*)
    *libps.a:hmac_sha*.o (.text .text.* .rodata .rodata.*)
    *libps.a:ps_com*.o (.text .text.* .rodata .rodata.*)
    *libps.a:ps_database*.o (.text .text.* .rodata .rodata.*)
    *libps.a:l2com_com*.o (.text .text.* .rodata .rodata.*)
    *libps.a:l2com_List*.o (.text .text.* .rodata .rodata.*)
    *libps.a:l2com_mem*.o (.text .text.* .rodata .rodata.*)
    *libps.a:l2com_stat*.o (.text .text.* .rodata .rodata.*)
    *libps.a:l2com_task*.o (.text .text.* .rodata .rodata.*)
    *libps.a:l2com_time*.o (.text .text.* .rodata .rodata.*)
    *libps.a:mac_*.o (.text .text.* .rodata .rodata.*)
    *libps.a:pdcp_*.o (.text .text.* .rodata .rodata.*)
    *libps.a:rlc_*.o (.text .text.* .rodata .rodata.*)
#else
    *libps.a:(.text .text.* .rodata .rodata.*)
#endif
    *libopencore-amr-0.1.6.a:*.o (.text .text.* .rodata .rodata.*)
    *libvo-amrwbenc-0.1.3.a:*.o (.text .text.* .rodata .rodata.*)
    *(.psram.code .psram.code.*)
    PROVIDE(__psram_code_end = .);
    PROVIDE(__psram_rodata_start = .);
    *(.psram.rodata .psram.rodata.*)
    PROVIDE(__psram_rodata_end = .);

    /* section information for psm dpm info */
    . = ALIGN(4);
     __psm_dpminfo_table_start = .;
     KEEP(*(SORT(.psm_dpminfo*)))
     __psm_dpminfo_table_end = .;

     /* section information for psm dpm info */
     KEEP(*(SORT(.psm_cmndpminfo*)))
     __psm_cmndpminfo_table_end = .;

    /* section information for psm sleepmode info */
    . = ALIGN(4);
     __psm_sleepmode_table_start = .;
     KEEP(*(SORT(.psm_sleepmode*)))
     __psm_sleepmode_table_end = .;

    /* section information for drv sby recover init */
    . = ALIGN(4);
     __psm_sbyfunc_start = .;
     KEEP(*(.psm_sbydrvinit_func))
     __psm_sbyfunc_end = .;
  } >DRAM

  .xip_text : ALIGN(0x10)
  {
    PROVIDE(__text_start = .);
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(.text .text.*)
	KEEP(*(SORT(.app_code*)))
    *(.gnu.linkonce.t.*)
  } >XIP

  .xip_rodata : ALIGN(0x10)
  {
    . = ALIGN(4);
    *(.rdata)
    *(.rodata .rodata.*)
   /* section information for finsh shell */
    . = ALIGN(4);
    __fsymtab_start = .;
    KEEP(*(FSymTab))
    __fsymtab_end = .;

    . = ALIGN(4);
    __vsymtab_start = .;
    KEEP(*(VSymTab))
    __vsymtab_end = .;
    *(.gnu.linkonce.r.*)

    /* section information for initial. */
     . = ALIGN(4);
    __rt_init_start = .;
    KEEP(*(SORT(.init_call*)))
    __rt_init_end = .;

     . = ALIGN(4);
    _nr_cmd_start_ = .;
    KEEP(*(SORT(.nr_shell_cmd*)))
    _nr_cmd_end_   = .;

    . = ALIGN(4);
    __rt_shutdown_start = .;
    KEEP(*(SORT(.shutdown_call*)))
    __rt_shutdown_end = .;

    /* section information for OneOS drivers */
    . = ALIGN(4);
    __driver_table_start = .;
    KEEP(*(driver_table))
    __driver_table_end = .;


    /* section information for OneOS devices */
    . = ALIGN(4);
    __device_table_start = .;
    KEEP(*(device_table))
    __device_table_end = .;

    /* section information for modules */
    . = ALIGN(4);
     __rtmsymtab_start = .;
     KEEP(*(RTMSymTab))
     __rtmsymtab_end = .;

    /* section information for ATTEST */
    . = ALIGN(4);
     __atest_tc_table_start = .;
     KEEP(*(AtestTcTab))
     __atest_tc_table_end = .;

    /* section information for AT */
    . = ALIGN(4);
     __at_cmd_table_start = .;
     KEEP(*(.at_cmd_table_user))
     KEEP(*(.at_cmd_table_modem))
     KEEP(*(.at_cmd_table_middleware))
     KEEP(*(.at_cmd_table_driver))
     __at_cmd_table_end = .;
  }>XIP AT>XIP

  .xip_dlmo : ALIGN(0x10)
  {
    /* section information for module function name */
     KEEP(*(RTMSymName))
  }>XIP


  .text : ALIGN(0x10)
  {
  } >DRAM AT>DRAM

  .rodata : ALIGN(0x10)
  {
  } >DRAM AT>DRAM
#else
  .psram : ALIGN(0x10)
  {
    PROVIDE(__psram_code_start = .);
    *(.psram.code .psram.code.*)
    PROVIDE(__psram_code_end = .);
    PROVIDE(__psram_rodata_start = .);
    *(.psram.rodata .psram.rodata.*)
    PROVIDE(__psram_rodata_end = .);

    /* section information for psm dpm info */
    . = ALIGN(4);
     __psm_dpminfo_table_start = .;
     KEEP(*(SORT(.psm_dpminfo*)))
     __psm_dpminfo_table_end = .;

     /* section information for psm dpm info */
     KEEP(*(SORT(.psm_cmndpminfo*)))
     __psm_cmndpminfo_table_end = .;

    /* section information for psm sleepmode info */
    . = ALIGN(4);
     __psm_sleepmode_table_start = .;
     KEEP(*(SORT(.psm_sleepmode*)))
     __psm_sleepmode_table_end = .;
    /* section information for drv sby recover init */
    . = ALIGN(4);
     __psm_sbyfunc_start = .;
     KEEP(*(.psm_sbydrvinit_func))
     __psm_sbyfunc_end = .;
  } >DRAM

  .xip_text : ALIGN(0x10)
  {
  } >XIP

  .xip_rodata : ALIGN(0x10)
  {
  }>XIP AT>XIP

  .text : ALIGN(0x10)
  {
    PROVIDE(__text_start = .);
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
  } >DRAM AT>DRAM

  .rodata : ALIGN(0x10)
  {
    . = ALIGN(4);
    *(.rdata)
    *(.rodata .rodata.*)
   /* section information for finsh shell */
    . = ALIGN(4);
    __fsymtab_start = .;
    KEEP(*(FSymTab))
    __fsymtab_end = .;

    . = ALIGN(4);
    __vsymtab_start = .;
    KEEP(*(VSymTab))
    __vsymtab_end = .;
    *(.gnu.linkonce.r.*)

    /* section information for initial. */
     . = ALIGN(4);
    __rt_init_start = .;
    KEEP(*(SORT(.init_call*)))
    __rt_init_end = .;

     . = ALIGN(4);
    _nr_cmd_start_ = .;
    KEEP(*(SORT(.nr_shell_cmd*)))
    _nr_cmd_end_   = .;

    . = ALIGN(4);
    __rt_shutdown_start = .;
    KEEP(*(SORT(.shutdown_call*)))
    __rt_shutdown_end = .;

    /* section information for OneOS drivers */
    . = ALIGN(4);
    __driver_table_start = .;
    KEEP(*(driver_table))
    __driver_table_end = .;


    /* section information for OneOS devices */
    . = ALIGN(4);
    __device_table_start = .;
    KEEP(*(device_table))
    __device_table_end = .;

    /* section information for modules */
    . = ALIGN(4);
     __rtmsymtab_start = .;
     KEEP(*(RTMSymTab))
     __rtmsymtab_end = .;

    /* section information for ATTEST */
    . = ALIGN(4);
     __atest_tc_table_start = .;
     KEEP(*(AtestTcTab))
     __atest_tc_table_end = .;

    /* section information for AT */
    . = ALIGN(4);
     __at_cmd_table_start = .;
     KEEP(*(.at_cmd_table_user))
     KEEP(*(.at_cmd_table_modem))
     KEEP(*(.at_cmd_table_middleware))
     KEEP(*(.at_cmd_table_driver))
     __at_cmd_table_end = .;
  } >DRAM AT>DRAM

  .dlmo : ALIGN(0x10)
  {
    /* section information for module function name */
     KEEP(*(RTMSymName))
  }>DRAM

#endif
  . = ALIGN(4);

  PROVIDE (__text_end = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);
  PROVIDE( _eilm = . );

  .data  : ALIGN(32)
  {
    PROVIDE( _data_lma = . );
    PROVIDE( _data = . );
    *(.data .data.*)
    *(.gnu.linkonce.d.*)
    . = ALIGN(8);
    PROVIDE( __global_pointer$ = . + 0x800);
    *(.sdata .sdata.* .sdata*)
    *(.gnu.linkonce.s.*)
    . = ALIGN(8);
    *(.srodata.cst16)
    *(.srodata.cst8)
    *(.srodata.cst4)
    *(.srodata.cst2)
    *(.srodata .srodata.*)
  } >DRAM AT>DRAM

  . = ALIGN(4);
  PROVIDE( _edata = . );
  PROVIDE( edata = . );

  PROVIDE( _fbss = . );
  PROVIDE( __bss_start = . );

  .bss (NOLOAD) :  ALIGN(32)
  {
    PROVIDE( __bss_start = . );
    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss.*)
    *(.psram.debugbss)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(32);
    PROVIDE( __excp_stack_start = . );
    . = . + __stack_size;
    PROVIDE( __excp_stack_end = . );
  } >DRAM AT>DRAM

  . = ALIGN(8);
  PROVIDE( _end = . );
  PROVIDE( end = . );
  
  .malloc   : ALIGN(32)
  {
    PROVIDE( __malloc_size = (__app_ram_start - 4096 - end));
    PROVIDE( __malloc_start = . );
    . = . + __malloc_size;
    PROVIDE( __malloc_end = . );
    ASSERT(( __malloc_end >= (__malloc_start    + __malloc_min_size)), "ERROR: MALLOC IS NOT ENOUGH! ");
	ASSERT(( __malloc_end < __app_ram_start ), "ERROR: MALLOC COVER APP DRAM! ");
    . = ALIGN(4);
  } >DRAM AT>DRAM

}
