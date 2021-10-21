#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "Freertos/FreeRTOS.h"
#include "Freertos/task.h"
#include "Freertos/queue.h"
#include "Freertos/semphr.h"


#include "ShimIPCP.h"
#include "ARP826.h"
#include "IPCP.h"
#include "NetworkInterface.h"
#include "configRINA.h"
#include "BufferManagement.h"
#include "du.h"


#include "esp_log.h"







/* @brief Created a Queue type rfifo_t when a flow is allocated for request */
static rfifo_t * prvShimCreateQueue ( void );

/* @brief Find a flow previously allocated*/
static shimFlow_t * prvShimFindFlow (struct ipcpInstanceData_t * xData, portId_t xId);

/* @brief Destroy an specific Flow */
static BaseType_t prvShimFlowDestroy(struct ipcpInstanceData_t * xData, shimFlow_t * xFlow);


/* @brief Unbind and Destroy a Flow*/
static BaseType_t prvShimUnbindDestroyFlow(struct ipcpInstanceData_t * xData, shimFlow_t * xFlow);


/** @brief  Concatenate the Information Application Name into a Complete Address
 * (ProcessName-ProcessInstance-EntityName-EntityInstance).
 * */
string_t xShimNameToString(const name_t * xNameInfo); //Could be reutilized by others?, private?


/** @brief Convert the Complete Address (ProcessName-ProcessInstance-EntityName-EntityInstance)
 *  to Generic Protocol Address
 * */
gpa_t * pxShimNameToGPA (const name_t * xAplicationInfo );

/* @brief Destroy an application Name from the Data structure*/
void vShimNameDestroy(name_t * pxName );

/* @brief Destroy an application Name from the Data structure*/
void vShimNameFini ( name_t * pxName );

/* @brief Create a generic protocol address based on an string address*/
gpa_t * pxShimCreateGPA(const uint8_t * pucAddress, size_t xLength); //ARP ALSO CHECK

/* @brief Create a generic hardware address based on the MAC address*/
gha_t * pxShimCreateGHA(eGHAType_t xType, const MACAddress_t * pxAddress );

/* @brief Check if a generic protocol Address was created correctly*/
BaseType_t xShimIsGPAOK ( const gpa_t * pxGpa );

/* @brief Check if a generic hardware Address was created correctly*/
BaseType_t xShimIsGHAOK ( const gha_t * pxGha );

/* @brief Destroy a generic protocol Address to clean up*/
void vShimGPADestroy( gpa_t * pxGpa );

/* @brief Destroy a generic hardware Address to clean up*/
void vShimGHADestroy( gha_t * pxGha );



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
BaseType_t xShimEnrollToDIF( const MACAddress_t * pxPhyDev )
{

	BaseType_t xEnroll;


	//Initialization of WiFi interface
	xEnroll = xNetworkInterfaceInitialise( pxPhyDev );

	if (xEnroll == pdTRUE)
	{

		ESP_LOGI(TAG_SHIM, "Enrolled To DIF %s", SHIM_DIF_NAME);

		vARPInitCache();

		return pdTRUE;
	}
	else
	{

		ESP_LOGE(TAG_SHIM, "Failed to enroll to DIF %s", SHIM_DIF_NAME);

		return pdFALSE;
	}
}




/*-------------------------------------------*/
/* @brief Deallocate a flow
 * */

BaseType_t xShimFlowDeallocate(struct ipcpInstanceData_t * xData, portId_t xId)
{
	shimFlow_t * xFlow;

	if (!xData)
	{
		ESP_LOGE(TAG_SHIM,"Bogus data passed, bailing out");
		return pdFALSE;
	}

	if (!IS_PORT_ID_OK(xId))
	{
		ESP_LOGE(TAG_SHIM,"Invalid port ID passed, bailing out");
		return pdFALSE;
	}

	xFlow = prvShimFindFlow ( xData,xId );
	if (!xFlow)
	{
		ESP_LOGE(TAG_SHIM,"Flow does not exist, cannot remove");
		return pdFALSE;
	}

	return prvShimUnbindDestroyFlow(xData, xFlow);
}


