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
#include "common/macros.h"

#include "rina_common_port.h"
#include "du.h"
#include "rmt.h"
#include "delim.h"

typedef struct
{
    portId_t xPortId;
    address_t xSourceAddress;
    address_t xDestinationAddress;
    cepId_t xSourceCepId;
    cepId_t xDestinationCepId;
    qosId_t xQosId;
} connection_t;

/* This is the DTCP configurations from connection policies */
typedef struct
{
    bool_t xFlowCtrl;
    bool_t rtx_ctrl;

    policy_t *lost_control_pdu;
    policy_t *dtcp_ps;
    policy_t *rtt_estimator;
} dtcpConfig_t;

typedef struct xDTCP_SV
{
    /*
     * When flow control is rate based this timeout may be
     * used to pace number of PDUs sent in TimeUnit
     */
    uint_t pdus_per_time_unit;

    /*
     * Lots of things were chucked off this structure. See the IRATI
     * kernel definition for the full thing.
     */
} dtcpSv_t;

/* This is the DT-SV part maintained by DTP */
typedef struct
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

typedef struct
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

/* Retransmission Queue RTXQ used to buffer those PDUs
 * that may require retransmission */
/*typedef struct xRTXQ_ENTRY {
        unsigned long    ulTimeStamp;
        du_t *      	 pxDu;
        int              retries;
       // struct list_head next;
}rtxqEntry_t;*/



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
    connection_t *pxConnection;
    struct ipcpInstance_t *pxUserIpcp; // IPCP NORMAL
    struct dtp_t *pxDtp;               // implement in EFCP Component
    delim_t *pxDelim;           // delimiting module
    struct efcpContainer_t *pxEfcpContainer;
    eEfcpState_t xState;
};

#define IPCP_DATA_FROM_EFCP_CONTAINER(x) \
    container_of(x, struct ipcpInstanceData_t, xEfcpContainer)

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_EFCP_INCLUDE_EFCPSTRUCTURES_H_ */
