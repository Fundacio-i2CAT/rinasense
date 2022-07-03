#ifndef _COMPONENTS_IPCP_INCLUDE_IPCP_FRAMES_H
#define _COMPONENTS_IPCP_INCLUDE_IPCP_FRAMES_H

typedef enum FRAMES_PROCESSING
{
        eReleaseBuffer = 0,   /* Processing the frame did not find anything to do - just release the buffer. */
        eProcessBuffer,       /* An Ethernet frame has a valid address - continue process its contents. */
        eReturnEthernetFrame, /* The Ethernet frame contains an ARP826 packet that can be returned to its source. */
        eFrameConsumed        /* Processing the Ethernet packet contents resulted in the payload being sent to the stack. */
} eFrameProcessingResult_t;

#endif // _COMPONENTS_IPCP_INCLUDE_IPCP_FRAMES_H
