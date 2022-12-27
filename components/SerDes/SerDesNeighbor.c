#include <stdio.h>

#include "common/arraylist.h"
#include "common/error.h"
#include "common/rsrc.h"
#include "common/macros.h"
#include "common/datapacker.h"

#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "SerDes.h"
#include "SerDesEnrollment.h"
#include "SerDesNeighbor.h"
#include "NeighborMessage.pb.h"
#include "NeighborArrayMessage.pb.h"

#define TAG_SD_NEIGHBOR "[SD-NEIGHBOR]"

typedef struct {
    NeighborSerDes_t *pxSD;

    neighborMessage_t *pxNeighbor;

} NeighborEncodeCbState_t;

typedef struct {
    NeighborSerDes_t *pxSD;

    arraylist_t xSupDifLst;

} NeighborDecodeCbState_t;

typedef struct {
    NeighborSerDes_t *pxSD;

    size_t unNeighCount;

    neighborMessage_t **pxNeighbors;
} NeighborListEncodeCbState_t;

typedef struct {
    NeighborSerDes_t *pxSD;

    /* List of neighbors we have decoded */
    arraylist_t xNbLst;

} NeighborListDecodeCbState_t;

rsErr_t xSerDesNeighborInit(NeighborSerDes_t *pxSD)
{
    size_t unSz;
    int n;

    unSz = ENROLLMENT_MSG_SIZE + sizeof(serObjectValue_t);

    if (!(pxSD->xEncPool = pxRsrcNewPool("Neighbor SerDes Encoding", unSz, 1, 1, 0)))
        return ERR_SET_OOM;

    if (!(pxSD->xDecPool = pxRsrcNewVarPool("Neighbor SerDes Decoding", 0)))
        return ERR_SET_OOM;

    if ((n = pthread_mutex_init(&pxSD->xDecPoolMutex, NULL)))
        return ERR_SET_PTHREAD(n);

    if ((n = pthread_mutex_init(&pxSD->xEncPoolMutex, NULL)))
        return ERR_SET_PTHREAD(n);

    vRsrcSetMutex(pxSD->xEncPool, &pxSD->xEncPoolMutex);
    vRsrcSetMutex(pxSD->xDecPool, &pxSD->xDecPoolMutex);

    return SUCCESS;
}

/* Return the amount of space required by the dynamic-sized items of a
 * neighbor message .*/
static size_t prvSerDesGetNeighborAllocSize(rina_messages_neighbor_t *message)
{
    size_t unSz;
    NeighborDecodeCbState_t *pxState;

    pxState = message->supportingDifs.arg;

    unSz = (!message->has_apname     ? 0 : strlen(message->apname) + 1)
        +  (!message->has_apinstance ? 0 : strlen(message->apinstance) + 1)
        +  (!message->has_aename     ? 0 : strlen(message->aename) + 1)
        +  (!message->has_aeinstance ? 0 : strlen(message->aeinstance) + 1);

    for (size_t n = 0; n < unArrayListCount(&pxState->xSupDifLst); n++)
        unSz += strlen(*(string_t *)pvArrayListGet(&pxState->xSupDifLst, n)) + 1;

    unSz += sizeof(string_t) * unArrayListCount(&pxState->xSupDifLst);

    return unSz;
}

