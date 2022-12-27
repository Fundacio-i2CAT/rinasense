#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "portability/port.h"

#include "Ribd_api.h"
#include "Ribd_defs.h"
#include "RibObject.h"

#include "unity.h"
#include "common/unity_fixups.h"

Ribd_t xRibd;

rsErr_t dummyStart(RIB_REQUEST_METHOD_ARGS);

rsErr_t dummyStop(RIB_REQUEST_METHOD_ARGS);

ribObject_t emptyObj;
ribObject_t dummyObj = {
    .ucObjName = "/dummy",
    .ucObjClass = "Dummy",
    .ulObjInst = 0,

    .fnStart = &dummyStart,
    .fnStop = &dummyStop,
    .fnCreate = NULL,
    .fnDelete = NULL,
    .fnWrite = NULL,
    .fnRead = NULL,

    .fnStartReply = NULL,
    .fnStopReply = NULL,
    .fnCreateReply = NULL,
    .fnDeleteReply = NULL,
    .fnWriteReply = NULL,
    .fnReadReply = NULL,

    .fnShow = NULL,

    .fnFree = NULL
};

rsErr_t dummyStart(RIB_REQUEST_METHOD_ARGS)
{
    return SUCCESS;
}

rsErr_t dummyStop(RIB_REQUEST_METHOD_ARGS)
{
    return SUCCESS;
}

RS_TEST_CASE_SETUP(test_rib) {
    xRibInit(&xRibd);

    bzero(&emptyObj, sizeof(ribObject_t));
    emptyObj.ucObjClass = "test",
    emptyObj.ucObjName = "/difm/test";
}

RS_TEST_CASE_TEARDOWN(test_rib) {}

RS_TEST_CASE(RibAddObject, "[rib]")
{
    RS_TEST_CASE_BEGIN(test_rib);
    TEST_ASSERT(!ERR_CHK(xRibObjectAdd(&xRibd, NULL, &emptyObj)));
    RS_TEST_CASE_END(test_rib);
}

RS_TEST_CASE(RibFindObject, "[rib]")
{
    RS_TEST_CASE_BEGIN(test_rib);
    TEST_ASSERT(!ERR_CHK(xRibObjectAdd(&xRibd, NULL, &emptyObj)));
    TEST_ASSERT(pxRibObjectFind(&xRibd, "/difm/test") != NULL);
    TEST_ASSERT(pxRibObjectFind(&xRibd, "blarg") == NULL);
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
