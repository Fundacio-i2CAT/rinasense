/*Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

/* RINA includes. */
#include "common.h"
#include "configSensor.h"
#include "configRINA.h"
#include "Ribd.h"
#include "Enrollment.h"
#include "EnrollmentInformationMessage.pb.h"
#include "NeighborMessage.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Rib.h"
#include "FlowMessage.pb.h"
#include "FlowAllocator.h"

#include "esp_log.h"

/**
 * @brief Fill the proto struct rina_messages_enrollmentInformation using the internal struct
 * enrollmentMessage_t
 * 
 * @param pxMsg  struct enrollmentMessage_t that need to encode.
 * @return rina_messages_enrollmentInformation_t 
 */
static rina_messages_enrollmentInformation_t prvSedesMsgEncodeEnroll(enrollmentMessage_t *pxMsg)
{
        rina_messages_enrollmentInformation_t msgPb = rina_messages_enrollmentInformation_t_init_zero;

        /*adding the address msg struct*/
        /****** For the moment just the address *****/
        if (pxMsg->ullAddress > 0)
        {
                msgPb.address = pxMsg->ullAddress;
                msgPb.has_address = true;
        }

        return msgPb;
}


/**
 * @brief Encode a Enrollment message and return the serialized object value to be
 * added into the CDAP message
 * 
 * @param pxMsg 
 * @return serObjectValue_t* 
 */
serObjectValue_t *pxSerdesMsgEnrollmentEncode(enrollmentMessage_t *pxMsg)
{
        BaseType_t status;
        rina_messages_enrollmentInformation_t enrollMsg = prvSedesMsgEncodeEnroll(pxMsg);

        // Allocate space on the stack to store the message data.
        void *pvBuffer = pvPortMalloc(MTU);
        int maxLength = MTU;

        // Create a stream that writes to our buffer.
        pb_ostream_t stream = pb_ostream_from_buffer(pvBuffer, maxLength);

        // Now we are ready to encode the message.
        status = pb_encode(&stream, rina_messages_enrollmentInformation_t_fields, &enrollMsg);

        // Check for errors...
        if (!status)
        {
                ESP_LOGE(TAG_ENROLLMENT, "Encoding failed: %s\n", PB_GET_ERROR(&stream));
                vPortFree(pvBuffer);
                return NULL;
        }

        serObjectValue_t *pxSerMsg = pvPortMalloc(sizeof(*pxSerMsg));
        pxSerMsg->pvSerBuffer = pvBuffer;
        pxSerMsg->xSerLength = stream.bytes_written;

        return pxSerMsg;
}

static enrollmentMessage_t * prvSerdesMsgDecodeEnrollment(rina_messages_enrollmentInformation_t message)
{
    enrollmentMessage_t * pxMsg;
    pxMsg = pvPortMalloc(sizeof(*pxMsg));

    if ( message.has_address )
    {
        pxMsg->ullAddress = message.address;
    }
    if (message.has_token)
    {
        pxMsg->pcToken = strdup(message.token);
        
    }

    return pxMsg;
}

enrollmentMessage_t *pxSerdesMsgEnrollmentDecode(uint8_t *pucBuffer, size_t xMessageLength)
{
   
    BaseType_t status;

    /*Allocate space for the decode message data*/
    rina_messages_enrollmentInformation_t message = rina_messages_enrollmentInformation_t_init_zero;

    /*Create a stream that will read from the buffer*/
    pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    status = pb_decode(&stream, rina_messages_enrollmentInformation_t_fields, &message);

    if (!status)
    {
        ESP_LOGE(TAG_RINA, "Decoding failed: %s", PB_GET_ERROR(&stream));
        return NULL;
    }

    return prvSerdesMsgDecodeEnrollment(message);
}

/**
 * @brief Fill the internal structure neighborMessage_t from the rina_messages_neighbor_t
 * It is call from the Decode Neighbor function.
 * 
 * @param message rina_messages_neighbor_t structure
 * @return neighborMessage_t* 
 */
static neighborMessage_t *prvSerdesMsgDecodeNeighbor(rina_messages_neighbor_t message)
{
    neighborMessage_t *pxMessage;
    

    pxMessage = pvPortMalloc(sizeof(*pxMessage));

    if (message.has_apname)
    {
        pxMessage->pcApName = strdup(message.apname);

    }

    if (message.has_apinstance)
    {
        pxMessage->pcApInstance = strdup(message.apinstance);
    }

    if (message.has_aename)
    {
         pxMessage->pcAeName = strdup(message.aename);
    }

    if (message.has_aeinstance)
    {
        pxMessage->pcAeInstance = strdup(message.aeinstance);
    }

    if (message.has_address)
    {
        pxMessage->ullAddress = message.address;
    }




    return pxMessage;
}




neighborMessage_t *pxserdesMsgDecodeNeighbor(uint8_t *pucBuffer, size_t xMessageLength)
{

    BaseType_t status;

    /*Allocate space for the decode message data*/
    rina_messages_neighbor_t message = rina_messages_neighbor_t_init_zero;

    /*Create a stream that will read from the buffer*/
    pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    status = pb_decode(&stream, rina_messages_neighbor_t_fields, &message);

    if (!status)
    {
        ESP_LOGE(TAG_RINA, "Decoding failed: %s", PB_GET_ERROR(&stream));
        return NULL;
    }

    return prvSerdesMsgDecodeNeighbor(message);
}