static void prvSerDesRinaNeighborToNeighborMessage(neighborMessage_t *pxMsg,
                                                   rina_messages_neighbor_t *pxRinaMessage,
                                                   size_t unMaxSz)
{
    Pk_t pk;
    size_t unOffset;
    NeighborDecodeCbState_t *pxState;

    pxState = pxRinaMessage->supportingDifs.arg;

    if (pxRinaMessage->has_address)
        pxMsg->ullAddress = pxRinaMessage->address;

    unOffset = sizeof(neighborMessage_t) + unArrayListCount(&pxState->xSupDifLst) * sizeof(char *);
    vPkInit(&pk, (void *)pxMsg, unOffset, unMaxSz);

    if (pxRinaMessage->has_apname) {
        pxMsg->pcApName = pxPkPtr(&pk);
        vPkWriteStr(&pk, pxRinaMessage->apname);
    }
    else pxMsg->pcApName = NULL;

    if (pxRinaMessage->has_apinstance) {
        pxMsg->pcApInstance = pxPkPtr(&pk);
        vPkWriteStr(&pk, pxRinaMessage->apinstance);
    }
    else pxMsg->pcApInstance = NULL;

    if (pxRinaMessage->has_aename) {
        pxMsg->pcAeName = pxPkPtr(&pk);
        vPkWriteStr(&pk, pxRinaMessage->aename);
    }
    else pxMsg->pcAeName = NULL;

    if (pxRinaMessage->has_aeinstance) {
        pxMsg->pcAeInstance = pxPkPtr(&pk);
        vPkWriteStr(&pk, pxRinaMessage->aeinstance);
    }

    pxMsg->unSupportingDifCount = unArrayListCount(&pxState->xSupDifLst);

    for (size_t n = 0; n < pxMsg->unSupportingDifCount; n++) {
        pxMsg->pcSupportingDifs[n] = pxPkPtr(&pk);
        vPkWriteStr(&pk, *(string_t *)pvArrayListGet(&pxState->xSupDifLst, n));
    }
}

static bool_t prvSerDesNeighborEncodeCb(pb_ostream_t *stream,
                                        const pb_field_iter_t *field,
                                        void * const *arg)
{
    NeighborEncodeCbState_t *pxState;
    buffer_t pcSupDif;
    size_t unSupDifSz;

    pxState = (NeighborEncodeCbState_t *)*arg;

    for (size_t n = 0; n < pxState->pxNeighbor->unSupportingDifCount; n++) {
        if (!pb_encode_tag_for_field(stream, field))
            return false;

        pcSupDif = (buffer_t)pxState->pxNeighbor->pcSupportingDifs[n];
        unSupDifSz = strlen(pxState->pxNeighbor->pcSupportingDifs[n]);

        if (!pb_encode_string(stream, (pb_byte_t *)pcSupDif, unSupDifSz))
            return false;
    }

    return true;
}

static bool_t prvSerDesNeighborDecodeCb(pb_istream_t *stream, const pb_field_iter_t *field, void **arg)
{
    buffer_t pcSupDif;
    NeighborDecodeCbState_t *pxState;
    size_t unSupDifSz;

    pxState = (NeighborDecodeCbState_t *)*arg;

    while (stream->bytes_left) {
        unSupDifSz = stream->bytes_left;

        pcSupDif = pxRsrcVarAlloc(pxState->pxSD->xDecPool, "Supporting DIF", unSupDifSz + 1);
        if (!pcSupDif)
            return false;

        if (!pb_read(stream, pcSupDif, unSupDifSz))
            return false;

        pcSupDif[unSupDifSz] = '\0';

        if (ERR_CHK_MEM(xArrayListAdd(&pxState->xSupDifLst, &pcSupDif)))
            return false;
    }

    return true;
}

static void pxSerDesNeighborEncodePrepare(NeighborSerDes_t *pxSD,
                                          neighborMessage_t *pxMsg,
                                          rina_messages_neighbor_t *pxRinaMsg,
                                          NeighborEncodeCbState_t *pxState)
{
    pxState->pxNeighbor = pxMsg;
    pxState->pxSD = pxSD;

    pxRinaMsg->supportingDifs.funcs.encode = prvSerDesNeighborEncodeCb;
    pxRinaMsg->supportingDifs.arg = pxState;

    _SERDES_FIELD_COPY((*pxRinaMsg), aeinstance, pxMsg->pcAeInstance);
    _SERDES_FIELD_COPY((*pxRinaMsg), aename, pxMsg->pcAeName);
    _SERDES_FIELD_COPY((*pxRinaMsg), apinstance, pxMsg->pcApInstance);
    _SERDES_FIELD_COPY((*pxRinaMsg), apname, pxMsg->pcApName);
}

