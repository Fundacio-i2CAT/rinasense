#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#endif

#include "portability/port.h"

#include "common/error.h"
#include "common/queue.h"
#include "common/simple_queue.h"
#include "common/rina_ids.h"

#include "ARP826.h"
#include "ARP826_defs.h"
#include "configRINA.h"
#include "IpcManager.h"
#include "IPCP.h"
#include "IPCP_api.h"
#include "IPCP_events.h"
#include "IPCP_normal_api.h"
#include "NetworkInterface.h"
#include "RINA_API_flows.h"
#include "ShimIPCP.h"
#include "IpcManager.h"
#include "Enrollment_api.h"
#include "rina_common_port.h"
#include "RINA_API.h"

/** @brief The queue used to pass events into the IPCP-task for processing. */
RsQueue_t xNetworkEventQueue;

/** @brief Set to pdTRUE when the IPCP task is ready to start processing packets. */
static bool_t xIPCPTaskInitialised = false;

/** @brief Simple set to pdTRUE or pdFALSE depending on whether the network is up or
 * down (connected, not connected) respectively. */
static bool_t xNetworkUp = false;

/** @brief Stores the handle of the task that handles the stack.  The handle is used
 * (indirectly) by some utility function to determine if the utility function is
 * being called by a task (in which case it is ok to block) or by the IPCP task
 * itself (in which case it is not ok to block). */
static pthread_t xIPCPThread;

#ifdef ESP_PLATFORM
/* This saves the FreeRTOS current task handle. */
static TaskHandle_t xIPCPTaskHandle;
#endif

/*********************************************************/

/* RIBD module */

/* Enrollment Task */

/* Flow Allocator */

/**************************************/
struct ipcpInstance_t *pxNormalInstance;

/** @brief ARP timer, to check its table entries. */
static IPCPTimer_t xARPTimer;

static IPCPTimer_t xFATimer;

void RINA_NetworkDown(void);

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer);

EthernetHeader_t *vCastPointerTo_EthernetPacket_t(const void *pvArgument);

/*----------------------------------------------*/
/*
 * The main IPCP stack processing task.  This task receives commands/events
 * from the network hardware drivers and tasks.  It also
 * maintains a set of protocol timers.
 */
static void *prvIPCPTask(void *pvParameters);

static bool_t prvIPCPTimerCheck(IPCPTimer_t *pxTimer);

void vIpcpSetFATimerExpiredState(bool_t xExpiredState);

/*
 * Utility functions for the light weight IP timers.
 */
static void prvIPCPTimerStart(IPCPTimer_t *pxTimer, useconds_t xTimeUS);

static bool_t prvIPCPTimerCheck(IPCPTimer_t *pxTimer);

#if 0
static void prvIPCPTimerReload(IPCPTimer_t *pxTimer, useconds_t xTimeUS);
#endif

/*
 * Returns true if the IP task has been created and is initialised.  Otherwise
 * returns false.
 */
bool_t xIPCPIsNetworkTaskReady(void);

/*
 * Checks the ARP, timers to see if any periodic or timeout
 * processing is required.
 */
static void prvCheckNetworkTimers(void);

/*
 * Determine how long the IPCP task can sleep for, which depends on when the next
 * periodic or timeout processing must be performed.
 */
static long prvCalculateSleepTimeUS();

/*
 * Called to create a network connection when the stack is first started, or
 * when the network connection is lost.
 */
static void prvProcessNetworkDownEvent(void);

bool_t xCreateIPCPModules(void);

void prvIPCPSetAttributes(void);

/*----------------------------------------------------*/

bool_t xIPCPTaskReady(void)
{
    return xIPCPTaskInitialised;
}

/**
 * @brief The IPCP task handles all requests from the user application and the
 *        network interface. It receives messages through a FreeRTOS queue called
 *        'xNetworkEventQueue'. prvIPTask() is the only task which has access to
 *        the data of the IPCP-stack, and so it has no need of using mutexes.
 *
 * @param[in] pvParameters: Not used.
 */
