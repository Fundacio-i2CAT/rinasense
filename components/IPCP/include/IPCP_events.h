#ifndef _COMPONENTS_IPCP_EVENTS_H
#define _COMPONENTS_IPCP_EVENTS_H

#include <stdint.h>

#include "du.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum RINA_EVENTS
{
    eNoEvent = -1,

    /* The network interface has been lost and/or needs [re]connecting. */
    eNetworkDownEvent,

    /* Shim Enrolled: network Interface Init*/
    eShimEnrolledEvent,

    /* The ARP timer expired. */
    eARPTimerEvent,

    /* Request to transmit a RINA DU down the stack
     * Args:
     * 1: Port ID
     * 2: DU
     */
    eRinaTxEvent,

    /* Request to transmit a RINA DU up the stack
     * Args:
     * 1: Port ID
     * 2: DU
     */
    eRinaRxEvent,

    eFATimerEvent,              /* 6: See if any IPCP socket needs attention. */
    eFlowBindEvent,             /* 7: Client API request to bind a flow. */
    eFlowDeallocateEvent,       /* 8: A flow must be deallocated */
    eStackRxEvent,              /* 9: The stack IPCP has queued a packet to received */
    eShimFlowAllocatedEvent,    /* 10: A flow has been allocated on the shimWiFi*/
    eStackFlowAllocateEvent,    /* 11: The Software stack IPCP has received a Flow allocate request. */
    eStackAppRegistrationEvent, /* 12: The Software stack IPCP has received a AppRegistration Event*/
    eShimAppRegisteredEvent,    /* 13: The Normal IPCP has been registered into the Shim*/
    eSendMgmtEvent,             /* 14: Send Mgmt PDU */

} eRINAEvent_t;

typedef union xRINA_Event_Data
{
    void    *PV;
    uint32_t UN;
    int32_t  N;
    char     C;
    uint8_t  B;

    du_t *DU;

} RINAEventData_u;

/**
 * Structure for the information of the commands issued to the RINA task.
 */
typedef struct xRINA_Stack_Event
{
    eRINAEvent_t eEventType; /**< The event-type enum */
    RINAEventData_u xData;
    RINAEventData_u xData2;
} RINAStackEvent_t;

#ifdef __cplusplus
}
#endif

#endif // _COMPONENTS_IPCP_EVENTS_H
