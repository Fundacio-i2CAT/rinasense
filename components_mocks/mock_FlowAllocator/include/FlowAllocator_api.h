#ifndef _COMPONENTS_MOCKS_FLOWALLOCATOR_H
#define _COMPONENTS_MOCKS_FLOWALLOCATOR_H

#include "portability/port.h"
#include "RINA_API_flows.h"
#include "Ribd.h"

#ifdef ESP_PLATFORM

#define vFlowAllocatorFlowRequest   mock_FlowAllocator_vFlowAllocatorFlowRequest
#define xFlowAllocatorHandleCreateR mock_FlowAllocator_xFlowAllocatorHandleCreateR
#define pxFAFindFlowHandle          mock_FlowAllocator_pxFAFindFlowHandle
#define xFlowAllocatorDuPost        mock_FlowAllocator_xFlowAllocatorDuPost
#define xFlowAllocatorHandleDelete  mock_FlowAllocator_xFlowAllocatorHandleDelete

void mock_FlowAllocator_vFlowAllocatorFlowRequest(portId_t xAppPortId,
                                                  flowAllocateHandle_t *pxFlowRequest);

bool_t mock_FlowAllocator_xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue,
                                                      int result);

flowAllocateHandle_t *mock_FlowAllocator_pxFAFindFlowHandle(portId_t xPortId);

bool_t mock_FlowAllocator_xFlowAllocatorDuPost(portId_t xAppPortId, struct du_t *pxDu);

bool_t mock_FlowAllocator_xFlowAllocatorHandleDelete(struct ribObject_t *pxRibObject,
                                                     int invoke_id);

#else

void vFlowAllocatorFlowRequest(portId_t xPortId,
                               flowAllocateHandle_t *pxFlowRequest);

bool_t xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue, int result);

flowAllocateHandle_t *pxFAFindFlowHandle(portId_t xPortId);

bool_t xFlowAllocatorDuPost(portId_t xAppPortId, struct du_t *pxDu);

bool_t xFlowAllocatorHandleDelete(struct ribObject_t *pxRibObject, int invoke_id);

#endif

#endif // _COMPONENTS_MOCKS_FLOWALLOCATOR_H
