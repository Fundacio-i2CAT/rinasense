
#ifndef COMPONENTS_RMT_INCLUDE_RMT_H_
#define COMPONENTS_RMT_INCLUDE_RMT_H_


#include "IPCP.h"

#include "EFCP.h"
#include "efcpStructures.h"
#include "du.h"

#define TAG_RMT "RMT"

#define stats_inc(name, n1_port, bytes)					\
        n1_port->xStats.name##Pdus++;					\
	n1_port->xStats.name##Bytes += (unsigned int) bytes;		\

typedef enum FLOW_STATE {
	eN1_PORT_STATE_ENABLED = 0,
	eN1_PORT_STATE_DISABLED,
	eN1_PORT_STATE_DO_NOT_DISABLE,
	eN1_PORT_STATE_DEALLOCATED,
}eFlowState_t;

typedef struct xN1_PORT_STATS {
	unsigned int plen; /* port len, all pdus enqueued in PS queue/s */
	unsigned int dropPdus;
	unsigned int errPdus;
	unsigned int txPdus;
	unsigned int txBytes;
	unsigned int rxPdus;
	unsigned int rxBytes;
}n1PortStats_t;



typedef struct xRMT_N1_PORT {

	/* Is it required?
	 spinlock_t		lock;*/

	/*Port Id from Shim*/
	portId_t			xPortId;

	/* Shim Instance*/
	ipcpInstance_t	* 	pxN1Ipcp;

	/* State of the Port*/
	eFlowState_t		eState;

	/*Data Unit Pending to send*/
	struct du_t * 		pxPendingDu;

	/*N1 Port Statistics */
	n1PortStats_t		xStats;

	/* If the Port is Busy or not*/
	BaseType_t			uxBusy;

	/*???*/
	void * 	 			pvRmtPsQueues;

}rmtN1Port_t;

typedef struct xRMT_ADDRESS {
        address_t	 xAddress;
        ListItem_t   xAddressListItem;
}rmtAddress_t;

typedef struct xRMT
{
	/* Maybe Spinlock is required
	spinlock_t	      lock;*/
	List_t      					xAddresses;
	ipcpInstance_t * 				pxParent;
	efcpContainer_t * 				pxEfcpc;
	rmtN1Port_t * 					pxN1Port;
	struct rmt_Config_t * 			pxRmtCfg;

}rmt_t;


typedef struct xPORT_TABLE_ENTRY
{
	rmtN1Port_t * pxPortN1;

}portTableEntry_t;


pci_t * vCastPointerTo_pci_t(void * pvArgument);


#endif /* COMPONENTS_RMT_INCLUDE_DU_H_ */