static void *prvIPCPTask(void *pvParameters)
{
    RINAStackEvent_t xEv;
    flowAllocateHandle_t *pxFlowAllocateHandle;
    useconds_t xSleepTimeUS;

    /* Just to prevent compiler warnings about unused parameters. */
    (void)pvParameters;

#ifdef ESP_PLATFORM
    /* FIXME: Protect with mutex. */
    xIPCPTaskHandle = xTaskGetCurrentTaskHandle();
#endif

    /* Initialization is complete and events can now be processed. */
    xIPCPTaskInitialised = true;

    LOGI(TAG_IPCPMANAGER, "ENTER: IPC Manager Thread");

    /* Enable the IPC process */
    xIpcManagerRunEnable();

    /* Loop, processing IP events. */
    for (;;) {
        // ipconfigWATCHDOG_TIMER();

        prvCheckNetworkTimers();

        /* Calculate the acceptable maximum sleep time. */
        xSleepTimeUS = prvCalculateSleepTimeUS();

        /* Wait until there is something to do. If the following call exits
         * due to a time out rather than a message being received, set a
         * 'NoEvent' value. */
        //LOGI(TAG_IPCPMANAGER, "Sleeping for %lu usecond(s)", xSleepTimeUS);
        if (ERR_CHK(xRsQueueReceiveTimed(&xNetworkEventQueue, (void *)&xEv, xSleepTimeUS)))
            xEv.eEventType = eNoEvent;

        switch (xEv.eEventType)
        {
        case eRinaRxEvent: {
            struct ipcpInstance_t *pxInstance;
            du_t *pxDu;
            portId_t unPort;
            bool_t xStatus;

            /*
             * A RINA packet was received by a shim module and wants
             * to go up the stack. In the future we will need to
             * determine where to send it, but right now just send it
             * to the first normal IPCP we find.
             */

            RsAssert((pxInstance = pxIpcManagerFindByType(&xIpcManager, eNormal)));

            unPort = xEv.xData.UN;
            pxDu = xEv.xData2.PV;

            CALL_IPCP_CHECK(xStatus, pxInstance, duEnqueue, unPort, pxDu) {
                LOGE(TAG_IPCPMANAGER, "Failed to receive DU");
            }

            break;
        }

        case eNetworkDownEvent:
        {
            struct timespec ts = {CFG_INITIALISATION_RETRY_DELAY_SEC, 0};

            /* FIXME: THIS IS ENTIRELY BROKEN AND UNTESTED AND IN NEED
             * OF SOME OR MANY LOVING. */

            LOGD(TAG_IPCPMANAGER, "Event: eNetworkDownEvent");

            /* Attempt to establish a connection. */
            nanosleep(&ts, NULL);
            LOGI(TAG_IPCPMANAGER, "eNetworkDownEvent");
            xNetworkUp = false;
            prvProcessNetworkDownEvent();
            break;
        }

        case eShimEnrolledEvent: {
            struct ipcpInstance_t *pxShimInstance, *pxNormalInstance;
            bool_t xStatus;
            const rname_t *ipcpName, *difName;

            LOGD(TAG_IPCPMANAGER, "Event: eShimEnrolledEvent");

            pxShimInstance = (struct ipcpInstance_t *)xEv.xData.PV;
            RsAssert((pxNormalInstance = pxIpcManagerFindByType(&xIpcManager, eNormal)));

            ipcpName = xNormalGetIpcpName(pxNormalInstance);
            difName = xNormalGetDifName(pxNormalInstance);

            CALL_IPCP_CHECK(xStatus, pxShimInstance, applicationRegister, ipcpName, difName) {
                LOGE(TAG_IPCPMANAGER, "IPCP not registered into the shim");
            }

            break;
        }

        case eShimFlowAllocatedEvent: {
            struct ipcpInstance_t *pxInstance, *pxShimInst;
            portId_t unPort;

            LOGD(TAG_IPCPMANAGER, "Event: eShimFlowAllocatedEvent");

            RsAssert((pxInstance = pxIpcManagerFindByType(&xIpcManager, eNormal)));

            unPort = xEv.xData.UN;
            pxShimInst = xEv.xData2.PV;

            CALL_IPCP(pxInstance, flowBindingIpcp, unPort, pxShimInst);

            //if (!xNormalFlowBinding(pxInstance->pxData, unPort, pxNormalIinstance))
            //    LOGW(TAG_IPCPMANAGER, "Failed to bind port %u on normal IPC", unPort);

            //if (!xEnrollmentInit(pxInstance->pxData, unPort))
            //    LOGW(TAG_IPCPMANAGER, "Failed initialize enrollment module on port %u", unPort);

            break;
        }

        case eFATimerEvent:
            LOGD(TAG_IPCPMANAGER, "Event: eFATimerEvent");
            vIpcpSetFATimerExpiredState(true);
            break;

        case eFlowDeallocateEvent:
            LOGD(TAG_IPCPMANAGER, "Event: eFlowDeallocateEvent");
            //(void)vFlowAllocatorDeallocate((portId_t *)xReceivedEvent.pvData);
            break;

        case eFlowBindEvent: {
            struct ipcpInstance_t *pxInstance;

            LOGD(TAG_IPCPMANAGER, "Event: eFlowBindEvent");

            RsAssert((pxInstance = pxIpcManagerFindByType(&xIpcManager, eNormal)));

            pxFlowAllocateHandle = (flowAllocateHandle_t *)xEv.xData.PV;

            //(void)xNormalFlowPrebind(pxNormalIpcpData,
            //pxFlowAllocateHandle);
            CALL_IPCP(pxInstance, flowPrebind, pxFlowAllocateHandle);

#if 0
            pxFlowAllocateRequest->xEventBits |= (EventBits_t)eFLOW_BOUND;
            vRINA_WeakUpUser(pxFlowAllocateRequest);
#endif

            vRINA_WakeUpFlowRequest(pxFlowAllocateHandle, eFLOW_BOUND);

            break;
        }
        case eSendMgmtEvent: {
            struct ipcpInstance_t *pxInstance;
            bool_t xStatus;

            LOGD(TAG_IPCPMANAGER, "Event: eSendMgmtEvent");

            RsAssert((pxInstance = pxIpcManagerFindByType(&xIpcManager, eNormal)));

            CALL_IPCP_CHECK(xStatus, pxInstance, mgmtDuWrite, xEv.xData.UN, xEv.xData2.DU) {
                LOGE(TAG_IPCPMANAGER, "Failed to sent management PDU");
            }

            break;
        }

        case eNoEvent:
            /* xQueueReceive() returned because of a normal time-out. */
            break;

        default:
            /* Should not get here. */
            break;
        }
    }

    LOGI(TAG_IPCPMANAGER, "EXIT: IPC Manager Thread");
    return NULL;
}
/*-----------------------------------------------------------*/

