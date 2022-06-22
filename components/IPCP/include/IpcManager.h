
#ifndef IPC_MANAGER_H__INCLUDED
#define IPC_MANAGER_H__INCLUDED

#include "pidm.h"
#include "ipcpIdm.h"
#include "RINA_API.h"
#include "EFCP.h"
#include "normalIPCP.h"

typedef struct xINSTANCE_TABLE_ROW
{
	/*The Ipcp Instance to register*/
	ipcpInstance_t *pxIpcpInstance;

	/*Type of the Ipcp Instance to register*/
	ipcpInstanceType_t pxIpcpType;

	/*The Ipcp Id to registered*/
	ipcProcessId_t xIpcpId;

	/*Is the Ipcp Instace active?*/
	BaseType_t xActive;

} InstanceTableRow_t;

typedef struct xIPC_MANAGER
{
	/*List of the Ipcp factories registered*/
	// factories_t *pxFactories;

	List_t xShimInstancesList;

	// flowAllocator_t * pxFlowAllocator;
	// InstanceTableRow_t * pxInstanceTable[ INSTANCES_IPCP_ENTRIES ];

	/*port Id manager*/
	pidm_t *pxPidm;

	/*IPCProcess Id manager*/
	ipcpIdm_t *pxIpcpIdm;

} ipcManager_t;

BaseType_t xIpcManagerInit(ipcManager_t *pxIpcManager);

void vIcpManagerEnrollmentFlowRequest(ipcpInstance_t *pxShimInstance, portId_t xN1PortId, name_t *pxIPCPName);

void vIpcpManagerAppFlowAllocateRequestHandle(flowAllocateHandle_t *pxFlowAllocateRequest);

// BaseType_t xIpcManagerWriteMgmtHandler(ipcpFactoryType_t xType, void *pxData);

ipcpInstance_t *pxIpcManagerFindInstanceById(ipcpInstanceId_t xIpcpId);
void vIpcManagerRINAPackettHandler(struct ipcpNormalData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer);

ipcpInstance_t *pxIpcManagerCreateShim(ipcManager_t *pxIpcManager);

#endif
