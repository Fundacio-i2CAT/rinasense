#include <stdio.h>
#include "portability/port.h"
#include "common/list.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_lists) {}
RS_TEST_CASE_TEARDOWN(test_lists) {}

struct item {
    int n;
    RsListItem_t item;
};

struct item i1, i2, i3;

/* This ain't a test! */
void addBogusItems(RsList_t *lst)
{
    i1.n = 1;
    i2.n = 3;
    i3.n = 100;

    vRsListInit(lst);
    vRsListInitItem(&(i1.item), &i1);
    vRsListInitItem(&(i2.item), &i2);
    vRsListInsert(lst, &i1.item);
    vRsListInsert(lst, &i2.item);
}

/* Test simple allocation removals. */
RS_TEST_CASE(ListBasics, "[list]")
{
    RsList_t lst;
    RsListItem_t *pLstItem = NULL;

    RS_TEST_CASE_BEGIN(test_lists);

    addBogusItems(&lst);

    /* Basic stuff. */
    TEST_ASSERT(pxRsListGetItemOwner(&i1.item) == &i1);
    TEST_ASSERT(pxRsListGetItemOwner(&i2.item) == &i2);

    /* Check the length of the list. */
    TEST_ASSERT(unRsListLength(&lst) == 2);

    /* Get the first item. */
    pLstItem = pxRsListGetFirst(&lst);
    TEST_ASSERT(pLstItem != NULL);

    /* Remove the item */
    vRsListRemove(&(i1.item));
    TEST_ASSERT(unRsListLength(&lst) == 1);

    /* Check that the first item in the list is now the second item we
     * inserted. */
    pLstItem = pxRsListGetFirst(&lst);
    TEST_ASSERT(pLstItem != NULL);

    /* Remove the last item */
    vRsListRemove(&(i2.item));
    TEST_ASSERT(unRsListLength(&lst) == 0);

    RS_TEST_CASE_END(test_lists);
}

/* Test that adding at the end of the list works as expected. */
RS_TEST_CASE(ListInsertEnd, "[list]")
{
    RsList_t lst;
    RsListItem_t *pLstItem = NULL;

    RS_TEST_CASE_BEGIN(test_lists);

    vRsListInit(&lst);
    vRsListInitItem(&(i1.item), &i1);
    vRsListInitItem(&(i2.item), &i2);
    vRsListInsertEnd(&lst, &i1.item);
    vRsListInsertEnd(&lst, &i2.item);

    pLstItem = pxRsListGetFirst(&lst);
    TEST_ASSERT(pLstItem == &i1.item);

    pLstItem = pxRsListGetLast(&lst);
    TEST_ASSERT(pLstItem == &i2.item);

    vRsListRemove(&(i1.item));
    TEST_ASSERT(unRsListLength(&lst) == 1);
    TEST_ASSERT(pxRsListGetFirst(&lst) == pxRsListGetLast(&lst));

    vRsListRemove(&(i2.item));
    TEST_ASSERT(unRsListLength(&lst) == 0);
    pLstItem = pxRsListGetFirst(&lst);
    TEST_ASSERT(pxRsListGetFirst(&lst) == NULL);
    TEST_ASSERT(pxRsListGetLast(&lst) == NULL);

    RS_TEST_CASE_END(test_lists);
}

/* Test iterating through a list. */
RS_TEST_CASE(ListIteration, "[list]")
{
    RsList_t lst;
    struct item *pos;
    RsListItem_t *pItem;
    int n = 0;

    RS_TEST_CASE_BEGIN(test_lists);

    /* Initialize the list with items. */
    addBogusItems(&lst);

    /* This is based on what is done in pidm.c */
    pItem = pxRsListGetFirst(&lst);

    while (pItem != NULL) {
        pos = pxRsListGetItemOwner(pItem);
        n += pos->n;
        pItem = pxRsListGetNext(pItem);
    }

    TEST_ASSERT(n == 4);

    RS_TEST_CASE_END(test_lists);
}

RS_TEST_CASE(ListGetHeadOwner, "[list]")
{
    RsList_t lst;

    RS_TEST_CASE_BEGIN(test_lists);

    addBogusItems(&lst);

    TEST_ASSERT(pxRsListGetHeadOwner(&lst));
    TEST_ASSERT(pxRsListGetHeadOwner(&lst) == &i1);
    vRsListRemove(&i1.item);
    TEST_ASSERT(pxRsListGetHeadOwner(&lst) == &i2);

    RS_TEST_CASE_END(test_lists);
}

RS_TEST_CASE(ListIsContainedWithin, "[list]")
{
    RsList_t lst;

    RS_TEST_CASE_BEGIN(test_lists);

    addBogusItems(&lst);

    TEST_ASSERT(xRsListIsContainedWithin(&lst, &i1.item));
    TEST_ASSERT(xRsListIsContainedWithin(&lst, &i2.item));
    TEST_ASSERT(!xRsListIsContainedWithin(&lst, &i3.item));

    RS_TEST_CASE_END(test_lists);
}

#ifndef TEST_CASE
int main() {
    UNITY_BEGIN();
    RS_RUN_TEST(ListBasics);
    RS_RUN_TEST(ListInsertEnd);
    RS_RUN_TEST(ListIteration);
    RS_RUN_TEST(ListGetHeadOwner);
    RS_RUN_TEST(ListIsContainedWithin);
    return UNITY_END();
}
#endif
