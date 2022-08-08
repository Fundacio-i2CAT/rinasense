#include <string.h>
#include <glib.h>

#include "glib_rslist.h"
#include "portability/rslist.h"
#include "macros.h"

void vRsListInit(RsList_t * const pList)
{
    pList->pxGList = g_list_alloc();
    pList->pxGList->data = NULL;
    pList->pxGList->next = NULL;
    pList->pxGList->prev = NULL;

    pList->nListLength = 0;
    pList->pIndex = NULL;
}

void vRsListInitItem(RsListItem_t * const pListItem)
{
    pListItem->pCont = NULL;
    pListItem->pOwner = NULL;
}

void vRsListInsert(RsList_t * const pList, RsListItem_t * const pListItem)
{
    GList *lastItem;

    lastItem = g_list_last(pList->pxGList);
    lastItem->next = g_list_alloc();
    lastItem->next->prev = lastItem;
    lastItem->next->next = NULL;
    lastItem->next->data = pListItem;

    pListItem->pCont = pList;
    pListItem->pxGList = lastItem->next;

    pList->nListLength++;
}

void vRsListInsertEnd(RsList_t * const pList, RsListItem_t * const pListItem)
{
}

void *pRsListGetOwnerOfHeadEntry(RsList_t * const pList)
{
    if (pList->pxGList->next != NULL)
        return ((RsListItem_t *)pList->pxGList->next->data)->pOwner;
    return NULL;
}

void *pRsListGetListItemOwner(RsListItem_t * const pListItem)
{
    return pListItem->pOwner;
}

void vRsListRemoveItem(RsListItem_t * const pListItem)
{
    RsList_t *pList;

    pList = pListItem->pCont;
    pList->pxGList = g_list_remove(pList->pxGList, pListItem);
    pList->nListLength--;
}

uint32_t unRsListCurrentListLength(RsList_t * const pList)
{
    return pList->nListLength;
}

bool_t vRsListIsContainedWithin(RsList_t * const pList, RsListItem_t * const pListItem)
{
}

void vRsListSetListItemOwner(RsListItem_t * const pListItem, void *pOwner)
{
    pListItem->pOwner = pOwner;
}

RsListItem_t* pRsListGetHeadEntry(RsList_t * const pList)
{
    if (pList->pxGList->next != NULL)
        return (RsListItem_t *)pList->pxGList->next->data;
    return NULL;
}

RsListItem_t* pRsListGetEndMarker(RsList_t * const pList)
{
    return NULL;
}

RsListItem_t* pRsListGetNext(RsListItem_t * const pListItem)
{
    if (pListItem->pxGList->next != NULL)
        return pListItem->pxGList->next->data;
    return NULL;
}

