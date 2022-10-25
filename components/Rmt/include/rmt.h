#ifndef COMPONENTS_RMT_INCLUDE_RMT_H_
#define COMPONENTS_RMT_INCLUDE_RMT_H_

#include "du.h"
#include "efcpStructures.h"
#include "IPCP_instance.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TAG_RMT "[RMT]"

/* rmt_enqueue_policy return values */
#define RMT_PS_ENQ_SEND 0  /* PDU can be transmitted by the RMT */
#define RMT_PS_ENQ_SCHED 1 /* PDU enqueued and RMT needs to schedule */
#define RMT_PS_ENQ_ERR 2   /* Error */
#define RMT_PS_ENQ_DROP 3  /* PDU dropped due to queue full occupation */

#define stats_inc(name, n1_port, bytes) \
    n1_port->xStats.name##Pdus++;       \
    n1_port->xStats.name##Bytes += (unsigned int)bytes;

typedef enum FLOW_STATE
{
    eN1_PORT_STATE_ENABLED = 0,
    eN1_PORT_STATE_DISABLED,
    eN1_PORT_STATE_DO_NOT_DISABLE,
    eN1_PORT_STATE_DEALLOCATED,
} eFlowState_t;

typedef struct xN1_PORT_STATS
{
    unsigned int plen; /* port len, all pdus enqueued in PS queue/s */
    unsigned int dropPdus;
    unsigned int errPdus;
    unsigned int txPdus;
    unsigned int txBytes;
    unsigned int rxPdus;
    unsigned int rxBytes;
} n1PortStats_t;

typedef struct xRMT_N1_PORT
{

    /* Is it required?
     spinlock_t		lock;*/

    /*Port Id from Shim*/
    portId_t xPortId;

    /* Shim Instance*/
    struct ipcpInstance_t *pxN1Ipcp;

    /* State of the Port*/
    eFlowState_t eState;

    /*Data Unit Pending to send*/
    struct du_t *pxPendingDu;

    /*N1 Port Statistics */
    n1PortStats_t xStats;

    /* If the Port is Busy or not*/
    bool_t uxBusy;

    /*???*/
    void *pvRmtPsQueues;

} rmtN1Port_t;

typedef struct xRMT_ADDRESS
{
    /*Address of the IPCP*/
    address_t xAddress;

    /*Address List Item*/
    RsListItem_t xAddressListItem;
} rmtAddress_t;

struct rmt_t
{
    /* List of Address */
    RsList_t xAddresses;

    /* IPCP Instances Parent*/
    struct ipcpInstance_t *pxParent;

    /* EFCP Container associated with */
    struct efcpContainer_t *pxEfcpc;

    /* N-1 */
    rmtN1Port_t *pxN1Port;
    struct rmt_Config_t *pxRmtCfg;
};

typedef struct xPORT_TABLE_ENTRY
{
    rmtN1Port_t *pxPortN1;

} portTableEntry_t;

pci_t *vCastPointerTo_pci_t(void *pvArgument);
bool_t xRmtSend(struct rmt_t *pxRmtInstance, struct du_t *pxDu);
struct rmt_t *pxRmtCreate(struct efcpContainer_t *pxEfcpc);
bool_t xRmtN1PortBind(struct rmt_t *pxRmtInstance, portId_t xId, struct ipcpInstance_t *pxN1Ipcp);
bool_t xRmtSendPortId(struct rmt_t *pxRmtInstance,
                      portId_t xPortId,
                      struct du_t *pxDu);

bool_t xRmtReceive(struct ipcpInstanceData_t *pxRmt, struct du_t *pxDu, portId_t xFrom);
bool_t xRmtAddressAdd(struct rmt_t *pxInstance, address_t xAddress);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_RMT_INCLUDE_DU_H_ */
