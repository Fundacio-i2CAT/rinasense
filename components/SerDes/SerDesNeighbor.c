#include "common/rsrc.h"
#include "common/macros.h"

#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "SerDes.h"
#include "SerDesEnrollment.h"
#include "SerDesNeighbor.h"
#include "NeighborMessage.pb.h"

#define TAG_SD_NEIGHBOR "[SD-NEIGHBOR]"

#if 0
/* Currently not in use. */
static rina_messages_neighbor_t prvSerdesMsgEncodeNeighbor(neighborMessage_t *pxMessage)
{
    rina_messages_neighbor_t message = rina_messages_neighbor_t_init_zero;

    if (pxMessage->pcAeInstance)
    {
        strcpy(message.aeinstance, pxMessage->pcAeInstance);
        message.has_aeinstance = true;
    }
    if (pxMessage->pcAeName)
    {
        strcpy(message.aename, pxMessage->pcAeName);
        message.has_aename = true;
    }
    if (pxMessage->pcApInstance)
    {
        strcpy(message.apinstance, pxMessage->pcApInstance);
        message.has_apinstance = true;
    }
    if (pxMessage->pcApName)
    {
        strcpy(message.apname, pxMessage->pcApName);
        message.has_apname = true;
    }

    return message;
}
#endif

bool_t xSerDesNeighborInit(NeighborSerDes_t *pxSD)
{
    size_t unSz;

    unSz = ENROLLMENT_MSG_SIZE + sizeof(serObjectValue_t);
    if (!(pxSD->xEncPool = pxRsrcNewPool("Neighbor SerDes Encoding", unSz, 1, 1, 0)))
        return false;

    unSz = member_size(rina_messages_neighbor_t, apname)
        + member_size(rina_messages_neighbor_t, apinstance)
        + member_size(rina_messages_neighbor_t, aename)
        + member_size(rina_messages_neighbor_t, aeinstance)
        + sizeof(neighborMessage_t);
    if (!(pxSD->xDecPool = pxRsrcNewPool("Neighbor SerDes Decoding", unSz, 1, 1, 0)))
        return false;

    return true;
}

neighborMessage_t *pxSerDesNeighborDecode(NeighborSerDes_t *pxSD, uint8_t *pucBuffer, size_t xMessageLength)
{
    bool_t xStatus;
    pb_istream_t xStream;
    neighborMessage_t *pxMsg;
    rina_messages_neighbor_t message = rina_messages_neighbor_t_init_zero;

    /*Create a stream that will read from the buffer*/
    xStream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    xStatus = pb_decode(&xStream, rina_messages_neighbor_t_fields, &message);

    if (!xStatus) {
        LOGE(TAG_SD_NEIGHBOR, "Decoding failed: %s", PB_GET_ERROR(&xStream));
        return NULL;
    }

    if (!(pxMsg = pxRsrcAlloc(pxSD->xDecPool, "Neighbor Decoding"))) {
        LOGE(TAG_SD_NEIGHBOR, "Failed to allocate memory for neighbor message decoding");
        return NULL;
    }

    /* Initialize the string pointers to point inside the memory block
       after the structure. */
    pxMsg->pcApName = (void *)pxMsg + sizeof(neighborMessage_t);
    pxMsg->pcApInstance = pxMsg->pcApName + member_size(rina_messages_neighbor_t, apname);
    pxMsg->pcAeName = pxMsg->pcApInstance + member_size(rina_messages_neighbor_t, apinstance);
    pxMsg->pcAeInstance = pxMsg->pcAeName + member_size(rina_messages_neighbor_t, aename);

    if (message.has_apname)
        strcpy(pxMsg->pcApName, message.apname);

    if (message.has_apinstance)
        strcpy(pxMsg->pcApInstance, message.apinstance);

    if (message.has_aename)
        strcpy(pxMsg->pcAeName, message.aename);

    if (message.has_aeinstance)
        strcpy(pxMsg->pcAeInstance, message.aeinstance);

    if (message.has_address)
        pxMsg->ullAddress = message.address;

    return pxMsg;
}

serObjectValue_t *pxSerDesNeighborEncode(NeighborSerDes_t *pxSD, neighborMessage_t *pxMessage)
{
    bool_t xStatus;
    serObjectValue_t *pxSer;
    pb_ostream_t xStream;
    rina_messages_neighbor_t message = rina_messages_neighbor_t_init_zero;

    if (pxMessage->pcAeInstance) {
        strcpy(message.aeinstance, pxMessage->pcAeInstance);
        message.has_aeinstance = true;
    }
    if (pxMessage->pcAeName) {
        strcpy(message.aename, pxMessage->pcAeName);
        message.has_aename = true;
    }
    if (pxMessage->pcApInstance) {
        strcpy(message.apinstance, pxMessage->pcApInstance);
        message.has_apinstance = true;
    }
    if (pxMessage->pcApName) {
        strcpy(message.apname, pxMessage->pcApName);
        message.has_apname = true;
    }

    // Allocate space on the stack to store the message data.
    if (!(pxSer = pxRsrcAlloc(pxSD->xEncPool, "Neighbor Encoding"))) {
        LOGE(TAG_SD_NEIGHBOR, "Failed to allocate memory for neighbor message encoding");
        return NULL;
    }

    pxSer->pvSerBuffer = pxSer + sizeof(serObjectValue_t);

    // Create a stream that writes to our buffer.
    xStream = pb_ostream_from_buffer(pxSer->pvSerBuffer, ENROLLMENT_MSG_SIZE);

    // Now we are ready to encode the message.
    xStatus = pb_encode(&xStream, rina_messages_neighbor_t_fields, &message);

    // Check for errors...
    if (!xStatus) {
        LOGE(TAG_SD_NEIGHBOR, "Encoding failed: %s\n", PB_GET_ERROR(&xStream));
        return NULL;
    }

    pxSer->xSerLength = xStream.bytes_written;

    return pxSer;
}
