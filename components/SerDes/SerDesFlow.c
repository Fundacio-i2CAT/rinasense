#include "portability/port.h"

#include "common/error.h"
#include "common/rinasense_errors.h"
#include "common/rsrc.h"

#include "SerDesFlow.h"
#include "FlowMessage.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

#define TAG_SD_FLOW "[SD-FLOW]"

#ifdef __LONG_WIDTH__
#if __LONG_WIDTH__ == 32
#define ULONG_FMT "%llu"
#elif __LONG_WIDTH__ == 64
#define ULONG_FMT "%lu"
#else
#error Not sure how to handle this __LONG_WIDTH__
#endif
#else
#error __LONG_WIDTH__ is not defined
#endif

void prvPrintDecodeFlow(rina_messages_Flow message)
{
    LOGI(TAG_SD_FLOW, "--------Flow Message--------");

    if (message.has_destinationAddress)
        LOGI(TAG_SD_FLOW, "Destination Address:"ULONG_FMT, message.destinationAddress);

    LOGI(TAG_SD_FLOW, "Destination PN:%s", message.destinationNamingInfo.applicationProcessName);
    if (message.destinationNamingInfo.has_applicationEntityName)
        LOGI(TAG_SD_FLOW, "Destination EN:%s", message.destinationNamingInfo.applicationEntityName);
    if (message.destinationNamingInfo.has_applicationEntityInstance)
        LOGI(TAG_SD_FLOW, "Destination EI:%s", message.destinationNamingInfo.applicationEntityInstance);
    if (message.destinationNamingInfo.has_applicationProcessInstance)
        LOGI(TAG_SD_FLOW, "Destination PI:%s", message.destinationNamingInfo.applicationProcessInstance);
    if (message.has_destinationPortId)
        LOGI(TAG_SD_FLOW, "Destination PortId:"ULONG_FMT, message.destinationPortId);

    LOGI(TAG_SD_FLOW, "Source Address:"ULONG_FMT, message.sourceAddress);

    LOGI(TAG_SD_FLOW, "Source PN:%s", message.sourceNamingInfo.applicationProcessName);
    if (message.sourceNamingInfo.has_applicationEntityName)
        LOGI(TAG_SD_FLOW, "Source EN:%s", message.sourceNamingInfo.applicationEntityName);
    if (message.sourceNamingInfo.has_applicationEntityInstance)
        LOGI(TAG_SD_FLOW, "Source EI:%s", message.sourceNamingInfo.applicationEntityInstance);
    if (message.sourceNamingInfo.has_applicationProcessInstance)
        LOGI(TAG_SD_FLOW, "Source PI:%s", message.sourceNamingInfo.applicationProcessInstance);

    LOGI(TAG_SD_FLOW, "Source PortId:"ULONG_FMT, message.sourcePortId);

    if (message.connectionIds->has_destinationCEPId)
        LOGI(TAG_SD_FLOW, "Connection Dest Cep Id:%d", (int)message.connectionIds->destinationCEPId);
    if (message.connectionIds->has_sourceCEPId)
        LOGI(TAG_SD_FLOW, "Connection Src Cep Id:%d", (int)message.connectionIds->sourceCEPId);
    if (message.connectionIds->has_qosId)
        LOGI(TAG_SD_FLOW, "Connection QoS Id:%d", (int)message.connectionIds->qosId);
}

static flow_t *prvSerdesMsgDecodeFlow(rina_messages_Flow message)
{
    flow_t *pxMessage;

    pxMessage = pvRsMemAlloc(sizeof(flow_t));

    if (message.has_destinationAddress)
        pxMessage->xRemoteAddress = message.destinationAddress;

    if (ERR_CHK(xNameAssignFromPartsDup(&pxMessage->xDestInfo,
                                        message.destinationNamingInfo.applicationProcessName,
                                        message.destinationNamingInfo.applicationProcessInstance,
                                        message.destinationNamingInfo.applicationEntityName,
                                        message.destinationNamingInfo.applicationEntityInstance))) {
        LOGE(TAG_SD_FLOW, "Failed to allocate memory for destination name");
        return NULL;
    }

    if (ERR_CHK(xNameAssignFromPartsDup(&pxMessage->xSourceInfo,
                                        message.sourceNamingInfo.applicationProcessName,
                                        message.sourceNamingInfo.applicationProcessInstance,
                                        message.sourceNamingInfo.applicationEntityName,
                                        message.sourceNamingInfo.applicationEntityInstance))) {
        LOGE(TAG_SD_FLOW, "Failed to allocate memory for source name");
        return NULL;
    }

    if (message.has_destinationPortId)
        pxMessage->xDestinationPortId = message.destinationPortId;

    pxMessage->xSourceAddress = message.sourceAddress;
    pxMessage->xSourcePortId = message.sourcePortId;

    if (message.connectionIds->has_destinationCEPId)
        pxMessage->xConnectionId.xDestination = (int)message.connectionIds->destinationCEPId;
    if (message.connectionIds->has_sourceCEPId)
        pxMessage->xConnectionId.xSource = (int)message.connectionIds->sourceCEPId;
    if (message.connectionIds->has_qosId)
        pxMessage->xConnectionId.xQosId = (int)message.connectionIds->qosId;

    return pxMessage;
}