/**
 * @brief Initialize the RINA network stack and initialize the IPCP-task.
 *
 *
 * @return pdPASS if the task was successfully created and added to a ready
 * list, otherwise an error code defined in the file projdefs.h
 */
rsErr_t RINA_IPCPInit()
{
    int n;
    pthread_attr_t attr;

    RsQueueParams_t xIpcpQueueParams = {
        .unMaxItemCount = 255,
        .xBlock = true,
        .unItemSz = sizeof(RINAStackEvent_t)
    };

    LOGI(TAG_IPCPMANAGER, "************* INIT RINA ***********");

    /* ***************************************************** */

    /* FIXME: TEMPORARY. THIS NEEDS TO BE REPLACED BY A CONFIGURATION
     * API. */
    struct ipcpInstance_t *pxNormalIpcp, *pxShimIpcp;
    ipcProcessId_t unNormalIpcpId, unShimIpcpId;

    unNormalIpcpId = unIpcManagerReservePort(&xIpcManager);
    unShimIpcpId = unIpcManagerReservePort(&xIpcManager);

    RsAssert((pxNormalIpcp = pxNormalCreate(unNormalIpcpId)));
    RsAssert((pxShimIpcp = pxShimWiFiCreate(unShimIpcpId)));

    vIpcManagerAdd(&xIpcManager, pxNormalIpcp);
    vIpcManagerAdd(&xIpcManager, pxShimIpcp);

    /* ***************************************************** */

    /* This function should only be called once. */
    RsAssert(xIPCPIsNetworkTaskReady() == false);

    if (ERR_CHK(xRsQueueInit("IPCPQueue", &xNetworkEventQueue, &xIpcpQueueParams))) {
        vErrorLog(TAG_IPCPMANAGER, "IPCP Queue Initialization");
        abort();
    }

    /* Tell the IPC to start. */
    xIpcManagerRunStart();

    if ((n = pthread_attr_init(&attr) != 0))
        return ERR_SET_PTHREAD(n);

    pthread_attr_setstacksize(&attr, CFG_IPCP_TASK_STACK_SIZE);

    if ((n = pthread_create(&xIPCPThread, &attr, prvIPCPTask, NULL)) != 0)
        return ERR_SET_PTHREAD(n);

    pthread_attr_destroy(&attr);

    return SUCCESS;
}
/*----------------------------------------------------------*/
/**
 * @brief Function to check whether the current context belongs to
 *        the IPCP-task.
 *
 * @return If the current context belongs to the IPCP-task, then pdTRUE is
 *         returned. Else pdFALSE is returned.
 *
 * @note Very important: the IPCP-task is not allowed to call its own API's,
 *        because it would easily get into a dead-lock.
 */
