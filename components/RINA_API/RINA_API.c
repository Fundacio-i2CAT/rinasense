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

struct appRegistration_t *RINA_application_register(string_t pcNameDif, string_t pcLocalApp, uint8_t Flags);

struct appRegistration_t *RINA_application_register(string_t pcNameDif, string_t pcLocalApp, uint8_t Flags)
{
    name_t *xDn, *xAppn, *xDan;
    registerApplicationHandle_t *xRegAppRequest = pvPortMalloc(sizeof(*xRegAppRequest));
    RINAStackEvent_t xStackAppRegistrationEvent = {eStackAppRegistrationEvent, NULL};

    /*Check Flags (RINA_F_NOWAIT)*/

    /*Create name_t objects*/
    xDn = xRinaNameCreate();
    xAppn = xRinaNameCreate();
    xDan = xRinaNameCreate();

    if (pcNameDif && xRinaNameFromString(pcNameDif, xDn))
    {
        ESP_LOGE(TAG_RINA, "DIFName incorrect");
        xRinaNameFree(xDn);
        return NULL;
    }
    if (pcLocalApp && xRinaNameFromString(pcLocalApp, xAppn))
    {
        ESP_LOGE(TAG_RINA, "LocalName incorrect");
        xRinaNameFree(pcNameDif);
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

    xRegAppRequest->pxAppName = xAppn;
    xRegAppRequest->pxDifName = xDn;
    xRegAppRequest->pxDafName = xDan;

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

portId_t RINA_flow_alloc(string_t pcNameDIF, string_t pcLocalApp, string_t pcRemoteApp, struct rinaFlowSpec_t *xFlowSpec, uint8_t Flags);

portId_t RINA_flow_alloc(string_t pcNameDIF, string_t pcLocalApp, string_t pcRemoteApp, struct rinaFlowSpec_t *xFlowSpec, uint8_t Flags)
{
    portId_t xPortId; /* PortId to return to the user*/
    RINAStackEvent_t xStackFlowAllocateEvent = {eStackFlowAllocateEvent, NULL};
    name_t *pxDIFName, *pxLocalName, *pxRemoteName;
    flowAllocateHandle_t *pxFlowAllocateRequest;
    struct flowSpec_t *pxFlowSpecTmp;
    EventGroupHandle_t xEventGroup;
    TickType_t xRemainingTime;
    BaseType_t xTimed = pdFALSE; /* Check non-blocking*/
    TimeOut_t xTimeOut;

    pxFlowSpecTmp = pvPortMalloc(sizeof(*pxFlowSpecTmp));

    pxFlowAllocateRequest = pvPortMalloc(sizeof(*pxFlowAllocateRequest));

    xEventGroup = xEventGroupCreate();

    if (xEventGroup == NULL)
    {
        vPortFree(pxFlowAllocateRequest);
        vPortFree(pxFlowSpecTmp);
        return -1;
    }
    else
    {

        /* Check Flow spec ok version*/

        /*Check Flags*/
        (void)memset(pxFlowAllocateRequest, 0, sizeof(*pxFlowAllocateRequest));

        /*Create objetcs type name_t from string_t*/
        //ESP_LOGI(TAG_RINA, "FlowAllocate: NameCreate");
        pxDIFName = xRinaNameCreate();
        pxLocalName = xRinaNameCreate();
        pxRemoteName = xRinaNameCreate();

        if (!pxDIFName && !pxLocalName && !pxRemoteName)
        {
            ESP_LOGE(TAG_RINA, "Error");
        }
        ESP_LOGE(TAG_RINA, "OK");

        if (!pcNameDIF && xRinaNameFromString(pcNameDIF, pxDIFName))
        {
            ESP_LOGE(TAG_RINA, "DIFName incorrect");
            xRinaNameFree(pxDIFName);
            return -1;
        }
        if (!pcLocalApp && xRinaNameFromString(pcLocalApp, pxLocalName))
        {
            ESP_LOGE(TAG_RINA, "LocalName incorrect");
            xRinaNameFree(pxDIFName);
            xRinaNameFree(pxLocalName);
            return -1;
        }
        if (!pcRemoteApp && xRinaNameFromString(pcRemoteApp, pxRemoteName))
        {
            ESP_LOGE(TAG_RINA, "RemoteName incorrect");
            xRinaNameFree(pxDIFName);
            //xRinaNameFree(pxDIFName);
            xRinaNameFree(pxRemoteName);
            return -1;
        }

        /*xPortId set to zero until the TASK fill properly.*/
        xPortId = 0;

        /*Struct Data to sent attached into the event*/

        if (pxFlowAllocateRequest != NULL)
        {
            pxFlowAllocateRequest->xEventGroup = xEventGroup;
            pxFlowAllocateRequest->xReceiveBlockTime = FLOW_DEFAULT_RECEIVE_BLOCK_TIME;
            pxFlowAllocateRequest->xSendBlockTime = FLOW_DEFAULT_SEND_BLOCK_TIME;

            pxFlowAllocateRequest->pxLocal = pxLocalName;
            pxFlowAllocateRequest->pxRemote = pxRemoteName;
            pxFlowAllocateRequest->pxDifName = pxDIFName;
            pxFlowAllocateRequest->pxFspec =pxFlowSpecTmp;
            pxFlowAllocateRequest->xPortId = xPortId;

            if (!xFlowSpec)
            {
                pxFlowAllocateRequest->pxFspec->ulAverageBandwidth = 0;
                pxFlowAllocateRequest->pxFspec->ulAverageSduBandwidth = 0;
                pxFlowAllocateRequest->pxFspec->ulDelay = 0;
                pxFlowAllocateRequest->pxFspec->ulJitter = 0;
                pxFlowAllocateRequest->pxFspec->usLoss = 10000;
                pxFlowAllocateRequest->pxFspec->ulMaxAllowableGap = 10;
                pxFlowAllocateRequest->pxFspec->xOrderedDelivery = false;
                pxFlowAllocateRequest->pxFspec->ulUndetectedBitErrorRate = 0;
                pxFlowAllocateRequest->pxFspec->xPartialDelivery = true;
                pxFlowAllocateRequest->pxFspec->xMsgBoundaries = false;
            }
            else
            {
                pxFlowAllocateRequest->pxFspec->ulAverageBandwidth = xFlowSpec->avg_bandwidth;
                pxFlowAllocateRequest->pxFspec->ulAverageSduBandwidth = 0;
                pxFlowAllocateRequest->pxFspec->ulDelay = xFlowSpec->max_delay;
                pxFlowAllocateRequest->pxFspec->ulJitter = xFlowSpec->max_jitter;
                pxFlowAllocateRequest->pxFspec->usLoss = xFlowSpec->max_loss;
                pxFlowAllocateRequest->pxFspec->ulMaxAllowableGap = xFlowSpec->max_sdu_gap;
                pxFlowAllocateRequest->pxFspec->xOrderedDelivery = xFlowSpec->in_order_delivery;
                pxFlowAllocateRequest->pxFspec->ulUndetectedBitErrorRate = 0;
                pxFlowAllocateRequest->pxFspec->xPartialDelivery = true;
                pxFlowAllocateRequest->pxFspec->xMsgBoundaries = xFlowSpec->msg_boundaries;
            }

            xStackFlowAllocateEvent.pvData = pxFlowAllocateRequest;

            if (xSendEventStructToIPCPTask(&xStackFlowAllocateEvent, (TickType_t)0U) == pdFAIL)
            {
                ESP_LOGE(TAG_RINA, "IPCP Task not working properly");
                return -1;
            }
            else
            {
                /* The IP-task will set the 'eFLOW_BOUND' bit when it has done its
             * job. */

                (void)xEventGroupWaitBits(pxFlowAllocateRequest->xEventGroup, (EventBits_t)eFLOW_BOUND, pdTRUE /*xClearOnExit*/, pdFALSE /*xWaitAllBits*/, portMAX_DELAY);

                ESP_LOGE(TAG_RINA, "TEST OK:%d",pxFlowAllocateRequest->xPortId);

                /*Check if the flow was bound*/
                /* if (!socketSOCKET_IS_BOUND(pxSocket))
                {
                    xReturn = -pdFREERTOS_ERRNO_EINVAL;
                }*/
                return pxFlowAllocateRequest->xPortId;
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
