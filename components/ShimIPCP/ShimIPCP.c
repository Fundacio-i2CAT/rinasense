#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "portability/port.h"
#include "common/eth.h"
#include "common/rsrc.h"
#include "common/list.h"
#include "common/netbuf.h"
#include "common/simple_queue.h"
#include "common/mac.h"
#include "common/rina_gpha.h"
#include "common/rina_name.h"
#include "common/rina_ids.h"

#include "configRINA.h"
#include "configSensor.h"

#include "ARP826_defs.h"
#include "ARP826.h"
#include "IPCP.h"
#include "IPCP_frames.h"
#include "IPCP_instance.h"
#include "IPCP_api.h"
#include "IPCP_events.h"
#include "NetworkInterface.h"
#include "pci.h"
#include "du.h"
#include "IpcManager.h"
#include "rina_common_port.h"
#include "ShimIPCP.h"
#include "ShimIPCP_instance.h"

#if 0
static shimFlow_t *prvShimFindFlowByPortId(struct ipcpInstanceData_t *pxData, portId_t unPort)
{
	shimFlow_t *pxFlow;
	RsListItem_t *pxListItem;

    RsAssert(pxData);

    pthread_mutex_lock(&pxData->xLock);

	if (!unRsListLength(&pxData->xFlowsList)) {
        pthread_mutex_unlock(&pxData->xLock);
		return NULL;
    }

    pxListItem = pxRsListGetFirst(&pxData->xFlowsList);

	while (pxListItem != NULL) {
        pxFlow = (shimFlow_t *)pxRsListGetItemOwner(pxListItem);

        LOGD(TAG_RIB, "Looking at flow port %u", pxFlow->unPort);

        if (pxFlow->unPort == unPort) {
            pthread_mutex_unlock(&pxData->xLock);
            return pxFlow;
        }

        pxListItem = pxRsListGetNext(pxListItem);
	}

    pthread_mutex_unlock(&pxData->xLock);

	return NULL;
}
#endif

static shimFlow_t *prvShimFindFlowByPortId(struct ipcpInstanceData_t *pxData, portId_t unPort)
{
    shimFlow_t *flow;

    pthread_mutex_lock(&pxData->xLock);
    flow = pxData->xFlows[unPort];
    pthread_mutex_unlock(&pxData->xLock);
    return flow;
}

#if 0
static shimFlow_t *prvShimFindFlowByGHA(struct ipcpInstanceData_t *pxData, gha_t *pxHa)
{
    shimFlow_t *pxFlow;
    RsListItem_t *pxListItem;

    RsAssert(pxData);
    RsAssert(pxHa);

    pthread_mutex_lock(&pxData->xLock);

    if (!unRsListLength(&pxData->xFlowsList)) {
        pthread_mutex_unlock(&pxData->xLock);
        return NULL;
    }

    pxListItem = pxRsListGetFirst(&pxData->xFlowsList);

    while (pxListItem != NULL) {
        pxFlow = (shimFlow_t *)pxRsListGetItemOwner(pxListItem);

        if (xGHACmp(pxFlow->pxDestHa, pxHa)) {
            pthread_mutex_unlock(&pxData->xLock);
            return pxFlow;
        }

        pxListItem = pxRsListGetNext(pxListItem);
    }

    pthread_mutex_unlock(&pxData->xLock);

    return NULL;
}
#endif

static shimFlow_t *prvShimFindFlowByGHA(struct ipcpInstanceData_t *pxData, gha_t *pxHa)
{
    pthread_mutex_lock(&pxData->xLock);

    for (size_t i = 0; i < sizeof(pxData->xFlows) / sizeof(shimFlow_t *); i++) {
        if (pxData->xFlows[i] && xGHACmp(pxData->xFlows[i]->pxDestHa, pxHa)) {
            pthread_mutex_unlock(&pxData->xLock);
            return pxData->xFlows[i];
        }
    }

    pthread_mutex_unlock(&pxData->xLock);

    return NULL;
}

static bool_t prvShimFlowDestroy(struct ipcpInstanceData_t *xData, shimFlow_t *xFlow)
{
    #if 0
	if (xFlow->pxSduQueue)
		vRsQueueDelete(xFlow->pxSduQueue->xQueue);
    #endif

	vRsMemFree(xFlow);

	return true;
}

