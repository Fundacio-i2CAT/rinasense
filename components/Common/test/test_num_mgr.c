#include "num_mgr.h"

#include "unity.h"
#include "unity_fixups.h"

#ifndef TEST_CASE
void setUp() {}
void tearDown() {}
#endif

RS_TEST_CASE(Bits8, "8 bits number allocator")
{
    NumMgr_t *nm;

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
}

RS_TEST_CASE(ManyBits, "Test bigger number allocator")
{
    NumMgr_t nm;
    uint32_t i = 0;

    TEST_ASSERT(xNumMgrInit(&nm, USHRT_MAX - 1));

    for (i = 1; i <= USHRT_MAX - 1; i++)
        TEST_ASSERT(ulNumMgrAllocate(&nm) == i);

    TEST_ASSERT((i = ulNumMgrAllocate(&nm)) == UINT_MAX);

    vNumMgrFini(&nm);
}

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_Bits8);
    RUN_TEST(test_ManyBits);
    return UNITY_END();
}
#endif
