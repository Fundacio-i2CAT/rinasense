#include <errno.h>

#include "portability/port.h"

#include "common/error.h"
#include "common/rinasense_errors.h"

#include "private/Ribd_internal.h"
#include "Ribd_requests.h"
#include "Ribd_defs.h"
#include "Ribd_msg.h"
#include "Ribd_api.h"

ribObjectResMethod xRibGetObjectReplyMethod(ribObject_t *pxRibObj, opCode_t eOpCode)
{
    switch (eOpCode) {
    case M_CREATE_R: return pxRibObj->fnCreateReply;
    case M_DELETE_R: return pxRibObj->fnDeleteReply;
    case M_READ_R:   return pxRibObj->fnReadReply;
    case M_WRITE_R:  return pxRibObj->fnWriteReply;
    case M_START_R:  return pxRibObj->fnStartReply;
    case M_STOP_R:   return pxRibObj->fnStopReply;

    default:
        /* This function should not be called to get a top of reply
         * callback that cannot exists in a RIB object. */
        abort();
    }
}

ribObjectReqMethod xRibGetObjectMethod(ribObject_t *pxRibObj, opCode_t eOpCode)
{
    switch (eOpCode) {
    case M_CREATE: return pxRibObj->fnCreate;
    case M_DELETE: return pxRibObj->fnDelete;
    case M_READ:   return pxRibObj->fnRead;
    case M_WRITE:  return pxRibObj->fnWrite;
    case M_START:  return pxRibObj->fnStart;
    case M_STOP:   return pxRibObj->fnStop;

    default:
        /* This function should not be called to get a type of
         * callback that cannot exists in a RIB object */
        abort();
    }
}

rsErr_t xRibObjectErr(Ribd_t *pxRibd,
                      ribObject_t *pxRibObj,
                      opCode_t eOpCode,
                      portId_t unPort,
                      invokeId_t unInvokeId,
                      int nResultCode,
                      string_t pcResultReason)
{
    messageCdap_t *pxMsgCdap = NULL;
    serObjectValue_t *pxSerVal;
    rsErr_t xStatus;

    vRibLock(pxRibd);

    pxMsgCdap = pxRibCdapMsgCreateResponse(pxRibd,
                                           pxRibObj->ucObjClass,
                                           pxRibObj->ucObjName,
                                           pxRibObj->ulObjInst,
                                           eOpCode,
                                           NULL,
                                           nResultCode,
                                           pcResultReason,
                                           unInvokeId);

    xStatus = pxRibd->fnRibOutput(pxRibd, pxMsgCdap, unPort);

    vRibUnlock(pxRibd);

    vRibCdapMsgFree(pxMsgCdap);

    return xStatus;
}

rsErr_t xRibObjectQuery(Ribd_t *pxRibd,
                        ribObject_t *pxRibObj,
                        opCode_t eOpCode,
                        portId_t unPort,
                        serObjectValue_t *pxObjVal,
                        ribQueryTypeInfo_t xRibQueryTypeInfo)
{
    messageCdap_t *pxMsgCdap = NULL;
    serObjectValue_t *pxSerVal;
    ribResponseCb fnCb;
    rsErr_t xStatus;

    vRibLock(pxRibd);

    pxMsgCdap = pxRibCdapMsgCreateRequest(pxRibd,
                                          pxRibObj->ucObjClass,
                                          pxRibObj->ucObjName,
                                          pxRibObj->ulObjInst,
                                          eOpCode,
                                          pxObjVal);

    /* If the caller wants to save the invoke ID, it means the reply
     * needs to be tracked */
    if (xRibQueryTypeInfo.eType == RIB_QUERY_TYPE_ASYNC ||
        xRibQueryTypeInfo.eType == RIB_QUERY_TYPE_SYNC) {

        /* For async requests, save the callback in the table. */
        if (xRibQueryTypeInfo.eType == RIB_QUERY_TYPE_ASYNC)
            fnCb = xRibQueryTypeInfo.fnCb;
        else
            fnCb = NULL;

        /* For sync requests, send the invokeID back to the caller */
        if (xRibQueryTypeInfo.eType == RIB_QUERY_TYPE_SYNC)
            *(xRibQueryTypeInfo.pxInvokeId) = pxMsgCdap->nInvokeID;

        if (ERR_CHK((xStatus = xRibAddPendingResponse(pxRibd,
                                                      pxRibObj->ucObjName,
                                                      pxMsgCdap->nInvokeID,
                                                      eOpCode,
                                                      fnCb))))
            goto fail;
    }
    /* RIB_QUERY_TYPE_CMD: we're not expecting a reply */
    else pxMsgCdap->nInvokeID = 0;

    xStatus = pxRibd->fnRibOutput(pxRibd, pxMsgCdap, unPort);

    fail:
    vRibCdapMsgFree(pxMsgCdap);

    vRibUnlock(pxRibd);

    return xStatus;
}

rsErr_t xRibObjectReply(Ribd_t *pxRibd,
                        ribObject_t *pxRibObj,
                        opCode_t eOpCode,
                        portId_t unPort,
                        invokeId_t unInvokeId,
                        int nResultCode,
                        string_t pcResultReason,
                        serObjectValue_t *pxObjVal)
{
    messageCdap_t *pxMsgCdap = NULL;
    rsErr_t xStatus = FAIL;

    vRibLock(pxRibd);

    pxMsgCdap = pxRibCdapMsgCreateResponse(pxRibd,
                                           pxRibObj->ucObjClass,
                                           pxRibObj->ucObjName,
                                           pxRibObj->ulObjInst,
                                           eOpCode,
                                           pxObjVal,
                                           nResultCode,
                                           pcResultReason,
                                           unInvokeId);

    xStatus = pxRibd->fnRibOutput(pxRibd, pxMsgCdap, unPort);

    vRibCdapMsgFree(pxMsgCdap);

    vRibUnlock(pxRibd);

    return xStatus;
}

rsErr_t xRibObjectWaitReply(Ribd_t *pxRibd,
                            invokeId_t unInvokeId,
                            useconds_t pxTimeout,
                            void **pxResp)
{
    ribPendingReq_t *pxPendingReq;
    struct timespec xTs = {0};
    int n;

    RsAssert(pxResp);

    if (!rstime_waitusec(&xTs, pxTimeout))
        return ERR_SET_ERRNO;

    if (!(pxPendingReq = xRibFindPendingResponse(pxRibd, unInvokeId)))
        return ERR_SET(ERR_RIB_NOT_PENDING);

    /* If the response is already available (could be!) */
    if (pxPendingReq->pxResp) {
        *pxResp = pxPendingReq->pxResp;
        return SUCCESS;
    }

    /* Wait for the answer to arrive */

    pthread_mutex_lock(&pxPendingReq->xMutex);

    n = pthread_cond_timedwait(&pxPendingReq->xWaitCond, &pxPendingReq->xMutex, &xTs);

    if (n == ETIMEDOUT)
        return ERR_SET(ERR_RIB_TIMEOUT);

    pthread_mutex_unlock(&pxPendingReq->xMutex);

    *pxResp = pxPendingReq->pxResp;

    return SUCCESS;
}