static bool_t prvShimUnbindDestroyFlow(struct ipcpInstanceData_t *xData,
									   shimFlow_t *xFlow)
{

	/*
	if (flow->user_ipcp) {
		ASSERT(flow->user_ipcp->ops);
		flow->user_ipcp->ops->
		flow_unbinding_ipcp(flow->user_ipcp->data,
				flow->port_id);
	}*/
	// Check this
	LOGI(TAG_SHIM, "Shim-WiFi unbinded port: %u", xFlow->unPort);

	if (prvShimFlowDestroy(xData, xFlow)) {
		LOGE(TAG_SHIM, "Failed to destroy shim flow");
		return false;
	}

	return true;
}

/**
 * Return an ethernet frame to its source. This always free the
 * buffer.
 */
static void prvReturnEthernetFrame(netbuf_t *pxNbFrame)
{
    EthernetHeader_t *pxEthernetHeader;
    MACAddress_t xTmpMac;
    MACAddress_t *pxDstMac, *pxSrcMac;

    RsAssert(eNetBufType(pxNbFrame) == NB_ETH_HDR);

    /* Switch source and address. */
    pxEthernetHeader = (EthernetHeader_t *)pvNetBufPtr(pxNbFrame);

    pxDstMac = &pxEthernetHeader->xDestinationAddress;
    pxSrcMac = &pxEthernetHeader->xSourceAddress;

    memcpy(&xTmpMac, pxDstMac, sizeof(MACAddress_t));
    memcpy(pxDstMac, pxSrcMac, sizeof(MACAddress_t));
    memcpy(pxSrcMac, &xTmpMac, sizeof(MACAddress_t));

    /* Send the packet back. No need to go through the IPCP */
    xNetworkInterfaceOutput(pxNbFrame);
}

static eFrameProcessingResult_t prvShimHandleARPFrame(struct ipcpInstance_t *pxSelf, netbuf_t *pxNbFrame)
{
    eFrameProcessingResult_t eReturned;

    /* The Ethernet frame contains an ARP packet. */
    LOGI(TAG_WIFI, "Handling ARP frame");

    if (unNetBufSize(pxNetBufNext(pxNbFrame)) >= sizeof(ARPStaticHeader_t))
        eReturned = eARPProcessPacket(&pxSelf->pxData->xARP, pxNbFrame);
    else {
        /* Drop invalid ARP packets */
        LOGW(TAG_IPCPMANAGER, "Discarding invalid ARP packet");
        eReturned = eReleaseBuffer;
    }

    return eReturned;
}

