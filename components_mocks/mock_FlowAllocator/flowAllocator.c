#include "common/rina_ids.h"

#include "RINA_API_flows.h"
#include "EFCP.h"
#include "Ribd.h"

#ifdef ESP_PLATFORM
void mock_FlowAllocator_vFlowAllocatorFlowRequest(struct efcpContainer_t *pxEfcpc,
                                                  portId_t xPortId,
                                                  flowAllocateHandle_t *pxFlowRequest,
                                                  struct ipcpNormalData_t *pxIpcpData)
#else
void vFlowAllocatorFlowRequest(struct efcpContainer_t *pxEfcpc,
                               portId_t xPortId,
                               flowAllocateHandle_t *pxFlowRequest,
                               struct ipcpNormalData_t *pxIpcpData)
#endif
{
}

#ifdef ESP_PLATFORM
bool_t mock_FlowAllocator_xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue,
                                                      int result)
#else
bool_t xFlowAllocatorHandleCreateR(serObjectValue_t *pxSerObjValue, int result)
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
bool_t xFlowAllocatorHandleDelete(struct ribObject_t *pxRibObject, int invoke_id)
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