static rina_messages_neighbor_t prvSerdesMsgEncodeNeighbor(neighborMessage_t *pxMessage)
{
    rina_messages_neighbor_t message = rina_messages_neighbor_t_init_zero;

    if(pxMessage->pcAeInstance)
    {
        strcpy(message.aeinstance, pxMessage->pcAeInstance);
        message.has_aeinstance = true;
    }
        if(pxMessage->pcAeName)
    {
        strcpy(message.aename, pxMessage->pcAeName);
        message.has_aename = true;
    }
        if(pxMessage->pcApInstance)
    {
        strcpy(message.apinstance, pxMessage->pcApInstance);
        message.has_apinstance = true;
    }
        if(pxMessage->pcApName)
    {
        strcpy(message.apname, pxMessage->pcApName);
        message.has_apname = true;
    }

    return message;
}
serObjectValue_t *pxSerdesMsgNeighborEncode(neighborMessage_t *pxMsg)
{
        BaseType_t status;
        rina_messages_neighbor_t  neighborMsg = prvSerdesMsgEncodeNeighbor(pxMsg);

        // Allocate space on the stack to store the message data.
        void *pvBuffer = pvPortMalloc(MTU);
        int maxLength = MTU;

        // Create a stream that writes to our buffer.
        pb_ostream_t stream = pb_ostream_from_buffer(pvBuffer, maxLength);

        // Now we are ready to encode the message.
        status = pb_encode(&stream, rina_messages_neighbor_t_fields , &neighborMsg);

        // Check for errors...
        if (!status)
        {
                ESP_LOGE(TAG_ENROLLMENT, "Encoding failed: %s\n", PB_GET_ERROR(&stream));
                vPortFree(pvBuffer);
                return NULL;
        }

        serObjectValue_t *pxSerMsg = pvPortMalloc(sizeof(*pxSerMsg));
        pxSerMsg->pvSerBuffer = pvBuffer;
        pxSerMsg->xSerLength = stream.bytes_written;

        return pxSerMsg;
}


static rina_messages_Flow prvSerdesMsgEncodeFlow(flow_t *pxMsg)
{
    rina_messages_Flow message = rina_messages_Flow_init_zero ;

    //Fill required attributes
    strcpy(message.sourceNamingInfo.applicationProcessName,pxMsg->pxSourceInfo->pcProcessName);
    strcpy(message.destinationNamingInfo.applicationProcessName, pxMsg->pxDestInfo->pcProcessName);

    message.sourcePortId = pxMsg->xSourcePortId;
    message.sourceAddress = pxMsg->xSourceAddress;

    message.connectionIds->sourceCEPId = pxMsg->pxConnectionId->xSource;
    message.connectionIds->has_sourceCEPId = true;
    message.connectionIds->qosId = pxMsg->pxConnectionId->xQosId;
    message.connectionIds->has_qosId = true;

    message.has_qosParameters = true;
    message.qosParameters.qosid = pxMsg->pxQosSpec->xQosId;
    message.qosParameters.has_qosid = true;
    strcpy(message.qosParameters.name, pxMsg->pxQosSpec->pcQosName);
    message.qosParameters.has_name = true;
    message.qosParameters.partialDelivery = pxMsg->pxQosSpec->pxFlowSpec->xPartialDelivery;
    message.qosParameters.has_partialDelivery = true;
    message.qosParameters.order = pxMsg->pxQosSpec->pxFlowSpec->xOrderedDelivery;
    message.qosParameters.has_order = true;

    message.has_dtpConfig = true;
    strcpy( message.dtpConfig.dtppolicyset.policyName,
        pxMsg->pxDtpConfig->pxDtpPolicySet->pcPolicyName );
    message.dtpConfig.dtppolicyset.has_policyName = true;
    strcpy( message.dtpConfig.dtppolicyset.version,
        pxMsg->pxDtpConfig->pxDtpPolicySet->pcPolicyVersion );
    message.dtpConfig.dtppolicyset.has_version = true;

    message.dtpConfig.initialATimer = pxMsg->pxDtpConfig->xInitialATimer;
    message.dtpConfig.has_initialATimer = true;

    message.dtpConfig.dtcpPresent = pxMsg->pxDtpConfig->xDtcpPresent;
    message.dtpConfig.has_dtcpPresent = true;

    return message;
}
serObjectValue_t *pxSerdesMsgFlowEncode(flow_t *pxMsg)
{
        BaseType_t status;
        rina_messages_Flow flowMsg = prvSerdesMsgEncodeFlow(pxMsg);

        // Allocate space on the stack to store the message data.
        void *pvBuffer = pvPortMalloc(MTU);
        int maxLength = MTU;

        // Create a stream that writes to our buffer.
        pb_ostream_t stream = pb_ostream_from_buffer(pvBuffer, maxLength);

        // Now we are ready to encode the message.
        status = pb_encode(&stream, rina_messages_Flow_fields , &flowMsg);

        // Check for errors...
        if (!status)
        {
                ESP_LOGE(TAG_ENROLLMENT, "Encoding failed: %s\n", PB_GET_ERROR(&stream));
                vPortFree(pvBuffer);
                return NULL;
        }

        serObjectValue_t *pxSerMsg = pvPortMalloc(sizeof(*pxSerMsg));
        pxSerMsg->pvSerBuffer = pvBuffer;
        pxSerMsg->xSerLength = stream.bytes_written;

        ESP_LOGE(TAG_ENROLLMENT, "Encoding Flow Message ok");

        return pxSerMsg;
}
