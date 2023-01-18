#if 0

/*Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

/* RINA includes. */
#include "common/rina_name.h"
#include "portability/port.h"
#include "rina_common_port.h"
#include "configRINA.h"
#include "Ribd.h"
#include "Ribd_api.h"
#include "Enrollment.h"
#include "EnrollmentInformationMessage.pb.h"
#include "NeighborMessage.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Rib.h"
#include "FlowMessage.pb.h"
#include "FlowAllocator.h"

/**
 * @brief Encode a Enrollment message and return the serialized object value to be
 * added into the CDAP message
 *
 * @param pxMsg
 * @return serObjectValue_t*
 */
serObjectValue_t *pxSerdesMsgEnrollmentEncode(enrollmentMessage_t *pxMsg)
{
    bool_t status;
    rina_messages_enrollmentInformation_t enrollMsg = rina_messages_enrollmentInformation_t_init_zero;

    /*adding the address msg struct*/
    /****** For the moment just the address *****/
    if (pxMsg->ullAddress > 0) {
        enrollMsg.address = pxMsg->ullAddress;
        enrollMsg.has_address = true;
    }

    // Allocate space on the stack to store the message data.
    void *pvBuffer = pvRsMemAlloc(MTU);
    int maxLength = MTU;

    // Create a stream that writes to our buffer.
    pb_ostream_t stream = pb_ostream_from_buffer(pvBuffer, maxLength);

    // Now we are ready to encode the message.
    status = pb_encode(&stream, rina_messages_enrollmentInformation_t_fields, &enrollMsg);

    // Check for errors...
    if (!status)
    {
        LOGE(TAG_ENROLLMENT, "Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return NULL;
    }

    serObjectValue_t *pxSerMsg = pvRsMemAlloc(sizeof(*pxSerMsg));
    pxSerMsg->pvSerBuffer = pvBuffer;
    pxSerMsg->xSerLength = stream.bytes_written;

    return pxSerMsg;
}

static enrollmentMessage_t *prvSerdesMsgDecodeEnrollment(rina_messages_enrollmentInformation_t message)
{
    enrollmentMessage_t *pxMsg;
    pxMsg = pvRsMemAlloc(sizeof(*pxMsg));

    if (message.has_address)
        pxMsg->ullAddress = message.address;

    if (message.has_token)
        pxMsg->pcToken = strdup(message.token);

    return pxMsg;
}

enrollmentMessage_t *pxSerdesMsgEnrollmentDecode(uint8_t *pucBuffer, size_t xMessageLength)
{
    bool_t status;

    /*Allocate space for the decode message data*/
    rina_messages_enrollmentInformation_t message = rina_messages_enrollmentInformation_t_init_zero;

    /*Create a stream that will read from the buffer*/
    pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    status = pb_decode(&stream, rina_messages_enrollmentInformation_t_fields, &message);

    if (!status)
        {
        LOGE(TAG_RINA, "Decoding failed: %s", PB_GET_ERROR(&stream));
        return NULL;
    }

    return prvSerdesMsgDecodeEnrollment(message);
}

#endif
