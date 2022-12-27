#include "portability/port.h"

#include "common/error.h"
#include "common/rina_name.h"
#include "common/rsrc.h"

#include "SerDes.h"
#include "SerDesMessage.h"
#include "SerDesEnrollment.h"
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

#include "unity.h"
#include "common/unity_fixups.h"

MessageSerDes_t xMsgSD;
EnrollmentSerDes_t xEnrollmentSD;

static void BullshitMsg(messageCdap_t *pxMsg)
{
    pxMsg->version = 1;
    pxMsg->nInvokeID = 1;
    pxMsg->eOpCode = M_START;
    pxMsg->objInst = 1;

    pxMsg->pcObjClass = "objClass";
    pxMsg->pcObjName = "objName";
    pxMsg->pcResultReason = "pcResultReason";
    pxMsg->result = 99;

    pxMsg->xAuthPolicy.pcName = "AuthPolicy";
    pxMsg->xAuthPolicy.pcVersion = "AP Version";
    pxMsg->xAuthPolicy.ucAbsSyntax = 1;

    vNameAssignFromPartsStatic(&pxMsg->xDestinationInfo, "D-PN", "D-PI", "D-EN", "D-EI");
    vNameAssignFromPartsStatic(&pxMsg->xSourceInfo, "S-PN", "S-PI", "S-EN", "S-EI");
}

static void IsBullshitMsgOkay(messageCdap_t *pxMsgA, messageCdap_t *pxMsgB)
{
    TEST_ASSERT(pxMsgA->version  == pxMsgB->version);
    TEST_ASSERT(pxMsgA->nInvokeID == pxMsgB->nInvokeID);
    TEST_ASSERT(pxMsgA->eOpCode  == pxMsgB->eOpCode);
    TEST_ASSERT(pxMsgA->objInst  == pxMsgB->objInst);
    TEST_ASSERT(pxMsgA->result   == pxMsgB->result);

    TEST_ASSERT(strcmp(pxMsgA->pcObjClass, pxMsgB->pcObjClass) == 0);
    TEST_ASSERT(strcmp(pxMsgA->pcObjName,  pxMsgB->pcObjName) == 0);
    TEST_ASSERT(strcmp(pxMsgA->pcResultReason, pxMsgB->pcResultReason) == 0);

    TEST_ASSERT(strcmp(pxMsgA->xDestinationInfo.pcProcessName, pxMsgB->xDestinationInfo.pcProcessName) == 0);
    TEST_ASSERT(strcmp(pxMsgA->xDestinationInfo.pcProcessInstance, pxMsgB->xDestinationInfo.pcProcessInstance) == 0);
    TEST_ASSERT(strcmp(pxMsgA->xDestinationInfo.pcEntityName, pxMsgB->xDestinationInfo.pcEntityName) == 0);
    TEST_ASSERT(strcmp(pxMsgA->xDestinationInfo.pcEntityInstance, pxMsgB->xDestinationInfo.pcEntityInstance) == 0);

    TEST_ASSERT(strcmp(pxMsgA->xSourceInfo.pcProcessName, pxMsgB->xSourceInfo.pcProcessName) == 0);
    TEST_ASSERT(strcmp(pxMsgA->xSourceInfo.pcProcessInstance, pxMsgB->xSourceInfo.pcProcessInstance) == 0);
    TEST_ASSERT(strcmp(pxMsgA->xSourceInfo.pcEntityName, pxMsgB->xSourceInfo.pcEntityName) == 0);
    TEST_ASSERT(strcmp(pxMsgA->xSourceInfo.pcEntityInstance, pxMsgB->xSourceInfo.pcEntityInstance) == 0);

    TEST_ASSERT(strcmp(pxMsgA->xAuthPolicy.pcName, pxMsgB->xAuthPolicy.pcName) == 0);
    TEST_ASSERT(strcmp(pxMsgA->xAuthPolicy.pcVersion, pxMsgB->xAuthPolicy.pcVersion) == 0);
    /*TEST_ASSERT(pxMsgA->xAuthPolicy.ucAbsSyntax == pxMsgB->xAuthPolicy.ucAbsSyntax);*/

    if (pxMsgA->pxObjValue && pxMsgB->pxObjValue) {
        TEST_ASSERT(pxMsgA->pxObjValue->xSerLength == pxMsgB->pxObjValue->xSerLength);
        TEST_ASSERT(memcmp(pxMsgA->pxObjValue->pvSerBuffer, pxMsgB->pxObjValue->pvSerBuffer,
                           pxMsgA->pxObjValue->xSerLength) == 0);
    }
}

