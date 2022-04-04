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
#include "configSensor.h"
#include "Rib.h"

#include "esp_log.h"

/* Table to manage the app connections */
appConnectionTableRow_t xAppConnectionTable[APP_CONNECTION_TABLE_SIZE];

/* Table to manage the pending request to response*/
responseHandlersRow_t xPendingResponseHandlersTable[ RESPONSE_HANDLER_TABLE_SIZE ];

/* Encode the CDAP message */
NetworkBufferDescriptor_t *prvRibEncodeCDAP(rina_messages_opCode_t xMessageOpCode,
                                            name_t *pxSrcInfo, name_t *pxDestInfo,
                                            int64_t version, authPolicy_t *pxAuth);

/* Decode the CDAP message */
BaseType_t xRibdecodeCDAP(uint8_t *pucBuffer, size_t xMessageLength, messageCdap_t *pxMessageCdap);

messageCdap_t *prvRibdFillDecodeMessage(rina_messages_CDAPMessage message);

BaseType_t xRibdppConnection(portId_t xPortId);

BaseType_t vRibHandleMessage(messageCdap_t *pxDecodeCdap, portId_t xN1FlowPortId);
BaseType_t xRibdProcessLayerManagementPDU(struct ipcpInstanceData_t *pxData, portId_t xN1flowPortId, struct du_t *pxDu);

void vRibdAddResponseHandler(int32_t invokeID, struct ribCallbackOps_t *pxCb )
{

    BaseType_t x = 0;

    for (x = 0; x < RESPONSE_HANDLER_TABLE_SIZE ; x++)
    {
        if (xPendingResponseHandlersTable[x].xValid == pdFALSE)
        {
            xPendingResponseHandlersTable[x].invokeID = invokeID;
            xPendingResponseHandlersTable[x].xValid = pdTRUE;
            xPendingResponseHandlersTable[x].pxCallbackHandler = pxCb;

            ESP_LOGE(TAG_RIB, "Pending Handlers Entry successful: %p", pxCb);

            break;
        }
    }
}

struct ribCallbackOps_t *pxRibdFindPendingResponseHandler(int32_t invokeID)
{

    BaseType_t x = 0;
    struct ribCallbackOps_t *pxCb;
    pxCb = pvPortMalloc(sizeof(*pxCb));

    for (x = 0; x < RESPONSE_HANDLER_TABLE_SIZE; x++)

