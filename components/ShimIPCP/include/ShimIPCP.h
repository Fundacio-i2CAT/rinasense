
//Include FreeRTOS

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "IPCP.h"
/*Application Level Configurations?
 * Shim_wifi name
 * Shim_wifi type (AP-STA)
 * Shim_wifi address
 */
#ifndef SHIM_IPCP_H__
#define SHIM_IPCP_H__







/*-------------------------------------------*/
/* FlowAllocateRequest (naming-info). Naming-info about the destination.
 * Primitive invoked by a source application to request a new flow:
 * - Check if there is a flow estabished (eALLOCATED), or a flow pending between the
 * source and destination application (ePENDING),
 * - If stated is eNULL then RINA_xARPMapping is called.
 * */
void Shim_flowAllocateRequest(APNameInfo_t destinationInfo);


/*-------------------------------------------*/
/* FlowDeallocate.
 * Primitive invoked by the application to discard all state regarding this flow.
 * - Port_id change to eNULL.
 * */
void Shim_flowDeallocate(void);


/*-------------------------------------------*/
/* applicationRegister (naming-info)
 * Primitive invoked before all other functions:
 * - Transform the naming-info structure into a single string (application-name)
 * separated by "/": ProcessName/ProcessInstance/EntityName/EntityInstance
 * - Call the RINA_ARPAdd API to map the application Name to the Hardware Address
 * in the cache ARP. (Update LocalAddressProtocol and LocalAddressMAC)
 * Return a pdTrue if Success or pdFalse Failure.
 * */
BaseType_t Shim_xApplicationRegister(APNameInfo_t destinationInfo);



/*-------------------------------------------*/
/* applicationUnregister (naming-info)
 * Primitive invoked before all other functions:
 * - Transform the naming-info into a single string (application-name)
 * separated by "/": ProcessName/ProcessInstance/EntityName/EntityInstance
 * - Call the RINA_ARPRemove API to remove the ARP
 * in the cache ARP.
 * Return a pdTrue if Success or pdFalse Failure.
 * */
BaseType_t Shim_xApplicationUnregister(void);


/*-------------------------------------------*/
/* Write (SDUs)
 * Primitive invoked by application to send SDUs.
 * - Create an Ethernet frame (802.11) using the Network Buffer Descriptor.
 * - Then put the Ethernet frame into the NetworkBufferDescriptor related to
 * Wifi layer.
 * */
void Shim_write(void);

/*-------------------------------------------*/
/* Read (SDUs)
 * Primitive invoked by application to read SDUs.
 *
 * */
void Shim_read(void);



/*-------------------------------------------*/
/* RINA_xEncodeIPCPAddress
 * Encode IPCP Address from APNameInfo_t to uint32_t IPCPaddress, even adding Padding.
 *
 * */
uint32_t RINA_xEncodeIPCPAddress (APNameInfo_t AplicationInfo );


#endif /* SHIM_IPCP_H__*/
