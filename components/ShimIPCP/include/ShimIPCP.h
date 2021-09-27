
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
#include "ARP826.h"



typedef int32_t  portId_t;

/* Represents the configuration of the EFCP */
/*
typedef struct  xEFCP_CONFIG{
	 // The data transfer constants
	struct dt_cons * dt_cons;

	size_t *		pci_offset_table;

	 // FIXME: Left here for phase 2
	//struct policy * unknown_flow;

	// List of qos_cubes supported by the EFCP config
	//struct list_head qos_cubes;

}efcpConfig_t;
*/

/*
typedef struct xSDU {
	//Configuration of EFCP (Policies, QoS, etc)
	efcpConfig_t *		cfg;

	struct pci pci;

	NetworkBufferDescriptor_t * pxNetworkBuffer;

}sdu_t;
*/

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


} shimFlow_t;



typedef struct xSHIM_IPCP_INSTANCE_DATA
{

	//list_head       list;
	ipcProcessId_t       xId;

	/* IPC Process name */
	name_t *         		pxName;
	name_t *          		pxDifName;
	string_t 				xIntefaceName;
	//struct packet_type *   	eth_vlan_packet_type;
	//struct net_device *    	dev;
	MACAddress_t  *    		pxPhyDev;
	flowSpec_t *     		pxFspec;

	/* The IPC Process using the shim-eth-vlan */
	name_t *          		pxAppName;
	name_t *         		pxDafName;

	/* Stores the state of flows indexed by port_id */
	//spinlock_t             lock;
	//struct list_head       flows;

	/* FIXME: Remove it as soon as the kipcm_kfa gets removed */
	//struct kfa *           kfa;

	/* RINARP related */
	rinarpHandle_t * 		pxAppHandle;
	rinarpHandle_t * 		pxDafHandle;

	/* To handle device notifications. */
	//struct notifier_block ntfy;

	/* Flow control between this IPCP and the associated netdev. */
	unsigned int 			ucTxBusy;

}shimInstanceData_t;



typedef struct  xIPCP_INSTANCE
{
		ipcpInstanceId_t         xId;
        ipcpInstanceType_t		 xType;
        shimInstanceData_t * 	 pxData;

        //struct ipcp_instance_ops *  ops;
}ipcpInstance_t;

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
		shimInstanceData_t * pxData);


/*-------------------------------------------*/
/* FlowDeallocate.
 * Primitive invoked by the application to discard all state regarding this flow.
 * - Port_id change to eNULL.
 * */
BaseType_t xShimFlowDeallocate(shimInstanceData_t * pxData, portId_t xId);


/*-------------------------------------------*/
/* applicationRegister (naming-info)
 * Primitive invoked before all other functions:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "/": ProcessName/ProcessInstance/EntityName/EntityInstance
 * - Call the RINA_ARPAdd API to map the application Name to the Hardware Address
 * in the cache ARP. (Update LocalAddressProtocol and LocalAddressMAC)
 * Return a pdTrue if Success or pdFalse Failure.
 * */


BaseType_t xShimApplicationRegister(shimInstanceData_t *  pxData,name_t * pxName, const name_t * pxDafName);
/*-------------------------------------------*/
/* applicationUnregister (naming-info)
 * Primitive invoked before all other functions:
 * - Transform the naming-info into a single string (application-name)
 * separated by "/": ProcessName/ProcessInstance/EntityName/EntityInstance
 * - Call the RINA_ARPRemove API to remove the ARP
 * in the cache ARP.
 * Return a pdTrue if Success or pdFalse Failure.
 * */
BaseType_t xShimApplicationUnregister(shimInstanceData_t *  pxData ,name_t * pxName);


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

//BaseType_t Shim_xSDUWrite(ipcpInstanceData_t * data, portId_t id, sdu_t * du, BaseType_t blocking);

#endif /* SHIM_IPCP_H__*/
