#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "configSensor.h"
#include "configRINA.h"
#include "portability/port.h"

#include "efcpStructures.h"
#include "Enrollment.h"
#include "Enrollment_api.h"
#include "EnrollmentInformationMessage.pb.h"
#include "FlowAllocator.h"
#include "FlowAllocator_api.h"
#include "IPCP_normal_defs.h"
#include "IPCP_normal_api.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Rib.h"
#include "Ribd.h"
#include "Ribd_api.h"
#include "rina_common_port.h"
#include "RINA_API_flows.h"
#include "SerdesMsg.h"
#include "IPCP_api.h"

static pthread_mutex_t mux;

static FlowRequestRow_t xFlowRequestTable[FLOWS_REQUEST];

void prvAddFlowRequestEntry(flowAllocatorInstance_t *pxFAI)
{
    num_t x = 0;

    for (x = 0; x < FLOWS_REQUEST; x++)
    {
        if (xFlowRequestTable[x].xValid == false)
        {
            xFlowRequestTable[x].pxFAI = pxFAI;
            xFlowRequestTable[x].xValid = true;

            break;
        }
    }
}

flowAllocatorInstance_t *pxFAFindInstance(portId_t xPortId)
{
    num_t x = 0;
    flowAllocatorInstance_t *pxFAI;

    for (x = 0; x < FLOWS_REQUEST; x++)
    {
        if (xFlowRequestTable[x].xValid == true)
        {
            pxFAI = xFlowRequestTable->pxFAI;
            if (pxFAI->xPortId == xPortId) // ad xPortId
            {
                return pxFAI;
            }
        }
    }
    return NULL;
}

flowAllocateHandle_t *pxFAFindFlowHandle(portId_t xPortId)
{
    num_t x = 0;
    flowAllocatorInstance_t *pxFAI;
    flowAllocateHandle_t *pxFlowAllocateRequest;

    for (x = 0; x < FLOWS_REQUEST; x++)
    {
        if (xFlowRequestTable[x].xValid == true)
        {
            pxFAI = xFlowRequestTable->pxFAI;
            if (pxFAI->xPortId == xPortId) // ad xPortId
            {

                return pxFAI->pxFlowAllocatorHandle;
            }
        }
    }
    return NULL;
}

flowAllocateHandle_t *prvGetPendingFlowRequest(portId_t xPortId)
{
    num_t x = 0;
    flowAllocateHandle_t *pxFlowAllocateRequest;
    flowAllocatorInstance_t *pxFAI;

    for (x = 0; x < FLOWS_REQUEST; x++)
    {
        if (xFlowRequestTable[x].xValid == true)
        {
            pxFAI = xFlowRequestTable->pxFAI;
            if (pxFAI->xPortId == xPortId) // ad xPortId
            {
                pxFlowAllocateRequest = pxFAI->pxFlowAllocatorHandle;
                return pxFlowAllocateRequest;
            }
        }
    }
    return NULL;
}

/* Create_Request: handle the request send by other IPCP. Consults the local
 * directory Forwarding Table. It is to me, create a FAI*/

flowAllocator_t *pxFlowAllocatorInit(void)
{
    flowAllocator_t *pxFlowAllocator;
    pxFlowAllocator = pvRsMemAlloc(sizeof(*pxFlowAllocator));

    /* Create object in the Rib*/

    /*Init List*/
    vRsListInit(&pxFlowAllocator->xFlowAllocatorInstances);

    return pxFlowAllocator;
}

static qosSpec_t *prvFlowAllocatorSelectQoSCube(void)
{
    qosSpec_t *pxQosSpec;
    struct flowSpec_t *pxFlowSpec;

    pxQosSpec = pvRsMemAlloc(sizeof(*pxQosSpec));
    pxFlowSpec = pvRsMemAlloc(sizeof(*pxFlowSpec));

    /* TODO: Found the most suitable QoScube for the pxFlowRequest->pxFspec
     * Now, it use a default cube to test */
    pxQosSpec->xQosId = QoS_CUBE_ID;
    pxQosSpec->pcQosName = QoS_CUBE_NAME;

    pxFlowSpec->xPartialDelivery = QoS_CUBE_PARTIAL_DELIVERY;
    pxFlowSpec->xOrderedDelivery = QoS_CUBE_ORDERED_DELIVERY;

    pxQosSpec->pxFlowSpec = pxFlowSpec;

    return pxQosSpec;
}

