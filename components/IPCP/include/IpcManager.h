
#ifndef IPC_MANAGER_H__INCLUDED
#define IPC_MANAGER_H__INCLUDED

#include "pidm.h"
#include "ipcpIdm.h"

typedef struct xINSTANCE_TABLE_ROW{
	/*The Ipcp Instance to register*/
	ipcpInstance_t * pxIpcpInstance;

	/*Type of the Ipcp Instance to register*/
	ipcpInstanceType_t pxIpcpType;

	/*The Ipcp Id to registered*/
	ipcProcessId_t	xIpcpId;

	/*Is the Ipcp Instace active?*/
	BaseType_t 	xActive;

}InstanceTableRow_t;

typedef struct xIPC_MANAGER{
	/*List of the Ipcp factories registered*/
    factories_t * pxFactories;

    //flowAllocator_t * pxFlowAllocator;
    //InstanceTableRow_t * pxInstanceTable[ INSTANCES_IPCP_ENTRIES ];

	/*port Id manager*/
	pidm_t * pxPidm;

	/*IPCProcess Id manager*/
	ipcpIdm_t * pxIpcpIdm;

}ipcManager_t;

BaseType_t xIpcManagerInit( ipcManager_t * pxIpcManager );

BaseType_t xIcpManagerNormalRegister(factories_t *pxFactories, ipcpFactoryType_t xFactoryTypeFrom,  ipcpFactoryType_t xFactoryTypeTo);

BaseType_t xIpcManagerCreateInstance(factories_t *pxFactories, ipcpFactoryType_t xFactoryType, ipcpIdm_t * pxIpcpIdManager);

BaseType_t xIcpManagerEnrollmentFlowRequest(factories_t *pxFactories, ipcpFactoryType_t xFactoryTypeFrom,  ipcpFactoryType_t xFactoryTypeTo, pidm_t * pxPidm);

BaseType_t xIpcpManagerShimAllocateResponseHandle(factories_t *pxFactories, ipcpFactoryType_t xShimType);
BaseType_t xIpcpManagerPreBindFlow(factories_t *pxFactories, ipcpFactoryType_t xNormalType);


BaseType_t xIpcpManagerAppFlowAllocateRequestHandle(pidm_t * pxPidm, void * data);
BaseType_t xIpcManagerWriteMgmtHandler(ipcpFactoryType_t xType, void *pxData);

ipcpInstance_t *pxIpcManagerFindInstanceById(ipcpInstanceId_t xIpcpId);
void vIpcManagerRINAPackettHandler(NetworkBufferDescriptor_t * pxNetworkBuffer);



#endif
