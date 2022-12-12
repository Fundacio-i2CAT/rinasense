#ifndef _COMMON_ARRAYLIST_H_INCLUDED
#define _COMMON_ARRAYLIST_H_INCLUDED

#include <stdint.h>

#include "linux_rsmem.h"
#include "portability/port.h"
#include "common/rsrc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *pvArray;

    size_t unMaxItemCount;

    size_t unItemSz;

    size_t unFreeIdx;

    rsrcPoolP_t xPool;

} arraylist_t;

bool_t xArrayListInit(arraylist_t *pxLst,
                      size_t unItemSz,
                      size_t unInitialMaxItemCount,
                      rsrcPoolP_t xVarPool);

bool_t xArrayListAdd(arraylist_t *pxLst, void *pvItem);

void vArrayListRemove(arraylist_t *pxLst, size_t unIdx);

static inline void vArrayListFree(arraylist_t *pxLst)
{
    if (pxLst->xPool)
        vRsrcFree(pxLst->pvArray);
    else
        vRsMemFree(pxLst->pvArray);
}

static inline const void *pvArrayListGet(arraylist_t *pxLst, size_t unIdx)
{
    if (unIdx > pxLst->unFreeIdx - 1)
        return NULL;

    return (void *)(pxLst->pvArray + (unIdx * pxLst->unItemSz));
}

static inline size_t unArrayListCount(arraylist_t *pxLst)
{
    return pxLst->unFreeIdx;
}

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_ARRAYLIST_H_INCLUDED */
