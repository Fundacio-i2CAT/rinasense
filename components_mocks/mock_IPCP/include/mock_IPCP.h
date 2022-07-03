#ifndef _mock_IPCP_H
#define _mock_IPCP_H

/* Mock utilities */

RINAStackEvent_t *pxMockGetLastSentEvent();

void vMockClearLastSentEvent();

void vMockSetIsCallingFromIPCPTask(bool_t v);

bool_t xMockIPCPInit();

void vMockIPCPClean();

#endif // _mock_IPCP_H
