/*
 * RINA_API.c
 *
 *  Created on: 12 jan. 2022
 *      Author: i2CAT- David Sarabia
 */

#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "portability/port.h"
#include "common/list.h"
#include "common/rina_ids.h"
#include "common/rina_name.h"

#include "BufferManagement.h"
#include "RINA_API_flows.h"
#include "common/list.h"
#include "configSensor.h"
#include "rina_buffers.h"
#include "rina_common_port.h"
#include "RINA_API.h"
#include "IPCP.h"
#include "IPCP_api.h"
#include "IPCP_events.h"
#include "IPCP_normal_api.h"
#include "FlowAllocator_api.h"

static pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

void prvWaitForBit(pthread_cond_t *pxCond,
                   pthread_mutex_t *pxCondMutex,
                   long *pnTargetBits,
                   long nDesiredBits,
                   bool_t xClearBits)
{
    pthread_mutex_lock(pxCondMutex);

    /* Wait until we get the actual bit we want. */
    while (!(*pnTargetBits & nDesiredBits))
        pthread_cond_wait(pxCond, pxCondMutex);

    /* Clear the bit if desired */
    if (xClearBits)
        *pnTargetBits |= ~nDesiredBits;

    pthread_mutex_unlock(pxCondMutex);
}

struct appRegistration_t *RINA_application_register(string_t pcNameDif,
                                                    string_t pcLocalApp,
                                                    uint8_t Flags);

struct appRegistration_t *RINA_application_register(string_t pcNameDif,
                                                    string_t pcLocalApp,
                                                    uint8_t Flags)
{
    name_t *xDn, *xAppn, *xDan;
    registerApplicationHandle_t *xRegAppRequest;
    RINAStackEvent_t xStackAppRegistrationEvent = {
        .eEventType = eStackAppRegistrationEvent,
        .xData.PV = NULL};

    xRegAppRequest = pvRsMemAlloc(sizeof(registerApplicationHandle_t));
    if (!xRegAppRequest)
    {
        LOGE(TAG_RINA, "Failed to allocate memory for registration handle");
        return NULL;
    }

    /*Check Flags (RINA_F_NOWAIT)*/

    /*Create name_t objects*/
    xDn = pxRStrNameCreate();
    xAppn = pxRStrNameCreate();
    xDan = pxRStrNameCreate();
#if 0
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
    xRegAppRequest->xSrcPort = 1; // Should be random?

    xRegAppRequest->pxAppName = xAppn;
    xRegAppRequest->pxDifName = xDn;
    xRegAppRequest->pxDafName = xDan;

    xStackAppRegistrationEvent.xData.PV = xRegAppRequest;

    if (xSendEventStructToIPCPTask(&xStackAppRegistrationEvent, (TickType_t)0U) == pdPASS)
        {
            return NULL; // Should return the object Application Registration.
            // It's not clear how to wait until the response.
        }
#endif
    return NULL;
}

bool_t xRINA_bind(flowAllocateHandle_t *pxFlowRequest)
{
    RINAStackEvent_t xBindEvent;

    if (!pxFlowRequest)
    {
        LOGE(TAG_RINA, "No flow Request");
        return false;
    }

    xBindEvent.eEventType = eFlowBindEvent;
    xBindEvent.xData.PV = (void *)pxFlowRequest;

    if (xSendEventStructToIPCPTask(&xBindEvent, 0) == false)
        /* Failed to wake-up the IPCP-task, no use to wait for it */
        return false;
    else
    {
        prvWaitForBit(&pxFlowRequest->xEventCond,
                      &pxFlowRequest->xEventMutex,
                      &pxFlowRequest->nEventBits,
                      eFLOW_BOUND,
                      true);
        LOGI(TAG_RINA, "Flow Bound");
        return true;
    }
}

