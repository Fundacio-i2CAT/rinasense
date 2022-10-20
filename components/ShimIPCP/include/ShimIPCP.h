
/*Application Level Configurations?
 * Shim_wifi name
 * Shim_wifi type (AP-STA)
 * Shim_wifi address
 */

// Include FreeRTOS

#ifndef SHIM_IPCP_H__INCLUDED
#define SHIM_IPCP_H__INCLUDED

#include "common/rina_ids.h"
#include "common/rina_gpha.h"
#include "common/simple_queue.h"

#include "ARP826.h"
#include "ARP826_defs.h"
#include "du.h"
#include "rina_common_port.h"
#include "IpcManager.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPCP_INSTANCE_DATA_ETHERNET_SHIM 0xA

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
	/* Hardware destination Address (MAC)*/
	const gha_t *pxDestHa;

	/* Protocol Destination Address (Address RINA)*/
	const gpa_t *pxDestPa;

	/* Port Id of???*/
	portId_t xPortId;

	/* State of the PortId */
	ePortidState_t ePortIdState;

	/* IPCP Instance who is going to use the Flow*/
	struct ipcpInstance * pxUserIpcp;

	/* Maybe this is not needed*/
	RsSimpleQueue_t xSduQueue;

	/* Flow item to register in the List of Shim WiFi Flows */
	RsListItem_t xFlowItem;
} shimFlow_t;

bool_t xShimEnrollToDIF(struct ipcpInstanceData_t *pxData);

bool_t xShimWiFiInit(struct ipcpInstance_t *pxShimWiFiInstance);

struct ipcpInstance_t *pxShimWiFiCreate(ipcProcessId_t xIpcpId);

bool_t xShimSDURead(struct ipcpInstanceData_t *pxData,
                    portId_t xId,
                    struct du_t *pxDu);

void vShimHandleEthernetPacket(void *pxShimData,
                               NetworkBufferDescriptor_t *pxNetworkBuffer);

#ifdef __cplusplus
}
#endif

#endif /* SHIM_IPCP_H__*/
