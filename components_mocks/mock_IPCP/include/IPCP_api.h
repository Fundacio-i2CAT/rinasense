#ifndef _mock_IPCP_API_H
#define _mock_IPCP_API_H

/*
 * This is a partial public interface for the IPCP component to allow
 * the IPCP to be mocked in unit tests.
 */
#include <unistd.h>

#include "portability/port.h"
#include "IPCP_frames.h"
#include "IPCP_events.h"

/* PUBLIC API */

#ifdef ESP_PLATFORM

#define xIsCallingFromIPCPTask      mock_IPCP_xIsCallingFromIPCPTask
#define xSendEventToIPCPTask        mock_IPCP_xSendEventToIPCPTask
#define xSendEventStructToIPCPTask  mock_IPCP_xSendEventStructToIPCPTask
#define pxIPCPGetRmt                mock_IPCP_pxIPCPGetRmt
#define eConsiderFrameForProcessing mock_IPCP_eConsiderFrameForProcessing

bool_t mock_IPCP_xIsCallingFromIPCPTask(void);

bool_t mock_IPCP_xSendEventToIPCPTask(eRINAEvent_t eEvent);

bool_t mock_IPCP_xSendEventStructToIPCPTask(const RINAStackEvent_t * pxEvent,
                                            useconds_t uxTimeoutUS);

struct rmt_t *mock_IPCP_pxIPCPGetRmt(void);

eFrameProcessingResult_t mock_IPCP_eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer);

#else

bool_t xIsCallingFromIPCPTask(void);

bool_t xSendEventToIPCPTask(eRINAEvent_t eEvent);

bool_t xSendEventStructToIPCPTask(const RINAStackEvent_t * pxEvent,
                                  useconds_t uxTimeoutUS);

struct rmt_t *pxIPCPGetRmt(void);

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer);

#endif

#endif // _mock_IPCP_API_H