bool_t xIsCallingFromIPCPTask(void)
{
#ifdef ESP_PLATFORM
    /* On ESP32, we use the FreeRTOS API here because calling
     * pthread_self on a FreeRTOS task will crash the runtime with an
     * assertion failure. You can't call this method on a task that
     * has not been started with the pthread API. */
    return xTaskGetCurrentTaskHandle() == xIPCPTaskHandle;
#else
    return pthread_self() == xIPCPThread;
#endif
}

/**
 * @brief Send an event to the IPCP task. It calls 'xSendEventStructToIPCPTask' internally.
 *
 * @param[in] eEvent: The event to be sent.
 *
 * @return pdPASS if the event was sent (or the desired effect was achieved). Else, pdFAIL.
 */
rsErr_t xSendEventToIPCPTask(eRINAEvent_t eEvent)
{
    RINAStackEvent_t xEventMessage;

    xEventMessage.eEventType = eEvent;
    xEventMessage.xData.PV = (void *)NULL;

    return xSendEventStructToIPCPTask(&xEventMessage, 0);
}

/**
 * @brief Send an event (in form of struct) to the IP task to be processed.
 *
 * @param[in] pxEvent: The event to be sent.
 * @param[in] uxTimeout: Timeout for waiting in case the queue is full. 0 for non-blocking calls.
 *
 * @return pdPASS if the event was sent (or the desired effect was achieved). Else, pdFAIL.
 */
rsErr_t xSendEventStructToIPCPTask(const RINAStackEvent_t *pxEvent, useconds_t xTimeOutUS)
{
    rsErr_t xStatus;
    useconds_t xCalculatedTimeOutUS;

    if (!xIPCPTaskReady() && (pxEvent->eEventType != eNetworkDownEvent))
    {
        LOGE(TAG_IPCPMANAGER, "IPCP Task Not Ready, cannot send event");

        /* Only allow eNetworkDownEvent events if the IP task is not ready
         * yet.  Not going to attempt to send the message so the send failed. */
        xStatus = FAIL;
    }
    else
    {
        /* The IP task cannot block itself while waiting for
         * itself to respond. */
        if (xIsCallingFromIPCPTask())
            xCalculatedTimeOutUS = 0;
        else
            xCalculatedTimeOutUS = xTimeOutUS;

        xStatus = xRsQueueSendToBackTimed(&xNetworkEventQueue, (void *)pxEvent, xCalculatedTimeOutUS);

        if (ERR_CHK(xStatus))
            LOGE(TAG_IPCPMANAGER, "Failed to add message to IPCP queue");
    }

    return xStatus;
}

