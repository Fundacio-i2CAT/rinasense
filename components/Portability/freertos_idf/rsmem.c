#include "freertos/FreeRTOS.h"

void * pvRsMemAlloc( size_t unSz )
{
    return pvPortMalloc( unSz );
}

void *pvRsMemCAlloc(size_t n, size_t sz)
{
    return calloc(n, sz);
}

void vRsMemFree( void * p )
{
    vPortFree( p );
}
