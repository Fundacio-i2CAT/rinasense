#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "ShimIPCP.h"
#include "ARP826.h"
#include "IPCP.h"
#include "NetworkInterface.h"
#include "configRINA.h"
#include "configSensor.h"
#include "BufferManagement.h"
#include "du.h"
#include "IpcManager.h"
#include "rstr.h"

#include "esp_log.h"

struct ipcpInstanceData_t
{

	ListItem_t xInstanceListItem;
	ipcProcessId_t xId;

	/* IPC Process name */
	name_t *pxName;
	name_t *pxDifName;
	string_t pcIntefaceName;

	MACAddress_t *pxPhyDev;
	struct flowSpec_t *pxFspec;

	/* The IPC Process using the shim-WiFi */
	name_t *pxAppName;
	name_t *pxDafName;

	/* Stores the state of flows indexed by port_id */
	// spinlock_t             lock;
	List_t xFlowsList;

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
	List_t xInstancesShimWifiList;
};

static struct ipcpFactoryData_t xFactoryShimWifiData;

/* @brief Created a Queue type rfifo_t when a flow is allocated for request */
static rfifo_t *prvShimCreateQueue(void);

/* @brief Find a flow previously allocated*/
static shimFlow_t *prvShimFindFlow(struct ipcpInstanceData_t *pxData);

static shimFlow_t *prvShimFindFlowByPortId(struct ipcpInstanceData_t *pxData, portId_t xPortId);

/* @brief Destroy an specific Flow */
static BaseType_t prvShimFlowDestroy(struct ipcpInstanceData_t *xData, shimFlow_t *xFlow);

/* @brief Unbind and Destroy a Flow*/
static BaseType_t prvShimUnbindDestroyFlow(struct ipcpInstanceData_t *xData, shimFlow_t *xFlow);

/** @brief  Concatenate the Information Application Name into a Complete Address
 * (ProcessName-ProcessInstance-EntityName-EntityInstance).
 * */
string_t pcShimNameToString(const name_t *xNameInfo); // Could be reutilized by others?, private?

/** @brief Convert the Complete Address (ProcessName-ProcessInstance-EntityName-EntityInstance)
 *  to Generic Protocol Address
 * */
gpa_t *pxShimNameToGPA(const name_t *xAplicationInfo);

/* @brief Destroy an application Name from the Data structure*/
void vShimNameDestroy(name_t *pxName);

/* @brief Destroy an application Name from the Data structure*/
void vShimNameFini(name_t *pxName);

/* @brief Create a generic protocol address based on an string address*/
gpa_t *pxShimCreateGPA(const uint8_t *pucAddress, size_t xLength); // ARP ALSO CHECK

/* @brief Create a generic hardware address based on the MAC address*/
gha_t *pxShimCreateGHA(eGHAType_t xType, const MACAddress_t *pxAddress);

/* @brief Check if a generic protocol Address was created correctly*/
BaseType_t xShimIsGPAOK(const gpa_t *pxGpa);

/* @brief Check if a generic hardware Address was created correctly*/
BaseType_t xShimIsGHAOK(const gha_t *pxGha);

/* @brief Destroy a generic protocol Address to clean up*/
void vShimGPADestroy(gpa_t *pxGpa);

/* @brief Destroy a generic hardware Address to clean up*/
void vShimGHADestroy(gha_t *pxGha);

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
BaseType_t xShimEnrollToDIF(const MACAddress_t *pxPhyDev)
{
	ESP_LOGI(TAG_SHIM, "Enrolling to DIF");

	// Initialization of WiFi interface

	if (xNetworkInterfaceInitialise(pxPhyDev))
	{
		/*Initialize ARP Cache*/
		vARPInitCache();

		/*Connect to remote point (WiFi AP)*/
		if (xNetworkInterfaceConnect())
		{
			ESP_LOGI(TAG_SHIM, "Enrolled To DIF %s", SHIM_DIF_NAME);
			return pdTRUE;
		}

		ESP_LOGE(TAG_SHIM, "Failed to enroll to DIF %s", SHIM_DIF_NAME);
		return pdFALSE;
	}

	ESP_LOGE(TAG_SHIM, "Failed to enroll to DIF %s", SHIM_DIF_NAME);

	return pdFALSE;
}