/*-------------------------------------------*/
/* FlowAllocateRequest (naming-info). Naming-info about the destination.
 * Primitive invoked by the IPCP task event_handler:
 * - Check if there is a flow established (eALLOCATED), or a flow pending between the
 * source and destination application (ePENDING),
 * - If stated is eNULL then RINA_xARPAdd is called.
 * */
BaseType_t xShimFlowAllocateRequest( portId_t xId,

		const name_t * pxSourceInfo,
		const name_t * pxDestinationInfo,
		struct ipcpInstanceData_t * pxData)
{

	shimFlow_t * pxFlow;

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

	if (!IS_PORT_ID_OK(xId))
	{
		ESP_LOGE(TAG_SHIM, "Bogus data passed, bailing out");
		return pdFALSE;
	}


	pxFlow = prvShimFindFlow ( pxData, xId );

	if (!pxFlow)
	{
		pxFlow = pvPortMalloc(sizeof(*pxFlow));
		if (!pxFlow)
			return pdFALSE;

		pxFlow->xPortId = xId;
		pxFlow->ePortIdState = ePENDING;
		pxFlow->pxDestPa = pxShimNameToGPA(pxDestinationInfo);
		//user_ipcp???

		if (!xShimIsGPAOK(pxFlow->pxDestPa))
		{
			ESP_LOGE (TAG_SHIM, "Destination protocol address is not OK");
			prvShimUnbindDestroyFlow(pxData, pxFlow);

			return pdFALSE;
		}

		//Register the flow in a list or in the Flow allocator

		pxFlow->pxSduQueue = prvShimCreateQueue();
		if (!pxFlow->pxSduQueue)
		{
			ESP_LOGE (TAG_SHIM, "Destination protocol address is not ok");
			prvShimUnbindDestroyFlow(pxData, pxFlow);
			return pdFALSE;
		}



		//************ RINAARP RESOLVE GPA

		if(!xARPResolveGPA(pxFlow->pxDestPa, pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa))
		{
			prvShimUnbindDestroyFlow(pxData, pxFlow);
			return pdFALSE;
		}


	} else if (pxFlow->ePortIdState == ePENDING)
	{
		ESP_LOGE (TAG_SHIM, "Port-id state is already pending");
	} else
	{
		ESP_LOGE (TAG_SHIM, "Allocate called in a wrong state");
		return pdFALSE;
	}

	return pdTRUE;

}

/*-------------------------------------------*/
/* @brief Primitive invoked before all other functions:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "-": ProcessName-ProcessInstance-EntityName-EntityInstance
 * - (Update LocalAddressProtocol which is part of the ARP module).
 * It is assumed only there is going to be one IPCP process in the N-1 DIF (over the Shim-DIF)
 * @return a pdTrue if Success or pdFalse Failure.
 * */
