/*Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* RINA includes. */
#include "rina_common_port.h"
#include "configSensor.h"
#include "configRINA.h"
#include "Ribd.h"
#include "Ribd_api.h"
#include "Enrollment.h"
#include "EnrollmentInformationMessage.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Rib.h"
#include "SerdesMsg.h"
#include "IPCP_normal_defs.h"
#include "IPCP_api.h"

static neighborsTableRow_t xNeighborsTable[NEIGHBOR_TABLE_SIZE];

void vEnrollmentAddNeighborEntry(neighborInfo_t *pxNeighbor)
{
        num_t x = 0;

        for (x = 0; x < NEIGHBOR_TABLE_SIZE; x++)
        {
                if (xNeighborsTable[x].xValid == false)
                {
                        xNeighborsTable[x].pxNeighborInfo = pxNeighbor;
                        xNeighborsTable[x].xValid = true;
                        LOGI(TAG_ENROLLMENT, "Neighbor Entry successful: %p", pxNeighbor);

                        break;
                }
        }
}

neighborInfo_t *pxEnrollmentFindNeighbor(string_t pcRemoteApName)
{

        num_t x = 0;
        neighborInfo_t *pxNeighbor;
        pxNeighbor = pvRsMemAlloc(sizeof(*pxNeighbor));

        LOGD(TAG_ENROLLMENT, "Looking for '%s'", pcRemoteApName);
        for (x = 0; x < NEIGHBOR_TABLE_SIZE; x++)

        {
                if (xNeighborsTable[x].xValid == true)
                {
                        pxNeighbor = xNeighborsTable[x].pxNeighborInfo;
                        LOGD(TAG_ENROLLMENT, "Neighbor checking '%p'", pxNeighbor);
                        LOGD(TAG_ENROLLMENT, "Comparing '%s' - '%s'", pxNeighbor->pcApName, pcRemoteApName);
                        if (!strcmp(pxNeighbor->pcApName, pcRemoteApName))
                        {
                                LOGD(TAG_ENROLLMENT, "Neighbor founded '%p'", pxNeighbor);

                                return pxNeighbor;
                                break;
                        }
                }
        }
        LOGD(TAG_ENROLLMENT, "Neighbor not founded");

        return NULL;
}
address_t xEnrollmentGetNeighborAddress(string_t pcRemoteApName)
{

        num_t x;
        neighborInfo_t *pxNeighbor;
        pxNeighbor = pvRsMemAlloc(sizeof(*pxNeighbor));
        if (!pcRemoteApName)
        {
                LOGE(TAG_ENROLLMENT, "No Remote Application Name valid");
                return 0;
        }

        LOGI(TAG_ENROLLMENT, "Looking for '%s'", pcRemoteApName);
        for (x = 0; x < NEIGHBOR_TABLE_SIZE; x++)

        {
                if (xNeighborsTable[x].xValid == true)
                {
                        pxNeighbor = xNeighborsTable[x].pxNeighborInfo;
                        LOGI(TAG_ENROLLMENT, "Neighbor checking '%p'", pxNeighbor);
                        LOGI(TAG_ENROLLMENT, "Comparing '%s' - '%s'", pxNeighbor->pcApName, pcRemoteApName);
                        if (!strcmp(pxNeighbor->pcApName, pcRemoteApName))
                        {
                                LOGD(TAG_ENROLLMENT, "Neighbor founded '%p'", pxNeighbor);

                                return pxNeighbor->xNeighborAddress;
                                break;
                        }
                }
        }
        LOGI(TAG_ENROLLMENT, "Neighbor not founded");

        return 3; // should be zero but for testing purposes.
}

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

neighborInfo_t *pxEnrollmentCreateNeighInfo(string_t pcApName, portId_t xN1Port)
{

        neighborInfo_t *pxNeighInfo;

        pxNeighInfo = pvRsMemAlloc(sizeof(*pxNeighInfo));

        pxNeighInfo->pcApName = strdup(pcApName);
        pxNeighInfo->eEnrollmentState = eENROLLMENT_NONE;
        pxNeighInfo->xNeighborAddress = -1;
        pxNeighInfo->xN1Port = xN1Port;
        pxNeighInfo->pcToken = NULL;

        // Save the info in the neighbor database
        // vListInitialiseItem(&(pxNeighInfo->xNeighborItem));
        // listSET_LIST_ITEM_OWNER(&(pxNeighInfo->xNeighborItem), (void *)pxNeighInfo);
        // vListInsert(&(pxNeighInfo->xNeighborItem), &(xNeighborsList));

        vEnrollmentAddNeighborEntry(pxNeighInfo);

        LOGI(TAG_ENROLLMENT, "Neighbor Created:%p", pxNeighInfo);

        return pxNeighInfo;
}

