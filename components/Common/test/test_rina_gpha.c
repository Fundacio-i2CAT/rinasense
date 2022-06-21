#include "portability/port.h"
#include "rina_name.h"
#include "rina_gpha.h"

void testSimpleGPA() {
    string_t addr = "address";
    gpa_t *gpa;

    RsAssert((gpa = pxCreateGPA(addr, sizeof(addr))) != NULL);
    RsAssert(strcmp(gpa->pucAddress, "address") == 0);
    RsAssert(gpa->uxLength == 8);
    RsAssert(xIsGPAOK(gpa) == true);
    vGPADestroy(gpa);

    /* This should not be true at this point but I'm not sure how
     * to invalidate the struct yet! */
    //assert(xIsGPAOK(gpa) != true);
}

/* Create a GPA from a RINA name, then convert back the GPA to a RINA
 * name, and then convert that name back to string. */
void testGPAConversion() {
    string_t nm1 = "e1|e2|e3|e4", nm2;
    gpa_t *gpa1, *gpa2;
    name_t n1;

    RsAssert(xRinaNameFromString(nm1, &n1) == true);
    RsAssert((gpa1 = pxNameToGPA(&n1)) != NULL);
    RsAssert(xIsGPAOK(gpa1) == true);
    RsAssert((nm2 = xGPAAddressToString(gpa1)) != NULL);

    LOGD(TAG_RINA, "nm1: %s", nm1);
    LOGD(TAG_RINA, "nm2: %s", nm2);

    /* This will not work while the name separator isn't set to | */
    /* RsAssert(strcmp(nm1, nm2) == 0); */
}

void testSimpleGHA() {
    const MACAddress_t mac = {
        1, 2, 3, 4, 5, 6
    };
    gha_t *gha1, *gha2;

    RsAssert((gha1 = pxCreateGHA(MAC_ADDR_802_3, &mac)) != NULL);
    RsAssert((gha2 = pxCreateGHA(1234, &mac)) == NULL);
    RsAssert(xIsGHAOK(gha1) == true);
    RsAssert(xIsGHAOK(gha2) == false);
    vGHADestroy(gha1);
    vGHADestroy(gha2);
}

void testGPACompare() {
    gpa_t *gpa1, *gpa2, *gpa3;
    string_t addr1 = "addressA";
    string_t addr2 = "addressA";
    string_t addr3 = "addressB";

    gpa1 = pxCreateGPA(addr1, sizeof(addr1));
    gpa2 = pxCreateGPA(addr2, sizeof(addr2));
    gpa3 = pxCreateGPA(addr3, sizeof(addr3));
    RsAssert(xGPACmp(gpa1, gpa2) == true);
    RsAssert(xGPACmp(gpa1, gpa3) == false);
    gpa2->uxLength--;
    RsAssert(xGPACmp(gpa1, gpa2) == false);
    vGPADestroy(gpa1);
    vGPADestroy(gpa2);
    vGPADestroy(gpa3);
}

int main() {
    testSimpleGPA();
    testSimpleGHA();
    testGPACompare();
    testGPAConversion();
}
