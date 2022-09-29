#ifndef _COMMON_RS_LIST_H
#define _COMMON_RS_LIST_H

#include <stdint.h>

#include "portability/port.h"
#include "private/iot_doubly_linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xRSLIST_T;

typedef struct xRSLISTITEMT {
    Link_t xLink;

    /* Pointer to the containing list. */
    struct xRSLIST_T *pvCont;

    /* Pointer to the containing struct. */
    void *pvOwner;

} RsListItem_t;

typedef struct xRSLIST_T {
    RsListItem_t xHead;

    /* Size of the list */
    size_t nLength;

} RsList_t;

#define _first(plist) \
    listCONTAINER((*plist).xHead.xLink.pxNext, RsListItem_t, xLink)

#define _last(plist) \
    listCONTAINER((*plist).xHead.xLink.pxPrev, RsListItem_t, xLink)

/* Public interface starts here */

/* void vRsListInit(RsList_t * const); */
#define vRsListInit(plist) \
    vRsListInitItem(&((*plist).xHead), NULL);   \
    (*plist).nLength = 0;

/* void vRsListInitItem(RsListItem_t * const); */
#define vRsListInitItem(pitem, powner)          \
    listINIT_HEAD(&((*pitem).xLink));           \
    vRsListSetItemOwner(pitem, powner)

/* void vRsListInsert(RsList_t * const, RsListItem_t * const); */
#define vRsListInsert(plist, pitem)                        \
    listADD(&(_first(plist)->xLink), &((*pitem).xLink));   \
    (*plist).nLength++;                                    \
    (*pitem).pvCont = plist

/* void vRsListInsertEnd(RsList_t *const pxList, RsListItem_t *const* pxItem) */
#define vRsListInsertEnd(plist, pitem) \
    listADD(&(_last(plist)->xLink), &((*pitem).xLink));     \
    (*plist).nLength++;                                     \
    (*pitem).pvCont = plist

/* void vRsListRemoveItem(RsListItem_t * const); */
#define vRsListRemove(pitem)                    \
    (*pitem).pvCont->nLength--;                 \
    listREMOVE(&((*pitem).xLink))

/* uint32_t unRsListLength(RsList_t * const) */
#define unRsListLength(plist) \
    (*plist).nLength

/* void vRsListSetItemOwner(RsListItem_t * const, void *); */
#define vRsListSetItemOwner(pitem, powner) \
    (*pitem).pvOwner = powner

/* void *pxRsListGetItemOwner(RsListItem_t * const); */
#define pxRsListGetItemOwner(pitem) \
    (*pitem).pvOwner

/* RsListItem_t* pRsListGetHeadEntry(RsList_t * const); */
#define pxRsListGetFirst(plist) \
    ((*plist).nLength == 0 ? NULL : _first(plist))

/* RsListItem_t* pRsListGetLast(RsList_t * const); */
#define pxRsListGetLast(plist) \
    ((*plist).nLength == 0 ? NULL : _last(plist))

/* RsListItem_t* pRsListGetNext(RsListItem_t * const); */
#define pxRsListGetNext(pitem) \
    listCONTAINER((*pitem).xLink.pxNext, RsListItem_t, xLink)->pvOwner != NULL ? \
        (RsListItem_t *)(*pitem).xLink.pxNext : \
        NULL

#define pxRsListGetHeadOwner(plist) \
    (*plist).xHead.pvOwner

bool_t xRsListIsContainedWithin(RsList_t * const, RsListItem_t * const);

#ifdef __cplusplus
}
#endif

#endif // _COMMON_RS_LIST_H

