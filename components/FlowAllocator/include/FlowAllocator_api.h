#ifndef FLOW_ALLOCATOR_API_H_INCLUDED
#define FLOW_ALLOCATOR_API_H_INCLUDED

#include "Rib.h"
#include "FlowAllocator.h"

#ifdef __cplusplus
extern "C" {
#endif

/* New reviewed API */

bool_t xFlowAllocatorInit(flowAllocator_t *pxFA);

void vFlowAllocatorFini(flowAllocator_t *pxFA);

/* Unreviewed API. */

void vFlowAllocatorFlowRequest(flowAllocator_t *pxFA,
                               portId_t xAppPortId,
                               flowAllocateHandle_t *pxFlowRequest);

bool_t xFlowAllocatorHandleCreateR(flowAllocator_t *pxFA,
                                   serObjectValue_t *pxSerObjValue,
                                   int result);

void vFlowAllocatorDeallocate(portId_t xAppPortId);

bool_t xFlowAllocatorHandleDelete(struct ribObject_t *pxRibObject, int invoke_id);

bool_t xFlowAllocatorHandleDeleteR(flowAllocator_t *pxFA,
                                   struct ribObject_t *pxRibObject,
                                   int invoke_id);

flowAllocateHandle_t *pxFAFindFlowHandle(flowAllocator_t *pxFA,
                                         portId_t xPortId);

bool_t xFlowAllocatorDuPost(flowAllocator_t *pxFA,
                            portId_t xAppPortId,
                            struct du_t *pxDu);

flowAllocator_t *pxFlowAllocatorCreate(struct ipcpInstance_t *pxNormalIpcp);

#ifdef __cplusplus
}
#endif

#endif // FLOW_ALLOCATOR_H_INCLUDED
