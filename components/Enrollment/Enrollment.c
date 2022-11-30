/*Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SerDesNeighbor.h"
#include "common/rina_ids.h"
#include "common/rsrc.h"
#include "portability/port.h"
#include "common/rina_name.h"

#include "configSensor.h"
#include "configRINA.h"

#include "rina_common_port.h"
#include "Rib.h"
#include "Ribd.h"
#include "Ribd_api.h"
#include "Enrollment_api.h"
#include "EnrollmentInformationMessage.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "IPCP_normal_defs.h"
#include "IPCP_api.h"
#include "SerDes.h"
#include "SerDesEnrollment.h"

/* REVIEWED CODE */

/*EnrollmentInit should create neighbor and enrollment object into the RIB*/
bool_t xEnrollmentInit(Enrollment_t *pxEnrollment, Ribd_t *pxRibd)
{
    rname_t xSource;
    rname_t xDestInfo;
    authPolicy_t xAuth;

    if (!xSerDesEnrollmentInit(&pxEnrollment->xEnrollmentSD)) {
        LOGE(TAG_ENROLLMENT, "Failed to allocate pool for enrollment SerDes");
        return false;
    }

    if (!xSerDesNeighborInit(&pxEnrollment->xNeighborSD)) {
        LOGE(TAG_ENROLLMENT, "Failed to allocate pool for neighbor SerDes");
        return false;
    }

    /*Creating Enrollment object into the RIB*/
    pxRibCreateObject(pxRibd, "/difm/enr", 0, "enrollment", "enrollment", ENROLLMENT);

    /*Creating Operational Status into the the RIB*/
    pxRibCreateObject(pxRibd, "/difm/ops", 1, "OperationalStatus", "OperationalStatus", OPERATIONAL);

    return true;
}


neighborInfo_t *pxEnrollmentFindNeighbor(Enrollment_t *pxEnrollment, string_t pcRemoteApName)
{
    num_t x = 0;
    neighborInfo_t *pxNeighbor;

    LOGI(TAG_ENROLLMENT, "Looking for '%s'", pcRemoteApName);

    for (x = 0; x < NEIGHBOR_TABLE_SIZE; x++) {
        if (pxEnrollment->xNeighborsTable[x].xValid == true) {
            pxNeighbor = pxEnrollment->xNeighborsTable[x].pxNeighborInfo;

            LOGI(TAG_ENROLLMENT, "Neighbor checking '%p'", pxNeighbor);
            LOGI(TAG_ENROLLMENT, "Comparing '%s' - '%s'", pxNeighbor->pcApName, pcRemoteApName);

            if (!strcmp(pxNeighbor->pcApName, pcRemoteApName)) {
                LOGI(TAG_ENROLLMENT, "Neighbor '%p' found", pxNeighbor);

                return pxNeighbor;
                break;
            }
        }
    }

    LOGW(TAG_ENROLLMENT, "Neighbor '%s' not found", pcRemoteApName);

    return NULL;
}

bool_t xEnrollmentAddNeighborEntry(Enrollment_t *pxEnrollment, neighborInfo_t *pxNeighbor)
{
    num_t x = 0;

    for (x = 0; x < NEIGHBOR_TABLE_SIZE; x++) {

        if (pxEnrollment->xNeighborsTable[x].xValid == false) {
            pxEnrollment->xNeighborsTable[x].pxNeighborInfo = pxNeighbor;
            pxEnrollment->xNeighborsTable[x].xValid = true;

            return true;
        }
    }

    return false;
}

neighborInfo_t *pxEnrollmentCreateNeighInfo(Enrollment_t *pxEnrollment, string_t pcApName, portId_t xN1Port)
{
    neighborInfo_t *pxNeighInfo;

    if (!(pxNeighInfo = pvRsMemAlloc(sizeof(*pxNeighInfo)))) {
        LOGE(TAG_ENROLLMENT, "Failed to allocate memory for neighbor information");
        return NULL;
    }

    pxNeighInfo->pcApName = strdup(pcApName);
    pxNeighInfo->eEnrollmentState = eENROLLMENT_NONE;
    pxNeighInfo->xNeighborAddress = -1;
    pxNeighInfo->xN1Port = xN1Port;
    pxNeighInfo->pcToken = NULL;

    if (!xEnrollmentAddNeighborEntry(pxEnrollment, pxNeighInfo)) {
        LOGE(TAG_ENROLLMENT, "Failed to add the neighbor to the table");
        vRsMemFree(pxNeighInfo);
        return NULL;
    }
    else LOGI(TAG_ENROLLMENT, "Neighbor added: %s", pcApName);

    return pxNeighInfo;
}

