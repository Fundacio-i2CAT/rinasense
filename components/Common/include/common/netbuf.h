#ifndef _COMMON_MBUF_H_INCLUDED
#define _COMMON_MBUF_H_INCLUDED

#include <stdint.h>
#include <stddef.h>

#include "common/rsrc.h"
#include "common/list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xNETBUF_T;

typedef enum {
    NB_UNKNOWN,

    /* Ethernet headers */
    NB_ETH_HDR,

    /* Ethernet content */
    NB_ETH_DATA,

    /* RINA ARP packet */
    NB_RINA_ARP,

    /* RINA packet header */
    NB_RINA_PCI,

    /* RINA packet content */
    NB_RINA_DATA

} eNetBufType_t;

typedef void (*freemethod_t)(struct xNETBUF_T *px);

typedef struct xNETBUF_T {
    RsListItem_t xListItem;

    struct xNETBUF_T *pxNext;
    struct xNETBUF_T *pxFirst;

    eNetBufType_t eType;

    /* Size of the local part of the netbuf. */
    size_t unSz;

    /* Pointer to the buffer */
    buffer_t pxBuf;

    /* Pointer to the start of the underlying buffer. */
    buffer_t pxBufStart;

    /* This is true if the netbuf is no longer in use. */
    bool_t xFreed;

	freemethod_t freemethod;

    rsrcPoolP_t xPool;
} netbuf_t;

void vNetBufFree(netbuf_t *pxNb);

void vNetBufFreeAll(netbuf_t *pxNb);

/* Create a memory pool with the correct parameters to create
 * netbufs structures */
rsrcPoolP_t xNetBufNewPool(const char *pcPoolName);

/* Create a netbuf including a single mbuf object including the whole
 * buffer passed as parameter. */
netbuf_t *pxNetBufNew(rsrcPoolP_t xPool, eNetBufType_t eType, buffer_t pxBuf, size_t unSz, freemethod_t pfnFree);

rsErr_t xNetBufSplit(netbuf_t *pxNb, eNetBufType_t eType, size_t unSz);

#if 0
/* Strip 'unSz' bytes from the start of a netbuf chain or a single
 * netbuf. */
netbuf_t *xNetBufPop(netbuf_t *pxNb, size_t unSz);
#endif

void vNetBufLink(netbuf_t *pxNb, ...);

size_t unNetBufTotalSize(netbuf_t *pxNb);

size_t unNetBufCount(netbuf_t *pxNb);

void vNetBufAppend(netbuf_t *pxNb, netbuf_t *pxNew);

static inline eNetBufType_t eNetBufType(netbuf_t *pxNb)
{
    RsAssert(!pxNb->xFreed);
    return pxNb->eType;
}

static inline size_t unNetBufSize(netbuf_t *pxNb)
{
    RsAssert(!pxNb->xFreed);
    return pxNb->unSz;
}

static inline void *pvNetBufPtr(netbuf_t *pxNb)
{
    RsAssert(!pxNb->xFreed);
    return pxNb->pxBuf;
}

netbuf_t *pxNetBufNext(netbuf_t *pxNb);

#define NETBUF_FREE_DONT    &vNetBufFreeButDont
#define NETBUF_FREE_POOL    &vNetBufFreePool
#define NETBUF_FREE_NORMAL  &vNetBufFreeNormal

void vNetBufFreeNormal(netbuf_t *pxNb);

void vNetBufFreePool(netbuf_t *pxNb);

void vNetBufFreeButDont(netbuf_t *pxNb);

#define FOREACH_NETBUF(_nb, _iterNb)                                           \
    for (netbuf_t *_iterNb = _nb->pxFirst; _iterNb; _iterNb = _iterNb->pxNext) \
        if (!_iterNb->xFreed)

#define FOREACH_ALL_NETBUF(_nb, _iterNb)                                         \
    for (netbuf_t *_iterNb = _nb->pxFirst; _iterNb; _iterNb = _iterNb->pxNext)

#define FOREACH_ALL_NETBUF_SAFE(_nb, _iterNb)   \
    for (netbuf_t *_iterNb = _nb->pxFirst, *_nnext = _nb->pxFirst->pxNext; _iterNb; _iterNb = _nnext, _nnext = _iterNb ? _iterNb->pxNext : NULL)

#define FOREACH_NETBUF_FROM(_nb, _iterNb)                                 \
    for (netbuf_t *_iterNb = _nb; _iterNb; _iterNb = _iterNb->pxNext) \
        if (!_iterNb->xFreed)

size_t unNetBufRead(netbuf_t *pxNb, void *pvBuffer, size_t unRdOff, size_t unSzBuf);

void vNetBufPrint(const string_t pcNbName, netbuf_t *pxNb);

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_MBUF_H_INCLUDED */
