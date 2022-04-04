#ifndef SERDES_MSG_H_INCLUDED
#define SERDES_MSG_H_INCLUDED

neighborMessage_t *pxserdesMsgDecodeNeighbor(uint8_t *pucBuffer, size_t xMessageLength);
serObjectValue_t *pxSerdesMsgEnrollmentEncode(enrollmentMessage_t *pxMsg);
enrollmentMessage_t *pxSerdesMsgEnrollmentDecode(uint8_t *pucBuffer, size_t xMessageLength);

#endif /* SERDES_MSG_H_ */