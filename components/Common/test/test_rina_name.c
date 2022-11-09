#include <stdio.h>
#include <string.h>

#include "common/rina_name.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_rina_name) {}
RS_TEST_CASE_TEARDOWN(test_rina_name) {}

/* Test that RinaNameFromString breaks down a name in all its
 * component. */
RS_TEST_CASE(RinaNameBreakdown, "[rina_name]")
{
    rname_t *n1;

    RS_TEST_CASE_BEGIN(test_rina_name);

    TEST_ASSERT(n1 = pxNameNewFromString("e1/e2/e3/e4"));
    TEST_ASSERT(strcmp(n1->pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1->pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityInstance, "e4") == 0);
    vNameFree(n1);

    TEST_ASSERT(n1 = pxNameNewFromString("e1/e2/e3/"));
    TEST_ASSERT(strcmp(n1->pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1->pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityInstance, "") == 0);
    vNameFree(n1);

    TEST_ASSERT(n1 = pxNameNewFromString("e1/e2/e3"));
    TEST_ASSERT(strcmp(n1->pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1->pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityInstance, "") == 0);
    vNameFree(n1);

    RS_TEST_CASE_END(test_rina_name);
}

RS_TEST_CASE(RinaNameNewFromParts, "[rina_name]")
{
    rname_t *n1;

    RS_TEST_CASE_BEGIN(test_rina_name);

    TEST_ASSERT((n1 = pxNameNewFromParts("e1", "e2", "e3", "e4")) != NULL);
    TEST_ASSERT(strcmp(n1->pcProcessName, "e1") == 0);
    TEST_ASSERT(strcmp(n1->pcProcessInstance, "e2") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityName, "e3") == 0);
    TEST_ASSERT(strcmp(n1->pcEntityInstance, "e4") == 0);
    vNameFree(n1);

    RS_TEST_CASE_END(test_rina_name);
}

RS_TEST_CASE(RinaNameToString, "[rina_name]")
{
    rname_t *n1, n2;
    char bbuf[255] = {0}, sbuf[5] = {0};
    string_t s;
    size_t sz;

    RS_TEST_CASE_BEGIN(test_rina_name);

    TEST_ASSERT((n1 = pxNameNewFromParts("e1", "e2", "e3", "e4")));
    vNameToStringBuf(n1, bbuf, sizeof(bbuf));
    TEST_ASSERT(strcmp(bbuf, "e1/e2/e3/e4") == 0);
    vNameToStringBuf(n1, sbuf, sizeof(sbuf));
    TEST_ASSERT(strcmp(sbuf, "e1/e") == 0);
    vNameFree(n1);

    TEST_ASSERT((n1 = pxNameNewFromParts("longpart", "", "", "")));
    vNameToStringBuf(n1, bbuf, sizeof(bbuf));
    TEST_ASSERT(strcmp(bbuf, "longpart///") == 0);
    vNameToStringBuf(n1, sbuf, sizeof(sbuf));
    TEST_ASSERT(strcmp(sbuf, "long") == 0);
    vNameFree(n1);

    TEST_ASSERT((n1 = pxNameNewFromString("ue1.mobile.DIF/1//")));
    TEST_ASSERT((s = pcNameToString(n1)));
    TEST_ASSERT((sz = strlen(s)) == 18);
    TEST_ASSERT(strcmp(s, "ue1.mobile.DIF/1//") == 0);
    vRsMemFree(s);
    vNameFree(n1);

    TEST_ASSERT(xNameAssignFromPartsDup(&n2, "ue1.mobile.DIF", "1", "", ""));
    TEST_ASSERT(s = pcNameToString(&n2));
    TEST_ASSERT((sz = strlen(s)) == 18);
    TEST_ASSERT(strcmp(s, "ue1.mobile.DIF/1//") == 0);
    vRsMemFree(s);
    vNameFree(&n2);

    RS_TEST_CASE_END(test_rina_name);
}

#ifndef TEST_CASE
int main() {
    RS_SUITE_BEGIN();
    RS_RUN_TEST(RinaNameBreakdown);
    RS_RUN_TEST(RinaNameNewFromParts);
    RS_RUN_TEST(RinaNameToString);
    RS_SUITE_END();
}
#endif
