
/*Application Level Configurations?
 * Shim_wifi name
 * Shim_wifi type (AP-STA)
 * Shim_wifi address
 */

//Include FreeRTOS



#ifndef SHIM_IPCP_H__INCLUDED
#define SHIM_IPCP_H__INCLUDED


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "IPCP.h"
#include "factoryIPCP.h"
#include "ARP826.h"
#include "du.h"



typedef int32_t  portId_t;


/* Flow states */
typedef enum xFLOW_STATES

{
	eNULL = 0,  // The Port_id cannot be used
	ePENDING,   // The protocol has either initiated the flow allocation or waiting for allocateResponse.
	eALLOCATED  // Flow allocated and the port_id can be used to read/write data from/to.
} ePortidState_t;

/* Holds the information related to one flow */

typedef struct xSHIM_WIFI_FLOW
{
	gha_t	*		pxDestHa;
	gpa_t	*		pxDestPa;

	portId_t 		xPortId;
	ePortidState_t 	ePortIdState;

	rfifo_t * 		pxSduQueue;
	ListItem_t		xFlowItem;


} shimFlow_t;



BaseType_t xShimEnrollToDIF( const MACAddress_t * pxPhyDev );

/*-------------------------------------------*/
/* FlowAllocateRequest (naming-info). Naming-info about the destination.
 * Primitive invoked by a source application to request a new flow:
 * - Check if there is a flow estabished (eALLOCATED), or a flow pending between the
 * source and destination application (ePENDING),
 * - If stated is eNULL then RINA_xARPMapping is called.
 * */
BaseType_t xShimFlowAllocateRequest(portId_t xId,

		const name_t * pxSourceInfo,
		const name_t * pxDestinationInfo,
		struct ipcpInstanceData_t * pxData);



BaseType_t xShimFlowAllocateResponse(ipcpInstance_t * pxShimInstance);


/*-------------------------------------------*/
/* FlowDeallocate.
 * Primitive invoked by the application to discard all state regarding this flow.
 * - Port_id change to eNULL.
 * */
BaseType_t xShimFlowDeallocate(struct ipcpInstanceData_t * pxData, portId_t xId);


/*-------------------------------------------*/
/* applicationRegister (naming-info)
 * Primitive invoked before all other functions:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "/": ProcessName/ProcessInstance/EntityName/EntityInstance
 * - Call the RINA_ARPAdd API to map the application Name to the Hardware Address
 * in the cache ARP. (Update LocalAddressProtocol and LocalAddressMAC)
 * Return a pdTrue if Success or pdFalse Failure.
 * */


BaseType_t xShimApplicationRegister(struct ipcpInstanceData_t *  pxData,name_t * pxName, const name_t * pxDafName);
/*-------------------------------------------*/
/* applicationUnregister (naming-info)
 * Primitive invoked before all other functions:
 * - Transform the naming-info into a single string (application-name)
 * separated by "/": ProcessName/ProcessInstance/EntityName/EntityInstance
 * - Call the RINA_ARPRemove API to remove the ARP
 * in the cache ARP.
 * Return a pdTrue if Success or pdFalse Failure.
 * */
BaseType_t xShimApplicationUnregister(struct ipcpInstanceData_t *  pxData ,name_t * pxName);


/*-------------------------------------------*/
/* Write (SDUs)
 * Primitive invoked by application to send SDUs.
 * - Create an Ethernet frame (802.11) using the Network Buffer Descriptor.
 * - Then put the Ethernet frame into the NetworkBufferDescriptor related to
 * Wifi layer.
 * */
void vShimWrite(void);

/*-------------------------------------------*/
/* Read (SDUs)
 * Primitive invoked by application to read SDUs.
 *
 * */
void vShimRead(void);






/* @brief Create a generic protocol address based on an string address*/
gpa_t * pxShimCreateGPA(const uint8_t * pucAddress, size_t xLength);

/* @brief Create a generic hardware address based on the MAC address*/
gha_t * pxShimCreateGHA(eGHAType_t xType, const MACAddress_t * pxAddress );


gpa_t * pxNameToGPA (const name_t * AplicationInfo );


void vNameDestroy(name_t * ptr );
void vNameFini ( name_t * n );

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


BaseType_t xShimWiFiInit( void );

BaseType_t xShimWiFiCreate( ipcpFactory_t * pxFactory, MACAddress_t * pxPhyDev );

BaseType_t xShimSDUWrite(struct ipcpInstanceData_t * pxData, portId_t xId, struct du_t * pxDu, BaseType_t uxBlocking);


#endif /* SHIM_IPCP_H__*/