static eFrameProcessingResult_t prvShimHandleRinaFrame(struct ipcpInstance_t *pxSelf, netbuf_t *pxNbFrame)
{
    uint8_t *ptr;
    size_t uxRinaLength;
    shimFlow_t *pxFlow;
    EthernetHeader_t *eth;
    gha_t *pxSrcHa;
	RINAStackEvent_t xEvent;
    du_t *pxDu;

    /* This function is inspired by what is coded in the functions
     * eth_rcv_process_packet and eth_rcv_worker in the
     * shim-eth-core.c in IRATI. */

    LOGI(TAG_WIFI, "Handling RINA frame");

    eth = (EthernetHeader_t *)pvNetBufPtr(pxNbFrame);

    pxSrcHa = pxCreateGHA(MAC_ADDR_802_3, &eth->xSourceAddress);

    /* Look for an flow to the destination. */
    pxFlow = prvShimFindFlowByGHA(pxSelf->pxData, pxSrcHa);

    /* If there is no such flow, we'll have to create it. */
    if (!pxFlow) {
        if (!(pxFlow = pvRsMemAlloc(sizeof(shimFlow_t)))) {
            LOGE(TAG_SHIM, "Failed to allocate memory for flow structure");
            goto err;
        }

        /* Initialise the flow as allocated because we actually have
         * the source address. */
        pxFlow->ePortIdState = eALLOCATED;
        pxFlow->pxDestHa = pxSrcHa;

        /* Make sure we do not free pxSrcHa as it is owned by the flow
           now. */
        pxSrcHa = NULL;

        /* find IPCP matching this target name */
        /* FIXME: For now this is always going to be the single normal
         * IPC */

        /* IF THE SOURCE ADDRESS ISN'T IN THE ARP TABLE, WE NEED TO DO
         * A REQUEST AND WAIT FOR THAT REQUEST TO GET RESOLVED BEFORE
         * BINDING THE FLOW TO THE PORT. SEE rinarp_resolve_handler. */

        /* ENQUEUE THE PACKETS WHILE THE PORT BINDING IS PENDING */

        /* EMPTY THE QUEUE WHEN THE ARP REQUEST REPLY HAS BEEN
         * RECEIVED */

        /* SEE ARP CODE */

        pthread_mutex_lock(&pxSelf->pxData->xLock);

        /* Reserve a port number */
        pxFlow->unPort = unIpcManagerReservePort(&xIpcManager);

        /* Create the flow in the IPCP. */
        //vRsListInitItem(&pxFlow->xFlowItem, &pxFlow->xFlowItem);
        //vRsListInsert(&pxSelf->pxData->xFlowsList,
        //&pxFlow->xFlowItem);
        pxSelf->pxData->xFlows[pxFlow->unPort] = pxFlow;

        pthread_mutex_unlock(&pxSelf->pxData->xLock);

        {
            bool_t xStatus;
            struct ipcpInstance_t *pxIpcp;

            RsAssert((pxIpcp = pxIpcManagerFindByType(&xIpcManager, eNormal)));

            CALL_IPCP_CHECK(xStatus, pxIpcp, flowBindingIpcp, pxFlow->unPort, pxSelf) {
                LOGE(TAG_SHIM, "Failed to bind new flow to port %d", pxFlow->unPort);
                goto err;
            }
        }
    }
    else {
        if (pxFlow->ePortIdState == ePENDING) {
            /* Bind the port. */
        }
        else if (pxFlow->ePortIdState == eALLOCATED) {
            /* Deliver the packet to the right layer. */
        }
        else
            LOGE(TAG_SHIM, "Incomprehensible port state %d", pxFlow->ePortIdState);
    }

    /* Free the first part of the buffer */
    pxDu = pxNetBufNext(pxNbFrame);
    vNetBufFree(pxNbFrame);

    /* Notify the IPCM that something was posted for the flow */
    xEvent.eEventType = eRinaRxEvent;
    xEvent.xData.UN = pxFlow->unPort;
    xEvent.xData2.DU = pxDu;

    xSendEventStructToIPCPTask(&xEvent, 1000);

    // Release the buffer with the Ethernet header, it is not needed any more
    // ESP_LOGE(TAG_ARP, "Releasing Buffer to copy the RINA PDU: ETH_P_RINA");
    // vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);

    // must be void function

    return eFrameConsumed;

    err:
    if (pxSrcHa)
        vGHADestroy(pxSrcHa);

    return eFrameConsumed;
}


/* Lifecycle functions */

bool_t xShimStart(struct ipcpInstance_t *pxSelf)
{
    /* Initialization of the network interface. */
	if (xNetworkInterfaceInitialise(pxSelf, &pxSelf->pxData->xPhyDev)) {

		/* Initialize ARP Cache */
		if (!xARPInit(&pxSelf->pxData->xARP)) {
            LOGE(TAG_SHIM, "Failed to initialize ARP for ethernet shim");
            return false;
        }

	} else {
        LOGE(TAG_SHIM, "Failed to initialize shim");
        return false;
    }

    return true;
}

bool_t xShimStop(struct ipcpInstance_t *pxSelf)
{
    /* FIXME: IMPLEMENT XSHIMSTOP */
    return true;
}

bool_t xShimEnable(struct ipcpInstance_t *pxSelf)
{
    /* Connect to remote point (WiFi AP) */
    if (xNetworkInterfaceConnect()) {
        LOGI(TAG_SHIM, "Enrolled to pshim DIF %s", SHIM_DIF_NAME);

        RINAStackEvent_t xEnrollEvent = {
            .eEventType = eShimEnrolledEvent,
            .xData.PV = pxSelf
        };
        xSendEventStructToIPCPTask(&xEnrollEvent, 50 * 1000);

    } else {
        LOGE(TAG_SHIM, "Failed to enroll to shim DIF %s", SHIM_DIF_NAME);
        return false;
    }

    return true;
}

bool_t xShimDisable(struct ipcpInstance_t *pxSelf)
{
    /* FIXME: IMPLEMENT XSHIMDISABLE */
    return true;
}

/*-------------------------------------------*/
/* @brief Deallocate a flow
 * */
