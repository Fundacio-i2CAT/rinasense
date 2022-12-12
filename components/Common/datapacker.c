#include <string.h>

#include "portability/port.h"
#include "common/datapacker.h"

void vPkInit(Pk_t *pxPk, void *pvBase, size_t unOffset, size_t unMaxSz)
{
    pxPk->pvBase = pvBase;
    pxPk->pvNext = pvBase + unOffset;
    pxPk->unMaxSz = unMaxSz;
}

void vPkWrite(Pk_t *pxPk, void *pvData, size_t unSz)
{
#ifndef NDEBUG
    if (pxPk->pvNext + unSz > pxPk->pvBase + pxPk->unMaxSz) {
        size_t sz = (pxPk->pvNext + unSz) - (pxPk->pvBase + pxPk->unMaxSz);
        LOGE("[DataPacker]", "Overflow: %zu bytes past maximum size", sz);
        abort();
    }
#endif
    memcpy(pxPk->pvNext, pvData, unSz);
    pxPk->pvNext += unSz;
}


