#include <stdint.h>
#include <string.h>

#include "portability/port.h"
#include "common/rsrc.h"
#include "common/arraylist.h"

bool_t xArrayListInit(arraylist_t *pxLst,
                      size_t unItemSz,
                      size_t unInitialMaxItemCount,
                      rsrcPoolP_t xVarPool)
{
    size_t unTotalInitSz;

    unTotalInitSz = unItemSz * unInitialMaxItemCount;

    if (xVarPool) {
        if (!(pxLst->pvArray = pxRsrcVarAlloc(xVarPool, "Array List", unTotalInitSz)))
            return false;
    } else
        if (!(pxLst->pvArray = pvRsMemAlloc(unTotalInitSz)))
            return false;

    pxLst->xPool = xVarPool;
    pxLst->unMaxItemCount = unInitialMaxItemCount;
    pxLst->unItemSz = unItemSz;
    pxLst->unFreeIdx = 0;

    return true;
}

bool_t xArrayListAdd(arraylist_t *pxLst, void *pvItem)
{
    /* Resize if needed */
    if (pxLst->unFreeIdx == pxLst->unMaxItemCount) {
        size_t unNewMaxItemCount, unNewSz;

        /* We double in size at each size. Not very sophisticated by
         * might work good enough most of the time. */
        unNewMaxItemCount = pxLst->unMaxItemCount * 2;
        unNewSz = unNewMaxItemCount * pxLst->unItemSz;

        /* Memory allocated from pool is not resizable */
        if (pxLst->xPool) {
            void *pvNewBuf;

            if (!(pvNewBuf = pxRsrcVarAlloc(pxLst->xPool, "Array List", unNewSz)))
                return false;

            memcpy(pvNewBuf, pxLst->pvArray, pxLst->unMaxItemCount * pxLst->unItemSz);

            vRsrcFree(pxLst->pvArray);
            pxLst->pvArray = pvNewBuf;
            pxLst->unMaxItemCount = unNewMaxItemCount;
        }
        /* Heap memory is resizable with realloc. */
        else {
            if (!(pxLst->pvArray = pvRsMemRealloc(pxLst->pvArray, unNewSz)))
                return false;

            pxLst->unMaxItemCount = unNewMaxItemCount;
        }
    }

    memcpy(pxLst->pvArray + (pxLst->unFreeIdx++ * pxLst->unItemSz), pvItem, pxLst->unItemSz);
    return true;
}

void vArrayListRemove(arraylist_t *pxLst, size_t unIdx)
{
    /* No-op if we're past the last item */
    if (unIdx > pxLst->unFreeIdx - 1)
        return;

    /* If it's the last item, nothing to move, just cancel out the
     * last item we added. */
    if (unIdx == pxLst->unFreeIdx - 1) {
        pxLst->unFreeIdx--;
        return;
    }
    else {
        void *pxMMDst, *pxMMSrc;
        size_t unMMSz;

        pxMMSrc = pxLst->pvArray + ((unIdx + 1) * pxLst->unItemSz);
        pxMMDst = pxLst->pvArray + (unIdx * pxLst->unItemSz);
        unMMSz = (pxMMSrc + (pxLst->unFreeIdx * pxLst->unItemSz)) - pxMMSrc;

        memmove(pxMMDst, pxMMSrc, unMMSz);
        pxLst->unFreeIdx--;
    }
}


