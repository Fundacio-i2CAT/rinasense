#ifndef _mock_IPCP_IPCP_H
#define _mock_IPCP_IPCP_H

/*
 * This is a partial public interface for the IPCP component to allow
 * the IPCP to be mocked in unit tests.
 */

#include "portability/port.h"
#include "IPCP_events.h"

/* PUBLIC API */

bool_t xIsCallingFromIPCPTask(void);

bool_t xSendEventToIPCPTask(eRINAEvent_t eEvent);

bool_t xSendEventStructToIPCPTask(const RINAStackEvent_t * pxEvent,
                                  struct timespec *uxTimeout);

/* Mock utilities */

RINAStackEvent_t *pxMockGetLastSentEvent();

void vMockClearLastSentEvent();

void vMockSetIsCallingFromIPCPTask(bool_t v);

bool_t xMockIPCPInit();
void vMockIPCPClean();

#endif // _mock_IPCP_IPCP_H
