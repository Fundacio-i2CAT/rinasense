#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "linux_rsmem.h"
#include "portability/port.h"

#include "Ribd_api.h"
#include "Ribd_defs.h"
#include "RibObject.h"

#include "unity.h"
#include "common/unity_fixups.h"

Ribd_t xRibd;

bool_t xDummyStartRequestCalled = false;
bool_t xDummyStartReplyCalled = false;
bool_t xDummyStartReplyCallbackCalled = false;
bool_t xDummyStopCalled = false;

int *pxCallbackResp;

rsErr_t dummyStartRequest(RIB_REQUEST_METHOD_ARGS);
rsErr_t dummyStartReply(RIB_REPLY_METHOD_ARGS);

rsErr_t dummyStop(RIB_REQUEST_METHOD_ARGS);

ribObject_t emptyObj;
ribObject_t dummyObj = {
    .ucObjName = "/dummy",
    .ucObjClass = "Dummy",
    .ulObjInst = 0,

    .fnStart = &dummyStartRequest,
    .fnStop = &dummyStartRequest,
    .fnCreate = NULL,
    .fnDelete = NULL,
    .fnWrite = NULL,
    .fnRead = NULL,

    .fnStartReply = &dummyStartReply,
    .fnStopReply = NULL,
    .fnCreateReply = NULL,
    .fnDeleteReply = NULL,
    .fnWriteReply = NULL,
    .fnReadReply = NULL,

    .fnShow = NULL,

    .fnFree = NULL
};

rsErr_t dummyStartReplyCallback(Ribd_t *pxRibd, ribObject_t *pxThis, void *pxResp)
{
    pxCallbackResp = (int *)pxResp;
    xDummyStartReplyCallbackCalled = true;

    return SUCCESS;
}

rsErr_t dummyStartRequest(ribObject_t *pxThis, appConnection_t *pxAppCon, messageCdap_t *pxMsg)
{
    xDummyStartRequestCalled = true;

    xRibSTART_REPLY(&xRibd, pxThis, -1, pxMsg->nInvokeID, NULL);

    return SUCCESS;
}

rsErr_t dummyStartReply(ribObject_t *pxThis, appConnection_t *pxAppCon, messageCdap_t *pxMsg, void **ppxResp)
{
    xDummyStartReplyCalled = true;

    TEST_ASSERT(*ppxResp = pvRsMemAlloc(sizeof(int)));
    *(int *)(*ppxResp) = 99;

    return SUCCESS;
}

rsErr_t dummyStop(RIB_REQUEST_METHOD_ARGS)
{
    return SUCCESS;
}

RS_TEST_CASE_SETUP(test_rib) {
    xRibLoopbackInit(&xRibd);

    xDummyStartRequestCalled = false;
    xDummyStartReplyCalled = false;
    xDummyStopCalled = false;

    bzero(&emptyObj, sizeof(ribObject_t));
    emptyObj.ucObjClass = "test",
    emptyObj.ucObjName = "/difm/test";

    xRibObjectAdd(&xRibd, NULL, &dummyObj);
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

RS_TEST_CASE(RibRequestSync, "[rib]")
{
    ribObject_t *pxRibObj;
    invokeId_t nId;
    int *n = NULL;

    RS_TEST_CASE_BEGIN(test_rib);

    TEST_ASSERT(!ERR_CHK(xRibObjectAdd(&xRibd, NULL, &dummyObj)));
    TEST_ASSERT((pxRibObj = pxRibObjectFind(&xRibd, "/dummy")) != NULL);
    TEST_ASSERT(!ERR_CHK(xRibSTART_QUERY_SYNC(&xRibd, pxRibObj, -1, NULL, &nId)));
    TEST_ASSERT(nId != 0);
    TEST_ASSERT(xDummyStartReplyCalled);
    TEST_ASSERT(xDummyStartReplyCalled);
    TEST_ASSERT(!ERR_CHK(xRibObjectWaitReply(&xRibd, nId, (void **)&n)));
    TEST_ASSERT(*n == 99);

    vRsMemFree(n);

    RS_TEST_CASE_END(test_rib);
}

RS_TEST_CASE(RibRequestAsync, "[rib]")
{
    ribObject_t *pxRibObj;
    invokeId_t nId;
    int n;

    RS_TEST_CASE_BEGIN(test_rib);

    TEST_ASSERT(!ERR_CHK(xRibObjectAdd(&xRibd, NULL, &dummyObj)));
    TEST_ASSERT((pxRibObj = pxRibObjectFind(&xRibd, "/dummy")) != NULL);
    TEST_ASSERT(!ERR_CHK(xRibSTART_QUERY_ASYNC(&xRibd, pxRibObj, -1, NULL, &dummyStartReplyCallback)));
    TEST_ASSERT(xDummyStartReplyCalled);
    TEST_ASSERT(xDummyStartReplyCalled);
    TEST_ASSERT(*pxCallbackResp == 99);

    vRsMemFree(pxCallbackResp);

    RS_TEST_CASE_END(test_rib);
}


#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(RibAddObject);
    RS_RUN_TEST(RibFindObject);
    RS_RUN_TEST(RibRequestSync);
    RS_RUN_TEST(RibRequestAsync);
    RS_SUITE_END();
}
#endif
