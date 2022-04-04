/*Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

/* RINA includes. */
#include "common.h"
#include "configSensor.h"
#include "configRINA.h"
#include "Ribd.h"
#include "Enrollment.h"
#include "EnrollmentInformationMessage.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Rib.h"
#include "SerdesMsg.h"

#include "esp_log.h"

/*List of Neighbors*/
static List_t xNeighborsList;

neighborInfo_t *pxEnrollmentNeighborLookup(string_t xRemoteAPName)
{
        neighborInfo_t *pxNeigh;

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        pxNeigh = pvPortMalloc(sizeof(*pxNeigh));

        /* Find a way to iterate in the list and compare the addesss*/

        pxListEnd = listGET_END_MARKER(&xNeighborsList);
        pxListItem = listGET_HEAD_ENTRY(&xNeighborsList);

        while (pxListItem != pxListEnd)
        {

                pxNeigh = (neighborInfo_t *)listGET_LIST_ITEM_VALUE(pxListItem);

                if (pxNeigh)
                {
                        ESP_LOGI(TAG_ENROLLMENT, "Neighbor %p, #: %s", pxNeigh, pxNeigh->xAPName);
                        return pxNeigh;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        /*Adding new Neigh info*/
        //strcpy(pxNeigh->xAPName, xRemoteAPName);
        // vListInitialiseItem(&pxNeigh->xNeighborItem);
        // vListInsert(&pxNeigh->xNeighborItem, &xNeighborsList);

        return pxNeigh;
}

neighborInfo_t * pxEnrollmentCreateNeighInfo(string_t pcApName, portId_t xN1Port)
{
    neighborInfo_t *pxNeighInfo;

    pxNeighInfo = pvPortMalloc(sizeof(*pxNeighInfo));

    pxNeighInfo->xAPName = strdup(pcApName);
    pxNeighInfo->eEnrollmentState = eENROLLMENT_NONE;
    pxNeighInfo->xNeighborAddress = -1;
    pxNeighInfo->xN1Port = xN1Port;

    // Save the info in the neighbor database
    vListInitialiseItem(&pxNeighInfo->xNeighborItem);
    vListInsert(&pxNeighInfo->xNeighborItem, &xNeighborsList);

    return pxNeighInfo;
}


void xEnrollmentInit(portId_t xPortId);

/*EnrollmentInit should create neighbor and enrollment object into the RIB*/
void xEnrollmentInit(portId_t xPortId)
{

        name_t *pxSource;
        name_t *pxDestInfo;
        // porId_t xN1FlowPortId;
        authPolicy_t *pxAuth;
        appConnection_t *test;
        test = pvPortMalloc(sizeof(*test));

        pxSource = pvPortMalloc(sizeof(*pxSource));
        pxDestInfo = pvPortMalloc(sizeof(*pxDestInfo));
        pxAuth = pvPortMalloc(sizeof(*pxAuth));

        pxSource->pcEntityInstance = NORMAL_ENTITY_NAME;
        pxSource->pcEntityName = MANAGEMENT_AE;

        pxSource->pcProcessInstance = NORMAL_PROCESS_INSTANCE;
        pxSource->pcProcessName = NORMAL_PROCESS_NAME;

        pxDestInfo->pcProcessName = NORMAL_DIF_NAME;
        pxDestInfo->pcProcessInstance = "";
        pxDestInfo->pcEntityInstance = "";
        pxDestInfo->pcEntityName = MANAGEMENT_AE;

        pxAuth->ucAbsSyntax = 1;
        pxAuth->pcVersion = "1";
        pxAuth->pcName = "PSOC_authentication-none";

        /*Initialization xNeighbors item*/
        vListInitialise(&xNeighborsList);

        // Find PortId

        xRibdConnectToIpcp(pxSource, pxDestInfo, xPortId, pxAuth);
}


BaseType_t xEnrollmentEnroller(struct ribObject_t *pxEnrRibObj, serObjectValue_t *pxObjValue, string_t pcRemoteApName,
                 string_t pcLocalApName, int invokeId, portId_t xN1Port)
{
 
        neighborInfo_t *pxNeighborInfo = NULL;
        
   
    //char *my_ap_name            = ipcp_get_process_name(ipcp);
    //char *n_dif                 = ipcp_get_dif_name(ipcp);

    // Check if the neighbor is already in the neighbor list, add it if not.
    pxNeighborInfo = pxEnrollmentNeighborLookup(pcRemoteApName);

    if (pxNeighborInfo == NULL) {
        // We don't know the neigh address yet, set it -1 by now. We need to
        // wait for the M_START CDAP msg
        pxNeighborInfo = pxEnrollmentCreateNeighInfo(pcRemoteApName,xN1Port);
    }

    // Check in which sate of the enrollment process the neighbor is
    switch (pxNeighborInfo->eEnrollmentState) {
    case eENROLLMENT_NONE:
        // Received M_CONNECT request from the enrollee

        // Check that if dst process name that the enrollee has used, is my
        // own process name or is the DIF name (and then he does not know my
        // name)
        // TODO test if this checking with the remote process name works
        /*if (strcmp(lapn, my_ap_name) != 0 && strcmp(lapn, n_dif) != 0) {
            warning("enrollee is using %s as rapn. Not using the N-DIF name "
                    "(%s) nor my AP name (%s)",
                    rapn, n_dif, my_ap_name);
            break; // TODO ¿Or answer with negative result?
        }

        if (ribd_send_connect_r(ipcp, my_ap_name, rapn, n1_port, invoke_id) <
            0) {
            error("abort enrollment");
        }
        debug("EE <-- ER M_CONNECT_R(enrollment)");
        ngh_info->enr_state = ENROLLING;*/

        break;

    case eENROLLMENT_IN_PROGRESS:
       {
                // Received M_START request from the enrollee

        // Decode the serialized object value from the CDAP message
        enrollmentMessage_t *pxEnrollmentMsg = pxSerdesMsgEnrollmentDecode(pxObjValue->pvSerBuffer, pxObjValue->xSerLength);

        // Update the neighbor address in the neighbor database
        pxNeighborInfo->xNeighborAddress = pxEnrollmentMsg->ullAddress;

        // Send M_START_R to the enrollee
        enrollmentMessage_t *pxResponseEnrObj = pvPortMalloc(sizeof(*pxResponseEnrObj));

        pxResponseEnrObj->ullAddress = LOCAL_ADDRESS;


        serObjectValue_t *pxResponseObjValue = pxSerdesMsgEnrollmentEncode(pxResponseEnrObj);

        /*ribd_send_resp(ipcp, enr_rib_obj->obj_class, enr_rib_obj->obj_name,
                       enr_rib_obj->obj_inst, 0, NULL, M_START_R, invoke_id,
                       n1_port, resp_obj_value);

        debug("EE <-- ER: M_START_R(enrollment)");

        // TODO Send M_CREATE to the enrollee in order to initialize the
        // Static and Near Static information required.

        // Send M_STOP to the enrollee to indicate enrollment has finished
        ribd_send_req(ipcp, enr_rib_obj->obj_class, enr_rib_obj->obj_name,
                      enr_rib_obj->obj_inst, M_STOP, n1_port, NULL);

        ngh_info->enr_state       = ENROLLED;
        ipcp->ipcp_info->enrolled = 1;

        debug("EE <-- ER: M_STOP");*/
       }

        break;

    default:
        break;
    }
    return pdTRUE;
}

BaseType_t xEnrollmentHandleConnectR(string_t xRemoteProcessName, portId_t xN1Port)
{

        string_t xRemoteAPN = xRemoteProcessName;
        neighborInfo_t *pxNeighbor = NULL;

        // Check if the neighbor is already in the neighbor list, add it if not
        pxNeighbor = pxEnrollmentNeighborLookup(xRemoteProcessName);

        configASSERT(pxNeighbor != NULL);

        pxNeighbor->eEnrollmentState = eENROLLMENT_IN_PROGRESS;

        // Send M_START to the enroller if not enrolled
        enrollmentMessage_t *pxEnrollmentMsg = pvPortMalloc(sizeof(*pxEnrollmentMsg));
        pxEnrollmentMsg->ullAddress = LOCAL_ADDRESS;

        serObjectValue_t *pxObjVal = pxSerdesMsgEnrollmentEncode(pxEnrollmentMsg);

        if (!xRibdSendRequest("Enrollment", "/difm/enr", -1, M_START, xN1Port, pxObjVal))
        {
                ESP_LOGE(TAG_ENROLLMENT, "It was a problem to sen the request");
                return pdFALSE;
        }

        vPortFree(pxEnrollmentMsg);
        vPortFree(pxObjVal);

        return pdTRUE;
}

/**
 * @brief Handle the Start Message: 1. Decode the serialized Object Value
 * 2. Found the Neighbor into the Neighbors List.
 * 3. Update the Neighbor Address.
 *
 * @param xRemoteApName Remote APName for looking up into the List
 * @param pxSerObjValue Enrollment message with neighbor info.
 * @return BaseType_t
 */
BaseType_t xEnrollmentHandleStartR(string_t xRemoteApName, serObjectValue_t *pxSerObjValue)
{

        enrollmentMessage_t *pxEnrollmentMsg;
        neighborInfo_t *pxNeighborInfo;

        pxEnrollmentMsg = pxSerdesMsgEnrollmentDecode((uint8_t *)pxSerObjValue->pvSerBuffer, pxSerObjValue->xSerLength);

        pxNeighborInfo = pxEnrollmentNeighborLookup(xRemoteApName);
        if (pxNeighborInfo == NULL)
        {
                ESP_LOGE(TAG_ENROLLMENT, "There is no Neighbor Info in the List");
                return pdFALSE;
        }

        pxNeighborInfo->xNeighborAddress = pxEnrollmentMsg->ullAddress;

        return pdTRUE;
}

BaseType_t xEnrollmentHandleStopR(string_t xRemoteApName)
{

        neighborInfo_t *pxNeighborInfo;
        pxNeighborInfo = pxEnrollmentNeighborLookup(xRemoteApName);
        ESP_LOGI(TAG_ENROLLMENT, "Enrollment finished with IPCP %s", pxNeighborInfo->xAPName);

        return 0;
}

BaseType_t xEnrollmentHandleStop(struct ribObject_t *pxEnrRibObj,
                                 serObjectValue_t *pxObjValue, string_t xRemoteApName,
                                 string_t xLocalProcessName, int invokeId, portId_t xN1Port)
{

        neighborInfo_t *pxNeighborInfo = NULL;

        pxNeighborInfo = pxEnrollmentNeighborLookup(xRemoteApName);

        configASSERT(pxNeighborInfo!= NULL);

        pxNeighborInfo->eEnrollmentState = eENROLLED;

        // Send an M_STOP_R back to the enroller
        if (xRibdSendRequest(pxEnrRibObj->ucObjClass, pxEnrRibObj->ucObjName, pxEnrRibObj->ulObjInst,
                        M_STOP_R, xN1Port, NULL))
        {
                ESP_LOGE(TAG_ENROLLMENT,"Failed to sent M_STOP_R via n-1 port: %d", xN1Port);
                return pdFALSE;
        }

        ESP_LOGI(TAG_ENROLLMENT,"Enrollment finished with IPCP %s", pxNeighborInfo->xAPName);

        return pdTRUE;
}
