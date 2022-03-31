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
#include "pb_encode.h"
#include "pb_decode.h"


#include "esp_log.h"

/*List of Neighbors*/
static List_t xNeighborsList;

neighborInfo_t *pxEnrollmentNeighborLookup(string_t xRemoteAPName)
{
        neighborInfo_t *pxNeigh;

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        pxNeigh = pvPortMalloc(sizeof(*pxNeigh));

        /* Find a way to iterate in the list and compare the addesss*/

        pxListEnd = listGET_END_MARKER(&xNeighborsList);
        pxListItem = listGET_HEAD_ENTRY(&xNeighborsList);

        while (pxListItem != pxListEnd)
        {

                pxNeigh = (neighborInfo_t *)listGET_LIST_ITEM_VALUE(pxListItem);

                if (pxNeigh)
                {
                        ESP_LOGI(TAG_ENROLLMENT, "Neighbor %p, #: %s", pxNeigh, pxNeigh->xAPName);
                        return pxNeigh;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        /*Adding new Neigh info*/
        strcpy(pxNeigh->xAPName, xRemoteAPName);
       // vListInitialiseItem(&pxNeigh->xNeighborItem);
        // vListInsert(&pxNeigh->xNeighborItem, &xNeighborsList);

        return pxNeigh;
}

BaseType_t xEnrollmentFlowAllocateRequest();

void xEnrollmentInit(portId_t xPortId);
void xEnrollmentInit(portId_t xPortId)
{

        name_t *pxSource;
        name_t *pxDestInfo;
        // porId_t xN1FlowPortId;
        authPolicy_t *pxAuth;
        appConnection_t *test;
        test = pvPortMalloc(sizeof(*test));

        pxSource = pvPortMalloc(sizeof(*pxSource));
        pxDestInfo = pvPortMalloc(sizeof(*pxDestInfo));
        pxAuth = pvPortMalloc(sizeof(*pxAuth));

        pxSource->pcEntityInstance = NORMAL_ENTITY_NAME;
        pxSource->pcEntityName = MANAGEMENT_AE;

        pxSource->pcProcessInstance = NORMAL_PROCESS_INSTANCE;
        pxSource->pcProcessName = NORMAL_PROCESS_NAME;

        pxDestInfo->pcProcessName = NORMAL_DIF_NAME;
        pxDestInfo->pcProcessInstance = "";
        pxDestInfo->pcEntityInstance = "";
        pxDestInfo->pcEntityName = MANAGEMENT_AE;

        pxAuth->ucAbsSyntax = 1;
        pxAuth->pcVersion = "1";
        pxAuth->pcName = "PSOC_authentication-none";

        /*Initialization xNeighbors item*/
        vListInitialise(&xNeighborsList);

        // Find PortId

        xRibdConnectToIpcp(pxSource, pxDestInfo, xPortId, pxAuth);
}

rina_messages_enrollmentInformation_t prvEnrollmentFillEncodeMsg(enrollmentMessage_t *pxMsg)
{
        rina_messages_enrollmentInformation_t msgPb = rina_messages_enrollmentInformation_t_init_zero;

        if (pxMsg->ullAddress > 0)
        {
                msgPb.address = pxMsg->ullAddress;
                msgPb.has_address = true;
        }

        return msgPb;
}

/*This should be moved to other module*/
serObjectValue_t *pxEnrollmentEncodeMsg(enrollmentMessage_t *pxMsg)
{
        BaseType_t status;
        rina_messages_enrollmentInformation_t enrollMsg = prvEnrollmentFillEncodeMsg(pxMsg);

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
                return NULL;
        }

        serObjectValue_t *pxSerMsg = pvPortMalloc(sizeof(*pxSerMsg));
        pxSerMsg->pvSerBuffer = pvBuffer;
        pxSerMsg->xSerLength = stream.bytes_written;

        return pxSerMsg;
}

BaseType_t xEnrollmentHandleConnectR(string_t xRemoteProcessName, portId_t xN1Port)
{

        string_t xRemoteAPN = xRemoteProcessName;
        neighborInfo_t *pxNeighbor = NULL;

        // Check if the neighbor is already in the neighbor list, add it if not
        pxNeighbor = pxEnrollmentNeighborLookup(xRemoteProcessName);

        configASSERT(pxNeighbor != NULL);

        pxNeighbor->eEnrollmentState = eENROLLMENT_IN_PROGRESS;

        // Send M_START to the enroller if not enrolled
        enrollmentMessage_t *pxEnrollmentMsg = pvPortMalloc(sizeof(*pxEnrollmentMsg));
        pxEnrollmentMsg->ullAddress = LOCAL_ADDRESS;

        serObjectValue_t *pxObjVal = pxEnrollmentEncodeMsg(pxEnrollmentMsg);

        if (!xRibdSendRequest("Enrollment", "/difm/enr", -1, M_START, xN1Port, pxObjVal))
        {
                ESP_LOGE(TAG_ENROLLMENT, "It was a problem to sen the request");
                return pdFALSE;
        }

        vPortFree(pxEnrollmentMsg);
        vPortFree(pxObjVal);

        return pdTRUE;
}
