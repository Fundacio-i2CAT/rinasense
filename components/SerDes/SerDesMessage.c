#include "SerDes.h"
#include "common/rina_name.h"
#include "common/rsrc.h"
#include "portability/port.h"

#include "SerDesMessage.h"
#include "CDAP.pb.h"
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Ribd.h"

#define TAG_SD_MSG "[SD-MSG]"

char *opcodeNamesTable[] = {
    [M_CONNECT] = "M_CONNECT",
    [M_CONNECT_R] = "M_CONNECT_R",
    [M_RELEASE] = "M_RELEASE",
    [M_RELEASE_R] = "M_RELEASE_R",
    [M_CREATE] = "M_CREATE",
    [M_CREATE_R] = "M_CREATE_R",
    [M_DELETE] = "M_DELETE",
    [M_DELETE_R] = "M_DELETE_R",
    [M_READ] = "M_READ",
    [M_READ_R] = "M_READ_R",
    [M_CANCELREAD] = "M_CANCELREAD",
    [M_CANCELREAD_R] = "M_CANCELREAD_R",
    [M_WRITE] = "M_WRITE",
    [M_WRITE_R] = "M_WRITE_R",
    [M_START] = "M_START",
    [M_START_R] = "M_START_R",
    [M_STOP] = "M_STOP",
    [M_STOP_R] = "M_STOP_R"};

void vRibdPrintCdapMessage(messageCdap_t *pxDecodeCdap)
{
    LOGD(TAG_SD_MSG, "DECODE");
    LOGD(TAG_SD_MSG, "opCode: %s", opcodeNamesTable[pxDecodeCdap->eOpCode]);
    LOGD(TAG_SD_MSG, "Invoke Id: %d ", pxDecodeCdap->invokeID);
    LOGD(TAG_SD_MSG, "Version: %jd", pxDecodeCdap->version);

    if (pxDecodeCdap->xAuthPolicy.pcName != NULL) {
        LOGD(TAG_SD_MSG, "AuthPolicy Name: %s", pxDecodeCdap->xAuthPolicy.pcName);
        LOGD(TAG_SD_MSG, "AuthPolicy Version: %s", pxDecodeCdap->xAuthPolicy.pcVersion);
    }

    LOGD(TAG_SD_MSG, "Source AEI: %s", pxDecodeCdap->xSourceInfo.pcEntityInstance);
    LOGD(TAG_SD_MSG, "Source AEN: %s", pxDecodeCdap->xSourceInfo.pcEntityName);
    LOGD(TAG_SD_MSG, "Source API: %s", pxDecodeCdap->xSourceInfo.pcProcessInstance);
    LOGD(TAG_SD_MSG, "Source APN: %s", pxDecodeCdap->xSourceInfo.pcProcessName);
    LOGD(TAG_SD_MSG, "Dest AEI: %s", pxDecodeCdap->xDestinationInfo.pcEntityInstance);
    LOGD(TAG_SD_MSG, "Dest AEN: %s", pxDecodeCdap->xDestinationInfo.pcEntityName);
    LOGD(TAG_SD_MSG, "Dest API: %s", pxDecodeCdap->xDestinationInfo.pcProcessInstance);
    LOGD(TAG_SD_MSG, "Dest APN: %s", pxDecodeCdap->xDestinationInfo.pcProcessName);
    LOGD(TAG_SD_MSG, "Result: %d", pxDecodeCdap->result);

    // configASSERT(pxDecodeCdap->xObjName == NULL);

    if (pxDecodeCdap->pcObjName)
        LOGE(TAG_SD_MSG, "ObjectName:%s", pxDecodeCdap->pcObjName);

    if (pxDecodeCdap->objInst)
        LOGE(TAG_SD_MSG, "ObjectInstance:%d", (int)pxDecodeCdap->objInst);

    if (pxDecodeCdap->pcObjClass)
        LOGE(TAG_SD_MSG, "ObjectClass:%s", pxDecodeCdap->pcObjClass);
}

