
#ifndef IPC_MANAGER_H__INCLUDED
#define IPC_MANAGER_H__INCLUDED

#include "pidm.h"
#include "ipcpIdm.h"

typedef struct xINSTANCE_TABLE_ROW{
	ipcpInstance_t * pxIpcpInstance;
	ipcpInstanceType_t pxIpcpType;
	ipcProcessId_t	xIpcpId;
	BaseType_t 	xActive;

}InstanceTableRow_t;

typedef struct xIPC_MANAGER_{
    factories_t * pxFactories;
    //flowAllocator_t * pxFlowAllocator;
    //InstanceTableRow_t * pxInstanceTable[ INSTANCES_IPCP_ENTRIES ];
	pidm_t * pxPidm;
	ipcpIdm_t * pxIpcpIdm;
}ipcManager_t;

BaseType_t xIpcManagerInit( ipcManager_t * pxIpcManager );

BaseType_t xIcpManagerNormalRegister(factories_t *pxFactories, ipcpFactoryType_t xFactoryTypeFrom,  ipcpFactoryType_t xFactoryTypeTo);

BaseType_t xIpcManagerCreateInstance(factories_t *pxFactories, ipcpFactoryType_t xFactoryType, ipcpIdm_t * pxIpcpIdManager);

BaseType_t xIcpManagerEnrollmentFlowRequest(factories_t *pxFactories, ipcpFactoryType_t xFactoryTypeFrom,  ipcpFactoryType_t xFactoryTypeTo, pidm_t * pxPidm);

BaseType_t xIpcpManagerShimAllocateResponseHandle(factories_t *pxFactories, ipcpFactoryType_t xShimType);
BaseType_t xIpcpManagerPreBindFlow(factories_t *pxFactories, ipcpFactoryType_t xNormalType);


BaseType_t xIpcpManagerAppFlowAllocateRequestHandle(pidm_t * pxPidm, void * data);



#endif
