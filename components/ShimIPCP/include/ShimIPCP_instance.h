#ifndef _SHIMIPCP_INSTANCE_H_INCLUDED
#define _SHIMIPCP_INSTANCE_H_INCLUDED

#include "ShimIPCP.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "common/list.h"
#include "common/rina_ids.h"

#include "ARP826_defs.h"
#include "FlowAllocator_defs.h"

/* Safeguards against mistakenly including a file which uses another
 * definition of struct ipcpInstance_data */
#ifdef IPCP_INSTANCE_DATA_TYPE
#error IPCP_INSTANCE_DATA_TYPE should not be defined here
#endif

struct ipcpInstanceData_t
{
#ifndef NDEBUG
    /* Used to assert on the type of instance data we're address is
     * correct. */
    uint8_t unInstanceDataType;
#endif

    pthread_mutex_t xLock;

	ipcProcessId_t xId;

    ARP_t xARP;

	/* IPC Process name */
	rname_t xName;
	rname_t xDifName;
	string_t pcInterfaceName;

    /* Physical interface address to which this flow is attached. */
	MACAddress_t xPhyDev;

	flowSpec_t xFspec;

    /* Memory pool for netbufs */
    rsrcPoolP_t pxNbPool;

    /* Memory pool for ethernet headers */
    rsrcPoolP_t pxEthPool;

    /* Hardware address for this shim */
    gha_t *pxHa;

	/* The IPC Process using the shim-WiFi */
	rname_t xAppName;
	rname_t xDafName;

    /* ARP registration names corresponding to the name_t above. */
    gpa_t *pxAppPa;
    gpa_t *pxDafPa;

	/* Stores the state of flows indexed by port_id */
	// spinlock_t             lock;
	//RsList_t xFlowsList;
    shimFlow_t *xFlows[256];

	/* FIXME: Remove it as soon as the kipcm_kfa gets removed */
	// struct kfa *           kfa;

	/* To handle device notifications. */
	// struct notifier_block ntfy;

	/* Flow control between this IPCP and the associated netdev. */
	unsigned int ucTxBusy;
};

#ifdef __cplusplus
}
#endif

#endif /* _SHIMIPCP_INSTANCE_H_INCLUDED */