BaseType_t xShimApplicationRegister(struct ipcpInstanceData_t *  pxData ,
		name_t * pxName, const name_t * pxDafName)
{
	gpa_t * pxPa;
	gha_t * pxHa;


	if ( !pxData )
	{
		ESP_LOGI(TAG_SHIM,"Data no valid ");
		return pdFALSE;
	}
	if ( !pxName )
	{
		ESP_LOGI(TAG_SHIM,"Name no valid ");
		return pdFALSE;
	}
	/*if (data->appName)
	{
		ESP_LOGI(TAG_SHIM,"AppName should not exist ");
		return -1;
	}*/


	pxData->pxAppName = pvPortMalloc(sizeof(pxName));

	if ( !pxData->pxAppName)
	{
		ESP_LOGI(TAG_SHIM,"AppName not created ");
		return pdFALSE;
	}
	pxPa = pxShimNameToGPA(pxName);

	if ( !xShimIsGPAOK(pxPa))
	{
		ESP_LOGI(TAG_SHIM,"Protocol Address is not OK ");
		vShimNameFini (pxData->pxAppName);
		return pdFALSE;
	}


	pxHa = pxShimCreateGHA(MAC_ADDR_802_3, pxData->pxPhyDev);


	if (!xShimIsGHAOK (pxHa) )
	{
		ESP_LOGI(TAG_SHIM,"Hardware Address is not OK ");
		vShimNameFini (pxData->pxAppName);
		vShimGHADestroy( pxHa );
		return pdFALSE;
	}


	pxData->pxAppHandle = pxARPAdd(pxPa, pxHa);



	if (!pxData->pxAppHandle)
	{
		//destroy all
		ESP_LOGI(TAG_SHIM,"APPHandle was not created ");
		vShimGPADestroy( pxPa );
		vShimGHADestroy( pxHa );
		vShimNameFini (pxData->pxAppName);
		return pdFALSE;
	}

	//vShimGPADestroy( pa );

	if (pxDafName) {
		pxData->pxDafName = pvPortMalloc(sizeof(pxDafName));



		if (!pxData->pxDafName) {

			ESP_LOGE(TAG_SHIM,"Removing ARP Entry for DAF");
			xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
			pxData->pxAppHandle = NULL;
			vShimNameDestroy(pxData->pxAppName);
			vShimGHADestroy(pxHa);
			return pdFALSE;
		}

		pxPa = pxShimNameToGPA(pxDafName);


		if (!xShimIsGPAOK(pxPa)) {

			ESP_LOGE(TAG_SHIM,"Failed to create gpa");
			xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
			pxData->pxAppHandle = NULL;
			vShimNameDestroy(pxData->pxDafName);
			vShimNameDestroy(pxData->pxAppName);
			vShimGHADestroy(pxHa);
			return pdFALSE;
		}

		pxData->pxDafHandle = pxARPAdd(pxPa, pxHa);

		if (!pxData->pxDafHandle) {
			ESP_LOGE(TAG_SHIM,"Failed to register DAF in ARP");
			xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa);
			pxData->pxAppHandle = NULL;
			vShimNameDestroy(pxData->pxAppName);
			vShimNameDestroy(pxData->pxDafName);
			vShimGPADestroy(pxPa);
			vShimGHADestroy(pxHa);

			return pdFALSE;
		}

		//vShimGPADestroy(pa);
	}

	//vShimGHADestroy(ha);

	ESP_LOGI(TAG_SHIM, "Application Registered");


	return pdTRUE;

}


/*-------------------------------------------*/
/* applicationUnregister (naming-info local)
 * Primitive invoked before all other functions:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "-": ProcessName-ProcessInstance-EntityName-EntityInstance
 * - (Update LocalAddressProtocol which is part of the ARP module).
 * It is assumed only there is going to be one IPCP process in the N-1 DIF (over the Shim-DIF)
 *  * Return a pdTrue if Success or pdFalse Failure.
 * */


BaseType_t xShimApplicationUnregister(struct ipcpInstanceData_t *  pxData ,name_t * pxName)
{
	ESP_LOGI(TAG_SHIM,"Application Unregistering");

	if (!pxData) {
		ESP_LOGE(TAG_SHIM,"Bogus data passed, bailing out");
		return pdFALSE;
	}

	if (!pxName) {
		ESP_LOGE(TAG_SHIM,"Invalid name passed, bailing out");
		return pdFALSE;
	}

	if (!pxData->pxAppName) {
		ESP_LOGE(TAG_SHIM, "Shim-WiFi has no application registered");
		return pdFALSE;
	}

	/* Remove from ARP cache */
	if (pxData->pxAppHandle)
	{


		if (xARPRemove(pxData->pxAppHandle->pxPa, pxData->pxAppHandle->pxHa))
		{
			ESP_LOGE(TAG_SHIM,"Failed to remove APP entry from the cache");
			return pdFALSE;
		}
		pxData->pxAppHandle = NULL;

	}

	if (pxData->pxDafHandle)
	{

		if (xARPRemove(pxData->pxDafHandle->pxPa, pxData->pxDafHandle->pxHa))
		{
			ESP_LOGE(TAG_SHIM,"Failed to remove DAF entry from the cache");
			return pdFALSE;
		}
		pxData->pxDafHandle = NULL;

	}

	vShimNameDestroy(pxData->pxAppName);
	pxData->pxAppName = NULL;
	vShimNameDestroy(pxData->pxDafName);
	pxData->pxDafName = NULL;

	ESP_LOGI(TAG_SHIM,"Application unregister");

	return pdTRUE;
}


