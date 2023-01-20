#ifndef _COMMON_DATAPACKER_H_INCLUDED
#define _COMMON_DATAPACKER_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "portability/port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *pvBase;

    void *pvNext;

    size_t unMaxSz;

} Pk_t;

void vPkInit(Pk_t *pxPk, void *pvBase, size_t unOffset, size_t unMaxSz);

void vPkWrite(Pk_t *pxPk, void *pvData, size_t unSz);

static inline void *pxPkPtr(Pk_t *pxPk)
{
    return pxPk->pvNext;
}

static inline size_t pxPkGetFree(Pk_t *pxPk)
{
    return (size_t)((void *)(pxPk->pvBase + pxPk->unMaxSz) - pxPk->pvNext);
}

static inline void vPkWriteStr(Pk_t *pxPk, string_t pcData)
{
    size_t unSz = strlen(pcData);
    vPkWrite(pxPk, pcData, unSz);
    *((char *)pxPk->pvNext++) = '\0';
}

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_DATAPACKER_H_INCLUDED */
