/*
 * efcpStructures.h
 *
 *  Created on: 11 oct. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_EFCP_INCLUDE_EFCPSTRUCTURES_H_
#define COMPONENTS_EFCP_INCLUDE_EFCPSTRUCTURES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "common/rina_ids.h"
#include "common/num_mgr.h"

#include "IPCP_instance.h"
#include "rina_common_port.h"
#include "du.h"
#include "rmt.h"
#include "delim.h"

struct dtp_t;

/* Retransmission Queue RTXQ used to buffer those PDUs
 * that may require retransmission */
/*typedef struct xRTXQ_ENTRY {
        unsigned long    ulTimeStamp;
        du_t *      	 pxDu;
        int              retries;
       // struct list_head next;
}rtxqEntry_t;*/

/* Close Window Queue (CWQ) used to buffer those PDUs
 * that cannot be transmitted when using flow control*/
typedef struct xCWQ
{

        RsQueue_t *pxQueue;
        // spinlock_t      lock;
} cwq_t;

typedef struct xRTX_QUEUE
{
        int len;
        int dropPdus;
        // struct list_head head;
} rtxqueue_t;

typedef struct xRTXQ
{
        // spinlock_t                lock;
        struct dtp_t *pxParent;
        struct rmt_t *rmt;
        rtxqueue_t *queue;
} rtxq_t;

typedef struct xRTT_ENTRY
{
        unsigned long ulTimeStamp;
        seqNum_t xSeqNum;
        // struct list_head next;
} rttEntry_t;

typedef struct xRTTQ
{
        // spinlock_t lock;
        struct dtp *pxParent;
        // struct list_head head;
} rttq_t;

/* This is the DT-SV part maintained by DTP */
typedef struct xDTP_SV
{
        /*Less or equal to those defined for the DIF*/
        uint_t xMaxFlowPduSize; // OK
        uint_t xMaxFlowSduSize; // OK
        // timeout_t    MPL;
        // timeout_t    R;
        // timeout_t    A;
        // timeout_t    tr;
        seqNum_t xRcvLeftWindowEdge;
        bool_t xWindowClosed;

        /* Indicates that the next PDU sent should have the
        DRF set. */
        bool_t xDrfFlag;
        /* used to notifies that a new connection will soon
        be needed to avoid sequence number rollover.*/
        uint_t xSeqNumberRolloverThreshold; // OK
        /* FIXME: we need to control rollovers...*/
        struct
        {
                unsigned int drop_pdus; // OK
                unsigned int err_pdus;  // OK
                unsigned int tx_pdus;   // ok
                unsigned int tx_bytes;  // ok
                unsigned int rx_pdus;   // ok
                unsigned int rx_bytes;  // ok
        } stats;
        seqNum_t xMaxSeqNumberRcvd;    // ok
        seqNum_t xNextSeqNumberToSend; // ok
        seqNum_t xMaxSeqNumberToSend;  // ok

        bool_t xWindowBased;
        bool_t xRexmsnCtrl;
        bool_t xRateBased;
        bool_t xDrfRequired;
        bool_t xRateFulfiled;
} dtpSv_t;

typedef struct xDTCP_SV
{
        /* TimeOuts */
        /*
         * When flow control is rate based this timeout may be
         * used to pace number of PDUs sent in TimeUnit
         */

        uint_t pdus_per_time_unit;
/*Do not consider control stage yet, but defined for the dtcp struct */
#if 0
        /* Sequencing */

        /*
         * Outbound: NextSndCtlSeq contains the Sequence Number to
         * be assigned to a control PDU
         */
        seq_num_t    next_snd_ctl_seq;

        /*
         * Inbound: LastRcvCtlSeq - Sequence number of the next
         * expected // Transfer(? seems an error in the spec�s
         * doc should be Control) PDU received on this connection
         */
        seq_num_t    last_rcv_ctl_seq;

        /*
         * Retransmission: There�s no retransmission queue,
         * when a lost PDU is detected a new one is generated
         */

        /* Outbound */
        seq_num_t    last_snd_data_ack;

        /*
         * Seq number of the lowest seq number expected to be
         * Acked. Seq number of the first PDU on the
         * RetransmissionQ. My LWE thus.
         */
        seq_num_t    snd_lft_win;

        /*
         * Maximum number of retransmissions of PDUs without a
         * positive ack before declaring an error
         */
        uint_t       data_retransmit_max;

        /* Inbound */
        seq_num_t    last_rcv_data_ack;

        /* Time (ms) over which the rate is computed */
        uint_t       time_unit;

        /* Flow Control State */

        /* Outbound */
        uint_t       sndr_credit;

        /* snd_rt_wind_edge = LastSendDataAck + PDU(credit) */
        seq_num_t    snd_rt_wind_edge;

        /* PDUs per TimeUnit */
        uint_t       sndr_rate;

        /* PDUs already sent in this time unit */
        uint_t       pdus_sent_in_time_unit;

        /* Inbound */

        /*
         * PDUs receiver believes sender may send before extending
         * credit or stopping the flow on the connection
         */
        uint_t       rcvr_credit;

        /* Value of credit in this flow */
        seq_num_t    rcvr_rt_wind_edge;

        /*
         * Current rate receiver has told sender it may send PDUs
         * at.
         */
        uint_t       rcvr_rate;

        /*
         * PDUs received in this time unit. When it equals
         * rcvr_rate, receiver is allowed to discard any PDUs
         * received until a new time unit begins
         */
        uint_t       pdus_rcvd_in_time_unit;

        /* Rate based both in and out-bound */

		/* Last time-instant when the credit check has been done.
		 * This is used by rate-based flow control mechanism.
		 */

	struct timespec64 last_time;


        /*
         * Control of duplicated control PDUs
         * */
        uint_t       acks;
        uint_t       flow_ctl;

        /* RTT estimation */
        uint_t       rtt;
        uint_t       srtt;
        uint_t       rttvar;

        /* Rendezvous */

        /* This Boolean indicates whether there is a zero-length window and a
         * Rendezvous PDU has been sent.
         */
        bool         rendezvous_sndr;

        /* This Boolean indicates whether a Rendezvous PDU was received. The
         * next DT-PDU is expected to have a DRF bit set to true.
         */
        bool         rendezvous_rcvr;
#endif
} dtcpSv_t;

