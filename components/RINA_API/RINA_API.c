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
#include "common/netbuf.h"
#include "common/rsrc.h"
#include "common/num_mgr.h"
#include "common/list.h"
#include "common/rina_ids.h"
#include "common/rina_name.h"

#include "configSensor.h"

#include "IpcManager.h"
#include "RINA_API_flows.h"
#include "rina_common_port.h"
#include "RINA_API.h"
#include "IPCP.h"
#include "IPCP_api.h"
#include "IPCP_normal_api.h"
#include "IPCP_normal_defs.h"
#include "IPCP_events.h"
#include "FlowAllocator_api.h"

static pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

/* This is the normal IPCP instance the RINA_API will address. */
static struct ipcpInstance_t *pxIpcp;

/* The pool of netbufs */
rsrcPoolP_t xPool;

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
    rname_t *xDn, *xAppn, *xDan;
    registerApplicationHandle_t *xRegAppRequest;
    RINAStackEvent_t xStackAppRegistrationEvent = {
        .eEventType = eStackAppRegistrationEvent,
        .xData.PV = NULL
    };

    xRegAppRequest = pvRsMemAlloc(sizeof(registerApplicationHandle_t));
    if (!xRegAppRequest) {
        LOGE(TAG_RINA, "Failed to allocate memory for registration handle");
        return NULL;
    }

    /*Check Flags (RINA_F_NOWAIT)*/
#if 0

    /*Create name_t objects*/
    xDn = pxRStrNameCreate();
    xAppn = pxRStrNameCreate();
    xDan = pxRStrNameCreate();
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

    if (!pxFlowRequest) {
        LOGE(TAG_RINA, "No flow Request");
        return false;
    }

    xBindEvent.eEventType = eFlowBindEvent;
    xBindEvent.xData.PV = (void *)pxFlowRequest;

    if (xSendEventStructToIPCPTask(&xBindEvent, 0) == false)
        /* Failed to wake-up the IPCP-task, no use to wait for it */
        return false;
    else {
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
    portId_t xPortId; /* PortId to return to the user*/
    flowAllocateHandle_t *pxFlowAllocateRequest = NULL;

    LOGI(TAG_RINA, "Creating a new flow request");

    if (!(pxFlowAllocateRequest = pvRsMemCAlloc(1, sizeof(*pxFlowAllocateRequest)))) {
        LOGE(TAG_RINA, "Failed to allocate memory for flow request");
        goto err;
    }

    if (pthread_cond_init(&pxFlowAllocateRequest->xEventCond, NULL) != 0) {
        LOGE(TAG_RINA, "Failed to initialize thread condition signal");
        goto err;
    }

    if (pthread_mutex_init(&pxFlowAllocateRequest->xEventMutex, NULL) != 0) {
        LOGE(TAG_RINA, "Failed to initialize thread condition signal");
        pthread_cond_destroy(&pxFlowAllocateRequest->xEventCond);
        goto err;
    }

    pxFlowAllocateRequest->nEventBits = 0;

    if (ERR_CHK(xNameAssignFromString(&pxFlowAllocateRequest->xDifName, pcNameDIF))) {
        LOGE(TAG_RINA, "Invalid DIF name");
        goto err;
    }

    if (ERR_CHK(xNameAssignFromString(&pxFlowAllocateRequest->xLocal, pcLocalApp))) {
        LOGE(TAG_RINA, "Invalid local name");
        goto err;
    }

    if (ERR_CHK(xNameAssignFromString(&pxFlowAllocateRequest->xRemote, pcRemoteApp))) {
        LOGE(TAG_RINA, "Invalid remote name");
        goto err;
    }

    /*xPortId set to zero until the TASK fill properly.*/
    //xPortId = xIPCPAllocatePortId();
    xPortId = unIpcManagerReservePort(&xIpcManager);
    LOGI(TAG_RINA, "Port Id: %d Allocated", xPortId);

    /*Struct Data to sent attached into the event*/

    pxFlowAllocateRequest->xReceiveBlockTime = FLOW_DEFAULT_RECEIVE_BLOCK_TIME;
    pxFlowAllocateRequest->xSendBlockTime = FLOW_DEFAULT_SEND_BLOCK_TIME;
    pxFlowAllocateRequest->xPortId = xPortId;

    vRsListInit(&pxFlowAllocateRequest->xListWaitingPackets);

    if (!xFlowSpec) {
        pxFlowAllocateRequest->xFspec.ulAverageBandwidth = 0;
        pxFlowAllocateRequest->xFspec.ulAverageSduBandwidth = 0;
        pxFlowAllocateRequest->xFspec.ulDelay = 0;
        pxFlowAllocateRequest->xFspec.ulJitter = 0;
        pxFlowAllocateRequest->xFspec.usLoss = 10000;
        pxFlowAllocateRequest->xFspec.ulMaxAllowableGap = 10;
        pxFlowAllocateRequest->xFspec.xOrderedDelivery = false;
        pxFlowAllocateRequest->xFspec.ulUndetectedBitErrorRate = 0;
        pxFlowAllocateRequest->xFspec.xPartialDelivery = true;
        pxFlowAllocateRequest->xFspec.xMsgBoundaries = false;
    } else {
        pxFlowAllocateRequest->xFspec.ulAverageBandwidth = xFlowSpec->avg_bandwidth;
        pxFlowAllocateRequest->xFspec.ulAverageSduBandwidth = 0;
        pxFlowAllocateRequest->xFspec.ulDelay = xFlowSpec->max_delay;
        pxFlowAllocateRequest->xFspec.ulJitter = xFlowSpec->max_jitter;
        pxFlowAllocateRequest->xFspec.usLoss = xFlowSpec->max_loss;
        pxFlowAllocateRequest->xFspec.ulMaxAllowableGap = xFlowSpec->max_sdu_gap;
        pxFlowAllocateRequest->xFspec.xOrderedDelivery = xFlowSpec->in_order_delivery;
        pxFlowAllocateRequest->xFspec.ulUndetectedBitErrorRate = 0;
        pxFlowAllocateRequest->xFspec.xPartialDelivery = true;
        pxFlowAllocateRequest->xFspec.xMsgBoundaries = xFlowSpec->msg_boundaries;
    }

    return pxFlowAllocateRequest;

    err:
    if (pxFlowAllocateRequest)
        vRsMemFree(pxFlowAllocateRequest);

    return NULL;
}

