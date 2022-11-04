#include <string.h>

#include "common/rsrc.h"
#include "portability/port.h"

#include "SerDes.h"
#include "SerDesAData.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "CommonMessages.pb.h"

#define TAG_SD_ADATA "[SD-ADATA]"

bool_t xSerDesADataInit(ADataSerDes_t *pxSD)
{
    if (!(pxSD->xPool = pxRsrcNewVarPool("AData Decoding", 0))) {
        LOGE(TAG_SD_ADATA, "Failed to allocate memory pool for AData decoding");
        return false;
    }

    return true;
}

aDataMsg_t *pxSerDesADataDecode(ADataSerDes_t *pxSD, uint8_t *pucBuffer, size_t xMessageLength)
{
    bool_t xStatus;
    aDataMsg_t *pxMessage;
    size_t unSz;
    pb_istream_t xStream;
    rina_messages_a_data_t message = rina_messages_a_data_t_init_zero;

    /*Create a stream that will read from the buffer*/
    xStream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    xStatus = pb_decode(&xStream, rina_messages_a_data_t_fields, &message);

    if (!xStatus) {
        LOGE(TAG_SD_ADATA, "Decoding failed: %s", PB_GET_ERROR(&xStream));
        return NULL;
    }

    if (message.has_cdapMessage)
        unSz = sizeof(aDataMsg_t) + sizeof(serObjectValue_t) + message.cdapMessage.size;
    else
        unSz = sizeof(aDataMsg_t);

    if (!(pxMessage = pxRsrcVarAlloc(pxSD->xPool, "AData decoding", unSz))) {
        LOGE(TAG_SD_ADATA, "Failed to allocate memory for AData decoding");
        return NULL;
    }

    if (message.has_destAddress)
        pxMessage->xDestinationAddress = message.destAddress;

    if (message.has_sourceAddress)
        pxMessage->xSourceAddress = message.sourceAddress;

    if (message.has_cdapMessage) {
        pxMessage->pxMsgCdap = (void *)pxMessage + sizeof(aDataMsg_t);
        pxMessage->pxMsgCdap->pvSerBuffer = pxMessage->pxMsgCdap + sizeof(serObjectValue_t);
        pxMessage->pxMsgCdap->xSerLength = message.cdapMessage.size;

        memcpy(pxMessage->pxMsgCdap->pvSerBuffer, message.cdapMessage.bytes,
               pxMessage->pxMsgCdap->xSerLength);
    }

    return pxMessage;
}

