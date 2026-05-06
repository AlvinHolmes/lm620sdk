/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Software GmbH
Licensed to:              Nuclei System Technology Co., Ltd., Room 101, No. 500 Bibo Road, Pilot Free Trade Zone, Shanghai, P. R. China
Licensed SEGGER software: emRun RISC-V
License number:           RTL-00126
License model:            License and Service Agreement, signed August 27, 2021
Licensed platform:        RISC-V based Processor Units designed, manufactured marketed and branded by LICENSEE based on the RV32 architecture
----------------------------------------------------------------------
Support and Update Agreement (SUA)
SUA period:               2021-09-09 - 2022-09-09
Contact to extend SUA:    sales@segger.com
-------------------------- END-OF-HEADER -----------------------------

Purpose: Sample configuration for real-time O(1) memory allocator.

*/

#ifndef __SEGGER_RTL_RTHEAP_CONF_H
#define __SEGGER_RTL_RTHEAP_CONF_H

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_Int.h"

//
// Restrict heap size to max. 8MB
//
#define __SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS   23

//
// Configuration of inlining.
//
#define __SEGGER_MEM_RTHEAP_INLINE              __SEGGER_RTL_INLINE

//
// Debugging level:
//   0: No checks, no traps.
//   1: Check incoming API parameters for validity.
//   2: As (1) but include internal checks for correct operation.
//   3: As (2) but include memory initialization.
//
#define __SEGGER_RTL_RTHEAP_DEBUG_LEVEL         0

//
// Detection of address size.
//
#define __SEGGER_RTL_RTHEAP_ADDRSIZE            __SEGGER_RTL_TYPESET

//
// Configuration of panic.
//
#define __SEGGER_RTL_RTHEAP_PANIC               abort

//
// Configuration of bit search.
//
#if defined(__GCC__) || defined(__clang__)   // Need to do this as MSVC chokes on __has_builtin() after the && even though __has_builtin is not defined, for some reason

#if defined(__has_builtin) && __has_builtin(__builtin_ffs)
  #define __SEGGER_RTL_RTHEAP_FFS(X)            (__builtin_ffs(X) - 1)
#endif

#if defined(__has_builtin) && __has_builtin(__builtin_clz)
  #define __SEGGER_RTL_RTHEAP_FLS(X)            (31 - __builtin_clz(X))
#endif

#endif

#endif

/*************************** End of file ****************************/
