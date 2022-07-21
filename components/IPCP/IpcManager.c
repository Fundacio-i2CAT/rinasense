/**
 * @file IPCManager.c
 * @author David Sarabia - i2CAT(you@domain.com)
 * @brief Handler IPCP events.
 * @version 0.1
 * @date 2022-02-23
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>
#include <string.h>

#include "rina_common_port.h"
#include "configRINA.h"
#include "IPCP_normal_defs.h"
#include "IPCP_normal_api.h"
#include "efcpStructures.h"
#include "du.h"
#include "ShimIPCP.h"
#include "EFCP.h"
#include "num_mgr.h"
#include "RINA_API_flows.h"
#include "FlowAllocator.h"
#include "FlowAllocator_api.h"

/* Table to store instances created */
static InstanceTableRow_t xInstanceTable[INSTANCES_IPCP_ENTRIES];

/**
 * @brief Initialize a IPC Manager object. Create a Port Id Manager
 * instance. Create an IPCP Id Manager. Finally Initialize the Factories
 * List
 *
 * @param pxIpcManager object created in the IPCP TASK.
 * @return BaseType_t
 */
bool_t xIpcManagerInit(ipcManager_t *pxIpcManager)
{
    if ((pxIpcManager->pxPidm = pxNumMgrCreate(MAX_PORT_ID)) == NULL)
        return false;

    if ((pxIpcManager->pxIpcpIdm = pxNumMgrCreate(MAX_IPCP_ID)) == NULL)
        return false;

    return true;
}

/**
 * @brief Add an IPCP instance into the Ipcp Instance Table.
 *
 * @param pxIpcpInstaceToAdd to added into the table
 */
void vIpcpManagerAddInstanceEntry(ipcpInstance_t *pxIpcpInstaceToAdd)
{
    num_t x = 0;

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++)
    {
        if (xInstanceTable[x].xActive == false)
        {
            xInstanceTable[x].pxIpcpInstance = pxIpcpInstaceToAdd;
            xInstanceTable[x].pxIpcpType = pxIpcpInstaceToAdd->xType;
            xInstanceTable[x].xIpcpId = pxIpcpInstaceToAdd->xId;
            xInstanceTable[x].xActive = true;

            break;
        }
    }
}

ipcpInstance_t *pxIpcManagerFindInstanceById(ipcpInstanceId_t xIpcpId)
{
    num_t x = 0;

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++)
    {
        if (xInstanceTable[x].xActive == true)
        {
            if (xInstanceTable[x].xIpcpId == xIpcpId)
            {
                LOGI(TAG_IPCPMANAGER, "Instance founded '%p'", xInstanceTable[x].pxIpcpInstance);
                return xInstanceTable[x].pxIpcpInstance;
                break;
            }
        }
    }
    return NULL;
}

/**
 * @brief Find an ipcp instances into the table by ipcp Type.
 *
 * @param xType Type of instance to be founded.
 * @return ipcpInstance_t* pointer to the ipcp instance.
 */

static ipcpInstance_t *pxIpcManagerFindInstanceByType(ipcpInstanceType_t xType)
{
    num_t x = 0;

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++)
    {
        if (xInstanceTable[x].xActive == true)
        {
            if (xInstanceTable[x].pxIpcpType == xType)
            {
                return xInstanceTable[x].pxIpcpInstance;
                break;
            }
        }
    }
    return NULL;
}

void vIcpManagerEnrollmentFlowRequest(ipcpInstance_t *pxShimInstance, NumMgr_t *pxPidm, name_t *pxIPCPName)
{
    portId_t xPortId;

    /*This should be proposed by the Flow Allocator?*/
    name_t *destinationInfo = pvRsMemAlloc(sizeof(*destinationInfo));
    destinationInfo->pcProcessName = REMOTE_ADDRESS_AP_NAME;
    destinationInfo->pcEntityName = "";
    destinationInfo->pcProcessInstance = REMOTE_ADDRESS_AP_INSTANCE;
    destinationInfo->pcEntityInstance = "";

    //xPortId = xPidmAllocate(pxPidm);
    xPortId = ulNumMgrAllocate(pxPidm);

    if (pxShimInstance->pxOps->flowAllocateRequest == NULL)
    {
        LOGI(TAG_IPCPNORMAL, "There is not Flow Allocate Request API");
    }

    if (pxShimInstance->pxOps->flowAllocateRequest(xPortId,
                                                   pxIPCPName,
                                                   destinationInfo,
                                                   pxShimInstance->pxData))
    {
        LOGI(TAG_IPCPNORMAL, "Flow Request processed by the Shim sucessfully");
        return true;
    }

    return false;
}

/* Handle a Flow allocation request sended by the User throught the RINA API.
 * Return a the Flow xPortID that the RINA API is going to use to send data. */