/*-----------------------------------------------------------*/


/* FIXME: THIS NEEDS TO BE MOVED OUT OF IPCP.c, POSSIBLY IN
 * THE IPCMANAGER THEN IN THE RESPECTIVE SHIMS. */
eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetFrame)
{
    eFrameProcessingResult_t eReturn = eReleaseBuffer;
    const EthernetHeader_t *pxEthernetHeader;
    uint16_t usFrameType;

    /* Map the buffer onto Ethernet Header struct for easy access to fields. */
    pxEthernetHeader = (EthernetHeader_t *)pucEthernetFrame;

    usFrameType = RsNtoHS(pxEthernetHeader->usFrameType);

    // Just ETH_P_ARP and ETH_P_RINA Should be processed by the stack
    if (usFrameType == ETH_P_RINA_ARP || usFrameType == ETH_P_RINA) {
        eReturn = eProcessBuffer;
        LOGD(TAG_IPCPMANAGER, "Ethernet packet of type %04X: ACCEPTED", usFrameType);
    }
    else
        LOGD(TAG_IPCPMANAGER, "Ethernet packet of type %04X: REJECTED", usFrameType);

    return eReturn;
}

/*-----------------------------------------------------------*/

/**
 * @brief Calculate the maximum sleep time remaining. It will go through all
 *        timers to see which timer will expire first. That will be the amount
 *        of time to block in the next call to xQueueReceive().
 *
 * @return The maximum sleep time or ipconfigMAX_IP_TASK_SLEEP_TIME,
 *         whichever is smaller.
 */
static long prvCalculateSleepTimeUS()
{
    long xMaximumSleepTimeUS;

    /* Start with the maximum sleep time, then check this against the remaining
     * time in any other timers that are active. */
    xMaximumSleepTimeUS = CFG_MAX_IPCP_TASK_SLEEP_TIME_US;

    if (xARPTimer.bActive && xARPTimer.ulRemainingTimeUS < xMaximumSleepTimeUS)
        xMaximumSleepTimeUS = xARPTimer.ulRemainingTimeUS;

    return xMaximumSleepTimeUS;
}

/**
 * @brief Check the network timers (ARP/DTP) and if they are
 *        expired, send an event to the IPCP-Task.
 */
static void prvCheckNetworkTimers(void)
{
    /* Is it time for ARP processing? */
    if (prvIPCPTimerCheck(&xARPTimer) != false)
        (void)xSendEventToIPCPTask(eARPTimerEvent);
}

/*-----------------------------------------------------------*/

/**
 * @brief Process a 'Network down' event and complete required processing.
 */
static void prvProcessNetworkDownEvent(void)
{
    /* Stop the ARP timer while there is no network. */
    xARPTimer.bActive = false;

    /* Per the ARP Cache Validation section of https://tools.ietf.org/html/rfc1122,
     * treat network down as a "delivery problem" and flush the ARP cache for this
     * interface. */
    // RINA_vARPRemove(  );

    /* The network has been disconnected (or is being initialised for the first
     * time).  Perform whatever hardware processing is necessary to bring it up
     * again, or wait for it to be available again.  This is hardware dependent. */
#if 0
    if (xShimWiFiInit() != pdTRUE)
    {
        /* Ideally the network interface initialisation function will only
         * return when the network is available.  In case this is not the case,
         * wait a while before retrying the initialisation. */
        vTaskDelay(CFG_INITIALISATION_RETRY_DELAY);
        RINA_NetworkDown();
    }
    vTaskDelay(CFG_INITIALISATION_RETRY_DELAY);
#endif
}
/*-----------------------------------------------------------*/
/**
 * @brief Check the IP timer to see whether an IP event should be processed or not.
 *
 * @param[in] pxTimer: Pointer to the IP timer.
 *
 * @return If the timer is expired then pdTRUE is returned. Else pdFALSE.
 */
