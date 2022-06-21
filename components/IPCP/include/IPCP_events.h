#ifndef _COMPONENTS_IPCP_EVENTS_H
#define _COMPONENTS_IPCP_EVENTS_H

typedef enum RINA_EVENTS
{
    eNoEvent = -1,
    eNetworkDownEvent,          /* 0: The network interface has been lost and/or needs [re]connecting. */
    eNetworkRxEvent,            /* 1: The network interface has queued a received Ethernet frame. */
    eNetworkTxEvent,            /* 2: Let the Shim-task send a network packet. */
    eShimEnrolledEvent,         /* 3: Shim Enrolled: network Interface Init*/
    eARPTimerEvent,             /* 4: The ARP timer expired. */
    eStackTxEvent,              /* 5: The software stack IPCP has queued a packet to transmit. */
    eEFCPTimerEvent,            /* 6: See if any IPCP socket needs attention. */
    eEFCPAcceptEvent,           /* 7: Client API FreeRTOS_accept() waiting for client connections. */
    eShimFlowAllocatedEvent,    /* 8: A flow has been allocated on the shimWiFi*/
    eStackFlowAllocateEvent,    /*9: The Software stack IPCP has received a Flow allocate request. */
    eStackAppRegistrationEvent, /*10: The Software stack IPCP has received a AppRegistration Event*/
    eFactoryInitEvent,          /*11: The IPCP factories has been initialized. */
    eShimAppRegisteredEvent,    /* 12: The Normal IPCP has been registered into the Shim*/
    eSendMgmtEvent,             /* 13: Send Mgmt PDU */

} eRINAEvent_t;

/**
 * Structure for the information of the commands issued to the RINA task.
 */
typedef struct xRINA_TASK_COMMANDS
{
    eRINAEvent_t eEventType; /**< The event-type enum */
    void *pvData;            /**< The data in the event */
} RINAStackEvent_t;

#endif // _COMPONENTS_IPCP_EVENTS_H
