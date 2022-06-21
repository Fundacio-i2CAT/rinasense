#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "IPCP.h"
#include "IPCP_events.h"
#include "IPCP_api.h"
#include "rina_common.h"
#include "configRINA.h"
#include "BufferManagement.h"
#include "CDAP.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Ribd.h"
#include "configSensor.h"
#include "Rib.h"
#include "RINA_API.h"
#include "normalIPCP.h"
#include "Rmt.h"
#include "FlowAllocator.h"

#include "esp_log.h"

int n = 0;
static char *opcodeNamesTable[] = {[M_CONNECT] = "M_CONNECT",
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

/* Table to manage the app connections */
appConnectionTableRow_t xAppConnectionTable[APP_CONNECTION_TABLE_SIZE];

/* Table to manage the pending request to response*/
responseHandlersRow_t xPendingResponseHandlersTable[RESPONSE_HANDLER_TABLE_SIZE];

/* Encode the CDAP message */
NetworkBufferDescriptor_t *prvRibEncodeCDAP(rina_messages_opCode_t xMessageOpCode,
                                            name_t *pxSrcInfo, name_t *pxDestInfo,
                                            int64_t version, authPolicy_t *pxAuth);

/* Decode the CDAP message */
BaseType_t xRibdecodeCDAP(uint8_t *pucBuffer, size_t xMessageLength, messageCdap_t *pxMessageCdap);

messageCdap_t *prvRibdFillDecodeMessage(rina_messages_CDAPMessage message);

BaseType_t xRibdppConnection(portId_t xPortId);

BaseType_t vRibHandleMessage(struct ipcpNormalData_t *pxData, messageCdap_t *pxDecodeCdap, portId_t xN1FlowPortId);
BaseType_t xRibdProcessLayerManagementPDU(struct ipcpNormalData_t *pxData, portId_t xN1flowPortId, struct du_t *pxDu);

struct ribCallbackOps_t *pxRibdCreateCdapCallback(opCode_t xOpCode, int invoke_id);

void vRibdAddResponseHandler(int32_t invokeID, struct ribCallbackOps_t *pxCb)
{

    BaseType_t x = 0;

    if (pxCb != NULL)
    {

        for (x = 0; x < RESPONSE_HANDLER_TABLE_SIZE; x++)
        {
            if (xPendingResponseHandlersTable[x].xValid == pdFALSE)
            {
                xPendingResponseHandlersTable[x].invokeID = invokeID;
                xPendingResponseHandlersTable[x].xValid = pdTRUE;
                xPendingResponseHandlersTable[x].pxCallbackHandler = pxCb;

                //                ESP_LOGE(TAG_RIB, "Pending Handlers Entry successful: %p", pxCb);

                break;
            }
        }
    }
    else
    {
        ESP_LOGE(TAG_RIB, "No pxcb");
    }
}

struct ribCallbackOps_t *pxRibdFindPendingResponseHandler(int32_t invokeID)
{

    BaseType_t x = 0;
    struct ribCallbackOps_t *pxCb;
    pxCb = pvPortMalloc(sizeof(*pxCb));

    for (x = 0; x < RESPONSE_HANDLER_TABLE_SIZE; x++)

    {
        if (xPendingResponseHandlersTable[x].xValid == pdTRUE)
        {

            if (xPendingResponseHandlersTable[x].invokeID == invokeID)
            {

                pxCb = xPendingResponseHandlersTable[x].pxCallbackHandler;
                ESP_LOGI(TAG_IPCPMANAGER, "Cb Handler founded '%p'", pxCb);

                return pxCb;
                break;
            }
        }
    }
    return NULL;
}

void vRibdAddAppConnectionEntry(appConnection_t *pxAppConnectionToAdd, portId_t xPortId)
{
    ESP_LOGI(TAG_RIB, "Adding a new APP Connection into the AppConnection Table");

    BaseType_t x = 0;

    for (x = 0; x < APP_CONNECTION_TABLE_SIZE; x++)
    {
        if (xAppConnectionTable[x].xValid == pdFALSE)
        {
            xAppConnectionTable[x].pxAppConnection = pxAppConnectionToAdd;
            xAppConnectionTable[x].xN1portId = xPortId;
            xAppConnectionTable[x].xValid = pdTRUE;
            ESP_LOGI(TAG_RIB, "AppConnection Entry successful: %p,id:%d", pxAppConnectionToAdd, xPortId);

            break;
        }
    }
}

BaseType_t xTest(void);
BaseType_t xTest(void)
{
    return pdTRUE;
}

messageCdap_t *prvRibMessageCdapInit(void);
messageCdap_t *prvRibMessageCdapInit(void)
{
    messageCdap_t *pxMessage = pvPortMalloc(sizeof(*pxMessage));
    name_t *pxDestinationInfo = pvPortMalloc(sizeof(*pxDestinationInfo));
    name_t *pxSourceInfo = pvPortMalloc(sizeof(*pxSourceInfo));
    authPolicy_t *pxAuthPolicy = pvPortMalloc(sizeof(*pxAuthPolicy));

    pxMessage->pxDestinationInfo = pxDestinationInfo;
    pxMessage->pxSourceInfo = pxSourceInfo;
    pxMessage->pxAuthPolicy = pxAuthPolicy;

    /* Init to Default Values*/

    pxMessage->version = 0x01;
    pxMessage->eOpCode = -1; // No code
    pxMessage->invokeID = 1; // by default
    pxMessage->objInst = -1;
    pxMessage->pcObjClass = NULL;
    pxMessage->pcObjName = NULL;
    pxMessage->pxObjValue = NULL;
    pxMessage->result = 0;

    pxMessage->pxDestinationInfo->pcEntityInstance = NULL;
    pxMessage->pxDestinationInfo->pcEntityName = MANAGEMENT_AE;
    pxMessage->pxDestinationInfo->pcProcessInstance = NULL;
    pxMessage->pxDestinationInfo->pcProcessName = NULL;

    pxMessage->pxSourceInfo->pcEntityInstance = "";
    pxMessage->pxSourceInfo->pcEntityName = MANAGEMENT_AE;
    pxMessage->pxSourceInfo->pcProcessInstance = NULL;
    pxMessage->pxSourceInfo->pcProcessName = NULL;

    pxMessage->pxAuthPolicy->pcName = NULL;
    pxMessage->pxAuthPolicy->pcVersion = NULL;

    return pxMessage;
}

messageCdap_t *prvRibdFillDecodeMessage(rina_messages_CDAPMessage message)
{
    messageCdap_t *pxMessageCdap;
    name_t *pxDestinationInfo = pvPortMalloc(sizeof(*pxDestinationInfo));
    name_t *pxSourceInfo = pvPortMalloc(sizeof(*pxSourceInfo));
    authPolicy_t *pxAuthPolicy = pvPortMalloc(sizeof(*pxAuthPolicy));

    pxMessageCdap = pvPortMalloc(sizeof(*pxMessageCdap));

    pxMessageCdap->pxDestinationInfo = pxDestinationInfo;
    pxMessageCdap->pxSourceInfo = pxSourceInfo;
    pxMessageCdap->pxAuthPolicy = pxAuthPolicy;

    pxMessageCdap->eOpCode = message.opCode;
    pxMessageCdap->version = message.version;
    pxMessageCdap->invokeID = message.invokeID;
    pxMessageCdap->result = message.result;

    if (message.has_destAEInst)
    {
        pxMessageCdap->pxDestinationInfo->pcEntityInstance = strdup(message.destAEInst);
    }

    if (message.has_destAEName)
    {
        pxMessageCdap->pxDestinationInfo->pcEntityName = strdup(message.destAEName);
    }

    if (message.has_destApInst)
    {
        pxMessageCdap->pxDestinationInfo->pcProcessInstance = strdup(message.destApInst);
    }

    if (message.has_destApName)
    {
        pxMessageCdap->pxDestinationInfo->pcProcessName = strdup(message.destApName);
    }

    if (message.has_srcAEInst)
    {
        pxMessageCdap->pxSourceInfo->pcEntityInstance = strdup(message.srcAEInst);
    }

    if (message.has_srcAEName)
    {
        pxMessageCdap->pxSourceInfo->pcEntityName = strdup(message.srcAEName);
    }

    if (message.has_srcApInst)
    {
        pxMessageCdap->pxSourceInfo->pcProcessInstance = strdup(message.srcApInst);
    }

    if (message.has_srcApName)
    {
        pxMessageCdap->pxSourceInfo->pcProcessName = strdup(message.srcApName);
    }

    if (message.has_authPolicy)
    {
        if (message.authPolicy.has_name)
        {
            pxMessageCdap->pxAuthPolicy->pcName = strdup(message.authPolicy.name);
        }
        pxMessageCdap->pxAuthPolicy->pcVersion = strdup(message.authPolicy.versions);
    }

    if (message.has_objClass)
    {
        pxMessageCdap->pcObjClass = strdup(message.objClass);
    }

    if (message.has_objName)
    {
        pxMessageCdap->pcObjName = strdup(message.objName);
    }

    if (message.has_objInst)
    {
        pxMessageCdap->objInst = message.objInst;
    }

    if (message.has_objValue)
    {
        // configASSERT(message.objValue.has_byteval == true);
        serObjectValue_t *pxSerObjVal = pvPortMalloc(sizeof(*pxSerObjVal));
        void *pvSerBuf = pvPortMalloc(message.objValue.byteval.size);

        pxMessageCdap->pxObjValue = pxSerObjVal;
        pxMessageCdap->pxObjValue->pvSerBuffer = pvSerBuf;
        pxMessageCdap->pxObjValue->xSerLength = message.objValue.byteval.size;

        memcpy(pxMessageCdap->pxObjValue->pvSerBuffer, message.objValue.byteval.bytes,
               pxMessageCdap->pxObjValue->xSerLength);
        // ESP_LOGE(TAG_RIB,"It has object Value!!!!!!!!!!!!!!!");
    }

    return pxMessageCdap;
}

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    const char *str = (const char *)(*arg);

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t *)str, strlen(str));
}

