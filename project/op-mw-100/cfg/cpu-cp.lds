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
 * @file     cp-cpu.ld
 * @brief    GNU Linker Script for CP CPU
 * @version  V1.0.0
 * @date     9. May 2023
 ******************************************************************************/
OUTPUT_ARCH( "riscv" )

ENTRY( __start )

MEMORY
{ 
    INTENAL_CODE   (wxa!ri) : ORIGIN = 0x00008000, LENGTH = 32K 	
    INTENAL_DATA   (wxa!ri) : ORIGIN = 0x00010000, LENGTH = 32K 		

    IRAM0_PSM_RE   (wxa!ri) : ORIGIN = 0xC0203800, LENGTH = 9K
    IRAM0_RE       (wxa!ri) : ORIGIN = 0xC0206400, LENGTH = 103K
    IRAM0          (wxa!ri) : ORIGIN = 0xC0220000, LENGTH = 64K

    XIP            (wxa!ri) : ORIGIN = CP_XIP_START, LENGTH = 300K
    DRAM           (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 352K
}


SECTIONS
{
  __stack_size  = DEFINED(__stack_size)  ? __stack_size   : 2K;
  __malloc_size = DEFINED(__malloc_size) ? __malloc_size  : 48K;

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

  .iram0_re : ALIGN(0x10)  
  {
    *(.iram.recode*)
    *(.iram.redata*)
  } >IRAM0_RE

  .iram0 : ALIGN(0x10)  
  {
    __iram_start = .;
    *(.iram.sharedata*)
    PROVIDE(__iram_code_start = .);
    *(.iram.code*)
    PROVIDE(__iram_rodata_start = .);
    *(.iram.rodata*)
    PROVIDE(__iram_srdata_start = .);
    *(.iram.srdata*)
    . = ALIGN(0x10);
    PROVIDE(__iram_srdata_end = .);
    *(.iram.rtdata*)
    __iram_end = .;
  } >IRAM0

#if CP_XIP_ON
  .xip   : ALIGN(0x10)
  {
    __flash_start = .;
    *(.xip.rodata*)
    __flash_end = .;
  } >XIP
#endif

  .text           : ALIGN(0x10)
  {
    PROVIDE(__text_start = .);
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
  } >DRAM 

  
  .rodata        :   ALIGN(0x10)
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
  }>DRAM AT>DRAM

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
    . = ALIGN(32);
  } >DRAM AT>DRAM

  . = ALIGN(4);
  PROVIDE( _edata = . );
  PROVIDE( edata = . );

  PROVIDE( _fbss = . ); 
  PROVIDE( __bss_start = . );

  .bss  :  ALIGN(32)
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

  .malloc   : ALIGN(0x20)
  {
    PROVIDE( __malloc_start = . );
    . = . + __malloc_size;
    PROVIDE( __malloc_end = . );
    . = ALIGN(4);
  } >DRAM AT>DRAM

}