bool_t xShimFlowDeallocate(struct ipcpInstance_t *pxSelf, portId_t xId)
{
	shimFlow_t *xFlow;

    RsAssert(pxSelf);
    RsAssert(is_port_id_ok(xId));

	xFlow = prvShimFindFlowByPortId(pxSelf->pxData, xId);
	if (!xFlow) {
		LOGE(TAG_SHIM, "Flow does not exist, cannot remove");
		return false;
	}

	return prvShimUnbindDestroyFlow(pxSelf->pxData, xFlow);
}

/**
 * @brief FlowAllocateRequest (naming-info). Naming-info about the destination.
 * Primitive invoked by the IPCP task event_handler:
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
bool_t xShimFlowAllocateRequest(struct ipcpInstance_t *pxSelf,
								rname_t *pxSourceInfo,
								rname_t *pxDestinationInfo,
								portId_t unPort)
{
	shimFlow_t *pxFlow;
    stringbuf_t pcNameBuf[64];

    RsAssert(pxSelf);
    RsAssert(pxSourceInfo);
    RsAssert(pxDestinationInfo);
    RsAssert(is_port_id_ok(unPort));

    vNameToStringBuf(pxSourceInfo, pcNameBuf, sizeof(pcNameBuf));
	LOGI(TAG_SHIM, "New flow allocation request from %s", pcNameBuf);

	pxFlow = prvShimFindFlowByPortId(pxSelf->pxData, unPort);

	if (!pxFlow) {
		pxFlow = pvRsMemAlloc(sizeof(*pxFlow));
		if (!pxFlow) {
            LOGE(TAG_SHIM, "Failed to allocate memory for flow structure");
			return false;
        }

        pxFlow->unPort = unPort;
		pxFlow->ePortIdState = ePENDING;

        /* Create a name from the protocol address. */
		if (!(pxFlow->pxDestPa = pxNameToGPA(pxDestinationInfo))) {
            LOGE(TAG_SHIM, "Failed to create name from protocol address");
            prvShimUnbindDestroyFlow(pxSelf->pxData, pxFlow);
            return false;
        }

		/* Register the flow in a list or in the Flow allocator */
#if 0
        vRsListInitItem(&pxFlow->xFlowItem, pxFlow);
        vRsListInsert(&pxSelf->pxData->xFlowsList, &pxFlow->xFlowItem);
#endif


        /* Create a packet queue */
        if (!xSimpleQueueInit("Shim Flow Queue", &pxFlow->xSduQueue)) {
            LOGE(TAG_SHIM, "Failed to create shim flow queue");
            prvShimUnbindDestroyFlow(pxSelf->pxData, pxFlow);
            return false;
        }

		LOGI(TAG_SHIM, "Created Flow: %p, portID: %d, portState: %d",
             pxFlow, pxFlow->unPort, pxFlow->ePortIdState);

		//************ RINAARP RESOLVE GPA

#if 0
		if (!xARPResolveGPA(pxFlow->pxDestPa, pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa))
		{
			prvShimUnbindDestroyFlow(pxData, pxFlow);
			return false;
		}
#endif
    } else if (pxFlow->ePortIdState == ePENDING) {
        LOGE(TAG_SHIM, "Port-id state is already pending");

    } else {
        LOGE(TAG_SHIM, "Invalid port state for flow allocation");
        return false;
    }

    return true;
}

/**
 * @brief Response to Flow allocation request.
 *
 * @param pxShimInstanceData
 * @param pxUserIpcp
 * @param xPortId
 * @return bool_t
 */
