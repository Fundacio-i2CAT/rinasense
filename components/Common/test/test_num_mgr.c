#include "num_mgr.h"

void test8bits()
{
    NumMgr_t *nm;

    // Allocate all 3 available numbers.
    RsAssert((nm = pxNumMgrCreate(3)) != NULL);
    RsAssert(ulNumMgrAllocate(nm) == 1);
    RsAssert(ulNumMgrAllocate(nm) == 2);
    RsAssert(ulNumMgrAllocate(nm) == 3);
    RsAssert(ulNumMgrAllocate(nm) == UINT_MAX);

    // Free the middle one
    RsAssert(xNumMgrRelease(nm, 2));

    // Make sure we get the one we just free if we allocate again.
    RsAssert(ulNumMgrAllocate(nm) == 2);
    RsAssert(ulNumMgrAllocate(nm) == UINT_MAX);

    vNumMgrDestroy(nm);
}

void testManyBits()
{
    NumMgr_t nm;
    uint32_t i = 0;

    RsAssert(xNumMgrInit(&nm, USHRT_MAX - 1));

    for (i = 1; i <= USHRT_MAX - 1; i++)
        RsAssert(ulNumMgrAllocate(&nm) == i);

    RsAssert((i = ulNumMgrAllocate(&nm)) == UINT_MAX);

    vNumMgrFini(&nm);
}

int main()
{
    test8bits();
    testManyBits();
}
