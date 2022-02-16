/*
 * RINA_API.c
 *
 *  Created on: 12 jan. 2022
 *      Author: i2CAT- David Sarabia
 */

#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "BufferManagement.h"
#include "configSensor.h"
#include "common.h"
#include "esp_log.h"
#include "RINA_API.h"

struct appRegistration_t *RINA_application_register(string_t xNameDif, string_t xLocalApp, uint8_t Flags);

struct appRegistration_t *RINA_application_register(string_t xNameDif, string_t xLocalApp, uint8_t Flags)
{
    name_t *xDn, *xAppn, *xDan;
    registerApplicationHandle_t *xRegAppRequest = pvPortMalloc(sizeof(*xRegAppRequest));
    RINAStackEvent_t xStackAppRegistrationEvent = {eStackAppRegistrationEvent, NULL};

    /*Check Flags (RINA_F_NOWAIT)*/

    /*Create name_t objects*/
    xDn = xRinaNameCreate();
    xAppn = xRinaNameCreate();
    xDan = xRinaNameCreate();

    if (xNameDif && xRinaNameFromString(xNameDif, xDn))
    {
        ESP_LOGE(TAG_RINA, "DIFName incorrect");
        xRinaNameFree(xDn);
        return NULL;
    }
    if (xLocalApp && xRinaNameFromString(xLocalApp, xAppn))
    {
        ESP_LOGE(TAG_RINA, "LocalName incorrect");
        xRinaNameFree(xNameDif);
        xRinaNameFree(xAppn);
        return NULL;
    }
    if (!xDan)
    {
        xRinaNameFree(xDn);
        xRinaNameFree(xAppn);
        return NULL;
    }

    /*Structure and event to be send to the RINA Stack */

    xRegAppRequest->xSrcIpcpId = 0;
    xRegAppRequest->xDestIpcpId = 0;
    xRegAppRequest->xDestPort = 1;
    xRegAppRequest->xSrcPort = 1; //Should be random?

    xRegAppRequest->xAppName = xAppn;
    xRegAppRequest->xDifName = xDn;
    xRegAppRequest->xDafName = xDan;

    xStackAppRegistrationEvent.pvData = xRegAppRequest;

    if (xSendEventStructToIPCPTask(&xStackAppRegistrationEvent, (TickType_t)0U) == pdPASS)
    {
        return NULL; //Should return the object Application Registration.
        //It's not clear how to wait until the response.
    }

    return NULL;
}

void xRINA_WeakUpUser(flowAllocateHandle_t *pxFlowAllocateResponse)
{

    if (!pxFlowAllocateResponse)
    {
        ESP_LOGE(TAG_RINA, "No Bits set");
    }

    if ((pxFlowAllocateResponse->xEventGroup != NULL) && (pxFlowAllocateResponse->xEventBits != 0U))
    {
        (void)xEventGroupSetBits(pxFlowAllocateResponse->xEventGroup, pxFlowAllocateResponse->xEventBits);
    }

    pxFlowAllocateResponse->xEventBits = 0U;
}

portId_t RINA_flow_alloc(string_t xNameDIF, string_t xLocalApp, string_t xRemoteApp, struct rinaFlowSpec_t *xFlowSpec, uint8_t Flags);

