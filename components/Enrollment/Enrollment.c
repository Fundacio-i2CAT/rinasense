#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "portability/port.h"
#include "common/error.h"
#include "common/rinasense_errors.h"
#include "common/rina_ids.h"
#include "common/rsrc.h"
#include "common/rina_name.h"

#include "configSensor.h"
#include "configRINA.h"

#include "rina_common_port.h"
#include "RibObject.h"
#include "Ribd_defs.h"
#include "Ribd_api.h"
#include "Ribd_msg.h"
#include "Enrollment_api.h"
#include "EnrollmentInformationMessage.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "IPCP_normal_defs.h"
#include "IPCP_api.h"
#include "SerDes.h"
#include "SerDesEnrollment.h"
#include "SerDesNeighbor.h"
#include "SerDesMessage.h"
#include "private/Enrollment_Object.h"

static DECLARE_RIB_OBJECT_REQUEST_METHOD(prvEnrollmentObjectStart);
static DECLARE_RIB_OBJECT_REQUEST_METHOD(prvEnrollmentObjectStop);
static DECLARE_RIB_OBJECT_REPLY_METHOD(prvEnrollmentObjectStopReply);
static DECLARE_RIB_OBJECT_INIT_METHOD(prvEnrollmentObjectInit);

ribObject_t xEnrollmentRibObject = {
    .ucObjName = "/difm/enr",
    .ucObjClass = "Enrollment",
    .ulObjInst = 0,

    .fnStart = &prvEnrollmentObjectStart,
    .fnStop = &prvEnrollmentObjectStop,
    .fnCreate = NULL,
    .fnDelete = NULL,
    .fnWrite = NULL,
    .fnRead = NULL,

    .fnStartReply = NULL,
    .fnStopReply = &prvEnrollmentObjectStopReply,
    .fnCreateReply = NULL,
    .fnDeleteReply = NULL,
    .fnWriteReply = NULL,
    .fnReadReply = NULL,

    .fnShow = NULL,

    .fnInit = &prvEnrollmentObjectInit,
    .fnFree = NULL
};

ribObject_t xEnrollmentNeighborsObject = {
    .ucObjName = "/difm/enr/neighs",
    .ucObjClass = "Neighbors",
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

static DECLARE_RIB_OBJECT_REQUEST_METHOD(prvEnrollmentHandleOperationalStart);

ribObject_t xOperationalStatus = {
    .ucObjName = "/difm/ops",
    .ucObjClass = "OperationalStatus",
    .ulObjInst = 1,

    .fnStart = &prvEnrollmentHandleOperationalStart,
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

rsErr_t prvEnrollmentObjectInit(Ribd_t *pxRibd, void *pvOwner, ribObject_t *pxThis) {
    enrollmentObjectData_t *pxEnrollmentObjectData;
    Enrollment_t *pxEnrollment;

    if (!(pxThis->pvPriv = pvRsMemAlloc(sizeof(enrollmentObjectData_t))))
        return ERR_OOM;

    /* This initialize the data that will be accessible to the
     * enrollment process threads */

    pxEnrollmentObjectData = (enrollmentObjectData_t *)pxThis->pvPriv;
    pxEnrollment = (Enrollment_t *)pvOwner;

    pxEnrollmentObjectData->pxRibd = pxRibd;
    pxEnrollmentObjectData->pxEnrollment = pxEnrollment;
    pxEnrollmentObjectData->pxThis = pxThis;

    return SUCCESS;
}

rsErr_t prvEnrollmentObjectStart(ribObject_t *pxThis, appConnection_t *pxAppCon, messageCdap_t *pxMsg)
{
    enrollmentObjectData_t *pxEnrollmentData;

    pxEnrollmentData = (enrollmentObjectData_t *)pxThis->pvPriv;

    pxEnrollmentData->unPort = pxAppCon->unPort;
    pxEnrollmentData->unStartInvokeId = pxMsg->nInvokeID;

    if (pthread_create(&pxEnrollmentData->xEnrollmentThread,
                       NULL,
                       &xEnrollmentInboundProcess,
                       pxThis) != 0) {
        return FAIL;
    }

    return SUCCESS;
}

rsErr_t prvEnrollmentObjectStop(ribObject_t *pxThis,
                                appConnection_t *pxAppCon,
                                messageCdap_t *pxMsg)
{
    return FAIL;
}

rsErr_t prvEnrollmentObjectStopReply(ribObject_t *pxThis,
                                     appConnection_t *pxAppCon,
                                     messageCdap_t *pxMsg,
                                     void **ppxResp)
{
    neighborInfo_t *pxNeighborInfo;
    Enrollment_t *pxEnrollment;
    enrollmentObjectData_t *pxObjData;

    /* There is enough space here to cram an int but that's bad
     * bad... */
    *((int *)ppxResp) = 1;

    return SUCCESS;
}

rsErr_t prvEnrollmentHandleOperationalStart(ribObject_t *pxThis,
                                            appConnection_t *pxAppCon,
                                            messageCdap_t *pxMsg)
{
    neighborMessage_t *pxNeighborMsg;
    neighborInfo_t *pxNeighborInfo;
    NeighborSerDes_t *pxNeighborSD;
    serObjectValue_t xSerObjValue;
    bool_t xStatus;

    LOGE(TAG_ENROLLMENT, "Handling OperationalStart");

    pxNeighborInfo = pxEnrollmentFindNeighbor((Enrollment_t *)pxThis->pvOwner,
                                              pxAppCon->xDestinationInfo.pcProcessName);

    if (!pxNeighborInfo)
        return ERR_RIB_NEIGHBOR_NOT_FOUND;

    /* FIXME: NOT FREED */
    pxNeighborMsg = pvRsMemAlloc(sizeof(*pxNeighborMsg));

    pxNeighborMsg->pcApName = NORMAL_PROCESS_NAME;
    pxNeighborMsg->pcApInstance = NORMAL_PROCESS_INSTANCE;
    pxNeighborMsg->pcAeName = MANAGEMENT_AE;
    pxNeighborMsg->pcAeInstance = pxNeighborInfo->pcToken;

    pxNeighborSD = &((Enrollment_t *)pxThis->pvOwner)->xNeighborSD;

    if (ERR_CHK(xSerDesNeighborEncode(pxNeighborSD, pxNeighborMsg, &xSerObjValue)))
        return FAIL;

    return xRibObjectReply(pxThis->pxRibd, pxThis, M_START_R, pxAppCon->unPort, pxMsg->nInvokeID, 0, NULL, NULL);
}

/* EnrollmentInit should create neighbor and enrollment object into the RIB*/
rsErr_t xEnrollmentInit(Enrollment_t *pxEnrollment, Ribd_t *pxRibd)
{
    rsErr_t xErr;

    if (ERR_CHK_MEM(xSerDesEnrollmentInit(&pxEnrollment->xEnrollmentSD)))
        return ERR_SET_OOM;

    if (ERR_CHK_MEM(xSerDesNeighborInit(&pxEnrollment->xNeighborSD)))
        return ERR_SET_OOM;

    if (ERR_CHK((xErr = xRibObjectAdd(pxRibd, pxEnrollment, &xEnrollmentRibObject))))
        return xErr;

    if (ERR_CHK((xErr = xRibObjectAdd(pxRibd, pxEnrollment, &xOperationalStatus))))
        return xErr;

    if (ERR_CHK((xErr = xRibObjectAdd(pxRibd, pxEnrollment, &xEnrollmentNeighborsObject))))
        return xErr;

    pthread_mutex_init(&pxEnrollment->xNeighborMutex, NULL);

    return SUCCESS;
}
