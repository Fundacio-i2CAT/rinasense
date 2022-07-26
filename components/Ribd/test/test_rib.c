#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "Ribd.h"

#include "unity.h"
#include "unity_fixups.h"

RS_TEST_CASE_SETUP(test_rib) {}
RS_TEST_CASE_TEARDOWN(test_rib) {}

RS_TEST_CASE(RibCreateObject, "[rib]")
{
    RS_TEST_CASE_BEGIN(test_rib);
    TEST_ASSERT(pxRibCreateObject("testobj", 0, "Test Object", "testobjclass", 0) != NULL);
    RS_TEST_CASE_END(test_rib);
}

RS_TEST_CASE(RibFindObject, "[rib]")
{
    RS_TEST_CASE_BEGIN(test_rib);
    TEST_ASSERT(pxRibCreateObject("testobj", 0, "Test Object", "testobjclass", 0) != NULL);
    TEST_ASSERT(pxRibFindObject("testobj") != NULL);
    TEST_ASSERT(pxRibFindObject("blarg") == NULL);
    RS_TEST_CASE_END(test_rib);
}

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();
    RS_RUN_TEST(RibCreateObject);
    RS_RUN_TEST(RibFindObject);
    exit(UNITY_END());
}
#endif