void vRINA_WakeUpFlowRequest(flowAllocateHandle_t *pxFlowAllocateResponse, int nNewBits)
{
    LOGI(TAG_RINA, "Waking up Flow Request Handle");

    /* It doesn't make sense to signal nothing. */
    RsAssert(nNewBits != 0);

    pthread_mutex_lock(&pxFlowAllocateResponse->xEventMutex);
    {
        pxFlowAllocateResponse->nEventBits |= nNewBits;
        pthread_cond_signal(&pxFlowAllocateResponse->xEventCond);
    }
    pthread_mutex_unlock(&pxFlowAllocateResponse->xEventMutex);
}

static flowAllocateHandle_t *prvRINACreateFlowRequest(string_t pcNameDIF,
                                                      string_t pcLocalApp,
                                                      string_t pcRemoteApp,
                                                      struct rinaFlowSpec_t *xFlowSpec)
{
    LOGI(TAG_RINA, "Creating a new flow request");
    portId_t xPortId; /* PortId to return to the user*/
    name_t *pxDIFName = NULL;
    name_t *pxLocalName = NULL;
    name_t *pxRemoteName = NULL;
    struct flowSpec_t *pxFlowSpecTmp = NULL;
    flowAllocateHandle_t *pxFlowAllocateRequest = NULL;

    pxFlowSpecTmp = pvRsMemAlloc(sizeof(*pxFlowSpecTmp));
    if (!pxFlowSpecTmp)
    {
        LOGE(TAG_RINA, "Failed to allocate memory for flow specifications");
        goto err;
    }

    pxFlowAllocateRequest = pvRsMemAlloc(sizeof(*pxFlowAllocateRequest));
    if (!pxFlowAllocateRequest)
    {
        LOGE(TAG_RINA, "Failed to allocate memory for flow request");
        goto err;
    }

    if (pthread_cond_init(&pxFlowAllocateRequest->xEventCond, NULL) != 0)
    {
        LOGE(TAG_RINA, "Failed to initialize thread condition signal");
        goto err;
    }

    if (pthread_mutex_init(&pxFlowAllocateRequest->xEventMutex, NULL) != 0)
    {
        LOGE(TAG_RINA, "Failed to initialize thread condition signal");
        pthread_cond_destroy(&pxFlowAllocateRequest->xEventCond);
        goto err;
    }

    pxFlowAllocateRequest->nEventBits = 0;

    /* Check Flow spec ok version*/

    /*Check Flags*/
    (void)memset(pxFlowAllocateRequest, 0, sizeof(*pxFlowAllocateRequest));

    /*Create objetcs type name_t from string_t*/
    pxDIFName = pxRStrNameCreate();
    pxLocalName = pxRStrNameCreate();
    pxRemoteName = pxRStrNameCreate();

    if (!pxDIFName || !pxLocalName || !pxRemoteName)
    {
        LOGE(TAG_RINA, "Rina Names were not created properly");
        goto err;
    }
    if (!xRinaNameFromString(pcNameDIF, pxDIFName))
    {
        LOGE(TAG_RINA, "No possible to convert String to Rina Name");
        goto err;
    }
    if (!pcLocalApp || !xRinaNameFromString(pcLocalApp, pxLocalName))
    {
        LOGE(TAG_RINA, "LocalName incorrect");
        goto err;
    }
    if (!pcRemoteApp || !xRinaNameFromString(pcRemoteApp, pxRemoteName))
    {
        LOGE(TAG_RINA, "RemoteName incorrect");
        goto err;
    }

    /*xPortId set to zero until the TASK fill properly.*/
    xPortId = xIPCPAllocatePortId();
    LOGI(TAG_RINA, "Port Id: %d Allocated", xPortId);

    /*Struct Data to sent attached into the event*/

    if (pxFlowAllocateRequest != NULL)
    {
        pxFlowAllocateRequest->xReceiveBlockTime = FLOW_DEFAULT_RECEIVE_BLOCK_TIME;
        pxFlowAllocateRequest->xSendBlockTime = FLOW_DEFAULT_SEND_BLOCK_TIME;

        pxFlowAllocateRequest->pxLocal = pxLocalName;
        pxFlowAllocateRequest->pxRemote = pxRemoteName;
        pxFlowAllocateRequest->pxDifName = pxDIFName;
        pxFlowAllocateRequest->pxFspec = pxFlowSpecTmp;
        pxFlowAllocateRequest->xPortId = xPortId;

        vRsListInit(&pxFlowAllocateRequest->xListWaitingPackets);

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
    }

