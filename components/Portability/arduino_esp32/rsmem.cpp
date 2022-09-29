#include <stddef.h>
#include <malloc.h>
#include "portability/rsmem.h"

void * pvRsMemAlloc(size_t unSz)
{
    return malloc(unSz);
}

void vRsMemFree(void * p)
{
    free(p);
}
