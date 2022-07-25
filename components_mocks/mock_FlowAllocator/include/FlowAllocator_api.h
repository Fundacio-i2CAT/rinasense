#ifndef _COMPONENTS_MOCKS_FLOWALLOCATOR_H
#define _COMPONENTS_MOCKS_FLOWALLOCATOR_H

#include "RINA_API_flows.h"
#include "Ribd.h"

void vFlowAllocatorFlowRequest(struct efcpContainer_t *pxEfcpc,
                               portId_t xPortId,
                               flowAllocateHandle_t *pxFlowRequest,
                               struct ipcpNormalData_t *pxIpcpData);

bool_t xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue, int result);


#endif // _COMPONENTS_MOCKS_FLOWALLOCATOR_H
