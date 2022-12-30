
#include <string.h>

#include "portability/port.h"

#include "common/error.h"
#include "common/list.h"
#include "common/rinasense_errors.h"
#include "common/rsrc.h"
#include "common/netbuf.h"

/* Set this if you want netbuf_t structs to be allocated on a pool. */
#define CONFIG_NETBUF_USES_POOLS

void vNetBufFreeNormal(netbuf_t *pxNb)
{
    if (pxNb->pxBufStart) {
        vRsMemFree(pxNb->pxBufStart);
        pxNb->pxBufStart = NULL;
    }
}

void vNetBufFreePool(netbuf_t *pxNb)
{
    if (pxNb->pxBufStart) {
        vRsrcFree(pxNb->pxBufStart);
        pxNb->pxBufStart = NULL;
    }
}

void vNetBufFreeButDont(netbuf_t *pxNb)
{
    /* We don't free the buffer */
}

rsrcPoolP_t xNetBufNewPool(const char *pcPoolName)
{
    return pxRsrcNewPool(pcPoolName, sizeof(netbuf_t), 0, 1, 0);
}

netbuf_t *pxNetBufNew(rsrcPoolP_t xPool,
                      eNetBufType_t eType,
                      buffer_t pxBuf,
                      size_t unSz,
                      freemethod_t pfnFree)
{
    netbuf_t *pxNewBuf;

    RsAssert(xPool);
    RsAssert(unSz);

#ifdef CONFIG_NETBUF_USES_POOLS
    if (!(pxNewBuf = pxRsrcAlloc(xPool, "pxNetBufNew")))
        return ERR_SET_OOM_NULL;
#else
    if (!(pxNewBuf = pvRsMemAlloc(sizeof(netbuf_t))))
        return ERR_SET_OOM_NULL;
#endif

    pxNewBuf->eType = eType;
    pxNewBuf->pxNext = NULL;
    pxNewBuf->pxFirst = pxNewBuf;
    pxNewBuf->pxBuf = pxBuf;
    pxNewBuf->unSz = unSz;
    pxNewBuf->pxBufStart = pxBuf;
    pxNewBuf->xFreed = false;
    pxNewBuf->freemethod = pfnFree;
    pxNewBuf->xPool = xPool;

    return pxNewBuf;
}

/* This ensures that the netbuf chain has a cut at 'unSz' bytes. If
 * it doesn't, it will be created. */
rsErr_t xNetBufSplit(netbuf_t *pxNb, eNetBufType_t eType, size_t unSz)
{
    netbuf_t *pxNewBuf;

    FOREACH_NETBUF(pxNb, pxNbIter) {
        /* If the current netbuf is shorter than what is required,
         * skip it. */
        if (pxNbIter->unSz < unSz) {
            unSz -= pxNbIter->unSz;
            continue;
        }
        /* There is already a cut! */
        else if (pxNbIter->unSz == unSz) {
            return SUCCESS;
        }
        else {
#ifdef CONFIG_NETBUF_USES_POOLS
            if (!(pxNewBuf = pxRsrcAlloc(pxNb->xPool, "xNetBufSplit")))
                return ERR_SET_OOM;
#else
            if (!(pxNewBuf = pvRsMemAlloc(sizeof(netbuf_t))))
                return ERR_SET_OOM;
#endif

            pxNewBuf->eType = eType;
            pxNewBuf->pxNext = pxNbIter->pxNext;
            pxNewBuf->pxFirst = pxNbIter->pxFirst;
            pxNewBuf->pxBuf = pxNbIter->pxBuf + unSz;
            pxNewBuf->unSz = pxNbIter->unSz - unSz;
            pxNewBuf->pxBufStart = pxNbIter->pxBufStart;
            pxNewBuf->xFreed = false;
            pxNewBuf->freemethod = pxNbIter->freemethod;
            pxNewBuf->xPool = pxNbIter->xPool;

            pxNbIter->unSz = unSz;
            pxNbIter->pxNext = pxNewBuf;

            return SUCCESS;
        }
    }

    return ERR_SET(ERR_NETBUF_SPLIT_FAIL);
}

void vNetBufFreeAll(netbuf_t *pxNb)
{
    FOREACH_ALL_NETBUF_SAFE(pxNb, pxNbIter) {
        vNetBufFree(pxNbIter);
    }
}

void vNetBufFree(netbuf_t *pxNb)
{
    int unCnt = 0;
    bool_t xFullyFreed;

    /* Mark the target netbuf as freed and not to be used as part of
     * the total buffer. */
    pxNb->xFreed = true;
    pxNb->unSz = 0;
    pxNb->pxBuf = NULL;

    /* Counter the number of netbufs pointing to the same underlying
     * buffer. If it's at least 2, clear the pointer here, if it's 0,
     * we can immediately free the underlying buffer. */
    xFullyFreed = true;
    FOREACH_ALL_NETBUF(pxNb, pxNbIter) {
        if (pxNb->pxBufStart == pxNbIter->pxBufStart)
            unCnt++;
        xFullyFreed &= pxNbIter->xFreed;
    }
    if (unCnt >= 2)
        pxNb->pxBufStart = NULL;
    else if (unCnt == 0)
        if (pxNb->freemethod)
            pxNb->freemethod(pxNb);

    if (xFullyFreed) {
        FOREACH_ALL_NETBUF_SAFE(pxNb, pxNbIter) {
            if (pxNbIter->freemethod)
                pxNbIter->freemethod(pxNbIter);

#ifdef CONFIG_NETBUF_USES_POOLS
            vRsrcFree(pxNbIter);
#else
            vRsMemFree(pxNbIter);
#endif
        }
    }
}

