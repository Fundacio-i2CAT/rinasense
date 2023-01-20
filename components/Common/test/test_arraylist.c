#include <string.h>

#include "portability/port.h"

#include "common/arraylist.h"
#include "common/rsrc.h"
#include "common/error.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_arraylist) {}
RS_TEST_CASE_TEARDOWN(test_arraylist) {}

void AddIntegersAndCheck(arraylist_t *pxLst)
{
    int i1 = 1, i2 = 2, i3 = 3;

    TEST_ASSERT(unArrayListCount(pxLst) == 0);
    TEST_ASSERT(!ERR_CHK_MEM(xArrayListAdd(pxLst, &i1)));
    TEST_ASSERT(unArrayListCount(pxLst) == 1);
    TEST_ASSERT(!ERR_CHK_MEM(xArrayListAdd(pxLst, &i2)));
    TEST_ASSERT(unArrayListCount(pxLst) == 2);
    TEST_ASSERT(!ERR_CHK_MEM(xArrayListAdd(pxLst, &i3)));
    TEST_ASSERT(unArrayListCount(pxLst) == 3);

    TEST_ASSERT(*(int *)pvArrayListGet(pxLst, 0) == 1);
    TEST_ASSERT(*(int *)pvArrayListGet(pxLst, 1) == 2);
    TEST_ASSERT(*(int *)pvArrayListGet(pxLst, 2) == 3);

    TEST_ASSERT(!pvArrayListGet(pxLst, 4));
    TEST_ASSERT(!pvArrayListGet(pxLst, 100));
}

RS_TEST_CASE(ArrayListSimple, "[arraylist]")
{
    arraylist_t xLst;

    TEST_ASSERT(!ERR_CHK_MEM(xArrayListInit(&xLst, sizeof(int), 10, NULL)));
    AddIntegersAndCheck(&xLst);
}

RS_TEST_CASE(ArrayListResize, "[arraylist]")
{
    arraylist_t xLst;

    TEST_ASSERT(!ERR_CHK_MEM(xArrayListInit(&xLst, sizeof(int), 1, NULL)));
    AddIntegersAndCheck(&xLst);
}

RS_TEST_CASE(ArrayListResizePool, "[arraylist]")
{
    arraylist_t xLst;
    rsrcPoolP_t xPool;

    TEST_ASSERT((xPool = pxRsrcNewVarPool("ArrayListResizePool", 0)));
    TEST_ASSERT(!ERR_CHK_MEM(xArrayListInit(&xLst, sizeof(int), 1, xPool)));
    AddIntegersAndCheck(&xLst);
}

RS_TEST_CASE(ArrayListRemove, "[arraylist]")
{
    arraylist_t xLst;

    TEST_ASSERT(!ERR_CHK_MEM(xArrayListInit(&xLst, sizeof(int), 10, NULL)));
    AddIntegersAndCheck(&xLst);

    vArrayListRemove(&xLst, 1);

    TEST_ASSERT(*(int *)pvArrayListGet(&xLst, 0) == 1);
    TEST_ASSERT(*(int *)pvArrayListGet(&xLst, 1) == 3);

    vArrayListRemove(&xLst, 0);

    TEST_ASSERT(*(int *)pvArrayListGet(&xLst, 0) == 3);
    TEST_ASSERT(!(int *)pvArrayListGet(&xLst, 1));
    TEST_ASSERT(!(int *)pvArrayListGet(&xLst, 2));
    TEST_ASSERT(unArrayListCount(&xLst) == 1);
}

RS_TEST_CASE(ArrayListPointers, "[arraylist]")
{
    arraylist_t xLst;
    const string_t hw1 = "hello, world";
    const string_t hw2 = "world, hello";

    TEST_ASSERT(!ERR_CHK_MEM(xArrayListInit(&xLst, sizeof(char *), 10, NULL)));
    TEST_ASSERT(!ERR_CHK_MEM(xArrayListAdd(&xLst, (void *)&hw1)));
    TEST_ASSERT(!ERR_CHK_MEM(xArrayListAdd(&xLst, (void *)&hw2)));

    TEST_ASSERT(*(char **)pvArrayListGet(&xLst, 0) == hw1);
    TEST_ASSERT(*(char **)pvArrayListGet(&xLst, 1) == hw2);
    TEST_ASSERT(strcmp(*(char **)pvArrayListGet(&xLst, 0), hw1) == 0);
    TEST_ASSERT(strcmp(*(char **)pvArrayListGet(&xLst, 1), hw2) == 0);
}

#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(ArrayListSimple);
    RS_RUN_TEST(ArrayListResize);
    RS_RUN_TEST(ArrayListResizePool);
    RS_RUN_TEST(ArrayListRemove);
    RS_RUN_TEST(ArrayListPointers);
    RS_SUITE_END();
}

#endif
