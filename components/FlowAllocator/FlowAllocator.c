/*Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "configSensor.h"
#include "configRINA.h"

#include "portability/port.h"

#include "EFCP.h"
#include "Enrollment.h"
#include "Enrollment_api.h"
#include "EnrollmentInformationMessage.pb.h"
#include "FlowAllocator.h"
#include "IPCP_normal_defs.h"
#include "IPCP_normal_api.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Rib.h"
#include "Ribd.h"
#include "rina_common_port.h"
#include "RINA_API_flows.h"
#include "SerdesMsg.h"

/* Create_Request: handle the request send by other IPCP. Consults the local
 * directory Forwarding Table. It is to me, create a FAI*/

bool_t xFlowAllocatorInit()
{
    flowAllocator_t *pxFlowAllocator;
    pxFlowAllocator = pvRsMemAlloc(sizeof(*pxFlowAllocator));

    /* Create object in the Rib*/
    pxRibCreateObject("/fa/flows/", 0, "Flow", "Flow", FLOW_ALLOCATOR);

    /*Init List*/
    // vListInitialise(&pxFlowAllocator->xFlowAllocatorInstances);

    return true;
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
    struct dtcpConfig_t *pxDtcpConfig = NULL;

    pxFlow = pvRsMemAlloc(sizeof(*pxFlow));

    pxDtpConfig = pvRsMemAlloc((sizeof(*pxDtpConfig)));
    pxDtpPolicySet = pvRsMemAlloc(sizeof(*pxDtpPolicySet));

    // ESP_LOGI(TAG_FA, "DEst:%s", strdup(pxFlowRequest->pxRemote));

    LOGE(TAG_FA, "pxLocal:%p", pxFlowRequest->pxLocal);
    LOGE(TAG_FA, "pxRemote:%p", pxFlowRequest->pxRemote);
    LOGE(TAG_FA, "-----DETAILS----");
    LOGE(TAG_FA, "pxLocal_APName:%s", pxFlowRequest->pxLocal->pcProcessName);
    LOGE(TAG_FA, "pxLocal_APInstance:%s", pxFlowRequest->pxLocal->pcProcessInstance);
    LOGE(TAG_FA, "pxLocal_AEName:%s", pxFlowRequest->pxLocal->pcEntityName);
    LOGE(TAG_FA, "pxLocal_AEInstance:%s", pxFlowRequest->pxLocal->pcEntityInstance);
    LOGE(TAG_FA, "pxRemote_APName:%s", pxFlowRequest->pxRemote->pcProcessName);
    LOGE(TAG_FA, "pxRemote_APInstance:%s", pxFlowRequest->pxRemote->pcProcessInstance);
    LOGE(TAG_FA, "pxRemote_AEName:%s", pxFlowRequest->pxRemote->pcEntityName);
    LOGE(TAG_FA, "pxRemote_AEInstnace:%s", pxFlowRequest->pxRemote->pcEntityInstance);

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
void vFlowAllocatorFlowRequest(struct efcpContainer_t *pxEfcpc, portId_t xPortId, flowAllocateHandle_t *pxFlowRequest, struct ipcpNormalData_t *pxIpcpData)
{
    LOGE(TAG_FA, "calling:%s", __func__);

    flow_t *pxFlow;
    cepId_t xCepSourceId;
    flowAllocatorInstace_t *pxFlowAllocatorInstance;
    connectionId_t *pxConnectionId;

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    /* Create a flow object and fill using the event FlowRequest */
    pxFlow = prvFlowAllocatorNewFlow(pxFlowRequest);

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    pxConnectionId = pvRsMemAlloc(sizeof(*pxConnectionId));

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    /* Create a FAI and fill the struct properly*/
    pxFlowAllocatorInstance = pvRsMemAlloc(sizeof(*pxFlowAllocatorInstance));

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    if (!pxFlowAllocatorInstance)
    {
        LOGE(TAG_FA, "FAI was not allocated");
    }

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    pxFlowAllocatorInstance->eFaiState = eFAI_NONE;
    pxFlowAllocatorInstance->xPortId = xPortId;

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    LOGE(TAG_FA, "GetNeighbor");

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    /* Request to DFT the Next Hop, at the moment request to EnrollmmentTask */
    pxFlow->xRemoteAddress = xEnrollmentGetNeighborAddress(pxFlow->pxDestInfo->pcProcessName);

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    pxFlow->xSourcePortId = xPortId;

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    if (pxFlow->xRemoteAddress == 0)
    {
        LOGE(TAG_FA, "Error to get Next Hop");
    }
#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    /* Call EFCP to create an EFCP instance following the EFCP Config */
    LOGE(TAG_FA, "Calling EFCP");

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    xCepSourceId = xNormalConnectionCreateRequest(pxEfcpc, 1,
                                                  LOCAL_ADDRESS, pxFlow->xRemoteAddress, pxFlow->pxQosSpec->xQosId,
                                                  pxFlow->pxDtpConfig, pxFlow->pxDtcpConfig);
#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    if (xCepSourceId == 0)
    {
        LOGE(TAG_FA, "CepId was not create properly");
    }

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    /* Fill the Flow connectionId */
    pxConnectionId->xSource = xCepSourceId;
    pxConnectionId->xQosId = pxFlow->pxQosSpec->xQosId;
    pxConnectionId->xDestination = 0;

    pxFlow->pxConnectionId = pxConnectionId;

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    /* Send the flow message to the neighbor */
    // Serialize the pxFLow Struct into FlowMsg and Encode the FlowMsg as obj_value
    LOGE(TAG_FA, "EncodingFLow");

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    serObjectValue_t *pxObjVal = NULL;

    // ESP_LOGI(TAG_RIB, "Pointer Flow_t: %p", pxFlow);
#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    LOGE(TAG_FA, "Calling SerdesMsgFlow");
    pxObjVal = pxSerdesMsgFlowEncode(pxFlow);

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    // Send using the ribd_send_req M_Create
    LOGE(TAG_FA, "SendingFlow");

    if (!xRibdSendRequest(pxIpcpData, "Flow", "/fa/flows/key=1-33", -1, M_CREATE, 1, pxObjVal))
    {
        LOGE(TAG_FA, "It was a problem to send the request");
        // return pdFALSE;
    }
    LOGE(TAG_FA, "end");
}

bool_t xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue, int result)
{
    /*if (!pxSerObjValue)
    {
        ESP_LOGE(TAG_FA, "No Object Value");
        return pdFALSE;
    }*/

    if (result != 0)
    {
        LOGI(TAG_FA, "Was not possible to create the Flow...");
    }
    // Decode the FA message
    LOGI(TAG_FA, "CDAP Message Result: Flow allocated ");
    // Do something with the decode message.
    return true;
}
