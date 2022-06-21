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

#include "rina_ids.h"

#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({         \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type,member) ); })

struct flowSpec_t
{
        /* This structure defines the characteristics of a flow */

        /* Average bandwidth in bytes/s */
        uint32_t ulAverageBandwidth;

        /* Average bandwidth in SDUs/s */
        uint32_t ulAverageSduBandwidth;

        /*
         * In milliseconds, indicates the maximum delay allowed in this
         * flow. A value of 0 indicates 'do not care'
         */
        uint32_t ulDelay;
        /*
         * In milliseconds, indicates the maximum jitter allowed
         * in this flow. A value of 0 indicates 'do not care'
         */
        uint32_t ulJitter;

        /*
         * Indicates the maximum packet loss (loss/10000) allowed in this
         * flow. A value of loss >=10000 indicates 'do not care'
         */
        uint16_t usLoss;

        /*
         * Indicates the maximum gap allowed among SDUs, a gap of N
         * SDUs is considered the same as all SDUs delivered.
         * A value of -1 indicates 'Any'
         */
        int32_t ulMaxAllowableGap;

        /*
         * The maximum SDU size for the flow. May influence the choice
         * of the DIF where the flow will be created.
         */
        uint32_t ulMaxSduSize;

        /* Indicates if SDUs have to be delivered in order */
        BaseType_t xOrderedDelivery;

        /* Indicates if partial delivery of SDUs is allowed or not */
        BaseType_t xPartialDelivery;

        /* In milliseconds */
        uint32_t ulPeakBandwidthDuration;

        /* In milliseconds */
        uint32_t ulPeakSduBandwidthDuration;

        /* A value of 0 indicates 'do not care' */
        uint32_t ulUndetectedBitErrorRate;

        /* Preserve message boundaries */
        BaseType_t xMsgBoundaries;
};

typedef enum FRAMES_PROCESSING
{
        eReleaseBuffer = 0,   /* Processing the frame did not find anything to do - just release the buffer. */
        eProcessBuffer,       /* An Ethernet frame has a valid address - continue process its contents. */
        eReturnEthernetFrame, /* The Ethernet frame contains an ARP826 packet that can be returned to its source. */
        eFrameConsumed        /* Processing the Ethernet packet contents resulted in the payload being sent to the stack. */
} eFrameProcessingResult_t;

typedef struct xPOLICY
{
        /* The name of the policy */
        string_t pcPolicyName;

        /* The version of the policy implementation */
        string_t pcPolicyVersion;

        /* The paramters of the policy */
        List_t xParameters;
} policy_t;

typedef struct xCONNECTION_ID
{
        qosId_t xQosId;       /**< QoS Id  3 + 1 = 4 */
        cepId_t xDestination; /**< Cep Id Dest 4 + 1 = 5 */
        cepId_t xSource;      /**< Cep Id Source  5 + 1 = 6 */
} connectionId_t;

/* The structure is useful when the Data Transfers value are variable
 * when it is required to change the PCI fields. Now it is config by
 * default. Future improvements will consider this.*/
typedef struct xDT_CONS
{
        /* The length of the address field in the DTP PCI, in bytes */
        uint16_t address_length;

        /* The length of the CEP-id field in the DTP PCI, in bytes */
        uint16_t cep_id_length;

        /* The length of the length field in the DTP PCI, in bytes */
        uint16_t length_length;

        /* The length of the Port-id field in the DTP PCI, in bytes */
        uint16_t port_id_length;

        /* The length of QoS-id field in the DTP PCI, in bytes */
        uint16_t qos_id_length;

        /* The length of the sequence number field in the DTP PCI, in bytes */
        uint16_t seq_num_length;

        /* The length of the sequence number field in the DTCP PCI, in bytes */
        uint16_t ctrl_seq_num_length;

        /* The maximum length allowed for a PDU in this DIF, in bytes */
        uint32_t max_pdu_size;

        /* The maximum size allowed for a SDU in this DIF, in bytes */
        uint32_t max_sdu_size;

        /*
         * The maximum PDU lifetime in this DIF, in milliseconds. This is MPL
         * in delta-T
         */
        uint32_t max_pdu_life;

        /* Rate for rate based mechanism. */
        uint16_t rate_length;

        /* Time frame for rate based mechanism. */
        uint16_t frame_length;

        /*
         * True if the PDUs in this DIF have CRC, TTL, and/or encryption.
         * Since headers are encrypted, not just user data, if any flow uses
         * encryption, all flows within the same DIF must do so and the same
         * encryption algorithm must be used for every PDU; we cannot identify
         * which flow owns a particular PDU until it has been decrypted.
         */
        bool dif_integrity;

        uint32_t seq_rollover_thres;
        bool dif_concat;
        bool dif_frag;
        uint32_t max_time_to_keep_ret_;
        uint32_t max_time_to_ack_;

} dtCons_t;

