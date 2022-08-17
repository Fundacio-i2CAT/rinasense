/* This file will hold the definition from rina_common.h that are
 * independent of FreeRTOS. Eventually, the content of this file
 * should replace rina_common.h */

#ifndef _COMPONENTS_IPCP_INCLUDE_COMMON_PORT_H
#define _COMPONENTS_IPCP_INCLUDE_COMMON_PORT_H

#include <unistd.h>

#include "portability/port.h"
#include "rina_ids.h"

typedef struct xQUEUE_FIFO
{
    RsQueue_t *xQueue;

} rfifo_t;

#define BITS_PER_BYTE 8
#define MAX_PORT_ID (((2 << BITS_PER_BYTE) * sizeof(portId_t)) - 1)
#define MAX_IPCP_ID (((2 << BITS_PER_BYTE) * sizeof(ipcProcessId_t)) - 1)
#define MAX_CEP_ID (((2 << BITS_PER_BYTE) * sizeof(cepId_t)) - 1)

struct ipcpInstance_t;
struct ipcpInstanceData_t;
struct du_t;

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
        bool_t xOrderedDelivery;

        /* Indicates if partial delivery of SDUs is allowed or not */
        bool_t xPartialDelivery;

        /* In milliseconds */
        uint32_t ulPeakBandwidthDuration;

        /* In milliseconds */
        uint32_t ulPeakSduBandwidthDuration;

        /* A value of 0 indicates 'do not care' */
        uint32_t ulUndetectedBitErrorRate;

        /* Preserve message boundaries */
        bool_t xMsgBoundaries;
};

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

typedef struct xPOLICY
{
        /* The name of the policy */
        string_t pcPolicyName;

        /* The version of the policy implementation */
        string_t pcPolicyVersion;

        /* The paramters of the policy */
        RsList_t xParameters;
} policy_t;

typedef struct xDTP_CONFIG
{
        /* It is DTCP used in this config: default-pdFALSE */
        bool_t xDtcpPresent;

        /* Sequence number rollover threshold */
        int seqNumRoTh;

        timeout_t xInitialATimer;
        bool_t xPartialDelivery;
        bool_t xIncompleteDelivery;
        bool_t xInOrderDelivery;
        seqNum_t xMaxSduGap;

        /* Describes a policy */
        policy_t *pxDtpPolicySet;
} dtpConfig_t;

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
        RsList_t xQosCubesList;

} efcpConfig_t;

typedef struct xAUTH_POLICY
{
        string_t pcName;
        string_t pcVersion;
        uint8_t ucAbsSyntax;

} authPolicy_t;

typedef struct xCONNECTION_ID
{
        qosId_t xQosId;       /**< QoS Id  3 + 1 = 4 */
        cepId_t xDestination; /**< Cep Id Dest 4 + 1 = 5 */
        cepId_t xSource;      /**< Cep Id Source  5 + 1 = 6 */
} connectionId_t;

int get_next_invoke_id(void);

/**
 * The software timer struct for various IPCP functions
 */
typedef struct xIPCP_TIMER
{
    bool_t bActive;             /**< This timer is running and must be processed. */
    bool_t bExpired;            /**< Timer has expired and a task must be processed. */
    struct timespec xTimeOut;   /**< The timeout value. */
    useconds_t ulRemainingTimeUS; /**< The amount of time remaining. */
    useconds_t ulReloadTimeUS;    /**< The value of reload time. */
} IPCPTimer_t;

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

#endif // _COMPONENTS_IPCP_INCLUDE_COMMON_PORT_H#

