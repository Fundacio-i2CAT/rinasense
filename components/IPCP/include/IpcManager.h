#ifndef IPC_MANAGER_H__INCLUDED
#define IPC_MANAGER_H__INCLUDED

#include "common/num_mgr.h"

#include "configRINA.h"
#include "IPCP_instance.h"
#include "EFCP.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xINSTANCE_TABLE_ROW
{
	/*The Ipcp Instance to register*/
	struct ipcpInstance_t *pxIpcpInstance;

	/*Is the Ipcp Instace active?*/
	bool_t xActive;

} ipcpTableRow_t;

typedef struct xIPC_MANAGER
{
    /* Table to store instances created */
    ipcpTableRow_t xIpcpTable[CFG_INSTANCES_IPCP_ENTRIES];

	/* port Id manager */
	NumMgr_t *pxPidm;

	/* IPCProcess Id manager */
	NumMgr_t *pxIpcpIdm;

} ipcManager_t;

#define DECLARE_IPC_MANAGER_LIFECYCLE_FUNCTION(x) \
    bool_t xIpcManagerRun##x();

extern ipcManager_t xIpcManager;

bool_t xIpcManagerInit(ipcManager_t *pxIpcManager);

void vIcpManagerEnrollmentFlowRequest(struct ipcpInstance_t *pxShimInstance, portId_t xN1PortId, rname_t *pxIPCPName);

void vIpcpManagerAppFlowAllocateRequestHandle(flowAllocateHandle_t *pxFlowAllocateRequest);

/* IPC ID Management */

ipcProcessId_t unIpcManagerReserveIpcpId(ipcManager_t *pxIpcManager);

void vIpcManagerReleaseIpcpId(ipcManager_t *pxIpcManager, ipcProcessId_t unIpcpId);

/* Port management */

portId_t unIpcManagerReservePort(ipcManager_t *pxIpcManager);

void vIpcManagerReleasePort(ipcManager_t *pxIpcManager, portId_t unPortId);

/* IPC list registration */

void vIpcManagerAdd(ipcManager_t *pxIpcManager, struct ipcpInstance_t *pxIpcp);

/* Lifecycle functions */

DECLARE_IPC_MANAGER_LIFECYCLE_FUNCTION(Start);

DECLARE_IPC_MANAGER_LIFECYCLE_FUNCTION(Stop);

DECLARE_IPC_MANAGER_LIFECYCLE_FUNCTION(Enable);

DECLARE_IPC_MANAGER_LIFECYCLE_FUNCTION(Disable);

/* Search */

struct ipcpInstance_t *pxIpcManagerFindById(ipcManager_t *pxIpcManager, ipcpInstanceId_t xIpcpId);

struct ipcpInstance_t *pxIpcManagerFindByType(ipcManager_t *pxIpcManager, ipcpInstanceType_t xType);

#if 0
void vIpcManagerRINAPackettHandler(struct ipcpInstanceData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer);
#endif

#ifdef __cplusplus
}
#endif

#endif
