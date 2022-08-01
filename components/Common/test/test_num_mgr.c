#include "num_mgr.h"

#include "unity.h"
#include "unity_fixups.h"

RS_TEST_CASE_SETUP(test_num_mgr) {}
RS_TEST_CASE_TEARDOWN(test_num_mgr) {}

RS_TEST_CASE(Bits8, "[num_mgr]")
{
    NumMgr_t *nm;

    RS_TEST_CASE_BEGIN(test_num_mgr);

    // Allocate all 3 available numbers.
    TEST_ASSERT((nm = pxNumMgrCreate(3)) != NULL);
    TEST_ASSERT(ulNumMgrAllocate(nm) == 1);
    TEST_ASSERT(ulNumMgrAllocate(nm) == 2);
    TEST_ASSERT(ulNumMgrAllocate(nm) == 3);
    TEST_ASSERT(ulNumMgrAllocate(nm) == UINT_MAX);

    // Free the middle one
    TEST_ASSERT(xNumMgrRelease(nm, 2));

    // Make sure we get the one we just free if we allocate again.
    TEST_ASSERT(ulNumMgrAllocate(nm) == 2);
    TEST_ASSERT(ulNumMgrAllocate(nm) == UINT_MAX);

    vNumMgrDestroy(nm);

    RS_TEST_CASE_END(test_num_mgr);
}

RS_TEST_CASE(ManyBits, "[num_mgr]")
{
    NumMgr_t nm;
    uint32_t i = 0;

    RS_TEST_CASE_BEGIN(test_num_mgr);

    TEST_ASSERT(xNumMgrInit(&nm, USHRT_MAX - 1));

    for (i = 1; i <= USHRT_MAX - 1; i++)
        TEST_ASSERT(ulNumMgrAllocate(&nm) == i);

    TEST_ASSERT((i = ulNumMgrAllocate(&nm)) == UINT_MAX);

    vNumMgrFini(&nm);

    RS_TEST_CASE_END(test_num_mgr);
}

#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(Bits8);
    RS_RUN_TEST(ManyBits);
    RS_SUITE_END();
}
#endif
