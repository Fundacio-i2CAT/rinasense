#include <stdio.h>
#include <string.h>

#include "common/list.h"
#include "common/rina_name.h"
#include "common/rina_ids.h"
#include "portability/port.h"

#include "Shim.h"
#include "ARP826.h"
#include "IPCP_instance.h"
#include "IPCP_api.h"
#include "IPCP_events.h"
#include "NetworkInterface.h"
#include "configRINA.h"
#include "configSensor.h"
#include "BufferManagement.h"
#include "du.h"
#include "IpcManager.h"

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

	/* The IPC Process using the shim-WiFi */
	name_t *pxAppName;
	name_t *pxDafName;

	/* Stores the state of flows indexed by port_id */
	// spinlock_t             lock;
	RsList_t xFlowsList;

	/* FIXME: Remove it as soon as the kipcm_kfa gets removed */
	// struct kfa *           kfa;

	/* RINARP related */
	struct rinarpHandle_t *pxAppHandle;
	struct rinarpHandle_t *pxDafHandle;

	/* To handle device notifications. */
	// struct notifier_block ntfy;

	/* Flow control between this IPCP and the associated netdev. */
	unsigned int ucTxBusy;
};

struct ipcpFactoryData_t
{
	RsList_t xInstancesShimWifiList;
};

static struct ipcpFactoryData_t xFactoryShimWifiData;

/* @brief Created a Queue type rfifo_t when a flow is allocated for request */
static rfifo_t *prvShimCreateQueue(void);

/* @brief Find a flow previously allocated*/
static shimFlow_t *prvShimFindFlow(struct ipcpInstanceData_t *pxData);

static shimFlow_t *prvShimFindFlowByPortId(struct ipcpInstanceData_t *pxData, portId_t xPortId);

/* @brief Destroy an specific Flow */
static bool_t prvShimFlowDestroy(struct ipcpInstanceData_t *xData, shimFlow_t *xFlow);

/* @brief Unbind and Destroy a Flow*/
static bool_t prvShimUnbindDestroyFlow(struct ipcpInstanceData_t *xData, shimFlow_t *xFlow);

EthernetHeader_t *vCastConstPointerTo_EthernetHeader_t(const void *pvArgument)
{
	return (void *)(pvArgument); // const void *
}

/******************** SHIM IPCP EventHandler **********/

/*-------------------------------------------*/
/* @brief Primitive invoked by the IPCP task event_handler to enroll to the shim DIF:
 * - Check if there is a flow established (eALLOCATED), or a flow pending between the
 * source and destination application (ePENDING),
 * - If stated is eNULL then initialized the wifi Interfaces to be ready to allocate Flows.
 * @param: pxPhyDev Local MacAddress to be fill with the wiFi Driver address.
 *
 * @return: pdTrue or pdFalse
 * */
bool_t xShimEnrollToDIF(MACAddress_t *pxPhyDev)
{
	LOGI(TAG_SHIM, "Enrolling to DIF");

	/* Initialization of WiFi interface */

	if (xNetworkInterfaceInitialise(pxPhyDev))
	{
		/* Initialize ARP Cache */
		vARPInitCache();

		/* Connect to remote point (WiFi AP) */
		if (xNetworkInterfaceConnect())
		{
			LOGI(TAG_SHIM, "Enrolled To DIF %s", SHIM_DIF_NAME);
			return true;
		}

		LOGE(TAG_SHIM, "Failed to enroll to DIF %s", SHIM_DIF_NAME);
		return false;
	}

	LOGE(TAG_SHIM, "Failed to enroll to DIF %s", SHIM_DIF_NAME);

	return false;
}

/*-------------------------------------------*/
/* @brief Deallocate a flow
 * */

