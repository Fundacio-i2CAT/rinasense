#include <stdio.h>
#include "portability/port.h"

#include "unity.h"
#include "unity_fixups.h"

RS_TEST_CASE_SETUP(test_lists) {}
RS_TEST_CASE_TEARDOWN(test_lists) {}

struct item {
    int n;
    RsListItem_t item;
};

struct item i1 = { 1 }, i2 = { 3 };

/* This ain't a test! */
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
RS_TEST_CASE(ListBasics, "[list]")
{
    RsList_t lst;
    RsListItem_t *pLstItem = NULL;
    struct item *pItem;

    RS_TEST_CASE_BEGIN(test_lists);

    addBogusItems(&lst);

    /* Basic stuff. */
    TEST_ASSERT(pRsListGetListItemOwner(&i1.item) == &i1);
    TEST_ASSERT(pRsListGetListItemOwner(&i2.item) == &i2);

    /* Check the length of the list. */
    TEST_ASSERT(unRsListCurrentListLength(&lst) == 2);

    /* Get the first item. */
    pLstItem = pRsListGetHeadEntry(&lst);
    TEST_ASSERT(pLstItem == &i1.item);

    /* Get the first owner structure. */
    pItem = pRsListGetOwnerOfHeadEntry(&lst);
    TEST_ASSERT(pItem->n == 1);

    /* Remove the first item */
    vRsListRemoveItem(&(i1.item));
    TEST_ASSERT(unRsListCurrentListLength(&lst) == 1);

    /* Check that the first item in the list is now the second item we
     * inserted. */
    pLstItem = pRsListGetHeadEntry(&lst);
    TEST_ASSERT(pLstItem == &i2.item);

    /* Remove the last item */
    vRsListRemoveItem(&(i2.item));
    TEST_ASSERT(unRsListCurrentListLength(&lst) == 0);

    RS_TEST_CASE_END(test_lists);
}

/* Test iterating through a list. */
RS_TEST_CASE(ListIteration, "[list]")
{
    RsList_t lst;
    struct item *pos;
    RsListItem_t *pEnd, *pItem;
    int n = 0;

    RS_TEST_CASE_BEGIN(test_lists);

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

    TEST_ASSERT(n == 4);

    RS_TEST_CASE_END(test_lists);
}

#ifndef TEST_CASE
int main() {
    UNITY_BEGIN();
    RS_RUN_TEST(ListBasics);
    RS_RUN_TEST(ListIteration);
    return UNITY_END();
}
#endif
