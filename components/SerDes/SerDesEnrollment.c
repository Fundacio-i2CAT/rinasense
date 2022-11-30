#include "portability/port.h"
#include "common/rsrc.h"
#include "common/macros.h"

#include "pb.h"
#include "EnrollmentInformationMessage.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "SerDes.h"
#include "SerDesEnrollment.h"

#define TAG_SD_ENROLLMENT "[SD-ENROLLMENT]"

bool_t xSerDesEnrollmentInit(EnrollmentSerDes_t *pxSD)
{
    size_t unSz;

    unSz = ENROLLMENT_MSG_SIZE + sizeof(serObjectValue_t);
    if (!(pxSD->xPoolEnc = pxRsrcNewPool("Enrollment SerDes Encoding", unSz, 1, 1, 0)))
        return false;

    unSz = member_size(rina_messages_enrollmentInformation_t, token) + sizeof(enrollmentMessage_t);
    if (!(pxSD->xPoolDec = pxRsrcNewPool("Enrollment SerDes Decoding", unSz, 1, 1, 0)))
        return false;

    return true;
}

/**
 * @brief Encode a Enrollment message and return the serialized object value to be
 * added into the CDAP message
 *
 * @param pxMsg
 * @return serObjectValue_t*
 */
serObjectValue_t *pxSerDesEnrollmentEncode(EnrollmentSerDes_t *pxSD, enrollmentMessage_t *pxMsg)
{
    bool_t xStatus;
    serObjectValue_t *pxSerVal;
    pb_ostream_t xStream;
    rina_messages_enrollmentInformation_t enrollMsg = rina_messages_enrollmentInformation_t_init_zero;

    enrollMsg.has_startEarly = true;
    enrollMsg.startEarly = pxMsg->xStartEarly;

    if (pxMsg->ullAddress > 0) {
        enrollMsg.address = pxMsg->ullAddress;
        enrollMsg.has_address = true;
    }

    if (pxMsg->pcToken) {
        strncpy(enrollMsg.token, pxMsg->pcToken, sizeof(enrollMsg.token));
        enrollMsg.has_token = true;
    }

    /* FIXME: Not handling supporting DIFs for now. */

    if (!(pxSerVal = pxRsrcAlloc(pxSD->xPoolEnc, "Enrollment Encoding"))) {
        LOGE(TAG_SD_ENROLLMENT, "Failed to allocate memory for enrollment message encoding");
        return NULL;
    }

    pxSerVal->pvSerBuffer = pxSerVal + sizeof(serObjectValue_t);

    /* Create a stream that writes to our buffer. */
    xStream = pb_ostream_from_buffer(pxSerVal->pvSerBuffer, ENROLLMENT_MSG_SIZE);

    /* Now we are ready to encode the message. */
    xStatus = pb_encode(&xStream, rina_messages_enrollmentInformation_t_fields, &enrollMsg);

    /* Check for errors... */
    if (!xStatus) {
        LOGE(TAG_SD_ENROLLMENT, "Encoding failed: %s\n", PB_GET_ERROR(&xStream));
        vRsrcFree(pxSerVal);
        return NULL;
    }

    pxSerVal->xSerLength = xStream.bytes_written;

    return pxSerVal;
}

enrollmentMessage_t *pxSerDesEnrollmentDecode(EnrollmentSerDes_t *pxSD,
                                              uint8_t *pucBuffer, size_t xMessageLength)
{
    bool_t xStatus;
    pb_istream_t xStream;
    enrollmentMessage_t *pxMsg;
    rina_messages_enrollmentInformation_t xMessage = rina_messages_enrollmentInformation_t_init_zero;

    /*Create a stream that will read from the buffer*/
    xStream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    xStatus = pb_decode(&xStream, rina_messages_enrollmentInformation_t_fields, &xMessage);

    if (!xStatus) {
        LOGE(TAG_SD_ENROLLMENT, "Decoding failed: %s", PB_GET_ERROR(&xStream));
        return NULL;
    }

    if (!(pxMsg = pxRsrcAlloc(pxSD->xPoolDec, "Enrollment Decoding"))) {
        LOGE(TAG_SD_ENROLLMENT, "Failed to allocate memory for enrollment message decoding");
        return NULL;
    }

    pxMsg->pcToken = (void *)pxMsg + sizeof(enrollmentMessage_t);

    if (xMessage.has_address)
        pxMsg->ullAddress = xMessage.address;

    if (xMessage.has_token)
        strcpy(pxMsg->pcToken, xMessage.token);

    return pxMsg;
}