rsErr_t xSerDesFlowInit(SerDesFlow_t *pxSD)
{
    size_t unSz;
    int n;

    unSz = sizeof(rina_messages_Flow) + sizeof(serObjectValue_t);
    if (!(pxSD->xPool = pxRsrcNewPool("Flow SerDes", unSz, 1, 1, 0)))
        return ERR_OOM;

    return SUCCESS;
}

serObjectValue_t *pxSerDesFlowEncode(SerDesFlow_t *pxSD, flow_t *pxMsg)
{
    bool_t xStatus;
    rina_messages_Flow msg = rina_messages_Flow_init_zero;
    pb_ostream_t xStream;
    serObjectValue_t *pxSerValue;

    RsAssert(pxSD);
    RsAssert(pxMsg);

#define _COPY(x, y) strncpy(x, y, sizeof(x))

    _COPY(msg.sourceNamingInfo.applicationProcessName, pxMsg->xSourceInfo.pcProcessName);
    _COPY(msg.sourceNamingInfo.applicationProcessInstance, pxMsg->xSourceInfo.pcProcessInstance);
    _COPY(msg.sourceNamingInfo.applicationEntityName, pxMsg->xSourceInfo.pcEntityName);
    _COPY(msg.sourceNamingInfo.applicationEntityInstance, pxMsg->xSourceInfo.pcEntityInstance);

    _COPY(msg.destinationNamingInfo.applicationProcessName, pxMsg->xDestInfo.pcProcessName);
    _COPY(msg.destinationNamingInfo.applicationProcessInstance, pxMsg->xDestInfo.pcProcessInstance);
    _COPY(msg.destinationNamingInfo.applicationEntityName, pxMsg->xDestInfo.pcEntityName);
    _COPY(msg.destinationNamingInfo.applicationEntityInstance, pxMsg->xDestInfo.pcEntityInstance);

    msg.sourcePortId = pxMsg->xSourcePortId;
    msg.sourceAddress = pxMsg->xSourceAddress;

    msg.has_hopCount = true;
    msg.hopCount = pxMsg->ulHopCount;

    msg.connectionIds_count = 1;
    msg.connectionIds->sourceCEPId = pxMsg->xConnectionId.xSource;
    msg.connectionIds->has_sourceCEPId = true;
    msg.connectionIds->qosId = pxMsg->xConnectionId.xQosId;
    msg.connectionIds->has_qosId = true;
    msg.connectionIds->destinationCEPId = pxMsg->xConnectionId.xDestination;
    msg.connectionIds->has_destinationCEPId = true;

    msg.has_qosParameters = true;
    msg.qosParameters.qosid = pxMsg->xQosSpec.xQosId;
    msg.qosParameters.has_qosid = true;
    strcpy(msg.qosParameters.name, pxMsg->xQosSpec.pcQosName);
    msg.qosParameters.has_name = true;
    msg.qosParameters.partialDelivery = pxMsg->xQosSpec.xFlowSpec.xPartialDelivery;
    msg.qosParameters.has_partialDelivery = true;
    msg.qosParameters.order = pxMsg->xQosSpec.xFlowSpec.xOrderedDelivery;
    msg.qosParameters.has_order = true;

    msg.has_dtpConfig = true;
    strcpy(msg.dtpConfig.dtppolicyset.policyName, pxMsg->xDtpConfig.xDtpPolicySet.pcPolicyName);
    msg.dtpConfig.dtppolicyset.has_policyName = true;
    strcpy(msg.dtpConfig.dtppolicyset.version, pxMsg->xDtpConfig.xDtpPolicySet.pcPolicyVersion);
    msg.dtpConfig.dtppolicyset.has_version = true;

    msg.dtpConfig.initialATimer = pxMsg->xDtpConfig.xInitialATimer;
    msg.dtpConfig.has_initialATimer = true;

    msg.dtpConfig.dtcpPresent = pxMsg->xDtpConfig.xDtcpPresent;
    msg.dtpConfig.has_dtcpPresent = true;

    if (!(pxSerValue = pxRsrcAlloc(pxSD->xPool, "Flow Encoding")))
        return ERR_SET_OOM_NULL;

    pxSerValue->pvSerBuffer = pxSerValue + sizeof(serObjectValue_t);

    // Create a stream that writes to our buffer.
    xStream = pb_ostream_from_buffer((pb_byte_t *)pxSerValue->pvSerBuffer,
                                     sizeof(rina_messages_Flow));

    // Now we are ready to encode the message.
    xStatus = pb_encode(&xStream, rina_messages_Flow_fields, &msg);

    // Check for errors...
    if (!xStatus)
        return ERR_SET_NULL(ERR_SERDES_ENCODING_FAIL);

    pxSerValue->xSerLength = xStream.bytes_written;

    return pxSerValue;
}

flow_t *pxSerDesFlowDecode(SerDesFlow_t *pxSD, uint8_t *pucBuffer, size_t xMessageLength)
{
    bool_t status;

    /*Allocate space for the decode message data*/
    rina_messages_Flow message = rina_messages_Flow_init_zero;

    /*Create a stream that will read from the buffer*/
    pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)pucBuffer, xMessageLength);

    status = pb_decode(&stream, rina_messages_Flow_fields, &message);

    if (!status)
        return ERR_SET_NULL(ERR_SERDES_DECODING_FAIL);

    prvPrintDecodeFlow(message);

    return prvSerdesMsgDecodeFlow(message);
}