RS_TEST_CASE_SETUP(test_SerDesMessage)
{
    TEST_ASSERT(xSerDesMessageInit(&xMsgSD));
    TEST_ASSERT(!ERR_CHK_MEM(xSerDesEnrollmentInit(&xEnrollmentSD)));
}

RS_TEST_CASE_TEARDOWN(test_SerDesMessage)
{
}

RS_TEST_CASE(Encode, "[SerDes]")
{
    messageCdap_t xMsg;
    serObjectValue_t *pxObjVal;

    RS_TEST_CASE_BEGIN(test_SerDesMessage);

    memset(&xMsg, 0, sizeof(messageCdap_t));

    BullshitMsg(&xMsg);

    TEST_ASSERT((pxObjVal = pxSerDesMessageEncode(&xMsgSD, &xMsg)));
    TEST_ASSERT(pxObjVal->xSerLength > 0);
    TEST_ASSERT(pxObjVal->pvSerBuffer);

    vRsrcFree(pxObjVal);

    RS_TEST_CASE_END(test_SerDesMessage);
}

RS_TEST_CASE(EncDec1, "[SerDes]")
{
    messageCdap_t xMsg, *pxMsgDec;
    serObjectValue_t *pxObjVal;

    RS_TEST_CASE_BEGIN(test_SerDesMessage);

    memset(&xMsg, 0, sizeof(messageCdap_t));

    BullshitMsg(&xMsg);

    TEST_ASSERT((pxObjVal = pxSerDesMessageEncode(&xMsgSD, &xMsg)));
    TEST_ASSERT((pxMsgDec = pxSerDesMessageDecode(&xMsgSD, pxObjVal->pvSerBuffer, pxObjVal->xSerLength)));

    IsBullshitMsgOkay(&xMsg, pxMsgDec);

    RS_TEST_CASE_END(test_SerDesMessage);
}

RS_TEST_CASE(EncDec2, "[SerDes]")
{
    messageCdap_t xMsg, *pxMsgDec;
    serObjectValue_t *pxObjVal, *pxErVal;
    enrollmentMessage_t xEnMsg;

    RS_TEST_CASE_BEGIN(test_SerDesMessage);

    memset(&xMsg, 0, sizeof(messageCdap_t));

    BullshitMsg(&xMsg);

    xEnMsg.pcSupportingDifs = "DIF";
    xEnMsg.pcToken = "Token";
    xEnMsg.ullAddress = 12345678;
    xEnMsg.xStartEarly = false;

    TEST_ASSERT((pxErVal = pxSerDesEnrollmentEncode(&xEnrollmentSD, &xEnMsg)));

    xMsg.pxObjValue = pxErVal;

    TEST_ASSERT((pxObjVal = pxSerDesMessageEncode(&xMsgSD, &xMsg)));
    TEST_ASSERT((pxMsgDec = pxSerDesMessageDecode(&xMsgSD, pxObjVal->pvSerBuffer, pxObjVal->xSerLength)));

    IsBullshitMsgOkay(&xMsg, pxMsgDec);

    RS_TEST_CASE_END(test_SerDesMessage);
}

#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(Encode);
    RS_RUN_TEST(EncDec1);
    RS_RUN_TEST(EncDec2);
    RS_SUITE_END();
}
#endif
