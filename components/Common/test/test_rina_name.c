#include <stdio.h>
#include <string.h>
#include "rina_name.h"

#include "unity.h"
#include "unity_fixups.h"

#ifndef TEST_CASE
void setUp() {}
void tearDown() {}
#endif

/* Tests plain string duplication. */
RS_TEST_CASE(StringDup, "String duplication")
{
    const string_t s1 = "hello";
    string_t s2, s3;

    TEST_ASSERT(xRstringDup(s1, &s2));
    TEST_ASSERT(strcmp(s1, s2) == 0);

    TEST_ASSERT(xRstringDup(s1, &s3));
    TEST_ASSERT(strcmp(s1, s3) == 0);
    TEST_ASSERT(s1 != s3);

    vRsMemFree(s2);
    vRsMemFree(s3);
}

/* Test that RinaNameFromString breaks down a name in all its
 * component. */
RS_TEST_CASE(RinaNameBreakdown, "RINA name breakdown in parts")
{
    name_t n1;

    TEST_ASSERT(xRinaNameFromString("e1|e2|e3|e4", &n1));
    TEST_ASSERT(strcmp(n1.pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1.pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1.pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1.pcEntityInstance, "e4") == 0);
    vRstrNameFini(&n1);
}

RS_TEST_CASE(RinaNameFromString, "Conversion of RINA names to string")
{
    name_t *n1;

    TEST_ASSERT((n1 = pxRStrNameCreate()) != NULL);
    TEST_ASSERT((n1 = xRINANameInitFrom(n1, "e1", "e2", "e3", "e4")) != NULL);
    TEST_ASSERT(strcmp(n1->pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1->pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityInstance, "e4") == 0);
    vRstrNameDestroy(n1);
}

RS_TEST_CASE(RinaStringToName, "Conversion ot string to RINA name")
{
    name_t *n1;

    TEST_ASSERT((n1 = xRINAstringToName("e1|e2|e3|e4")) != NULL);
    vRstrNameDestroy(n1);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_StringDup);
    RUN_TEST(test_RinaNameBreakdown);
    RUN_TEST(test_RinaNameFromString);
    RUN_TEST(test_RinaStringToName);
    return UNITY_END();
}
