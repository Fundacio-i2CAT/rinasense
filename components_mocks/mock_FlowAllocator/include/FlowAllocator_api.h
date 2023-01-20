#ifndef _COMPONENTS_MOCKS_FLOWALLOCATOR_H
#define _COMPONENTS_MOCKS_FLOWALLOCATOR_H

#include "FlowAllocator_obj.h"
#include "portability/port.h"

#include "RINA_API_flows.h"
#include "Ribd_api.h"
#include "FlowAllocator_defs.h"

#ifdef ESP_PLATFORM

#define vFlowAllocatorFlowRequest   mock_FlowAllocator_vFlowAllocatorFlowRequest
#define xFlowAllocatorHandleCreateR mock_FlowAllocator_xFlowAllocatorHandleCreateR
#define pxFAFindFlowHandle          mock_FlowAllocator_pxFAFindFlowHandle
#define xFlowAllocatorDuPost        mock_FlowAllocator_xFlowAllocatorDuPost
#define xFlowAllocatorHandleDelete  mock_FlowAllocator_xFlowAllocatorHandleDelete

void mock_FlowAllocator_vFlowAllocatorFlowRequest(flowAllocator_t *pxFA,
                                                  portId_t xAppPortId,
                                                  flowAllocateHandle_t *pxFlowRequest);

flowAllocateHandle_t *mock_FlowAllocator_pxFAFindFlowHandle(portId_t xPortId);

bool_t mock_FlowAllocator_xFlowAllocatorDuPost(flowAllocator_t *pxFA,
                                               portId_t xAppPortId,
                                               du_t *pxDu);

bool_t mock_FlowAllocator_xFlowAllocatorHandleCreateR(struct ipcpInstanceData_t *pxData,
                                                      serObjectValue_t *pxSerObjValue,
                                                      int result);

bool_t mock_FlowAllocator_xFlowAllocatorHandleDelete(ribObject_t *pxThis,
                                                     appConnection_t *pxAppCon,
                                                     messageCdap_t *pxMsg);

flowAllocator_t *mock_FlowAllocator_pxFlowAllocatorCreate(struct ipcpInstance_t *pxNormalIpcp);

#else

void vFlowAllocatorFlowRequest(flowAllocator_t *pxFA,
                               portId_t xPortId,
                               flowAllocateHandle_t *pxFlowRequest);

bool_t xFlowAllocatorHandleCreateR(struct ipcpInstanceData_t *pxData,
                                   serObjectValue_t *pxSerObjValue,
                                   int result);

flowAllocateHandle_t *pxFAFindFlowHandle(portId_t xPortId);

bool_t xFlowAllocatorDuPost(flowAllocator_t *pxFA,
                            portId_t xAppPortId,
                            du_t *pxDu);

bool_t xFlowAllocatorHandleDelete(ribObject_t *pxThis,
                                  appConnection_t *pxAppCon,
                                  messageCdap_t *pxMsg);

flowAllocator_t *pxFlowAllocatorCreate(struct ipcpInstance_t *pxNormalIpcp);

#endif

#endif // _COMPONENTS_MOCKS_FLOWALLOCATOR_H
