#include <glib.h>

void * pvRsMemAlloc( size_t unSz )
{
    return g_malloc( unSz );
}

void vRsMemFree( void * p )
{
    return g_free( p );
}