/*EnrollmentInit should create neighbor and enrollment object into the RIB*/
void xEnrollmentInit(struct ipcpInstanceData_t *pxIpcpData, portId_t xN1PortId)
{
        LOGI(TAG_ENROLLMENT, "-------- Init Enrollment Task --------");
        name_t *pxSource;
        name_t *pxDestInfo;
        // porId_t xN1FlowPortId;
        authPolicy_t *pxAuth;
        // appConnection_t *test;
        // test = pvPortMalloc(sizeof(*test));

        pxSource = pvRsMemAlloc(sizeof(*pxSource));
        pxDestInfo = pvRsMemAlloc(sizeof(*pxDestInfo));
        pxAuth = pvRsMemAlloc(sizeof(*pxAuth));

        pxSource->pcEntityInstance = NORMAL_ENTITY_INSTANCE;
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
        // vListInitialise(&xNeighborsList);

        /*Creating Enrollment object into the RIB*/
        pxRibCreateObject("/difm/enr", 0, "enrollment", "enrollment", ENROLLMENT);

        /*Creating Operational Status into the the RIB*/
        pxRibCreateObject("/difm/ops", 1, "OperationalStatus", "OperationalStatus", OPERATIONAL);

        (void)xRibdConnectToIpcp(pxIpcpData, pxSource, pxDestInfo, xN1PortId, pxAuth);
}

