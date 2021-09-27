#ifndef IPCP_H_
#define IPCP_H_


#include "configSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ARP826.h"

/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/



typedef struct xQUEUE_FIFO
{
	QueueHandle_t * xQueue;

} rfifo_t;




typedef enum RINA_EVENTS
{
	eNoEvent = -1,
	eNetworkDownEvent,     /* 0: The network interface has been lost and/or needs [re]connecting. */
	eNetworkRxEvent,       /* 1: The network interface has queued a received Ethernet frame. */
	eNetworkTxEvent,       /* 2: Let the Shim-task send a network packet. */
	eARPTimerEvent,        /* 3: The ARP timer expired. */
	eStackTxEvent,         /* 4: The software stack IPCP has queued a packet to transmit. */
	eEFCPTimerEvent,        /* 5: See if any IPCP socket needs attention. */
	eEFCPAcceptEvent,       /* 6: Client API FreeRTOS_accept() waiting for client connections. */
	eShimFlowEvent, 		/* 7: C*/
	eShimEnrollEvent,		/* 8: Enroll shim to DIF */

} eRINAEvent_t;

/**
 * Structure for the information of the commands issued to the RINA task.
 */
typedef struct xRINA_TASK_COMMANDS
{
	eRINAEvent_t eEventType; /**< The event-type enum */
	void * pvData;         /**< The data in the event */
} RINAStackEvent_t;


typedef struct xNETWORK_BUFFER
{
    ListItem_t xBufferListItem;                /**< Used to reference the buffer form the free buffer list or a socket. */
    uint8_t ulGpa;                      		/**< Source or destination Protocol address, depending on usage scenario. */
    uint8_t * pucEthernetBuffer;               /**< Pointer to the start of the Ethernet frame. */
    size_t xDataLength;                        /**< Starts by holding the total Ethernet frame length, then the UDP/TCP payload length. */
    uint16_t usPort;                           /**< Source or destination port, depending on usage scenario. */
    uint16_t usBoundPort;                      /**< The port to which a transmitting socket is bound. */

} NetworkBufferDescriptor_t;

typedef char* string_t;
//typedef char string_t;

typedef uint16_t ipcProcessId_t;
typedef uint16_t ipcpInstanceId_t;

typedef int32_t  portId_t;




typedef struct xName_info
{
	string_t  pcProcessName;  			/*> Process Name*/
	string_t  pcProcessInstance;		/*> Process Instance*/
	string_t  pcEntityName;				/*> Entity Name*/
	string_t  pcEntityInstance;		    /*> Entity Instance*/

}name_t;


typedef enum TYPE_IPCP_INSTANCE
{
	eShimWiFi = 0,
	eNormal

}ipcpInstanceType_t;

/*
 * Contains all the information associated to an instance of a
 * shim Ethernet IPC Process
 */

typedef struct xFLOW_SPECIFICATIONS
{
        /* This structure defines the characteristics of a flow */

        /* Average bandwidth in bytes/s */
        uint32_t 		ulAverageBandwidth;

        /* Average bandwidth in SDUs/s */
        uint32_t 		ulAverageSduBandwidth;

        /*
         * In milliseconds, indicates the maximum delay allowed in this
         * flow. A value of 0 indicates 'do not care'
         */
        uint32_t 		ulDelay;
        /*
         * In milliseconds, indicates the maximum jitter allowed
         * in this flow. A value of 0 indicates 'do not care'
         */
        uint32_t 		ulJitter;

        /*
         * Indicates the maximum packet loss (loss/10000) allowed in this
         * flow. A value of loss >=10000 indicates 'do not care'
         */
        uint16_t 		usLoss;

        /*
         * Indicates the maximum gap allowed among SDUs, a gap of N
         * SDUs is considered the same as all SDUs delivered.
         * A value of -1 indicates 'Any'
         */
        int32_t 		ulMaxAllowableGap;

        /*
         * The maximum SDU size for the flow. May influence the choice
         * of the DIF where the flow will be created.
         */
        uint32_t 		ulMaxSduSize;

        /* Indicates if SDUs have to be delivered in order */
        BaseType_t   	xOrderedDelivery;

        /* Indicates if partial delivery of SDUs is allowed or not */
        BaseType_t   	xPartialDelivery;

        /* In milliseconds */
        uint32_t 		ulPeakBandwidthDuration;

        /* In milliseconds */
        uint32_t 		ulPeakSduBandwidthDuration;

        /* A value of 0 indicates 'do not care' */
        uint32_t 		ulUndetectedBitErrorRate;

        /* Preserve message boundaries */
        BaseType_t 		xMsgBoundaries;
}flowSpec_t;








/**
 * The software timer struct for various IPCP functions
 */
    typedef struct xIPCP_TIMER
    {
        uint32_t
            bActive : 1,            /**< This timer is running and must be processed. */
            bExpired : 1;           /**< Timer has expired and a task must be processed. */
        TimeOut_t xTimeOut;         /**< The timeout value. */
        TickType_t ulRemainingTime; /**< The amount of time remaining. */
        TickType_t ulReloadTime;    /**< The value of reload time. */
    } IPCPTimer_t;


/*
 * Send the event eEvent to the IPCP task event queue, using a block time of
 * zero.  Return pdPASS if the message was sent successfully, otherwise return
 * pdFALSE.
 */
BaseType_t xSendEventToIPCPTask( eRINAEvent_t eEvent );

/* Returns pdTRUE is this function is called from the IPCP-task */
BaseType_t xIsCallingFromIPCPTask( void );

BaseType_t xSendEventStructToIPCPTask( const RINAStackEvent_t * pxEvent,
                                         TickType_t uxTimeout );

eFrameProcessingResult_t eConsiderFrameForProcessing( const uint8_t * const pucEthernetBuffer );

BaseType_t RINA_IPCPInit( void );


#endif