    return pxFlowAllocateRequest;

err:
    if (pxFlowAllocateRequest)
        vRsMemFree(pxFlowAllocateRequest);
    if (pxFlowSpecTmp)
        vRsMemFree(pxFlowSpecTmp);
    if (pxDIFName)
        vRstrNameFree(pxDIFName);
    if (pxLocalName)
        vRstrNameFree(pxLocalName);
    if (pxRemoteName)
        vRstrNameFree(pxRemoteName);

    return NULL;
}

bool_t RINA_flowStatus(portId_t xAppPortId)
{
    return xNormalIsFlowAllocated(xAppPortId);
}

bool_t prvConnect(flowAllocateHandle_t *pxFlowAllocateRequest)
{
    bool_t xResult = 0;
    RINAStackEvent_t xStackFlowAllocateEvent = {
        .eEventType = eStackFlowAllocateEvent,
        .xData.PV = NULL};

    if (pxFlowAllocateRequest == NULL)
    {
        LOGE(TAG_RINA, "No flow request passed");
        xResult = false;
    }
    /* Check if the flow is already allocated */
    else if (RINA_flowStatus(pxFlowAllocateRequest->xPortId) == 1)
    {
        LOGE(TAG_RINA, "There is a flow allocated for port ID: %d", pxFlowAllocateRequest->xPortId);
        xResult = false;
    }
    xResult = xRINA_bind(pxFlowAllocateRequest);

    LOGE(TAG_RINA, "xresult=%d", xResult);

    /* FIXME: Maybe do a prebind? */
    if (xResult)
    {
        LOGD(TAG_RINA, "Sending Flow Allocate Request");
        vFlowAllocatorFlowRequest(pxFlowAllocateRequest->xPortId, pxFlowAllocateRequest);

        pxFlowAllocateRequest->usTimeout = 1U;

        if (xSendEventToIPCPTask(eFATimerEvent) != true)
        {
            LOGE(TAG_RINA, "Error sending timer event to IPCP");
            xResult = false;
        }
    }

    return xResult;
}

portId_t RINA_flow_alloc(string_t pcNameDIF,
                         string_t pcLocalApp,
                         string_t pcRemoteApp,
                         struct rinaFlowSpec_t *xFlowSpec,
                         uint8_t Flags);

portId_t RINA_flow_alloc(string_t pcNameDIF,
                         string_t pcLocalApp,
                         string_t pcRemoteApp,
                         struct rinaFlowSpec_t *xFlowSpec,
                         uint8_t Flags)
{

    flowAllocateHandle_t *pxFlowAllocateRequest;
    useconds_t xRemainingTime;
    bool_t xTimed = false; /* Check non-blocking*/
    struct RsTimeOut xTimeOut;
    bool_t xResult = false;

    pxFlowAllocateRequest = prvRINACreateFlowRequest(pcNameDIF, pcLocalApp, pcRemoteApp, xFlowSpec);

    LOGI(TAG_RINA, "Connecting to IPCP Task");

    xResult = prvConnect(pxFlowAllocateRequest);

    if (xResult == 0)
    {
        for (;;)
        {
            if (!xTimed)
            {
                xRemainingTime = pxFlowAllocateRequest->xReceiveBlockTime;

                if (xRemainingTime == 0)
                {
                    xResult = PORT_ID_WRONG;
                    break;
                }

                xTimed = true;

                if (xRsTimeSetTimeOut(&xTimeOut))
                {
                    LOGE(TAG_RINA, "Error initializing timeout object");
                    xResult = PORT_ID_WRONG;
                }
            }

            LOGI(TAG_RINA, "Checking Flow Status");
            xResult = RINA_flowStatus(pxFlowAllocateRequest->xPortId);

            if (xResult)
            {
                xResult = 0;
                break;
            }

            if (xRsTimeCheckTimeOut(&xTimeOut, &xRemainingTime) != false)
            {
                LOGE(TAG_RINA, "Error checking for flow allocation timeout expiration");
                xResult = PORT_ID_WRONG;
                break;
            }

            /* Is the timeout expired? */
            if (xRemainingTime == 0)
            {
                LOGE(TAG_RINA, "Flow allocation timed out");
                xResult = PORT_ID_WRONG;
                break;
            }

            /* The IPCP-task will set the 'eFLOW_BOUND' bit when it has done its
             * job. */
            prvWaitForBit(&pxFlowAllocateRequest->xEventCond,
                          &pxFlowAllocateRequest->xEventMutex,
                          &pxFlowAllocateRequest->nEventBits,
                          eFLOW_BOUND,
                          true);
        }
    }

    LOGI(TAG_RINA, "Flow allocated in the port Id:%d", pxFlowAllocateRequest->xPortId);

    return pxFlowAllocateRequest->xPortId;
}

