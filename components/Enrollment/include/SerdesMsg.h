#ifndef SERDES_MSG_H_INCLUDED
#define SERDES_MSG_H_INCLUDED

#include "FlowAllocator.h"

#ifdef __cplusplus
extern "C" {
#endif

serObjectValue_t *pxSerdesMsgEnrollmentEncode(enrollmentMessage_t *pxMsg);
enrollmentMessage_t *pxSerdesMsgEnrollmentDecode(uint8_t *pucBuffer, size_t xMessageLength);

serObjectValue_t *pxSerdesMsgNeighborEncode(neighborMessage_t *pxMsg);
neighborMessage_t *pxserdesMsgDecodeNeighbor(uint8_t *pucBuffer, size_t xMessageLength);

serObjectValue_t *pxSerdesMsgFlowEncode(flow_t *pxFlow);

aDataMsg_t *pxSerdesMsgDecodeAData(uint8_t *pucBuffer, size_t xMessageLength);
flow_t *pxSerdesMsgDecodeFlow(uint8_t *pucBuffer, size_t xMessageLength);

#ifdef __cplusplus
}
#endif

#endif /* SERDES_MSG_H_ */
