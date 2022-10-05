#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#endif

#include "common/rina_ids.h"
#include "ARP826_defs.h"
#include "configSensor.h"
#include "configRINA.h"
#include "portability/port.h"
#include "IPCP.h"
#include "IPCP_api.h"
#include "IPCP_events.h"
#include "ARP826.h"
#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "RINA_API_flows.h"
#include "ShimIPCP.h"
#include "IPCP_normal_defs.h"
#include "IPCP_normal_api.h"
#include "IpcManager.h"
#include "Enrollment.h"
#include "Enrollment_api.h"
#include "FlowAllocator.h"
#include "rina_buffers.h"
#include "rina_common_port.h"
#include "RINA_API.h"

/** @brief The queue used to pass events into the IPCP-task for processing. */
RsQueue_t *xNetworkEventQueue = NULL;

/** @brief Set to pdTRUE when the IPCP task is ready to start processing packets. */
static bool_t xIPCPTaskInitialised = false;

/** @brief Simple set to pdTRUE or pdFALSE depending on whether the network is up or
 * down (connected, not connected) respectively. */
static bool_t xNetworkUp = false;

static portId_t xN1PortId = PORT_ID_WRONG;

static portId_t xAppPortId = PORT_ID_WRONG;

/** @brief Used to ensure network down events cannot be missed when they cannot be
 * posted to the network event queue because the network event queue is already
 * full. */
static volatile bool_t xNetworkDownEventPending = false;

/** @brief Stores the handle of the task that handles the stack.  The handle is used
 * (indirectly) by some utility function to determine if the utility function is
 * being called by a task (in which case it is ok to block) or by the IPCP task
 * itself (in which case it is not ok to block). */
static pthread_t xIPCPThread;

#ifdef ESP_PLATFORM
/* This saves the FreeRTOS current task handle. */
static TaskHandle_t xIPCPTaskHandle;
#endif

ARP_t xARP;

/* List of Factories */
// static factories_t *pxFactories;
ipcManager_t *pxIpcManager;

/*********************************************************/

struct ipcpInstanceData_t *pxIpcpData;

/* RIBD module */

/* Enrollment Task */

/* Flow Allocator */

/**************************************/
struct ipcpInstance_t *pxShimInstance;

/** @brief ARP timer, to check its table entries. */
static IPCPTimer_t xARPTimer;

static IPCPTimer_t xFATimer;

void RINA_NetworkDown(void);

bool_t vIpcpInit(void);

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer);

bool_t xSendEventToIPCPTask(eRINAEvent_t eEvent);

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

static void prvIPCPTimerReload(IPCPTimer_t *pxTimer, useconds_t xTimeUS);

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
    RINAStackEvent_t xReceivedEvent;
    struct timespec xNextIPCPSleep;
    flowAllocateHandle_t *pxFlowAllocateHandle;
    useconds_t xSleepTimeUS;

    /* Just to prevent compiler warnings about unused parameters. */
    (void)pvParameters;

    if (!vIpcpInit())
    {
        LOGE(TAG_IPCPMANAGER, "IPC Managed Thread initialization failed");
        return NULL;
    }

#ifdef ESP_PLATFORM
    /* FIXME: Protect with mutex. */
    xIPCPTaskHandle = xTaskGetCurrentTaskHandle();
