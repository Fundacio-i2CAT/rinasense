#ifndef _COMPONENTS_MOCKS_FLOWALLOCATOR_H
#define _COMPONENTS_MOCKS_FLOWALLOCATOR_H

void vFlowAllocatorFlowRequest(struct efcpContainer_t *pxEfcpc,
                               portId_t xPortId,
                               flowAllocateHandle_t *pxFlowRequest,
                               struct ipcpNormalData_t *pxIpcpData);

#endif // _COMPONENTS_MOCKS_FLOWALLOCATOR_H
