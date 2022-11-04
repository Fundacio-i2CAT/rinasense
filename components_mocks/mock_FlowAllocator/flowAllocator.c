#include "common/rina_ids.h"

#include "RINA_API_flows.h"
#include "EFCP.h"
#include "RibObject.h"

#ifdef ESP_PLATFORM
void mock_FlowAllocator_vFlowAllocatorFlowRequest(flowAllocator_t *pxFA,
                                                  portId_t xPortId,
                                                  flowAllocateHandle_t *pxFlowRequest,
                                                  struct ipcpInstanceData_t *pxIpcpData)
#else
void vFlowAllocatorFlowRequest(flowAllocator_t *pxFA,
                               portId_t xPortId,
                               flowAllocateHandle_t *pxFlowRequest)
#endif
{
}

#ifdef ESP_PLATFORM
bool_t mock_FlowAllocator_xFlowAllocatorHandleCreateR(struct ipcpInstanceData_t *pxData,
                                                      serObjectValue_t *pxSerObjValue,
                                                      Uint result)
#else
bool_t xFlowAllocatorHandleCreateR(struct ipcpInstanceData_t *pxData,
                                   serObjectValue_t *pxSerObjValue, int result)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_FlowAllocator_xFlowAllocatorDuPost(portId_t xAppPortId, struct du_t *pxDu)
#else
bool_t xFlowAllocatorDuPost(portId_t xAppPortId, struct du_t *pxDu)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_FlowAllocator_xFlowAllocatorHandleDelete(struct ribObject_t *pxRibObject, int invoke_id)
#else
bool_t xFlowAllocatorHandleDelete(ribObject_t *pxRibObject, int invoke_id)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
flowAllocateHandle_t *mock_FlowAllocator_pxFAFindFlowHandle(portId_t xPortId)
#else
flowAllocateHandle_t *pxFAFindFlowHandle(portId_t xPortId)
#endif
{
    return NULL;
}