/* This is the DTCP configurations from connection policies */
struct dtcpConfig_t
{
        bool_t xFlowCtrl;
        struct dtcp_fctrl_config *fctrl_cfg;
        bool_t rtx_ctrl;
        struct dtcp_rxctrl_config *rxctrl_cfg;
        policy_t *lost_control_pdu;
        policy_t *dtcp_ps;
        policy_t *rtt_estimator;
};

typedef struct xDTCP
{
        struct dtp_t *pxParent;
        /*
         * NOTE: The DTCP State Vector can be discarded during long periods of
         *       no traffic
         */
        dtcpSv_t *pxSv; /* The state-vector */

        struct dtcpConfig_t *pxCfg;
        struct rmt_t *pxRmt;
        // struct timer_list 	   rendezvous_rcv;

} dtcp_t;

struct dtp_t
{

        dtcp_t *pxDtcp;
        struct efcp_t *pxEfcp;

        cwq_t *pxCwq;
        rtxq_t *pxRtxq;
        rttq_t *pxRttq;

        /*
         * NOTE: The DTP State Vector is discarded only after and explicit
         *       release by the AP or by the system (if the AP crashes).
         */
        dtpSv_t *pxDtpStateVector; /* The state-vector */
                                   // spinlock_t          sv_lock; /* The state vector lock (DTP & DTCP) */

        dtpConfig_t *pxDtpCfg;
        struct rmt_t *pxRmt;
        // struct squeue *           seqq;
        // struct ringq *            to_post;
        // struct ringq *            to_send;
        /*struct {
                struct timer_list sender_inactivity;
                struct timer_list receiver_inactivity;
                struct timer_list a;
                struct timer_list rate_window;
                struct timer_list rtx;
                struct timer_list rendezvous;
        } timers;*/

};

struct connection_t
{
        portId_t xPortId;
        address_t xSourceAddress;
        address_t xDestinationAddress;
        cepId_t xSourceCepId;
        cepId_t xDestinationCepId;
        qosId_t xQosId;
};

typedef enum
{
        eEfcpAllocated = 1,
        eEfcpDeallocated
} eEfcpState_t;

// struct efcp_t;

typedef struct xEFCP_IMAP_ROW
{
        cepId_t xCepIdKey;
        struct efcp_t *xEfcpValue;
        uint8_t ucValid;

} efcpImapRow_t;

struct efcpContainer_t
{
        // struct rset *        rset;
        efcpImapRow_t *pxEfcpImap;
        NumMgr_t *pxCidm;
        // cepIdm_t                *pxCidm;
        efcpConfig_t *pxConfig;
        struct rmt_t *pxRmt;
        // struct kfa *         kfa;
        // spinlock_t           lock;
        // wait_queue_head_t    del_wq;
};

struct efcp_t
{

        struct connection_t *pxConnection;
        struct ipcpInstance_t *pxUserIpcp; // IPCP NORMAL
        struct dtp_t *pxDtp;               // implement in EFCP Component
        delim_t *pxDelim;           // delimiting module
        struct efcpContainer_t *pxEfcpContainer;
        eEfcpState_t xState;
};

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_EFCP_INCLUDE_EFCPSTRUCTURES_H_ */