rina_messages_CDAPMessage prvRibdSerToRinaMessage(messageCdap_t *pxMessageCdap)
{

    /*Allocate space on the Stack to store the message data*/
    rina_messages_CDAPMessage message = rina_messages_CDAPMessage_init_zero;

    message.version = pxMessageCdap->version;
    message.has_version = true;

    message.opCode = pxMessageCdap->eOpCode;

    message.invokeID = pxMessageCdap->invokeID;
    message.has_invokeID = true;

    if (pxMessageCdap->result != -1)
    {
        message.result = pxMessageCdap->result;
        message.has_result = true;
    }

    /*Destination*/
    if (pxMessageCdap->pxDestinationInfo->pcEntityInstance != NULL)
    {
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
        strcpy(message.authPolicy.versions, pxMessageCdap->pxAuthPolicy->pcVersion);

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

NetworkBufferDescriptor_t *prvRibdEncodeCDAP(messageCdap_t *pxMessageCdap)
{
    ESP_LOGI(TAG_RIB, "Encoding an CDAP message...");
    ESP_LOGI(TAG_RIB, "Encoding a %s message", opcodeNamesTable[pxMessageCdap->eOpCode]);
    BaseType_t status;
    uint8_t *pucBuffer[128];
    size_t xMessageLength;

    /*Create a stream that will write to the buffer*/
    pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)pucBuffer, sizeof(pucBuffer));

    /*Fill the message properly*/
    rina_messages_CDAPMessage message = rina_messages_CDAPMessage_init_zero;

    message.version = pxMessageCdap->version;
    message.has_version = true;

    message.opCode = pxMessageCdap->eOpCode;

    message.invokeID = pxMessageCdap->invokeID;
    message.has_invokeID = true;

    if (pxMessageCdap->result != -1)
    {
        message.result = pxMessageCdap->result;
        message.has_result = true;
    }

    /*Destination*/
    if (pxMessageCdap->pxDestinationInfo->pcEntityInstance != NULL)
    {
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
        strcpy(message.authPolicy.versions, pxMessageCdap->pxAuthPolicy->pcVersion);

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

    /*Encode the message*/
    status = pb_encode(&stream, rina_messages_CDAPMessage_fields, &message);
    xMessageLength = stream.bytes_written;

    if (!status)
    {
        ESP_LOGE(TAG_RINA, "Encoding failed: %s", PB_GET_ERROR(&stream));
        return NULL;
    }

    ESP_LOGI(TAG_RIB, "Message CDAP with length: %d encoded sucessfully ", xMessageLength);

    /*Request a Network Buffer according to Message Length*/
    NetworkBufferDescriptor_t *pxNetworkBuffer;

    // ESP_LOGE(TAG_RIB, "Taking Buffer to encode the CDAP message: RIBD");
    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(xMessageLength, (TickType_t)0U);

    /*Copy Buffer into the NetworkBuffer*/
    memcpy(pxNetworkBuffer->pucEthernetBuffer, &pucBuffer, xMessageLength);

    pxNetworkBuffer->xDataLength = xMessageLength;
    // ESP_LOGE(TAG_RIB, "Buffering OK");

    return pxNetworkBuffer;
}

messageCdap_t *prvRibdDecodeCDAP(uint8_t *pucBuffer, size_t xMessageLength)
{
    ESP_LOGI(TAG_RIB, "Decoding CDAP Message");

    BaseType_t status;

    /*Allocate space for the decode message data*/
    rina_messages_CDAPMessage message = rina_messages_CDAPMessage_init_zero;

    /*Create a stream that will read from the buffer*/
    pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    status = pb_decode(&stream, rina_messages_CDAPMessage_fields, &message);

    if (!status)
    {
        ESP_LOGE(TAG_RINA, "Decoding failed: %s", PB_GET_ERROR(&stream));
        return NULL;
    }
    ESP_LOGI(TAG_RIB, "CDAP Message Decode sucessfully");
    return prvRibdFillDecodeMessage(message);
}

appConnection_t *pxRibdFindAppConnection(portId_t xPortId)
{
    ESP_LOGI(TAG_RIB, "Looking for an active connection in the port id %d", xPortId);
    BaseType_t x = 0;
    appConnection_t *pxAppConnection;
    pxAppConnection = pvPortMalloc(sizeof(*pxAppConnection));

    for (x = 0; x < APP_CONNECTION_TABLE_SIZE; x++)

    {
        if (xAppConnectionTable[x].xValid == pdTRUE)
        {
            if (xAppConnectionTable[x].xN1portId == xPortId)
            {
                ESP_LOGI(TAG_RIB, "AppConnection founded: '%p'", xAppConnectionTable[x].pxAppConnection);
                pxAppConnection = xAppConnectionTable[x].pxAppConnection;

                return pxAppConnection;
                break;
            }
        }
    }
    return NULL;
}

void vRibdPrintCdapMessage(messageCdap_t *pxDecodeCdap);

appConnection_t *prvRibCreateConnection(name_t *pxSource, name_t *pxDestInfo)

{
    appConnection_t *pxAppConnectionTmp = pvPortMalloc(sizeof(*pxAppConnectionTmp));
    name_t *pxDestinationInfo = pvPortMalloc(sizeof(*pxDestinationInfo));
    name_t *pxSourceInfo = pvPortMalloc(sizeof(*pxSourceInfo));

    pxAppConnectionTmp->pxDestinationInfo = pxDestinationInfo;
    pxAppConnectionTmp->pxSourceInfo = pxSourceInfo;

    pxAppConnectionTmp->uCdapVersion = 0x01;
    pxAppConnectionTmp->pxSourceInfo->pcEntityInstance = strdup(pxSource->pcEntityInstance);
    pxAppConnectionTmp->pxSourceInfo->pcEntityName = strdup(pxSource->pcEntityName);
    pxAppConnectionTmp->pxSourceInfo->pcProcessInstance = strdup(pxSource->pcProcessInstance);
    pxAppConnectionTmp->pxSourceInfo->pcEntityInstance = strdup(pxSource->pcProcessName);
    pxAppConnectionTmp->pxDestinationInfo->pcEntityInstance = strdup(pxDestInfo->pcEntityInstance);
    pxAppConnectionTmp->pxDestinationInfo->pcEntityName = strdup(pxDestInfo->pcEntityName);
    pxAppConnectionTmp->pxDestinationInfo->pcProcessInstance = strdup(pxDestInfo->pcProcessInstance);
    pxAppConnectionTmp->pxDestinationInfo->pcEntityInstance = strdup(pxDestInfo->pcProcessName);
    pxAppConnectionTmp->xStatus = eCONNECTION_IN_PROGRESS;
    pxAppConnectionTmp->uRibVersion = 0x01;

    return pxAppConnectionTmp;
}

void vRibdSentCdapMsg(NetworkBufferDescriptor_t *pxNetworkBuffer, portId_t xN1FlowPortId)
{

    ESP_LOGI(TAG_RIB, "Sending the CDAP Message to the RMT");
    struct du_t *pxMessagePDU;
    struct rmt_t *pxRmt;

    pxRmt = pxIPCPGetRmt();
    /* Fill the DU with PDU type (layer management)*/
    pxMessagePDU = pvPortMalloc(sizeof(*pxMessagePDU));
    pxMessagePDU->pxNetworkBuffer = pxNetworkBuffer;
    pxMessagePDU->pxNetworkBuffer->ulBoundPort = xN1FlowPortId;

    if (!xNormalMgmtDuWrite(pxRmt, xN1FlowPortId, pxMessagePDU))
    {
        ESP_LOGE(TAG_RIB, "Error to send the CDAP message");
    }
}

messageCdap_t *prvRibdFillEnrollMsg(string_t pcObjClass, string_t pcObjName, long objInst, opCode_t eOpCode,
                                    serObjectValue_t *pxObjValue)
{
    messageCdap_t *pxMessage = prvRibMessageCdapInit();

    pxMessage->invokeID = get_next_invoke_id(); //???
    pxMessage->eOpCode = eOpCode;
    pxMessage->objInst = objInst;

    // ESP_LOGI(TAG_RIB, "pcObjectClass: %s", pcObjClass);

    /*if (pcObjClass != NULL)
    {
        pxMessage->pcObjClass = strdup(pcObjClass);
    }*/

    if (pcObjName != NULL)
    {
        pxMessage->pcObjName = strdup(pcObjName);
    }
    // Check
    if (pxObjValue != NULL)
    {
        pxMessage->pxObjValue = (void *)(pxObjValue);
    }

    return pxMessage;
}

messageCdap_t *prvRibdFillCommon(string_t pcObjClass, string_t pcObjName, long objInst, opCode_t eOpCode,
                                 serObjectValue_t *pxObjValue);

messageCdap_t *prvRibdFillCommon(string_t pcObjClass, string_t pcObjName, long objInst, opCode_t eOpCode,
                                 serObjectValue_t *pxObjValue)
{
    messageCdap_t *pxMsgCdap = prvRibMessageCdapInit();

    pxMsgCdap->invokeID = get_next_invoke_id(); //???
    pxMsgCdap->eOpCode = eOpCode;
    pxMsgCdap->objInst = objInst;

    if (pcObjClass != NULL)
    {
        pxMsgCdap->pcObjClass = strdup(pcObjClass);
    }

    if (pcObjName != NULL)
    {
        pxMsgCdap->pcObjName = strdup(pcObjName);
    }
    // Check
    if (pxObjValue != NULL)
    {
        pxMsgCdap->pxObjValue = (void *)(pxObjValue);
    }
    return pxMsgCdap;
}

messageCdap_t *prvRibdFillEnrollMsgStop(string_t pcObjClass, string_t pcObjName, long objInst, opCode_t eOpCode,
                                        serObjectValue_t *pxObjValue, int result, string_t pcResultReason, int invokeID)

{
    messageCdap_t *pxMessage = prvRibdFillEnrollMsg(pcObjClass, pcObjName, objInst, eOpCode, pxObjValue);

    pxMessage->invokeID = invokeID;
    pxMessage->result = result;

    return pxMessage;
}

messageCdap_t *prvRibdFillMsgCreate(string_t pcObjClass, string_t pcObjName, long objInst, serObjectValue_t *pxObjValue);

messageCdap_t *prvRibdFillMsgCreate(string_t pcObjClass, string_t pcObjName, long objInst, serObjectValue_t *pxObjValue)
{
    return prvRibdFillCommon(pcObjClass, pcObjName, objInst, M_CREATE, pxObjValue);
}

messageCdap_t *prvRibdFillEnrollMsgStart(string_t pcObjClass, string_t pcObjName, long objInst, opCode_t eOpCode,
                                         serObjectValue_t *pxObjValue, int result, string_t pcResultReason, int invokeID)

{
    messageCdap_t *pxMessage = prvRibdFillEnrollMsg(pcObjClass, pcObjName, objInst, eOpCode, pxObjValue);

    pxMessage->invokeID = invokeID;
    pxMessage->result = result;

    return pxMessage;
}

BaseType_t xRibdConnectToIpcp(struct ipcpNormalData_t *pxIpcpData, name_t *pxSource, name_t *pxDestInfo, portId_t xN1flowPortId, authPolicy_t *pxAuth)
{

    ESP_LOGI(TAG_RIB, "Preparing a M_CONNECT message");
    /*Check for app_connections*/
    appConnection_t *pxAppConnectionTmp;
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    size_t xBufferSize;

    messageCdap_t *pxMessageEncode = prvRibMessageCdapInit();

    /*Fill the Message to be encoded in the connection*/
    pxMessageEncode->eOpCode = rina_messages_opCode_t_M_CONNECT;
    pxMessageEncode->pxDestinationInfo->pcEntityName = strdup(pxDestInfo->pcEntityName);
    pxMessageEncode->pxDestinationInfo->pcProcessInstance = strdup(pxDestInfo->pcProcessInstance);
    pxMessageEncode->pxDestinationInfo->pcProcessName = strdup(pxDestInfo->pcProcessName);

    pxMessageEncode->pxSourceInfo->pcEntityName = strdup(pxSource->pcEntityName);
    pxMessageEncode->pxSourceInfo->pcProcessInstance = strdup(pxSource->pcProcessInstance);
    pxMessageEncode->pxSourceInfo->pcProcessName = strdup(pxSource->pcProcessName);

    pxMessageEncode->pxAuthPolicy->pcName = strdup(pxAuth->pcName);
    pxMessageEncode->pxAuthPolicy->pcVersion = strdup(pxAuth->pcVersion);

    // printf("ENCODE\n");
    // vRibdPrintCdapMessage(pxMessageEncode);

    /*Fill the appConnection structure*/
    pxAppConnectionTmp = prvRibCreateConnection(pxSource, pxDestInfo);
    vRibdAddAppConnectionEntry(pxAppConnectionTmp, xN1flowPortId);

    /* Generate and Encode Message M_CONNECT*/
    pxNetworkBuffer = prvRibdEncodeCDAP(pxMessageEncode);
    if (!pxNetworkBuffer)
    {
        ESP_LOGE(TAG_RINA, "Error encoding CDAP message");
        return pdFALSE;
    }

    /*Testing*/
    /* pxMessageDecode = prvRibdDecodeCDAP(pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength);
     vRibdPrintCdapMessage( pxMessageDecode );*/

    vRibdSentCdapMsg(pxNetworkBuffer, xN1flowPortId);

    return pdTRUE;
}

void vRibdPrintCdapMessage(messageCdap_t *pxDecodeCdap)
{
    ESP_LOGE(TAG_RIB, "DECODE");
    ESP_LOGE(TAG_RIB, "opCode: %s", opcodeNamesTable[pxDecodeCdap->eOpCode]);
    ESP_LOGE(TAG_RIB, "Invoke Id: %d ", pxDecodeCdap->invokeID);
    ESP_LOGE(TAG_RIB, "Version: %lld", pxDecodeCdap->version);
    if (pxDecodeCdap->pxAuthPolicy->pcName != NULL)
    {
        ESP_LOGE(TAG_RIB, "AuthPolicy Name: %s", pxDecodeCdap->pxAuthPolicy->pcName);
        ESP_LOGE(TAG_RIB, "AuthPolicy Version: %s", pxDecodeCdap->pxAuthPolicy->pcVersion);
    }

    ESP_LOGE(TAG_RIB, "Source AEI: %s", pxDecodeCdap->pxSourceInfo->pcEntityInstance);
    ESP_LOGE(TAG_RIB, "Source AEN: %s", pxDecodeCdap->pxSourceInfo->pcEntityName);
    ESP_LOGE(TAG_RIB, "Source API: %s", pxDecodeCdap->pxSourceInfo->pcProcessInstance);
    ESP_LOGE(TAG_RIB, "Source APN: %s", pxDecodeCdap->pxSourceInfo->pcProcessName);
    ESP_LOGE(TAG_RIB, "Dest AEI: %s", pxDecodeCdap->pxDestinationInfo->pcEntityInstance);
    ESP_LOGE(TAG_RIB, "Dest AEN: %s", pxDecodeCdap->pxDestinationInfo->pcEntityName);
    ESP_LOGE(TAG_RIB, "Dest API: %s", pxDecodeCdap->pxDestinationInfo->pcProcessInstance);
    ESP_LOGE(TAG_RIB, "Dest APN: %s", pxDecodeCdap->pxDestinationInfo->pcProcessName);

    // configASSERT(pxDecodeCdap->xObjName == NULL);

    if (pxDecodeCdap->pcObjName)
    {
        ESP_LOGE(TAG_RIB, "ObjectName:%s", pxDecodeCdap->pcObjName);
    }
    if (!pxDecodeCdap->objInst)
    {
        ESP_LOGE(TAG_RIB, "ObjectInstance:%d", (int)pxDecodeCdap->objInst);
    }
    if (!pxDecodeCdap->pcObjClass)
    {
        ESP_LOGE(TAG_RIB, "ObjectClass:%s", pxDecodeCdap->pcObjClass);
    }
}

BaseType_t xRibdProcessLayerManagementPDU(struct ipcpNormalData_t *pxData, portId_t xN1flowPortId, struct du_t *pxDu)
{
    ESP_LOGI(TAG_RIB, "Processing a Management PDU");

    /*Struct parallel CDAP message*/
    messageCdap_t *pxDecodeCdap;

    /*Decode CDAP Message*/
    pxDecodeCdap = prvRibdDecodeCDAP(pxDu->pxNetworkBuffer->pucDataBuffer, pxDu->pxNetworkBuffer->xDataLength);

    // vRibdPrintCdapMessage(pxDecodeCdap);

    if (!pxDecodeCdap)
    {
        ESP_LOGE(TAG_RINA, "Error decoding CDAP message");
        return pdFALSE;
    }

    /* Destroying the PDU it is not longer required */
    ESP_LOGI(TAG_RIB, "Destroying the PDU that is no longer used");
    xDuDestroy(pxDu);

    /*Call to rib Handle Message*/
    vRibHandleMessage(pxData, pxDecodeCdap, xN1flowPortId);

    return pdTRUE;
}

void vPrintAppConnection(appConnection_t *pxAppConnection)
{
    ESP_LOGI(TAG_RIB, "--------APP CONNECTION--------");
    ESP_LOGI(TAG_RIB, "Destination EI:%s", pxAppConnection->pxDestinationInfo->pcEntityInstance);
    ESP_LOGI(TAG_RIB, "Destination EN:%s", pxAppConnection->pxDestinationInfo->pcEntityName);
    ESP_LOGI(TAG_RIB, "Destination PI:%s", pxAppConnection->pxDestinationInfo->pcProcessInstance);
    ESP_LOGI(TAG_RIB, "Destination PN:%s", pxAppConnection->pxDestinationInfo->pcProcessName);

    ESP_LOGI(TAG_RIB, "Source EI:%s", pxAppConnection->pxSourceInfo->pcEntityInstance);
    ESP_LOGI(TAG_RIB, "Source EN:%s", pxAppConnection->pxSourceInfo->pcEntityName);
    ESP_LOGI(TAG_RIB, "Source PI:%s", pxAppConnection->pxSourceInfo->pcProcessInstance);
    ESP_LOGI(TAG_RIB, "Source PN:%s", pxAppConnection->pxSourceInfo->pcProcessName);
    ESP_LOGI(TAG_RIB, "Connection Status:%d", pxAppConnection->xStatus);
}

BaseType_t vRibHandleMessage(struct ipcpNormalData_t *pxData, messageCdap_t *pxDecodeCdap, portId_t xN1FlowPortId)
{

    BaseType_t ret = pdTRUE;
    appConnection_t *pxAppConnectionTmp;
    struct ribObject_t *pxRibObject;
    struct ribCallbackOps_t *pxCallback;

    /*Check if the Operation Code is valid*/
    if (pxDecodeCdap->eOpCode > MAX_CDAP_OPCODE)
    {
        ESP_LOGE(TAG_RIB, "Invalid opcode %s", opcodeNamesTable[pxDecodeCdap->eOpCode]);
        vPortFree(pxDecodeCdap);
        return ret;
    }

    ESP_LOGI(TAG_RIB, "Handling CDAP Message: %s", opcodeNamesTable[pxDecodeCdap->eOpCode]);
    /* Looking for an App Connection using the N-1 Flow Port */
    pxAppConnectionTmp = pxRibdFindAppConnection(xN1FlowPortId);

    // vPrintAppConnection(pxAppConnectionTmp);

    /* Looking for the object into the RIB */
    pxRibObject = pxRibFindObject(pxDecodeCdap->pcObjName);

    /*if (!pxRibObject)
    {
        return pdFALSE;
    }*/

    switch (pxDecodeCdap->eOpCode)
    {
    case M_CONNECT:
        /* 1. Check AppConnection status(If it does not exist, create a new one)
        * under the status eCONNECTION_IN_PROGRESS), else put the current
        appConnection under the status eCONNECTION_IN_PROGRESS
        2. Call to the Enrollment Handler Connect request */
        /*If App Connection is not registered, create a new one*/
        if (pxAppConnectionTmp == NULL)
        {
            /*Check that the OpCode is a M_Connect*/
            if (pxDecodeCdap->eOpCode != M_CONNECT)
            {
                ESP_LOGE(TAG_RIB, "Error wrong opCode in cdap PDU");
                vPortFree(pxDecodeCdap);
            }

            // TODO: Create Connection and add into the table
            // Call to the xEnrollmentHandleConnect
        }

        break;

    case M_CONNECT_R:
        /**
         * @brief 1. Update the pxAppconnection status to eCONNECTED
         * 2. Call to the Enrollment Handle ConnectR
         */
        /* Check if the current AppConnection status is in progress */
        if (pxAppConnectionTmp->xStatus != eCONNECTION_IN_PROGRESS)
        {
            ESP_LOGE(TAG_RIB, "Invalid AppConnection State");
            return pdTRUE;
        }

        /*Update Connection Status*/
        pxAppConnectionTmp->pxDestinationInfo->pcProcessName = pxDecodeCdap->pxSourceInfo->pcProcessName;
        pxAppConnectionTmp->xStatus = eCONNECTED;
        ESP_LOGI(TAG_RIB, "Application Connection Status Updated to 'CONNECTED'");

        /*Call to Enrollment Handle ConnectR*/
        xEnrollmentHandleConnectR(pxData, pxAppConnectionTmp->pxDestinationInfo->pcProcessName, xN1FlowPortId);

        break;

    case M_RELEASE_R:
        /**
         * @brief 1. Update the pxAppconnection status to eRELEASED
         *
         */

        if (pxAppConnectionTmp->xStatus != eRELEASED)
        {
            ESP_LOGE(TAG_RIB, "The connection is already released");
            return pdTRUE;
        }
        /*Update Connection Status*/
        pxAppConnectionTmp->xStatus = eRELEASED;
        ESP_LOGI(TAG_RIB, "Status Updated Released");

        // TODO: delete App connection from the table (APPConnection)

        break;

    case M_CREATE:
        /* Calling the Create function of the enrollment object to create a Neighbor object and
         * add into the RibObject table
         */

        pxRibCreateObject(pxDecodeCdap->pcObjName, pxDecodeCdap->objInst, pxDecodeCdap->pcObjName, pxDecodeCdap->pcObjClass, ENROLLMENT);
        break;

    case M_STOP:
        // configASSERT(pxRibObject != NULL);
        ESP_LOGI(TAG_RIB, "Preparing a M_STOP_R");
        pxRibObject->pxObjOps->stop(pxRibObject, pxDecodeCdap->pxObjValue, pxAppConnectionTmp->pxDestinationInfo->pcProcessName,
                                    pxAppConnectionTmp->pxSourceInfo->pcProcessName, pxDecodeCdap->invokeID, xN1FlowPortId);
        break;

    case M_START:

        // configASSERT(pxRibObject != NULL);
        // configASSERT(pxDecodeCdap->pxObjValue != NULL);
        pxRibObject->pxObjOps->start(pxRibObject, pxDecodeCdap->pxObjValue, pxAppConnectionTmp->pxDestinationInfo->pcProcessName,
                                     pxAppConnectionTmp->pxSourceInfo->pcProcessName, pxDecodeCdap->invokeID, xN1FlowPortId);
        break;

    case M_START_R:
        /* Looking for a pending request */
        pxCallback = pxRibdFindPendingResponseHandler(pxDecodeCdap->invokeID);

        pxCallback->start_response(pxAppConnectionTmp->pxDestinationInfo->pcProcessName, pxDecodeCdap->pxObjValue); // No es necesario pcProcessName

        break;
    case M_STOP_R:
        /* Looking for a pending request */
        pxCallback = pxRibdFindPendingResponseHandler(pxDecodeCdap->invokeID);

        pxCallback->stop_response(pxAppConnectionTmp->pxDestinationInfo->pcProcessName); // añadir Result y no processName

        break;
    case M_CREATE_R:
        pxCallback = pxRibdFindPendingResponseHandler(pxDecodeCdap->invokeID);
        pxCallback->create_response(pxDecodeCdap->pxObjValue, pxDecodeCdap->result);

        break;
    // for testing purposes
    case M_WRITE:

        // send a message to the IPCP task a release a FlowAllocationRequest
        ESP_LOGE(TAG_RIB, "-------Handling M_WRITE----------");
        if (n != 0)
        {
            RINAStackEvent_t xStackFlowAllocateEvent = {eStackFlowAllocateEvent, NULL};
            flowAllocateHandle_t *pxFlowAllocateRequest;
            name_t *pxDIFName, *pxLocalName, *pxRemoteName;

            pxFlowAllocateRequest = pvPortMalloc(sizeof(*pxFlowAllocateRequest));
            (void)memset(pxFlowAllocateRequest, 0, sizeof(*pxFlowAllocateRequest));

            pxDIFName = pvPortMalloc(sizeof(*pxDIFName));
            (void)memset(pxDIFName, 0, sizeof(*pxDIFName));
            pxLocalName = pvPortMalloc(sizeof(*pxLocalName));
            (void)memset(pxLocalName, 0, sizeof(*pxLocalName));
            pxRemoteName = pvPortMalloc(sizeof(*pxRemoteName));
            (void)memset(pxRemoteName, 0, sizeof(*pxRemoteName));

            pxDIFName->pcProcessName = "irati";
            pxLocalName->pcProcessName = "Test";
            pxRemoteName->pcProcessName = "ar1.mobile";
            pxFlowAllocateRequest->pxDifName = pxDIFName;
            pxFlowAllocateRequest->pxLocal = pxLocalName;
            pxFlowAllocateRequest->pxRemote = pxRemoteName;
            pxFlowAllocateRequest->xPortId = 51;

            ESP_LOGE(TAG_RIB, "Pointer pxLocalName:%p", pxLocalName);
            ESP_LOGE(TAG_RIB, "Pointer pxRemoteName:%p", pxRemoteName);

            xStackFlowAllocateEvent.pvData = pxFlowAllocateRequest;

            if (xSendEventStructToIPCPTask(&xStackFlowAllocateEvent, (TickType_t)0U) == pdFAIL)
            {
                ESP_LOGE(TAG_RINA, "IPCP Task not working properly");
                // return -1;
            }
        }
        n = 1;

        break;

    default:
        ret = pdFALSE;
        break;
    }

    return ret;
}

BaseType_t xRibdSendResponse(string_t pcObjClass, string_t pcObjName, long objInst,
                             int result, string_t pcResultReason,
                             opCode_t eOpCode, int invokeId, portId_t xN1Port,
                             serObjectValue_t *pxObjVal)
{
    messageCdap_t *pxMsgCdap = NULL;
    NetworkBufferDescriptor_t *pxNetworkBuffer;

    switch (eOpCode)
    {

    case M_START_R:
        pxMsgCdap = prvRibdFillEnrollMsgStart(pcObjClass, pcObjName, objInst, eOpCode, pxObjVal,
                                              result, pcResultReason, invokeId);
    case M_STOP_R:
        ESP_LOGE(TAG_RIB, "FLAG1");
        pxMsgCdap = prvRibdFillEnrollMsgStop(pcObjClass, pcObjName, objInst, eOpCode, pxObjVal,
                                             result, pcResultReason, invokeId);
        break;

    default:
        break;
    }

    /* Generate and Encode Message M_CONNECT*/
    pxNetworkBuffer = prvRibdEncodeCDAP(pxMsgCdap);

    if (!pxNetworkBuffer)
    {
        ESP_LOGE(TAG_RINA, "Error encoding CDAP message");
        return pdFALSE;
    }

    /*Sent to the IPCP task*/
    vRibdSentCdapMsg(pxNetworkBuffer, xN1Port);

    return pdTRUE;
}

BaseType_t xRibdSendRequest(struct ipcpNormalData_t *pxIpcpData, string_t pcObjClass, string_t pcObjName, long objInst,
                            opCode_t eOpCode, portId_t xN1flowPortId, serObjectValue_t *pxObjVal)
{
    messageCdap_t *pxMsgCdap = NULL;
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    struct ribCallbackOps_t *pxCb = NULL;

    switch (eOpCode)
    {
    case M_START:
        pxMsgCdap = prvRibdFillEnrollMsg(pcObjClass, pcObjName, objInst, eOpCode, pxObjVal);

        break;

    case M_STOP:
        pxMsgCdap = prvRibdFillEnrollMsg(pcObjClass, pcObjName, objInst, eOpCode, pxObjVal);

        break;

    case M_CREATE:
        pxMsgCdap = prvRibdFillMsgCreate(pcObjClass, pcObjName, objInst, pxObjVal);
        break;

    default:
        ESP_LOGE(TAG_RIB, "Can't process request with mesg type %s", opcodeNamesTable[eOpCode]);
        return pdFALSE;

        break;
    }

    pxCb = pxRibdCreateCdapCallback(eOpCode, pxMsgCdap->invokeID);
    vRibdAddResponseHandler(pxMsgCdap->invokeID, pxCb);

    /* Generate and Encode Message M_CONNECT*/
    pxNetworkBuffer = prvRibdEncodeCDAP(pxMsgCdap);

    if (!pxNetworkBuffer)
    {
        ESP_LOGE(TAG_RINA, "Error encoding CDAP message");
        return pdFALSE;
    }

    /*Sent to the IPCP task*/
    vRibdSentCdapMsg(pxNetworkBuffer, xN1flowPortId);

    return pdTRUE;
}

struct ribCallbackOps_t *pxRibdCreateCdapCallback(opCode_t xOpCode, int invoke_id)
{
    struct ribCallbackOps_t *pxCallback = pvPortMalloc(sizeof(*pxCallback));

    switch (xOpCode)
    {
    case M_START:
        pxCallback->start_response = xEnrollmentHandleStartR;
        break;

    case M_STOP:
        pxCallback->stop_response = xEnrollmentHandleStopR;
        break;

    case M_CREATE:
        /*FLow Allocator*/
        pxCallback->create_response = xFlowAllocatorHandleCreateR;

        break;

    default:

        break;
    }

    return pxCallback;
}
