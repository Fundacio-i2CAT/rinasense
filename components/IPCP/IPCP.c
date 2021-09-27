#include <stdio.h>
#include <string.h>


/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"


#include "IPCP.h"
#include "ARP826.h"
#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "ShimIPCP.h"
#include "configSensor.h"

#include "ESP_log.h"


MACAddress_t xlocalMACAddress = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

/** @brief For convenience, a MAC address of all 0xffs is defined const for quick
 * reference. */
const MACAddress_t xBroadcastMACAddress = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };

/** @brief The queue used to pass events into the IPCP-task for processing. */
QueueHandle_t xNetworkEventQueue = NULL;

/** @brief Set to pdTRUE when the IPCP task is ready to start processing packets. */
static BaseType_t xIPCPTaskInitialised = pdFALSE;

/** @brief Simple set to pdTRUE or pdFALSE depending on whether the network is up or
 * down (connected, not connected) respectively. */
static BaseType_t xNetworkUp = pdFALSE;

/** @brief Used to ensure network down events cannot be missed when they cannot be
 * posted to the network event queue because the network event queue is already
 * full. */
static volatile BaseType_t xNetworkDownEventPending = pdFALSE;

/** @brief Stores the handle of the task that handles the stack.  The handle is used
 * (indirectly) by some utility function to determine if the utility function is
 * being called by a task (in which case it is ok to block) or by the IPCP task
 * itself (in which case it is not ok to block). */
static TaskHandle_t xIPCPTaskHandle = NULL;

/**
 * @brief Utility function to cast pointer of a type to pointer of type NetworkBufferDescriptor_t.
 *
 * @return The casted pointer.
 */
static portINLINE DECL_CAST_PTR_FUNC_FOR_TYPE( NetworkBufferDescriptor_t )
{
    return ( NetworkBufferDescriptor_t * ) pvArgument;
}

/** @brief ARP timer, to check its table entries. */
static IPCPTimer_t xARPTimer;

void RINA_NetworkDown( void );

eFrameProcessingResult_t eConsiderFrameForProcessing( const uint8_t * const pucEthernetBuffer );

BaseType_t xSendEventToIPCPTask( eRINAEvent_t eEvent );

EthernetHeader_t * vCastPointerTo_EthernetPacket_t(const void * pvArgument);

/*----------------------------------------------*/
/*
 * The main IPCP stack processing task.  This task receives commands/events
 * from the network hardware drivers and tasks.  It also
 * maintains a set of protocol timers.
 */
static void prvIPCPTask( void * pvParameters );

static BaseType_t prvIPCPTimerCheck( IPCPTimer_t * pxTimer );


/*
 * Utility functions for the light weight IP timers.
 */
static void prvIPCPTimerStart( IPCPTimer_t * pxTimer,
                             TickType_t xTime );

static BaseType_t prvIPCPTimerCheck( IPCPTimer_t * pxTimer );

static void prvIPCPTimerReload( IPCPTimer_t * pxTimer,
                              TickType_t xTime );

/*
 * Returns pdTRUE if the IP task has been created and is initialised.  Otherwise
 * returns pdFALSE.
 */
BaseType_t xIPCPIsNetworkTaskReady( void );

/*
 * Checks the ARP, timers to see if any periodic or timeout
 * processing is required.
 */
static void prvCheckNetworkTimers( void );

/*
 * Determine how long the IPCP task can sleep for, which depends on when the next
 * periodic or timeout processing must be performed.
 */
static TickType_t prvCalculateSleepTime( void );
/*
* Called to create a network connection when the stack is first started, or
* when the network connection is lost.
*/
static void prvProcessNetworkDownEvent( void );

/*
 * Called when new data is available from the network interface.
 */
static void prvProcessEthernetPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer );


/*
 * The network card driver has received a packet.  In the case that it is part
 * of a linked packet chain, walk through it to handle every message.
 */
static void prvHandleEthernetPacket( NetworkBufferDescriptor_t * pxBuffer );

/*----------------------------------------------------*/