/**/
static flow_t *prvFlowAllocatorNewFlow(flowAllocateHandle_t *pxFlowRequest)
{
    flow_t *pxFlow;

    dtpConfig_t *pxDtpConfig;
    policy_t *pxDtpPolicySet;
    /* UNUSED struct dtcpConfig_t *pxDtcpConfig = NULL; */

    pxFlow = pvRsMemAlloc(sizeof(*pxFlow));

    pxDtpConfig = pvRsMemAlloc((sizeof(*pxDtpConfig)));
    pxDtpPolicySet = pvRsMemAlloc(sizeof(*pxDtpPolicySet));

    // ESP_LOGI(TAG_FA, "DEst:%s", strdup(pxFlowRequest->pxRemote));

    pxFlow->pxSourceInfo = pxFlowRequest->pxLocal;
    pxFlow->pxDestInfo = pxFlowRequest->pxRemote;
    pxFlow->ulHopCount = 4;
    pxFlow->ulMaxCreateFlowRetries = 1;
    pxFlow->eState = eFA_ALLOCATION_IN_PROGRESS;
    pxFlow->xSourceAddress = LOCAL_ADDRESS;
    pxFlow->ulCurrentConnectionId = 0;

    /* Select QoS Cube based on the FlowSpec Required */
    pxFlow->pxQosSpec = prvFlowAllocatorSelectQoSCube();

    /* Fulfill the DTP_config and the DTCP_config based on the QoSCube*/

    pxDtpConfig->xDtcpPresent = DTP_DTCP_PRESENT;
    pxDtpConfig->xInitialATimer = DTP_INITIAL_A_TIMER;

    pxDtpPolicySet->pcPolicyName = DTP_POLICY_SET_NAME;
    pxDtpPolicySet->pcPolicyVersion = DTP_POLICY_SET_VERSION;
    pxDtpConfig->pxDtpPolicySet = pxDtpPolicySet;

    pxFlow->pxDtpConfig = pxDtpConfig;

    // By the moment the DTCP is not implemented yet so we are using DTP_DTCP_PRESENT = pdFALSE

    return pxFlow;
}

/*Allocate_Request: Handle the request send by the application
 * if it is well-formed, create a new FlowAllocator-Instance*/