bool_t xShimFlowAllocateResponse(struct ipcpInstance_t *pxSelf, portId_t unPort)
{
	RINAStackEvent_t xEnrollEvent = {
        .eEventType = eShimFlowAllocatedEvent,
        .xData.PV = NULL
    };
	shimFlow_t *pxFlow;
	struct ipcpInstance_t *pxShimIpcp;

    RsAssert(pxSelf);
    RsAssert(is_port_id_ok(unPort));

	LOGI(TAG_SHIM, "Generating a Flow Allocate Response for a pending request");

	/* Searching for the Flow registered into the shim Instance Flow list */
	// Should include the portId into the search.
	pxFlow = prvShimFindFlowByPortId(pxSelf->pxData, unPort);
	if (!pxFlow) {
		LOGE(TAG_SHIM, "Flow does not exist, you shouldn't call this");
		return false;
	}

	/* Check if the flow is already allocated*/
	if (pxFlow->ePortIdState != ePENDING)
	{
		LOGE(TAG_SHIM, "Flow is already allocated");
		return false;
	}

	/* On positive response, flow should transition to allocated state */

	/*Retrieving the IPCP Shim Instance */

	/*Call to IPCP User to flow binding*/
	/*configASSERT(pxUserIpcp->pxOps);
	RsAssert(pxUserIpcp->pxOps->flowBindingIpcp);

	if (!pxUserIpcp->pxOps->flowBindingIpcp(pxUserIpcp->pxData,
											xPortId,
											pxShimIpcp))
	{
		LOGE(TAG_SHIM, "Could not bind flow with user_ipcp");
		// kfa_port_id_release(data->kfa, port_id);
		// unbind_and_destroy_flow(data, flow);
		return pdFALSE;
	}*/

	// spin_lock(&data->lock);
	pxFlow->ePortIdState = eALLOCATED;
	pxFlow->unPort = unPort;

	// pxFlow->pxUserIpcp = pxUserIpcp;

	pxFlow->pxDestHa = pxARPLookupGHA(&pxSelf->pxData->xARP, pxFlow->pxDestPa);

	/*
	ESP_LOGE(TAG_SHIM, "Printing GHA founded:");
	vARPPrintMACAddress(pxFlow->pxDestHa);
	*/

	if (pxFlow->ePortIdState == eALLOCATED)
	{
		LOGI(TAG_SHIM, "Flow with id:%d was allocated", pxFlow->unPort);
		xEnrollEvent.xData.UN = unPort;
		xSendEventStructToIPCPTask(&xEnrollEvent, 250 * 1000);
	}

	return true;
}

/*-------------------------------------------*/
/* @brief Primitive invoked before all other functions:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "-": ProcessName-ProcessInstance-EntityName-EntityInstance
 * - (Update LocalAddressProtocol which is part of the ARP module).
 * It is assumed only there is going to be one IPCP process over the Shim-DIF.
 * pxAppName, and pxDafName come from the Normal IPCP (user_app), while the pxData refers to
 * the shimWiFi ipcp instance.
 * @return a pdTrue if Success or pdFalse Failure.
 * */
bool_t xShimApplicationRegister(struct ipcpInstance_t *pxSelf,
                                const rname_t *pxAppName,
                                const rname_t *pxDafName)
{
    struct ipcpInstanceData_t *pxData;

	LOGI(TAG_SHIM, "Registering Application");

    RsAssert(pxSelf);
    RsAssert(pxAppName);
    RsAssert(pxDafName);

    pxData = pxSelf->pxData;

    pthread_mutex_lock(&pxData->xLock);

    if (ERR_CHK(xNameAssignDup(&pxSelf->pxData->xAppName, pxAppName))) {
		LOGE(TAG_SHIM, "Failed to create application name for shim");
        goto err;
    }

	if (!(pxData->pxAppPa = pxNameToGPA(pxAppName))) {
        LOGE(TAG_SHIM, "Failed to create protocol address object for shim application name");
        goto err;
    }

    if (!(pxData->pxDafPa = pxNameToGPA(pxDafName))) {
        LOGE(TAG_SHIM, "Failed to create protocol address object for shim DAF name");
        goto err;
    }

	//pxData->pxDafName = pxRstrNameDup(pxDafName);
    if (ERR_CHK(xNameAssignDup(&pxSelf->pxData->xDafName, pxDafName))) {
		LOGE(TAG_SHIM, "Failed to create DAF name for shim");
		goto err;
    }

	pxData->pxHa = pxCreateGHA(MAC_ADDR_802_3, &pxData->xPhyDev);

	if (!xIsGHAOK(pxSelf->pxData->pxHa)) {
		LOGI(TAG_SHIM, "Failed to create hardware address object for shim");
        goto err;
	}

    /* Add the required application in the ARP cache. */

	if (!xARPAddApplication(&pxData->xARP, pxData->pxAppPa, pxData->pxHa)) {
        LOGE(TAG_SHIM, "Failed to add application mapping");
        goto unregister_err;
	}

    if (!xARPAddApplication(&pxData->xARP, pxData->pxDafPa, pxData->pxHa)) {
        LOGE(TAG_SHIM, "Failed to add DAF mapping");
        goto unregister_err;
    }

    pthread_mutex_unlock(&pxData->xLock);

    return true;

    unregister_err:
    if (xARPRemoveApplication(&pxData->xARP, pxData->pxAppPa))
        LOGW(TAG_SHIM, "Failed to remove application mapping");

    if (xARPRemoveApplication(&pxData->xARP, pxData->pxAppPa))
        LOGW(TAG_SHIM, "Failed to remove DAF mapping");

    err:
    vNameFree(&pxData->xAppName);
    vNameFree(&pxData->xAppName);

    if (pxData->pxAppPa)
        vGPADestroy(pxData->pxAppPa);
    if (pxData->pxDafPa)
        vGPADestroy(pxData->pxDafPa);

    if (pxData->pxHa)
        vGHADestroy(pxData->pxHa);

    pthread_mutex_unlock(&pxData->xLock);

    return false;
}

