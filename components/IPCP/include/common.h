/*
 * common.h
 *
 *  Created on: 21 oct. 2021
 *      Author: i2CAT
 */







#ifndef COMPONENTS_IPCP_INCLUDE_COMMON_H_
#define COMPONENTS_IPCP_INCLUDE_COMMON_H_

#include "configSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({         \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type,member) );})

#define PORT_ID_WRONG -1
#define CEP_ID_WRONG -1
#define ADDRESS_WRONG -1
#define QOS_ID_WRONG -1

typedef int32_t  portId_t;

typedef uint32_t seqNum_t;

/* CEPIdLength field 1 Byte*/
typedef uint8_t  cepId_t;

/* QoSIdLength field 1 Byte*/
typedef uint8_t  qosId_t;

/* QoSIdLength field 1 Byte*/
typedef uint8_t address_t;


typedef char* string_t;
typedef unsigned int  uint_t;
typedef unsigned int  timeout_t;
/* SeqNumLength field 4 Byte*/
typedef uint32_t seqNum_t;;


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

typedef struct xDTP_CONFIG {
        BaseType_t                 xDtcpPresent;
        /* Sequence number rollover threshold */
        int                  seqNumRoTh;
        timeout_t            xInitialATimer;
        BaseType_t                  xPartialDelivery;
        BaseType_t                  xIncompleteDelivery;
        BaseType_t                  xInOrderDelivery;
        seqNum_t            xMaxSduGap;

        struct policy *      dtp_ps;
}dtpConfig_t;

/* Endian related definitions. */
//#if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )

/* FreeRTOS_htons / FreeRTOS_htonl: some platforms might have built-in versions
 * using a single instruction so allow these versions to be overridden. */
//#ifndef FreeRTOS_htons
#define FreeRTOS_htons( usIn )    ( ( uint16_t ) ( ( ( usIn ) << 8U ) | ( ( usIn ) >> 8U ) ) )
//#endif

#ifndef FreeRTOS_htonl
#define FreeRTOS_htonl( ulIn )                          \
		(                                                               \
				( uint32_t )                                                \
				(                                                           \
						( ( ( ( uint32_t ) ( ulIn ) ) ) << 24 ) |               \
						( ( ( ( uint32_t ) ( ulIn ) ) & 0x0000ff00UL ) << 8 ) | \
						( ( ( ( uint32_t ) ( ulIn ) ) & 0x00ff0000UL ) >> 8 ) | \
						( ( ( ( uint32_t ) ( ulIn ) ) ) >> 24 )                 \
				)                                                           \
		)
#endif /* ifndef FreeRTOS_htonl */

//#else /* ipconfigBYTE_ORDER */

//#define FreeRTOS_htons( x )    ( ( uint16_t ) ( x ) )
//#define FreeRTOS_htonl( x )    ( ( uint32_t ) ( x ) )

//#endif /* ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN */

#define FreeRTOS_ntohs( x )    FreeRTOS_htons( x )
#define FreeRTOS_ntohl( x )    FreeRTOS_htonl( x )




//Structure MAC ADDRESS
typedef struct xMAC_ADDRESS
{
	uint8_t ucBytes[ MAC_ADDRESS_LENGTH_BYTES ]; /**< Byte array of the MAC address */
}MACAddress_t;

cepId_t cep_id_bad(void);

/* ALWAYS use this function to check if the id looks good */
BaseType_t      is_port_id_ok(portId_t id);

/* ALWAYS use this function to get a bad id */
portId_t port_id_bad(void);

/* ALWAYS use this function to check if the id looks good */
BaseType_t    is_cep_id_ok(cepId_t id);

/* ALWAYS use this function to get a bad id */
cepId_t cep_id_bad(void);

BaseType_t     is_address_ok(address_t address);

address_t address_bad(void);

/* ALWAYS use this function to check if the id looks good */
BaseType_t is_qos_id_ok(qosId_t id);



#endif /* COMPONENTS_IPCP_INCLUDE_COMMON_H_ */
