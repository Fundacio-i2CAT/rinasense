#ifndef _PORTABILITY_RS_LIST_H_
#define _PORTABILITY_RS_LIST_H

#include <stdint.h>
#include "port.h"

void vRsListInit(RsList_t *const);

void vRsListInitItem(RsListItem_t *const);

void vRsListInsert(RsList_t *const, RsListItem_t *const);

void vRsListInsertEnd(RsList_t *const, RsListItem_t *const);

void *pRsListGetOwnerOfHeadEntry(RsList_t *const);

void vRsListRemoveItem(RsListItem_t *const);

uint32_t unRsListCurrentListLength(RsList_t *const);

bool vRsListIsContainedWithin(RsList_t *const, RsListItem_t *const);

void vRsListSetListItemOwner(RsListItem_t *const, void *);

void *pRsListGetListItemOwner(RsListItem_t *const);

RsListItem_t *pRsListGetHeadEntry(RsList_t *const);

RsListItem_t *pRsListGetEndMarker(RsList_t *const);

RsListItem_t *pRsListGetNext(RsListItem_t *const);

bool RsListIsInitilised(RsList_t *const pList);

bool RsListIsEmpty(RsList_t *const pList);

#endif // _PORTABILITY_RS_LIST_H