void vFlowAllocatorFlowRequest(
    portId_t xAppPortId,
    flowAllocateHandle_t *pxFlowRequest)
{

    flow_t *pxFlow;
    neighborInfo_t *pxNeighbor;
    cepId_t xCepSourceId;
    flowAllocatorInstance_t *pxFlowAllocatorInstance;
    connectionId_t *pxConnectionId;
    string_t pcNeighbor;
    struct efcpContainer_t *pxEfcpc;

    /* Create a flow object and fill using the event FlowRequest */
    pxFlow = prvFlowAllocatorNewFlow(pxFlowRequest);

    pxConnectionId = pvRsMemAlloc(sizeof(*pxConnectionId));

    /* Create a FAI and fill the struct properly*/
    pxFlowAllocatorInstance = pvRsMemAlloc(sizeof(*pxFlowAllocatorInstance));

    if (!pxFlowAllocatorInstance)
    {
        LOGE(TAG_FA, "FAI was not allocated");
    }
    // heap_caps_check_integrity(MALLOC_CAP_DEFAULT, pdTRUE);
    pxFlowAllocatorInstance->eFaiState = eFAI_NONE;
    pxFlowAllocatorInstance->xPortId = xAppPortId;
    pxFlowAllocatorInstance->pxFlowAllocatorHandle = pxFlowRequest;

    prvAddFlowRequestEntry(pxFlowAllocatorInstance);
    LOGD(TAG_FA, "FAI added properly");

    // Query NameManager to getting neighbor how knows the destination requested
    // pcNeighbor = xNmsGetNextHop(pxFlow->pxDesInfo->pcProcessName;

    pcNeighbor = REMOTE_ADDRESS_AP_NAME; // "ar1.mobile"; // Hardcode for testing
    LOGD(TAG_FA, "Getting Neighbor");

    /* Request to DFT the Next Hop, at the moment request to EnrollmmentTask */
    pxNeighbor = pxEnrollmentFindNeighbor(pcNeighbor);
    if (!pxNeighbor) {
        LOGE(TAG_FA, "No neighbor found");
        return;
    }

    pxFlow->xRemoteAddress = pxNeighbor->xNeighborAddress;

    pxFlow->xSourcePortId = xAppPortId;

    if (pxFlow->xRemoteAddress == 0)
    {
        LOGE(TAG_FA, "Error to get Next Hop");
    }

    /* Call EFCP to create an EFCP instance following the EFCP Config */
    LOGD(TAG_FA, "Creating a Connection");

    pxEfcpc = pxIPCPGetEfcpc();

    xCepSourceId = xNormalConnectionCreateRequest(pxEfcpc, xAppPortId,
                                                  LOCAL_ADDRESS, pxFlow->xRemoteAddress, pxFlow->pxQosSpec->xQosId,
                                                  pxFlow->pxDtpConfig, pxFlow->pxDtcpConfig);

    if (xCepSourceId == 0)
    {
        LOGE(TAG_FA, "CepId was not create properly");
    }

    /*------ Add CepSourceID into the Flow------*/
    if (!xNormalUpdateCepIdFlow(xAppPortId, xCepSourceId))
    {
        LOGE(TAG_FA, "CepId not updated into the flow");
    }
    LOGD(TAG_FA, "CepId updated into the flow");
    /* Fill the Flow connectionId */
    pxConnectionId->xSource = xCepSourceId;
    pxConnectionId->xQosId = pxFlow->pxQosSpec->xQosId;
    pxConnectionId->xDestination = 0;

    pxFlow->pxConnectionId = pxConnectionId;

    /* Send the flow message to the neighbor */
    // Serialize the pxFLow Struct into FlowMsg and Encode the FlowMsg as obj_value
    serObjectValue_t *pxObjVal = NULL;

    pxObjVal = pxSerdesMsgFlowEncode(pxFlow);

    char flowObj[24];
    sprintf(flowObj, "/fa/flows/key=%d-%d", pxFlow->xSourceAddress, pxFlow->xSourcePortId);

    if (!pxRibCreateObject(flowObj, -1, "Flow", "Flow", FLOW))
    {
        LOGE(TAG_FA, "It was a problem to create Rib Object");
    }

    if (!xRibdSendRequest("Flow", flowObj, -1, M_CREATE, pxNeighbor->xN1Port, pxObjVal))
    {
        LOGE(TAG_FA, "It was a problem to send the request");
        // return pdFALSE;
    }

    if (pxObjVal != NULL)
        vRsMemFree(pxObjVal);
}

