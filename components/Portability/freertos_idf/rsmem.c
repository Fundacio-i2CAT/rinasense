#include "freertos/FreeRTOS.h"

void * pvRsMemAlloc( size_t unSz )
{
    return pvPortMalloc( unSz );
}

void vRsMemFree( void * p )
{
    vPortFree( p );
}