bool_t RINA_flowStatus(portId_t xAppPortId)
{
    struct ipcpInstance_t *pxIpcp;
    bool_t xStatus;

    RsAssert((pxIpcp = pxIpcManagerFindByType(&xIpcManager, eNormal)));

    return xNormalIsFlowAllocated(pxIpcp, xAppPortId);
}

bool_t prvConnect(flowAllocator_t *pxFA, flowAllocateHandle_t *pxFlowAllocateRequest)
{
    bool_t xResult = 0;
    RINAStackEvent_t xStackFlowAllocateEvent = {
        .eEventType = eStackFlowAllocateEvent,
        .xData.PV = NULL
    };

    if (pxFlowAllocateRequest == NULL) {
        LOGE(TAG_RINA, "No flow request passed");
        xResult = false;
    }
    /* Check if the flow is already allocated */
    else if (RINA_flowStatus(pxFlowAllocateRequest->xPortId) == 1) {
        LOGE(TAG_RINA, "There is a flow allocated for port ID: %d", pxFlowAllocateRequest->xPortId);
        xResult = false;
    }
    xResult = xRINA_bind(pxFlowAllocateRequest);

    /* FIXME: Maybe do a prebind? */
    if (xResult) {
        vFlowAllocatorFlowRequest(pxFA, pxFlowAllocateRequest->xPortId, pxFlowAllocateRequest);

        pxFlowAllocateRequest->usTimeout = 1U;

        if (xSendEventToIPCPTask(eFATimerEvent) != true) {
            LOGE(TAG_RINA, "Error sending timer event to IPCP");
            xResult = false;
        }
    }

    return xResult;
}

/**
 * Initialize the RINA API globals
 */
