/*Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* RINA includes. */
#include "rina_common_port.h"
#include "configSensor.h"
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
    if (pxMsg->ullAddress > 0)
    {
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

    pxMessage = pvRsMemAlloc(sizeof(*pxMessage));

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
    bool_t status;

    /*Allocate space for the decode message data*/
    rina_messages_neighbor_t message = rina_messages_neighbor_t_init_zero;

    /*Create a stream that will read from the buffer*/
    pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    status = pb_decode(&stream, rina_messages_neighbor_t_fields, &message);

    if (!status)
    {
        LOGE(TAG_RINA, "Decoding failed: %s", PB_GET_ERROR(&stream));
        return NULL;
    }

    return prvSerdesMsgDecodeNeighbor(message);
}

static aDataMsg_t *prvSerdesMsgDecodeAData(rina_messages_a_data_t message)
{
    aDataMsg_t *pxMessage;

    pxMessage = pvPortMalloc(sizeof(*pxMessage));

    if (message.has_destAddress)
    {
        pxMessage->xDestinationAddress = message.destAddress;
    }

    if (message.has_sourceAddress)
    {
        pxMessage->xSourceAddress = message.sourceAddress;
    }
    if (message.has_cdapMessage)
    {
        serObjectValue_t *pxMsgCdap = pvPortMalloc(sizeof(*pxMsgCdap));
        void *pvSerBuf = pvPortMalloc(message.cdapMessage.size);

        pxMessage->pxMsgCdap = pxMsgCdap;
        pxMessage->pxMsgCdap->pvSerBuffer = pvSerBuf;
        pxMessage->pxMsgCdap->xSerLength = message.cdapMessage.size;

        memcpy(pxMessage->pxMsgCdap->pvSerBuffer, message.cdapMessage.bytes,
               pxMessage->pxMsgCdap->xSerLength);
    }

    return pxMessage;
}

aDataMsg_t *pxSerdesMsgDecodeAData(uint8_t *pucBuffer, size_t xMessageLength)
{

    BaseType_t status;

    /*Allocate space for the decode message data*/
    rina_messages_a_data_t message = rina_messages_a_data_t_init_zero;

    /*Create a stream that will read from the buffer*/
    pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    status = pb_decode(&stream, rina_messages_a_data_t_fields, &message);

    if (!status)
    {
        ESP_LOGE(TAG_RINA, "Decoding failed: %s", PB_GET_ERROR(&stream));
        return NULL;
    }

    return prvSerdesMsgDecodeAData(message);
}

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

