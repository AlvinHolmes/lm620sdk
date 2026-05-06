/*asdfa
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
 * @file     app.ld
 * @brief    GNU Linker Script for AP CPU
 * @version  V1.0.0
 * @date     9. May 2023
 ******************************************************************************/
OUTPUT_ARCH( "riscv" )

ENTRY( app_main )

MEMORY
{
	DRAM_APP       (wxa!ri) : ORIGIN = 0x80380000, LENGTH = 512K
}

PROVIDE(__app_ram_start = ORIGIN(DRAM_APP));
PROVIDE(__app_ram_end = ORIGIN(DRAM_APP) + LENGTH(DRAM_APP));

SECTIONS
{ 
  .text : ALIGN(0x10)
  {
	. = ALIGN(4);
    	
	*(.text.app_main .text.app_main.*)
    *(.text.unlikely .text.unlikely.*)    
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
  } >DRAM_APP AT>DRAM_APP
  
  .rodata : ALIGN(0x10)
  {
    . = ALIGN(4);
    *(.rdata)
    *(.rodata .rodata.*)
    *(.gnu.linkonce.r.*)
  } >DRAM_APP AT>DRAM_APP
  
  .data  : ALIGN(0x10)
  {
	. = ALIGN(4);
    *(.data .data.*)
    *(.gnu.linkonce.d.*)
    . = ALIGN(8);
    *(.sdata .sdata.* .sdata*)
    *(.gnu.linkonce.s.*)
    . = ALIGN(8);
    *(.srodata.cst16)
    *(.srodata.cst8)
    *(.srodata.cst4)
    *(.srodata.cst2)
    *(.srodata .srodata.*)
  } >DRAM_APP AT>DRAM_APP
  
  .bss (NOLOAD) :  ALIGN(0x10)
  {
	. = ALIGN(4);
    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss.*)
    *(.psram.debugbss)
    *(.gnu.linkonce.b.*)
    *(COMMON)
	
     _end = .;   
	ASSERT(( _end <= __app_ram_end), "ERROR: APP RAM IS NOT ENOUGH! ");
  } >DRAM_APP AT>DRAM_APP
}
