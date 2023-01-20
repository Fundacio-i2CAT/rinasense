#include "freertos/FreeRTOS.h"

void * pvRsMemAlloc( size_t unSz )
{
    heap_caps_check_integrity_all(true);
    return pvPortMalloc( unSz );
}

void *pvRsMemCAlloc(size_t n, size_t sz)
{
    heap_caps_check_integrity_all(true);
    return calloc(n, sz);
}

void *pvRsMemRealloc(void *p, size_t sz)
{
    heap_caps_check_integrity_all(true);
    return realloc(p, sz);
}

void vRsMemFree( void * p )
{
    heap_caps_check_integrity_all(true);
    vPortFree( p );
}
