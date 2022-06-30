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
#include "FlowAllocator.h"
#include "pidm.h"
#include "RINA_API.h"
#include "IPCP.h"
#include "normalIPCP.h"
#include "EFCP.h"

#include "esp_log.h"

static FlowRequestRow_t xFlowRequestTable[FLOWS_REQUEST];

void prvAddFlowRequestEntry(flowAllocatorInstance_t *pxFAI)
{

    BaseType_t x = 0;

    for (x = 0; x < FLOWS_REQUEST; x++)
    {
        if (xFlowRequestTable[x].xValid == pdFALSE)
        {
            xFlowRequestTable[x].pxFAI = pxFAI;
            xFlowRequestTable[x].xValid = pdTRUE;

            break;
        }
    }
}

flowAllocateHandle_t *prvGetPendingFlowRequest(portId_t xPortId)
{
    BaseType_t x = 0;
    flowAllocateHandle_t *pxFlowAllocateRequest;
    flowAllocatorInstance_t *pxFAI;

    for (x = 0; x < FLOWS_REQUEST; x++)
    {
        if (xFlowRequestTable[x].xValid == pdTRUE)
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
    pxFlowAllocator = pvPortMalloc(sizeof(*pxFlowAllocator));

    /* Create object in the Rib*/

    /*Init List*/
    vListInitialise(&pxFlowAllocator->xFlowAllocatorInstances);

    return pxFlowAllocator;
}

static qosSpec_t *prvFlowAllocatorSelectQoSCube(void)
{
    qosSpec_t *pxQosSpec;
    struct flowSpec_t *pxFlowSpec;

    pxQosSpec = pvPortMalloc(sizeof(*pxQosSpec));
    pxFlowSpec = pvPortMalloc(sizeof(*pxFlowSpec));

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
    struct dtcpConfig_t *pxDtcpConfig = NULL;

    pxFlow = pvPortMalloc(sizeof(*pxFlow));

    pxDtpConfig = pvPortMalloc((sizeof(*pxDtpConfig)));
    pxDtpPolicySet = pvPortMalloc(sizeof(*pxDtpPolicySet));

    // ESP_LOGI(TAG_FA, "DEst:%s", strdup(pxFlowRequest->pxRemote));

    pxFlow->pxSourceInfo = pxFlowRequest->pxLocal;
    pxFlow->pxDestInfo = pxFlowRequest->pxRemote;
    pxFlow->ulHopCount = 3;
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
    ESP_LOGI(TAG_FA, "Handling the Flow Allocation Request");

    flow_t *pxFlow;
    neighborInfo_t *pxNeighbor;
    cepId_t xCepSourceId;
    flowAllocatorInstance_t *pxFlowAllocatorInstance;
    connectionId_t *pxConnectionId;
    string_t pcNeighbor;
    struct efcpContainter_t *pxEfcpc;

    /* Create a flow object and fill using the event FlowRequest */
    pxFlow = prvFlowAllocatorNewFlow(pxFlowRequest);

    pxConnectionId = pvPortMalloc(sizeof(*pxConnectionId));

    /* Create a FAI and fill the struct properly*/
    pxFlowAllocatorInstance = pvPortMalloc(sizeof(*pxFlowAllocatorInstance));

    if (!pxFlowAllocatorInstance)
    {
        ESP_LOGE(TAG_FA, "FAI was not allocated");
    }
    // heap_caps_check_integrity(MALLOC_CAP_DEFAULT, pdTRUE);
    pxFlowAllocatorInstance->eFaiState = eFAI_NONE;
    pxFlowAllocatorInstance->xPortId = xAppPortId;
    pxFlowAllocatorInstance->pxFlowAllocatorHandle = pxFlowRequest;

    prvAddFlowRequestEntry(pxFlowAllocatorInstance);
    ESP_LOGE(TAG_FA, "FAI added properly");

    // Query NameManager to getting neighbor how knows the destination requested
    // pcNeighbor = xNmsGetNextHop(pxFlow->pxDesInfo->pcProcessName;

    pcNeighbor = "ar1.mobile"; // Hardcode for testing
    ESP_LOGI(TAG_FA, "Getting Neighbor");

    /* Request to DFT the Next Hop, at the moment request to EnrollmmentTask */
    pxNeighbor = pxEnrollmentFindNeighbor(pcNeighbor);
    pxFlow->xRemoteAddress = pxNeighbor->xNeighborAddress;

    pxFlow->xSourcePortId = xAppPortId;

    if (pxFlow->xRemoteAddress == 0)
    {
        ESP_LOGE(TAG_FA, "Error to get Next Hop");
    }

    /* Call EFCP to create an EFCP instance following the EFCP Config */
    ESP_LOGI(TAG_FA, "Creating a Connection");

    pxEfcpc = pxIPCPGetEfcpc();

    xCepSourceId = xNormalConnectionCreateRequest(pxEfcpc, xAppPortId,
                                                  LOCAL_ADDRESS, pxFlow->xRemoteAddress, pxFlow->pxQosSpec->xQosId,
                                                  pxFlow->pxDtpConfig, pxFlow->pxDtcpConfig);

    if (xCepSourceId == 0)
    {
        ESP_LOGE(TAG_FA, "CepId was not create properly");
    }

    /*------ Add CepSourceID into the Flow------*/
    if (!xNormalUpdateCepIdFlow(xAppPortId, xCepSourceId))
    {
        ESP_LOGI(TAG_FA, "CepId not updated into the flow");
    }
    ESP_LOGI(TAG_FA, "CepId updated into the flow");
    /* Fill the Flow connectionId */
    pxConnectionId->xSource = xCepSourceId;
    pxConnectionId->xQosId = pxFlow->pxQosSpec->xQosId;
    pxConnectionId->xDestination = 0;

    pxFlow->pxConnectionId = pxConnectionId;

    /* Send the flow message to the neighbor */
    // Serialize the pxFLow Struct into FlowMsg and Encode the FlowMsg as obj_value
    serObjectValue_t *pxObjVal = NULL;

    pxObjVal = pxSerdesMsgFlowEncode(pxFlow);

    // add somewhere the pxFlow???

    // Send using the ribd_send_req M_Create
    char flowObj[24];
    sprintf(flowObj, "/fa/flows/key=%d-%d", pxFlow->xSourceAddress, pxFlow->xSourcePortId);
    ESP_LOGE(TAG_FA, "Flow Object: %s", flowObj);

    // xPortId?? AppPortId or N1PortId
    if (!xRibdSendRequest("Flow", flowObj, -1, M_CREATE, pxNeighbor->xN1Port, pxObjVal)) // fixing N1PortId
    {
        ESP_LOGE(TAG_FA, "It was a problem to send the request");
        // return pdFALSE;
    }
}

BaseType_t xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue, int result)
{
    portId_t xAppPortId;
    flowAllocateHandle_t *pxFlowAllocateRequest;
    flow_t *pxFlow;

    if (pxSerObjValue == NULL)
    {
        ESP_LOGI(TAG_FA, "no object value ");
        return pdFALSE;
    }

    if (result != 0)
    {
        ESP_LOGI(TAG_FA, "Was not possible to create the Flow...");
        return pdFALSE;
    }
    // Decode the FA message

    pxFlow = pxSerdesMsgDecodeFlow(pxSerObjValue->pvSerBuffer, pxSerObjValue->xSerLength);

    if (!pxFlow)
        return pdFALSE;

    pxFlowAllocateRequest = prvGetPendingFlowRequest(pxFlow->xDestinationPortId);

    if (!pxFlowAllocateRequest)
    {
        ESP_LOGE(TAG_FA, "Flow Allocate Request was not founded ");
        return pdFALSE;
    }

    if (!xNormalConnectionModify(pxFlow->pxConnectionId->xDestination,
                                 pxFlow->xRemoteAddress,
                                 pxFlow->xSourceAddress))
    {
        ESP_LOGE(TAG_FA, "It was not possible to modify the connection");
        return pdFALSE;
    }

    if (!xNormalConnectionUpdate(pxFlow->xDestinationPortId, pxFlow->pxConnectionId->xSource,
                                 pxFlow->pxConnectionId->xDestination))
    {
        ESP_LOGE(TAG_FA, "It was not possible to update the connection");
        return pdFALSE;
    }
    ESP_LOGI(TAG_FA, "Flow state updated to Allocated");

    return pdTRUE;
}

void vFlowAllocatorDeallocate(portId_t xAppPortId)
{
}