/*-------------------------------------------*/
/**
 * @brief applicationUnregister (naming-info local)
 * Primitive invoked before all other functions:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "-": ProcessName-ProcessInstance-EntityName-EntityInstance
 * - (Update LocalAddressProtocol which is part of the ARP module).
 * It is assumed only there is going to be one IPCP process in the N+1 DIF (over the Shim-DIF)
 *
 * @param pxData Shim IPCP Data
 * @param pxName pxName to register. Normal Instance pxName or DIFName
 * @return bool_t
 */
bool_t xShimApplicationUnregister(struct ipcpInstance_t *pxSelf, const rname_t *pxName)
{
    struct ipcpInstanceData_t *pxData;
    gpa_t *pxPa;

    RsAssert(pxSelf);
    RsAssert(pxName);

    LOGI(TAG_SHIM, "Application Unregistering");

    pthread_mutex_lock(&pxSelf->pxData->xLock);

    pxData = pxSelf->pxData;
    pxPa = pxNameToGPA(pxName);

    if (!xARPRemoveApplication(&pxData->xARP, pxPa))
        LOGW(TAG_SHIM, "Failed to remove application registered in the shim");

    vNameFree(&pxData->xAppName);
    vNameFree(&pxData->xDafName);

    LOGI(TAG_SHIM, "Application unregister");

    pthread_mutex_unlock(&pxSelf->pxData->xLock);

    return true;
}

bool_t xShimSDUWrite(struct ipcpInstance_t *pxSelf, portId_t unPort, du_t *pxDu, bool_t uxBlocking)
{
	shimFlow_t *pxFlow;
	gha_t *pxSrcHw;
	size_t uxHeadLen, uxLength;
    buffer_t pxBufEthHdr;
    EthernetHeader_t *pxEthHdr;
	netbuf_t *pxNbFrame = NULL;
    bool_t xStatus = false;

    RsAssert(pxSelf);
    RsAssert(pxDu);

	LOGI(TAG_SHIM, "SDU write received");

	uxLength = unNetBufTotalSize(pxDu); // total length PDU

	if (uxLength > MTU) {
		LOGE(TAG_SHIM, "SDU too large (%zu), dropping", uxLength);
		return false;
	}

    if (!(pxFlow = prvShimFindFlowByPortId(pxSelf->pxData, unPort))) {
		LOGE(TAG_SHIM, "Flow does not exist, you shouldn't call this");
		return false;
	}

	if (pxFlow->ePortIdState != eALLOCATED) {
		LOGE(TAG_SHIM, "Flow is not in the right state to call this");
		return false;
	}

	LOGI(TAG_SHIM, "SDUWrite: creating source GHA");

	if (!(pxSrcHw = pxCreateGHA(MAC_ADDR_802_3, &pxSelf->pxData->xPhyDev))) {
		LOGE(TAG_SHIM, "Failed to get source HW addr");
		return false;
	}

	if (!pxFlow->pxDestHa) {
		LOGE(TAG_SHIM, "Destination HW address is unknown");
        goto fail;
	}

	LOGI(TAG_SHIM, "SDUWrite: Encapsulating packet into Ethernet Frame");

    if (!(pxBufEthHdr = pxRsrcAlloc(pxSelf->pxData->pxEthPool, "xShimSDUWrite"))) {
        LOGE(TAG_SHIM, "Failed to allocate memory for ethernet header");
        goto fail;
    }

    /* Generate an ethernet header */
    if (!(pxNbFrame = pxNetBufNew(pxSelf->pxData->pxNbPool, NB_ETH_HDR, pxBufEthHdr,
                                sizeof(EthernetHeader_t), NETBUF_FREE_POOL))) {
        LOGE(TAG_SHIM, "Failed to allocate memory for netbuf");
        goto fail;
    }

    vNetBufAppend(pxNbFrame, pxDu);

    pxEthHdr = (EthernetHeader_t *)pvNetBufPtr(pxNbFrame);

	pxEthHdr->usFrameType = RsHtoNS(ETH_P_RINA);
	memcpy(&pxEthHdr->xSourceAddress.ucBytes,
           pxSrcHw->xAddress.ucBytes, sizeof(pxSrcHw->xAddress));
	memcpy(&pxEthHdr->xDestinationAddress.ucBytes,
           pxFlow->pxDestHa->xAddress.ucBytes, sizeof(pxFlow->pxDestHa->xAddress));

    /* Send the packet back. No need to go through the IPCP */
    xStatus = xNetworkInterfaceOutput(pxNbFrame);

    fail:
    if (pxSrcHw)
        vGHADestroy(pxSrcHw);
    if (pxNbFrame)
        vNetBufFreeAll(pxNbFrame);

    return xStatus;
}

