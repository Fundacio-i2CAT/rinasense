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

#include "common/rina_ids.h"
#include "common/num_mgr.h"

#include "configRINA.h"
#include "configSensor.h"

#include "IPCP_normal_api.h"
#include "IPCP_normal_defs.h"
#include "rina_common_port.h"
#include "efcpStructures.h"
#include "du.h"
#include "ShimIPCP.h"
#include "EFCP.h"
#include "RINA_API_flows.h"
#include "FlowAllocator_api.h"

ipcManager_t xIpcManager;

/**
 * @brief Initialize a IPC Manager object. Create a Port Id Manager
 * instance. Create an IPCP Id Manager.
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
void vIpcManagerAdd(ipcManager_t *pxIpcManager, struct ipcpInstance_t *pxIpcp)
{
    num_t x = 0;

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++) {
        if (pxIpcManager->xIpcpTable[x].xActive == false) {
            pxIpcManager->xIpcpTable[x].pxIpcpInstance = pxIpcp;
            pxIpcManager->xIpcpTable[x].xActive = true;
            return;
        }
    }
}

struct ipcpInstance_t *pxIpcManagerFindById(ipcManager_t *pxIpcManager,
                                            ipcpInstanceId_t xIpcpId)
{
    num_t x = 0;

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++) {
        if (pxIpcManager->xIpcpTable[x].xActive == true) {
            if (pxIpcManager->xIpcpTable[x].pxIpcpInstance->xId == xIpcpId)
                return pxIpcManager->xIpcpTable[x].pxIpcpInstance;
        }
    }

    return NULL;
}

/**
 * @brief Find an ipcp instances into the table by ipcp Type.
 *
 * @param xType Type of instance.
 * @return ipcpInstance_t* pointer to the ipcp instance.
 */
struct ipcpInstance_t *pxIpcManagerFindByType(ipcManager_t *pxIpcManager,
                                              ipcpInstanceType_t xType)
{
    num_t x = 0;

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++) {
        if (pxIpcManager->xIpcpTable[x].xActive == true) {
            if (pxIpcManager->xIpcpTable[x].pxIpcpInstance->xType == xType)
                return pxIpcManager->xIpcpTable[x].pxIpcpInstance;
        }
    }
    return NULL;
}

/**
 * Reserve a port ID
 */
portId_t unIpcManagerReservePort(ipcManager_t *pxIpcManager)
{
    portId_t unPort;

    RsAssert(pxIpcManager);

    if ((unPort = ulNumMgrAllocate(pxIpcManager->pxPidm)) == PORT_ID_WRONG)
        LOGW(TAG_IPCPMANAGER, "Out of available ports");

    return unPort;
}

/**
 * Release a port previously allocated in unIpcManagerReservePort.
 */
void vIpcManagerReleasePort(ipcManager_t *pxIpcManager, portId_t unPortId)
{
    RsAssert(pxIpcManager);

    if (!xNumMgrRelease(pxIpcManager->pxPidm, unPortId))
        LOGW(TAG_IPCPMANAGER, "Tried to release unreserved port %u", unPortId);
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

    if (pxShimInstance->pxOps->flowAllocateRequest(pxShimInstance,
                                                   pxIPCPName,
                                                   destinationInfo,
                                                   xN1PortId))
        LOGI(TAG_IPCPNORMAL, "Flow Request processed by the Shim sucessfully");
}

#if 0
void vIpcManagerRINAPackettHandler(struct ipcpInstanceData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer);
void vIpcManagerRINAPackettHandler(struct ipcpInstanceData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer)
{
/* ==3024816== Thread 2: */
/* ==3024816== Invalid read of size 8 */
/* ==3024816==    at 0x48753E4: xDuDestroy (du.c:22) */
/* ==3024816==    by 0x486D1E8: vIpcManagerRINAPackettHandler (IpcManager.c:156) */
/* ==3024816==    by 0x486E6E4: prvIPCPTask (IPCP.c:253) */
/* ==3024816==    by 0x4943849: start_thread (pthread_create.c:442) */
/* ==3024816==    by 0x49C630F: clone (clone.S:100) */
/* ==3024816==  Address 0x4ac4f40 is 16 bytes inside a block of size 32 free'd */
/* ==3024816==    at 0x484617B: free (vg_replace_malloc.c:872) */
/* ==3024816==    by 0x4875435: xDuDestroy (du.c:27) */
/* ==3024816==    by 0x487482E: xRmtReceive (Rmt.c:278) */
/* ==3024816==    by 0x486DAE0: xNormalDuEnqueue (normalIPCP.c:434) */
/* ==3024816==    by 0x486D1A8: vIpcManagerRINAPackettHandler (IpcManager.c:153) */
/* ==3024816==    by 0x486E6E4: prvIPCPTask (IPCP.c:253) */
/* ==3024816==    by 0x4943849: start_thread (pthread_create.c:442) */
/* ==3024816==    by 0x49C630F: clone (clone.S:100) */
/* ==3024816==  Block was alloc'd at */
/* ==3024816==    at 0x48437B4: malloc (vg_replace_malloc.c:381) */
/* ==3024816==    by 0x486D14D: vIpcManagerRINAPackettHandler (IpcManager.c:144) */
/* ==3024816==    by 0x486E6E4: prvIPCPTask (IPCP.c:253) */
/* ==3024816==    by 0x4943849: start_thread (pthread_create.c:442) */
/* ==3024816==    by 0x49C630F: clone (clone.S:100) */

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
#endif

struct ipcpInstance_t *pxIpcManagerCreateShim()
{
    ipcProcessId_t xIpcpId;

    xIpcpId = ulNumMgrAllocate(xIpcManager.pxIpcpIdm);

    // add the shimInstance into the instance list.

    return pxShimWiFiCreate(xIpcpId);
}