/***************** **************************/


gpa_t * pxShimNameToGPA ( const name_t * xLocalInfo )
{
	//uint32_t ulIPCPAddress;
	gpa_t * 	pxGpa;
	string_t 	xTmp;


	//Concatenate the localInfo "ProcessName"+"-"+"ProcessInstance"
	xTmp = xShimNameToString(xLocalInfo);


	if (!xTmp)
	{
		ESP_LOGI(TAG_SHIM, "Name to String not correct");
		return NULL;
	}

	//Convert the IPCPAddress Concatenate to bits
	pxGpa = pxShimCreateGPA((uint8_t *)(xTmp), strlen(xTmp));

	if (!pxGpa)
	{
		ESP_LOGI(TAG_SHIM, "GPA was not created correct");
		vPortFree(xTmp);
		return NULL;
	}

	vPortFree(xTmp);

	return pxGpa;

}

gpa_t * pxShimCreateGPA(const uint8_t * pucAddress, size_t xLength)
{
	gpa_t * pxGPA;


	if(!pucAddress || xLength == 0)
	{
		ESP_LOGI(TAG_SHIM, "Bad input parameters, cannot create GPA");
		return NULL;
	}

	pxGPA = pvPortMalloc(sizeof(*pxGPA));

	if (!pxGPA)
		return NULL;

	pxGPA->uxLength = xLength;
	pxGPA->ucAddress = pvPortMalloc(pxGPA->uxLength);
	if (!pxGPA->ucAddress)
	{
		vPortFree(pxGPA);
		return NULL;
	}

	memcpy (pxGPA->ucAddress, pucAddress, pxGPA->uxLength);

	ESP_LOGD(TAG_SHIM, "CREATE GPA address: %s", pxGPA->ucAddress);
	ESP_LOGD(TAG_SHIM, "CREATE GPA size: %d", pxGPA->uxLength);

	return pxGPA;

}

gha_t * pxShimCreateGHA(eGHAType_t xType, const MACAddress_t * pxAddress ) //Changes to uint8_t
{
	gha_t * pxGha;

	if (xType != MAC_ADDR_802_3 || !pxAddress->ucBytes)
	{
		ESP_LOGE(TAG_SHIM, "Wrong input parameters, cannot create GHA");
		return NULL;
	}
	pxGha = pvPortMalloc(sizeof(*pxGha));
	if(!pxGha)
		return NULL;

	pxGha->xType = xType;
	memcpy(pxGha->xAddress.ucBytes, pxAddress->ucBytes, sizeof(pxGha->xAddress));

	return pxGha;
}


