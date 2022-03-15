#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "Freertos/FreeRTOS.h"
#include "Freertos/task.h"
#include "Freertos/queue.h"
#include "Freertos/semphr.h"

#include "IPCP.h"
#include "common.h"
#include "configRINA.h"
#include "BufferManagement.h"
#include "CDAP.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Ribd.h"

#include "esp_log.h"



/* Table to manage the app connections */
appConnectionTableRow_t xAppConnectionTable[APP_CONNECTION_TABLE_SIZE];

/* Encode the CDAP message */
NetworkBufferDescriptor_t *prvRibdeCDAP(rina_messages_opCode_t xMessageOpCode,
                                        name_t *pxSrcInfo, name_t *pxDestInfo,
                                        int64_t version, authPolicy_t *pxAuth);

/* Decode the CDAP message */
BaseType_t xRibdeCDAP(uint8_t *pucBuffer, size_t xMessageLength, messageCdap_t *pxMessageCdap);

messageCdap_t *prvRibdFillDecodeMessage(rina_messages_CDAPMessage message);

BaseType_t xRibdppConnection(portId_t xPortId);

void vRibdAddInstanceEntry(appConnection_t *pxAppConnectionToAdd, portId_t xPortId)
{

    BaseType_t x = 0;

    for (x = 0; x < APP_CONNECTION_TABLE_SIZE; x++)
    {
        if (xAppConnectionTable[x].xValid == pdFALSE)
        {
            xAppConnectionTable[x].pxAppConnection = pxAppConnectionToAdd;
            xAppConnectionTable[x].xN1portId = xPortId;
            xAppConnectionTable[x].xValid = pdTRUE;
            ESP_LOGE(TAG_IPCPMANAGER, "AppConnection Entry successful: %p,id:%d", pxAppConnectionToAdd, xPortId);

            break;
        }
    }
}

appConnectionTableRow_t *pvFindLastAppConnRow(void);
appConnectionTableRow_t *pvFindLastAppConnRow(void)
{
    appConnectionTableRow_t *xTmp;

    BaseType_t x = 0;

    xTmp = pvPortMalloc(sizeof(xTmp));

    for (x = 0; x < APP_CONNECTION_TABLE_SIZE; x++)

    {
        if (xAppConnectionTable[x].xValid == pdTRUE)
        {
            if (xAppConnectionTable[x].pxAppConnection->xStatus == eCONNECTION_INIT)
            {
                // ESP_LOGI(TAG_IPCPMANAGER, "Instance founded '%p'", xInstanceTable [ x ].pxIpcpInstance);
                xTmp = &xAppConnectionTable[x];
                return xTmp;
                break;
            }
        }
    }
    return NULL;
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

    pxMessage->pxDestinationInfo = pvPortMalloc(sizeof(name_t *));
    pxMessage->pxSourceInfo = pvPortMalloc(sizeof(name_t *));
    pxMessage->pxAuthPolicy = pvPortMalloc(sizeof(authPolicy_t *));

    /* Init to Default Values*/

    pxMessage->version = 1;
    pxMessage->eOpCode = -1; // No code
    pxMessage->invokeID = 1; // by default

    pxMessage->pxDestinationInfo->pcEntityInstance = MANAGEMENT_AE;
    pxMessage->pxDestinationInfo->pcEntityName = NULL;
    pxMessage->pxDestinationInfo->pcProcessInstance = NULL;
    pxMessage->pxDestinationInfo->pcProcessName = NULL;

    pxMessage->pxSourceInfo->pcEntityInstance = MANAGEMENT_AE;
    pxMessage->pxSourceInfo->pcEntityName = NULL;
    pxMessage->pxSourceInfo->pcProcessInstance = NULL;
    pxMessage->pxSourceInfo->pcProcessName = NULL;

    pxMessage->pxAuthPolicy->pcName = NULL;
    pxMessage->pxAuthPolicy->pcVersion = NULL;

    return pxMessage;
}


messageCdap_t *prvRibdFillDecodeMessage(rina_messages_CDAPMessage message)
{
    messageCdap_t *pxMessageCdap;

    pxMessageCdap = pvPortMalloc(sizeof(*pxMessageCdap));
   
    pxMessageCdap->pxDestinationInfo = pvPortMalloc(sizeof(name_t *));
    pxMessageCdap->pxSourceInfo = pvPortMalloc(sizeof(name_t *));
    pxMessageCdap->pxAuthPolicy = pvPortMalloc(sizeof(authPolicy_t *));

    pxMessageCdap->eOpCode = message.opCode;
    pxMessageCdap->version = message.version;
    pxMessageCdap->invokeID = message.invokeID;

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
    }

    return pxMessageCdap;
}