bool_t xEnrollmentHandleConnect(struct ipcpInstanceData_t *pxData, string_t pcRemoteName, portId_t unPort)
{
    neighborInfo_t *pxNeighbor = NULL;
    enrollmentMessage_t *pxEnrollmentMsg = NULL;
    serObjectValue_t *pxObjVal = NULL;
    bool_t xStatus = false;

    RsAssert(pxData);
    RsAssert(pcRemoteName);
    RsAssert(is_port_id_ok(unPort));

    /* Check if the neighbor is already in the neighbor list, add it
     * if not */
    pxNeighbor = pxEnrollmentFindNeighbor(&pxData->xEnrollment, pcRemoteName);

    if (pxNeighbor == NULL) {
        /* We don't know the neigh address yet, set it -1 by now. We
         * need to wait for the M_START CDAP msg */
        pxNeighbor = pxEnrollmentCreateNeighInfo(&pxData->xEnrollment, pcRemoteName, unPort);

        if (!pxNeighbor) {
            LOGE(TAG_ENROLLMENT, "Failed to add create neighbor information");
            return false;
        }
    }

    /* FIXME: Currently this not take into account that we might
     * already be enrolled */

    pxNeighbor->eEnrollmentState = eENROLLMENT_IN_PROGRESS;

    /* Send M_START to the enroller if not enrolled */
    if (!(pxEnrollmentMsg = pvRsMemCAlloc(1, sizeof(*pxEnrollmentMsg)))) {
        LOGE(TAG_ENROLLMENT, "Failed to allocate memory for enrollment message");
        return false;
    }

    pxEnrollmentMsg->ullAddress = LOCAL_ADDRESS;

    /* Encode the enrollment message */
    if ((pxObjVal = pxSerDesEnrollmentEncode(&pxData->xEnrollment.xEnrollmentSD, pxEnrollmentMsg))) {

        /* Try to send it. */
        if (!xRibdSendResponse(&pxData->xRibd, "Enrollment", "/difm/enr", -1, 0, "", M_CONNECT_R, -1, unPort, pxObjVal))
            LOGE(TAG_ENROLLMENT, "Failed to send the RIB enrollment request");
        else
            xStatus = true;
    }
    /* Encoding failed. */
    else {
        LOGE(TAG_ENROLLMENT, "Failed to encode outgoing enrollment message");
        xStatus = false;
    }

    if (pxEnrollmentMsg)
        vRsMemFree(pxEnrollmentMsg);

    if (pxObjVal)
        vRsrcFree(pxObjVal);

    return xStatus;
}

bool_t xEnrollmentHandleConnectR(struct ipcpInstanceData_t *pxData, string_t pcRemoteName, portId_t unPort)
{
    neighborInfo_t *pxNeighbor = NULL;
    enrollmentMessage_t *pxEnrollmentMsg = NULL;
    serObjectValue_t *pxObjVal = NULL;
    bool_t xStatus = false;

    // Check if the neighbor is already in the neighbor list, add it if not
    pxNeighbor = pxEnrollmentFindNeighbor(&pxData->xEnrollment, pcRemoteName);

    if (pxNeighbor == NULL) {
        // We don't know the neigh address yet, set it -1 by now. We
        // need to wait for the M_START CDAP msg
        pxNeighbor = pxEnrollmentCreateNeighInfo(&pxData->xEnrollment, pcRemoteName, unPort);

        if (!pxNeighbor) {
            LOGE(TAG_ENROLLMENT, "Failed to add create neighbor information");
            return false;
        }
    }

    /* FIXME: Currently this not take into account that we might
     * already be enrolled */

    pxNeighbor->eEnrollmentState = eENROLLMENT_IN_PROGRESS;

    // Send M_START to the enroller if not enrolled
    if (!(pxEnrollmentMsg = pvRsMemAlloc(sizeof(enrollmentMessage_t)))) {
        LOGE(TAG_ENROLLMENT, "Failed to allocate memory for enrollment message");
        return false;
    }

    pxEnrollmentMsg->ullAddress = LOCAL_ADDRESS;

    /* Encode the enrollment message */
    if ((pxObjVal = pxSerDesEnrollmentEncode(&pxData->xEnrollment.xEnrollmentSD, pxEnrollmentMsg))) {
        if (!xRibdSendRequest(&pxData->xRibd, "Enrollment", "/difm/enr", -1, M_START, unPort, pxObjVal))
            LOGE(TAG_ENROLLMENT, "Failed to send the RIB enrollment request");
        else
            xStatus = true;
    }
    /* Encoding failed */
    else {
        LOGE(TAG_ENROLLMENT, "Failed to encode outgoing enrollment message");
        xStatus = false;
    }

    if (pxEnrollmentMsg)
        vRsMemFree(pxEnrollmentMsg);

    if (pxObjVal)
        vRsrcFree(pxObjVal);

    return xStatus;
}

