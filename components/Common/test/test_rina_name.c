#include <stdio.h>
#include <string.h>

#include "common/rina_name.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_rina_name) {}
RS_TEST_CASE_TEARDOWN(test_rina_name) {}

/* Tests plain string duplication. */
RS_TEST_CASE(StringDup, "[rina_name]")
{
    const string_t s1 = "hello";
    string_t s2, s3;

    RS_TEST_CASE_BEGIN(test_rina_name);

    TEST_ASSERT(xRstringDup(s1, &s2));
    TEST_ASSERT(strcmp(s1, s2) == 0);

    TEST_ASSERT(xRstringDup(s1, &s3));
    TEST_ASSERT(strcmp(s1, s3) == 0);
    TEST_ASSERT(s1 != s3);

    vRsMemFree(s2);
    vRsMemFree(s3);

    RS_TEST_CASE_END(test_rina_name);
}

/* Test that RinaNameFromString breaks down a name in all its
 * component. */
RS_TEST_CASE(RinaNameBreakdown, "[rina_name]")
{
    name_t n1;

    RS_TEST_CASE_BEGIN(test_rina_name);

    TEST_ASSERT(xRinaNameFromString("e1|e2|e3|e4", &n1));
    TEST_ASSERT(strcmp(n1.pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1.pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1.pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1.pcEntityInstance, "e4") == 0);
    vRstrNameFini(&n1);

    TEST_ASSERT(xRinaNameFromString("e1|e2|e3|", &n1));
    TEST_ASSERT(strcmp(n1.pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1.pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1.pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1.pcEntityInstance, "") == 0);
    vRstrNameFini(&n1);

    TEST_ASSERT(xRinaNameFromString("e1|e2|e3", &n1));
    TEST_ASSERT(strcmp(n1.pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1.pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1.pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1.pcEntityInstance, "") == 0);
    vRstrNameFini(&n1);

    RS_TEST_CASE_END(test_rina_name);
}

RS_TEST_CASE(RinaNameFromString, "[rina_name]")
{
    name_t *n1;

    RS_TEST_CASE_BEGIN(test_rina_name);

    TEST_ASSERT((n1 = pxRStrNameCreate()) != NULL);
    TEST_ASSERT((n1 = xRINANameInitFrom(n1, "e1", "e2", "e3", "e4")) != NULL);
    TEST_ASSERT(strcmp(n1->pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1->pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityInstance, "e4") == 0);
    vRstrNameDestroy(n1);

    RS_TEST_CASE_END(test_rina_name);
}

RS_TEST_CASE(RinaStringToName, "[rina_name]")
{
    name_t *n1;

    RS_TEST_CASE_BEGIN(test_rina_name);

    TEST_ASSERT((n1 = xRINAstringToName("e1|e2|e3|e4")) != NULL);
    vRstrNameDestroy(n1);

    RS_TEST_CASE_END(test_rina_name);
}

#ifndef TEST_CASE
int main() {
    RS_SUITE_BEGIN();
    RS_RUN_TEST(StringDup);
    RS_RUN_TEST(RinaNameBreakdown);
    RS_RUN_TEST(RinaNameFromString);
    RS_RUN_TEST(RinaStringToName);
    RS_SUITE_END();
}
#endif