rina_messages_CDAPMessage prvRibdFillEncodeMessage(messageCdap_t *pxMessageCdap)
{

    /*Allocate space on the Stack to store the message data*/
    rina_messages_CDAPMessage message = rina_messages_CDAPMessage_init_zero;

    message.version = pxMessageCdap->version;
    message.has_version = true;

    message.opCode = pxMessageCdap->eOpCode;

    message.invokeID = pxMessageCdap->invokeID;
    message.has_invokeID = true;

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
        message.authPolicy.has_name = true;
    }

    return message;
}

NetworkBufferDescriptor_t *prvRibdEncodeCDAP(messageCdap_t *pxMessageCdap)
{
    BaseType_t status;
    uint8_t *pucBuffer[128];
    size_t xMessageLength;

    // rina_messages_CDAPMessage message =
    /*Create a stream that will write to the buffer*/
    pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)pucBuffer, sizeof(pucBuffer));

    /*Fill the message properly*/
    rina_messages_CDAPMessage message = prvRibdFillEncodeMessage(pxMessageCdap);

    /*Encode the message*/
    status = pb_encode(&stream, rina_messages_CDAPMessage_fields, &message);
    xMessageLength = stream.bytes_written;

    if (!status)
    {
        ESP_LOGE(TAG_RINA, "Encoding failed: %s", PB_GET_ERROR(&stream));
        return NULL;
    }

    ESP_LOGI(TAG_RINA, "Message CDAP length: %d", xMessageLength);

    /*Request a Network Buffer according to Message Length*/
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(xMessageLength, (TickType_t)0U);

    /*Copy Buffer into the NetworkBuffer*/
    // TODO: Check if just assign pointer is ok.
    memcpy(pxNetworkBuffer->pucEthernetBuffer, &pucBuffer, xMessageLength);

    pxNetworkBuffer->xDataLength = xMessageLength;

    return pxNetworkBuffer;
}

messageCdap_t *prvRibdDecodeCDAP(uint8_t *pucBuffer, size_t xMessageLength)
{

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

    return prvRibdFillDecodeMessage(message);
}

BaseType_t xRibdFindAppConnection(portId_t xPortId)
{

    BaseType_t x = 0;

    for (x = 0; x < APP_CONNECTION_TABLE_SIZE; x++)

    {
        if (xAppConnectionTable[x].xValid == pdTRUE)
        {
            if (xAppConnectionTable[x].xN1portId == xPortId)
            {
                ESP_LOGI(TAG_IPCPMANAGER, "AppConnection founded '%p'", xAppConnectionTable[x].pxAppConnection);
                return pdTRUE;
                break;
            }
        }
    }
    return pdFALSE;
}
BaseType_t xRibdConnectToIpcp(name_t *pxSource, name_t *pxDestInfo, portId_t xN1flowPortId, authPolicy_t *pxAuth);
void vRibdPrintDecodeCdap(messageCdap_t *pxDecodeCdap);