string_t xShimNameToString( const name_t * xNameInfo)
{
	string_t xNameInfoConcatenated;

	if (!xNameInfo)
		return NULL;


	xNameInfoConcatenated = pvPortMalloc(strlen(xNameInfo->pcProcessName)+strlen(xNameInfo->pcProcessInstance)
			+3+strlen(xNameInfo->pcEntityName)+strlen(xNameInfo->pcEntityInstance));

	strcpy(xNameInfoConcatenated,xNameInfo->pcProcessName);
	strcat(xNameInfoConcatenated, DELIMITER);
	strcat(xNameInfoConcatenated,xNameInfo->pcProcessInstance);
	strcat(xNameInfoConcatenated, DELIMITER);
	strcat(xNameInfoConcatenated,xNameInfo->pcEntityName);
	strcat(xNameInfoConcatenated, DELIMITER);
	strcat(xNameInfoConcatenated,xNameInfo->pcEntityInstance);

	if (!xNameInfoConcatenated)
	{
		vPortFree(xNameInfoConcatenated);
		return NULL;
	}

	ESP_LOGD(TAG_SHIM,"Concatenated: %s", xNameInfoConcatenated);

	return xNameInfoConcatenated;
}



static shimFlow_t * prvShimFindFlow (struct ipcpInstanceData_t * xData, portId_t xId)
{
	shimFlow_t * xFlow;

	return NULL;

}

static rfifo_t * prvShimCreateQueue ( void )
{
	rfifo_t * xFifo = pvPortMalloc(sizeof(*xFifo));

	xFifo->xQueue = xQueueCreate( SIZE_SDU_QUEUE, sizeof( uint32_t ) );

	if ( !xFifo->xQueue )
	{
		vPortFree(xFifo);
		return NULL;
	}

	return xFifo;
}

int QueueDestroy(rfifo_t * f,
                  void        (* dtor)(void * e))
{
        if (!f) {
                ESP_LOGE(TAG_SHIM,"Bogus input parameters, can't destroy NULL");
                return -1;
        }
        if (!dtor) {
        	ESP_LOGE(TAG_SHIM,"Bogus input parameters, no destructor provided");
                return -1;
        }

        vQueueDelete(f->xQueue);

        vPortFree(f);

        ESP_LOGI(TAG_SHIM,"FIFO %pK destroyed successfully", f);

        return 0;
}

//Move to ARP

BaseType_t xShimIsGPAOK ( const gpa_t * pxGpa )
{

	if (!pxGpa)
	{
		ESP_LOGI(TAG_SHIM, " !Gpa");
		return pdFALSE;
	}
	if ( pxGpa->ucAddress == NULL)
	{

		ESP_LOGI(TAG_SHIM, "xShimIsGPAOK Address is NULL");
		return pdFALSE;
	}

	if ( pxGpa->uxLength == 0 )
	{
		ESP_LOGI(TAG_SHIM, "Length = 0");
		return pdFALSE;
	}
	return pdTRUE;
}

BaseType_t xShimIsGHAOK ( const gha_t * pxGha )
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



void vShimGPADestroy( gpa_t * pxGpa )
{
	if (!xShimIsGPAOK(pxGpa))
	{
		return;

	}
	vPortFree( pxGpa->ucAddress );
	vPortFree( pxGpa );

	return;
}

void vShimGHADestroy( gha_t * pxGha )
{
	if (!xShimIsGHAOK(pxGha))
	{
		return;

	}

	vPortFree( pxGha );

	return;
}

void vShimNameDestroy ( name_t * pxName)
{

	vPortFree(pxName);
}


void vShimNameFini ( name_t * pxName)
{


        if (pxName->pcProcessName) {
                vPortFree(pxName->pcProcessName);
                pxName->pcProcessName = NULL;
        }
        if (pxName->pcProcessInstance) {
                vPortFree(pxName->pcProcessInstance);
                pxName->pcProcessInstance = NULL;
        }
        if (pxName->pcEntityName) {
                vPortFree(pxName->pcEntityName);
                pxName->pcEntityName = NULL;
        }
        if (pxName->pcEntityInstance) {
                vPortFree(pxName->pcEntityInstance);
                pxName->pcEntityInstance = NULL;
        }

        ESP_LOGI(TAG_SHIM, "Name at %pK finalized successfully", pxName);
}


