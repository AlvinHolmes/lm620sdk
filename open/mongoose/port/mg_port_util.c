#include <os.h>
#include "util.h"

#if MG_ENABLE_CUSTOM_MILLIS
uint64_t mg_millis(void)
{
    return osTickGet();
}

#endif

#if MG_ENABLE_CUSTOM_RANDOM
OS_WEAK void mg_random(void *buf, size_t len)
{
    (void) buf;
    (void) len;
}


#endif