void vShimHandleEthernetPacket(struct ipcpInstance_t *pxSelf, netbuf_t *pxNbFrame)
{
    const EthernetHeader_t *pxEthHdr;
    eFrameProcessingResult_t eReturned = eFrameConsumed;
    uint16_t usFrameType;

    RsAssert(pxNbFrame != NULL);

    ASSERT_INSTANCE_DATA(pxSelf->pxData, IPCP_INSTANCE_DATA_ETHERNET_SHIM);

    /* Carve out the ethernet header from the frame */
    if (ERR_CHK(xNetBufSplit(pxNbFrame, NB_RINA_PCI, sizeof(EthernetHeader_t)))) {
        LOGE(TAG_WIFI, "Failed to allocate netbuf for ethernet header, rejecting packet");
        eReturned = eReleaseBuffer;
    }
    else {
        /* Map the buffer onto the Ethernet Header struct for easy access to the fields. */
        pxEthHdr = (EthernetHeader_t *)pvNetBufPtr(pxNbFrame);
        usFrameType = RsNtoHS(pxEthHdr->usFrameType);

        /* Interpret the received Ethernet packet. */
        switch (usFrameType) {
        case ETH_P_RINA_ARP:
            eReturned = prvShimHandleARPFrame(pxSelf, pxNbFrame);
            break;

        case ETH_P_RINA:
            eReturned = prvShimHandleRinaFrame(pxSelf, pxNbFrame);
            break;

        default:
            LOGE(TAG_WIFI, "Unhandled type of ethernet frame: %xu", usFrameType);
            eReturned = eReleaseBuffer;
            break;
        }
    }

    /* Perform any actions that resulted from processing the Ethernet frame. */
    switch (eReturned) {
    case eFrameConsumed:
        /* The frame is in use somewhere, don't release the buffer
         * yet. */
        LOGI(TAG_SHIM, "Frame Consumed");
        break;

    case eReturnEthernetFrame:
        /* The Ethernet frame will have been updated (maybe it was an
         * ARP request) and should be sent back to its source. */
        prvReturnEthernetFrame(pxNbFrame);

        /* Fall through! */
    case eReleaseBuffer:
        if (pxNbFrame)
            vNetBufFreeAll(pxNbFrame);
        break;

    case eProcessBuffer:
        /* Not sure what to do here ! */
        if (pxNbFrame)
            vNetBufFreeAll(pxNbFrame);
        break;

    default:

        /* The frame is not being used anywhere, and the
         * NetworkBufferDescriptor_t structure containing the frame should
         * just be released back to the list of free buffers. */
        // ESP_LOGI(TAG_SHIM, "Default: Releasing Buffer");
        
        break;
    }
}

static struct ipcpInstanceOps_t xShimWifiOps = {
    .start = xShimStart,
    .stop = xShimStop,
    .enable = xShimEnable,
    .disable = xShimDisable,
	.flowAllocateRequest = xShimFlowAllocateRequest,   // ok
	.flowAllocateResponse = xShimFlowAllocateResponse, // ok
	.flowDeallocate = xShimFlowDeallocate,			   // ok
	.flowPrebind = NULL,							   // ok
	.flowBindingIpcp = NULL,						   // ok
	.flowUnbindingIpcp = NULL,						   // ok
	.flowUnbindingUserIpcp = NULL,					   // ok
	.nm1FlowStateChange = NULL,						   // ok

