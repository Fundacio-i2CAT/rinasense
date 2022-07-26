#ifndef FLOW_ALLOCATOR_API_H_INCLUDED
#define FLOW_ALLOCATOR_API_H_INCLUDED

#include "FlowAllocator.h"

void vFlowAllocatorFlowRequest(struct efcpContainer_t *pxEfcpc,
                               portId_t xPortId,
                               flowAllocateHandle_t *pxFlowRequest,
                               struct ipcpNormalData_t *pxIpcpData);

bool_t xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue, int result);

#endif // FLOW_ALLOCATOR_H_INCLUDED