#endif

    pxIpcManager = pvRsMemAlloc(sizeof(*pxIpcManager));

    if (!xIpcManagerInit(pxIpcManager))
        LOGE(TAG_IPCPMANAGER, "Error to initializing IPC Manager");

    /* Initialization is complete and events can now be processed. */
    xIPCPTaskInitialised = true;

    /* Generate a dummy message to say that the network connection has gone
     *  down.  This will cause this task to initialise the network interface.  After
     *  this it is the responsibility of the network interface hardware driver to
     *  send this message if a previously connected network is disconnected. */

    // RINA_NetworkDown();

    /* Create Shim */
    pxShimInstance = pxIpcManagerCreateShim(pxIpcManager); // list of Instances, shimWifi Should request a xIpcpId. Use API?
    if (!pxShimInstance)
        LOGE(TAG_IPCPNORMAL, "It was not possible to create Shim ");

    // Init shim use API?
    if (!xShimWiFiInit(pxShimInstance)) {
        LOGE(TAG_IPCPMANAGER, "Failed to initialize WiFi shim");
        return NULL;
    }

    LOGI(TAG_IPCPMANAGER, "ENTER: IPC Manager Thread");

    /* Loop, processing IP events. */
    for (;;)
    {
        // ipconfigWATCHDOG_TIMER();

        /* Check the ARP, DHCP and TCP timers to see if there is any periodic
         * or timeout processing to perform. */
        prvCheckNetworkTimers();

        /* Calculate the acceptable maximum sleep time. */
        xSleepTimeUS = prvCalculateSleepTimeUS();

        /* Wait until there is something to do. If the following call exits
         * due to a time out rather than a message being received, set a
         * 'NoEvent' value. */
        if (xRsQueueReceive(xNetworkEventQueue, (void *)&xReceivedEvent, sizeof(RINAStackEvent_t), xSleepTimeUS) == false)
            xReceivedEvent.eEventType = eNoEvent;

        switch (xReceivedEvent.eEventType)
        {
        case eNetworkDownEvent:
        {
            struct timespec ts = {INITIALISATION_RETRY_DELAY_SEC, 0};

            LOGD(TAG_IPCPMANAGER, "Event: eNetworkDownEvent");

            /* Attempt to establish a connection. */
            nanosleep(&ts, NULL);
            LOGI(TAG_IPCPMANAGER, "eNetworkDownEvent");
            xNetworkUp = false;
            prvProcessNetworkDownEvent();
            break;
        }

        case eNetworkTxEvent:
        {
            NetworkBufferDescriptor_t *pxNetworkBuffer;

            LOGD(TAG_IPCPMANAGER, "Event: eNetworkTxEvent");

            pxNetworkBuffer = (NetworkBufferDescriptor_t *)xReceivedEvent.xData.PV;

            /* Send a network packet. The ownership will  be transferred to
             * the driver, which will release it after delivery. */
            xNetworkInterfaceOutput(pxNetworkBuffer, true);
        }
        break;

        case eShimEnrolledEvent:
            LOGD(TAG_IPCPMANAGER, "Event: eShimEnrolledEvent");

            /* Registering into the shim */
            if (!xNormalRegistering(pxShimInstance, pxIpcpData->pxDifName, pxIpcpData->pxName))
                LOGE(TAG_IPCPMANAGER, "IPCP not registered into the shim");

            xN1PortId = xIPCPAllocatePortId(); // check this

            (void)vIcpManagerEnrollmentFlowRequest(pxShimInstance, xN1PortId, pxIpcpData->pxName);

            break;

        case eShimFlowAllocatedEvent:
            LOGD(TAG_IPCPMANAGER, "Event: eShimFlowAllocatedEvent");

            RsAssert(xN1PortId != PORT_ID_WRONG);

            /*Call to the method to init the enrollment*/

            (void)xNormalFlowBinding(pxIpcpData, xN1PortId, pxShimInstance);
            (void)xEnrollmentInit(pxIpcpData, xN1PortId);

            break;

        case eFATimerEvent:
            LOGD(TAG_IPCPMANAGER, "Event: eFATimerEvent");
            vIpcpSetFATimerExpiredState(true);
            break;

        case eFlowDeallocateEvent:
            LOGD(TAG_IPCPMANAGER, "Event: eFlowDeallocateEvent");
            //(void)vFlowAllocatorDeallocate((portId_t *)xReceivedEvent.pvData);
            break;

        case eFlowBindEvent:
            LOGD(TAG_IPCPMANAGER, "Event: eFlowBindEvent");

            pxFlowAllocateHandle = (flowAllocateHandle_t *)xReceivedEvent.xData.PV;

            (void)xNormalFlowPrebind(pxIpcpData, pxFlowAllocateHandle);

#if 0
            pxFlowAllocateRequest->xEventBits |= (EventBits_t)eFLOW_BOUND;
            vRINA_WeakUpUser(pxFlowAllocateRequest);
#endif

            vRINA_WakeUpFlowRequest(pxFlowAllocateHandle, eFLOW_BOUND);

            break;

        case eSendMgmtEvent:
            LOGD(TAG_IPCPMANAGER, "Event: eSendMgmtEvent");
            break;

        case eStackTxEvent:
        {
            LOGD(TAG_IPCPMANAGER, "Event: eStackTxEvent");

            // call Efcp to write SDU.
            NetworkBufferDescriptor_t *pxNetBuffer = (NetworkBufferDescriptor_t *)xReceivedEvent.xData.PV;

            (void)xNormalDuWrite(pxIpcpData, pxNetBuffer->ulBoundPort, pxNetBuffer);
        }
        break;

        case eNoEvent:
            /* xQueueReceive() returned because of a normal time-out. */
            break;

        default:
            /* Should not get here. */
            break;
        }

        if (xNetworkDownEventPending != false)
        {
            /* A network down event could not be posted to the network event
             * queue because the queue was full.
             * As this code runs in the IP-task, it can be done directly by
             * calling prvProcessNetworkDownEvent(). */
            prvProcessNetworkDownEvent();
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
bool_t RINA_IPCPInit()
{
    bool_t xReturn = false;

    LOGI(TAG_IPCPMANAGER, "************* INIT RINA ***********");

    /* This function should only be called once. */
    RsAssert(xIPCPIsNetworkTaskReady() == false);
    RsAssert(xNetworkEventQueue == NULL);

    // Temporary
    if (!xARPInit(&xARP)) {
        LOGE(TAG_IPCPMANAGER, "Failed to initialize ARP component");
        return false;
    }

#if 0

    /* ESP32 is 32-bits platform, so this is not executed*/
    if (sizeof(uintptr_t) == 8)
    {
        /* This is a 64-bit platform, make sure there is enough space in
         * pucEthernetBuffer to store a pointer. */

        RsAssert(BUFFER_PADDING >= 14);

        /* But it must have this strange alignment: */
        RsAssert((((BUFFER_PADDING) + 2) % 4) == 0);
    }

#endif

    /* Check if MTU is big enough. */

    /* Check structure packing is correct. */

    /* Attempt to create the queue used to communicate with the IPCP task. */
    xNetworkEventQueue = pxRsQueueCreate("IPCPQueue",
                                         EVENT_QUEUE_LENGTH,
                                         sizeof(RINAStackEvent_t));
    RsAssert(xNetworkEventQueue != NULL);

    if (xNetworkEventQueue != NULL)
    {

        if (xNetworkBuffersInitialise())
        {
            pthread_attr_t attr;

            if (pthread_attr_init(&attr) != 0)
                return false;

            pthread_attr_setstacksize(&attr, IPCP_TASK_STACK_SIZE);

            if (pthread_create(&xIPCPThread, &attr, prvIPCPTask, NULL) != 0)
            {
                LOGE(TAG_IPCPMANAGER, "RINAInit: failed to start IPC process");
                xReturn = false;
            }
            else
                xReturn = true;

            pthread_attr_destroy(&attr);
        }
        else
        {
            LOGE(TAG_IPCPMANAGER, "RINAInit: xNetworkBuffersInitialise() failed\n");

            /* Clean up. */
            vRsQueueDelete(xNetworkEventQueue);
            xNetworkEventQueue = NULL;
        }
    }
    else
        LOGE(TAG_IPCPMANAGER, "RINAInit: Network event queue could not be created\n");

    return xReturn;
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
bool_t xSendEventToIPCPTask(eRINAEvent_t eEvent)
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
bool_t xSendEventStructToIPCPTask(const RINAStackEvent_t *pxEvent, useconds_t xTimeOutUS)
{
    bool_t xReturn, xSendMessage;
    useconds_t xCalculatedTimeOutUS;

    if (!xIPCPTaskReady() && (pxEvent->eEventType != eNetworkDownEvent))
    {
        LOGE(TAG_IPCPMANAGER, "IPCP Task Not Ready, cannot send event");

        /* Only allow eNetworkDownEvent events if the IP task is not ready
         * yet.  Not going to attempt to send the message so the send failed. */
        xReturn = false;
    }
    else
    {
        xSendMessage = true;

        if (xSendMessage)
        {
            /* The IP task cannot block itself while waiting for
             * itself to respond. */
            if (xIsCallingFromIPCPTask())
                xCalculatedTimeOutUS = 0;
            else
                xCalculatedTimeOutUS = xTimeOutUS;

            xReturn = xRsQueueSendToBack(xNetworkEventQueue, pxEvent, sizeof(RINAStackEvent_t), xCalculatedTimeOutUS);

            if (!xReturn)
                LOGE(TAG_IPCPMANAGER, "Failed to add message to IPCP queue");
        }
        else
        {
            /* It was not necessary to send the message to process the event so
             * even though the message was not sent the call was successful. */
            xReturn = true;
        }
    }

    return xReturn;
}

/*-----------------------------------------------------------*/


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
        LOGD(TAG_IPCPMANAGER, "Ethernet packet of type %xu: ACCEPTED", usFrameType);
    }
    else
        LOGD(TAG_IPCPMANAGER, "Ethernet packet of type %xu: REJECTED", usFrameType);

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
    xMaximumSleepTimeUS = MAX_IPCP_TASK_SLEEP_TIME_US;

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
        vTaskDelay(INITIALISATION_RETRY_DELAY);
        RINA_NetworkDown();
    }
    vTaskDelay(INITIALISATION_RETRY_DELAY);
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

void RINA_NetworkDown(void)
{
    static const RINAStackEvent_t xNetworkDownEvent = {
        .eEventType = eNetworkDownEvent,
        .xData = { .PV = NULL }
    };

    LOGI(TAG_IPCPMANAGER, "RINA_NetworkDown");

    /* Simply send the network task the appropriate event. */
    if (xSendEventStructToIPCPTask(&xNetworkDownEvent, 0) != true)
        /* Could not send the message, so it is still pending. */
        xNetworkDownEventPending = true;
    else
        /* Message was sent so it is not pending. */
        xNetworkDownEventPending = false;
}

bool_t vIpcpInit(void)
{
    name_t *pxDifName, *pxIPCPName;

    /*** IPCP Modules ***/
    /* RMT module */
    struct rmt_t *pxRmt;

    /* EFCP Container */
    struct efcpContainer_t *pxEfcpContainer;

    /*Initialize IPC Manager*/

    pxIpcpData = pvRsMemAlloc(sizeof(*pxIpcpData));
    pxIPCPName = pvRsMemAlloc(sizeof(*pxIPCPName));
    pxDifName = pvRsMemAlloc(sizeof(*pxDifName));

    if (pxIpcpData == NULL || pxIPCPName == NULL || pxDifName == NULL)
        goto fail;

    pxIPCPName->pcEntityInstance = NORMAL_ENTITY_INSTANCE;
    pxIPCPName->pcEntityName = NORMAL_ENTITY_NAME;
    pxIPCPName->pcProcessInstance = NORMAL_PROCESS_INSTANCE;
    pxIPCPName->pcProcessName = NORMAL_PROCESS_NAME;

    pxDifName->pcProcessName = NORMAL_DIF_NAME;
    pxDifName->pcProcessInstance = "";
    pxDifName->pcEntityInstance = "";
    pxDifName->pcEntityName = "";

    /* Create EFPC Container */
    pxEfcpContainer = pxEfcpContainerCreate();
    if (!pxEfcpContainer)
    {
        LOGE(TAG_IPCPMANAGER, "Failed creation of EFCP container");
        goto fail;
    }

    /* Create RMT*/
    pxRmt = pxRmtCreate(pxEfcpContainer);
    if (!pxRmt)
    {
        LOGE(TAG_IPCPMANAGER, "Failed creation of RMT instance");
        goto fail;
    }

    pxEfcpContainer->pxRmt = pxRmt;

    pxIpcpData->pxDifName = pxDifName;
    pxIpcpData->pxName = pxIPCPName;
    pxIpcpData->pxEfcpc = pxEfcpContainer;
    pxIpcpData->pxRmt = pxRmt;
    pxIpcpData->xAddress = LOCAL_ADDRESS;

    // pxIpcpData->pxFa = pxFlowAllocatorInit();

    // pxIpcpData->pxIpcManager = pxIpcManager;
    /*Initialialise flows list*/
    vRsListInit(&(pxIpcpData->xFlowsList));

    return true;

fail:
    /* FIXME: is this cleanup necessary since failing here this means
     * nothing will work. */

    if (pxIpcpData != NULL)
        vRsMemFree(pxIpcpData);
    if (pxIPCPName != NULL)
        vRsMemFree(pxIPCPName);
    if (pxDifName != NULL)
        vRsMemFree(pxDifName);

    return false;
}

struct rmt_t *pxIPCPGetRmt(void);
struct rmt_t *pxIPCPGetRmt(void)
{
    return pxIpcpData->pxRmt;
}

struct efcpContainer_t *pxIPCPGetEfcpc(void)
{
    return pxIpcpData->pxEfcpc;
}

struct ipcpInstanceData_t *pxIpcpGetData(void)
{
    return pxIpcpData;
}

portId_t xIPCPAllocatePortId(void)
{
    return ulNumMgrAllocate(pxIpcManager->pxPidm);
}
