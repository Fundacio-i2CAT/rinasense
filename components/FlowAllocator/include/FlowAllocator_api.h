#ifndef FLOW_ALLOCATOR_API_H_INCLUDED
#define FLOW_ALLOCATOR_API_H_INCLUDED

#include "common/rina_ids.h"

#include "RibObject.h"
#include "SerDesFlow.h"
#include "FlowAllocator_obj.h"
#include "Ribd_api.h"
#include "Enrollment_obj.h" 

#ifdef __cplusplus
extern "C" {
#endif

#define TAG_FA "[FLOW_ALLOCATOR]"

/* New reviewed API */

bool_t xFlowAllocatorInit(flowAllocator_t *pxFA, Enrollment_t *pxEnrollment, Ribd_t *pxRibd);

/* Handlers */

rsErr_t xFlowAllocatorHandleDelete(ribObject_t *pxThis,
                                   appConnection_t *pxAppCon,
                                   messageCdap_t *pxMsg);

bool_t xFlowAllocatorHandleCreateR(struct ipcpInstanceData_t *pxData,
                                   serObjectValue_t *pxSerObjValue,
                                   int result);

bool_t xFlowAllocatorHandleDeleteR(struct ipcpInstanceData_t *pxData,
                                   ribObject_t *pxRibObject,
                                   int invoke_id);

/* Unreviewed API. */

void vFlowAllocatorFlowRequest(flowAllocator_t *pxFA,
                               portId_t xAppPortId,
                               flowAllocateHandle_t *pxFlowRequest);

void vFlowAllocatorDeallocate(portId_t xAppPortId);

flowAllocateHandle_t *pxFAFindFlowHandle(flowAllocator_t *pxFA,
                                         portId_t xPortId);

bool_t xFlowAllocatorDuPost(flowAllocator_t *pxFA,
                            portId_t xAppPortId,
                            du_t *pxDu);

flowAllocator_t *pxFlowAllocatorCreate(struct ipcpInstance_t *pxNormalIpcp);


#ifdef __cplusplus
}
#endif

#endif // FLOW_ALLOCATOR_H_INCLUDED