bool_t xEnrollmentEnroller(struct ribObject_t *pxEnrRibObj,
                           serObjectValue_t *pxObjValue,
                           string_t pcRemoteApName,
                           string_t pcLocalApName,
                           int invokeId,
                           portId_t xN1Port)
{

        neighborInfo_t *pxNeighborInfo = NULL;

        // char *my_ap_name            = ipcp_get_process_name(ipcp);
        // char *n_dif                 = ipcp_get_dif_name(ipcp);

        // Check if the neighbor is already in the neighbor list, add it if not.
        pxNeighborInfo = pxEnrollmentFindNeighbor(pcRemoteApName);

        if (pxNeighborInfo == NULL)
        {
                // We don't know the neigh address yet, set it -1 by now. We need to
                // wait for the M_START CDAP msg
                pxNeighborInfo = pxEnrollmentCreateNeighInfo(pcRemoteApName, xN1Port);
        }

        // Check in which sate of the enrollment process the neighbor is
        switch (pxNeighborInfo->eEnrollmentState)
        {
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
                enrollmentMessage_t *pxResponseEnrObj = pvRsMemAlloc(sizeof(*pxResponseEnrObj));

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
        return true;
}

bool_t xEnrollmentHandleConnectR(struct ipcpInstanceData_t *pxData, string_t pcRemoteApName, portId_t xN1Port)
{
        neighborInfo_t *pxNeighbor = NULL;
        struct rmt_t *pxRmt;

        // Check if the neighbor is already in the neighbor list, add it if not
        pxNeighbor = pxEnrollmentFindNeighbor(pcRemoteApName);

        if (pxNeighbor == NULL)
        {
                // We don't know the neigh address yet, set it -1 by now. We need to
                // wait for the M_START CDAP msg
                pxNeighbor = pxEnrollmentCreateNeighInfo(pcRemoteApName, xN1Port);
        }

        pxNeighbor->eEnrollmentState = eENROLLMENT_IN_PROGRESS;

        pxRmt = pxIPCPGetRmt();

        // Send M_START to the enroller if not enrolled
        enrollmentMessage_t *pxEnrollmentMsg = pvRsMemAlloc(sizeof(*pxEnrollmentMsg));
        pxEnrollmentMsg->ullAddress = LOCAL_ADDRESS;

        serObjectValue_t *pxObjVal = pxSerdesMsgEnrollmentEncode(pxEnrollmentMsg);

        if (!xRibdSendRequest("Enrollment", "/difm/enr", -1, M_START, xN1Port, pxObjVal))
        {
                LOGE(TAG_ENROLLMENT, "It was a problem to sen the request");
                return false;
        }

        vRsMemFree(pxEnrollmentMsg);
        vRsMemFree(pxObjVal);

        return true;
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
bool_t xEnrollmentHandleStartR(string_t pcRemoteApName, serObjectValue_t *pxSerObjValue)
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

        return true;
}

bool_t xEnrollmentHandleStopR(string_t pcRemoteApName)
{
        LOGE(TAG_ENROLLMENT, "Handling a M_STOP_R CDAP Message");

        neighborInfo_t *pxNeighborInfo;

        if (!pcRemoteApName)
        {
                LOGE(TAG_ENROLLMENT, "No valid Remote Application Process Name");
                return false;
        }

        pxNeighborInfo = pxEnrollmentFindNeighbor(pcRemoteApName);
        if (pxNeighborInfo == NULL)
        {
                LOGE(TAG_ENROLLMENT, "No neighbor founded with the name: '%s'", pcRemoteApName);
                return false;
        }
        LOGI(TAG_ENROLLMENT, "Enrollment finished with IPCP %s", pxNeighborInfo->pcApName);
        LOGI(TAG_ENROLLMENT, "Enrollment STOP_R");

        return true;
}

bool_t xEnrollmentHandleStop(struct ribObject_t *pxEnrRibObj,
                             serObjectValue_t *pxSerObjectValue,
                             string_t pcRemoteApName,
                             string_t pxLocalApName,
                             int invokeId,
                             portId_t xN1Port)
{
        LOGD(TAG_ENROLLMENT, "Handling a M_STOP CDAP Message");
        neighborInfo_t *pxNeighborInfo = NULL;
        enrollmentMessage_t *pxEnrollmentMsg;

        pxNeighborInfo = pxEnrollmentFindNeighbor(pcRemoteApName);

        if (pxNeighborInfo == NULL)
        {
                LOGE(TAG_ENROLLMENT, "Neighbor was not founded into the list of Neighbors");
                return false;
        }

        pxNeighborInfo->eEnrollmentState = eENROLLED;

        /* Decoding Object Value */
        pxEnrollmentMsg = pxSerdesMsgEnrollmentDecode(pxSerObjectValue->pvSerBuffer, pxSerObjectValue->xSerLength);

        pxNeighborInfo->pcToken = pxEnrollmentMsg->pcToken;

        // Send an M_STOP_R back to the enroller
        if (!xRibdSendResponse(pxEnrRibObj->ucObjClass, pxEnrRibObj->ucObjName, pxEnrRibObj->ulObjInst,
                               0, NULL, M_STOP_R, invokeId, xN1Port, NULL))
        {
                LOGE(TAG_ENROLLMENT, "Failed to sent M_STOP_R via n-1 port: %u", xN1Port);
                return false;
        }

        // ESP_LOGI(TAG_ENROLLMENT, "Enrollment finished with IPCP %s", pxNeighborInfo->pcApName);
        // ESP_LOGI(TAG_ENROLLMENT, "Enrollment STOP");

        return true;
}

bool_t xEnrollmentHandleOperationalStart(struct ribObject_t *pxOperRibObj,
                                         serObjectValue_t *pxSerObjectValue,
                                         string_t pcRemoteApName,
                                         string_t pcLocalApName,
                                         int invokeId,
                                         portId_t xN1Port)
{

        neighborMessage_t *pxNeighborMsg;
        neighborInfo_t *pxNeighborInfo = NULL;

        LOGD(TAG_ENROLLMENT, "Looking for a neighbor");
        pxNeighborInfo = pxEnrollmentFindNeighbor(pcRemoteApName);

        pxNeighborMsg = pvRsMemAlloc(sizeof(*pxNeighborMsg));

        /*Looking for the token */
        pxNeighborMsg->pcApName = NORMAL_PROCESS_NAME;
        pxNeighborMsg->pcApInstance = NORMAL_PROCESS_INSTANCE;
        pxNeighborMsg->pcAeName = MANAGEMENT_AE;
        pxNeighborMsg->pcAeInstance = pxNeighborInfo->pcToken;

        /*encode the neighborMessage*/
        serObjectValue_t *pxSerObjValue = pxSerdesMsgNeighborEncode(pxNeighborMsg);

        // Send an M_STOP_R back to the enroller
        if (!xRibdSendResponse(pxOperRibObj->ucObjClass, pxOperRibObj->ucObjName, pxOperRibObj->ulObjInst,
                               0, NULL, M_START_R, invokeId, xN1Port, pxSerObjValue))
        {
                LOGE(TAG_ENROLLMENT, "Failed to sent M_STAR_R via n-1 port: %u", xN1Port);
                return false;
        }

        // ESP_LOGI(TAG_ENROLLMENT,"Enrollment finished with IPCP %s", pxNeighborInfo->xAPName);

        return true;
}