    {
        if (  xPendingResponseHandlersTable[x].xValid == pdTRUE)
        {
            
            if ( xPendingResponseHandlersTable[x].invokeID == invokeID)
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

    pxMessage->version = 0x01;
    pxMessage->eOpCode = -1; // No code
    pxMessage->invokeID = 1; // by default
    pxMessage->objInst = -1;
    pxMessage->xObjClass = NULL;
    pxMessage->xObjName = NULL;
    pxMessage->pxObjValue = NULL;

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
        pxMessageCdap->pxAuthPolicy->pcVersion = strdup(message.authPolicy.versions);
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
    if (pxMessageCdap->xObjClass != NULL)
    {
        strcpy(message.objClass, pxMessageCdap->xObjClass);
        message.has_objClass = true;
    }

    if (pxMessageCdap->xObjName != NULL)
    {
        strcpy(message.objName, pxMessageCdap->xObjName);
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
    BaseType_t status;
    uint8_t *pucBuffer[128];
    size_t xMessageLength;

    // rina_messages_CDAPMessage message =
    /*Create a stream that will write to the buffer*/
    pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)pucBuffer, sizeof(pucBuffer));

    /*Fill the message properly*/
    rina_messages_CDAPMessage message = prvRibdSerToRinaMessage(pxMessageCdap);

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

appConnection_t *pxRibdFindAppConnection(portId_t xPortId)
{

    BaseType_t x = 0;
    appConnection_t *pxAppConnection;
    pxAppConnection = pvPortMalloc(sizeof(*pxAppConnection));

    for (x = 0; x < APP_CONNECTION_TABLE_SIZE; x++)

    {
        if (xAppConnectionTable[x].xValid == pdTRUE)
        {
            if (xAppConnectionTable[x].xN1portId == xPortId)
            {
                ESP_LOGI(TAG_IPCPMANAGER, "AppConnection founded '%p'", xAppConnectionTable[x].pxAppConnection);
                pxAppConnection = xAppConnectionTable[x].pxAppConnection;

                return pxAppConnection;
                break;
            }
        }
    }
    return NULL;
}
BaseType_t xRibdConnectToIpcp(name_t *pxSource, name_t *pxDestInfo, portId_t xN1flowPortId, authPolicy_t *pxAuth);
void vRibdPrintCdapMessage(messageCdap_t *pxDecodeCdap);

appConnection_t *prvRibCreateConnection(name_t *pxSource, name_t *pxDestInfo)

{
    appConnection_t *pxAppConnectionTmp;
    pxAppConnectionTmp = pvPortMalloc(sizeof(*pxAppConnectionTmp));
    pxAppConnectionTmp->pxDestinationInfo = pvPortMalloc(sizeof(name_t *));
    pxAppConnectionTmp->pxSourceInfo = pvPortMalloc(sizeof(name_t *));

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
    ESP_LOGE(TAG_RIB, "Sendd CDAP MSG");
    RINAStackEvent_t xSendMgmtEvent = {eSendMgmtEvent, NULL};
    const TickType_t xDontBlock = pdMS_TO_TICKS(50);
    struct du_t *pxMessagePDU;

    /* Fill the DU with PDU type (layer management)*/
    pxMessagePDU = pvPortMalloc(sizeof(*pxMessagePDU));
    pxMessagePDU->pxNetworkBuffer = pxNetworkBuffer;
    pxMessagePDU->pxNetworkBuffer->ulBoundPort = xN1FlowPortId;

    xSendMgmtEvent.pvData = (void *)(pxMessagePDU);
    xSendEventStructToIPCPTask(&xSendMgmtEvent, xDontBlock);
    ESP_LOGE(TAG_RIB, "Sendd CDAP MSG STRUCT to IPCPTask");
}

messageCdap_t *prvRibdFillEnrollMsg(string_t xObjClass, string_t xObjName, long objInst, opCode_t eOpCode,
                                    serObjectValue_t *pxObjValue)
{
    messageCdap_t *pxMessage = prvRibMessageCdapInit();

    pxMessage->invokeID = get_next_invoke_id(); //???
    pxMessage->eOpCode = eOpCode;
    pxMessage->objInst = objInst;

    if (xObjClass != NULL)
    {
        pxMessage->xObjClass = strdup(xObjClass);
    }

    if (xObjName != NULL)
    {
        pxMessage->xObjName = strdup(xObjName);
    }
    // Check
    if (pxObjValue != NULL)
    {
        pxMessage->pxObjValue = (void *)(pxObjValue);
    }

    return pxMessage;
}

BaseType_t xRibdConnectToIpcp(name_t *pxSource, name_t *pxDestInfo, portId_t xN1flowPortId, authPolicy_t *pxAuth)
{

    /*Check for app_connections*/
    appConnection_t *pxAppConnectionTmp;

    NetworkBufferDescriptor_t *pxNetworkBuffer;
    size_t xBufferSize;

    /*Testing*/
    /* messageCdap_t *pxMessageDecode;
     pxMessageDecode = pvPortMalloc(sizeof(*pxMessageDecode));*/

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
    // TODO complete the fill function and add request to the table.
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
    /*pxMessageDecode = prvRibdDecodeCDAP(pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength);
    vRibdPrintCdapMessage( pxMessageDecode );*/

    vRibdSentCdapMsg(pxNetworkBuffer, xN1flowPortId);

    return pdTRUE;
}

void vRibdPrintCdapMessage(messageCdap_t *pxDecodeCdap)
{
    printf("DECODE");
    printf("opCode: %d\n", pxDecodeCdap->eOpCode);
    printf("Invoke Id: %d\n ", pxDecodeCdap->invokeID);
    printf("Version: %lld\n", pxDecodeCdap->version);
    if (pxDecodeCdap->pxAuthPolicy->pcName)
    {
        printf("AuthPolicy Name: %s\n", pxDecodeCdap->pxAuthPolicy->pcName);
        printf("AuthPolicy Version: %s\n", pxDecodeCdap->pxAuthPolicy->pcVersion);
    }

    printf("Source AEI: %s\n", pxDecodeCdap->pxSourceInfo->pcEntityInstance);
    printf("Source AEN: %s\n", pxDecodeCdap->pxSourceInfo->pcEntityName);
    printf("Source API: %s\n", pxDecodeCdap->pxSourceInfo->pcProcessInstance);
    printf("Source APN: %s\n", pxDecodeCdap->pxSourceInfo->pcProcessName);
    printf("Dest AEI: %s\n", pxDecodeCdap->pxDestinationInfo->pcEntityInstance);
    printf("Dest AEN: %s\n", pxDecodeCdap->pxDestinationInfo->pcEntityName);
    printf("Dest API: %s\n", pxDecodeCdap->pxDestinationInfo->pcProcessInstance);
    printf("Dest APN: %s\n", pxDecodeCdap->pxDestinationInfo->pcProcessName);
}

BaseType_t xRibdProcessLayerManagementPDU(struct ipcpInstanceData_t *pxData, portId_t xN1flowPortId, struct du_t *pxDu)
{
    /*Struct parallel CDAP message*/
    ESP_LOGE(TAG_RIB, "xRibdProcessLayerManagementPDU");
    messageCdap_t *pxDecodeCdap;

    /*Decode CDAP Message*/
    pxDecodeCdap = prvRibdDecodeCDAP(pxDu->pxNetworkBuffer->pucEthernetBuffer, pxDu->pxNetworkBuffer->xDataLength);

    if (!pxDecodeCdap)
    {
        ESP_LOGE(TAG_RINA, "Error decoding CDAP message");
        return pdFALSE;
    }

    /*Release Buffer Network*/
    vReleaseNetworkBuffer(pxDu->pxNetworkBuffer->pucEthernetBuffer);

    /*Call to rib Handle Message*/
    ESP_LOGE(TAG_RIB, "Calling vRibHandleMessage");
    vRibHandleMessage(pxDecodeCdap, xN1flowPortId);

    return pdTRUE;
}

BaseType_t vRibHandleMessage(messageCdap_t *pxDecodeCdap, portId_t xN1FlowPortId)
{
    BaseType_t ret = pdFALSE;
    appConnection_t *pxAppConnectionTmp;
    struct ribObject_t *pxRibObject;

    /*Check if the Operation Code is valid*/
    if (pxDecodeCdap->eOpCode > MAX_CDAP_OPCODE)
    {
        ESP_LOGE(TAG_RIB, "Invalid opcode %d", pxDecodeCdap->eOpCode);
        vPortFree(pxDecodeCdap);
        return ret;
    }

    /* Looking for an App Connection using the N-1 Flow Port */
    pxAppConnectionTmp = pxRibdFindAppConnection(xN1FlowPortId);

    /* Looking for the object into the RIB */
    pxRibObject = pxRibFindObject(pxDecodeCdap->xObjName);

    /* Looking for a pending request */

    
    
    ESP_LOGE(TAG_RIB, "OpCOde: %d", pxDecodeCdap->eOpCode);
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
        pxAppConnectionTmp->xStatus = eCONNECTED;
        ESP_LOGI(TAG_RIB, "Status Updated Connected");

        /*Call to Enrollment Handle ConnectR*/
        xEnrollmentHandleConnectR(pxDecodeCdap->pxDestinationInfo->pcProcessName, xN1FlowPortId);

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
        /* Calling the Create function of the enrollment object to create a Negihbor object and
        * add into the RibObject table 
        */

       pxRibObject->pxObjOps->create(pxRibObject, pxDecodeCdap->pxObjValue,  pxAppConnectionTmp->pxDestinationInfo->pcProcessName,
            pxAppConnectionTmp->pxSourceInfo->pcProcessName,pxDecodeCdap->invokeID, xN1FlowPortId);

    case M_START_R:

        pxRibObject->pxObjOps->

        break;

    default:
        break;
    }

    return pdTRUE;
}

BaseType_t xRibdSendRequest(string_t xObjClass, string_t xObjName, long objInst,
                            opCode_t eMsgType, portId_t xN1flowPortId, serObjectValue_t *pxObjVal)
{
    messageCdap_t *pxMsgCdap = NULL;
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    struct ribCallbackOps_t *pxCb = NULL;

    switch (eMsgType)
    {
    case M_START:
        pxMsgCdap = prvRibdFillEnrollMsg(xObjClass, xObjName, objInst, eMsgType, pxObjVal);
        break;

    default:
        ESP_LOGE(TAG_RIB, "Can't process request with mesg type %d", eMsgType);
        return pdFALSE;

        break;
    }

    pxCb = pxRibdCreateCdapCallback(eMsgType, pxMsgCdap->invokeID);
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


struct ribCallbackOps_t *
pxRibdCreateCdapCallback( opCode_t xOpCode,
                          int invoke_id)
{
    struct ribCallbackOps_t *pxCallback = pvPortMalloc(sizeof(struct ribCallbackOps_t));

    // TODO nest switches in function of the msg OpCode
    switch (xOpCode) {
    case M_START:
        pxCallback->start_response = xEnrollmentHandleStartR;
        break;

    case M_STOP:
        pxCallback->stop_response = xEnrollmentHandleStopR;
        break;

    case M_CREATE:
        //pxCllback->create_response = fa_handle_create_r;
        break;

    default:

        break;
    }

    return pxCallback;
}