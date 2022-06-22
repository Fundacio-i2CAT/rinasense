#include "portability/port.h"
#include "rina_name.h"
#include "rina_gpha.h"

#include "unity.h"
#include "unity_fixups.h"
#include <string.h>

#ifndef TEST_CASE
void setUp() {}
void tearDown() {}
#endif

RS_TEST_CASE(SimpleGPA, "Simple GPA manipulations")
{
    string_t addr = "address";
    gpa_t *gpa;

    TEST_ASSERT((gpa = pxCreateGPA((buffer_t)addr, strlen(addr))) != NULL);
    TEST_ASSERT(strcmp((string_t)gpa->pucAddress, "address") == 0);
    TEST_ASSERT(gpa->uxLength == 7);
    TEST_ASSERT(xIsGPAOK(gpa) == true);
    vGPADestroy(gpa);

    /* This should not be true at this point but I'm not sure how
     * to invalidate the struct yet! */
    //assert(xIsGPAOK(gpa) != true);
}

/* Create a GPA from a RINA name, then convert back the GPA to a RINA
 * name, and then convert that name back to string. */
RS_TEST_CASE(GPAConversion, "GPA conversion / comparison")
{
    string_t nm1 = "e1|e2|e3|e4", nm2;
    gpa_t *gpa1;
    name_t n1;

    TEST_ASSERT(xRinaNameFromString(nm1, &n1) == true);
    TEST_ASSERT((gpa1 = pxNameToGPA(&n1)) != NULL);
    TEST_ASSERT(xIsGPAOK(gpa1) == true);
    TEST_ASSERT((nm2 = xGPAAddressToString(gpa1)) != NULL);

    LOGD(TAG_RINA, "nm1: %s", nm1);
    LOGD(TAG_RINA, "nm2: %s", nm2);

    /* This will not work while the name separator isn't set to | */
    /* RsAssert(strcmp(nm1, nm2) == 0); */
}

RS_TEST_CASE(SimpleGHA, "Simple GHA manipulation")
{
    const MACAddress_t mac = {
        {1, 2, 3, 4, 5, 6}
    };
    gha_t *gha1, *gha2;

    TEST_ASSERT((gha1 = pxCreateGHA(MAC_ADDR_802_3, &mac)) != NULL);
    TEST_ASSERT((gha2 = pxCreateGHA(1234, &mac)) == NULL);
    TEST_ASSERT(xIsGHAOK(gha1) == true);
    TEST_ASSERT(xIsGHAOK(gha2) == false);
    vGHADestroy(gha1);
    vGHADestroy(gha2);
}

RS_TEST_CASE(GPACompare, "GPA conversion / comparison") {
    gpa_t *gpa1, *gpa2, *gpa3;
    string_t addr1 = "addressA";
    string_t addr2 = "addressA";
    string_t addr3 = "addressB";

    gpa1 = pxCreateGPA((buffer_t)addr1, strlen(addr1));
    gpa2 = pxCreateGPA((buffer_t)addr2, strlen(addr2));
    gpa3 = pxCreateGPA((buffer_t)addr3, strlen(addr3));
    TEST_ASSERT(xGPACmp(gpa1, gpa2) == true);
    TEST_ASSERT(xGPACmp(gpa1, gpa3) == false);
    gpa2->uxLength--;
    TEST_ASSERT(xGPACmp(gpa1, gpa2) == false);
    vGPADestroy(gpa1);
    vGPADestroy(gpa2);
    vGPADestroy(gpa3);
}

#ifndef TEST_CASE
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_SimpleGPA);
    RUN_TEST(test_SimpleGHA);
    RUN_TEST(test_GPACompare);
    RUN_TEST(test_GPAConversion);
    return UNITY_END();
}
#endif