serObjectValue_t *pxSerdesMsgNeighborEncode(neighborMessage_t *pxMessage)
{
    bool_t status;
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
    // Allocate space on the stack to store the message data.
    void *pvBuffer = pvRsMemAlloc(MTU);
    int maxLength = MTU;

    // Create a stream that writes to our buffer.
    pb_ostream_t stream = pb_ostream_from_buffer(pvBuffer, maxLength);

    // Now we are ready to encode the message.
    status = pb_encode(&stream, rina_messages_neighbor_t_fields, &message);

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

serObjectValue_t *pxSerdesMsgFlowEncode(flow_t *pxMsg)
{

#ifdef ESP_PLATFORM
    heap_caps_check_integrity(MALLOC_CAP_DEFAULT, true);
#endif

    bool_t status;
    rina_messages_Flow message = rina_messages_Flow_init_zero;

    // Allocate space on the stack to store the message data.
    uint8_t *pucBuffer[1500];
    int maxLength = MTU;

    if (!pxMsg)
    {
        LOGE(TAG_RIB, "No flow message to be sended");
        return NULL;
    }

    // Fill required attributes
    strcpy(message.sourceNamingInfo.applicationProcessName, pxMsg->pxSourceInfo->pcProcessName);
    strcpy(message.destinationNamingInfo.applicationProcessName, pxMsg->pxDestInfo->pcProcessName);

    message.sourcePortId = pxMsg->xSourcePortId;
    message.sourceAddress = pxMsg->xSourceAddress;

    message.has_hopCount = true;
    message.hopCount = pxMsg->ulHopCount;

    message.connectionIds_count = 1;
    message.connectionIds->sourceCEPId = pxMsg->pxConnectionId->xSource;
    message.connectionIds->has_sourceCEPId = true;
    message.connectionIds->qosId = pxMsg->pxConnectionId->xQosId;
    message.connectionIds->has_qosId = true;
    message.connectionIds->destinationCEPId = pxMsg->pxConnectionId->xDestination;
    message.connectionIds->has_destinationCEPId = true;

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
    strcpy(message.dtpConfig.dtppolicyset.policyName,
           pxMsg->pxDtpConfig->pxDtpPolicySet->pcPolicyName);
    message.dtpConfig.dtppolicyset.has_policyName = true;
    strcpy(message.dtpConfig.dtppolicyset.version,
           pxMsg->pxDtpConfig->pxDtpPolicySet->pcPolicyVersion);
    message.dtpConfig.dtppolicyset.has_version = true;

    message.dtpConfig.initialATimer = pxMsg->pxDtpConfig->xInitialATimer;
    message.dtpConfig.has_initialATimer = true;

    message.dtpConfig.dtcpPresent = pxMsg->pxDtpConfig->xDtcpPresent;
    message.dtpConfig.has_dtcpPresent = true;

    // Create a stream that writes to our buffer.
    pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)pucBuffer, sizeof(pucBuffer));

    // Now we are ready to encode the message.
    status = pb_encode(&stream, rina_messages_Flow_fields, &message);

    // Check for errors...
    if (!status)
    {
        LOGE(TAG_ENROLLMENT, "Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return NULL;
    }

    serObjectValue_t *pxSerValue = pvRsMemAlloc(sizeof(*pxSerValue));
    pxSerValue->pvSerBuffer = pucBuffer;
    pxSerValue->xSerLength = stream.bytes_written;

    return pxSerValue;
}

