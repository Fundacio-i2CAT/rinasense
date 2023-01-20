#ifndef _COMPONENTS_IPCP_API_H
#define _COMPONENTS_IPCP_API_H

#include <unistd.h>

#include "portability/port.h"
#include "common/rina_ids.h"

#include "efcpStructures.h"
#include "IPCP_frames.h"
#include "IPCP_events.h"

#ifdef __cplusplus
extern "C" {
#endif

rsErr_t xSendEventToIPCPTask(eRINAEvent_t eEvent);

/* Returns true is this function is called from the IPCP-task */
bool_t xIsCallingFromIPCPTask(void);

rsErr_t xSendEventStructToIPCPTask(const RINAStackEvent_t *pxEvent,
                                   useconds_t uxTimeoutUS);

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer);

rsErr_t RINA_IPCPInit(void);

#ifdef __cplusplus
}
#endif

#endif // _COMPONENTS_IPCP_API_H
