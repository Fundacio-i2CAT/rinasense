
/*Application Level Configurations?
 * Shim_wifi name
 * Shim_wifi type (AP-STA)
 * Shim_wifi address
 */

// Include FreeRTOS

#ifndef SHIM_IPCP_H__INCLUDED
#define SHIM_IPCP_H__INCLUDED

#include "ARP826.h"
#include "ARP826_defs.h"
#include "rina_gpha.h"
#include "du.h"
#include "rina_ids.h"
#include "rina_common_port.h"

typedef int32_t portId_t;

/* Flow states */
typedef enum xFLOW_STATES

{
	eNULL = 0,	// The Port_id cannot be used
	ePENDING,	// The protocol has either initiated the flow allocation or waiting for allocateResponse.
	eALLOCATED, // Flow allocated and the port_id can be used to read/write data from/to.
} ePortidState_t;

/* Holds the information related to one flow */

typedef struct xSHIM_WIFI_FLOW
{
	/* Harward Destination Address (MAC)*/
	gha_t *pxDestHa;

	/* Protocol Destination Address (Address RINA)*/
	gpa_t *pxDestPa;

	/* Port Id of???*/
	portId_t xPortId;

	/* State of the PortId */
	ePortidState_t ePortIdState;

	/* IPCP Instance who is going to use the Flow*/
	struct ipcpInstance * pxUserIpcp;

	/* Maybe this is not needed*/
	rfifo_t *pxSduQueue;

	/* Flow item to register in the List of Shim WiFi Flows */
	RsListItem_t		xFlowItem;
} shimFlow_t;

bool_t xShimEnrollToDIF( const MACAddress_t * pxPhyDev );

/*-------------------------------------------*/
/* FlowAllocateRequest (naming-info). Naming-info about the destination.
 * Primitive invoked by a source application to request a new flow:
 * - Check if there is a flow estabished (eALLOCATED), or a flow pending between the
 * source and destination application (ePENDING),
 * - If stated is eNULL then RINA_xARPMapping is called.
 * */
bool_t xShimFlowAllocateRequest(struct ipcpInstanceData_t *pxData,
                                name_t *pxSourceInfo,
                                name_t *pxDestinationInfo,
                                portId_t xPortId);


bool_t xShimFlowAllocateResponse(struct ipcpInstanceData_t *pxShimInstanceData, portId_t xPortId);

/*-------------------------------------------*/
/* FlowDeallocate.
 * Primitive invoked by the application to discard all state regarding this flow.
 * - Port_id change to eNULL.
 * */
bool_t xShimFlowDeallocate(struct ipcpInstanceData_t * pxData, portId_t xId);

/*-------------------------------------------*/
/* applicationRegister (naming-info)
 * Primitive invoked before all other functions:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "/": ProcessName/ProcessInstance/EntityName/EntityInstance
 * - Call the RINA_ARPAdd API to map the application Name to the Hardware Address
 * in the cache ARP. (Update LocalAddressProtocol and LocalAddressMAC)
 * Return a pdTrue if Success or pdFalse Failure.
 * */

bool_t xShimApplicationRegister(struct ipcpInstanceData_t *pxData,
                                name_t * pxAppName,
								name_t * pxDafName);

/*-------------------------------------------*/
/* applicationUnregister (naming-info)
 * Primitive invoked before all other functions:
 * - Transform the naming-info into a single string (application-name)
 * separated by "/": ProcessName/ProcessInstance/EntityName/EntityInstance
 * - Call the RINA_ARPRemove API to remove the ARP
 * in the cache ARP.
 * Return a pdTrue if Success or pdFalse Failure.
 * */
bool_t xShimApplicationUnregister(struct ipcpInstanceData_t *pxData, const name_t * pxName);

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

void vShimWiFiInit(ipcpInstance_t *pxShimWiFiInstance);

ipcpInstance_t *pxShimWiFiCreate(ipcProcessId_t xIpcpId);

bool_t xShimSDUWrite(struct ipcpInstanceData_t *pxData, portId_t xId, struct du_t *pxDu, bool_t uxBlocking);

EthernetHeader_t *vCastConstPointerTo_EthernetHeader_t(const void *pvArgument);

#endif /* SHIM_IPCP_H__*/