void RINA_Init()
{
    RsAssert(xIpcManagerInit(&xIpcManager));

    RINA_IPCPInit();

    /* FIXME: This picks the first normal IPCP instance found in the
       list. */
    RsAssert((pxIpcp = pxIpcManagerFindByType(&xIpcManager, eNormal)));

    /* FIXME: We should handle failure here but I can't be bothered
       for now as it seems very unlikely that we'll run out of memory
       right at initialisation. */
    RsAssert((xPool = xNetBufNewPool("RINA_API Netbufs")));
}

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

    xResult = prvConnect(&pxIpcp->pxData->xFA, pxFlowAllocateRequest);

    if (xResult == 0) {
        for (;;) {
            if (!xTimed) {
                xRemainingTime = pxFlowAllocateRequest->xReceiveBlockTime;

                if (xRemainingTime == 0) {
                    xResult = PORT_ID_WRONG;
                    break;
                }

                xTimed = true;

                if (xRsTimeSetTimeOut(&xTimeOut)) {
                    LOGE(TAG_RINA, "Error initializing timeout object");
                    xResult = PORT_ID_WRONG;
                }
            }

            LOGI(TAG_RINA, "Checking Flow Status");
            xResult = RINA_flowStatus(pxFlowAllocateRequest->xPortId);

            if (xResult) {
                xResult = 0;
                break;
            }

            if (xRsTimeCheckTimeOut(&xTimeOut, &xRemainingTime) != false) {
                LOGE(TAG_RINA, "Error checking for flow allocation timeout expiration");
                xResult = PORT_ID_WRONG;
                break;
            }

            /* Is the timeout expired? */
            if (xRemainingTime == 0) {
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

size_t RINA_flow_write(portId_t unPort, void *pvBuffer, size_t uxTotalDataLength)
{
    netbuf_t *pxNb;
    void *pvCopyDest;
    struct RsTimeOut xTimeOut;
    useconds_t xTimeToWait, xTimeDiff;
    RINAStackEvent_t xTxEvent = {
        .eEventType = eRinaTxEvent,
        .xData.PV = NULL
    };

    /* Check that DataLength is not longer than MAX_SDU_SIZE. We
     * don't know yet WHAT to do if this is not true so make sure we
     * don't fail silently in case this happens. */
    RsAssert(uxTotalDataLength <= MAX_SDU_SIZE);

    /*Check if the Flow is active*/
    xTimeToWait = 250 * 1000;

    if (!xRsTimeSetTimeOut(&xTimeOut)) {
        LOGE(TAG_RINA, "xRsTimeSetTimeOut failed");
        return 0;
    }

    /* We're not freeing the buffer as it's owned by the app */
    pxNb = pxNetBufNew(xPool, NB_RINA_DATA, pvBuffer, uxTotalDataLength, NETBUF_FREE_DONT);

    if (pxNb != NULL) {
        /* This will update xTimeToWait with whatever is left to
           wait. */
        if (!xRsTimeCheckTimeOut(&xTimeOut, &xTimeToWait)) {
            LOGE(TAG_RINA, "Error checking timeout expiration");
            return 0;
        }

        xTxEvent.xData.UN = unPort;
        xTxEvent.xData2.PV = pxNb;

        if (xSendEventStructToIPCPTask(&xTxEvent, xTimeToWait))
            return uxTotalDataLength;
        else {
            vNetBufFree(pxNb);
            return -1;
        }
    }
    else {
        LOGE(TAG_RINA, "Error allocating network buffer for writing");
        return -1;
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
    else {
        if (!xSendEventStructToIPCPTask(&xDeallocateEvent, 0)) {
            LOGI(TAG_RINA, "RINA Deallocate Flow: failed");
            xResult = false;
        }
        else xResult = true;
    }

    return xResult;
}

int32_t RINA_flow_read(portId_t xPortId, void *pvBuffer, size_t uxTotalDataLength)
{
    size_t xPacketCount;
    netbuf_t *pxNb;
    flowAllocateHandle_t *pxFlowHandle;
    useconds_t xRemainingTime;
    bool_t xTimed = false;
    struct RsTimeOut xTimeOut;
    int32_t lDataLength;
    size_t uxPayloadLength, unIdx;

    /* Validate if the flow is valid, if the xPortId is working status
       CONNECTED */
    if (!RINA_flowStatus(xPortId)) {
        LOGE(TAG_RINA, "No flow for port ID: %u", xPortId);
        return 0;
    } else {
        /* find the flow handle associated to the xPortId */
        pxFlowHandle = pxFAFindFlowHandle(&pxIpcp->pxData->xFA, xPortId);
        xPacketCount = unRsListLength(&(pxFlowHandle->xListWaitingPackets));

        LOGD(TAG_RINA, "Numbers of packet in the queue to read: %zu", xPacketCount);

        while (xPacketCount == 0) {

            if (xTimed == false) {
                /* Check to see if the flow is non blocking on the first
                 * iteration.  */
                xRemainingTime = pxFlowHandle->xReceiveBlockTime;

                if (xRemainingTime == 0)
                    break;

                /* To ensure this part only executes once. */
                xTimed = true;

                /* Fetch the current time. */
                if (!xRsTimeSetTimeOut(&xTimeOut)) {
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

            if (!xRsTimeCheckTimeOut(&xTimeOut, &xRemainingTime)) {
                LOGE(TAG_RINA, "Error checking write timeout expiration");
                return 0;
            }
        }

        if (xPacketCount != 0) {
            pthread_mutex_lock(&mux);
            {
                /* The owner of the list item is the network buffer. */
                pxNb = (netbuf_t *)pxRsListGetHeadOwner((&pxFlowHandle->xListWaitingPackets));
                vRsListRemove(&(pxNb->xListItem));
            }
            pthread_mutex_unlock(&mux);

            lDataLength = unNetBufTotalSize(pxNb);
            unIdx = 0;

            unNetBufRead(pxNb, pvBuffer, 0, lDataLength);
            vNetBufFree(pxNb);
        }
        else {
            LOGE(TAG_RINA, "Timeout reading from flow");
            lDataLength = -1;
        }
    }

    return lDataLength;
}
