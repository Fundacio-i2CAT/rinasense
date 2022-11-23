#ifndef __FREERTOS__
/* For struct iovec */
#include <sys/uio.h>
#endif

#include <string.h>

#include "common/list.h"
#include "common/rsrc.h"
#include "common/netbuf.h"

void vNetBufFreeNormal(netbuf_t *pxNb)
{
    vRsMemFree(pxNb->pxBufStart);
    vRsrcFree(pxNb);
}

void vNetBufFreePool(netbuf_t *pxNb)
{
    vRsrcFree(pxNb->pxBufStart);
    vRsrcFree(pxNb);
}

void vNetBufFreeButDont(netbuf_t *pxNb)
{
    /* We don't free the buffer */
    vRsrcFree(pxNb);
}

rsrcPoolP_t xNetBufNewPool(const char *pcPoolName)
{
    return pxRsrcNewPool(pcPoolName, sizeof(netbuf_t), 5, 1, 0);
}

netbuf_t *pxNetBufNew(rsrcPoolP_t xPool,
                      eNetBufType_t eType,
                      buffer_t pxBuf,
                      size_t unSz,
                      freemethod_t pfnFree)
{
    netbuf_t *pxNewBuf;

    if (!(pxNewBuf = pxRsrcAlloc(xPool, "")))
        return NULL;

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

bool_t xNetBufSplit(netbuf_t *pxNb, eNetBufType_t eType, size_t unSz)
{
    netbuf_t *pxNewBuf;

    if (!(pxNewBuf = pxRsrcAlloc(pxNb->xPool, "")))
        return NULL;

    pxNewBuf->eType = eType;
    pxNewBuf->pxNext = NULL;
    pxNewBuf->pxFirst = pxNb->pxFirst;
    pxNewBuf->pxBuf = pxNb->pxBuf + unSz;
    pxNewBuf->unSz = pxNb->unSz - unSz;
    pxNewBuf->pxBufStart = pxNb->pxBufStart;
    pxNewBuf->xFreed = false;
    pxNewBuf->freemethod = pxNb->freemethod;
    pxNewBuf->xPool = pxNb->xPool;

    pxNb->unSz = unSz;
    pxNb->pxNext = pxNewBuf;

    return true;
}

bool_t xNetBufPop(netbuf_t *pxNb, size_t unSz)
{
    FOREACH_NETBUF(pxNb, pxNbIter) {
        /* If the desired pop size is bigger than the current netbuf,
           free the current netbuf in the chain. */
        if (pxNbIter->unSz <= unSz) {
            unSz -= pxNbIter->unSz;
            vNetBufFree(pxNbIter);
        }
        /* Otherwise just shit the size and pointer of the current
         * netbuf link */
        else {
            pxNbIter->unSz -= unSz;
            pxNbIter->pxBuf += unSz;
        }

        /* If there is some bytes left to pop, move onto the next
           netbuf, otherwise just get out of the loop */
        if (!unSz) break;
    }

    /* Success here means that we could push enough bytes in the
       chain. */
    return unSz == 0;
}

void vNetBufFreeAll(netbuf_t *pxNb)
{
    FOREACH_ALL_NETBUF(pxNb, pxNbIter) {
        if (pxNbIter->freemethod)
            pxNbIter->freemethod(pxNb);
    }
}

void vNetBufFree(netbuf_t *pxNb)
{
    int unCnt;
    netbuf_t *pxNbFirst, *pxNbIter;
    bool_t xFullyFreed;

    /* Mark the target netbuf as freed */
    pxNb->xFreed = true;
    pxNb->unSz = 0;
    pxNb->pxBuf = NULL;

    /* Get the first netbuf in the chain. */
    pxNbFirst = pxNb->pxFirst;
    pxNbIter = pxNbFirst;

    /* Scan through the netbuf list, checking if all the netbufs that
     * are pointing to the same buffer are freed. If we find that all
     * the buffers are freed, we will be able to free the underlying
     * buffer.  */
    xFullyFreed = true;
    FOREACH_ALL_NETBUF(pxNb, pxNbIter) {
        xFullyFreed &= pxNbIter->xFreed;
    }

    /* If all the parts of the netbuf are freed, we can actually free
       the buffer. */
    if (xFullyFreed)
        pxNb->freemethod(pxNbFirst);
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
    netbuf_t *pxNbIter;
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
    netbuf_t *pxNbIter;

    RsAssert(!pxNbLeft->xFreed);

    /* Loop until we find the next netbuf in the chain that wasn't
     * been freed */

    pxNbIter = pxNbLeft;
    while (pxNbIter->pxNext != NULL && !pxNbIter->xFreed)
        pxNbIter = pxNbLeft->pxNext;

    pxNbIter->pxNext = pxNbRight;
    pxNbRight->pxFirst = pxNbLeft->pxFirst;
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
