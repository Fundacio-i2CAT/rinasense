#include "RINA_API_flows.h"
#include "rina_ids.h"
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

