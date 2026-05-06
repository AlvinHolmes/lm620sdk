#pragma once

#define MG_ARCH MG_ARCH_CUSTOM

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>


#ifndef MG_IO_SIZE
#define MG_IO_SIZE 1460
#endif

#ifndef MG_ENABLE_LWIP
#define MG_ENABLE_LWIP  1
#endif
