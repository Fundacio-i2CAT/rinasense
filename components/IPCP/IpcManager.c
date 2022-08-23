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

#include "configRINA.h"
#include "common/num_mgr.h"

#include "rina_common_port.h"
#include "IPCP_normal_defs.h"
#include "IPCP_normal_api.h"
#include "efcpStructures.h"
#include "du.h"
#include "ShimIPCP.h"
#include "EFCP.h"
#include "RINA_API_flows.h"
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
void vIpcpManagerAddInstanceEntry(struct ipcpInstance_t *pxIpcpInstaceToAdd)
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

struct ipcpInstance_t *pxIpcManagerFindInstanceById(ipcpInstanceId_t xIpcpId)
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

static struct ipcpInstance_t *pxIpcManagerFindInstanceByType(ipcpInstanceType_t xType)
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


void vIcpManagerEnrollmentFlowRequest(struct ipcpInstance_t *pxShimInstance, portId_t xN1PortId, name_t *pxIPCPName)
{
    /*This should be proposed by the Flow Allocator?*/
    name_t *destinationInfo = pvRsMemAlloc(sizeof(*destinationInfo));
    destinationInfo->pcProcessName = REMOTE_ADDRESS_AP_NAME;
    destinationInfo->pcEntityName = "";
    destinationInfo->pcProcessInstance = REMOTE_ADDRESS_AP_INSTANCE;
    destinationInfo->pcEntityInstance = "";

    if (pxShimInstance->pxOps->flowAllocateRequest == NULL)
    {
        LOGI(TAG_IPCPNORMAL, "There is not Flow Allocate Request API");
    }

    if (pxShimInstance->pxOps->flowAllocateRequest(pxShimInstance->pxData,
                                                   pxIPCPName,
                                                   destinationInfo,
                                                   xN1PortId))
        LOGI(TAG_IPCPNORMAL, "Flow Request processed by the Shim sucessfully");
}

void vIpcManagerRINAPackettHandler(struct ipcpInstanceData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer);
void vIpcManagerRINAPackettHandler(struct ipcpInstanceData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer)
{
    struct du_t *pxMessagePDU;

    pxMessagePDU = pvRsMemAlloc(sizeof(*pxMessagePDU));

    if (!pxMessagePDU) {
        LOGE(TAG_IPCPMANAGER, "pxMessagePDU was not allocated");
        return;
    }

    pxMessagePDU->pxNetworkBuffer = pxNetworkBuffer;

    if (!xNormalDuEnqueue(pxData, 1, pxMessagePDU)) // must change this
    {
        LOGI(TAG_IPCPMANAGER, "Drop frame because there is not enough memory space");
        xDuDestroy(pxMessagePDU);
    }
}

struct ipcpInstance_t *pxIpcManagerCreateShim(ipcManager_t *pxIpcManager);
struct ipcpInstance_t *pxIpcManagerCreateShim(ipcManager_t *pxIpcManager)
{

    ipcProcessId_t xIpcpId;

    xIpcpId = ulNumMgrAllocate(pxIpcManager->pxIpcpIdm);

    // add the shimInstance into the instance list.

    return pxShimWiFiCreate(xIpcpId);
}
