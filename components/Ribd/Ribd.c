#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "portability/port.h"
#include "common/error.h"
#include "common/netbuf.h"
#include "common/rinasense_errors.h"
#include "common/rsrc.h"
#include "common/rina_ids.h"
#include "common/rina_name.h"

#include "configRINA.h"
#include "configSensor.h"

#include "ARP826_defs.h"
#include "CdapMessage.h"
#include "private/Ribd_internal.h"
#include "Ribd_connections.h"
#include "RibObject.h"
#include "Ribd_msg.h"
#include "Ribd_defs.h"
#include "SerDes.h"
#include "du.h"
#include "CDAP.pb.h"
#include "Enrollment_api.h"
#include "FlowAllocator_defs.h"
#include "FlowAllocator_api.h"
#include "IPCP_api.h"
#include "IPCP_events.h"
#include "IPCP_normal_api.h"
#include "rina_common_port.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "rmt.h"
#include "RINA_API_flows.h"
#include "SerDesMessage.h"
#include "SerDesAData.h"

ribObject_t xRibConnectObject = {
    .ucObjName = "",
    .ucObjClass = "",
    .ulObjInst = 0,

    .fnStart = NULL,
    .fnStop = NULL,
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

rsErr_t xRibAddPendingResponse(Ribd_t *pxRibd,
                               string_t pcObjName,
                               invokeId_t unInvokeID,
                               opCode_t eReqOpCode,
                               ribResponseCb xRibResCb)
{
    num_t x = 0;

    RsAssert(pxRibd);

    for (x = 0; x < RESPONSE_HANDLER_TABLE_SIZE; x++) {

        if (!pxRibd->xPendingReqs[x].unInvokeID) {
            pxRibd->xPendingReqs[x].pcObjName = pcObjName;
            pxRibd->xPendingReqs[x].unInvokeID = unInvokeID;
            pxRibd->xPendingReqs[x].eReqOpCode = eReqOpCode;
            pxRibd->xPendingReqs[x].xRibResCb = xRibResCb;
            pxRibd->xPendingReqs[x].pxResp = NULL;

            /* Create wait mechanic */
            if (pthread_mutex_init(&pxRibd->xPendingReqs[x].xMutex, NULL) != 0 ||
                pthread_cond_init(&pxRibd->xPendingReqs[x].xWaitCond, NULL) != 0) {

                return FAIL;
            }

            return SUCCESS;
        }
    }

    LOGW(TAG_RIB, "Out of space in response handler table");

    return ERR_RIB_TABLE_FULL;
}

ribPendingReq_t *xRibFindPendingResponse(Ribd_t *pxRibd, invokeId_t unInvokeID)
{
    num_t x = 0;

    RsAssert(pxRibd);

    vRibLock(pxRibd);

    for (x = 0; x < RESPONSE_HANDLER_TABLE_SIZE; x++) {
        if (pxRibd->xPendingReqs[x].unInvokeID == unInvokeID) {
            vRibUnlock(pxRibd);
            return &pxRibd->xPendingReqs[x];
        }
    }

    vRibUnlock(pxRibd);

    return NULL;
}

void xRibFreePendingResponse(ribPendingReq_t *pxPendingReq)
{
    memset(pxPendingReq, 0, sizeof(ribPendingReq_t));
}

#if 0
void prvRibHandleAData(struct ipcpInstanceData_t *pxData, serObjectValue_t *pxObjValue)
{
    messageCdap_t *pxDecodeCdap;
    ribObject_t *pxRibObject;
    ribCallbackOps_t *pxCallback;

    pxDecodeCdap = pxSerDesMessageDecode(&pxData->xRibd.xMsgSD,
                                         pxObjValue->pvSerBuffer, pxObjValue->xSerLength);
    if (!pxDecodeCdap) {
        LOGE(TAG_RIB, "Failed to decode CDAP message");
        return;
    }

    vRibPrintCdapMessage(pxDecodeCdap);

    if (pxDecodeCdap->eOpCode > MAX_CDAP_OPCODE) {
        LOGE(TAG_RIB, "Invalid opcode %s", opcodeNamesTable[pxDecodeCdap->eOpCode]);
        vRsrcFree(pxDecodeCdap);
    }

    LOGI(TAG_RIB, "Handling CDAP Message: %s", opcodeNamesTable[pxDecodeCdap->eOpCode]);

    pxRibObject = pxRibFindObject(&pxData->xRibd, pxDecodeCdap->pcObjName);

    switch (pxDecodeCdap->eOpCode) {
    case M_CREATE_R:
        pxCallback = prvRibFindPendingResponse(&pxData->xRibd, pxDecodeCdap->nInvokeID);
        if (!pxCallback) {
            LOGE(TAG_RIB, "Failed to find proper response handler");
            break;
        }

        if (!pxCallback->create_response(pxData, pxDecodeCdap->pxObjValue, pxDecodeCdap->result))
            LOGE(TAG_RIB, "It was not possible to handle the a_data message properly");

        break;

    case M_DELETE:
        if (pxRibObject->fnDelete)
            pxRibObject->fnDelete(pxData, pxRibObject, NULL,
                                  &pxDecodeCdap->xDestinationInfo,
                                  &pxDecodeCdap->xSourceInfo,
                                  pxDecodeCdap->nInvokeID,
                                  PORT_ID_WRONG);
        else
            LOGW(TAG_RIB, "Unsupported method M_DELETE on object %s", pxRibObject->ucObjName);

        break;
    default:

        break;
    }
}
#endif

static rsErr_t prvRibHandleConnect(Ribd_t *pxRibd,
                                   portId_t unPort,
                                   appConnection_t *pxAppCon,
                                   messageCdap_t *pxMsg)
{
    messageCdap_t *pxMsgCdap = NULL;
    rsErr_t xStatus;

    /* If App Connection is not registered, create a new one */
    if (pxAppCon == NULL) {
        if (ERR_CHK(xRibConnectionAdd(pxRibd, &pxMsg->xSourceInfo, &pxMsg->xDestinationInfo, unPort)))
            return FAIL;

        vRibLock(pxRibd);

        pxMsgCdap = pxRibCdapMsgCreateResponse(pxRibd,
                                               "",
                                               "",
                                               -1,
                                               M_CONNECT_R,
                                               NULL,
                                               0,
                                               NULL,
                                               pxMsg->nInvokeID);

        xStatus = pxRibd->fnRibOutput(pxRibd, pxMsgCdap, unPort);

        vRibUnlock(pxRibd);

        vRibCdapMsgFree(pxMsgCdap);

        return xStatus;
    }
    else
        /* Already connected or already known? */
        return ERR_SET(ERR_RIB_CONNECTION_EXISTS);
}

static rsErr_t prvRibHandleConnectReply(Ribd_t *pxRibd,
                                         portId_t unPort,
                                         appConnection_t *pxAppCon,
                                         messageCdap_t *pxMsg)
{
    if (!pxAppCon)
        return ERR_SET(ERR_RIB_NO_SUCH_CONNECTION);

    /* Check if the current AppConnection status is in progress */
    if (pxAppCon->xStatus != CONNECTION_IN_PROGRESS)
        return ERR_SET(ERR_RIB_BAD_CONNECTION_STATE);

    /* Update Connection Status */
    pxAppCon->xStatus = CONNECTION_OK;

    LOGI(TAG_RIB, "Application Connection Status Updated to 'CONNECTED'");

    return SUCCESS;
}

static rsErr_t prvRibHandleRelease(Ribd_t *pxRibd,
                                    portId_t unPort,
                                    appConnection_t *pxAppCon,
                                    messageCdap_t *pxMsg)
{
    if (!pxAppCon)
        return ERR_SET(ERR_RIB_NO_SUCH_CONNECTION);
    if (pxAppCon->xStatus == CONNECTION_RELEASED)
        return ERR_SET(ERR_RIB_BAD_CONNECTION_STATE);

    /* Send a release reply if needed */
    if (pxMsg->nInvokeID != 0)
        return xRibRELEASE_REPLY(pxRibd, &xRibConnectObject, unPort, pxMsg->nInvokeID);
    else {
        /* Otherwise just release */
        vRibConnectionRelease(pxRibd, pxAppCon);

        LOGI(TAG_RIB, "Connection to %s was released", pxAppCon->xDestinationInfo.pcProcessName);
    }

    return SUCCESS;
}

static rsErr_t prvRibHandleReleaseReply(Ribd_t *pxRibd,
                                         portId_t unPort,
                                         appConnection_t *pxAppCon,
                                         messageCdap_t *pxMsg)
{
    if (!pxAppCon)
        return ERR_SET(ERR_RIB_NO_SUCH_CONNECTION);
    if (pxAppCon->xStatus != CONNECTION_RELEASED)
        return ERR_SET(ERR_RIB_BAD_CONNECTION_STATE);

    /* Weird? */
    if (pxMsg->result != 0) {
        if (pxMsg->pcResultReason != NULL)
            LOGW(TAG_RIB, "M_RELEASE_R result code is %d, result reason: %s",
                 pxMsg->result, pxMsg->pcResultReason);
        else
            LOGW(TAG_RIB, "M_RELEASE result code is %d", pxMsg->result);
    }

    /*Update Connection Status*/
    pxAppCon->xStatus = CONNECTION_RELEASED;

    return SUCCESS;
}

static rsErr_t prvRibHandleAnyReply(Ribd_t *pxRibd,
                                    portId_t unPort,
                                    appConnection_t *pxAppCon,
                                    messageCdap_t *pxMsg)
{
    ribObjectResMethod xRibObjResCb;
    ribPendingReq_t *pxPendingReq;
    ribObject_t *pxRibObj;
    rsErr_t xErr;
    void *pxResp;

    /* There is if there is at least something to handle this
     * response */
    if (!(pxPendingReq = xRibFindPendingResponse(pxRibd, pxMsg->nInvokeID)))
        return ERR_SETF(ERR_RIB_NOT_PENDING, pxMsg->nInvokeID);

    /* Looking for the object into the RIB */
    if (!(pxRibObj = pxRibObjectFind(pxRibd, pxPendingReq->pcObjName)))
        return ERR_SETF(ERR_RIB_OBJECT_UNSUPPORTED, pxMsg->pcObjName);

    /* Then check that the object can handle a response. */
    if ((xRibObjResCb = xRibGetObjectReplyMethod(pxRibObj, pxMsg->eOpCode))) {
        xErr = xRibObjResCb(pxRibObj, pxAppCon, pxMsg, &pxResp);

        /* Error dealing with the reply */
        if (ERR_CHK(xErr)) return FAIL;
        else {
            /* Deliver the reply */
            if (pxPendingReq->xRibResCb)
                return pxPendingReq->xRibResCb(pxRibd, pxRibObj, pxResp);
            else {
                pxPendingReq->pxResp = pxResp;
                pthread_cond_signal(&pxPendingReq->xWaitCond);
            }
        }
    }
    else return ERR_SETF(ERR_RIB_OBJECT_UNSUP_METHOD, opcodeNamesTable[pxMsg->eOpCode], pxRibObj->ucObjName);

    return SUCCESS;
}

static rsErr_t prvRibHandleAnyRequest(Ribd_t *pxRibd,
                                       portId_t unPort,
                                       appConnection_t *pxAppCon,
                                       messageCdap_t *pxMsg)
{
    ribObjectReqMethod xRibObjCb;
    ribObject_t *pxRibObj;
    rsErr_t xErr;

    /* Looking for the object into the RIB */
    if (!(pxRibObj = pxRibObjectFind(pxRibd, pxMsg->pcObjName)))
        return ERR_SETF(ERR_RIB_OBJECT_UNSUPPORTED, pxMsg->pcObjName);

    if ((xRibObjCb = xRibGetObjectMethod(pxRibObj, pxMsg->eOpCode)))
        xErr = xRibObjCb(pxRibObj, pxAppCon, pxMsg);
    else
        return ERR_SETF(ERR_RIB_OBJECT_UNSUP_METHOD, opcodeNamesTable[pxMsg->eOpCode], pxRibObj->ucObjName);

    return xErr;
}

rsErr_t xRibNormalIncoming(Ribd_t *pxRibd,
                           messageCdap_t *pxDecodeCdap,
                           portId_t unPort)
{
    bool_t ret = true;
    appConnection_t *pxAppCon;
    rsErr_t xStatus;

    /*Check if the Operation Code is valid*/
    if (pxDecodeCdap->eOpCode > MAX_CDAP_OPCODE) {
        LOGE(TAG_RIB, "Invalid opcode %s", opcodeNamesTable[pxDecodeCdap->eOpCode]);
        vRsMemFree(pxDecodeCdap);
        return SUCCESS;
    }

    LOGI(TAG_RIB, "Handling CDAP Message: %s", opcodeNamesTable[pxDecodeCdap->eOpCode]);

    /* Looking for an App Connection using the N-1 Flow Port */
    pxAppCon = pxRibConnectionFind(pxRibd, unPort);

    switch (pxDecodeCdap->eOpCode) {
    case M_CONNECT:
        xStatus = prvRibHandleConnect(pxRibd, unPort, pxAppCon, pxDecodeCdap);
        break;

    case M_CONNECT_R:
        xStatus = prvRibHandleConnectReply(pxRibd, unPort, pxAppCon, pxDecodeCdap);
        break;

    case M_RELEASE:
        xStatus = prvRibHandleRelease(pxRibd, unPort, pxAppCon, pxDecodeCdap);
        break;

    case M_RELEASE_R:
        xStatus = prvRibHandleReleaseReply(pxRibd, unPort, pxAppCon, pxDecodeCdap);
        break;

    case M_CREATE:
    case M_DELETE:
    case M_READ:
    case M_WRITE:
    case M_START:
    case M_STOP:
        xStatus = prvRibHandleAnyRequest(pxRibd, unPort, pxAppCon, pxDecodeCdap);
        break;

    case M_CREATE_R:
    case M_DELETE_R:
    case M_READ_R:
    case M_WRITE_R:
    case M_START_R:
    case M_STOP_R:
        xStatus = prvRibHandleAnyReply(pxRibd, unPort, pxAppCon, pxDecodeCdap);
        break;

    default:
        LOGE(TAG_RIB, "Unhandled CDAP message type: %s", opcodeNamesTable[pxDecodeCdap->eOpCode]);
        ret = false;
        break;
    }

    /* Stop error propagation here. */
    if (ERR_CHK(xStatus)) {
        vErrorLog(TAG_RIB, "RIB CDAP handling");
        vErrorClear();
    }

    return SUCCESS;
}

rsErr_t prvRibNormalOutgoing(Ribd_t *pxRibd,
                             messageCdap_t *pxOutgoingCdap,
                             portId_t unPort)
{
    serObjectValue_t *pxSerVal;
    RINAStackEvent_t xEv;
    rsErr_t xStatus;
    du_t *pxDu;

    LOGI(TAG_RIB, "Sending the CDAP Message to the RMT");

#ifndef NDEBUG
    vRibPrintCdapMessage(TAG_RIB, "ENCODE", pxOutgoingCdap);
#endif

    if (!(pxSerVal = pxSerDesMessageEncode(&pxRibd->xMsgSD, pxOutgoingCdap)))
        return FAIL;

    pxDu = pxNetBufNew(pxRibd->xDuPool, NB_RINA_DATA,
                       pxSerVal->pvSerBuffer, pxSerVal->xSerLength,
                       NETBUF_FREE_POOL);
    if (!pxDu)
        goto fail;

    xEv.eEventType = eSendMgmtEvent;
    xEv.xData.UN = unPort;
    xEv.xData2.DU = pxDu;

    if (!xSendEventStructToIPCPTask(&xEv, 1000)) {
        LOGE(TAG_RIB, "Failed to send management PDU to IPCP");
        goto fail;
    }

    vRsrcFree(pxSerVal);

    return SUCCESS;

    fail:
    if (pxSerVal)
        vRsrcFree(pxSerVal);
    if (pxDu)
        vNetBufFree(pxDu);

    return FAIL;
}

/* Called by the lower layers of the stack to send a CDAP messages to
 * the RIB. */
rsErr_t xRibIncoming(Ribd_t *pxRibd, du_t *pxDu, portId_t unPort)
{
    messageCdap_t *pxIncomingCdap;
    rsErr_t xStatus;

    LOGI(TAG_RIB, "Processing a Management PDU");

    RsAssert(eNetBufType(pxDu) == NB_RINA_DATA);

    /* Decode CDAP Message */
    pxIncomingCdap = pxSerDesMessageDecode(&pxRibd->xMsgSD,
                                           pvNetBufPtr(pxDu),
                                           unNetBufSize(pxDu));
    if (!pxIncomingCdap) {
        LOGE(TAG_RIB, "Failed to decode management PDU");
        return FAIL;
    }

    xStatus = pxRibd->fnRibInput(pxRibd, pxIncomingCdap, unPort);

#ifndef NDEBUG
    vRibPrintCdapMessage(TAG_RIB, "DECODE", pxIncomingCdap);
#endif

    vRsrcFree(pxIncomingCdap);

    return xStatus;
}

rsErr_t xRibCommonInit(Ribd_t *pxRibd)
{
    if (!xSerDesMessageInit(&pxRibd->xMsgSD) ||
        !xSerDesADataInit(&pxRibd->xADataSD))
        return ERR_SET_OOM;

    /* Pool for CDAP message content */
    if (!(pxRibd->xMsgPool = pxRsrcNewVarPool("RIB CDAP message pool", 0)))
        return ERR_SET_OOM;

    /* Pool for DU object containing CDAP data */
    if (!(pxRibd->xDuPool = xNetBufNewPool("RIBD DU pool")))
        return ERR_SET_OOM;

    if (pthread_mutex_init(&pxRibd->xMutex, NULL) != 0)
        return FAIL;

    return SUCCESS;
}

rsErr_t xRibNormalInit(Ribd_t *pxRibd)
{
    size_t unSz;

    pxRibd->fnRibInput = &xRibNormalIncoming;
    pxRibd->fnRibOutput = &prvRibNormalOutgoing;
    pxRibd->xDoLock = true;

    return xRibCommonInit(pxRibd);
}
