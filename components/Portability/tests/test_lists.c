#include <stdio.h>
#include "portability/port.h"

struct item {
    int n;
    RsListItem_t item;
};

struct item i1 = { 1 }, i2 = { 3 };

void addBogusItems(RsList_t *lst)
{
    vRsListInit(lst);
    vRsListInitItem(&(i1.item));
    vRsListInitItem(&(i2.item));
    vRsListSetListItemOwner(&(i1.item), &i1);
    vRsListSetListItemOwner(&(i2.item), &i2);
    vRsListInsert(lst, &i1.item);
    vRsListInsert(lst, &i2.item);
}

/* Test simple allocation removals. */
void testListBasics()
{
    RsList_t lst;
    RsListItem_t *pLstItem = NULL;
    struct item *pItem;

    addBogusItems(&lst);

    /* Basic stuff. */
    RsAssert(pRsListGetListItemOwner(&i1.item) == &i1);
    RsAssert(pRsListGetListItemOwner(&i2.item) == &i2);

    /* Check the length of the list. */
    RsAssert(unRsListCurrentListLength(&lst) == 2);

    /* Get the first item. */
    pLstItem = pRsListGetHeadEntry(&lst);
    RsAssert(pLstItem == &i1.item);

    /* Get the first owner structure. */
    pItem = pRsListGetOwnerOfHeadEntry(&lst);
    RsAssert(pItem->n == 1);

    /* Remove the first item */
    vRsListRemoveItem(&(i1.item));
    RsAssert(unRsListCurrentListLength(&lst) == 1);

    /* Check that the first item in the list is now the second item we
     * inserted. */
    pLstItem = pRsListGetHeadEntry(&lst);
    RsAssert(pLstItem == &i2.item);

    /* Remove the last item */
    vRsListRemoveItem(&(i2.item));
    RsAssert(unRsListCurrentListLength(&lst) == 0);
}

/* Test iterating through a list. */
void testListIteration()
{
    RsList_t lst;
    struct item *pos;
    RsListItem_t *pEnd, *pItem;
    int n = 0;

    /* Initialize the list with items. */
    addBogusItems(&lst);

    /* This is based on what is done in pidm.c */
    pEnd = pRsListGetEndMarker(&lst);
    pItem = pRsListGetHeadEntry(&lst);

    while (pItem != pEnd) {
        pos = pRsListGetListItemOwner(pItem);
        n += pos->n;
        pItem = pRsListGetNext(pItem);
    }

    RsAssert(n == 4);
}

int main() {
    testListBasics();
    testListIteration();
}
