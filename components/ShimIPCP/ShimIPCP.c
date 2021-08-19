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


static ePortidState_t xFlowState = eNULL;

/** @brief  Concatenate the Information Application Name into a Complete Address
 * (ProcessName-ProcessInstance-EntityName-EntityInstance) or a Half Address
 * (ProcessName-ProcessInstance).
 * pdTrue = CompleteAddress
 * pdFalse = Half Address
 * */
string_t xConcatenateString(APNameInfo_t nameInfo, BaseType_t uCompleteAddress);

/******************** SHIM IPCP EventHandler **********/


/*-------------------------------------------*/
/* FlowAllocateRequest (naming-info). Naming-info about the destination.
 * Primitive invoked by the IPCP task event_handler:
 * - Check if there is a flow established (eALLOCATED), or a flow pending between the
 * source and destination application (ePENDING),
 * - If stated is eNULL then RINA_xARPAdd is called.
 * */
void Shim_flowAllocateRequest(APNameInfo_t destinationInfo)
{

	uint32_t ulIPCPAddress;
	RINAStackEvent_t xSendEvent;

	//Initialize ulIPCPAddress by converting the destinationInfo into a 32bits Address
	ulIPCPAddress = RINA_xEncodeIPCPAddress(destinationInfo );

	if ( xFlowState == eALLOCATED )
	{
		//Send an event to the IPCPTask responding as error
	}
	if ( xFlowState == ePENDING)
	{
		//Send an event to the IPCPTask responding as pending
	}
	if ( xFlowState == eNULL)
	{
		//Send a ARPADD REQUEST.
		RINA_vARPAdd( ulIPCPAddress );
		//Change PortState
		xFlowState = ePENDING;
	}
}

/*-------------------------------------------*/
/* applicationRegister (naming-info local)
 * Primitive invoked before all other functions:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "/": ProcessName/ProcessInstance/EntityName/EntityInstance
 * - (Update LocalAddressProtocol and LocalAddressMAC)
 * Return a pdTrue if Success or pdFalse Failure.
 * */
BaseType_t Shim_xApplicationRegister(APNameInfo_t localInfo)
{
	uint32_t ulIPCPAddress;

	//Initialize ulIPCPAddress by converting the destinationInfo into a 32bits Address
	ulIPCPAddress = RINA_xEncodeIPCPAddress(localInfo );

	//Send a ARPADD REQUEST to add the
	RINA_vARPAdd( ulIPCPAddress );

	return pdTRUE;


}


uint32_t RINA_xEncodeIPCPAddress( APNameInfo_t localInfo )
{
	uint32_t ulIPCPAddress;
	string_t uInfoConcatenated;

	//Concatenate the localInfo "ProcessName"+"-"+"ProcessInstance"
	uInfoConcatenated = xConcatenateString(localInfo, pdFALSE);

	//Convert the IPCPAddress Concatenate to bits

	//Shrink or grown the Address until fill the 32 bits.

	return ulIPCPAddress;

}


string_t xConcatenateString(APNameInfo_t nameInfo, BaseType_t uCompleteAddress)
{
	string_t uNameInfoConcatenated;
	string_t separationChar = "-";



	if (uCompleteAddress == pdTRUE)
	{
		uNameInfoConcatenated = malloc(strlen(nameInfo.pcProcessName)+strlen(nameInfo.pcProcessInstance)
				+3+strlen(nameInfo.pcEntityName)+strlen(nameInfo.pcEntityInstance));

		strcpy(uNameInfoConcatenated,nameInfo.pcProcessName);
		strcat(uNameInfoConcatenated, separationChar);
		strcat(uNameInfoConcatenated,nameInfo.pcProcessInstance);
		strcat(uNameInfoConcatenated, separationChar);
		strcat(uNameInfoConcatenated,nameInfo.pcEntityName);
		strcat(uNameInfoConcatenated, separationChar);
		strcat(uNameInfoConcatenated,nameInfo.pcEntityInstance);
	}
	else
	{
		uNameInfoConcatenated = malloc(strlen(nameInfo.pcProcessName)+strlen(nameInfo.pcProcessInstance)+1);

		strcpy(uNameInfoConcatenated,nameInfo.pcProcessName);
		strcat(uNameInfoConcatenated, separationChar);
		strcat(uNameInfoConcatenated,nameInfo.pcProcessInstance);
	}



	return uNameInfoConcatenated;
}