address_t xEnrollmentGetNeighborAddress(Enrollment_t *pxEnrollment, string_t pcRemoteApName)
{
    num_t x;
    neighborInfo_t *pxNeighbor;

    RsAssert(pxEnrollment);
    RsAssert(pcRemoteApName);

    LOGI(TAG_ENROLLMENT, "Looking for '%s'", pcRemoteApName);

    for (x = 0; x < NEIGHBOR_TABLE_SIZE; x++) {
        if (pxEnrollment->xNeighborsTable[x].xValid == true) {
            pxNeighbor = pxEnrollment->xNeighborsTable[x].pxNeighborInfo;

            if (!strcmp(pxNeighbor->pcApName, pcRemoteApName))
                return pxNeighbor->xNeighborAddress;
        }
    }

    LOGI(TAG_ENROLLMENT, "Neighbor not founded");

    return ADDRESS_WRONG; // should be zero but for testing purposes.
}

bool_t xEnrollmentEnroller(struct ipcpInstanceData_t *pxData,
                           ribObject_t *pxEnrRibObj,
                           serObjectValue_t *pxObjValue,
                           string_t pcRemoteApName,
                           string_t pcLocalApName,
                           int invokeId,
                           portId_t unPort)
{
    neighborInfo_t *pxNeighborInfo = NULL;

    /* Check if the neighbor is already in the neighbor list, add it
       if not. */
    pxNeighborInfo = pxEnrollmentFindNeighbor(&pxData->xEnrollment, pcRemoteApName);

    if (!pxNeighborInfo)
        pxNeighborInfo = pxEnrollmentCreateNeighInfo(&pxData->xEnrollment, pcRemoteApName, unPort);

    /* Check in which sate of the enrollment process the neighbor is */
    switch (pxNeighborInfo->eEnrollmentState) {
    case eENROLLMENT_NONE:
        /* FIXME: This is only filled with comments in RinaSense
           main. I'm not sure what is supposed to be here. */
        break;

    case eENROLLMENT_IN_PROGRESS: {
        enrollmentMessage_t *pxEnrollMsg, xEnrollRes = {0}, xEnrollStop = {0};
        serObjectValue_t *pxResObjVal, *pxStopObjVal;

        pxEnrollMsg = pxSerDesEnrollmentDecode(&pxData->xEnrollment.xEnrollmentSD,
                                                   pxObjValue->pvSerBuffer,
                                                   pxObjValue->xSerLength);

        /* Update the neighbor address in the neighbor database */
        pxNeighborInfo->xNeighborAddress = pxEnrollMsg->ullAddress;

        /* Send M_START_R to the enrollee */
        xEnrollRes.ullAddress = LOCAL_ADDRESS;

        pxResObjVal = pxSerDesEnrollmentEncode(&pxData->xEnrollment.xEnrollmentSD, &xEnrollRes);

        if (pxResObjVal) {
            xRibdSendResponse(&pxData->xRibd, "Enrollment", "/difm/enr", -1, 0, NULL, M_START_R, invokeId, unPort, pxResObjVal);
        }

        xEnrollStop.xStartEarly = 1;

        pxStopObjVal = pxSerDesEnrollmentEncode(&pxData->xEnrollment.xEnrollmentSD, &xEnrollStop);

        xRibdSendRequest(&pxData->xRibd, "Enrollment", "/difm/enr", -1, M_STOP, unPort, pxStopObjVal);

        vRsrcFree(pxStopObjVal);
        vRsrcFree(pxResObjVal);

        /*ribd_send_resp(ipcp, enr_rib_obj->obj_class, enr_rib_obj->obj_name,
          enr_rib_obj->obj_inst, 0, NULL, M_START_R, invoke_id,
          n1_port, resp_obj_value);

          debug("EE <-- ER: M_START_R(enrollment)");g

          // TODO Send M_CREATE to the enrollee in order to initialize the
          // Static and Near Static information required.

          // Send M_STOP to the enrollee to indicate enrollment has finished
          ribd_send_req(ipcp, enr_rib_obj->obj_class, enr_rib_obj->obj_name,
          enr_rib_obj->obj_inst, M_STOP, n1_port, NULL);

          ngh_info->enr_state       = ENROLLED;
          ipcp->ipcp_info->enrolled = 1;

          debug("EE <-- ER: M_STOP");*/
        break;
    }

    default: break;
    }

    return true;
}

