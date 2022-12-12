#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "Rib.h"
#include "RibObject.h"
#include "Ribd.h"

#include "unity.h"
#include "common/unity_fixups.h"

Ribd_t xRibd;
ribObject_t obj;

RS_TEST_CASE_SETUP(test_rib) {
    xRibdInit(&xRibd);

    bzero(&obj, sizeof(ribObject_t));
    obj.ucObjClass = "test",
    obj.ucObjName = "/difm/test";
}

RS_TEST_CASE_TEARDOWN(test_rib) {}

RS_TEST_CASE(RibAddObject, "[rib]")
{
    RS_TEST_CASE_BEGIN(test_rib);
    TEST_ASSERT(xRibAddObjectEntry(&xRibd, &obj));
    RS_TEST_CASE_END(test_rib);
}

RS_TEST_CASE(RibFindObject, "[rib]")
{
    RS_TEST_CASE_BEGIN(test_rib);
    TEST_ASSERT(xRibAddObjectEntry(&xRibd, &obj));
    TEST_ASSERT(pxRibFindObject(&xRibd, "/difm/test") != NULL);
    TEST_ASSERT(pxRibFindObject(&xRibd, "blarg") == NULL);
    RS_TEST_CASE_END(test_rib);
}

#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(RibAddObject);
    RS_RUN_TEST(RibFindObject);
    RS_SUITE_END();
}
#endif
