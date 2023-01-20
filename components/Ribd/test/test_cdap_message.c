#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "portability/port.h"

#include "CdapMessage.h"
#include "Ribd_defs.h"
#include "Ribd_api.h"

#include "unity.h"
#include "common/unity_fixups.h"

Ribd_t xRibd;

#define OBJCLASS "a"
#define OBJNAME  "bcde"

RS_TEST_CASE_SETUP(test_cdap_message) {
    xRibLoopbackInit(&xRibd);
}

RS_TEST_CASE_TEARDOWN(test_cdap_message) {}

RS_TEST_CASE(CdapMessageRequest, "[cdap]")
{
    messageCdap_t *pxMsg;

    RS_TEST_CASE_BEGIN(test_cdap_message);

    TEST_ASSERT(pxMsg = pxRibCdapMsgCreateRequest(&xRibd, OBJCLASS, OBJNAME, 99, M_CREATE, NULL));
    TEST_ASSERT(strcmp(pxMsg->pcObjClass, OBJCLASS) == 0);
    TEST_ASSERT(strcmp(pxMsg->pcObjName, OBJNAME) == 0);
    TEST_ASSERT(strcmp(pxMsg->xDestinationInfo.pcProcessInstance, "Management") == 0);
    vRibCdapMsgFree(pxMsg);

    RS_TEST_CASE_END(test_cdap_message);
}

RS_TEST_CASE(CdapMessageResponse, "[cdap]")
{
    messageCdap_t *pxMsg;

    RS_TEST_CASE_BEGIN(test_cdap_message);

    TEST_ASSERT(pxMsg = pxRibCdapMsgCreateResponse(&xRibd, OBJCLASS, OBJNAME, 99, M_CREATE_R, NULL, 1, "Test", 1));
    TEST_ASSERT(strcmp(pxMsg->pcResultReason, "Test") == 0);
    TEST_ASSERT(strcmp(pxMsg->pcObjClass, OBJCLASS) == 0);
    TEST_ASSERT(strcmp(pxMsg->pcObjName, OBJNAME) == 0);
    vRibCdapMsgFree(pxMsg);

    RS_TEST_CASE_END(test_cdap_message);
}

#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(CdapMessageRequest);
    RS_RUN_TEST(CdapMessageResponse);
    RS_SUITE_END();
}
#endif