#if 0
rina_messages_CDAPMessage prvRibdSerToRinaMessage(messageCdap_t *pxMessageCdap)
{
    /*Allocate space on the Stack to store the message data*/
    rina_messages_CDAPMessage message = rina_messages_CDAPMessage_init_zero;

    message.version = pxMessageCdap->version;
    message.has_version = true;
    message.opCode = (rina_messages_opCode_t)pxMessageCdap->eOpCode;
    message.invokeID = pxMessageCdap->invokeID;
    message.has_invokeID = true;

    if (pxMessageCdap->result != -1) {
        message.result = pxMessageCdap->result;
        message.has_result = true;
    }

#define _COPY(x, y) strncpy(x, y, sizeof(x))

    /*Destination*/
    if (pxMessageCdap->pxDestinationInfo->pcEntityInstance != NULL) {
        strcpy(message.destAEInst, pxMessageCdap->pxDestinationInfo->pcEntityInstance);
        message.has_destAEInst = true;
    }

    if (pxMessageCdap->pxDestinationInfo->pcEntityName != NULL)
    {
        strcpy(message.destAEName, pxMessageCdap->pxDestinationInfo->pcEntityName);
        message.has_destAEName = true;
    }

    if (pxMessageCdap->pxDestinationInfo->pcProcessInstance != NULL)
    {
        strcpy(message.destApInst, pxMessageCdap->pxDestinationInfo->pcProcessInstance);
        message.has_destApInst = true;
    }

    if (pxMessageCdap->pxDestinationInfo->pcProcessName != NULL)
    {
        strcpy(message.destApName, pxMessageCdap->pxDestinationInfo->pcProcessName);
        message.has_destApName = true;
    }

    /*Source*/
    if (pxMessageCdap->pxSourceInfo->pcEntityInstance != NULL)
    {
        strcpy(message.srcAEInst, pxMessageCdap->pxSourceInfo->pcEntityInstance);
        message.has_srcAEInst = true;
    }

    if (pxMessageCdap->pxSourceInfo->pcEntityName != NULL)
    {
        strcpy(message.srcAEName, pxMessageCdap->pxSourceInfo->pcEntityName);
        message.has_srcAEName = true;
    }

    if (pxMessageCdap->pxSourceInfo->pcProcessInstance != NULL)
    {
        strcpy(message.srcApInst, pxMessageCdap->pxSourceInfo->pcProcessInstance);
        message.has_srcApInst = true;
    }

    if (pxMessageCdap->pxSourceInfo->pcProcessName != NULL)
    {
        strcpy(message.srcApName, pxMessageCdap->pxSourceInfo->pcProcessName);
        message.has_srcApName = true;
    }

    /*Authentication Policy*/
    if (pxMessageCdap->pxAuthPolicy->pcName != NULL)
    {
        strcpy(message.authPolicy.name, pxMessageCdap->pxAuthPolicy->pcName);
        message.has_authPolicy = true;
        message.authPolicy.versions_count = 1;
        strcpy(message.authPolicy.versions[0], pxMessageCdap->pxAuthPolicy->pcVersion);

        message.authPolicy.has_name = true;
    }

    /*Object Value*/
    if (pxMessageCdap->pcObjClass != NULL)
    {
        strcpy(message.objClass, pxMessageCdap->pcObjClass);
        message.has_objClass = true;
    }

    if (pxMessageCdap->pcObjName != NULL)
    {
        strcpy(message.objName, pxMessageCdap->pcObjName);
        message.has_objName = true;
    }

    if (pxMessageCdap->objInst != -1)
    {
        message.objInst = pxMessageCdap->objInst;
        message.has_objInst = true;
    }

    if (pxMessageCdap->pxObjValue != NULL)
    {
        message.objValue.byteval.size = pxMessageCdap->pxObjValue->xSerLength;
        memcpy(message.objValue.byteval.bytes, pxMessageCdap->pxObjValue->pvSerBuffer, pxMessageCdap->pxObjValue->xSerLength);

        message.has_objValue = true;
        message.objValue.has_byteval = true;
    }

    return message;
}
#endif

bool_t xSerDesMessageInit(MessageSerDes_t *pxSD)
{
    size_t unSz;

    unSz = member_size(rina_messages_CDAPMessage, objClass)
        + member_size(rina_messages_CDAPMessage, objName)
        + member_size(rina_messages_objVal_t_byteval_t, bytes)
        + member_size(rina_messages_authPolicy_t, name)
        + member_size(rina_messages_authPolicy_t, versions)
        + sizeof(messageCdap_t)
        + sizeof(serObjectValue_t);
    if (!(pxSD->xDecPool = pxRsrcNewPool("CDAP Message SerDes Decoding", unSz, 1, 0, 0)))
        return false;

    unSz = sizeof(rina_messages_CDAPMessage) + sizeof(serObjectValue_t);
    if (!(pxSD->xEncPool = pxRsrcNewPool("CDAP Message SerDes Encoding", unSz, 1, 0, 0)))
        return false;

    return true;
}

