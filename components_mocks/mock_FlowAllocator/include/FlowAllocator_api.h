#ifndef _COMPONENTS_MOCKS_FLOWALLOCATOR_H
#define _COMPONENTS_MOCKS_FLOWALLOCATOR_H

#include "portability/port.h"
#include "RINA_API_flows.h"
#include "Ribd.h"

#ifdef ESP_PLATFORM

#define vFlowAllocatorFlowRequest   mock_FlowAllocator_vFlowAllocatorFlowRequest
#define xFlowAllocatorHandleCreateR mock_FlowAllocator_xFlowAllocatorHandleCreateR

void mock_FlowAllocator_vFlowAllocatorFlowRequest(struct efcpContainer_t *pxEfcpc,
                                                  portId_t xPortId,
                                                  flowAllocateHandle_t *pxFlowRequest,
                                                  struct ipcpNormalData_t *pxIpcpData);

bool_t mock_FlowAllocator_xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue,
                                                      int result);

#else

void vFlowAllocatorFlowRequest(struct efcpContainer_t *pxEfcpc,
                               portId_t xPortId,
                               flowAllocateHandle_t *pxFlowRequest,
                               struct ipcpNormalData_t *pxIpcpData);

bool_t xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue, int result);

#endif

#endif // _COMPONENTS_MOCKS_FLOWALLOCATOR_H
