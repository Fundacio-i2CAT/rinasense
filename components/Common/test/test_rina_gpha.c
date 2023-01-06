#include <string.h>

#include "portability/port.h"
#include "common/rina_name.h"
#include "common/rina_gpha.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_rina_gpha) {}
RS_TEST_CASE_TEARDOWN(test_rina_gpha) {}

RS_TEST_CASE(SimpleGPA, "[gpa][gpha]")
{
    string_t addr = "address";
    gpa_t *gpa;

    RS_TEST_CASE_BEGIN(test_rina_gpha);

    TEST_ASSERT((gpa = pxCreateGPA((buffer_t)addr, strlen(addr))) != NULL);
    TEST_ASSERT(strcmp((string_t)gpa->pucAddress, "address") == 0);
    TEST_ASSERT(gpa->uxLength == 7);
    TEST_ASSERT(xIsGPAOK(gpa) == true);
    vGPADestroy(gpa);

    /* This should not be true at this point but I'm not sure how
     * to invalidate the struct yet! */
    //assert(xIsGPAOK(gpa) != true);

    RS_TEST_CASE_END(test_rina_gpha);
}

/* Create a GPA from a RINA name, then convert back the GPA to a RINA
 * name, and then convert that name back to string. */
RS_TEST_CASE(GPAConversion, "[gpa][gpha]")
{
    string_t nm1 = "ue1.mobile.DIF/1//", nm2;
    gpa_t *gpa1;
    rname_t *n1;

    RS_TEST_CASE_BEGIN(test_rina_gpha);

    TEST_ASSERT(n1 = pxNameNewFromString(nm1));
    TEST_ASSERT((gpa1 = pxNameToGPA(n1)) != NULL);
    TEST_ASSERT(xIsGPAOK(gpa1) == true);
    TEST_ASSERT((nm2 = xGPAAddressToString(gpa1)) != NULL);

    vNameFree(n1);
    vGPADestroy(gpa1);
    vRsMemFree(nm2);

    /* This will not work while the name separator isn't set to | */
    /* RsAssert(strcmp(nm1, nm2) == 0); */

    RS_TEST_CASE_END(test_rina_gpha);
}

RS_TEST_CASE(SimpleGHA, "[gha][gpha]")
{
    const MACAddress_t mac = {
        {1, 2, 3, 4, 5, 6}
    };
    gha_t *gha1, *gha2;

    RS_TEST_CASE_BEGIN(test_rina_gpha);

    TEST_ASSERT((gha1 = pxCreateGHA(MAC_ADDR_802_3, &mac)) != NULL);
    TEST_ASSERT((gha2 = pxCreateGHA(1234, &mac)) == NULL);
    TEST_ASSERT(xIsGHAOK(gha1) == true);
    TEST_ASSERT(xIsGHAOK(gha2) == false);
    vGHADestroy(gha1);
    vGHADestroy(gha2);

    RS_TEST_CASE_END(test_rina_gpha);
}

RS_TEST_CASE(GPACompare, "[gha][gpha]") {
    gpa_t *gpa1, *gpa2, *gpa3;
    string_t addr1 = "addressA";
    string_t addr2 = "addressA\0\0\0";
    string_t addr3 = "addressB\0\0\0";

    RS_TEST_CASE_BEGIN(test_rina_gpha);

    gpa1 = pxCreateGPA((buffer_t)addr1, strlen(addr1));
    gpa2 = pxCreateGPA((buffer_t)addr2, strlen(addr2) + 3);
    gpa3 = pxCreateGPA((buffer_t)addr3, strlen(addr3) + 3);
    TEST_ASSERT(xGPACmp(gpa1, gpa2) == true);
    TEST_ASSERT(xGPACmp(gpa1, gpa3) == false);

    vGPADestroy(gpa1);
    vGPADestroy(gpa2);
    vGPADestroy(gpa3);

    RS_TEST_CASE_END(test_rina_gpha);
}

RS_TEST_CASE(GPADup, "[gpa][gpha]") {
    gpa_t *gpa1, *gpa2, *gpa3;
    string_t addr1 = "A\0\0\0";

    RS_TEST_CASE_BEGIN(test_rina_gpha);

    gpa1 = pxCreateGPA((buffer_t)addr1, strlen(addr1) + 3);
    gpa2 = pxDupGPA(gpa1, false, 0);
    TEST_ASSERT(xGPACmp(gpa1, gpa2));
    TEST_ASSERT(gpa1->uxLength == gpa2->uxLength);

    gpa3 = pxDupGPA(gpa1, true, 0);
    TEST_ASSERT(xGPACmp(gpa1, gpa3));
    TEST_ASSERT(gpa3->uxLength == 1);

    vGPADestroy(gpa1);
    vGPADestroy(gpa2);
    vGPADestroy(gpa3);

    RS_TEST_CASE_END(test_rina_gpha);
}

RS_TEST_CASE(GPAShrink, "[gpa][gpha]") {
    gpa_t *gpa1, *gpa2, *gpa3;
    string_t addr1 = "A\0\0\0";
    string_t addr2 = "A";

    RS_TEST_CASE_BEGIN(test_rina_gpha);

    gpa1 = pxCreateGPA((buffer_t)addr1, strlen(addr1) + 3);
    gpa2 = pxDupGPA(gpa1, false, 0);
    TEST_ASSERT(xGPACmp(gpa1, gpa2));
    TEST_ASSERT(gpa1->uxLength == 4);
    TEST_ASSERT(!ERR_CHK(xGPAShrink(gpa1, 0)));
    TEST_ASSERT(xGPACmp(gpa1, gpa2));
    TEST_ASSERT(gpa1->uxLength == 1);
    gpa3 = pxCreateGPA((buffer_t)addr2, strlen(addr1));
    TEST_ASSERT(!ERR_CHK(xGPAShrink(gpa3, 0)));

    vGPADestroy(gpa1);
    vGPADestroy(gpa2);
    vGPADestroy(gpa3);

    RS_TEST_CASE_END(test_rina_gpha);
}

#ifndef TEST_CASE
int main() {
    RS_SUITE_BEGIN();
    RS_RUN_TEST(SimpleGPA);
    RS_RUN_TEST(SimpleGHA);
    RS_RUN_TEST(GPACompare);
    RS_RUN_TEST(GPAConversion);
    RS_RUN_TEST(GPADup);
    RS_RUN_TEST(GPAShrink);
    RS_SUITE_END();
}
#endif
