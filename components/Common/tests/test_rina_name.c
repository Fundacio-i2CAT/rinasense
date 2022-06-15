#include <stdio.h>
#include <string.h>
#include "rina_name.h"

/* Tests plain string duplication. */
void testStringDup()
{
    const string_t s1 = "hello";
    string_t s2, s3;

    RsAssert(xRstringDup(s1, &s2));
    RsAssert(strcmp(s1, s2) == 0);

    RsAssert(xRstringDup(s1, &s3));
    RsAssert(strcmp(s1, s3) == 0);
    RsAssert(s1 != s3);

    vRsMemFree(s2);
    vRsMemFree(s3);
}

/* Test that RinaNameFromString breaks down a name in all its
 * component. */
void testRinaNameBreakdown()
{
    name_t n1;

    RsAssert(xRinaNameFromString("e1|e2|e3|e4", &n1));
    RsAssert(strcmp(n1.pcProcessName, "e1") == 0);
    RsAssert(strcmp(n1.pcProcessInstance, "e2") == 0);
    RsAssert(strcmp(n1.pcEntityName, "e3") == 0);
    RsAssert(strcmp(n1.pcEntityInstance, "e4") == 0);
    vRstrNameFini(&n1);
}

void testRinaNameFromString()
{
    name_t *n1;

    RsAssert((n1 = pxRStrNameCreate()) != NULL);
    RsAssert((n1 = xRINANameInitFrom(n1, "e1", "e2", "e3", "e4")) != NULL);
    RsAssert(strcmp(n1->pcProcessName, "e1") == 0);
    RsAssert(strcmp(n1->pcProcessInstance, "e2") == 0);
    RsAssert(strcmp(n1->pcEntityName, "e3") == 0);
    RsAssert(strcmp(n1->pcEntityInstance, "e4") == 0);
    vRstrNameDestroy(n1);
}

void testRinaStringToName()
{
    name_t *n1;

    RsAssert((n1 = xRINAstringToName("e1|e2|e3|e4")) != NULL);
    vRstrNameDestroy(n1);
}

int main() {
    testStringDup();
    testRinaNameBreakdown();
    testRinaNameFromString();
    testRinaStringToName();
}
