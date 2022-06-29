#ifndef _COMPONENTS_IPCP_API_H
#define _COMPONENTS_IPCP_API_H

#include "IPCP_frames.h"
#include "IPCP_events.h"

/*
 * Send the event eEvent to the IPCP task event queue, using a block time of
 * zero.  Return pdPASS if the message was sent successfully, otherwise return
 * pdFALSE.
 */
BaseType_t xSendEventToIPCPTask(eRINAEvent_t eEvent);

/* Returns pdTRUE is this function is called from the IPCP-task */
BaseType_t xIsCallingFromIPCPTask(void);

BaseType_t xSendEventStructToIPCPTask(const RINAStackEvent_t *pxEvent,
                                      TickType_t uxTimeout);

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer);

BaseType_t RINA_IPCPInit(void);
struct rmt_t *pxIPCPGetRmt(void);

#endif // _COMPONENTS_IPCP_API_H
