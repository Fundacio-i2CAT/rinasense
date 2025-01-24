#include <stdio.h>
#include <string.h>

#include "common/list.h"
#include "common/rina_name.h"
#include "common/rina_ids.h"
#include "common/mac.h" //This is more common in the ShimIPCPs
#include "portability/port.h"

#include "configRINA.h"
#include "configSensor.h"

#include "shim.h"
#include "shim_IPCP_flows.h"
#include "shim_IPCP_events.h"

#include "ieee802154_NetworkInterface.h"
#include "IPCP_instance.h"
#include "IPCP_manager.h"
#include "ieee802154_IPCP.h"

#include "Arp826.h"
#include "du.h"

/* IPCP Shim Instance particular data structure */
struct ipcpInstanceData_t
{

    RsListItem_t xInstanceListItem;
    ipcProcessId_t xId;

    /* IPC Process name */
    name_t *pxName;
    name_t *pxDifName;
    string_t pcInterfaceName;

    MACAddress_t *pxPhyDev;
    struct flowSpec_t *pxFspec;

    /* The IPC Process using ieee 802.15.4 */
    name_t *pxAppName;
    name_t *pxDafName;

    /* RINARP related */
    struct rinarpHandle_t *pxAppHandle;
    struct rinarpHandle_t *pxDafHandle;

    /* Flow control between this IPCP and the associated netdev. */
    unsigned int ucTxBusy;
};

/** @brief Enrollment operation must be called by the IPCP manager after initializing
 * the shim IPCP task.
 * @param pxShimInstanceData is a pointer in the IPCP instance data. The IPCP instance is stored at the
 * IPCP manager list.
 */
bool_t xShim802154EnrollToDIF(struct ipcpInstanceData_t *pxShimInstanceData)
{
    LOGI(TAG_SHIM_802154, "Enrolling to DIF");

    /* Initialization of 802.15.4 interface */

    if (xIeee802154NetworkInterfaceInitialise(pxShimInstanceData->pxPhyDev))
    {
        /* Initialize ARP Cache */
        vARPInitCache();

        /* If Coordinator then init PAN, otherwise try to connect to Coordinator */
        if (xIeee802154NetworkInterfaceConnect())
        {
            LOGI(TAG_SHIM_802154, "Enrolled To DIF %x", ieee802154_PANID_SOURCE);
            return true;
        }

        LOGE(TAG_SHIM_802154, "Failed to enroll to DIF ");
        return false;
    }

    LOGE(TAG_SHIM_802154, "Failed to enroll to DIF");

    return false;
}
/** @brief Primitive invoked before all other functions. The N+1 DIF calls the application register:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "-": ProcessName-ProcessInstance-EntityName-EntityInstance
 * - (Update LocalAddressProtocol which is part of the ARP module).
 * It is assumed only there is going to be one IPCP process over the Shim-DIF.
 * pxAppName, and pxDafName come from the Normal IPCP (user_app), while the pxData refers to
 * the shimWiFi ipcp instance.
 * @return a pdTrue if Success or pdFalse Failure.
 * */
