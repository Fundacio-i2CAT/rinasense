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
#include "RIBDaemon.h"

#include "esp_log.h"

/* Table to manage the app connections */
appConnectionTableRow_t xAppConnectionTable[APP_CONNECTION_TABLE_SIZE];


/* Encode the CDAP message */
BaseType_t prvRIBDaemonEncodeCDAP(NetworkBufferDescriptor_t *pxNetworkBuffer, rina_messages_opCode_t xMessageOpCode,
                                int64_t version, name_t *pxSource, name_t *pxDest, authPolicy_t auth);

/* Decode the CDAP message */
rina_messages_opCode_t  xRIBDaemonDecodeCDAP(uint8_t * pucBuffer, size_t xMessageLength);

BaseType_t xRIBDaemonFindAppConnection(portId_t xPortId);


BaseType_t prvRIBDaemonEncodeCDAP(NetworkBufferDescriptor_t *pxNetworkBuffer, rina_messages_opCode_t xMessageOpCode,
                                int64_t version, name_t *pxSource, name_t *pxDest, authPolicy_t auth)
{
    BaseType_t status;
    uint8_t * pucBuffer[128];
    size_t xMessageLength;

    /*Allocate space on the Stack to store the message data*/
    rina_messages_CDAPMessage message = rina_messages_CDAPMessage_init_zero;

    /*Create a stream that will write to the buffer*/
    pb_ostream_t stream = pb_ostream_from_buffer(pucBuffer, sizeof(pucBuffer));

    /*Fill the message properly*/

     
    message.opCode = xMessageOpCode;
    message.version = version;
    message.authPolicy = auth;


    message.srcApName = pxSource->pcProcessName;
    message.srcApInst = pxSource->pcProcessInstance;
    message.srcAEName = MANAGEMENT_AE;

    message.destApName = pxDest->pcProcessName;
    message.destApInst = pxDest->pcProcessInstance;
    message.destAEName = MANAGEMENT_AE;


    /*Encode the message*/
    status = pb_encode(&stream, rina_messages_CDAPMessage_fields, &message);
    xMessageLength = stream.bytes_written;

    if (!status)
    {
        ESP_LOGE(TAG_RINA, "Encoding failed: %s", PB_GET_ERROR(&stream));
        return pdFALSE;
    }

    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(xMessageLength, (TickType_t)0U); //sizeof length Message.
    memcpy(pxNetworkBuffer->pucEthernetBuffer, pucBuffer, xMessageLength);

    pxNetworkBuffer->xDataLength = xMessageLength;


    return pdTRUE;

}

rina_messages_opCode_t  xRIBDaemonDecodeCDAP(uint8_t * pucBuffer, size_t xMessageLength)
{

 BaseType_t status;
    /*Allocate space for the decode message data*/
    rina_messages_CDAPMessage message = rina_messages_CDAPMessage_init_zero;

    /*Create a stream that will read from the buffer*/
    pb_istream_t stream = pb_istream_from_buffer(pucBuffer, xMessageLength);




    /*Encode the message*/
    status = pb_decode(&stream, rina_messages_CDAPMessage_fields, &message);


    if (!status)
    {
        ESP_LOGE(TAG_RINA, "Decoding failed: %s", PB_GET_ERROR(&stream));
        return -1;
    }
    return message.opCode;

}

BaseType_t xRIBDaemonFindAppConnection(portId_t xPortId)
{

    BaseType_t x = 0;

    for (x = 0; x < APP_CONNECTION_TABLE_SIZE; x++)

    {
        if (xAppConnectionTable[x].xValid == pdTRUE)
        {
            if (xAppConnectionTable[x].xN1portId == xPortId)
            {
                //ESP_LOGI(TAG_IPCPMANAGER, "Instance founded '%p'", xInstanceTable [ x ].pxIpcpInstance);
                return pdTRUE;
                break;
            }
        }
    }
    return pdFALSE;
}

BaseType_t xRIBDaemonConnectToIpcp( name_t *pxSource, name_t * pxDestInfo, portId_t xN1flowPortId, authPolicy_t auth)
{
    /*Check for app_connections*/
    appConnection_t * pxAppConnectionTmp;
    uint64_t ucVersion = 0x01;

    RINAStackEvent_t xSendMgmtEvent = {eSendMgmtEvent, NULL};
	const TickType_t xDontBlock = pdMS_TO_TICKS(50);
  
    struct du_t *pxMessagePDU;
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    size_t xBufferSize;
    rina_messages_opCode_t xMessageOpCode = rina_messages_opCode_t_M_CONNECT;

    pxNetworkBuffer = pvPortMalloc(sizeof(*pxNetworkBuffer));


    pxAppConnectionTmp = pvPortMalloc(sizeof(pxAppConnectionTmp));
    if(!xRIBDaemonFindAppConnection(xN1flowPortId))
    {
        /*Fill the appConnection structure*/ 


        /* Generate and Encode Message M_CONNECT*/
        if (! prvRIBDaemonEncodeCDAP(pxNetworkBuffer,xMessageOpCode, ucVersion, pxSource,
                               pxDestInfo, auth ))
        {
            ESP_LOGE(TAG_RINA, "Error encoding CDAP message");
            return pdFALSE;
        }

        /* Fill the DU with PDU type (layer management)*/
        pxMessagePDU = pvPortMalloc(sizeof(*pxMessagePDU));
        pxMessagePDU->pxNetworkBuffer = pxNetworkBuffer;
        //pxMessagePDU->pxPci->xType = ;

        xSendMgmtEvent.pvData = (void *)(pxMessagePDU);
		xSendEventStructToIPCPTask(&xSendMgmtEvent, xDontBlock);

        return pdTRUE;

    }

    return pdFALSE;




}