static BaseType_t prvShimUnbindDestroyFlow(struct ipcpInstanceData_t * xData,
                                   shimFlow_t *   xFlow)
{

/*
	if (flow->user_ipcp) {
		ASSERT(flow->user_ipcp->ops);
		flow->user_ipcp->ops->
		flow_unbinding_ipcp(flow->user_ipcp->data,
				flow->port_id);
	}*/ //Check this
	ESP_LOGI(TAG_SHIM,"Shim-WiFi unbinded port: %u", xFlow->xPortId);
	if (prvShimFlowDestroy(xData, xFlow)) {
		ESP_LOGE(TAG_SHIM,"Failed to destroy Shim-WiFi flow");
		return pdFALSE;
	}

	return pdTRUE;
}


static BaseType_t prvShimFlowDestroy(struct ipcpInstanceData_t * xData, shimFlow_t * xFlow)
{

	/* FIXME: Complete what to do with xData*/
	if (xFlow->pxDestPa) vShimGPADestroy(xFlow->pxDestPa);
	if (xFlow->pxDestHa) vShimGHADestroy(xFlow->pxDestHa);
	if (xFlow->pxSduQueue)
		vQueueDelete(xFlow->pxSduQueue->xQueue);
	vPortFree(xFlow);

	return pdTRUE;
}