bool_t xShim802154ApplicationRegister(struct ipcpInstanceData_t *pxData, name_t *pxAppName, name_t *pxDafName)
{
    LOGI(TAG_802154, "Registering Application");

    gpa_t *pxPa;
    gha_t *pxHa;

    if (!pxData)
    {
        LOGI(TAG_SHIM_802154, "Data no valid ");
        return false;
    }
    if (!pxAppName)
    {
        LOGI(TAG_SHIM_802154, "Name no valid ");
        return false;
    }
    if (pxData->pxAppName != NULL)
    {
        LOGI(TAG_SHIM_802154, "AppName should not exist");
        return false;
    }

    pxData->pxAppName = pxRstrNameDup(pxAppName);

    if (!pxData->pxAppName)
    {
        LOGI(TAG_SHIM_802154, "AppName not created ");
        return false;
    }

    pxPa = pxNameToGPA(pxAppName);

    if (!xIsGPAOK(pxPa))
    {
        LOGI(TAG_SHIM_802154, "Protocol Address is not OK ");
        vRstrNameFini(pxData->pxAppName);
        return false;
    }

    if (!pxData->pxPhyDev)
    {
        xIeee802154NetworkInterfaceInitialise(pxData->pxPhyDev);
    }

    pxHa = pxCreateGHA(MAC_ADDR_802_15_4, pxData->pxPhyDev);

    if (!xIsGHAOK(pxHa))
    {
        LOGI(TAG_SHIM_802154, "Hardware Address is not OK ");
        vRstrNameFini(pxData->pxAppName);
        vGHADestroy(pxHa);
        return false;
    }

    pxData->pxAppHandle = pxARPAdd(pxPa, pxHa);

    if (!pxData->pxAppHandle)
    {
        // destroy all
        LOGI(TAG_SHIM_802154, "APPHandle was not created ");
        vGPADestroy(pxPa);
        vGHADestroy(pxHa);
        vRstrNameFini(pxData->pxAppName);
        return false;
    }

    // vShimGPADestroy( pa );

    pxData->pxDafName = pxRstrNameDup(pxDafName);

    if (!pxData->pxDafName)
    {
        LOGE(TAG_SHIM_802154, "Removing ARP Entry for DAF");
        xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
        pxData->pxAppHandle = NULL;
        vRstrNameFree(pxData->pxAppName);
        vGHADestroy(pxHa);
        return false;
    }

    pxPa = pxNameToGPA(pxDafName);

    if (!xIsGPAOK(pxPa))
    {
        LOGE(TAG_SHIM_802154, "Failed to create gpa");
        xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
        pxData->pxAppHandle = NULL;
        vRstrNameFree(pxData->pxDafName);
        vRstrNameFree(pxData->pxAppName);
        vGHADestroy(pxHa);
        return false;
    }

    pxData->pxDafHandle = pxARPAdd(pxPa, pxHa);

    if (!pxData->pxDafHandle)
    {
        LOGE(TAG_SHIM_802154, "Failed to register DAF in ARP");
        xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
        pxData->pxAppHandle = NULL;
        vRstrNameFree(pxData->pxAppName);
        vRstrNameFree(pxData->pxDafName);
        vGPADestroy(pxPa);
        vGHADestroy(pxHa);
        return false;
    }

    vARPPrintCache();

    return true;
}

/**
 * @brief FlowAllocateRequest. The N+1 DIF requests for allocating a flow. So, it sends the
 * name-info (source info and destination info).
 * - Check if there is a flow established (eALLOCATED), or a flow pending between the
 * source and destination application (ePENDING),
 * - If stated is eNULL then RINA_xARPAdd is called.
 *
 * @param xId PortId created in the IPCManager and allocated to this flow
 * @param pxUserIpcp IPCP who is going to use the flow
 * @param pxSourceInfo Source Information
 * @param pxDestinationInfo Destination Information
 * @param pxData Shim IPCP Data to update during the flow allocation request
 * @return BaseType_t
 */