void vIpcpManagerAppFlowAllocateRequestHandle(NumMgr_t *pxPidm,
                                              struct efcpContainer_t *pxEfcpc,
                                              struct ipcpNormalData_t *pxIpcpData)
{
    portId_t xPortId;
    flowAllocateHandle_t *pxFlowAllocateRequest;

    pxFlowAllocateRequest = pvRsMemAlloc(sizeof(*pxFlowAllocateRequest));

    struct flowSpec_t *pxFlowSpecTmp;
    pxFlowSpecTmp = pvRsMemAlloc(sizeof(*pxFlowSpecTmp));
    pxFlowAllocateRequest->pxFspec = pxFlowSpecTmp;

    name_t *pxLocal, *pxRemote, *pxDIF;
    pxLocal = pvRsMemAlloc(sizeof(*pxLocal));
    pxRemote = pvRsMemAlloc(sizeof(*pxRemote));
    pxDIF = pvRsMemAlloc(sizeof(*pxDIF));

    pxLocal->pcProcessName = "Test";
    pxLocal->pcProcessInstance = "1";
    pxLocal->pcEntityName = "test";
    pxLocal->pcEntityInstance = "1";
    pxRemote->pcProcessName = "sensor1";
    pxRemote->pcProcessInstance = "";
    pxRemote->pcEntityName = "";
    pxRemote->pcEntityInstance = "";
    pxDIF->pcProcessName = NORMAL_DIF_NAME;
    pxDIF->pcProcessInstance = "";
    pxDIF->pcEntityName = "";
    pxDIF->pcEntityInstance = "";

    pxFlowAllocateRequest->pxDifName = pxDIF;
    pxFlowAllocateRequest->pxLocal = pxLocal;
    pxFlowAllocateRequest->pxRemote = pxRemote;
    pxFlowAllocateRequest->pxFspec->ulAverageBandwidth = 0;
    pxFlowAllocateRequest->pxFspec->ulAverageSduBandwidth = 0;
    pxFlowAllocateRequest->pxFspec->ulDelay = 0;
    pxFlowAllocateRequest->pxFspec->ulJitter = 0;
    pxFlowAllocateRequest->pxFspec->usLoss = 10000;
    pxFlowAllocateRequest->pxFspec->ulMaxAllowableGap = 10;
    pxFlowAllocateRequest->pxFspec->xOrderedDelivery = false;
    pxFlowAllocateRequest->pxFspec->ulUndetectedBitErrorRate = 0;
    pxFlowAllocateRequest->pxFspec->xPartialDelivery = true;
    pxFlowAllocateRequest->pxFspec->xMsgBoundaries = false;

    // dtpConfig_t *pxDtpCfg;
    // struct dtcpConfig_t *pxDtcpCfg;

    /* This shouldbe read from configRINA.h */
    // address_t xSource = 10;
    // address_t xDest = 3;
    // qosId_t xQosId = 1;

    // pxDtcpCfg = pvPortMalloc(sizeof(*pxDtcpCfg));
    // pxDtpCfg = pvPortMalloc(sizeof(*pxDtpCfg));

    xPortId = 33;

    // if (pxNormalInstance->pxOps->flowPrebind(pxNormalInstance->pxData, xPortId))
    //{

    // call to FlowAllocator.
    vFlowAllocatorFlowRequest(pxEfcpc, xPortId, pxFlowAllocateRequest, pxIpcpData);

    //}
    LOGE(TAG_IPCPMANAGER, "end");
}

void vIpcManagerRINAPackettHandler(struct ipcpNormalData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer);
void vIpcManagerRINAPackettHandler(struct ipcpNormalData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer)
{
    struct du_t *pxMessagePDU;

    pxMessagePDU = pvRsMemAlloc(sizeof(*pxMessagePDU));

    if (!pxMessagePDU)
    {
        LOGE(TAG_IPCPMANAGER, "pxMessagePDU was not allocated");
    }
    pxMessagePDU->pxNetworkBuffer = pxNetworkBuffer;

    LOGI(TAG_IPCPMANAGER, "The RINA packet is a managment packet");

    if (!xNormalDuEnqueue(pxData, 1, pxMessagePDU))
    {
        LOGI(TAG_IPCPMANAGER, "Drop frame because there is not enough memory space");
        xDuDestroy(pxMessagePDU);
    }
}

ipcpInstance_t *pxIpcManagerCreateShim(ipcManager_t *pxIpcManager);
ipcpInstance_t *pxIpcManagerCreateShim(ipcManager_t *pxIpcManager)
{

    ipcProcessId_t xIpcpId;

    xIpcpId = ulNumMgrAllocate(pxIpcManager->pxIpcpIdm);

    // add the shimInstance into the instance list.

    return pxShimWiFiCreate(xIpcpId);
}