serObjectValue_t *pxSerDesMessageEncode(MessageSerDes_t *pxSD, messageCdap_t *pxMsgCdap)
{
    bool_t xStatus;
    uint8_t *pucBuffer[128];
    size_t unMessageLength;
    pb_ostream_t xStream;
    rina_messages_CDAPMessage xMsg = rina_messages_CDAPMessage_init_zero;
    serObjectValue_t *pxSerValue;

    LOGI(TAG_RIB, "Encoding a %s CDAP message", opcodeNamesTable[pxMsgCdap->eOpCode]);

    /*Create a stream that will write to the buffer*/
    xStream = pb_ostream_from_buffer((pb_byte_t *)pucBuffer, sizeof(pucBuffer));

    /*Fill the message properly*/
    xMsg.version = pxMsgCdap->version;
    xMsg.has_version = true;
    xMsg.opCode = (rina_messages_opCode_t)pxMsgCdap->eOpCode;
    xMsg.invokeID = pxMsgCdap->invokeID;
    xMsg.has_invokeID = true;

    if (pxMsgCdap->result != -1) {
        xMsg.result = pxMsgCdap->result;
        xMsg.has_result = true;
    }

#define _COPY(x, i, y)                             \
    strncpy(x.i , y, sizeof(x.i));                 \
    xMsg.has_##i = true

    /* Destination */
    _COPY(xMsg, destAEInst, pxMsgCdap->xDestinationInfo.pcEntityInstance);
    _COPY(xMsg, destAEName, pxMsgCdap->xDestinationInfo.pcEntityName);
    _COPY(xMsg, destApInst, pxMsgCdap->xDestinationInfo.pcProcessInstance);
    _COPY(xMsg, destApName, pxMsgCdap->xDestinationInfo.pcProcessName);

    /* Source */
    _COPY(xMsg, srcAEInst, pxMsgCdap->xSourceInfo.pcEntityInstance);
    _COPY(xMsg, srcAEName, pxMsgCdap->xSourceInfo.pcEntityName);
    _COPY(xMsg, srcApInst, pxMsgCdap->xSourceInfo.pcProcessInstance);
    _COPY(xMsg, srcApName, pxMsgCdap->xSourceInfo.pcProcessName);

    /* Authentication Policy */
    if (pxMsgCdap->xAuthPolicy.pcName != NULL) {
        strcpy(xMsg.authPolicy.name, pxMsgCdap->xAuthPolicy.pcName);
        xMsg.has_authPolicy = true;
        xMsg.authPolicy.versions_count = 1;
        strncpy(xMsg.authPolicy.versions[0], pxMsgCdap->xAuthPolicy.pcVersion,
                sizeof(xMsg.authPolicy.versions[0]));

        xMsg.authPolicy.has_name = true;
    }

    /* Object value */
    if (pxMsgCdap->pcObjClass != NULL) {
        _COPY(xMsg, objClass, pxMsgCdap->pcObjClass);
    }

    /* Object name */
    if (pxMsgCdap->pcObjName != NULL) {
        _COPY(xMsg, objName, pxMsgCdap->pcObjName);
    }

    /* Object instance */
    if (pxMsgCdap->objInst != -1) {
        xMsg.objInst = pxMsgCdap->objInst;
        xMsg.has_objInst = true;
    }

    if (pxMsgCdap->pxObjValue != NULL) {
        xMsg.objValue.byteval.size = pxMsgCdap->pxObjValue->xSerLength;
        memcpy(xMsg.objValue.byteval.bytes, pxMsgCdap->pxObjValue->pvSerBuffer, pxMsgCdap->pxObjValue->xSerLength);

        xMsg.has_objValue = true;
        xMsg.objValue.has_byteval = true;
    }

    if (!(pxSerValue = pxRsrcAlloc(pxSD->xEncPool, "CDAP message encoding"))) {
        LOGE(TAG_SD_MSG, "Failed to allocate memory for CDAP message encoding");
        return NULL;
    }

    pxSerValue->pvSerBuffer = pxSerValue + sizeof(serObjectValue_t);

    xStream = pb_ostream_from_buffer((pb_byte_t *)pxSerValue->pvSerBuffer,
                                     sizeof(rina_messages_CDAPMessage));

    /*Encode the message*/
    xStatus = pb_encode(&xStream, rina_messages_CDAPMessage_fields, &xMsg);
    unMessageLength = xStream.bytes_written;

    if (!xStatus) {
        LOGE(TAG_RIB, "Encoding failed: %s", PB_GET_ERROR(&xStream));
        return NULL;
    }

    LOGI(TAG_RIB, "Message CDAP with length: %zu encoded sucessfully ", unMessageLength);

    return pxSerValue;
}

