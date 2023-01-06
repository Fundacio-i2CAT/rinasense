#include "common/rina_ids.h"

#include "RINA_API_flows.h"
#include "EFCP.h"
#include "RibObject.h"

#ifdef ESP_PLATFORM
void mock_FlowAllocator_vFlowAllocatorFlowRequest(flowAllocator_t *pxFA,
                                                  portId_t xPortId,
                                                  flowAllocateHandle_t *pxFlowRequest)
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
                                                      int result)
#else
bool_t xFlowAllocatorHandleCreateR(struct ipcpInstanceData_t *pxData,
                                   serObjectValue_t *pxSerObjValue, int result)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_FlowAllocator_xFlowAllocatorDuPost(flowAllocator_t *pxFA,
                                               portId_t xAppPortId,
                                               du_t *pxDu)
#else
bool_t xFlowAllocatorDuPost(flowAllocator_t *pxFA,
                            portId_t xAppPortId,
                            du_t *pxDu)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_FlowAllocator_xFlowAllocatorHandleDelete(ribObject_t *pxThis,
                                                     appConnection_t *pxAppCon,
                                                     messageCdap_t *pxMsg)
#else
bool_t xFlowAllocatorHandleDelete(ribObject_t *pxThis,
                                  appConnection_t *pxAppCon,
                                  messageCdap_t *pxMsg)
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