size_t RINA_flow_write(portId_t xPortId, void *pvBuffer, size_t uxTotalDataLength)
{
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    void *pvCopyDest;
    struct RsTimeOut xTimeOut;
    useconds_t xTimeToWait, xTimeDiff;
    RINAStackEvent_t xStackTxEvent = {
        .eEventType = eStackTxEvent,
        .xData.PV = NULL};

    /* Check that DataLength is not longer than MAX_SDU_SIZE. We
     * don't know yet WHAT to do if this is not true so make sure we
     * don't fail silently in case this happens. */
    RsAssert(uxTotalDataLength <= MAX_SDU_SIZE);

    /*Check if the Flow is active*/
    xTimeToWait = 250 * 1000;

    if (!xRsTimeSetTimeOut(&xTimeOut))
    {
        LOGE(TAG_RINA, "xRsTimeSetTimeOut failed");
        return 0;
    }

    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(uxTotalDataLength, xTimeToWait);

    if (pxNetworkBuffer != NULL)
    {
        pvCopyDest = (void *)pxNetworkBuffer->pucEthernetBuffer;
        (void)memcpy(pvCopyDest, pvBuffer, uxTotalDataLength);

        /* This will update xTimeToWait with whatever is left to
           wait. */
        if (!xRsTimeCheckTimeOut(&xTimeOut, &xTimeToWait))
        {
            LOGE(TAG_RINA, "Error checking timeout expiration");
            return 0;
        }

        /*Fill the pxNetworkBuffer descriptor properly*/
        pxNetworkBuffer->xDataLength = uxTotalDataLength;
        pxNetworkBuffer->ulBoundPort = xPortId;

        xStackTxEvent.xData.PV = pxNetworkBuffer;

        if (xSendEventStructToIPCPTask(&xStackTxEvent, xTimeToWait))
            return uxTotalDataLength;
        else
        {
            vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
            return 0;
        }
    }
    else
    {
        LOGE(TAG_RINA, "Error allocating network buffer for writing");
        return 0;
    }

    return 0;
}

bool_t RINA_close(portId_t xAppPortId)
{
    bool_t xResult;

    RINAStackEvent_t xDeallocateEvent;
    xDeallocateEvent.eEventType = eFlowDeallocateEvent;
    xDeallocateEvent.xData.UN = xAppPortId;

    if (!is_port_id_ok(xAppPortId))
        xResult = false;
    else
    {
        if (!xSendEventStructToIPCPTask(&xDeallocateEvent, 0))
        {
            LOGI(TAG_RINA, "RINA Deallocate Flow: failed");
            xResult = false;
        }
        else
            xResult = true;
    }

    return xResult;
}