/* Represents the configuration of the EFCP */
typedef struct xEFCP_CONFIG
{

        /* The data transfer constants. FUTURE IMPLEMENTATION */
        dtCons_t *pxDtCons;

        /* Useful when the dt sizes are variable. FUTURE IMPLEMENTATION*/
        size_t *uxPciOffsetTable;

        /* Policy to implement. FUTURE IMPLEMENTATION*/
        policy_t *pxUnknownFlow;

        /* List of qos_cubes supported by the EFCP config */
        List_t xQosCubesList;

} efcpConfig_t;

typedef struct xRMT_CONFIG
{
        /* The PS name for the RMT */
        policy_t *pxPolicySet;

        /* The configuration of the PDU Forwarding Function subcomponent */
        // struct pff_config * pff_conf;
} rmtConfig_t;

/* Represents a DIF configuration (policies, parameters, etc) */
typedef struct xDIF_CONFIG
{
        /* List of configuration entries */
        List_t xIpcpConfigEntries;

        /* the config of the efcp */
        efcpConfig_t *pxEfcpConfig;

        /* the config of the rmt */
        rmtConfig_t *pxRmtConfig;

        /* The address of the IPC Process*/
        address_t xAddress;
        /*
        struct fa_config * fa_config;
        struct et_config * et_config;
        struct nsm_config * nsm_config;
        struct routing_config * routing_config;
        struct resall_config * resall_config;
        struct secman_config * secman_config;*/
} difConfig_t;

typedef struct xDTP_CONFIG
{
        /* It is DTCP used in this config: default-pdFALSE */
        BaseType_t xDtcpPresent;

        /* Sequence number rollover threshold */
        int seqNumRoTh;

        timeout_t xInitialATimer;
        BaseType_t xPartialDelivery;
        BaseType_t xIncompleteDelivery;
        BaseType_t xInOrderDelivery;
        seqNum_t xMaxSduGap;

        /* Describes a policy */
        policy_t *pxDtpPolicySet;
} dtpConfig_t;

/* Endian related definitions. */
//#if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )

/* FreeRTOS_htons / FreeRTOS_htonl: some platforms might have built-in versions
 * using a single instruction so allow these versions to be overridden. */
//#ifndef FreeRTOS_htons
#define FreeRTOS_htons(usIn) ((uint16_t)(((usIn) << 8U) | ((usIn) >> 8U)))
//#endif

#ifndef FreeRTOS_htonl
#define FreeRTOS_htonl(ulIn)                                        \
        (                                                           \
            (uint32_t)(((((uint32_t)(ulIn))) << 24) |               \
                       ((((uint32_t)(ulIn)) & 0x0000ff00UL) << 8) | \
                       ((((uint32_t)(ulIn)) & 0x00ff0000UL) >> 8) | \
                       ((((uint32_t)(ulIn))) >> 24)))
#endif /* ifndef FreeRTOS_htonl */

//#else /* ipconfigBYTE_ORDER */

//#define FreeRTOS_htons( x )    ( ( uint16_t ) ( x ) )
//#define FreeRTOS_htonl( x )    ( ( uint32_t ) ( x ) )

//#endif /* ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN */

#define FreeRTOS_ntohs(x) FreeRTOS_htons(x)
#define FreeRTOS_ntohl(x) FreeRTOS_htonl(x)
#define SOCKET_EVENT_BIT_COUNT 8

enum eFLOW_EVENT
{
        eFLOW_RECEIVE = 0x0001,
        eFLOW_SEND = 0x0002,
        eFLOW_ACCEPT = 0x0004,
        eFLOW_CONNECT = 0x0008,
        eFLOW_BOUND = 0x0010,
        eFLOW_CLOSED = 0x0020,
        eSELECT_ALL = 0x000F,

};

typedef struct xAUTH_POLICY
{
        string_t pcName;
        string_t pcVersion;
        uint8_t ucAbsSyntax;

} authPolicy_t;

// name_t *xRinaNameCreate(void);

// BaseType_t xRinaNameFromString(const string_t pcString, name_t *xName);
// void xRinaNameFree(name_t *xName);

// BaseType_t xRINAStringDup(const string_t *pcSrc, string_t **pcDst);

// name_t *xRINAstringToName(const string_t *pxInput);

void memcheck(void);

int get_next_invoke_id(void);

#endif /* COMPONENTS_IPCP_INCLUDE_COMMON_H_ */