static bool_t prvSerDesNeighborListDecodeCb(pb_istream_t *stream,
                                          const pb_field_iter_t *field,
                                          void **arg)
{
    rina_messages_neighbor_t xNbMsg;
    neighborMessage_t *pxNbMsg;
    NeighborListDecodeCbState_t *pxListState;
    NeighborDecodeCbState_t xState;
    NeighborSerDes_t *pxSD;
    size_t unSz;
    bool_t xStatus = false;

    pxListState = (NeighborListDecodeCbState_t *)*arg;

    pxSD = pxListState->pxSD;
    xState.pxSD = pxSD;

    if (ERR_CHK_MEM(xArrayListInit(&xState.xSupDifLst, sizeof(char *), 10, pxSD->xDecPool)))
        return false;

    xNbMsg.supportingDifs.funcs.decode = prvSerDesNeighborDecodeCb;
    xNbMsg.supportingDifs.arg = &xState;

    while (stream->bytes_left) {
        pb_wire_type_t wire_type;
        uint32_t tag;
        bool_t eof;

        if (!pb_decode(stream, rina_messages_neighbor_t_fields, &xNbMsg))
            goto cleanup;

        unSz = sizeof(neighborMessage_t) + prvSerDesGetNeighborAllocSize(&xNbMsg);

        if (!(pxNbMsg = pxRsrcVarAlloc(pxSD->xDecPool, "Neighbor Decoding", unSz))) {
            LOGE(TAG_SD_NEIGHBOR, "Failed to allocate memory for neighbor message decoding");
            goto cleanup;
        }

        /* Convert the nanopb message to something palatable. */
        prvSerDesRinaNeighborToNeighborMessage(pxNbMsg, &xNbMsg, unSz);

        if (ERR_CHK_MEM(xArrayListAdd(&pxListState->xNbLst, &pxNbMsg)))
            goto cleanup;
    }

    xStatus = true;

    cleanup:
    for (size_t n = 0; n < unArrayListCount(&xState.xSupDifLst); n++)
        vRsrcFree(*(char **)pvArrayListGet(&xState.xSupDifLst, n));

    vArrayListFree(&xState.xSupDifLst);

    return xStatus;
}

static bool prvSerDesNeighborListEncodeCb(pb_ostream_t *stream,
                                          const pb_field_iter_t *field,
                                          void * const *arg)
{
    rina_messages_neighbor_t pxNbMsg = rina_messages_neighbor_t_init_zero;
    neighborMessage_t *pxCurNeighborMsg;
    NeighborListEncodeCbState_t *pxState;
    NeighborEncodeCbState_t xNbState;

    pxState = (NeighborListEncodeCbState_t *)*arg;

    for (size_t n = 0; n < pxState->unNeighCount; n++) {
        pxCurNeighborMsg = pxState->pxNeighbors[n];

        pxSerDesNeighborEncodePrepare(pxState->pxSD, pxCurNeighborMsg, &pxNbMsg, &xNbState);

        if (!pb_encode_tag_for_field(stream, field))
            return false;

        if (!pb_encode_submessage(stream, rina_messages_neighbor_t_fields, &pxNbMsg))
            return false;
    }

    return true;
}

