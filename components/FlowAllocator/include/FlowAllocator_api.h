#ifndef FLOW_ALLOCATOR_API_H_INCLUDED
#define FLOW_ALLOCATOR_API_H_INCLUDED

#include "FlowAllocator.h"

void vFlowAllocatorFlowRequest(
    portId_t xAppPortId,
    flowAllocateHandle_t *pxFlowRequest);

bool_t xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue, int result);

void vFlowAllocatorDeallocate(portId_t xAppPortId);

bool_t xFlowAllocatorHandleDelete(struct ribObject_t *pxRibObject, int invoke_id);

flowAllocateHandle_t *pxFAFindFlowHandle(portId_t xPortId);

bool_t xFlowAllocatorDuPost(portId_t xAppPortId, struct du_t *pxDu);

#endif // FLOW_ALLOCATOR_H_INCLUDED
