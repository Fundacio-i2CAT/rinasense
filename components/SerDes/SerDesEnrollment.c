#include <pthread.h>

#include "common/rinasense_errors.h"
#include "portability/port.h"
#include "common/error.h"
#include "common/rsrc.h"
#include "common/macros.h"

#include "pb.h"
#include "EnrollmentInformationMessage.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "SerDes.h"
#include "SerDesEnrollment.h"

#define TAG_SD_ENROLLMENT "[SD-ENROLLMENT]"

rsMemErr_t xSerDesEnrollmentInit(EnrollmentSerDes_t *pxSD)
{
    size_t unSz;
    int n;

    unSz = ENROLLMENT_MSG_SIZE + sizeof(serObjectValue_t);
    if (!(pxSD->xPoolEnc = pxRsrcNewPool("Enrollment SerDes Encoding", unSz, 1, 1, 0)))
        return ERR_SET_OOM;

    unSz = member_size(rina_messages_enrollmentInformation_t, token) + sizeof(enrollmentMessage_t);
    if (!(pxSD->xPoolDec = pxRsrcNewPool("Enrollment SerDes Decoding", unSz, 1, 1, 0)))
        return ERR_SET_OOM;

    if ((n = pthread_mutex_init(&pxSD->xPoolDecMutex, NULL) != 0))
        return ERR_SET_PTHREAD(n);

    if ((n = pthread_mutex_init(&pxSD->xPoolEncMutex, NULL) != 0))
        return ERR_SET_PTHREAD(n);

    vRsrcSetMutex(pxSD->xPoolEnc, &pxSD->xPoolEncMutex);
    vRsrcSetMutex(pxSD->xPoolDec, &pxSD->xPoolDecMutex);

    return SUCCESS;
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

    if (!(pxSerVal = pxRsrcAlloc(pxSD->xPoolEnc, "Enrollment Encoding")))
        return ERR_SET_OOM_NULL;

    pxSerVal->pvSerBuffer = pxSerVal + sizeof(serObjectValue_t);

    /* Create a stream that writes to our buffer. */
    xStream = pb_ostream_from_buffer(pxSerVal->pvSerBuffer, ENROLLMENT_MSG_SIZE);

    /* Now we are ready to encode the message. */
    xStatus = pb_encode(&xStream, rina_messages_enrollmentInformation_t_fields, &enrollMsg);

    /* Check for errors... */
    if (!xStatus) {
        vRsrcFree(pxSerVal);
        return ERR_SET_MSGF_NULL(ERR_SERDES_ENCODING_FAIL, "Encoding failed: %s", PB_GET_ERROR(&xStream));
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

    if (!xStatus)
        return ERR_SET_MSGF_NULL(ERR_SERDES_DECODING_FAIL, "Decoding failed: %s", PB_GET_ERROR(&xStream));

    if (!(pxMsg = pxRsrcAlloc(pxSD->xPoolDec, "Enrollment Decoding")))
        return ERR_SET_OOM_NULL;

    pxMsg->pcToken = (void *)pxMsg + sizeof(enrollmentMessage_t);

    if (xMessage.has_address)
        pxMsg->ullAddress = xMessage.address;

    if (xMessage.has_token)
        strcpy(pxMsg->pcToken, xMessage.token);

    return pxMsg;
}
