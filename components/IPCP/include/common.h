/*
 * common.h
 *
 *  Created on: 21 oct. 2021
 *      Author: i2CAT
 */


#include "configSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"



#ifndef COMPONENTS_IPCP_INCLUDE_COMMON_H_
#define COMPONENTS_IPCP_INCLUDE_COMMON_H_

typedef char* string_t;

typedef struct xName_info
{
	string_t  pcProcessName;  			/*> Process Name*/
	string_t  pcProcessInstance;		/*> Process Instance*/
	string_t  pcEntityName;				/*> Entity Name*/
	string_t  pcEntityInstance;		    /*> Entity Instance*/

}name_t;

struct flowSpec_t
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
};


typedef struct xNETWORK_BUFFER
{
    ListItem_t xBufferListItem;                /**< Used to reference the buffer form the free buffer list or a socket. */
    uint8_t ulGpa;                      		/**< Source or destination Protocol address, depending on usage scenario. */
    uint8_t * pucEthernetBuffer;               /**< Pointer to the start of the Ethernet frame. */
    size_t xDataLength;                        /**< Starts by holding the total Ethernet frame length, then the UDP/TCP payload length. */
    uint16_t usPort;                           /**< Source or destination port, depending on usage scenario. */
    uint16_t usBoundPort;                      /**< The port to which a transmitting socket is bound. */

} NetworkBufferDescriptor_t;
typedef enum FRAMES_PROCESSING
{
	eReleaseBuffer = 0,   /* Processing the frame did not find anything to do - just release the buffer. */
	eProcessBuffer,       /* An Ethernet frame has a valid address - continue process its contents. */
	eReturnEthernetFrame, /* The Ethernet frame contains an ARP826 packet that can be returned to its source. */
	eFrameConsumed        /* Processing the Ethernet packet contents resulted in the payload being sent to the stack. */
} eFrameProcessingResult_t;

typedef struct xPOLICY {
        string_t *       pxName;
        string_t *       pxVersion;
        //struct list_head params;//List_Freertos
}policy_t;

/* Represents the configuration of the EFCP */
typedef struct  xEFCP_CONFIG{
	 // The data transfer constants
	//struct dt_cons * dt_cons;

	size_t *		uxPciOffsetTable;

	 // FIXME: Left here for phase 2
	//struct policy * unknown_flow;

	// List of qos_cubes supported by the EFCP config
	//struct list_head qos_cubes;

}efcpConfig_t;

typedef struct xDTP_CONFIG
{
#if 0
	bool                 dtcp_present;
        /* Sequence number rollover threshold */
        int                  seq_num_ro_th;
        //timeout_t            initial_a_timer;
        bool                 partial_delivery;
        bool                 incomplete_delivery;
        bool                 in_order_delivery;
        //seq_num_t            max_sdu_gap;
#endif
        policy_t *      pxDtpPs;

}dtpConfig_t;

/* This is the DTCP configurations from connection policies */
typedef struct xDTCP_CONFIG
{
        BaseType_t                    xFlowCtrl;
        struct dtcp_fctrl_config *  fctrl_cfg;
        bool                        rtx_ctrl;
        struct dtcp_rxctrl_config * rxctrl_cfg;
        policy_t *             lost_control_pdu;
        policy_t *             dtcp_ps;
        policy_t *             rtt_estimator;
}dtcpConfig_t;

#endif /* COMPONENTS_IPCP_INCLUDE_COMMON_H_ */
