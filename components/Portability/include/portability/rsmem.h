#ifndef _PORTABILITY_RS_MEM_H
#define _PORTABILITY_RS_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

void *pvRsMemAlloc( size_t );

void vRsMemFree( void * );

#ifdef __cplusplus
}
#endif

#endif // _PORTABILITY_RS_MEM_H