BaseType_t xShimSDUWrite(struct ipcpInstanceData_t * pxData, portId_t xId, du_t * pxDu, BaseType_t uxBlocking)
{

	shimFlow_t *   					pxFlow;
	NetworkBufferDescriptor_t * 	pxNetworkBuffer;

	EthernetHeader_t *				pxEthernetHeader;
	gha_t *    						pxSrcHw;
	gha_t *    						pxDestHw;
	size_t                     		uxHeadLen, uxLength;

	unsigned char * 				pucArpPtr;

	ESP_LOGI(TAG_SHIM,"Entered the sdu-write");

	if (unlikely(!pxData)) {
		ESP_LOGE(TAG_SHIM,"Bogus data passed, bailing out");
		return pdFALSE;
	}

	uxHeadLen   = sizeof(EthernetHeader_t);//Header length Ethernet
	//tlen   = data->dev->needed_tailroom; check it is required?
	uxLength = pxDu->pxNetworkBuffer->xDataLength; //total length PDU

	if (unlikely(uxLength > MTU)) {
		ESP_LOGE(TAG_SHIM,"SDU too large (%d), dropping", uxLength);
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	pxFlow = prvShimFindFlow(pxData, xId);
	if (!pxFlow) {
		ESP_LOGE(TAG_SHIM,"Flow does not exist, you shouldn't call this");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	//spin_lock_bh(&data->lock);
	if (pxFlow->ePortIdState != eALLOCATED) {
		ESP_LOGE(TAG_SHIM,"Flow is not in the right state to call this");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	if (pxData->ucTxBusy) {
		//spin_unlock_bh(&data->lock);
		return pdFAIL;
	}
	//spin_unlock_bh(&data->lock);

	pxSrcHw = pxShimCreateGHA(MAC_ADDR_802_3,pxData->pxPhyDev);
	if (!pxSrcHw) {
		ESP_LOGE(TAG_SHIM,"Failed to get source HW addr");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	pxDestHw = pxShimCreateGHA(MAC_ADDR_802_3, pxFlow->pxDestHa);//check or implement
	if (!pxDestHw) {
		ESP_LOGE(TAG_SHIM,"Destination HW address is unknown");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	/* Get a Network Buffer with size total ethernet + PDU size*/
	pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( uxHeadLen + uxLength, ( TickType_t ) 0U );



	if( pxNetworkBuffer == NULL )
	{
		ESP_LOGE(TAG_SHIM,"pxNetworkBuffer is null");
		xDuDestroy(pxDu);
		return pdFALSE;
	}

	pxEthernetHeader = CAST_CONST_PTR_TO_CONST_TYPE_PTR( EthernetHeader_t, pxNetworkBuffer->pucEthernetBuffer );

	pxEthernetHeader->usFrameType = FreeRTOS_htons(ETH_P_RINA);

	memcpy(pxEthernetHeader->xSourceAddress.ucBytes, pxSrcHw->xAddress.ucBytes, sizeof(pxSrcHw->xAddress));
	memcpy(pxEthernetHeader->xDestinationAddress.ucBytes, pxDestHw->xAddress.ucBytes, sizeof(pxDestHw->xAddress));

	/*Copy from the buffer PDU to the buffer Ethernet*/
	pucArpPtr = (unsigned char *)(pxEthernetHeader + 1);

	memcpy(pucArpPtr, pxDu->pxNetworkBuffer->pucEthernetBuffer, uxLength);

	pxNetworkBuffer->xDataLength = uxHeadLen + uxLength;

	/* Generate an event to sent or send from here*/

	/* ReleaseBuffer, no need anymore that why pdTRUE here*/
	( void ) xNetworkInterfaceOutput( pxNetworkBuffer, pdTRUE );

	/*Destroy pxDU no need anymore the stackbuffer*/
	xDuDestroy(pxDu);

	ESP_LOGE(TAG_SHIM,"Packet sent");

	return pdTRUE;


}



/************* CREATED, DESTROY, INIT, CLEAN SHIM IPCP ******/

BaseType_t xShimWiFiCreate( MACAddress_t * pxPhyDev )
{
	ipcpInstance_t * pxInst;
	string_t 		xIntefaceName = SHIM_INTERFACE;

	name_t * pxName;
	name_t * pxDafName;

	/* Create an instance */
	pxInst = pvPortMalloc(sizeof(pxInst));

	pxInst->pxData = pvPortMalloc(sizeof(struct ipcpInstanceData_t));
	pxInst->pxData->pxFspec = pvPortMalloc(sizeof(struct flowSpec_t));

	pxName = pvPortMalloc(sizeof(*pxName));
	pxDafName = pvPortMalloc(sizeof(struct ipcpInstanceData_t));

	pxName->pcProcessName = SHIM_PROCESS_NAME;
	pxName->pcEntityName = SHIM_ENTITY_NAME;
	pxName->pcProcessInstance = SHIM_PROCESS_INSTANCE;
	pxName->pcEntityInstance = SHIM_ENTITY_INSTANCE;

	pxDafName->pcProcessName = SHIM_DAF_PROCESS_NAME;
	pxDafName->pcEntityName = SHIM_DAF_ENTITY_NAME;
	pxDafName->pcProcessInstance = SHIM_DAF_PROCESS_INSTANCE;
	pxDafName->pcEntityInstance = SHIM_DAF_ENTITY_INSTANCE;

	/*Testing Proposes*/
	name_t * destinationInfo = pvPortMalloc(sizeof(*destinationInfo));
	destinationInfo->pcProcessName = "mobile.DIF";
	destinationInfo->pcEntityName = "";
	destinationInfo->pcProcessInstance = "";
	destinationInfo->pcEntityInstance = "";

	pxInst->pxData->pxName = pxName;
	pxInst->pxData->xId = 1;
	pxInst->pxData->pxPhyDev = pxPhyDev;
	pxInst->pxData->pxDafName = pxDafName;
	pxInst->pxData->xIntefaceName = xIntefaceName;
	pxInst->pxData->pxFspec->ulAverageBandwidth          = 0;
	pxInst->pxData->pxFspec->ulAverageSduBandwidth       = 0;
	pxInst->pxData->pxFspec->ulDelay                     = 0;
	pxInst->pxData->pxFspec->ulJitter                    = 0;
	pxInst->pxData->pxFspec->ulMaxAllowableGap           = -1;
	pxInst->pxData->pxFspec->ulMaxSduSize                = 1500;
	pxInst->pxData->pxFspec->xOrderedDelivery            = 0;
	pxInst->pxData->pxFspec->xPartialDelivery            = 1;
	pxInst->pxData->pxFspec->ulPeakBandwidthDuration     = 0;
	pxInst->pxData->pxFspec->ulPeakSduBandwidthDuration  = 0;
	pxInst->pxData->pxFspec->ulUndetectedBitErrorRate    = 0;

	if(!xShimApplicationRegister(pxInst->pxData ,
			pxInst->pxData->pxName, pxInst->pxData->pxDafName))
	{
		ESP_LOGE(TAG_SHIM, "SHIM instance can't application register");
	}

	if(!xShimFlowAllocateRequest(pxInst->pxData->xId,pxInst->pxData->pxName,destinationInfo,pxInst->pxData))
	{
		ESP_LOGE(TAG_SHIM, "SHIM flow allocate Failed");
	}
	ESP_LOGI(TAG_SHIM, "SHIM instance created");

	return pdPASS;

}

BaseType_t xShimWiFiInit( void )
{

    RINAStackEvent_t xEnrollEvent = { eShimEnrollEvent, NULL };
    const TickType_t xDontBlock = pdMS_TO_TICKS( 50 );

	MACAddress_t  *   pxPhyDev;

	pxPhyDev = pvPortMalloc(sizeof(*pxPhyDev));

	pxPhyDev->ucBytes[0] = 0x00;
	pxPhyDev->ucBytes[1] = 0x00;
	pxPhyDev->ucBytes[2] = 0x00;
	pxPhyDev->ucBytes[3] = 0x00;
	pxPhyDev->ucBytes[4] = 0x00;
	pxPhyDev->ucBytes[5] = 0x00;


	if(!xShimEnrollToDIF( pxPhyDev ))
	{
		ESP_LOGE(TAG_SHIM, "SHIM instance can't enroll to DIF");
	}

	xEnrollEvent.pvData = (void * ) (pxPhyDev);
	xSendEventStructToIPCPTask(&xEnrollEvent, xDontBlock);

	return pdPASS;
}


static struct ipcpInstanceOps_t xShimWifiOps = {
        .flowAllocateRequest     = xShimFlowAllocateRequest, //ok
        .flowAllocateResponse    = NULL, //ok
        .flowDeallocate           = xShimFlowDeallocate, //ok
        .flowPrebind              = NULL, //ok
        .flowBindingIpcp         = NULL, //ok
        .flowUnbindingIpcp       = NULL, //ok
        .flowUnbindingUserIpcp  = NULL, //ok
		.nm1FlowStateChange	   = NULL, //ok

        .applicationRegister      = xShimApplicationRegister,//ok
        .applicationUnregister    = xShimApplicationUnregister, //ok

        .assignToDif             = NULL, //ok
        .updateDifConfig         = NULL, //ok

        .connectionCreate         = NULL, //ok
        .connectionUpdate         = NULL, //ok
        .connectionDestroy        = NULL, //ok
        .connectionCreateArrived = NULL, // ok
		.connectionModify 	   = NULL, //ok

        .duEnqueue               = NULL, //ok
        .duWrite                 = xShimSDUWrite, //ok

        .mgmtDuWrite            = NULL, //ok
        .mgmtDuPost             = NULL, //ok

        .pffAdd                   = NULL,//ok
        .pffRemove                = NULL, //ok
        //.pff_dump                  = NULL,
        //.pff_flush                 = NULL,
		//.pff_modify		   		   = NULL,

        //.query_rib		  		   = NULL,

        .ipcpName                 = NULL,//ok
        .difName                  = NULL, //ok
		//.ipcp_id		  		   = NULL,

        //.set_policy_set_param      = NULL,
        //.select_policy_set         = NULL,
        //.update_crypto_state	   = NULL,
		//.address_change            = NULL,
        //.dif_name		   		   = NULL,
		.maxSduSize		   	   = NULL
};
