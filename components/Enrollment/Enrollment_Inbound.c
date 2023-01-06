#include "portability/port.h"

#include "common/error.h"
#include "common/rina_ids.h"

#include "common/rinasense_errors.h"
#include "configRINA.h"
#include "configSensor.h"

#include "Enrollment_api.h"
#include "Enrollment_obj.h"
#include "SerDes.h"
#include "SerDesEnrollment.h"
#include "Ribd_api.h"
#include "private/Enrollment_Object.h"

void *xEnrollmentInboundProcess(void *pvArg) {
    ribObject_t *pxThis;
    enrollmentObjectData_t *pxObjData;
    invokeId_t unStop;

    pxThis = (ribObject_t *)pvArg;
    pxObjData = (enrollmentObjectData_t *)pxThis->pvPriv;

    LOGI(TAG_ENROLLMENT, "STARTING: Inbound Enrollment Process");

    { /* START reply on enrollment object */
        serObjectValue_t *pxStartReplyVal;
        enrollmentMessage_t xEnrollStartReply = {0};

        xEnrollStartReply.ullAddress = LOCAL_ADDRESS;

        pxStartReplyVal = pxSerDesEnrollmentEncode(&pxObjData->pxEnrollment->xEnrollmentSD,
                                                   &xEnrollStartReply);

        /* Send the START_R request */
        if (ERR_CHK(xRibSTART_REPLY(pxObjData->pxRibd,
                                    pxObjData->pxThis,
                                    pxObjData->unPort,
                                    pxObjData->unStartInvokeId,
                                    pxStartReplyVal))) {
            vRsrcFree(pxStartReplyVal);
            goto fail;
        }

        vRsrcFree(pxStartReplyVal);
    }

    { /* CREATE on remote neighbor object */
        neighborMessage_t *pxNeighMsgs[1];
        serObjectValue_t *pxNeighObj;

        if (!(pxNeighMsgs[0] = pvRsMemAlloc(sizeof(neighborMessage_t) + sizeof(char *))))
            return false;

        pxNeighMsgs[0]->pcApName = LOCAL_ADDRESS_AP_NAME;
        pxNeighMsgs[0]->pcApInstance = "";
        pxNeighMsgs[0]->pcAeName = "";
        pxNeighMsgs[0]->pcAeInstance = "";
        pxNeighMsgs[0]->ullAddress = LOCAL_ADDRESS;
        pxNeighMsgs[0]->unSupportingDifCount = 1;
        pxNeighMsgs[0]->pcSupportingDifs[0] = SHIM_DIF_NAME;

        pxNeighObj = pxSerDesNeighborListEncode(&pxObjData->pxEnrollment->xNeighborSD, 1, pxNeighMsgs);

        vRsMemFree(pxNeighMsgs[0]);

        /* Send a CREATE command creating the local neighbors in the
         * caller. */
        if (ERR_CHK(xRibCREATE_CMD(pxObjData->pxRibd,
                                   &xEnrollmentNeighborsObject,
                                   pxObjData->unPort,
                                   pxNeighObj))) {
            vRsrcFree(pxNeighObj);
            goto fail;
        }

        vRsrcFree(pxNeighObj);
    }

    { /* STOP request on enrollment object */
        serObjectValue_t *pxStopObjVal;
        enrollmentMessage_t xEnrollStop;
        rsErr_t xStatus;
        int nStopReply;

        xEnrollStop.xStartEarly = 0;

        pxStopObjVal = pxSerDesEnrollmentEncode(&pxObjData->pxEnrollment->xEnrollmentSD, &xEnrollStop);

        /* Send a STOP request on the enrollment object, expect a reply
         * within a certain delay */
        if (ERR_CHK(xRibSTOP_QUERY_SYNC(pxObjData->pxRibd,
                                        &xEnrollmentRibObject,
                                        pxObjData->unPort,
                                        pxStopObjVal,
                                        &unStop))) {
            vRsrcFree(pxStopObjVal);
            goto fail;
        }

        vRsrcFree(pxStopObjVal);

        /* Wait for a reply to the stop command. */
        xStatus = xRibObjectWaitReply(pxObjData->pxRibd, unStop, 5 * 1000000, (void *)&nStopReply);

        /* Check for timeout. */
        if (ERR_IS(xStatus, ERR_RIB_TIMEOUT)) {
            LOGW(TAG_ENROLLMENT, "Timeout");
            ERR_SET(ERR_ENROLLMENT_INBOUND_FAIL);
            goto fail;
        }

        if (nStopReply != 1) {
            LOGW(TAG_ENROLLMENT, "Unexpected reply to STOP: %d", nStopReply);

            ERR_SET(ERR_ENROLLMENT_INBOUND_FAIL);
            goto fail;
        }
    }

    { /* Send a START command on the operational status object */
        if (ERR_CHK(xRibSTART_CMD(pxObjData->pxRibd, &xOperationalStatus, pxObjData->unPort, NULL)))
            goto fail;
    }

    /* FIXME: Wait for START_R for a certain time...maybe? */

    LOGI(TAG_ENROLLMENT, "FINISHING: Inbound Enrollment Process");

    return NULL;

    fail:

    { rsErrInfo_t *pxErrInfo = NULL;

        if (xErrorGet()) {
            vErrorLog(TAG_ENROLLMENT, "Inbound Enrollment Process");
            vErrorClear();
        }
        else LOGE(TAG_ENROLLMENT, "Inbound enrollment process FAILED");
    }

    return NULL;
}