bool_t xEnrollmentHandleStopR(struct ipcpInstanceData_t *pxData, string_t pcRemoteApName)
{
    neighborInfo_t *pxNeighborInfo;

    LOGE(TAG_ENROLLMENT, "Handling a M_STOP_R CDAP Message");

    if (!pcRemoteApName) {
        LOGE(TAG_ENROLLMENT, "No valid Remote Application Process Name");
        return false;
    }

    pxNeighborInfo = pxEnrollmentFindNeighbor(&pxData->xEnrollment, pcRemoteApName);
    if (pxNeighborInfo == NULL) {
        LOGE(TAG_ENROLLMENT, "No neighbor founded with the name: '%s'", pcRemoteApName);
        return false;
    }

    LOGI(TAG_ENROLLMENT, "Enrollment finished with IPCP %s", pxNeighborInfo->pcApName);
    LOGI(TAG_ENROLLMENT, "Enrollment STOP_R");

    return true;
}

bool_t xEnrollmentHandleStop(struct ipcpInstanceData_t *pxData,
                             ribObject_t *pxEnrRibObj,
                             serObjectValue_t *pxSerObjectValue,
                             string_t pcRemoteApName,
                             string_t pxLocalApName,
                             int invokeId,
                             portId_t xN1Port)
{
    neighborInfo_t *pxNeighborInfo = NULL;
    enrollmentMessage_t *pxEnrollmentMsg;

    LOGI(TAG_ENROLLMENT, "Handling a M_STOP CDAP Message");

    pxNeighborInfo = pxEnrollmentFindNeighbor(&pxData->xEnrollment, pcRemoteApName);

    if (!pxNeighborInfo) {
        LOGE(TAG_ENROLLMENT, "Neighbor not found: %s", pcRemoteApName);
        return false;
    }

    pxNeighborInfo->eEnrollmentState = eENROLLED;

    /* Decoding Object Value */
    pxEnrollmentMsg = pxSerDesEnrollmentDecode(&pxData->xEnrollment.xEnrollmentSD,
                                               pxSerObjectValue->pvSerBuffer,
                                               pxSerObjectValue->xSerLength);

    pxNeighborInfo->pcToken = pxEnrollmentMsg->pcToken;

    // Send an M_STOP_R back to the enroller
    if (!xRibdSendResponse(&pxData->xRibd,
                           pxEnrRibObj->ucObjClass, pxEnrRibObj->ucObjName, pxEnrRibObj->ulObjInst,
                           0, NULL, M_STOP_R, invokeId, xN1Port, NULL)) {
        LOGE(TAG_ENROLLMENT, "Failed to sent M_STOP_R via n-1 port: %d", xN1Port);
        return false;
    }

    // ESP_LOGI(TAG_ENROLLMENT, "Enrollment finished with IPCP %s", pxNeighborInfo->pcApName);
    // ESP_LOGI(TAG_ENROLLMENT, "Enrollment STOP");

    return true;
}

