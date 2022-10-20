#ifndef _PORTABILITY_RS_MEM_H
#define _PORTABILITY_RS_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

void *pvRsMemCAlloc(size_t unNb, size_t unSz);

void *pvRsMemAlloc(size_t unSz);

void vRsMemFree(void *px);

#ifdef __cplusplus
}
#endif

#endif // _PORTABILITY_RS_MEM_H