static bool_t prvIPCPTimerCheck(IPCPTimer_t *pxTimer)
{
    bool_t xReturn;
    struct timespec n;

    if (!pxTimer->bActive)
        xReturn = false;
    else
    {
        /* The timer might have set the bExpired flag already, if not, check the
         * value of xTimeOut against ulRemainingTime. */
        if (pxTimer->bExpired == false)
        {
            /* FIXME: A system call here is a problem as this function
               is not supposed to fail. */
            if (clock_gettime(CLOCK_REALTIME, &n) < 0)
                LOGE(TAG_IPCPMANAGER, "clock_gettime error");

            if (n.tv_sec == pxTimer->xTimeOut.tv_sec)
                pxTimer->bExpired = n.tv_nsec < pxTimer->xTimeOut.tv_nsec;
            else
                pxTimer->bExpired = n.tv_sec < pxTimer->xTimeOut.tv_sec;
        }

        if (pxTimer->bExpired != false)
        {
            prvIPCPTimerStart(pxTimer, pxTimer->ulReloadTimeUS);
            xReturn = true;
        }
        else
            xReturn = false;
    }

    return xReturn;
}

/**
 * @brief Enable/disable the Flow Allocator timer.
 *
 * @param[in] xExpiredState: pdTRUE - set as expired; pdFALSE - set as non-expired.
 */
void vIpcpSetFATimerExpiredState(bool_t xExpiredState)
{
    xFATimer.bActive = true;

    if (xExpiredState != false)
        xFATimer.bExpired = true;
    else
        xFATimer.bExpired = false;
}

/**
 * @brief Start an IP timer. The IP-task has its own implementation of a timer
 *        called 'IPTimer_t', which is based on the FreeRTOS 'TimeOut_t'.
 *
 * @param[in] pxTimer: Pointer to the timer. When zero, the timer is marked
 *                     as expired.
 * @param[in] xTime: Time to be loaded into the IP timer, in nanoseconds.
 */
static void prvIPCPTimerStart(IPCPTimer_t *pxTimer, useconds_t xTimeUS)
{
    struct timespec n;
    uint64_t nsec;

    if (clock_gettime(CLOCK_REALTIME, &n) < 0)
    {
        /* FIXME: This can fail. */
    }

    nsec = (xTimeUS * 1000) + n.tv_nsec;
    pxTimer->xTimeOut.tv_sec = (time_t)(nsec / 1000000000UL);
    pxTimer->xTimeOut.tv_nsec = (nsec % 1000000000UL);
    pxTimer->ulRemainingTimeUS = xTimeUS;

    if (!xTimeUS)
        pxTimer->bExpired = true;
    else
        pxTimer->bExpired = false;

    pxTimer->bActive = true;
}
/*-----------------------------------------------------------*/

#if 0
/**
 * @brief Sets the reload time of an IP timer and restarts it.
 *
 * @param[in] pxTimer: Pointer to the IP timer.
 * @param[in] xTime: Time to be reloaded into the IP timer.
 */
static void prvIPCPTimerReload(IPCPTimer_t *pxTimer, useconds_t xTimeUS)
{
    pxTimer->ulReloadTimeUS = xTimeUS;
    prvIPCPTimerStart(pxTimer, xTimeUS);
}
#endif

/*-----------------------------------------------------------*/
/**
 * @brief Returns whether the IP task is ready.
 *
 * @return pdTRUE if IP task is ready, else pdFALSE.
 */
bool_t xIPCPIsNetworkTaskReady(void)
{
    return xIPCPTaskInitialised;
}