messageCdap_t *pxSerDesMessageDecode(MessageSerDes_t *pxSD, uint8_t *pucBuffer, size_t xMessageLength)
{
    bool_t xStatus;
    pb_istream_t xStream;
    messageCdap_t *pxMsg;
    void *px;
    rina_messages_CDAPMessage message = rina_messages_CDAPMessage_init_zero;

    /*Create a stream that will read from the buffer*/
    xStream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    xStatus = pb_decode(&xStream, rina_messages_CDAPMessage_fields, &message);

    if (!xStatus) {
        LOGE(TAG_RIB, "Decoding failed: %s", PB_GET_ERROR(&xStream));
        return NULL;
    }

    if (!(pxMsg = pxRsrcAlloc(pxSD->xDecPool, "CDAP Message Decoding"))) {
        LOGE(TAG_SD_MSG, "Failed to decode CDAP message");
        return NULL;
    }

    if (!(xNameAssignFromPartsDup(&pxMsg->xDestinationInfo,
                                  message.destApName, message.destApInst,
                                  message.destAEName, message.destAEInst))) {
        LOGE(TAG_SD_MSG, "Failed to assign destination RINA name");
        return NULL;
    }

    if (!(xNameAssignFromPartsDup(&pxMsg->xSourceInfo,
                                  message.srcApName, message.srcApInst,
                                  message.srcAEName, message.srcAEName))) {
        LOGE(TAG_SD_MSG, "Failed to assigne source RINA name");
        return NULL;
    }

    pxMsg->eOpCode = (opCode_t)message.opCode;
    pxMsg->version = message.version;
    pxMsg->invokeID = message.invokeID;
    pxMsg->result = message.result;
    pxMsg->objInst = message.objInst;

    /* Initialize the string pointers */
    pxMsg->pcObjClass = (px = (void *)pxMsg + sizeof(messageCdap_t));
    pxMsg->pcObjName = (px += member_size(rina_messages_CDAPMessage, objClass));
    pxMsg->xAuthPolicy.pcName = (px += member_size(rina_messages_CDAPMessage, objName));
    pxMsg->xAuthPolicy.pcVersion = (px += member_size(rina_messages_authPolicy_t, name));

    /* Move the pointer past the version size for next use */
    px += member_size(rina_messages_authPolicy_t, versions);

    /* Copy the strings */
    if (message.has_objClass)
        strcpy(pxMsg->pcObjClass, message.objClass);
    else
        strcpy(pxMsg->pcObjClass, "");

    if (message.has_objName)
        strcpy(pxMsg->pcObjName, message.objName);
    else
        strcpy(pxMsg->pcObjName, "");

    if (message.has_authPolicy) {
        if (message.has_authPolicy && message.authPolicy.has_name)
            strcpy(pxMsg->xAuthPolicy.pcName, message.authPolicy.name);

        strcpy(pxMsg->xAuthPolicy.pcVersion, message.authPolicy.versions[0]);
    }
    else {
        strcpy(pxMsg->xAuthPolicy.pcName, "");
        strcpy(pxMsg->xAuthPolicy.pcVersion, "");
    }

    if (message.has_objValue) {
        pxMsg->pxObjValue = (px += member_size(rina_messages_objVal_t, byteval));
        pxMsg->pxObjValue->pvSerBuffer = (px += member_size(rina_messages_objVal_t_byteval_t, bytes));
        pxMsg->pxObjValue->xSerLength = message.objValue.byteval.size;

        memcpy(pxMsg->pxObjValue->pvSerBuffer, message.objValue.byteval.bytes,
               pxMsg->pxObjValue->xSerLength);
    }

    return pxMsg;
}