bool_t xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue, int result)
{
    portId_t xAppPortId;
    flowAllocatorInstance_t *pxFAI;

    flow_t *pxFlow;

    if (pxSerObjValue == NULL)
    {
        LOGI(TAG_FA, "no object value ");
        return false;
    }

    if (result != 0)
    {
        LOGE(TAG_FA, "Was not possible to create the Flow...");
        return false;
    }
    // Decode the FA message

    pxFlow = pxSerdesMsgDecodeFlow(pxSerObjValue->pvSerBuffer, pxSerObjValue->xSerLength);

    if (!pxFlow)
        return false;

    pxFAI = pxFAFindInstance(pxFlow->xDestinationPortId);

    if (!pxFAI)
    {
        LOGE(TAG_FA, "Flow Allocator Instance was not founded ");
        return false;
    }

    if (!xNormalConnectionModify(pxFlow->pxConnectionId->xDestination,
                                 pxFlow->xRemoteAddress,
                                 pxFlow->xSourceAddress))
    {
        LOGE(TAG_FA, "It was not possible to modify the connection");
        return false;
    }

    if (!xNormalConnectionUpdate(pxFlow->xDestinationPortId, pxFlow->pxConnectionId->xSource,
                                 pxFlow->pxConnectionId->xDestination))
    {
        LOGE(TAG_FA, "It was not possible to update the connection");
        return false;
    }

    pxFAI->eFaiState = eFAI_ALLOCATED;
    LOGI(TAG_FA, "Flow state updated to Allocated");

    pthread_mutex_lock(&pxFAI->pxFlowAllocatorHandle->xEventMutex);
    {
        pxFAI->pxFlowAllocatorHandle->nEventBits |= eFLOW_BOUND;
        pthread_cond_signal(&pxFAI->pxFlowAllocatorHandle->xEventCond);
    }
    pthread_mutex_unlock(&pxFAI->pxFlowAllocatorHandle->xEventMutex);

    return true;
}

bool_t xFlowAllocatorHandleDelete(struct ribObject_t *pxRibObject, int invoke_id)
{
    LOGD(TAG_FA, "Calling: %s", __func__);

    // Delete connection
    // delete EFCP instance
    // change portId to NO ALLOCATED,
    // Send message to User with close,

    return false;
}

void vFlowAllocatorDeallocate(portId_t xAppPortId)
{

    if (!xAppPortId)
    {
        LOGE(TAG_FA, "Bogus data passed, bailing out");
    }

    // Find Flow and move to deallocate status.
}

bool_t xFlowAllocatorDuPost(portId_t xAppPortId, struct du_t *pxDu)
{
    flowAllocatorInstance_t *pxFlowAllocatorInstance;
    NetworkBufferDescriptor_t *pxNetworkBuffer;

    if (!xDuIsOk(pxDu) || !is_port_id_ok(xAppPortId))
    {
        LOGE(TAG_FA, "Bogus Network Buffer passed, cannot post SDU");
        xDuDestroy(pxDu);
        return false;
    }

    pxFlowAllocatorInstance = pxFAFindInstance(xAppPortId);

    if (!pxFlowAllocatorInstance)
    {
        LOGE(TAG_FA, "Flow Allocator instance was not founded");
        xDuDestroy(pxDu);
        return false;
    }

    LOGE(TAG_FA, "Posting DU to port-id %d ", xAppPortId);

    pxNetworkBuffer = pxDu->pxNetworkBuffer;

    if (pxFlowAllocatorInstance->eFaiState != eFAI_ALLOCATED)
    {
        LOGE(TAG_FA, "Flow with port-id %d is not allocated", xAppPortId);
        xDuDestroy(pxDu);
        return false;
    }

    /* Add the network packet to the list of packets to be
     * processed by the socket. */

    pthread_mutex_lock(&mux);
    {
        vRsListInitItem(&(pxNetworkBuffer->xBufferListItem), (void *)pxNetworkBuffer);
        vRsListInsert(&(pxFlowAllocatorInstance->pxFlowAllocatorHandle->xListWaitingPackets), &(pxNetworkBuffer->xBufferListItem));
    }
    pthread_mutex_unlock(&mux);

    pthread_mutex_lock(&pxFlowAllocatorInstance->pxFlowAllocatorHandle->xEventMutex);
    {
        pxFlowAllocatorInstance->pxFlowAllocatorHandle->nEventBits |= eFLOW_RECEIVE;
        pthread_cond_signal(&pxFlowAllocatorInstance->pxFlowAllocatorHandle->xEventCond);
    }
    pthread_mutex_unlock(&pxFlowAllocatorInstance->pxFlowAllocatorHandle->xEventMutex);

    return true;
}