BaseType_t xRibdConnectToIpcp(name_t *pxSource, name_t *pxDestInfo, portId_t xN1flowPortId, authPolicy_t *pxAuth)
{

    RINAStackEvent_t xSendMgmtEvent = {eSendMgmtEvent, NULL};
    const TickType_t xDontBlock = pdMS_TO_TICKS(50);

    /*Check for app_connections*/
    appConnection_t *pxAppConnectionTmp;
    int64_t version = 1;

    struct du_t *pxMessagePDU;
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    size_t xBufferSize;

    rina_messages_opCode_t xMessageOpCode = rina_messages_opCode_t_M_CONNECT;

    /*Testing*/
    messageCdap_t *pxMessageDecode;
    pxMessageDecode = pvPortMalloc(sizeof(*pxMessageDecode));

    messageCdap_t *pxMessageEncode = prvRibMessageCdapInit();

    /*Fill the Message to be encoded in the connection*/
    pxMessageEncode->eOpCode = xMessageOpCode;
    pxMessageEncode->pxDestinationInfo->pcEntityName = strdup(pxDestInfo->pcEntityName);
    pxMessageEncode->pxDestinationInfo->pcProcessInstance = strdup(pxDestInfo->pcProcessInstance);
    pxMessageEncode->pxDestinationInfo->pcProcessName = strdup(pxDestInfo->pcProcessName);

    pxMessageEncode->pxSourceInfo->pcEntityName = strdup(pxSource->pcEntityName);
    pxMessageEncode->pxSourceInfo->pcProcessInstance = strdup(pxSource->pcProcessInstance);
    pxMessageEncode->pxSourceInfo->pcProcessName = strdup(pxSource->pcProcessName);

    pxMessageEncode->pxAuthPolicy->pcName = strdup(pxAuth->pcName);
    pxMessageEncode->pxAuthPolicy->pcVersion = strdup(pxAuth->pcVersion);

    // printf("opCode: %d\n", pxAuth->pcName);

    printf("ENCODE\n");
    printf("opCode: %d\n", pxMessageEncode->eOpCode);
    printf("Invoke Id: %d\n ", pxMessageEncode->invokeID);
    printf("Version: %lld\n", pxMessageEncode->version);
    printf("AuthPolicy Name: %s\n", (char *)pxMessageEncode->pxAuthPolicy->pcName);
    printf("Source AEI: %s\n", (char *)pxMessageEncode->pxSourceInfo->pcEntityInstance);
    printf("Source AEN: %s\n", pxMessageEncode->pxSourceInfo->pcEntityName);
    printf("Source API: %s\n", pxMessageEncode->pxSourceInfo->pcProcessInstance);
    printf("Source APN: %s\n", pxMessageEncode->pxSourceInfo->pcProcessName);
    printf("Dest AEI: %s\n", pxMessageEncode->pxDestinationInfo->pcEntityInstance);
    printf("Dest AEN: %s\n", pxMessageEncode->pxDestinationInfo->pcEntityName);
    printf("Dest API: %s\n", pxMessageEncode->pxDestinationInfo->pcProcessInstance);
    printf("Dest APN: %s\n", pxMessageEncode->pxDestinationInfo->pcProcessName);

    /*Fill the appConnection structure*/
    // TODO complete the fill function and add request to the table.

    /* Generate and Encode Message M_CONNECT*/
    pxNetworkBuffer = prvRibdEncodeCDAP(pxMessageEncode);
    if (!pxNetworkBuffer)
    {
        ESP_LOGE(TAG_RINA, "Error encoding CDAP message");
        return pdFALSE;
    }

    /*Testing*/
    pxMessageDecode = prvRibdDecodeCDAP(pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength);
    // vRibdPrintDecodeCdap( pxMessageDecode );

    /* Fill the DU with PDU type (layer management)*/
    pxMessagePDU = pvPortMalloc(sizeof(*pxMessagePDU));
    pxMessagePDU->pxNetworkBuffer = pxNetworkBuffer;
    pxMessagePDU->pxNetworkBuffer->ulBoundPort = xN1flowPortId;

    xSendMgmtEvent.pvData = (void *)(pxMessagePDU);
    xSendEventStructToIPCPTask(&xSendMgmtEvent, xDontBlock);

    return pdTRUE;
}

void vRibdPrintDecodeCdap(messageCdap_t *pxDecodeCdap)
{
    printf("DECODE");
    printf("opCode: %d\n", pxDecodeCdap->eOpCode);
    printf("Invoke Id: %d\n ", pxDecodeCdap->invokeID);
    printf("Version: %lld\n", pxDecodeCdap->version);
    if (!pxDecodeCdap->pxAuthPolicy->pcName)
        printf("AuthPolicy Name: %s\n", pxDecodeCdap->pxAuthPolicy->pcName);
    printf("Source AEI: %s\n", pxDecodeCdap->pxSourceInfo->pcEntityInstance);
    printf("Source AEN: %s\n", pxDecodeCdap->pxSourceInfo->pcEntityName);
    printf("Source API: %s\n", pxDecodeCdap->pxSourceInfo->pcProcessInstance);
    printf("Source APN: %s\n", pxDecodeCdap->pxSourceInfo->pcProcessName);
    printf("Dest AEI: %s\n", pxDecodeCdap->pxDestinationInfo->pcEntityInstance);
    printf("Dest AEN: %s\n", pxDecodeCdap->pxDestinationInfo->pcEntityName);
    printf("Dest API: %s\n", pxDecodeCdap->pxDestinationInfo->pcProcessInstance);
    printf("Dest APN: %s\n", pxDecodeCdap->pxDestinationInfo->pcProcessName);
}

BaseType_t xRibdProcessLayerManagementPDU(struct ipcpInstanceData_t *pxData, portId_t xN1flowPortId, struct du_t *pxDu);
BaseType_t xRibdProcessLayerManagementPDU(struct ipcpInstanceData_t *pxData, portId_t xN1flowPortId, struct du_t *pxDu)
{
    /*Struct parallel CDAP message*/
    messageCdap_t *pxDecodeCdap;

    pxDecodeCdap = prvRibdDecodeCDAP(pxDu->pxNetworkBuffer->pucEthernetBuffer, pxDu->pxNetworkBuffer->xDataLength);

    /*Decode Message*/
    if (!pxDecodeCdap)
    {
        ESP_LOGE(TAG_RINA, "Error decoding CDAP message");
        return pdFALSE;
    }

    /*Update Table Connections*/

    /*Call to enrollment task*/
    return pdTRUE;
}