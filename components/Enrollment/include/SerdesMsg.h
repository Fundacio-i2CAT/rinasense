#ifndef SERDES_MSG_H_INCLUDED
#define SERDES_MSG_H_INCLUDED


serObjectValue_t *pxSerdesMsgEnrollmentEncode(enrollmentMessage_t *pxMsg);
enrollmentMessage_t *pxSerdesMsgEnrollmentDecode(uint8_t *pucBuffer, size_t xMessageLength);

serObjectValue_t *pxSerdesMsgNeighborEncode(neighborMessage_t *pxMsg);
neighborMessage_t *pxserdesMsgDecodeNeighbor(uint8_t *pucBuffer, size_t xMessageLength);

serObjectValue_t *pxSerdesMsgFlowEncode(flow_t *pxFlow);


#endif /* SERDES_MSG_H_ */