#include <stdint.h>

#include "portability/port.h"

#include "num_mgr.h"
#include "bit_array.h"

NumMgr_t *pxNumMgrCreate(size_t numCnt)
{
    NumMgr_t *nm;

    /* UINT_MAX is our error code. */
    if (numCnt == UINT_MAX)
        return NULL;

    nm = pvRsMemAlloc(sizeof(NumMgr_t));
    if (!nm)
        return NULL;

    memset(nm, 0, sizeof(NumMgr_t));

    if (!xNumMgrInit(nm, numCnt)) {
        vRsMemFree(nm);
        return NULL;
    }

    return nm;
}

bool_t xNumMgrInit(NumMgr_t *im, size_t numCnt)
{
    im->numCnt = numCnt + 1;
    im->lastAllocated = 0;

    im->ba = pxBitArrayAlloc(im->numCnt);
    if (!im->ba)
        return false;

    return true;
}

void vNumMgrFini(NumMgr_t *im)
{
    RsAssert(im != NULL);
    RsAssert(im->ba != NULL);

    vBitArrayFree(im->ba);
}

void vNumMgrDestroy(NumMgr_t *im)
{
    RsAssert(im != NULL);

    vRsMemFree(im);
}

uint32_t ulNumMgrAllocate(NumMgr_t *im)
{
    uint32_t p;

    RsAssert(im != NULL);

    p = im->lastAllocated + 1;

    /* Loops from the next to last allocated port, until we reach the last
     * allocated port again, wrapping over MAX_PORT_ID. */
    for (;; p++) {

        /* We return UINT_MAX if we overflow. */
        if (p == im->lastAllocated)
            return UINT_MAX;

        if (p == im->numCnt)
            p = 1;

        if (!xBitArrayGetBit(im->ba, p)) {
            vBitArraySetBit(im->ba, p);
            im->lastAllocated = p;
            break;
        }
    }

    return p;
}

bool_t xNumMgrRelease(NumMgr_t *im, uint32_t n)
{
    RsAssert(im != NULL);

    if (!xBitArrayGetBit(im->ba, n))
        return false;
    else {
        vBitArrayClearBit(im->ba, n);
        return true;
    }
}

bool_t xNumMgrIsAllocated(NumMgr_t *im, uint32_t n)
{
    RsAssert(im != NULL);

    return xBitArrayGetBit(im->ba, n);
}