neighborMessage_t *pxSerDesNeighborDecode(NeighborSerDes_t *pxSD, uint8_t *pucBuffer, size_t xMessageLength)
{
    bool_t xStatus;
    pb_istream_t xStream;
    neighborMessage_t *pxMsg, *pxMsgToRet = NULL;
    rina_messages_neighbor_t message = rina_messages_neighbor_t_init_zero;
    NeighborDecodeCbState_t xState;
    size_t unSz;

    xState.pxSD = pxSD;

    if (ERR_CHK_MEM(xArrayListInit(&xState.xSupDifLst, sizeof(buffer_t), 10, pxSD->xDecPool)))
        return NULL;

    message.supportingDifs.funcs.decode = prvSerDesNeighborDecodeCb;
    message.supportingDifs.arg = &xState;

    /*Create a stream that will read from the buffer*/
    xStream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    /* Decode the message */
    xStatus = pb_decode(&xStream, rina_messages_neighbor_t_fields, &message);

    if (!xStatus) {
        LOGE(TAG_SD_NEIGHBOR, "Decoding failed: %s", PB_GET_ERROR(&xStream));
        goto cleanup;
    }

    unSz = sizeof(neighborMessage_t) + prvSerDesGetNeighborAllocSize(&message);

    if (!(pxMsg = pxRsrcVarAlloc(pxSD->xDecPool, "Neighbor Decoding", unSz))) {
        LOGE(TAG_SD_NEIGHBOR, "Failed to allocate memory for neighbor message decoding");
        goto cleanup;
    }

    /* Convert the nanopb message to something palatable. */
    prvSerDesRinaNeighborToNeighborMessage(pxMsg, &message, unSz);
    pxMsgToRet = pxMsg;

    cleanup:
    for (size_t n = 0; n < unArrayListCount(&xState.xSupDifLst); n++)
        vRsrcFree(*(void **)pvArrayListGet(&xState.xSupDifLst, n));

    vArrayListFree(&xState.xSupDifLst);

    return pxMsgToRet;
}

serObjectValue_t *pxSerDesNeighborEncode(NeighborSerDes_t *pxSD, neighborMessage_t *pxMessage)
{
    bool_t xStatus;
    serObjectValue_t *pxSer;
    pb_ostream_t xStream;
    rina_messages_neighbor_t message = rina_messages_neighbor_t_init_zero;
    NeighborEncodeCbState_t xState;

    xState.pxNeighbor = pxMessage;
    xState.pxSD = pxSD;

    message.supportingDifs.funcs.encode = prvSerDesNeighborEncodeCb;
    message.supportingDifs.arg = &xState;

    _SERDES_FIELD_COPY(message, aeinstance, pxMessage->pcAeInstance);
    _SERDES_FIELD_COPY(message, aename, pxMessage->pcAeName);
    _SERDES_FIELD_COPY(message, apinstance, pxMessage->pcApInstance);
    _SERDES_FIELD_COPY(message, apname, pxMessage->pcApName);

    /* Allocate space on the stack to store the message data. */
    if (!(pxSer = pxRsrcAlloc(pxSD->xEncPool, "Neighbor Encoding"))) {
        LOGE(TAG_SD_NEIGHBOR, "Failed to allocate memory for neighbor message encoding");
        return NULL;
    }

    pxSer->pvSerBuffer = pxSer + sizeof(serObjectValue_t);

    /* Create a stream that writes to our buffer. */
    xStream = pb_ostream_from_buffer(pxSer->pvSerBuffer, ENROLLMENT_MSG_SIZE);

    /* Now we are ready to encode the message. */
    xStatus = pb_encode(&xStream, rina_messages_neighbor_t_fields, &message);

    /* Check for errors... */
    if (!xStatus) {
        LOGE(TAG_SD_NEIGHBOR, "Encoding failed: %s\n", PB_GET_ERROR(&xStream));
        return NULL;
    }

    pxSer->xSerLength = xStream.bytes_written;

    return pxSer;
}