static flow_t *prvSerdesMsgDecodeFlow(rina_messages_Flow message)
{
    flow_t *pxMessage;

    pxMessage = pvRsMemAlloc(sizeof(*pxMessage));
    pxMessage->pxDestInfo = pvRsMemAlloc(sizeof(name_t));
    pxMessage->pxSourceInfo = pvRsMemAlloc(sizeof(name_t));
    pxMessage->pxConnectionId = pvRsMemAlloc(sizeof(connectionId_t));

    if (message.has_destinationAddress)
        pxMessage->xRemoteAddress = message.destinationAddress;

    pxMessage->pxDestInfo->pcProcessName = strdup(message.destinationNamingInfo.applicationProcessName);
    if (message.destinationNamingInfo.has_applicationEntityName)
        pxMessage->pxDestInfo->pcEntityName = strdup(message.destinationNamingInfo.applicationEntityName);
    if (message.destinationNamingInfo.has_applicationEntityInstance)
        pxMessage->pxDestInfo->pcEntityInstance = strdup(message.destinationNamingInfo.applicationEntityInstance);
    if (message.destinationNamingInfo.has_applicationProcessInstance)
        pxMessage->pxDestInfo->pcProcessInstance = strdup(message.destinationNamingInfo.applicationProcessInstance);
    if (message.has_destinationPortId)
        pxMessage->xDestinationPortId = message.destinationPortId;

    pxMessage->xSourceAddress = message.sourceAddress;
    pxMessage->pxSourceInfo->pcProcessName = strdup(message.sourceNamingInfo.applicationProcessName);
    if (message.sourceNamingInfo.has_applicationEntityName)
        pxMessage->pxSourceInfo->pcEntityName = strdup(message.sourceNamingInfo.applicationEntityName);
    if (message.sourceNamingInfo.has_applicationEntityInstance)
        pxMessage->pxSourceInfo->pcEntityInstance = strdup(message.sourceNamingInfo.applicationEntityInstance);
    if (message.sourceNamingInfo.has_applicationProcessInstance)
        pxMessage->pxSourceInfo->pcProcessInstance = strdup(message.sourceNamingInfo.applicationProcessInstance);

    pxMessage->xSourcePortId = message.sourcePortId;

    if (message.connectionIds->has_destinationCEPId)
        pxMessage->pxConnectionId->xDestination = (int)message.connectionIds->destinationCEPId;
    if (message.connectionIds->has_sourceCEPId)
        pxMessage->pxConnectionId->xSource = (int)message.connectionIds->sourceCEPId;
    if (message.connectionIds->has_qosId)
        pxMessage->pxConnectionId->xQosId = (int)message.connectionIds->qosId;

    return pxMessage;
}
void prvPrintDecodeFlow(rina_messages_Flow message)
{
    LOGI(TAG_RIB, "--------Flow Message--------");

    if (message.has_destinationAddress)
        LOGI(TAG_RIB, "Destination Address:%lld", message.destinationAddress);

    LOGI(TAG_RIB, "Destination PN:%s", message.destinationNamingInfo.applicationProcessName);
    if (message.destinationNamingInfo.has_applicationEntityName)
        LOGI(TAG_RIB, "Destination EN:%s", message.destinationNamingInfo.applicationEntityName);
    if (message.destinationNamingInfo.has_applicationEntityInstance)
        LOGI(TAG_RIB, "Destination EI:%s", message.destinationNamingInfo.applicationEntityInstance);
    if (message.destinationNamingInfo.has_applicationProcessInstance)
        LOGI(TAG_RIB, "Destination PI:%s", message.destinationNamingInfo.applicationProcessInstance);
    if (message.has_destinationPortId)
        LOGI(TAG_RIB, "Destination PortId:%lld", message.destinationPortId);

    LOGI(TAG_RIB, "Source Address:%lld", message.sourceAddress);
    LOGI(TAG_RIB, "Source PN:%s", message.sourceNamingInfo.applicationProcessName);
    if (message.sourceNamingInfo.has_applicationEntityName)
        LOGI(TAG_RIB, "Source EN:%s", message.sourceNamingInfo.applicationEntityName);
    if (message.sourceNamingInfo.has_applicationEntityInstance)
        LOGI(TAG_RIB, "Source EI:%s", message.sourceNamingInfo.applicationEntityInstance);
    if (message.sourceNamingInfo.has_applicationProcessInstance)
        LOGI(TAG_RIB, "Source PI:%s", message.sourceNamingInfo.applicationProcessInstance);

    LOGI(TAG_RIB, "Source PortId:%lld", message.sourcePortId);

    if (message.connectionIds->has_destinationCEPId)
        LOGI(TAG_RIB, "Connection Dest Cep Id:%d", (int)message.connectionIds->destinationCEPId);
    if (message.connectionIds->has_sourceCEPId)
        LOGI(TAG_RIB, "Connection Src Cep Id:%d", (int)message.connectionIds->sourceCEPId);
    if (message.connectionIds->has_qosId)
        LOGI(TAG_RIB, "Connection QoS Id:%d", (int)message.connectionIds->qosId);
}

flow_t *pxSerdesMsgDecodeFlow(uint8_t *pucBuffer, size_t xMessageLength)
{

    BaseType_t status;

    /*Allocate space for the decode message data*/
    rina_messages_Flow message = rina_messages_Flow_init_zero;

    /*Create a stream that will read from the buffer*/
    pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    status = pb_decode(&stream, rina_messages_Flow_fields, &message);

    if (!status)
    {
        LOGE(TAG_RINA, "Decoding failed: %s", PB_GET_ERROR(&stream));
        return NULL;
    }

    prvPrintDecodeFlow(message);

    return prvSerdesMsgDecodeFlow(message);
}