void vNetBufLink(netbuf_t *pxNbFirst, ...)
{
    va_list ap;
    netbuf_t *pxNb, *pxNbCur;

    pxNbCur = pxNbFirst;
    va_start(ap, pxNbFirst);
    while ((pxNb = va_arg(ap, netbuf_t *))) {
        vNetBufAppend(pxNbCur, pxNb);
        pxNbCur = pxNb;
    }
    va_end(ap);
}

size_t unNetBufTotalSize(netbuf_t *pxNb)
{
    size_t unSz = 0;

    FOREACH_NETBUF(pxNb, pxNbIter) {
        unSz += pxNbIter->unSz;
    }

    return unSz;
}

size_t unNetBufCount(netbuf_t *pxNb)
{
    size_t unCnt = 0;

    FOREACH_NETBUF(pxNb, pxNbIter) {
        unCnt++;
    }

    return unCnt;
}

/* Read from a netbuf chain, starting from the first.
 *
 * Please thread delicately when modifying this.
 */
size_t unNetBufRead(netbuf_t *pxNb, void *pvBuffer, size_t unRdOff, size_t unSzBuf)
{
    size_t unWrIdx = 0;    /* Index in the write buffer */
    size_t unSzDstBufLeft; /* Amount of space left in the target buffer */

    /* There is the whole buffer to read */
    unSzDstBufLeft = unSzBuf;

    FOREACH_NETBUF(pxNb, pxNbIter) {
        void *pStart; /* Ptr the start for this netbuf */
        size_t szCpy; /* Size to copy from the netbuf */

        /* Offset - size of the netbuf. */
        size_t szNbBufLeft;

        /* If the current netbuf size isn't past the offset, update
           the offset and move onto the next netbuf in the chain. */
        if (pxNbIter->unSz <= unRdOff) {
            unRdOff -= pxNbIter->unSz;
            continue;
        }

        /* Otherwise, we start copying. */

        /* This is how much we can read in the current netbuf
         * starting from the desired offset */
        szNbBufLeft = pxNbIter->unSz - unRdOff;

        /* Position from which we start reading the netbuf */
        pStart = (void *)pxNbIter->pxBuf + unRdOff;

        /* Determine how much we will read from the buffer */

        /* If what we can read in the netbuf is smaller than how much
           space there is left in the target, then read the whole netbuf
           content. */
        if (szNbBufLeft <= unSzDstBufLeft)
            szCpy = szNbBufLeft;
        else
            /* Otherwise, read just how much space is left in the
               target buffer */
            szCpy = unSzDstBufLeft;

        /* Copy! */
        memcpy(pvBuffer + unWrIdx, pStart, szCpy);

        /* Substract how much bytes we have read from the space
           available in the target buffer. */
        unSzDstBufLeft -= szCpy;

        /* Update the write index in the target buffer. */
        unWrIdx += szCpy;

        /* The offset is dealt with! */
        unRdOff = 0;

        /* If there is no space left in the target buffer, return how
           much bytes we have read. */
        if (!unSzDstBufLeft)
            return unWrIdx;
    }

    return unWrIdx;
}

void vNetBufAppend(netbuf_t *pxNbLeft, netbuf_t *pxNbRight)
{
    netbuf_t *pxNbLeftLast = NULL, *pxNbPrev;

    RsAssert(!pxNbLeft->xFreed);

    /* Find the last buffer in the pxNbLeft chain */
    FOREACH_ALL_NETBUF(pxNbLeft, pxNbIter) {
        if (!pxNbIter->pxNext) {
            pxNbLeftLast = pxNbIter;
            break;
        }
    }

    /* Append the first item of the pxNbRight chain to the last item
     * of the pxNbLeft chain. */
    pxNbLeftLast->pxNext = pxNbRight->pxFirst;

    /* Scan through the netbufs attached to pxNbRight and set them to
     * point at the start of the pxNbLeft chain. */
    FOREACH_NETBUF_FROM(pxNbRight, pxNbIter) {
        pxNbIter->pxFirst = pxNbLeft->pxFirst;
    }
}

netbuf_t *pxNetBufNext(netbuf_t *pxNb)
{
    RsAssert(!pxNb->xFreed);

    /* Scans the buffer chain starting from 'pxNb' until we find one
     * that we've not freed. Most often it's going to be the immediate
     * next one. */

    FOREACH_NETBUF_FROM(pxNb->pxNext, pxNbIter) {
        return pxNbIter;
    }

    /* Should not happen */
    return NULL;
}

void vNetBufPrint(const string_t pcNbName, netbuf_t *pxNb)
{
    int i = 1;
    const string_t pcTag = "[netbuf]";

    LOG_LOCK();

    LOGD(pcTag, "---- netbuf: %s ----", pcNbName);

    FOREACH_ALL_NETBUF_SAFE(pxNb, pxNbIter) {
        LOG_LOCK();
        LOGD(pcTag, "** Idx: %d", i++);
        LOGD(pcTag, "Ptr: %p", pxNbIter);
        LOGD(pcTag, "Buf: %p", pxNbIter->pxBufStart);
        LOGD(pcTag, "Siz: %zu", pxNbIter->unSz);
        LOGD(pcTag, "Freed: %d, Type: %d", pxNbIter->xFreed, pxNbIter->eType);
    }
}