neighborsMessage_t *pxSerDesNeighborListDecode(NeighborSerDes_t *pxSD,
                                               uint8_t *pucBuffer,
                                               size_t xMessageLength)
{
    bool_t xStatus;
    pb_istream_t xStream;
    neighborsMessage_t *pxNbListMsg, *pxRet = NULL;
    size_t unNbCount,
        unNbAllocSz = 0,
        unNbListSz = 0,
        unNbSz = 0,
        unArraySz = 0;
    rina_messages_neighbors_t message = rina_messages_neighbors_t_init_zero;
    NeighborListDecodeCbState_t xState = {
        .pxSD = pxSD,
    };

    message.neighbor.funcs.decode = prvSerDesNeighborListDecodeCb;
    message.neighbor.arg = &xState;

    if (ERR_CHK_MEM(xArrayListInit(&xState.xNbLst, sizeof(neighborMessage_t *), 10, pxSD->xDecPool)))
        return NULL;

    /* Create a stream that will read from the buffer */
    xStream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    /* Decode the message */
    xStatus = pb_decode(&xStream, rina_messages_neighbors_t_fields, &message);

    if (xStatus) {
        unNbCount = unArrayListCount(&xState.xNbLst);
        unArraySz = unNbCount * sizeof(neighborsMessage_t *);

        /* Prepare the array for the neighbors. */
        unNbListSz = sizeof(neighborsMessage_t) + unArraySz + unNbAllocSz;

        if (!(pxNbListMsg = pxRsrcVarAlloc(pxSD->xDecPool, "Neighbor List Decode", unNbListSz)))
            LOGE(TAG_SD_NEIGHBOR, "Failed to allocate memory for neighbor list");

        for (size_t n = 0; n < unNbCount; n++) {
            neighborMessage_t *pxNbMsg;

            pxNbMsg = *(neighborMessage_t **)pvArrayListGet(&xState.xNbLst, n);
            pxNbListMsg->pxNeighsMsg[n] = pxNbMsg;
        }

        pxNbListMsg->unNb = unNbCount;
        pxRet = pxNbListMsg;
    }
    else LOGE(TAG_SD_NEIGHBOR, "Decoding failed: %s", PB_GET_ERROR(&xStream));

    /* If we're not returning anything, clear any memory we may have
     * allocated for the neighbor structures. Otherwise, we need to
     * keep those because the returned neighbor list will have
     * pointers. */
    if (!pxRet)
        for (size_t n = 0; n < unArrayListCount(&xState.xNbLst); n++)
            vRsrcFree(*(void **)pvArrayListGet(&xState.xNbLst, n));

    /* This is safe to free though. */
    vArrayListFree(&xState.xNbLst);

    return pxRet;
}

serObjectValue_t *pxSerDesNeighborListEncode(NeighborSerDes_t *pxSD,
                                             size_t unNeighCount,
                                             neighborMessage_t **pxNeighbors)
{
    rina_messages_neighbors_t message = rina_messages_neighbors_t_init_zero;
    bool_t xStatus;
    serObjectValue_t *pxSer;
    pb_ostream_t xStream;
    NeighborListEncodeCbState_t xState = {
        .pxSD = pxSD,
        .unNeighCount = unNeighCount,
        .pxNeighbors = pxNeighbors
    };
    size_t unSz;

    message.neighbor.funcs.encode = prvSerDesNeighborListEncodeCb;
    message.neighbor.arg = &xState;

    /* Allocate space on the heap to store the message data. */
    if (!(pxSer = pxRsrcAlloc(pxSD->xEncPool, "Neighbor List Encoding"))) {
        LOGE(TAG_SD_NEIGHBOR, "Failed to allocate memory for neighbor message encoding");
        return NULL;
    }

    pxSer->pvSerBuffer = pxSer + sizeof(serObjectValue_t);

    /* Create a stream that writes to our buffer. */
    xStream = pb_ostream_from_buffer(pxSer->pvSerBuffer, ENROLLMENT_MSG_SIZE);

    /* Now we are ready to encode the message. */
    xStatus = pb_encode(&xStream, rina_messages_neighbors_t_fields, &message);

    /* Check for errors... */
    if (!xStatus) {
        LOGE(TAG_SD_NEIGHBOR, "Encoding failed: %s\n", PB_GET_ERROR(&xStream));
        return NULL;
    }

    pxSer->xSerLength = xStream.bytes_written;

    return pxSer;
}
