#ifndef _PORT_LINUX_GLIB_RSLIST_H
#define _PORT_LINUX_GLIB_RSLIST_H

#include <glib.h>

struct xRSLIST_T;

typedef struct xRSLISTITEMT {
    GList *pxGList;

    /* Pointer to the start of the containing list. */
    struct xRSLIST_T *pCont;

    /* Owner structure of the item */
    void *pOwner;

} RsListItem_t;

typedef struct xRSLIST_T {
    GList *pxGList;

    /* Size of the list */
    size_t nListLength;

    /* For iterating in the list. */
    RsListItem_t *pIndex;

} RsList_t;

#endif // _PORT_LINUX_GLIB_RSLIST_H