bool_t xShimFlowDeallocate(struct ipcpInstanceData_t *xData, portId_t xId)
{
	shimFlow_t *xFlow;

	if (!xData)
	{
		LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return false;
	}

	if (!is_port_id_ok(xId))
	{
		LOGE(TAG_SHIM, "Invalid port ID passed, bailing out");
		return false;
	}

	xFlow = prvShimFindFlow(xData);
	if (!xFlow)
	{
		LOGE(TAG_SHIM, "Flow does not exist, cannot remove");
		return false;
	}

	return prvShimUnbindDestroyFlow(xData, xFlow);
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
bool_t xShimFlowAllocateRequest(struct ipcpInstanceData_t *pxData,
								name_t *pxSourceInfo,
								name_t *pxDestinationInfo,
								portId_t xPortId)
{

	LOGI(TAG_SHIM, "New flow allocation request");

	shimFlow_t *pxFlow;

	if (!pxData)
	{
		LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return false;
	}

	if (!pxSourceInfo)
	{
		LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return false;
	}
	if (!pxDestinationInfo)
	{
		LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return false;
	}

	if (!is_port_id_ok(xPortId))
	{
		LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return false;
	}

	LOGI(TAG_SHIM, "Finding Flows");
	pxFlow = prvShimFindFlowByPortId(pxData, xPortId);

	if (!pxFlow)
	{
		pxFlow = pvRsMemAlloc(sizeof(*pxFlow));
		if (!pxFlow)
			return false;

		pxFlow->xPortId = xPortId;
		pxFlow->ePortIdState = ePENDING;
		pxFlow->pxDestPa = pxNameToGPA(pxDestinationInfo);
		// pxFlow->pxUserIpcp = pxUserIpcp;

		if (!xIsGPAOK(pxFlow->pxDestPa))
		{
			LOGE(TAG_SHIM, "Destination protocol address is not OK");
			prvShimUnbindDestroyFlow(pxData, pxFlow);

			return false;
		}

		// Register the flow in a list or in the Flow allocator
		LOGI(TAG_SHIM, "Created Flow: %p, portID: %d, portState: %d", pxFlow, pxFlow->xPortId, pxFlow->ePortIdState);
		vRsListInitItem(&pxFlow->xFlowItem, pxFlow);
		vRsListInsert(&pxData->xFlowsList, &pxFlow->xFlowItem);

		pxFlow->pxSduQueue = prvShimCreateQueue();
		if (!pxFlow->pxSduQueue)
		{
			LOGE(TAG_SHIM, "Destination protocol address is not ok");
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
		LOGE(TAG_SHIM, "Port-id state is already pending");
	}
	else
	{
		LOGE(TAG_SHIM, "Allocate called in a wrong state");
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
bool_t xShimFlowAllocateResponse(struct ipcpInstanceData_t *pxShimInstanceData,
								 portId_t xPortId)

{
	RINAStackEvent_t xEnrollEvent = {
		.eEventType = eShimFlowAllocatedEvent,
		.xData.PV = NULL};
	shimFlow_t *pxFlow;
	struct ipcpInstance_t *pxShimIpcp;

	LOGI(TAG_SHIM, "Generating a Flow Allocate Response for a pending request");

	if (!pxShimInstanceData)
	{
		LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return false;
	}

	if (!is_port_id_ok(xPortId))
	{
		LOGE(TAG_SHIM, "Invalid port ID passed, bailing out");
		return false;
	}

	/* Searching for the Flow registered into the shim Instance Flow list */
	// Should include the portId into the search.
	pxFlow = prvShimFindFlowByPortId(pxShimInstanceData, xPortId);
	if (!pxFlow)
	{
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
	pxFlow->xPortId = xPortId;

	// pxFlow->pxUserIpcp = pxUserIpcp;

	pxFlow->pxDestHa = pxARPLookupGHA(pxFlow->pxDestPa);

	/*
	ESP_LOGE(TAG_SHIM, "Printing GHA founded:");
	vARPPrintMACAddress(pxFlow->pxDestHa);
	*/

	if (pxFlow->ePortIdState == eALLOCATED)
	{
		LOGI(TAG_SHIM, "Flow with id:%d was allocated", pxFlow->xPortId);
		xEnrollEvent.xData.UN = xPortId;
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
bool_t xShimApplicationRegister(struct ipcpInstanceData_t *pxData, name_t *pxAppName, name_t *pxDafName)
{
	LOGI(TAG_SHIM, "Registering Application");

	gpa_t *pxPa;
	gha_t *pxHa;

	if (!pxData)
	{
		LOGI(TAG_SHIM, "Data no valid ");
		return false;
	}
	if (!pxAppName)
	{
		LOGI(TAG_SHIM, "Name no valid ");
		return false;
	}
	if (pxData->pxAppName != NULL)
	{
		LOGI(TAG_SHIM, "AppName should not exist");
		return false;
	}

	pxData->pxAppName = pxRstrNameDup(pxAppName);

	if (!pxData->pxAppName)
	{
		LOGI(TAG_SHIM, "AppName not created ");
		return false;
	}

	pxPa = pxNameToGPA(pxAppName);

	if (!xIsGPAOK(pxPa))
	{
		LOGI(TAG_SHIM, "Protocol Address is not OK ");
		vRstrNameFini(pxData->pxAppName);
		return false;
	}

	if (!pxData->pxPhyDev)
	{
		xNetworkInterfaceInitialise(pxData->pxPhyDev);
	}

	pxHa = pxCreateGHA(MAC_ADDR_802_3, pxData->pxPhyDev);

	if (!xIsGHAOK(pxHa))
	{
		LOGI(TAG_SHIM, "Hardware Address is not OK ");
		vRstrNameFini(pxData->pxAppName);
		vGHADestroy(pxHa);
		return false;
	}

	pxData->pxAppHandle = pxARPAdd(pxPa, pxHa);

	if (!pxData->pxAppHandle)
	{
		// destroy all
		LOGI(TAG_SHIM, "APPHandle was not created ");
		vGPADestroy(pxPa);
		vGHADestroy(pxHa);
		vRstrNameFini(pxData->pxAppName);
		return false;
	}

	// vShimGPADestroy( pa );

	pxData->pxDafName = pxRstrNameDup(pxDafName);

	if (!pxData->pxDafName)
	{
		LOGE(TAG_SHIM, "Removing ARP Entry for DAF");
		xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
		pxData->pxAppHandle = NULL;
		vRstrNameFree(pxData->pxAppName);
		vGHADestroy(pxHa);
		return false;
	}

	pxPa = pxNameToGPA(pxDafName);

	if (!xIsGPAOK(pxPa))
	{
		LOGE(TAG_SHIM, "Failed to create gpa");
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
		LOGE(TAG_SHIM, "Failed to register DAF in ARP");
		xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
		pxData->pxAppHandle = NULL;
		vRstrNameFree(pxData->pxAppName);
		vRstrNameFree(pxData->pxDafName);
		vGPADestroy(pxPa);
		vGHADestroy(pxHa);
		return false;
	}

	// vShimGPADestroy(pa);

	// xSendEventToIPCPTask(eShimAppRegisteredEvent);

	// vARPPrintCache();

	return true;

	// vShimGHADestroy(ha);
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
bool_t xShimApplicationUnregister(struct ipcpInstanceData_t *pxData, const name_t *pxName)
{
	LOGI(TAG_SHIM, "Application Unregistering");

	if (!pxData)
	{
		LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return false;
	}

	if (!pxName)
	{
		LOGE(TAG_SHIM, "Invalid name passed, bailing out");
		return false;
	}

	if (!pxData->pxAppName)
	{
		LOGE(TAG_SHIM, "Shim-WiFi has no application registered");
		return false;
	}

	/* Remove from ARP cache */
	if (pxData->pxAppHandle)
	{
		if (xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa))
		{
			LOGE(TAG_SHIM, "Failed to remove APP entry from the cache");
			return false;
		}
		pxData->pxAppHandle = NULL;
	}

	if (pxData->pxDafHandle)
	{

		if (xARPRemove(pxData->pxDafHandle->pxPa, pxData->pxDafHandle->pxHa))
		{
			LOGE(TAG_SHIM, "Failed to remove DAF entry from the cache");
			return false;
		}
		pxData->pxDafHandle = NULL;
	}

	vRstrNameFree(pxData->pxAppName);
	pxData->pxAppName = NULL;
	vRstrNameFree(pxData->pxDafName);
	pxData->pxDafName = NULL;

	LOGI(TAG_SHIM, "Application unregister");

	return true;
}

/***************** **************************/

string_t pcShimNameToString(const name_t *n)
{
	string_t tmp;
	ssize_t size;
	const string_t none = "";
	size_t none_len = strlen(none);

	if (!n)
		return NULL;

	size = 0;

	size += (n->pcProcessName ? strlen(n->pcProcessName) : none_len);
	size += strlen(DELIMITER);

	size += (n->pcProcessInstance ? strlen(n->pcProcessInstance) : none_len);
	size += strlen(DELIMITER);

	size += (n->pcEntityName ? strlen(n->pcEntityName) : none_len);
	size += strlen(DELIMITER);

	size += (n->pcEntityInstance ? strlen(n->pcEntityInstance) : none_len);
	size += strlen(DELIMITER);

	tmp = pvRsMemAlloc(size);
	memset(tmp, 0, sizeof(*tmp));

	if (!tmp)
		return NULL;

	if (snprintf(tmp, size,
				 "%s%s%s%s%s%s%s",
				 (n->pcProcessName ? n->pcProcessName : none),
				 DELIMITER,
				 (n->pcProcessInstance ? n->pcProcessInstance : none),
				 DELIMITER,
				 (n->pcEntityName ? n->pcEntityName : none),
				 DELIMITER,
				 (n->pcEntityInstance ? n->pcEntityInstance : none)) !=
		size - 1)
	{
		vRsMemFree(tmp);
		return NULL;
	}

	return tmp;
}

static shimFlow_t *prvShimFindFlowByPortId(struct ipcpInstanceData_t *pxData, portId_t xPortId)
{

	shimFlow_t *pxFlow;
	RsListItem_t *pxListItem;

	RsAssert(pxData);

	pxFlow = pvRsMemAlloc(sizeof(*pxFlow));
	if (!pxFlow)
	{
		LOGE(TAG_SHIM, "Failed to allocate memory for flow");
		return NULL;
	}

#if 0
    /* FIXME: Is this validation at all necessary? */
	if (!RsListIsInitilised(&pxData->xFlowsList))
	{
		LOGE(TAG_SHIM, "Flow list is not initilized");
		return NULL;
	}
#endif
	if (!unRsListLength(&pxData->xFlowsList))
	{
		LOGI(TAG_SHIM, "Flow list is empty");
		return NULL;
	}

	/* Find a way to iterate in the list and compare the addesss*/
	pxListItem = pxRsListGetFirst(&pxData->xFlowsList);

	while (pxListItem != NULL)
	{
		pxFlow = (shimFlow_t *)pxRsListGetItemOwner(pxListItem);

		if (pxFlow)
		{
			// ESP_LOGI(TAG_SHIM, "Flow founded: %p, portID: %d, portState:%d", pxFlow, pxFlow->xPortId, pxFlow->ePortIdState);
			if (pxFlow->xPortId == xPortId)
			{
				return pxFlow;
			}
		}

		pxListItem = pxRsListGetNext(pxListItem);
	}

	LOGI(TAG_SHIM, "Flow not found");
	return NULL;
}

static shimFlow_t *prvShimFindFlow(struct ipcpInstanceData_t *pxData)
{

	shimFlow_t *pxFlow;
	RsListItem_t *pxListItem;

	pxFlow = pvRsMemAlloc(sizeof(*pxFlow));

	/* Find a way to iterate in the list and compare the addesss*/
	pxListItem = pxRsListGetFirst(&pxData->xFlowsList);

	while (pxListItem != NULL)
	{
		pxFlow = (shimFlow_t *)pxRsListGetItemOwner(pxListItem);

		if (pxFlow)
		{
			// ESP_LOGI(TAG_SHIM, "Flow founded: %p, portID: %d, portState:%d", pxFlow, pxFlow->xPortId, pxFlow->ePortIdState);

			return pxFlow;
			// return true;
		}

		pxListItem = pxRsListGetNext(pxListItem);
	}

	LOGI(TAG_SHIM, "Flow not found");
	return NULL;
}

static rfifo_t *prvShimCreateQueue(void)
{
	rfifo_t *xFifo = pvRsMemAlloc(sizeof(*xFifo));

	xFifo->xQueue = pxRsQueueCreate("ShimIPCPQueue", SIZE_SDU_QUEUE, sizeof(uint32_t));

	if (!xFifo->xQueue)
	{
		vRsMemFree(xFifo);
		return NULL;
	}

	return xFifo;
}

int QueueDestroy(rfifo_t *f,
				 void (*dtor)(void *e))
{
	if (!f)
	{
		LOGE(TAG_SHIM, "Bogus input parameters, can't destroy NULL");
		return -1;
	}
	if (!dtor)
	{
		LOGE(TAG_SHIM, "Bogus input parameters, no destructor provided");
		return -1;
	}

	vRsQueueDelete(f->xQueue);

	LOGI(TAG_SHIM, "FIFO %pK destroyed successfully", f);

	vRsMemFree(f);

	return 0;
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
	LOGI(TAG_SHIM, "Shim-WiFi unbinded port: %u", xFlow->xPortId);
	if (prvShimFlowDestroy(xData, xFlow))
	{
		LOGE(TAG_SHIM, "Failed to destroy Shim-WiFi flow");
		return false;
	}

	return true;
}

static bool_t prvShimFlowDestroy(struct ipcpInstanceData_t *xData, shimFlow_t *xFlow)
{

	/* FIXME: Complete what to do with xData*/
	if (xFlow->pxDestPa)
		vGPADestroy(xFlow->pxDestPa);
	if (xFlow->pxDestHa)
		vGHADestroy(xFlow->pxDestHa);
	if (xFlow->pxSduQueue)
		vRsQueueDelete(xFlow->pxSduQueue->xQueue);
	vRsMemFree(xFlow);

	return true;
}

bool_t xShimSDUWrite(struct ipcpInstanceData_t *pxData, portId_t xId, struct du_t *pxDu, bool_t uxBlocking)
{
	shimFlow_t *pxFlow;
	NetworkBufferDescriptor_t *pxNetworkBuffer;
	EthernetHeader_t *pxEthernetHeader;
	gha_t *pxSrcHw;
	gha_t *pxDestHw;
	size_t uxHeadLen, uxLength;
	struct timespec ts;
	RINAStackEvent_t xTxEvent = {
		.eEventType = eNetworkTxEvent,
		.xData.PV = NULL};
	unsigned char *pucArpPtr;

	LOGI(TAG_SHIM, "SDU write received");

	if (!pxData)
	{
		LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return false;
	}

	uxHeadLen = sizeof(EthernetHeader_t);		   // Header length Ethernet
	uxLength = pxDu->pxNetworkBuffer->xDataLength; // total length PDU

	if (uxLength > MTU)
	{
		LOGE(TAG_SHIM, "SDU too large (%zu), dropping", uxLength);
		xDuDestroy(pxDu);
		return false;
	}

	pxFlow = prvShimFindFlowByPortId(pxData, xId);
	if (!pxFlow)
	{
		LOGE(TAG_SHIM, "Flow does not exist, you shouldn't call this");
		xDuDestroy(pxDu);
		return false;
	}

	// spin_lock_bh(&data->lock);
	LOGI(TAG_SHIM, "SDUWrite: flow state check %d", pxFlow->ePortIdState);
	if (pxFlow->ePortIdState != eALLOCATED)
	{
		LOGE(TAG_SHIM, "Flow is not in the right state to call this");
		xDuDestroy(pxDu);
		return false;
	}

	LOGI(TAG_SHIM, "SDUWrite: creating source GHA");
	pxSrcHw = pxCreateGHA(MAC_ADDR_802_3, pxData->pxPhyDev);
	if (!pxSrcHw)
	{
		LOGE(TAG_SHIM, "Failed to get source HW addr");
		xDuDestroy(pxDu);
		return false;
	}

	/*
	vARPPrintMACAddress(pxFlow->pxDestHa);
	*/
	// pxDestHw = pxShimCreateGHA(MAC_ADDR_802_3, pxFlow->pxDestHa->xAddress);
	pxDestHw = pxFlow->pxDestHa;
	if (!pxDestHw)
	{
		LOGE(TAG_SHIM, "Destination HW address is unknown");
		xDuDestroy(pxDu);
		return false;
	}

	LOGI(TAG_SHIM, "SDUWrite: Encapsulating packet into Ethernet Frame");
	/* Get a Network Buffer with size total ethernet + PDU size*/

	pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(uxHeadLen + uxLength, 250 * 1000);

	if (pxNetworkBuffer == NULL)
	{
		LOGE(TAG_SHIM, "pxNetworkBuffer is null");
		xDuDestroy(pxDu);
		return false;
	}

	pxEthernetHeader = CAST_CONST_PTR_TO_CONST_TYPE_PTR(EthernetHeader_t, pxNetworkBuffer->pucEthernetBuffer);

	pxEthernetHeader->usFrameType = RsHtoNS(ETH_P_RINA);

	memcpy(pxEthernetHeader->xSourceAddress.ucBytes, pxSrcHw->xAddress.ucBytes, sizeof(pxSrcHw->xAddress));
	memcpy(pxEthernetHeader->xDestinationAddress.ucBytes, pxDestHw->xAddress.ucBytes, sizeof(pxDestHw->xAddress));

	/*Copy from the buffer PDU to the buffer Ethernet*/
	pucArpPtr = (unsigned char *)(pxEthernetHeader + 1);

	memcpy(pucArpPtr, pxDu->pxNetworkBuffer->pucEthernetBuffer, uxLength);

	pxNetworkBuffer->xDataLength = uxHeadLen + uxLength;

	/* Generate an event to sent or send from here*/
	/* Destroy pxDU no need anymore the stackbuffer*/
	xDuDestroy(pxDu);
	// ESP_LOGE(TAG_SHIM, "Releasing Buffer used in RMT");

	// vReleaseNetworkBufferAndDescriptor( pxDu->pxNetworkBuffer);

	/* ReleaseBuffer, no need anymore that why pdTRUE here */

	xTxEvent.xData.PV = (void *)pxNetworkBuffer;

	if (xSendEventStructToIPCPTask(&xTxEvent, 250 * 1000) == false)
	{
		LOGE(TAG_WIFI, "Failed to enqueue packet to network stack %p, len %zu", pxNetworkBuffer, pxNetworkBuffer->xDataLength);
		vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
		return false;
	}

	LOGI(TAG_SHIM, "Data sent to the IPCP TAsk");

	return true;
}

static struct ipcpInstanceOps_t xShimWifiOps = {
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
	struct ipcpInstanceData_t *pxInstData;
	struct flowSpec_t *pxFspec;
	string_t pcInterfaceName = SHIM_INTERFACE;
	name_t *pxName;
	MACAddress_t *pxPhyDev;

	pxPhyDev = pvRsMemAlloc(sizeof(*pxPhyDev));
	if (!pxPhyDev)
	{
		LOGE(TAG_WIFI, "Failed to allocate memory for WiFi shim instance");
		return NULL;
	}

	pxPhyDev->ucBytes[0] = 0x00;
	pxPhyDev->ucBytes[1] = 0x00;
	pxPhyDev->ucBytes[2] = 0x00;
	pxPhyDev->ucBytes[3] = 0x00;
	pxPhyDev->ucBytes[4] = 0x00;
	pxPhyDev->ucBytes[5] = 0x00;

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
	pxInst->pxData->pxFspec->ulMaxSduSize = 1500;
	pxInst->pxData->pxFspec->xOrderedDelivery = 0;
	pxInst->pxData->pxFspec->xPartialDelivery = 1;
	pxInst->pxData->pxFspec->ulPeakBandwidthDuration = 0;
	pxInst->pxData->pxFspec->ulPeakSduBandwidthDuration = 0;
	pxInst->pxData->pxFspec->ulUndetectedBitErrorRate = 0;

	pxInst->pxOps = &xShimWifiOps;
	pxInst->xType = eShimWiFi;
	pxInst->xId = xIpcpId;

	/*Initialialise flows list*/
	vRsListInit((&pxInst->pxData->xFlowsList));

	/*Initialialise instance item and add to the pxFactory*/
	/* vRsListInitItem(&(pxInst->pxData->xInstanceListItem)); */
	/* vRsListSetListItemOwner(&(pxInst->pxData->xInstanceListItem), pxInst); */
	/* vRsListInsert(&(pxFactoryData->xInstancesShimWifiList), &(pxInst->pxData->xInstanceListItem)); */

	LOGI(TAG_SHIM, "Instance Created: %p, IPCP id:%d", pxInst, pxInst->pxData->xId);

	/*Enroll to DIF (Connect to AP)*/

	return pxInst;
}

/*Check this logic.*/
bool_t xShimWiFiInit(struct ipcpInstance_t *pxShimWiFiInstance)
{
	/*xShimWiFiInit is going to init the  WiFi drivers and associate to the AP.
	 * Update de MacAddress variable depending on the WiFi drivers. Sent this variable
	 * as event data to be used when the shimWiFi D(F will be created.*/

	LOGI(TAG_SHIM, "Wifi shim initialization");
	RINAStackEvent_t xEnrollEvent = {
		.eEventType = eShimEnrolledEvent,
		.xData.PV = NULL};

	if (!xShimEnrollToDIF(pxShimWiFiInstance->pxData->pxPhyDev))
	{
		LOGE(TAG_SHIM, "Wifi shim instance can't enroll to DIF");
		return false;
	}
	else
	{
		xEnrollEvent.xData.PV = (void *)(pxShimWiFiInstance->pxData->pxPhyDev);
		xSendEventStructToIPCPTask(&xEnrollEvent, 50 * 1000);
		return true;
	}
}

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer)
{
	eFrameProcessingResult_t eReturn = eReleaseBuffer;
	const EthernetHeader_t *pxEthernetHeader;
	uint16_t usFrameType;

	/* Map the buffer onto Ethernet Header struct for easy access to fields. */
	pxEthernetHeader = (EthernetHeader_t *)pucEthernetBuffer;

	usFrameType = RsNtoHS(pxEthernetHeader->usFrameType);

	// Just ETH_P_ARP and ETH_P_RINA Should be processed by the stack
	if (usFrameType == ETH_P_RINA_ARP || usFrameType == ETH_P_RINA)
	{
		eReturn = eProcessBuffer;
		LOGD(TAG_IPCPMANAGER, "Ethernet packet of type %xu: ACCEPTED", usFrameType);
	}
	else
		LOGD(TAG_IPCPMANAGER, "Ethernet packet of type %xu: REJECTED", usFrameType);

	return eReturn;
}

void vShimHandleEthernetPacket(NetworkBufferDescriptor_t *const pxNetworkBuffer)
{
	const EthernetHeader_t *pxEthernetHeader;
	eFrameProcessingResult_t eReturned = eFrameConsumed;
	uint16_t usFrameType;
	struct ipcpInstance_t *pxSelf;

	RsAssert(pxNetworkBuffer != NULL);

	/*Call the IpcManager to retrieve the IPCP shim WiFi instance*/
	pxSelf = pxIpcManagerFindInstanceByType(eShimWiFi);

	if (pxSelf == NULL)
	{
		LOGE(TAG_SHIM, "Error no instance founded");
	}

	/* Interpret the Ethernet frame. */
	if (pxNetworkBuffer->xEthernetDataLength >= sizeof(EthernetHeader_t))
	{
		/* Map the buffer onto the Ethernet Header struct for easy access to the fields. */
		pxEthernetHeader = (EthernetHeader_t *)pxNetworkBuffer->pucEthernetBuffer;
		usFrameType = RsNtoHS(pxEthernetHeader->usFrameType);

		/* Interpret the received Ethernet packet. */
		switch (usFrameType)
		{
		case ETH_P_RINA_ARP:

			/* The Ethernet frame contains an ARP packet. */
			LOGI(TAG_IPCPMANAGER, "ARP Packet Received");

			if (pxNetworkBuffer->xEthernetDataLength >= sizeof(ARPPacket_t))
			{
				/*Process the Packet ARP in case of REPLY -> eProcessBuffer, REQUEST -> eReturnEthernet to
				 * send to the destination a REPLY (It requires more processing tasks) */
				eReturned = eARPProcessPacket(CAST_PTR_TO_TYPE_PTR(ARPPacket_t, pxNetworkBuffer->pucEthernetBuffer));
			}
			else
			{
				/*If ARP packet is not correct estructured then release buffer*/
				eReturned = eReleaseBuffer;
			}

			break;

		case ETH_P_RINA:

			LOGI(TAG_IPCPMANAGER, "RINA Packet Received");

			uint8_t *ptr;
			size_t uxRinaLength;
			// NetworkBufferDescriptor_t *pxBuffer;

			// removing Ethernet Header
			uxRinaLength = pxNetworkBuffer->xEthernetDataLength - (size_t)14;

			// ESP_LOGE(TAG_ARP, "Taking Buffer to copy the RINA PDU: ETH_P_RINA");
			// pxBuffer = pxGetNetworkBufferWithDescriptor(xlength, (TickType_t)0U);

			// Copy into the newBuffer but just the RINA PDU, and not the Ethernet Header
			ptr = (uint8_t *)pxNetworkBuffer->pucEthernetBuffer + 14;

			pxNetworkBuffer->xRinaDataLength = uxRinaLength;
			pxNetworkBuffer->pucRinaBuffer = ptr;

			// Release the buffer with the Ethernet header, it is not needed any more
			// ESP_LOGE(TAG_ARP, "Releasing Buffer to copy the RINA PDU: ETH_P_RINA");
			// vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);

			// must be void function
			vIpcManagerRINAPackettHandler(pxSelf->pxData, pxNetworkBuffer); // must change this must be managed by the shim instead to call IPCPManager

			break;

		default:
			LOGE(TAG_WIFI, "No Case Ethernet Type, Drop Frame");
			eReturned = eReleaseBuffer;

			break;
		}
	}
	//}

	/* Perform any actions that resulted from processing the Ethernet frame. */
	switch (eReturned)
	{
	case eReturnEthernetFrame:

		/* The Ethernet frame will have been updated (maybe it was
		 * an ARP request) and should be sent back to
		 * its source. */
		// vReturnEthernetFrame( pxNetworkBuffer, pdTRUE );

		/* parameter pdTRUE: the buffer must be released once
		 * the frame has been transmitted */
		break;

	case eFrameConsumed:

		/* The frame is in use somewhere, don't release the buffer
		 * yet. */
		LOGI(TAG_SHIM, "Frame Consumed");
		break;

	case eReleaseBuffer:
		// ESP_LOGI(TAG_SHIM, "Releasing Buffer: ProcessEthernet");
		if (pxNetworkBuffer != NULL)
		{
			vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
		}

		break;
	case eProcessBuffer:
		/*ARP process buffer, call to ShimAllocateResponse*/

		/* Finding an instance of eShimiFi and call the floww allocate Response using this instance*/

		if (!pxSelf->pxOps->flowAllocateResponse(pxSelf->pxData, 1))
		{
			LOGE(TAG_IPCPMANAGER, "Error during the Allocation Request at Shim");
			vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
		}
		else
		{
			LOGI(TAG_IPCPMANAGER, "Buffer Processed");
			vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
		}

		break;
	default:

		/* The frame is not being used anywhere, and the
		 * NetworkBufferDescriptor_t structure containing the frame should
		 * just be released back to the list of free buffers. */
		// ESP_LOGI(TAG_SHIM, "Default: Releasing Buffer");
		vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
		break;
	}
}