#include "common/datapacker.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_datapacker) {}
RS_TEST_CASE_TEARDOWN(test_datapacker) {}

RS_TEST_CASE(DataPacker, "[datapacker]")
{
    char buf[10];
    Pk_t pk;

    vPkInit(&pk, &buf, 0, sizeof(buf));
    vPkWriteStr(&pk, "a");
    vPkWriteStr(&pk, "b");
    vPkWriteStr(&pk, "c");
    vPkWriteStr(&pk, "d");
    TEST_ASSERT(pxPkGetFree(&pk) == 2);
    vPkWriteStr(&pk, "e");
    TEST_ASSERT(pxPkGetFree(&pk) == 0);
}

#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(DataPacker);
    RS_SUITE_END();
}
#endif