int32_t RINA_flow_read(portId_t xPortId, void *pvBuffer, size_t uxTotalDataLength)
{
    size_t xPacketCount;
    const void *pvCopySource; // to copy data from networkBuffer to pvBuffer
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    flowAllocateHandle_t *pxFlowHandle;
    useconds_t xRemainingTime;
    bool_t xTimed = false;
    struct RsTimeOut xTimeOut;
    int32_t lDataLength;
    size_t uxPayloadLength;

    // Validate if the flow is valid, if the xPortId is working status CONNECTED
    if (!RINA_flowStatus(xPortId))
    {
        LOGE(TAG_RINA, "No flow for port ID: %lu", xPortId);
        return 0;
    }
    else
    {
        // find the flow handle associated to the xPortId.
        pxFlowHandle = pxFAFindFlowHandle(xPortId);
        xPacketCount = unRsListLength(&(pxFlowHandle->xListWaitingPackets));

        LOGD(TAG_RINA, "Numbers of packet in the queue to read: %zu", xPacketCount);

        while (xPacketCount == 0)
        {

            if (xTimed == false)
            {
                /* Check to see if the flow is non blocking on the first
                 * iteration.  */
                xRemainingTime = pxFlowHandle->xReceiveBlockTime;
#if __LONG_WIDTH__ == 32
                LOGD(TAG_RINA, "xRemainingTime: %lu", xRemainingTime);
#elif __LONG_WIDTH__ == 64
                LOGD(TAG_RINA, "xRemainingTime: %u", xRemainingTime);
#else
#error Not sure how to handle this size of __LONG_WIDTH__
#endif

                if (xRemainingTime == 0)
                {
                    /* Check for the interrupt flag. */
#if __LONG_WIDTH__ == 32
                    LOGD(TAG_RINA, "xRemainingTime: %lu", xRemainingTime);
#elif __LONG_WIDTH__ == 64
                    LOGD(TAG_RINA, "xRemainingTime: %u", xRemainingTime);
#else
#error Not sure how to handle this size of __LONG_WIDTH__
#endif
                    break;
                }

                /* To ensure this part only executes once. */
                xTimed = true;

                /* Fetch the current time. */
                if (!xRsTimeSetTimeOut(&xTimeOut))
                {
                    LOGE(TAG_RINA, "Error initializing timeout object");
                    break;
                }
            }

            prvWaitForBit(&pxFlowHandle->xEventCond,
                          &pxFlowHandle->xEventMutex,
                          &pxFlowHandle->nEventBits,
                          eFLOW_RECEIVE,
                          true);

            xPacketCount = unRsListLength(&(pxFlowHandle->xListWaitingPackets));

            if (xPacketCount != 0)
                break;

            if (!xRsTimeCheckTimeOut(&xTimeOut, &xRemainingTime))
            {
                LOGE(TAG_RINA, "Error checking write timeout expiration");
                return 0;
            }
#if __LONG_WIDTH__ == 32
            LOGD(TAG_RINA, "xRemainingTime: %lu", xRemainingTime);
#elif __LONG_WIDTH__ == 64
            LOGD(TAG_RINA, "xRemainingTime: %u", xRemainingTime);
#else
#error Not sure how to handle this size of __LONG_WIDTH__
#endif
        }

        if (xPacketCount != 0)
        {
            pthread_mutex_lock(&mux);
            {
                /* The owner of the list item is the network buffer. */
                pxNetworkBuffer = (NetworkBufferDescriptor_t *)pxRsListGetHeadOwner((&pxFlowHandle->xListWaitingPackets));
                vRsListRemove(&(pxNetworkBuffer->xBufferListItem));
            }
            pthread_mutex_unlock(&mux);

            lDataLength = (int32_t)pxNetworkBuffer->xDataLength;

            // vPrintBytes((void *)pxNetworkBuffer->pucDataBuffer, lDataLength);

            (void)memcpy(pvBuffer, (const void *)pxNetworkBuffer->pucDataBuffer, (size_t)lDataLength);
            vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
        }
        else
        {
            LOGE(TAG_RINA, "Timeout reading from flow");
            lDataLength = -1;
        }
    }

    return lDataLength;
}
