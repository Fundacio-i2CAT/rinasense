#ifndef _COMPONENTS_IPCP_INCLUDE_IPCP_FRAMES_H
#define _COMPONENTS_IPCP_INCLUDE_IPCP_FRAMES_H

#ifdef __cplusplus
extern "C" {
#endif

/* FIXME: THE TERM "FRAME" HAS NO BUSINESS IN THE IPCP, LIKE, AT
 * ALL. THIS SHOULD BE BROKEN DOWN AND MOVED ELSEWHERE. */

typedef enum FRAMES_PROCESSING
{
    /* Processing the frame did not find anything to do - just release the buffer. */
    eReleaseBuffer = 0,

    /* An Ethernet frame has a valid address - continue process its contents. */
    eProcessBuffer,

    /* The Ethernet frame contains an ARP826 packet that can be returned to its source. */
    eReturnEthernetFrame,

    /* Processing the Ethernet packet contents resulted in the payload being sent to the stack. */
    eFrameConsumed
} eFrameProcessingResult_t;

#ifdef __cplusplus
}
#endif

#endif // _COMPONENTS_IPCP_INCLUDE_IPCP_FRAMES_H