BaseType_t xIPCPTaskReady( void )
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
static void prvIPCPTask( void * pvParameters )
{
    RINAStackEvent_t xReceivedEvent;
    TickType_t xNextIPCPSleep;



    /* Just to prevent compiler warnings about unused parameters. */
    ( void ) pvParameters;


    /* Generate a dummy message to say that the network connection has gone
     *  down.  This will cause this task to initialise the network interface.  After
     *  this it is the responsibility of the network interface hardware driver to
     *  send this message if a previously connected network is disconnected. */
    RINA_NetworkDown();



    /* Initialization is complete and events can now be processed. */
    xIPCPTaskInitialised = pdTRUE;



    /* Loop, processing IP events. */
    for( ; ; )
    {
    	//ipconfigWATCHDOG_TIMER();


    	        /* Check the ARP, DHCP and TCP timers to see if there is any periodic
    	         * or timeout processing to perform. */
    	        prvCheckNetworkTimers();

    	        /* Calculate the acceptable maximum sleep time. */
    	        xNextIPCPSleep = prvCalculateSleepTime();

    	        /* Wait until there is something to do. If the following call exits
    	         * due to a time out rather than a message being received, set a
    	         * 'NoEvent' value. */
    	        if( xQueueReceive( xNetworkEventQueue, ( void * ) &xReceivedEvent, xNextIPCPSleep ) == pdFALSE )
    	        {
    	            xReceivedEvent.eEventType = eNoEvent;
    	        }

    	        switch( xReceivedEvent.eEventType )
    	        {
    	        case eNetworkDownEvent:
    	        	/* Attempt to establish a connection. */
    	        	xNetworkUp = pdFALSE;
    	        	prvProcessNetworkDownEvent();
    	        	break;

    	        case eNetworkRxEvent:

    	        	/* The network hardware driver has received a new packet.  A
    	        	 * pointer to the received buffer is located in the pvData member
    	        	 * of the received event structure. */
    	        	prvHandleEthernetPacket( CAST_PTR_TO_TYPE_PTR( NetworkBufferDescriptor_t, xReceivedEvent.pvData ) );
    	        	break;

    	        case eNetworkTxEvent:

    	        {
    	        	NetworkBufferDescriptor_t * pxDescriptor = CAST_PTR_TO_TYPE_PTR( NetworkBufferDescriptor_t, xReceivedEvent.pvData );

    	        	/* Send a network packet. The ownership will  be transferred to
    	        	 * the driver, which will release it after delivery. */

    	        	( void ) xNetworkInterfaceOutput( pxDescriptor, pdTRUE );
    	        }

    	        break;

    	        case eNoEvent:
    	        	/* xQueueReceive() returned because of a normal time-out. */
    	        	break;

    	        default:
    	        	/* Should not get here. */
    	        	break;
    	        }

    	        if( xNetworkDownEventPending != pdFALSE )
    	        {
    	        	/* A network down event could not be posted to the network event
    	        	 * queue because the queue was full.
    	        	 * As this code runs in the IP-task, it can be done directly by
    	        	 * calling prvProcessNetworkDownEvent(). */
    	        	prvProcessNetworkDownEvent();
    	        }


    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Initialize the RINA network stack and initialize the IPCP-task.
 *
 *
 * @return pdPASS if the task was successfully created and added to a ready
 * list, otherwise an error code defined in the file projdefs.h
 */
BaseType_t RINA_IPCPInit(  )
{
    BaseType_t xReturn = pdFALSE;

    ESP_LOGI(TAG_IPCP, "************* INIT RINA ***********");

    /* This function should only be called once. */
    configASSERT( xIPCPIsNetworkTaskReady() == pdFALSE );
    configASSERT( xNetworkEventQueue == NULL );
    configASSERT( xIPCPTaskHandle == NULL );

    /* ESP32 is 32-bits platform, so this is not executed*/
    if( sizeof( uintptr_t ) == 8 )
    {
        /* This is a 64-bit platform, make sure there is enough space in
         * pucEthernetBuffer to store a pointer. */

        configASSERT( BUFFER_PADDING >= 14 );

        /* But it must have this strange alignment: */
        configASSERT( ( ( ( BUFFER_PADDING ) + 2 ) % 4 ) == 0 );
    }

    /* Check if MTU is big enough. */

    /* Check structure packing is correct. */


    /* Attempt to create the queue used to communicate with the IPCP task. */
    #if ( configSUPPORT_STATIC_ALLOCATION == 1 )
        {

            static StaticQueue_t xNetworkEventStaticQueue;
            static uint8_t ucNetworkEventQueueStorageArea[ EVENT_QUEUE_LENGTH * sizeof( RINAStackEvent_t ) ];
            xNetworkEventQueue = xQueueCreateStatic( EVENT_QUEUE_LENGTH, sizeof( RINAStackEvent_t ), ucNetworkEventQueueStorageArea, &xNetworkEventStaticQueue );
        }
    #else
        {

            xNetworkEventQueue = xQueueCreate( EVENT_QUEUE_LENGTH, sizeof( RINAStackEvent_t ) );
            configASSERT( xNetworkEventQueue != NULL );
        }
    #endif /* SUPPORT_STATIC_ALLOCATION */


    if( xNetworkEventQueue != NULL )
    {

        #if ( configQUEUE_REGISTRY_SIZE > 0 )
            {
                /* A queue registry is normally used to assist a kernel aware
                 * debugger.  If one is in use then it will be helpful for the debugger
                 * to show information about the network event queue. */
                vQueueAddToRegistry( xNetworkEventQueue, "NetEvnt" );
                ESP_LOGI(TAG_IPCP, "Queue added to Registry: %d", configQUEUE_REGISTRY_SIZE);
            }
        #endif /* QUEUE_REGISTRY_SIZE */



        if( xNetworkBuffersInitialise() == pdPASS )
        {

            /* Create the task that processes Ethernet and stack events. */
            #if ( configSUPPORT_STATIC_ALLOCATION == 1 )
                {

                    static StaticTask_t xIPCPTaskBuffer;
                    static StackType_t xIPCPTaskStack[ IPCP_TASK_STACK_SIZE_WORDS ];
                    xIPCPTaskHandle = xTaskCreateStatic( prvIPCPTask,
                                                       "IPCP-Task",
                                                       IPCP_TASK_STACK_SIZE_WORDS,
                                                       NULL,
                                                       IPCP_TASK_PRIORITY,
                                                       xIPCPTaskStack,
                                                       &xIPCPTaskBuffer );



                    if( xIPCPTaskHandle != NULL )
                    {

                        xReturn = pdTRUE;
                    }

                }
            #else /* if ( SUPPORT_STATIC_ALLOCATION == 1 ) */
                {

                    xReturn = xTaskCreate( prvIPCPTask,
                                           "IPCP-task",
                                           IPCP_TASK_STACK_SIZE_WORDS,
                                           NULL,
                                           IPCP_TASK_PRIORITY,
                                           &( xIPCPTaskHandle ) );

                }
            #endif /* SUPPORT_STATIC_ALLOCATION */

        }
        else
        {
            ESP_LOGE(TAG_IPCP, "RINAInit: xNetworkBuffersInitialise() failed\n" ) ;

            /* Clean up. */
            vQueueDelete( xNetworkEventQueue );
            xNetworkEventQueue = NULL;
        }
    }
    else
    {
    	ESP_LOGE(TAG_IPCP,  "RINAInit: Network event queue could not be created\n" );
    }

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
BaseType_t xIsCallingFromIPCPTask( void )
{
    BaseType_t xReturn;

    if( xTaskGetCurrentTaskHandle() == xIPCPTaskHandle )
    {
        xReturn = pdTRUE;
    }
    else
    {
        xReturn = pdFALSE;
    }

    return xReturn;
}


/**
 * @brief Send an event to the IPCP task. It calls 'xSendEventStructToIPCPTask' internally.
 *
 * @param[in] eEvent: The event to be sent.
 *
 * @return pdPASS if the event was sent (or the desired effect was achieved). Else, pdFAIL.
 */
BaseType_t xSendEventToIPCPTask( eRINAEvent_t eEvent )
{
    RINAStackEvent_t xEventMessage;
    const TickType_t xDontBlock = ( TickType_t ) 0;

    xEventMessage.eEventType = eEvent;
    xEventMessage.pvData = ( void * ) NULL;

    return xSendEventStructToIPCPTask( &xEventMessage, xDontBlock );
}

/**
 * @brief Send an event (in form of struct) to the IP task to be processed.
 *
 * @param[in] pxEvent: The event to be sent.
 * @param[in] uxTimeout: Timeout for waiting in case the queue is full. 0 for non-blocking calls.
 *
 * @return pdPASS if the event was sent (or the desired effect was achieved). Else, pdFAIL.
 */
BaseType_t xSendEventStructToIPCPTask( const RINAStackEvent_t * pxEvent,
                                     TickType_t uxTimeout )
{
    BaseType_t xReturn, xSendMessage;
    TickType_t uxUseTimeout = uxTimeout;

    if( ( xIPCPTaskReady() == pdFALSE ) && ( pxEvent->eEventType != eNetworkDownEvent ) )
    {
        /* Only allow eNetworkDownEvent events if the IP task is not ready
         * yet.  Not going to attempt to send the message so the send failed. */
        xReturn = pdFAIL;
    }
    else
    {
        xSendMessage = pdTRUE;

        if( xSendMessage != pdFALSE )
        {
            /* The IP task cannot block itself while waiting for itself to
             * respond. */
            if( ( xIsCallingFromIPCPTask() == pdTRUE ) && ( uxUseTimeout > ( TickType_t ) 0U ) )
            {
                uxUseTimeout = ( TickType_t ) 0;
            }

            xReturn = xQueueSendToBack( xNetworkEventQueue, pxEvent, uxUseTimeout );

            if( xReturn == pdFAIL )
            {
                /* A message should have been sent to the IP task, but wasn't. */
                //FreeRTOS_debug_printf( ( "xSendEventStructToIPTask: CAN NOT ADD %d\n", pxEvent->eEventType ) );

            }
        }
        else
        {
            /* It was not necessary to send the message to process the event so
             * even though the message was not sent the call was successful. */
            xReturn = pdPASS;
        }
    }

    return xReturn;
}




void prvHandleEthernetPacket( NetworkBufferDescriptor_t * pxBuffer )
{
    #if ( USE_LINKED_RX_MESSAGES == 0 )
        {
            /* When ipconfigUSE_LINKED_RX_MESSAGES is not set to 0 then only one
             * buffer will be sent at a time.  This is the default way for +TCP to pass
             * messages from the MAC to the TCP/IP stack. */
        	ESP_LOGI( TAG_IPCP, "Packet to network stack %p, len %d", pxBuffer, pxBuffer->xDataLength );
            prvProcessEthernetPacket( pxBuffer );
        }
    #else /* configUSE_LINKED_RX_MESSAGES */
        {
            NetworkBufferDescriptor_t * pxNextBuffer;

            /* An optimisation that is useful when there is high network traffic.
             * Instead of passing received packets into the IP task one at a time the
             * network interface can chain received packets together and pass them into
             * the IP task in one go.  The packets are chained using the pxNextBuffer
             * member.  The loop below walks through the chain processing each packet
             * in the chain in turn. */
            do
            {
                /* Store a pointer to the buffer after pxBuffer for use later on. */
                pxNextBuffer = pxBuffer->pxNextBuffer;

                /* Make it NULL to avoid using it later on. */
                pxBuffer->pxNextBuffer = NULL;

                prvProcessEthernetPacket( pxBuffer );
                pxBuffer = pxNextBuffer;

                /* While there is another packet in the chain. */
            } while( pxBuffer != NULL );
        }
    #endif /* USE_LINKED_RX_MESSAGES */
}


/*-----------------------------------------------------------*/


void prvProcessEthernetPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer )
{
    const EthernetHeader_t * pxEthernetHeader;
    eFrameProcessingResult_t eReturned = eReleaseBuffer;

    configASSERT( pxNetworkBuffer != NULL );

    ESP_LOGI(TAG_IPCP, "Processing Ethernet Packet");

    /* Interpret the Ethernet frame. */
    if( pxNetworkBuffer->xDataLength >= sizeof( EthernetHeader_t ) )
    {
    	ESP_LOGI(TAG_IPCP, "Interprete Packet");

    	//eReturned = eConsiderFrameForProcessing( pxNetworkBuffer->pucEthernetBuffer );

    	//ESP_LOGI(TAG_IPCP, "eProccessBuffer: %i", eReturned);

        /* Map the buffer onto the Ethernet Header struct for easy access to the fields. */
    	pxEthernetHeader = vCastPointerTo_EthernetPacket_t(pxNetworkBuffer->pucEthernetBuffer);
        //pxEthernetHeader = CAST_CONST_PTR_TO_CONST_TYPE_PTR( EthernetHeader_t, pxNetworkBuffer->pucEthernetBuffer );

        /* The condition "eReturned == eProcessBuffer" must be true. */
       /*#if ( ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0 )
        	if( eReturned == eProcessBuffer )
        #endif*/
       //{
        	ESP_LOGI(TAG_IPCP, "eProccessBuffer");
            ESP_LOGI(TAG_ARP,"MAC Ethernet Dest: %02x:%02x:%02x:%02x:%02x:%02x", pxEthernetHeader->xDestinationAddress.ucBytes[0],
            		pxEthernetHeader->xDestinationAddress.ucBytes[1],
        			pxEthernetHeader->xDestinationAddress.ucBytes[2],
        			pxEthernetHeader->xDestinationAddress.ucBytes[3],
        			pxEthernetHeader->xDestinationAddress.ucBytes[4],
        			pxEthernetHeader->xDestinationAddress.ucBytes[5]);

            ESP_LOGI(TAG_ARP,"MAC Ethernet Source: %02x:%02x:%02x:%02x:%02x:%02x", pxEthernetHeader->xSourceAddress.ucBytes[0],
            		pxEthernetHeader->xSourceAddress.ucBytes[1],
        			pxEthernetHeader->xSourceAddress.ucBytes[2],
        			pxEthernetHeader->xSourceAddress.ucBytes[3],
        			pxEthernetHeader->xSourceAddress.ucBytes[4],
        			pxEthernetHeader->xSourceAddress.ucBytes[5]);
            ESP_LOGI(TAG_ARP,"Ethernet Type: %04x", pxEthernetHeader->usFrameType);
            /* Interpret the received Ethernet packet. */
            switch( pxEthernetHeader->usFrameType )
            {
                case ETH_P_BATMAN:

                    /* The Ethernet frame contains an ARP packet. */
                	ESP_LOGI(TAG_IPCP, "Case BATMAN");
                    if( pxNetworkBuffer->xDataLength >= sizeof( ARPPacket_t ) )
                    {
                        eReturned = eARPProcessPacket( CAST_PTR_TO_TYPE_PTR( ARPPacket_t, pxNetworkBuffer->pucEthernetBuffer ) );
                    }
                    else
                    {
                        eReturned = eReleaseBuffer;
                    }

                    break;
#if 0
                case ipIPv4_FRAME_TYPE:

                    /* The Ethernet frame contains an IP packet. */
                    if( pxNetworkBuffer->xDataLength >= sizeof( IPPacket_t ) )


                    {
                        eReturned = prvProcessIPPacket( CAST_PTR_TO_TYPE_PTR( IPPacket_t, pxNetworkBuffer->pucEthernetBuffer ), pxNetworkBuffer );
                    }
                    else
                    {
                        eReturned = eReleaseBuffer;
                    }

                    break;

#endif
                default:

                    break;
            }
        }
    //}

    /* Perform any actions that resulted from processing the Ethernet frame. */
    switch( eReturned )
    {
        case eReturnEthernetFrame:

            /* The Ethernet frame will have been updated (maybe it was
             * an ARP request or a PING request?) and should be sent back to
             * its source. */
            //vReturnEthernetFrame( pxNetworkBuffer, pdTRUE );

            /* parameter pdTRUE: the buffer must be released once
             * the frame has been transmitted */
            break;

        case eFrameConsumed:

            /* The frame is in use somewhere, don't release the buffer
             * yet. */
            break;

        case eReleaseBuffer:
        case eProcessBuffer:
        default:

            /* The frame is not being used anywhere, and the
             * NetworkBufferDescriptor_t structure containing the frame should
             * just be released back to the list of free buffers. */
            vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
            break;
    }
}


eFrameProcessingResult_t eConsiderFrameForProcessing( const uint8_t * const pucEthernetBuffer )
{
    eFrameProcessingResult_t eReturn;
    const EthernetHeader_t * pxEthernetHeader;
    uint16_t usFrameType;



    /* Map the buffer onto Ethernet Header struct for easy access to fields. */
    pxEthernetHeader = vCastPointerTo_EthernetPacket_t( pucEthernetBuffer );

    usFrameType = pxEthernetHeader->usFrameType;
    usFrameType = FreeRTOS_ntohs( usFrameType );

    ESP_LOGI(TAG_ARP,"MAC Ethernet Dest: %02x:%02x:%02x:%02x:%02x:%02x", pxEthernetHeader->xDestinationAddress.ucBytes[0],
    		pxEthernetHeader->xDestinationAddress.ucBytes[1],
			pxEthernetHeader->xDestinationAddress.ucBytes[2],
			pxEthernetHeader->xDestinationAddress.ucBytes[3],
			pxEthernetHeader->xDestinationAddress.ucBytes[4],
			pxEthernetHeader->xDestinationAddress.ucBytes[5]);

    ESP_LOGI(TAG_ARP,"MAC Ethernet Source: %02x:%02x:%02x:%02x:%02x:%02x", pxEthernetHeader->xSourceAddress.ucBytes[0],
    		pxEthernetHeader->xSourceAddress.ucBytes[1],
			pxEthernetHeader->xSourceAddress.ucBytes[2],
			pxEthernetHeader->xSourceAddress.ucBytes[3],
			pxEthernetHeader->xSourceAddress.ucBytes[4],
			pxEthernetHeader->xSourceAddress.ucBytes[5]);
    /*
    ESP_LOGI(TAG_ARP,"MAC Local: %02x:%02x:%02x:%02x:%02x:%02x", xlocalMAC->ucBytes[0],
      		xlocalMAC->ucBytes[1],
  			xlocalMAC->ucBytes[2],
  			xlocalMAC->ucBytes[3],
  			xlocalMAC->ucBytes[4],
  			xlocalMAC->ucBytes[5]);*/
    ESP_LOGI(TAG_ARP,"Ethernet Type: %04x", usFrameType);

    if( memcmp( xlocalMACAddress.ucBytes, pxEthernetHeader->xDestinationAddress.ucBytes, sizeof( MACAddress_t ) ) == 0 )
    {
        /* The packet was directed to this node - process it. */
    	ESP_LOGI(TAG_IPCP,"Process!"); //test

        eReturn = eProcessBuffer;
    }
    else if( memcmp( xBroadcastMACAddress.ucBytes, pxEthernetHeader->xDestinationAddress.ucBytes, sizeof( MACAddress_t ) ) == 0
    		&& usFrameType == ETH_P_BATMAN)
    {
        /* The packet was a broadcast - process it. */
    	ESP_LOGI(TAG_IPCP,"Process!"); //test

        eReturn = eProcessBuffer;
    }
    else
    {
        /* The packet was not a broadcast, or for this node, just release
         * the buffer without taking any other action. */
    	ESP_LOGI(TAG_IPCP,"No Process!");
        eReturn = eReleaseBuffer;
    }



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
static TickType_t prvCalculateSleepTime( void )
{
    TickType_t xMaximumSleepTime;

    /* Start with the maximum sleep time, then check this against the remaining
     * time in any other timers that are active. */
    xMaximumSleepTime = MAX_IPCP_TASK_SLEEP_TIME;

    if( xARPTimer.bActive != pdFALSE_UNSIGNED )
    {
        if( xARPTimer.ulRemainingTime < xMaximumSleepTime )
        {
            xMaximumSleepTime = xARPTimer.ulRemainingTime;
        }
    }



    return xMaximumSleepTime;
}


/**
 * @brief Check the network timers (ARP/DTP) and if they are
 *        expired, send an event to the IPCP-Task.
 */
static void prvCheckNetworkTimers( void )
{
    /* Is it time for ARP processing? */
    if( prvIPCPTimerCheck( &xARPTimer ) != pdFALSE )
    {
    	ESP_LOGI(TAG_SHIM,"TEST");
        ( void ) xSendEventToIPCPTask( eARPTimerEvent );
    }

}

/*-----------------------------------------------------------*/

/**
 * @brief Process a 'Network down' event and complete required processing.
 */
static void prvProcessNetworkDownEvent( void )
{
    /* Stop the ARP timer while there is no network. */
    xARPTimer.bActive = pdFALSE_UNSIGNED;


    /* Per the ARP Cache Validation section of https://tools.ietf.org/html/rfc1122,
     * treat network down as a "delivery problem" and flush the ARP cache for this
     * interface. */
   // RINA_vARPRemove(  );

    /* The network has been disconnected (or is being initialised for the first
     * time).  Perform whatever hardware processing is necessary to bring it up
     * again, or wait for it to be available again.  This is hardware dependent. */

    if( xShimWiFiInit(  ) != pdPASS )
    {
        /* Ideally the network interface initialisation function will only
         * return when the network is available.  In case this is not the case,
         * wait a while before retrying the initialisation. */
        vTaskDelay( INITIALISATION_RETRY_DELAY );
        //FreeRTOS_NetworkDown();
    }

}
/*-----------------------------------------------------------*/
/**
 * @brief Check the IP timer to see whether an IP event should be processed or not.
 *
 * @param[in] pxTimer: Pointer to the IP timer.
 *
 * @return If the timer is expired then pdTRUE is returned. Else pdFALSE.
 */
static BaseType_t prvIPCPTimerCheck( IPCPTimer_t * pxTimer )
{
    BaseType_t xReturn;

    if( pxTimer->bActive == pdFALSE_UNSIGNED )
    {
        /* The timer is not enabled. */
        xReturn = pdFALSE;
    }
    else
    {
        /* The timer might have set the bExpired flag already, if not, check the
         * value of xTimeOut against ulRemainingTime. */
        if( pxTimer->bExpired == pdFALSE_UNSIGNED )
        {
            if( xTaskCheckForTimeOut( &( pxTimer->xTimeOut ), &( pxTimer->ulRemainingTime ) ) != pdFALSE )
            {
                pxTimer->bExpired = pdTRUE_UNSIGNED;
            }
        }

        if( pxTimer->bExpired != pdFALSE_UNSIGNED )
        {
            prvIPCPTimerStart( pxTimer, pxTimer->ulReloadTime );
            xReturn = pdTRUE;
        }
        else
        {
            xReturn = pdFALSE;
        }
    }

    return xReturn;
}

/**
 * @brief Start an IP timer. The IP-task has its own implementation of a timer
 *        called 'IPTimer_t', which is based on the FreeRTOS 'TimeOut_t'.
 *
 * @param[in] pxTimer: Pointer to the IP timer. When zero, the timer is marked
 *                     as expired.
 * @param[in] xTime: Time to be loaded into the IP timer.
 */
static void prvIPCPTimerStart( IPCPTimer_t * pxTimer,
                             TickType_t xTime )
{
    vTaskSetTimeOutState( &pxTimer->xTimeOut );
    pxTimer->ulRemainingTime = xTime;

    if( xTime == ( TickType_t ) 0 )
    {
        pxTimer->bExpired = pdTRUE_UNSIGNED;
    }
    else
    {
        pxTimer->bExpired = pdFALSE_UNSIGNED;
    }

    pxTimer->bActive = pdTRUE_UNSIGNED;
}
/*-----------------------------------------------------------*/

/**
 * @brief Sets the reload time of an IP timer and restarts it.
 *
 * @param[in] pxTimer: Pointer to the IP timer.
 * @param[in] xTime: Time to be reloaded into the IP timer.
 */
static void prvIPCPTimerReload( IPCPTimer_t * pxTimer,
                              TickType_t xTime )
{
    pxTimer->ulReloadTime = xTime;
    prvIPCPTimerStart( pxTimer, xTime );
}
/*-----------------------------------------------------------*/
/**
 * @brief Returns whether the IP task is ready.
 *
 * @return pdTRUE if IP task is ready, else pdFALSE.
 */
BaseType_t xIPCPIsNetworkTaskReady( void )
{
    return xIPCPTaskInitialised;
}

void RINA_NetworkDown( void )
{
    static const RINAStackEvent_t xNetworkDownEvent = { eNetworkDownEvent, NULL };
    const TickType_t xDontBlock = ( TickType_t ) 0;

    /* Simply send the network task the appropriate event. */
    if( xSendEventStructToIPCPTask( &xNetworkDownEvent, xDontBlock ) != pdPASS )
    {
        /* Could not send the message, so it is still pending. */
        xNetworkDownEventPending = pdTRUE;
    }
    else
    {
        /* Message was sent so it is not pending. */
        xNetworkDownEventPending = pdFALSE;
    }



}


/*-----------------------------------------------------------*/
EthernetHeader_t * vCastPointerTo_EthernetPacket_t(const void * pvArgument)
{
	return (const void *) (pvArgument);
}