bool_t xShim802154FlowAllocateRequest(struct ipcpInstanceData_t *pxData, struct ipcpInstance_t *pxUserIpcp,
                                      name_t *pxSourceInfo,
                                      name_t *pxDestinationInfo,
                                      portId_t xPortId)
{

    LOGI(TAG_SHIM_802154, "New flow allocation request");

    shimFlow_t *pxFlow;

    if (!pxData)
    {
        LOGE(TAG_SHIM_802154, "Bogus data passed, bailing out");
        return false;
    }

    if (!pxSourceInfo)
    {
        LOGE(TAG_SHIM_802154, "Bogus data passed, bailing out");
        return false;
    }
    if (!pxDestinationInfo)
    {
        LOGE(TAG_SHIM_802154, "Bogus data passed, bailing out");
        return false;
    }

    if (!is_port_id_ok(xPortId))
    {
        LOGE(TAG_SHIM_802154, "Bogus data passed, bailing out");
        return false;
    }

    LOGI(TAG_SHIM_802154, "Finding Flows");
    pxFlow = prvShimFindFlowByPortId(pxData, xPortId);

    if (!pxFlow)
    {
        pxFlow = pvRsMemAlloc(sizeof(*pxFlow));
        if (!pxFlow)
            return false;

        pxFlow->xPortId = xPortId;
        pxFlow->ePortIdState = ePENDING;
        pxFlow->pxDestPa = pxNameToGPA(pxDestinationInfo);
        pxFlow->pxUserIpcp = pxUserIpcp;

        if (!xIsGPAOK(pxFlow->pxDestPa))
        {
            LOGE(TAG_SHIM_802154, "Destination protocol address is not OK");
            prvShimUnbindDestroyFlow(pxData, pxFlow);

            return false;
        }

        // Register the flow in a list or in the Flow allocator
        LOGI(TAG_SHIM_802154, "Created Flow: %p, portID: %d, portState: %d", pxFlow, pxFlow->xPortId, pxFlow->ePortIdState);
        // vRsListInitItem(&pxFlow->xFlowItem, pxFlow);
        // vRsListInsert(&pxData->xFlowsList, &pxFlow->xFlowItem);
        vShimFlowAdd(pxFlow);

        pxFlow->pxSduQueue = prvShimCreateQueue();
        if (!pxFlow->pxSduQueue)
        {
            LOGE(TAG_SHIM_802154, "Destination protocol address is not ok");
            prvShimUnbindDestroyFlow(pxData, pxFlow);
            return false;
        }

        //************ RINAARP RESOLVE GPA

        if (!xARPResolveGPA(pxFlow->pxDestPa, pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa))
        {
            prvShimUnbindDestroyFlow(pxData, pxFlow);
            return false;
        }
    }
    else if (pxFlow->ePortIdState == ePENDING)
    {
        LOGE(TAG_SHIM_802154, "Port-id state is already pending");
    }
    else
    {
        LOGE(TAG_SHIM_802154, "Allocate called in a wrong state");
        return false;
    }

    return true;
}

/* Structure to define the IPCP instance Operations */
static struct ipcpInstanceOps_t xShimIeee802154Ops = {
    .flowAllocateRequest = xShim802154FlowAllocateRequest, // ok
    .flowAllocateResponse = NULL,                          // xShimFlowAllocateResponse, // ok
    .flowDeallocate = NULL,                                // xShimFlowDeallocate,             // ok
    .flowPrebind = NULL,                                   // ok
    .flowBindingIpcp = NULL,                               // ok
    .flowUnbindingIpcp = NULL,                             // ok
    .flowUnbindingUserIpcp = NULL,                         // ok
    .nm1FlowStateChange = NULL,                            // ok

    .applicationRegister = xShim802154ApplicationRegister, // ok
    .applicationUnregister = NULL,                         // xShimApplicationUnregister, // ok

    .assignToDif = NULL,     // ok
    .updateDifConfig = NULL, // ok

    .connectionCreate = NULL,        // ok
    .connectionUpdate = NULL,        // ok
    .connectionDestroy = NULL,       // ok
    .connectionCreateArrived = NULL, // ok
    .connectionModify = NULL,        // ok

    .duEnqueue = NULL, // ok
    .duWrite = NULL,   // xShimSDUWrite,

    .mgmtDuWrite = NULL, // ok
    .mgmtDuPost = NULL,  // ok

    .pffAdd = NULL,    // ok
    .pffRemove = NULL, // ok
    //.pff_dump                  = NULL,
    //.pff_flush                 = NULL,
    //.pff_modify		   		   = NULL,

    //.query_rib		  		   = NULL,

    .ipcpName = NULL, // ok
    .difName = NULL,  // ok
    //.ipcp_id		  		   = NULL,

    //.set_policy_set_param      = NULL,
    //.select_policy_set         = NULL,
    //.update_crypto_state	   = NULL,
    //.address_change            = NULL,
    //.dif_name		   		   = NULL,
    .maxSduSize = NULL};

/***
 * @brief Create the Shim WiFi instance. This primitive must called by the Manager, who is
 * going to create. pxShimCreate is generic to add new Shims with the same process.
 * @param xIpcpId Unique Id provided by the Manager.
 * @return ipcpInstance structure for registering into the Manager IPCP List.
 * */

