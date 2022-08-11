#ifndef _COMPONENTS_IPCP_API_H
#define _COMPONENTS_IPCP_API_H

#include <unistd.h>

#include "portability/port.h"

#include "efcpStructures.h"
#include "IPCP_frames.h"
#include "IPCP_events.h"
#include "rina_ids.h"

/*
 * Send the event eEvent to the IPCP task event queue, using a block time of
 * zero.  Return pdPASS if the message was sent successfully, otherwise return
 * pdFALSE.
 */
bool_t xSendEventToIPCPTask(eRINAEvent_t eEvent);

/* Returns true is this function is called from the IPCP-task */
bool_t xIsCallingFromIPCPTask(void);

bool_t xSendEventStructToIPCPTask(const RINAStackEvent_t *pxEvent,
                                  useconds_t uxTimeoutUS);

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer);

bool_t RINA_IPCPInit(void);

struct rmt_t *pxIPCPGetRmt(void);
struct efcpContainer_t *pxIPCPGetEfcpc(void);

portId_t xIPCPAllocatePortId(void);

#endif // _COMPONENTS_IPCP_API_H
