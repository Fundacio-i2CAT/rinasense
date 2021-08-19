#ifndef IPCP_H
#define IPCP_H


#include "configSensor.h"


/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/

typedef enum FLOW_STATES

{
	eNULL = 0,  // The Port_id cannot be used
	ePENDING,   // The protocol has either initiated the flow allocation or waiting for allocateResponse.
	eALLOCATED  // Flow allocated and the port_id can be used to read/write data from/to.
} ePortidState_t;


typedef enum FRAMES_PROCESSING
{
	eReleaseBuffer = 0,   /* Processing the frame did not find anything to do - just release the buffer. */
	eProcessBuffer,       /* An Ethernet frame has a valid address - continue process its contents. */
	eReturnEthernetFrame, /* The Ethernet frame contains an ARP826 packet that can be returned to its source. */
	eFrameConsumed        /* Processing the Ethernet packet contents resulted in the payload being sent to the stack. */
} eFrameProcessingResult_t;

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

} eRINAEvent_t;

/**
 * Structure for the information of the commands issued to the RINA task.
 */
typedef struct RINA_TASK_COMMANDS
{
	eRINAEvent_t eEventType; /**< The event-type enum */
	void * pvData;         /**< The data in the event */
} RINAStackEvent_t;


typedef struct xNETWORK_BUFFER
{
    ListItem_t xBufferListItem;                /**< Used to reference the buffer form the free buffer list or a socket. */
    uint32_t ulIPCPAddress;                      /**< Source or destination IP address, depending on usage scenario. */
    uint8_t * pucEthernetBuffer;               /**< Pointer to the start of the Ethernet frame. */
    size_t xDataLength;                        /**< Starts by holding the total Ethernet frame length, then the UDP/TCP payload length. */
    uint16_t usPort;                           /**< Source or destination port, depending on usage scenario. */
    uint16_t usBoundPort;                      /**< The port to which a transmitting socket is bound. */

} NetworkBufferDescriptor_t;

typedef char* string_t;
//typedef char string_t;

typedef struct xName_info
{
	string_t  pcProcessName;  			/*> Process Name*/
	string_t  pcProcessInstance;		/*> Process Instance*/
	string_t  pcEntityName;				/*> Entity Name*/
	string_t  pcEntityInstance;		    /*> Entity Instance*/

}APNameInfo_t;

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

#endif