struct ipcpInstance_t *pxShim802154Create(ipcProcessId_t xIpcpId)
{
    struct ipcpInstance_t *pxInst;
    struct ipcpInstanceData_t *pxInstData;
    struct flowSpec_t *pxFspec;
    string_t pcInterfaceName = SHIM_INTERFACE_802154;
    name_t *pxName;
    MACAddress_t *pxPhyDev;

    pxPhyDev = pvRsMemAlloc(sizeof(*pxPhyDev));
    if (!pxPhyDev)
    {
        LOGE(TAG_SHIM_802154, "Failed to allocate memory for WiFi shim instance");
        return NULL;
    }

    pxPhyDev->ucBytes[0] = 0x00;
    pxPhyDev->ucBytes[1] = 0x00;
    pxPhyDev->ucBytes[2] = 0x00;
    pxPhyDev->ucBytes[3] = 0x00;
    pxPhyDev->ucBytes[4] = 0x00;
    pxPhyDev->ucBytes[5] = 0x00;
    pxPhyDev->ucBytes[6] = 0x00;
    pxPhyDev->ucBytes[7] = 0x00;

    /* Create an instance */
    pxInst = pvRsMemAlloc(sizeof(*pxInst));
    if (!pxInst)
        return NULL;

    /* Create Data instance and Flow Specifications*/
    pxInstData = pvRsMemAlloc(sizeof(*pxInstData));
    if (!pxInstData)
        return NULL;

    pxInst->pxData = pxInstData;
    pxFspec = pvRsMemAlloc(sizeof(*pxFspec));
    pxInst->pxData->pxFspec = pxFspec;

    /*Create Dif Name and Daf Name*/
    pxName = pvRsMemAlloc(sizeof(*pxName));
    /*pxDafName = pvPortMalloc(sizeof(struct ipcpInstanceData_t));*/

    pxName->pcProcessName = SHIM_PROCESS_NAME;
    pxName->pcEntityName = SHIM_ENTITY_NAME;
    pxName->pcProcessInstance = SHIM_PROCESS_INSTANCE;
    pxName->pcEntityInstance = SHIM_ENTITY_INSTANCE;

    pxInst->pxData->pxAppName = NULL;
    pxInst->pxData->pxDafName = NULL;

    /*Filling the ShimWiFi instance properly*/
    pxInst->pxData->pxName = pxName;
    pxInst->pxData->xId = xIpcpId;
    pxInst->pxData->pxPhyDev = pxPhyDev;
    pxInst->pxData->pcInterfaceName = pcInterfaceName;

    pxInst->pxData->pxFspec->ulAverageBandwidth = 0;
    pxInst->pxData->pxFspec->ulAverageSduBandwidth = 0;
    pxInst->pxData->pxFspec->ulDelay = 0;
    pxInst->pxData->pxFspec->ulJitter = 0;
    pxInst->pxData->pxFspec->ulMaxAllowableGap = -1;
    pxInst->pxData->pxFspec->ulMaxSduSize = 1500; // change
    pxInst->pxData->pxFspec->xOrderedDelivery = 0;
    pxInst->pxData->pxFspec->xPartialDelivery = 1;
    pxInst->pxData->pxFspec->ulPeakBandwidthDuration = 0;
    pxInst->pxData->pxFspec->ulPeakSduBandwidthDuration = 0;
    pxInst->pxData->pxFspec->ulUndetectedBitErrorRate = 0;

    pxInst->pxOps = &xShimIeee802154Ops; // change
    pxInst->xType = eShim802154;
    pxInst->xId = xIpcpId;

    /*Initialialise flows list*/
    vShimFlowListInit();
    // vRsListInit((&pxInst->pxData->xFlowsList));

    LOGI(TAG_SHIM_802154, "Instance Created: %p, IPCP id:%d, Type: %d", pxInst, pxInst->xId, pxInst->xType);

    return pxInst;
}

/*static struct shim_ops shimWiFi_ops = {
    //.init = tcp_udp_init,
    //.fini = tcp_udp_fini,
    .create = pxShimWiFiCreate
    //.destroy = tcp_udp_destroy,
};*/