/*-------------------------------------------*/
/* @brief Deallocate a flow
 * */

BaseType_t xShimFlowDeallocate(struct ipcpInstanceData_t *xData, portId_t xId)
{
	shimFlow_t *xFlow;

	if (!xData)
	{
		ESP_LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return pdFALSE;
	}

	if (!IS_PORT_ID_OK(xId))
	{
		ESP_LOGE(TAG_SHIM, "Invalid port ID passed, bailing out");
		return pdFALSE;
	}

	xFlow = prvShimFindFlow(xData);
	if (!xFlow)
	{
		ESP_LOGE(TAG_SHIM, "Flow does not exist, cannot remove");
		return pdFALSE;
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
BaseType_t xShimFlowAllocateRequest(portId_t xPortId,
									const name_t *pxSourceInfo,
									const name_t *pxDestinationInfo,
									struct ipcpInstanceData_t *pxData)
{

	ESP_LOGI(TAG_SHIM, "New flow allocation request");

	shimFlow_t *pxFlow;

	if (!pxData)
	{
		ESP_LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return pdFALSE;
	}

	if (!pxSourceInfo)
	{
		ESP_LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return pdFALSE;
	}
	if (!pxDestinationInfo)
	{
		ESP_LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return pdFALSE;
	}

	if (!IS_PORT_ID_OK(xPortId))
	{
		ESP_LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return pdFALSE;
	}

	ESP_LOGI(TAG_SHIM, "Finding Flows");
	pxFlow = prvShimFindFlowByPortId(pxData, xPortId);

	if (!pxFlow)
	{
		pxFlow = pvPortMalloc(sizeof(*pxFlow));
		if (!pxFlow)
			return pdFALSE;

		pxFlow->xPortId = xPortId;
		pxFlow->ePortIdState = ePENDING;
		pxFlow->pxDestPa = pxShimNameToGPA(pxDestinationInfo);
		// pxFlow->pxUserIpcp = pxUserIpcp;

		if (!xShimIsGPAOK(pxFlow->pxDestPa))
		{
			ESP_LOGE(TAG_SHIM, "Destination protocol address is not OK");
			prvShimUnbindDestroyFlow(pxData, pxFlow);

			return pdFALSE;
		}

		// Register the flow in a list or in the Flow allocator

		ESP_LOGI(TAG_SHIM, "Created Flow: %p, portID: %d, portState: %d", pxFlow, pxFlow->xPortId, pxFlow->ePortIdState);
		vListInitialiseItem(&pxFlow->xFlowItem);
		listSET_LIST_ITEM_OWNER(&(pxFlow->xFlowItem), pxFlow);
		vListInsert(&pxData->xFlowsList, &pxFlow->xFlowItem);

		pxFlow->pxSduQueue = prvShimCreateQueue();
		if (!pxFlow->pxSduQueue)
		{
			ESP_LOGE(TAG_SHIM, "Destination protocol address is not ok");
			prvShimUnbindDestroyFlow(pxData, pxFlow);
			return pdFALSE;
		}

		//************ RINAARP RESOLVE GPA

		if (!xARPResolveGPA(pxFlow->pxDestPa, pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa))
		{
			prvShimUnbindDestroyFlow(pxData, pxFlow);
			return pdFALSE;
		}
	}
	else if (pxFlow->ePortIdState == ePENDING)
	{
		ESP_LOGE(TAG_SHIM, "Port-id state is already pending");
	}
	else
	{
		ESP_LOGE(TAG_SHIM, "Allocate called in a wrong state");
		return pdFALSE;
	}

	return pdTRUE;
}

/**
 * @brief Response to Flow allocation request.
 *
 * @param pxShimInstanceData
 * @param pxUserIpcp
 * @param xPortId
 * @return BaseType_t
 */
BaseType_t xShimFlowAllocateResponse(struct ipcpInstanceData_t *pxShimInstanceData, portId_t xPortId)

{
	RINAStackEvent_t xEnrollEvent = {eShimFlowAllocatedEvent, NULL};
	const TickType_t xDontBlock = pdMS_TO_TICKS(50); //(TickType_t) 0U; // pdMS_TO_TICKS(50);
	shimFlow_t *pxFlow;
	ipcpInstance_t *pxShimIpcp;

	ESP_LOGI(TAG_SHIM, "Generating a Flow Allocate Response for a pending request");

	if (!pxShimInstanceData)
	{
		ESP_LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return pdFALSE;
	}

	if (!IS_PORT_ID_OK(xPortId))
	{
		ESP_LOGE(TAG_SHIM, "Invalid port ID passed, bailing out");
		return pdFALSE;
	}

	/* Searching for the Flow registered into the shim Instance Flow list */
	// Should include the portId into the search.
	pxFlow = prvShimFindFlowByPortId(pxShimInstanceData, xPortId);
	if (!pxFlow)
	{
		ESP_LOGE(TAG_SHIM, "Flow does not exist, you shouldn't call this");

		return pdFALSE;
	}

	/* Check if the flow is already allocated*/
	if (pxFlow->ePortIdState != ePENDING)
	{
		ESP_LOGE(TAG_SHIM, "Flow is already allocated");
		return pdFALSE;
	}

	/* On positive response, flow should transition to allocated state */

	/*Retrieving the IPCP Shim Instance */
	/*Call to IPCP User to flow binding*/
	/*configASSERT(pxUserIpcp->pxOps);
	configASSERT(pxUserIpcp->pxOps->flowBindingIpcp);
	if (!pxUserIpcp->pxOps->flowBindingIpcp(pxUserIpcp->pxData,
											xPortId,
											pxShimIpcp))
	{
		ESP_LOGE(TAG_SHIM, "Could not bind flow with user_ipcp");
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
		ESP_LOGI(TAG_SHIM, "Flow with id:%d was allocated", pxFlow->xPortId);
		xEnrollEvent.pvData = xPortId;
		xSendEventStructToIPCPTask(&xEnrollEvent, xDontBlock);
	}

	return pdTRUE;
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
BaseType_t xShimApplicationRegister(struct ipcpInstanceData_t *pxData, name_t *pxAppName, name_t *pxDafName)
{
	ESP_LOGI(TAG_SHIM, "Registering Application");

	gpa_t *pxPa;
	gha_t *pxHa;

	if (!pxData)
	{
		ESP_LOGI(TAG_SHIM, "Data no valid ");
		return pdFALSE;
	}
	if (!pxAppName)
	{
		ESP_LOGI(TAG_SHIM, "Name no valid ");
		return pdFALSE;
	}
	if (pxData->pxAppName != NULL)
	{
		ESP_LOGI(TAG_SHIM, "AppName should not exist");
		return pdFALSE;
	}

	pxData->pxAppName = pxRstrNameDup(pxAppName);

	if (!pxData->pxAppName)
	{
		ESP_LOGI(TAG_SHIM, "AppName not created ");
		return pdFALSE;
	}

	pxPa = pxShimNameToGPA(pxAppName);

	if (!xShimIsGPAOK(pxPa))
	{
		ESP_LOGI(TAG_SHIM, "Protocol Address is not OK ");
		vShimNameFini(pxData->pxAppName);
		return pdFALSE;
	}

	if (!pxData->pxPhyDev)
	{
		xNetworkInterfaceInitialise(pxData->pxPhyDev);
	}

	pxHa = pxShimCreateGHA(MAC_ADDR_802_3, pxData->pxPhyDev);

	if (!xShimIsGHAOK(pxHa))
	{
		ESP_LOGI(TAG_SHIM, "Hardware Address is not OK ");
		vShimNameFini(pxData->pxAppName);
		vShimGHADestroy(pxHa);
		return pdFALSE;
	}

	pxData->pxAppHandle = pxARPAdd(pxPa, pxHa);

	if (!pxData->pxAppHandle)
	{
		// destroy all
		ESP_LOGI(TAG_SHIM, "APPHandle was not created ");
		vShimGPADestroy(pxPa);
		vShimGHADestroy(pxHa);
		vShimNameFini(pxData->pxAppName);
		return pdFALSE;
	}

	// vShimGPADestroy( pa );

	pxData->pxDafName = pxRstrNameDup(pxDafName);

	if (!pxData->pxDafName)
	{

		ESP_LOGE(TAG_SHIM, "Removing ARP Entry for DAF");
		xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
		pxData->pxAppHandle = NULL;
		vShimNameDestroy(pxData->pxAppName);
		vShimGHADestroy(pxHa);
		return pdFALSE;
	}

	pxPa = pxShimNameToGPA(pxDafName);

	if (!xShimIsGPAOK(pxPa))
	{

		ESP_LOGE(TAG_SHIM, "Failed to create gpa");
		xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
		pxData->pxAppHandle = NULL;
		vShimNameDestroy(pxData->pxDafName);
		vShimNameDestroy(pxData->pxAppName);
		vShimGHADestroy(pxHa);
		return pdFALSE;
	}

	pxData->pxDafHandle = pxARPAdd(pxPa, pxHa);

	if (!pxData->pxDafHandle)
	{
		ESP_LOGE(TAG_SHIM, "Failed to register DAF in ARP");
		xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
		pxData->pxAppHandle = NULL;
		vShimNameDestroy(pxData->pxAppName);
		vShimNameDestroy(pxData->pxDafName);
		vShimGPADestroy(pxPa);
		vShimGHADestroy(pxHa);

		return pdFALSE;
	}

	// vShimGPADestroy(pa);

	// xSendEventToIPCPTask(eShimAppRegisteredEvent);

	// vARPPrintCache();

	return pdTRUE;

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
 * @return BaseType_t
 */
BaseType_t xShimApplicationUnregister(struct ipcpInstanceData_t *pxData, name_t *pxName)
{
	ESP_LOGI(TAG_SHIM, "Application Unregistering");

	if (!pxData)
	{
		ESP_LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return pdFALSE;
	}

	if (!pxName)
	{
		ESP_LOGE(TAG_SHIM, "Invalid name passed, bailing out");
		return pdFALSE;
	}

	if (!pxData->pxAppName)
	{
		ESP_LOGE(TAG_SHIM, "Shim-WiFi has no application registered");
		return pdFALSE;
	}

	/* Remove from ARP cache */
	if (pxData->pxAppHandle)
	{

		if (xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa))
		{
			ESP_LOGE(TAG_SHIM, "Failed to remove APP entry from the cache");
			return pdFALSE;
		}
		pxData->pxAppHandle = NULL;
	}

	if (pxData->pxDafHandle)
	{

		if (xARPRemove(pxData->pxDafHandle->pxPa, pxData->pxDafHandle->pxHa))
		{
			ESP_LOGE(TAG_SHIM, "Failed to remove DAF entry from the cache");
			return pdFALSE;
		}
		pxData->pxDafHandle = NULL;
	}

	vShimNameDestroy(pxData->pxAppName);
	pxData->pxAppName = NULL;
	vShimNameDestroy(pxData->pxDafName);
	pxData->pxDafName = NULL;

	ESP_LOGI(TAG_SHIM, "Application unregister");

	return pdTRUE;
}

/***************** **************************/

int string_len(const string_t *s)
{
	return strlen(s);
}

string_t pcShimNameToString(const name_t *n)
{
	string_t tmp;
	size_t size;
	const string_t none = "";
	size_t none_len = strlen(none);

	if (!n)
		return NULL;

	size = 0;

	size += (n->pcProcessName ? string_len(n->pcProcessName) : none_len);
	size += strlen(DELIMITER);

	size += (n->pcProcessInstance ? string_len(n->pcProcessInstance) : none_len);
	size += strlen(DELIMITER);

	size += (n->pcEntityName ? string_len(n->pcEntityName) : none_len);
	size += strlen(DELIMITER);

	size += (n->pcEntityInstance ? string_len(n->pcEntityInstance) : none_len);
	size += strlen(DELIMITER);

	tmp = pvPortMalloc(size);
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
		vPortFree(tmp);
		return NULL;
	}

	return tmp;
}

gpa_t *pxShimNameToGPA(const name_t *xLocalInfo)
{
	// uint32_t ulIPCPAddress;
	gpa_t *pxGpa;
	string_t pcTmp;

	pcTmp = pcShimNameToString(xLocalInfo);

	if (!pcTmp)
	{
		ESP_LOGI(TAG_SHIM, "Name to String not correct");
		return NULL;
	}

	// Convert the IPCPAddress Concatenate to bits
	pxGpa = pxShimCreateGPA((uint8_t *)(pcTmp), strlen(pcTmp));

	if (!pxGpa)
	{
		ESP_LOGI(TAG_SHIM, "GPA was not created correct");
		vPortFree(pcTmp);
		return NULL;
	}

	// vPortFree(pcTmp);

	return pxGpa;
}

string_t *xShimGPAAddressToString(const gpa_t *pxGpa)
{
	string_t *tmp;
	string_t *p;

	if (!xShimIsGPAOK(pxGpa))
	{
		ESP_LOGE(TAG_ARP, "Bad input parameter, "
						  "cannot get a meaningful address from GPA");
		return NULL;
	}

	tmp = pvPortMalloc(pxGpa->uxLength + 1);
	if (!tmp)
		return NULL;

	memcpy(tmp, pxGpa->ucAddress, pxGpa->uxLength);

	p = tmp + pxGpa->uxLength;
	*(p) = '\0';

	ESP_LOGE(TAG_ARP, "GPA:%s", pxGpa->ucAddress);
	ESP_LOGE(TAG_ARP, "GPA:%s", *tmp);

	return tmp;
}

uint8_t *pucCreateAddress(size_t uxLength)
{
	uint8_t *pucAddress;

	pucAddress = (uint8_t *)pvPortMalloc(uxLength);
	memset(pucAddress, 0, uxLength);

	return pucAddress;
}

gpa_t *pxShimCreateGPA(const uint8_t *pucAddress, size_t uxLength)
{
	gpa_t *pxGPA;

	if (!pucAddress || uxLength == 0)
	{
		ESP_LOGI(TAG_SHIM, "Bad input parameters, cannot create GPA");
		return NULL;
	}

	pxGPA = pvPortMalloc(sizeof(*pxGPA));

	if (!pxGPA)
		return NULL;

	pxGPA->uxLength = uxLength;						   // strlen of the address without '\0'
	pxGPA->ucAddress = pucCreateAddress(uxLength + 1); // Create an address an include the '\0'

	if (!pxGPA->ucAddress)
	{
		vPortFree(pxGPA);
		return NULL;
	}

	memcpy(pxGPA->ucAddress, pucAddress, pxGPA->uxLength);

	ESP_LOGI(TAG_SHIM, "Created GPA address: %s with size %d", pxGPA->ucAddress, pxGPA->uxLength);

	return pxGPA;
}

gha_t *pxShimCreateGHA(eGHAType_t xType, const MACAddress_t *pxAddress) // Changes to uint8_t
{
	gha_t *pxGha;

	if (xType != MAC_ADDR_802_3 || !pxAddress->ucBytes)
	{
		ESP_LOGE(TAG_SHIM, "Wrong input parameters, cannot create GHA");
		return NULL;
	}
	pxGha = pvPortMalloc(sizeof(*pxGha));
	if (!pxGha)
		return NULL;

	pxGha->xType = xType;
	memcpy(pxGha->xAddress.ucBytes, pxAddress->ucBytes, sizeof(pxGha->xAddress));

	return pxGha;
}

static shimFlow_t *prvShimFindFlowByPortId(struct ipcpInstanceData_t *pxData, portId_t xPortId)
{

	shimFlow_t *pxFlow;

	ListItem_t *pxListItem;
	ListItem_t const *pxListEnd;

	pxFlow = pvPortMalloc(sizeof(*pxFlow));

	/* Find a way to iterate in the list and compare the addesss*/
	pxListEnd = listGET_END_MARKER(&pxData->xFlowsList);
	pxListItem = listGET_HEAD_ENTRY(&pxData->xFlowsList);

	while (pxListItem != pxListEnd)
	{

		pxFlow = (shimFlow_t *)listGET_LIST_ITEM_OWNER(pxListItem);

		if (pxFlow)
		{

			// ESP_LOGI(TAG_SHIM, "Flow founded: %p, portID: %d, portState:%d", pxFlow, pxFlow->xPortId, pxFlow->ePortIdState);
			if (pxFlow->xPortId == xPortId)
			{
				return pxFlow;
			}
		}

		pxListItem = listGET_NEXT(pxListItem);
	}

	ESP_LOGI(TAG_SHIM, "Flow not founded");
	return NULL;
}

static shimFlow_t *prvShimFindFlow(struct ipcpInstanceData_t *pxData)
{

	shimFlow_t *pxFlow;

	ListItem_t *pxListItem;
	ListItem_t const *pxListEnd;

	pxFlow = pvPortMalloc(sizeof(*pxFlow));

	/* Find a way to iterate in the list and compare the addesss*/
	pxListEnd = listGET_END_MARKER(&pxData->xFlowsList);
	pxListItem = listGET_HEAD_ENTRY(&pxData->xFlowsList);

	while (pxListItem != pxListEnd)
	{

		pxFlow = (shimFlow_t *)listGET_LIST_ITEM_VALUE(pxListItem);

		if (pxFlow)
		{

			// ESP_LOGI(TAG_SHIM, "Flow founded: %p, portID: %d, portState:%d", pxFlow, pxFlow->xPortId, pxFlow->ePortIdState);

			return pxFlow;
			// return pdTRUE;
		}

		pxListItem = listGET_NEXT(pxListItem);
	}

	ESP_LOGI(TAG_SHIM, "Flow not founded");
	return NULL;
}

static rfifo_t *prvShimCreateQueue(void)
{
	rfifo_t *xFifo = pvPortMalloc(sizeof(*xFifo));

	xFifo->xQueue = xQueueCreate(SIZE_SDU_QUEUE, sizeof(uint32_t));

	if (!xFifo->xQueue)
	{
		vPortFree(xFifo);
		return NULL;
	}

	return xFifo;
}

int QueueDestroy(rfifo_t *f,
				 void (*dtor)(void *e))
{
	if (!f)
	{
		ESP_LOGE(TAG_SHIM, "Bogus input parameters, can't destroy NULL");
		return -1;
	}
	if (!dtor)
	{
		ESP_LOGE(TAG_SHIM, "Bogus input parameters, no destructor provided");
		return -1;
	}

	vQueueDelete(f->xQueue);

	vPortFree(f);

	ESP_LOGI(TAG_SHIM, "FIFO %pK destroyed successfully", f);

	return 0;
}

// Move to ARP

BaseType_t xShimIsGPAOK(const gpa_t *pxGpa)
{

	if (!pxGpa)
	{
		ESP_LOGI(TAG_SHIM, " !Gpa");
		return pdFALSE;
	}
	if (pxGpa->ucAddress == NULL)
	{

		ESP_LOGI(TAG_SHIM, "xShimIsGPAOK Address is NULL");
		return pdFALSE;
	}

	if (pxGpa->uxLength == 0)
	{
		ESP_LOGI(TAG_SHIM, "Length = 0");
		return pdFALSE;
	}
	return pdTRUE;
}

BaseType_t xShimIsGHAOK(const gha_t *pxGha)
{
	if (!pxGha)
	{
		ESP_LOGI(TAG_SHIM, "No Valid GHA");
		return pdFALSE;
	}
	if (pxGha->xType != MAC_ADDR_802_3)
	{

		return pdFALSE;
	}
	return pdTRUE;
}

void vShimGPADestroy(gpa_t *pxGpa)
{
	if (!xShimIsGPAOK(pxGpa))
	{
		return;
	}
	vPortFree(pxGpa->ucAddress);
	vPortFree(pxGpa);

	return;
}

void vShimGHADestroy(gha_t *pxGha)
{
	if (!xShimIsGHAOK(pxGha))
	{
		return;
	}

	vPortFree(pxGha);

	return;
}

void vShimNameDestroy(name_t *pxName)
{

	vPortFree(pxName);
}

void vShimNameFini(name_t *pxName)
{

	if (pxName->pcProcessName)
	{
		vPortFree(pxName->pcProcessName);
		pxName->pcProcessName = NULL;
	}
	if (pxName->pcProcessInstance)
	{
		vPortFree(pxName->pcProcessInstance);
		pxName->pcProcessInstance = NULL;
	}
	if (pxName->pcEntityName)
	{
		vPortFree(pxName->pcEntityName);
		pxName->pcEntityName = NULL;
	}
	if (pxName->pcEntityInstance)
	{
		vPortFree(pxName->pcEntityInstance);
		pxName->pcEntityInstance = NULL;
	}

	ESP_LOGI(TAG_SHIM, "Name at %pK finalized successfully", pxName);
}

static BaseType_t prvShimUnbindDestroyFlow(struct ipcpInstanceData_t *xData,
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
	ESP_LOGI(TAG_SHIM, "Shim-WiFi unbinded port: %u", xFlow->xPortId);
	if (prvShimFlowDestroy(xData, xFlow))
	{
		ESP_LOGE(TAG_SHIM, "Failed to destroy Shim-WiFi flow");
		return pdFALSE;
	}

	return pdTRUE;
}

static BaseType_t prvShimFlowDestroy(struct ipcpInstanceData_t *xData, shimFlow_t *xFlow)
{

	/* FIXME: Complete what to do with xData*/
	if (xFlow->pxDestPa)
		vShimGPADestroy(xFlow->pxDestPa);
	if (xFlow->pxDestHa)
		vShimGHADestroy(xFlow->pxDestHa);
	if (xFlow->pxSduQueue)
		vQueueDelete(xFlow->pxSduQueue->xQueue);
	vPortFree(xFlow);

	return pdTRUE;
}

BaseType_t xShimSDUWrite(struct ipcpInstanceData_t *pxData, portId_t xId, struct du_t *pxDu, BaseType_t uxBlocking)
{

	shimFlow_t *pxFlow;
	NetworkBufferDescriptor_t *pxNetworkBuffer;

	EthernetHeader_t *pxEthernetHeader;
	gha_t *pxSrcHw;
	gha_t *pxDestHw;
	size_t uxHeadLen, uxLength;

	RINAStackEvent_t xTxEvent = {eNetworkTxEvent, NULL};
	const TickType_t xDescriptorWaitTime = pdMS_TO_TICKS(250);

	unsigned char *pucArpPtr;

	ESP_LOGI(TAG_SHIM, "SDU write received");

	if (unlikely(!pxData))
	{
		ESP_LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return pdFALSE;
	}

	uxHeadLen = sizeof(EthernetHeader_t);		   // Header length Ethernet
	uxLength = pxDu->pxNetworkBuffer->xDataLength; // total length PDU

	if (unlikely(uxLength > MTU))
	{
		ESP_LOGE(TAG_SHIM, "SDU too large (%d), dropping", uxLength);
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	pxFlow = prvShimFindFlowByPortId(pxData, xId);
	if (!pxFlow)
	{
		ESP_LOGE(TAG_SHIM, "Flow does not exist, you shouldn't call this");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	// spin_lock_bh(&data->lock);
	ESP_LOGI(TAG_SHIM, "SDUWrite: flow state check %d", pxFlow->ePortIdState);
	if (pxFlow->ePortIdState != eALLOCATED)
	{
		ESP_LOGE(TAG_SHIM, "Flow is not in the right state to call this");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	ESP_LOGI(TAG_SHIM, "SDUWrite: creating source GHA");
	pxSrcHw = pxShimCreateGHA(MAC_ADDR_802_3, pxData->pxPhyDev);
	if (!pxSrcHw)
	{
		ESP_LOGE(TAG_SHIM, "Failed to get source HW addr");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	/*
	vARPPrintMACAddress(pxFlow->pxDestHa);
	*/
	// pxDestHw = pxShimCreateGHA(MAC_ADDR_802_3, pxFlow->pxDestHa->xAddress);
	pxDestHw = pxFlow->pxDestHa;
	if (!pxDestHw)
	{
		ESP_LOGE(TAG_SHIM, "Destination HW address is unknown");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	ESP_LOGI(TAG_SHIM, "SDUWrite: Encapsulating packet into Ethernet Frame");
	/* Get a Network Buffer with size total ethernet + PDU size*/

	pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(uxHeadLen + uxLength, (TickType_t)0U);

	if (pxNetworkBuffer == NULL)
	{
		ESP_LOGE(TAG_SHIM, "pxNetworkBuffer is null");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	pxEthernetHeader = CAST_CONST_PTR_TO_CONST_TYPE_PTR(EthernetHeader_t, pxNetworkBuffer->pucEthernetBuffer);

	pxEthernetHeader->usFrameType = FreeRTOS_htons(ETH_P_RINA);

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

	xTxEvent.pvData = (void *)pxNetworkBuffer;

	if (xSendEventStructToIPCPTask(&xTxEvent, xDescriptorWaitTime) == pdFAIL)
	{
		ESP_LOGE(TAG_WIFI, "Failed to enqueue packet to network stack %p, len %d", pxNetworkBuffer, pxNetworkBuffer->xDataLength);
		vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
		return ESP_FAIL;
	}

	ESP_LOGI(TAG_SHIM, "Data sent to the IPCP TAsk");

	return pdTRUE;
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

ipcpInstance_t *pxShimWiFiCreate(ipcProcessId_t xIpcpId)
{

	ipcpInstance_t *pxInst;
	struct ipcpInstanceData_t *pxInstData;
	string_t pcIntefaceName = SHIM_INTERFACE;
	struct flowSpec_t *pxFspec;
	name_t *pxName;
	MACAddress_t *pxPhyDev;

	pxPhyDev = pvPortMalloc(sizeof(*pxPhyDev));

	pxPhyDev->ucBytes[0] = 0x00;
	pxPhyDev->ucBytes[1] = 0x00;
	pxPhyDev->ucBytes[2] = 0x00;
	pxPhyDev->ucBytes[3] = 0x00;
	pxPhyDev->ucBytes[4] = 0x00;
	pxPhyDev->ucBytes[5] = 0x00;

	/* Create an instance */
	pxInst = pvPortMalloc(sizeof(*pxInst));

	/* Create Data instance and Flow Specifications*/
	pxInstData = pvPortMalloc(sizeof(*pxInstData));
	pxInst->pxData = pxInstData;

	pxFspec = pvPortMalloc(sizeof(*pxFspec));
	pxInst->pxData->pxFspec = pxFspec;

	/*Create Dif Name and Daf Name*/
	pxName = pvPortMalloc(sizeof(*pxName));
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

	pxInst->pxData->pcIntefaceName = pcIntefaceName;

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
	vListInitialise(&(pxInst->pxData->xFlowsList));

	/*Initialialise instance item and add to the pxFactory*/
	/*vListInitialiseItem(&(pxInst->pxData->xInstanceListItem));
	listSET_LIST_ITEM_OWNER(&(pxInst->pxData->xInstanceListItem), pxInst);
	vListInsert(&(pxFactoryData->xInstancesShimWifiList), &(pxInst->pxData->xInstanceListItem));*/

	ESP_LOGI(TAG_SHIM, "Instance Created: %p, IPCP id:%d", pxInst, pxInst->pxData->xId);

	/*Enroll to DIF (Connect to AP)*/

	return pxInst;
}

/*Check this logic.*/
void vShimWiFiInit(ipcpInstance_t *pxShimWiFiInstance)
{
	/*xShimWiFiInit is going to init the  WiFi drivers and associate to the AP.
	 * Update de MacAddress variable depending on the WiFi drivers. Sent this variable
	 * as event data to be used when the shimWiFi DIF will be created.*/

	ESP_LOGI(TAG_SHIM, "SHIMWiFIInit NEW");
	RINAStackEvent_t xEnrollEvent = {eShimEnrolledEvent, NULL};
	const TickType_t xDontBlock = pdMS_TO_TICKS(50);

	if (!xShimEnrollToDIF(pxShimWiFiInstance->pxData->pxPhyDev))
	{
		ESP_LOGE(TAG_SHIM, "SHIM instance can't enroll to DIF");

		return pdFALSE;
	}
	else
	{
		xEnrollEvent.pvData = (void *)(pxShimWiFiInstance->pxData->pxPhyDev);
		xSendEventStructToIPCPTask(&xEnrollEvent, xDontBlock);
		return pdTRUE;
	}

	// return pdPASS;
}