bool_t xEnrollmentHandleStartR(struct ipcpInstanceData_t *pxData,
                               string_t pcRemoteApName,
                               serObjectValue_t *pxSerObjValue)
{

        enrollmentMessage_t *pxEnrollmentMsg;
        neighborInfo_t *pxNeighborInfo;

        /*
                if (pxSerObjValue == NULL)
                {
                        ESP_LOGE(TAG_ENROLLMENT, "Serialized Object Value is NULL");
                        return pdFALSE;
                }
                pxEnrollmentMsg = pxSerdesMsgEnrollmentDecode((uint8_t *)pxSerObjValue->pvSerBuffer, pxSerObjValue->xSerLength);
                pxNeighborInfo = pxEnrollmentFindNeighbor(pcRemoteApName);
                if (pxNeighborInfo == NULL)
                {
                        ESP_LOGE(TAG_ENROLLMENT, "There is no Neighbor Info in the List");
                        return pdFALSE;
                }
                pxNeighborInfo->xNeighborAddress = pxEnrollmentMsg->ullAddress;*/

        return false;
}

bool_t xEnrollmentHandleOperationalStart(struct ipcpInstanceData_t *pxData,
                                         ribObject_t *pxOperRibObj,
                                         serObjectValue_t *pxSerObjectValue,
                                         string_t pcRemoteApName,
                                         string_t pcLocalApName,
                                         int invokeId,
                                         portId_t xN1Port)
{
    neighborMessage_t *pxNeighborMsg;
    neighborInfo_t *pxNeighborInfo;
    serObjectValue_t *pxSerObjValue;
    bool_t xStatus;

    LOGE(TAG_ENROLLMENT, "Handling OperationalStart");

    pxNeighborInfo = pxEnrollmentFindNeighbor(&pxData->xEnrollment, pcRemoteApName);

    if (!pxNeighborInfo) {
        LOGE(TAG_ENROLLMENT, "Neighbor not found: %s", pcRemoteApName);
        return false;
    }

    pxNeighborMsg = pvRsMemAlloc(sizeof(*pxNeighborMsg));

    /*Looking for the token */
    pxNeighborMsg->pcApName = NORMAL_PROCESS_NAME;
    pxNeighborMsg->pcApInstance = NORMAL_PROCESS_INSTANCE;
    pxNeighborMsg->pcAeName = MANAGEMENT_AE;
    pxNeighborMsg->pcAeInstance = pxNeighborInfo->pcToken;

    /*encode the neighborMessage*/
    pxSerObjValue = pxSerDesNeighborEncode(&pxData->xEnrollment.xNeighborSD, pxNeighborMsg);

    // Send an M_STOP_R back to the enroller
    if (!xRibdSendResponse(&pxData->xRibd,
                           pxOperRibObj->ucObjClass,
                           pxOperRibObj->ucObjName,
                           pxOperRibObj->ulObjInst,
                           0, NULL, M_START_R, invokeId, xN1Port, pxSerObjValue)) {
        LOGE(TAG_ENROLLMENT, "Failed to sent M_STAR_R via n-1 port: %d", xN1Port);
        return false;
    }

    // ESP_LOGI(TAG_ENROLLMENT,"Enrollment finished with IPCP %s", pxNeighborInfo->xAPName);

    return true;
}


/* OLD CODE */

#if 0
neighborInfo_t *pxEnrollmentNeighborLookup(string_t pcRemoteApName)
{
        LOGE(TAG_ENROLLMENT, "--------------pxEnrollmentNeighborLookup----------");
        neighborInfo_t *pxNeigh;

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        pxNeigh = pvRsMemAlloc(sizeof(*pxNeigh));

        /* Find a way to iterate in the list and compare the addesss*/

        pxListEnd = listGET_END_MARKER(&xNeighborsList);
        pxListItem = listGET_HEAD_ENTRY(&xNeighborsList);

        while (pxListItem != pxListEnd)
        {

                pxNeigh = (neighborInfo_t *)listGET_LIST_ITEM_VALUE(pxListItem);
                LOGE(TAG_ENROLLMENT, "--------------pxNeigh:%p", pxNeigh);

                if (strcmp(pxNeigh->xAPName, pcRemoteApName))
                {
                        LOGI(TAG_ENROLLMENT, "Neighbor %p, #: %s", pxNeigh, pxNeigh->xAPName);
                        return pxNeigh;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        LOGE(TAG_ENROLLMENT, "--------------pxNeigh No neighbor founded");


        return NULL;
}
#endif