portId_t RINA_flow_alloc(string_t xNameDIF, string_t xLocalApp, string_t xRemoteApp, struct rinaFlowSpec_t *xFlowSpec, uint8_t Flags)
{
    portId_t xPortId; /* PortId to return to the user*/
    RINAStackEvent_t xStackFlowAllocateEvent = {eStackFlowAllocateEvent, NULL};
    name_t *xDIFName, *xLocalName, *xRemoteName;
    flowAllocateHandle_t *xFlowAllocateRequest;
    struct flowSpec_t *xFlowSpecTmp;
    EventGroupHandle_t xEventGroup;
    TickType_t xRemainingTime;
    BaseType_t xTimed = pdFALSE; /* Check non-blocking*/
    TimeOut_t xTimeOut;

    xFlowSpecTmp = pvPortMalloc(sizeof(*xFlowSpecTmp));

    xFlowAllocateRequest = pvPortMalloc(sizeof(*xFlowAllocateRequest));

    xEventGroup = xEventGroupCreate();

    if (xEventGroup == NULL)
    {
        vPortFree(xFlowAllocateRequest);
        vPortFree(xFlowSpecTmp);
        return -1;
    }
    else
    {

        /* Check Flow spec ok version*/

        /*Check Flags*/
        (void)memset(xFlowAllocateRequest, 0, sizeof(*xFlowAllocateRequest));

        /*Create objetcs type name_t from string_t*/
        ESP_LOGI(TAG_RINA, "FlowAllocate: NameCreate");
        xDIFName = xRinaNameCreate();
        xLocalName = xRinaNameCreate();
        xRemoteName = xRinaNameCreate();

        if (!xDIFName && !xLocalName && !xRemoteName)
        {
            ESP_LOGE(TAG_RINA, "Error");
        }
        ESP_LOGE(TAG_RINA, "OK");

        if (!xNameDIF && xRinaNameFromString(xNameDIF, xDIFName))
        {
            ESP_LOGE(TAG_RINA, "DIFName incorrect");
            xRinaNameFree(xDIFName);
            return -1;
        }
        if (!xLocalApp && xRinaNameFromString(xLocalApp, xLocalName))
        {
            ESP_LOGE(TAG_RINA, "LocalName incorrect");
            xRinaNameFree(xDIFName);
            xRinaNameFree(xLocalName);
            return -1;
        }
        if (!xRemoteApp && xRinaNameFromString(xRemoteApp, xRemoteName))
        {
            ESP_LOGE(TAG_RINA, "RemoteName incorrect");
            xRinaNameFree(xDIFName);
            xRinaNameFree(xDIFName);
            xRinaNameFree(xRemoteName);
            return -1;
        }

        /*xPortId set to zero until the TASK fill properly.*/
        xPortId = 0;

        /*Struct Data to sent attached into the event*/

        if (xFlowAllocateRequest != NULL)
        {
            xFlowAllocateRequest->xEventGroup = xEventGroup;
            xFlowAllocateRequest->xReceiveBlockTime = FLOW_DEFAULT_RECEIVE_BLOCK_TIME;
            xFlowAllocateRequest->xSendBlockTime = FLOW_DEFAULT_SEND_BLOCK_TIME;

            xFlowAllocateRequest->xLocal = xLocalName;
            xFlowAllocateRequest->xRemote = xRemoteName;
            xFlowAllocateRequest->xDifName = xDIFName;
            xFlowAllocateRequest->xFspec = xFlowSpecTmp;
            xFlowAllocateRequest->xPortId = xPortId;

            if (!xFlowSpec)
            {
                xFlowAllocateRequest->xFspec->ulAverageBandwidth = 0;
                xFlowAllocateRequest->xFspec->ulAverageSduBandwidth = 0;
                xFlowAllocateRequest->xFspec->ulDelay = 0;
                xFlowAllocateRequest->xFspec->ulJitter = 0;
                xFlowAllocateRequest->xFspec->usLoss = 10000;
                xFlowAllocateRequest->xFspec->ulMaxAllowableGap = 10;
                xFlowAllocateRequest->xFspec->xOrderedDelivery = false;
                xFlowAllocateRequest->xFspec->ulUndetectedBitErrorRate = 0;
                xFlowAllocateRequest->xFspec->xPartialDelivery = true;
                xFlowAllocateRequest->xFspec->xMsgBoundaries = false;
            }
            else
            {
                xFlowAllocateRequest->xFspec->ulAverageBandwidth = xFlowSpec->avg_bandwidth;
                xFlowAllocateRequest->xFspec->ulAverageSduBandwidth = 0;
                xFlowAllocateRequest->xFspec->ulDelay = xFlowSpec->max_delay;
                xFlowAllocateRequest->xFspec->ulJitter = xFlowSpec->max_jitter;
                xFlowAllocateRequest->xFspec->usLoss = xFlowSpec->max_loss;
                xFlowAllocateRequest->xFspec->ulMaxAllowableGap = xFlowSpec->max_sdu_gap;
                xFlowAllocateRequest->xFspec->xOrderedDelivery = xFlowSpec->in_order_delivery;
                xFlowAllocateRequest->xFspec->ulUndetectedBitErrorRate = 0;
                xFlowAllocateRequest->xFspec->xPartialDelivery = true;
                xFlowAllocateRequest->xFspec->xMsgBoundaries = xFlowSpec->msg_boundaries;
            }

            xStackFlowAllocateEvent.pvData = xFlowAllocateRequest;

            if (xSendEventStructToIPCPTask(&xStackFlowAllocateEvent, (TickType_t)0U) == pdFAIL)
            {
                ESP_LOGE(TAG_RINA, "IPCP Task not working properly");
                return -1;
            }
            else
            {
                /* The IP-task will set the 'eFLOW_BOUND' bit when it has done its
             * job. */

                (void)xEventGroupWaitBits(xFlowAllocateRequest->xEventGroup, (EventBits_t)eFLOW_BOUND, pdTRUE /*xClearOnExit*/, pdFALSE /*xWaitAllBits*/, portMAX_DELAY);

                ESP_LOGE(TAG_RINA, "TEST OK");

                /*Check if the flow was bound*/
                /* if (!socketSOCKET_IS_BOUND(pxSocket))
                {
                    xReturn = -pdFREERTOS_ERRNO_EINVAL;
                }*/
                return xPortId;
            }
        }
    }

    return -1;
}

BaseType_t RINA_flow_write(portId_t xPortId, void *pvBuffer, size_t uxTotalDataLength);
BaseType_t RINA_flow_write(portId_t xPortId, void *pvBuffer, size_t uxTotalDataLength)
{
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    void *pvCopyDest;
    TimeOut_t xTimeOut;
    TickType_t xTicksToWait;
    RINAStackEvent_t xStackTxEvent = {eStackTxEvent, NULL};

    /*Check that DataLength is not longer than MAX_SDU_SIZE*/
    //This should not consider??? because the delimiter split into several packets??
    if (uxTotalDataLength <= MAX_SDU_SIZE)
    {
        /*Check if the Flow is active*/
        xTicksToWait = (TickType_t)0U;

        /*Request a NetworkBuffer to copy data*/

        vTaskSetTimeOutState(&xTimeOut);

        pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(uxTotalDataLength, xTicksToWait); //sizeof length DataUser packet.

        if (pxNetworkBuffer != NULL)
        {
            pvCopyDest = (void *)&pxNetworkBuffer->pucEthernetBuffer;
            (void)memcpy(pvCopyDest, pvBuffer, uxTotalDataLength);

            if (xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdTRUE)
            {
                /* The entire block time has been used up. */
                xTicksToWait = (TickType_t)0;
            }
        }

        if (pxNetworkBuffer != NULL)
        {
            /*Fill the pxNetworkBuffer descriptor properly*/
            pxNetworkBuffer->xDataLength = uxTotalDataLength;

            xStackTxEvent.pvData = pxNetworkBuffer;

            if (xSendEventStructToIPCPTask(&xStackTxEvent, xTicksToWait) == pdPASS)
            {
                return pdTRUE;
            }
            else
            {
                vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
                return pdFALSE;
            }
        }

        return pdFALSE;
    }

    return pdFALSE;
}
