#ifndef _mock_IPCP_API_H
#define _mock_IPCP_API_H

/*
 * This is a partial public interface for the IPCP component to allow
 * the IPCP to be mocked in unit tests.
 */

#include "portability/port.h"
#include "IPCP_frames.h"
#include "IPCP_events.h"

/* PUBLIC API */

bool_t xIsCallingFromIPCPTask(void);

bool_t xSendEventToIPCPTask(eRINAEvent_t eEvent);

bool_t xSendEventStructToIPCPTask(const RINAStackEvent_t * pxEvent,
                                  struct timespec *uxTimeout);

struct rmt_t *pxIPCPGetRmt(void);

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer);

#endif // _mock_IPCP_API_H