	.applicationRegister = xShimApplicationRegister,	 // ok
	.applicationUnregister = xShimApplicationUnregister, // ok

	.assignToDif = NULL,	 // ok
	.updateDifConfig = NULL, // ok

	.connectionCreate = NULL,		 // ok
	.connectionUpdate = NULL,		 // ok
	.connectionDestroy = NULL,		 // ok
	.connectionCreateArrived = NULL, // ok
	.connectionModify = NULL,		 // ok

	.duEnqueue = NULL,		  // ok
	.duWrite = xShimSDUWrite, // xShimSDUWrite, //ok

	.mgmtDuWrite = NULL, // ok
	.mgmtDuPost = NULL,	 // ok

	.pffAdd = NULL,	   // ok
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

/************* CREATED, DESTROY, INIT, CLEAN SHIM IPCP ******/

struct ipcpInstance_t *pxShimWiFiCreate(ipcProcessId_t xIpcpId)
{
	struct ipcpInstance_t *pxInst;
	struct ipcpInstanceData_t *pxInstData = NULL;
	flowSpec_t *pxFspec;
	string_t pcInterfaceName = SHIM_INTERFACE;
    rname_t xName;

    /* Create an instance */
    if (!(pxInst = pvRsMemCAlloc(1, sizeof(struct ipcpInstance_t))))
        goto err;

    /* Create Data instance and Flow Specifications*/
    if (!(pxInstData = pvRsMemCAlloc(1, sizeof(struct ipcpInstanceData_t))))
        goto err;

    /* FIXME: ARBITRARY NUMBERS HERE, PUT IN CONFIGURATION SOMEWHERE. */
    if (!(pxInstData->pxNbPool = xNetBufNewPool("Ethernet shim netbuf pool")))
        goto err;

    if (!(pxInstData->pxEthPool = pxRsrcNewPool("Ethernet shim header pool",
                                                sizeof(EthernetHeader_t), 5, 1, 0)))
        goto err;

    if (pthread_mutex_init(&pxInstData->xLock, NULL))
        goto err;

    pxInst->pxData = pxInstData;

    /* Create Dif Name and Daf Name */

	pxInstData->xName.pcProcessName = SHIM_PROCESS_NAME;
	pxInstData->xName.pcEntityName = SHIM_ENTITY_NAME;
	pxInstData->xName.pcProcessInstance = SHIM_PROCESS_INSTANCE;
	pxInstData->xName.pcEntityInstance = SHIM_ENTITY_INSTANCE;

	pxInst->xType = eShimWiFi;
	pxInst->xId = xIpcpId;
	pxInst->pxOps = &xShimWifiOps;

	/* Filling the ShimWiFi instance properly */
    pxInst->pxData->unInstanceDataType = IPCP_INSTANCE_DATA_ETHERNET_SHIM;
	pxInst->pxData->xId = xIpcpId;
	pxInst->pxData->pcInterfaceName = pcInterfaceName;

	pxInst->pxData->xFspec.ulAverageBandwidth = 0;
	pxInst->pxData->xFspec.ulAverageSduBandwidth = 0;
	pxInst->pxData->xFspec.ulDelay = 0;
	pxInst->pxData->xFspec.ulJitter = 0;
	pxInst->pxData->xFspec.ulMaxAllowableGap = -1;
	pxInst->pxData->xFspec.ulMaxSduSize = 1500;
	pxInst->pxData->xFspec.xOrderedDelivery = 0;
	pxInst->pxData->xFspec.xPartialDelivery = 1;
	pxInst->pxData->xFspec.ulPeakBandwidthDuration = 0;
	pxInst->pxData->xFspec.ulPeakSduBandwidthDuration = 0;
	pxInst->pxData->xFspec.ulUndetectedBitErrorRate = 0;

	/*Initialialise flows list*/
	//vRsListInit((&pxInst->pxData->xFlowsList));
    memset(pxInst->pxData->xFlows, 0, sizeof(pxInst->pxData->xFlows));

	LOGI(TAG_SHIM, "Instance Created: %p, IPCP id:%d", pxInst, pxInst->pxData->xId);

	return pxInst;

    err:
    if (pxInstData)
        vRsMemFree(pxInst);
    if (pxInst)
        vRsMemFree(pxInst);

    return NULL;
}
