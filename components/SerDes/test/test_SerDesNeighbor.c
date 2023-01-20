#include <string.h>

#include "common/rsrc.h"
#include "common/error.h"

#include "SerDes.h"
#include "SerDesNeighbor.h"

#include "unity.h"
#include "common/unity_fixups.h"

NeighborSerDes_t xNeighborSD;

void xMakeBullshitNeighbor(neighborMessage_t *pxNbMsg)
{
    pxNbMsg->pcApName = "apname";
    pxNbMsg->pcApInstance = "apinstance";
    pxNbMsg->pcAeName = "aename";
    pxNbMsg->pcAeInstance = "aeinstance";
    pxNbMsg->ullAddress = 10;
    pxNbMsg->unSupportingDifCount = 2;
    pxNbMsg->pcSupportingDifs[0] = "mobile.DIF";
    pxNbMsg->pcSupportingDifs[1] = "mobile.DIF";
}

RS_TEST_CASE_SETUP(test_SerDesNeighbor)
{
    TEST_ASSERT(!ERR_CHK_MEM(xSerDesNeighborInit(&xNeighborSD)));
}

RS_TEST_CASE_TEARDOWN(test_SerDesNeighbor)
{
}

RS_TEST_CASE(EncDecNeighbor, "[SerDes]")
{
    neighborMessage_t *pxNbMsgEnc, *pxNbMsgDec;
    serObjectValue_t xNbVal;

    RS_TEST_CASE_BEGIN(test_SerDesNeighbor);

    TEST_ASSERT((pxNbMsgEnc = pvRsMemAlloc(sizeof(neighborMessage_t) + sizeof(char *) * 2)));

    xMakeBullshitNeighbor(pxNbMsgEnc);

    TEST_ASSERT(!ERR_CHK(xSerDesNeighborEncode(&xNeighborSD, pxNbMsgEnc, &xNbVal)));
    TEST_ASSERT(pxNbMsgDec = pxSerDesNeighborDecode(&xNeighborSD,
                                                    xNbVal.pvSerBuffer,
                                                    xNbVal.xSerLength));

    TEST_ASSERT(strcmp(pxNbMsgDec->pcApName, pxNbMsgEnc->pcApName) == 0);
    TEST_ASSERT(strcmp(pxNbMsgDec->pcApInstance, pxNbMsgEnc->pcApInstance) == 0);
    TEST_ASSERT(strcmp(pxNbMsgDec->pcAeName, pxNbMsgEnc->pcAeName) == 0);
    TEST_ASSERT(strcmp(pxNbMsgDec->pcAeInstance, pxNbMsgEnc->pcAeInstance) == 0);
    TEST_ASSERT(pxNbMsgDec->unSupportingDifCount == pxNbMsgEnc->unSupportingDifCount);
    TEST_ASSERT(strcmp(pxNbMsgDec->pcSupportingDifs[0], pxNbMsgEnc->pcSupportingDifs[0]) == 0);

    vRsMemFree(pxNbMsgEnc);
    vRsrcFree(pxNbMsgDec);
    vRsrcFree(xNbVal.pvSerBuffer);

    RS_TEST_CASE_END(test_SerDesNeighbor);
}

RS_TEST_CASE(EncDecNeighbors, "[SerDes]")
{
    neighborMessage_t *pxNbMsg[3];
    neighborsMessage_t *pxNbListMsg;
    serObjectValue_t xNbVal;

    RS_TEST_CASE_BEGIN(test_SerDesNeighbor);

    for (int i = 0; i < 3; i++) {
        TEST_ASSERT((pxNbMsg[i] = pvRsMemAlloc(sizeof(neighborMessage_t) + sizeof(char *) * 2)));
        xMakeBullshitNeighbor(pxNbMsg[i]);
    }

    TEST_ASSERT(!ERR_CHK(pxSerDesNeighborListEncode(&xNeighborSD, 3, pxNbMsg, &xNbVal)));
    TEST_ASSERT(pxNbListMsg = pxSerDesNeighborListDecode(&xNeighborSD, xNbVal.pvSerBuffer, xNbVal.xSerLength));
    TEST_ASSERT(pxNbListMsg->unNb == 3);
    TEST_ASSERT(strcmp(pxNbListMsg->pxNeighsMsg[0]->pcApName, pxNbMsg[0]->pcApName) == 0);
    TEST_ASSERT(strcmp(pxNbListMsg->pxNeighsMsg[1]->pcApInstance, pxNbMsg[1]->pcApInstance) == 0);
    TEST_ASSERT(strcmp(pxNbListMsg->pxNeighsMsg[2]->pcAeName, pxNbMsg[2]->pcAeName) == 0);
    TEST_ASSERT(strcmp(pxNbListMsg->pxNeighsMsg[0]->pcAeInstance, pxNbMsg[0]->pcAeInstance) == 0);
    TEST_ASSERT(pxNbListMsg->pxNeighsMsg[1]->unSupportingDifCount == pxNbMsg[0]->unSupportingDifCount);
    TEST_ASSERT(strcmp(pxNbListMsg->pxNeighsMsg[2]->pcSupportingDifs[0],
                       pxNbMsg[2]->pcSupportingDifs[0]) == 0);

    for (int i = 0; i < 3; i++) {
        vRsMemFree(pxNbMsg[i]);
        vRsrcFree(pxNbListMsg->pxNeighsMsg[i]);
    }

    vRsrcFree(pxNbListMsg);
    vRsrcFree(xNbVal.pvSerBuffer);

    RS_TEST_CASE_END(test_SerDesNeighbor);
}

#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(EncDecNeighbor);
    RS_RUN_TEST(EncDecNeighbors);
    RS_SUITE_END();
}
#endif
