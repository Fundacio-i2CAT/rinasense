#include <stdio.h>

#include "common/netbuf.h"
#include "portability/port.h"
#include "common/rina_ids.h"
#include "common/list.h"

#include "configRINA.h"

#include "IPCP_normal_api.h"
#include "IpcManager.h"
#include "efcpStructures.h"
#include "rmt.h"
#include "du.h"
#include "pci.h"
#include "IPCP.h"
#include "IPCP_instance.h"
#include "EFCP.h"

rmtN1Port_t *prvRmtPortFind(struct rmt_t *pxRmt, portId_t unPort)
{
    rmtN1Port_t *pxPort;
    RsListItem_t *pxListItem;

    pxListItem = pxRsListGetFirst(&pxRmt->xPorts);

    while (pxListItem != NULL) {
        pxPort = (rmtN1Port_t *)pxRsListGetItemOwner(pxListItem);

        if (pxPort && pxPort->unPort == unPort)
            return pxPort;

        pxListItem = pxRsListGetNext(pxListItem);
    }

    return NULL;
}

bool_t xRmtInit(struct rmt_t *pxRmt)
{
	rmtN1Port_t *pxPortN1;

    RsAssert(pxRmt);

    if (!(pxPortN1 = pvRsMemAlloc(sizeof(rmtN1Port_t)))) {
        LOGE(TAG_RMT, "Failed to allocate memory for port");
        return false;
    }

	vRsListInit(&pxRmt->xAddresses);
    vRsListInit(&pxRmt->xPorts);

#if 0
	// tmp->n1_ports = n1pmap_create(&tmp->robj);
	if (!pxRmtTmp->pxN1Port)
	{
		LOGI(TAG_RMT, "Failed to create N-1 ports map");
		// rmt_destroy(tmp);
		return NULL;
	}
#endif

	/*if (pff_cache_init(&tmp->cache)) {
		LOG_ERR("Failed to init pff cache");
		rmt_destroy(tmp);
		return NULL;
	}

	tasklet_init(&tmp->egress_tasklet,
			 send_worker,
			 (unsigned long) tmp);*/

	LOGI(TAG_RMT, "RMT initialized successfully");

    return true;
}

void vRmtFini(struct rmt_t *pxRmt)
{
    RsAssert(pxRmt);
}

/* @brief Called when a SDU arrived into the RMT from the Shim DIF */
// bool_t xRmtReceive(struct ipcpInstance_t *pxRmt, struct du_t *pxDu, portId_t xFrom);

/* @brief Called when a SDU arrived into the RMT from the EFCP Container*/
static bool_t xRmtN1PortWriteDu(struct rmt_t *pxRmt, rmtN1Port_t *pxN1Port, du_t *pxDu);

/* Create an N-1 Port in the RMT Component*/
static rmtN1Port_t *pxRmtN1PortCreate(portId_t unPort, struct ipcpInstance_t *pxN1Ipcp)
{
	rmtN1Port_t *pxTmp;

	RsAssert(is_port_id_ok(unPort));
    RsAssert(pxN1Ipcp);

	if (!(pxTmp = pvRsMemAlloc(sizeof(rmtN1Port_t)))) {
        LOGE(TAG_RMT, "Failed to allocate memory for port");
		return NULL;
    }

	pxTmp->unPort = unPort;
	pxTmp->pxN1Ipcp = pxN1Ipcp;
	pxTmp->eState = eN1_PORT_STATE_ENABLED;

	pxTmp->uxBusy = false;
	pxTmp->xStats.plen = 0;
	pxTmp->xStats.dropPdus = 0;
	pxTmp->xStats.errPdus = 0;
	pxTmp->xStats.txPdus = 0;
	pxTmp->xStats.txBytes = 0;
	pxTmp->xStats.rxPdus = 0;
	pxTmp->xStats.rxBytes = 0;

    vRsListInitItem(&pxTmp->xPortItem, pxTmp);

	LOGI(TAG_RMT, "N-1 port %pK created successfully (port = %d)", pxTmp, unPort);

	return pxTmp;
}

/* Bind the N-1 Port with the RMT. SDUP and RMT Policies are not
 * considered.  It is called from the IPCP normal when a Flow is
 * required to be bound. From the IPCP normal is send the RMT
 * instance, the portId from the Shim, and Shim Instance */
bool_t xRmtN1PortBind(struct rmt_t *pxRmt, portId_t unPort, struct ipcpInstance_t *pxN1Ipcp)
{
	rmtN1Port_t *pxTmp;

	LOGI(TAG_RMT, "Binding the RMT with the port id:%d", unPort);

    RsAssert(pxRmt);
    RsAssert(is_port_id_ok(unPort));
    RsAssert(pxN1Ipcp);

    if (prvRmtPortFind(pxRmt, unPort) != NULL) {
        LOGE(TAG_RMT, "This RMT has already an N-1 Port bound ");
		return false;
	}

	pxTmp = pxRmtN1PortCreate(unPort, pxN1Ipcp);
	if (!pxTmp)
		return false;

    vRsListInsert(&pxRmt->xPorts, &pxTmp->xPortItem);

	/*
	if (!ps || !ps->rmt_q_create_policy) {
		rcu_read_unlock();
		LOG_ERR("No PS in the RMT, can't bind");
		n1_port_destroy(tmp);
		return -1;
	}

	tmp->rmt_ps_queues = ps->rmt_q_create_policy(ps, tmp);
	rcu_read_unlock();

	if (!tmp->rmt_ps_queues) {
		LOG_ERR("Cannot create structs for scheduling policy");
		n1_port_destroy(tmp);
		return -1;
	}*/

	/*No added to the Hash Table because we've assumed there is only a flow in the shim DIF
	 * Instead, it is aggregate to the PortIdArray with an unique member.*/

	LOGI(TAG_RMT, "Added send queue to RMT instance %pK for port %d", pxRmt, unPort);

	/*Associate the DIF name with the SDUP configuration, SDUP will not be used for the moment*/

	return true;
}

/**
 * Add an Address into the RMT list. This list is useful when the
 * packet arrived and need to know whether it is for us or not.
 */
bool_t xRmtAddressAdd(struct rmt_t *pxInstance, address_t xAddress)
{
	rmtAddress_t *pxRmtAddr;

    RsAssert(pxInstance);

	pxRmtAddr = pvRsMemAlloc(sizeof(*pxRmtAddr));
	if (!pxRmtAddr) {
        LOGE(TAG_RMT, "Failed to allocate memory for RMT address");
		return false;
    }

	pxRmtAddr->xAddress = xAddress;

	LOGI(TAG_RMT, "Adding and Address into the RMT list:%d", pxRmtAddr->xAddress);

    vRsListInitItem(&(pxRmtAddr->xAddressListItem), pxRmtAddr);
    vRsListInsert(&pxInstance->xAddresses, &(pxRmtAddr->xAddressListItem));

	return true;
}

/**
 * Check if the Address defined in the PDU is stored in the
 * address list in the RMT.
 */
bool_t xRmtPduIsAddressedToMe(struct rmt_t *pxRmt, address_t xAddress)
{
	rmtAddress_t *pxAddr;
	RsListItem_t *pxListItem;

    pxListItem = pxRsListGetFirst(&pxRmt->xAddresses);

	while (pxListItem != NULL) {
        pxAddr = (rmtAddress_t *)pxRsListGetItemOwner(pxListItem);

		if (pxAddr->xAddress == xAddress)
			return true;

        pxListItem = pxRsListGetNext(pxListItem);
	}

	return false;
}

static bool_t xRmtProcessMgmtPdu(portId_t unPort, du_t *pxDu)
{
    struct ipcpInstance_t *pxNormalInstance;
    bool_t xStatus;

    RsAssert((pxNormalInstance = pxIpcManagerFindByType(&xIpcManager, eNormal)));

    if (!xNormalMgmtDuPost(pxNormalInstance, unPort, pxDu)) {
        LOGE(TAG_RMT, "Failed posting management PDU to normal IPCP");
        return false;
    }

    return true;
}

static bool_t xRmtProcessDtPdu(struct rmt_t *pxRmt, portId_t xPortId, du_t *pxDu)
{
	address_t xDstAddrTmp;
	cepId_t xCepTmp;
	pduType_t xPduTypeTmp;

    RsAssert(pxRmt);
    RsAssert(is_port_id_ok(xPortId));
    RsAssert(pxDu);

    xDstAddrTmp = PCI_GET(pxDu, xDestination);

	if (!is_address_ok(xDstAddrTmp)) {
		LOGE(TAG_RMT, "PDU has invalid destination address");
		return false;
	}

    xPduTypeTmp = PCI_GET(pxDu, PCI_TYPE);

	if (xPduTypeTmp == PDU_TYPE_MGMT) {
		LOGE(TAG_RMT, "Management PDU took the wrong code path");
		return false;
	}

    xCepTmp = PCI_GET(pxDu, PCI_CONN_DST_ID);

	if (!is_cep_id_ok(xCepTmp))	{
		LOGE(TAG_RMT, "Invalid CEP-id in PDU");
		return false;
	}

	if (!xEfcpContainerReceive(pxRmt->pxEfcpc, xCepTmp, pxDu)) {
		LOGE(TAG_RMT, "EFCP container problems");
		return false;
	}

	return true;
}

bool_t xRmtReceive(struct rmt_t *pxRmt,
                   struct efcpContainer_t *pxEfcp,
                   du_t *pxDu,
                   portId_t unPortId)
{
	pduType_t xPduType;
	address_t xDstAddr;
	qosId_t xQosId;
	rmtN1Port_t *pxN1Port;
	size_t uxBytes;
    du_t *pxDuData;

	LOGI(TAG_RMT, "RMT has received a RINA packet from the port %d", unPortId);

    RsAssert(pxRmt);
    RsAssert(is_port_id_ok(unPortId));

	uxBytes = unNetBufTotalSize(pxDu);
	/*pxDu->pxCfg = pxEfcp->pxConfig;*/

    if (!(pxN1Port = prvRmtPortFind(pxRmt, unPortId))) {
		LOGE(TAG_RMT, "Could not get a N-1 port for the received PDU");
		return false;
	}
	stats_inc(rx, pxN1Port, uxBytes);

	/* SDU Protection to be implemented after testing if it is required.
	if (sdup_unprotect_pdu(n1_port->sdup_port, pxDu)) {
		ESP_LOGE(TAG_RMT,"Failed to unprotect PDU");
		xDuDestroy(pxDu);
		return pdFALSE;
	}*/

	/* This one updates the pci->sdup_header and pdu->skb->data pointers
	if (sdup_get_lifetime_limit(n1_port->sdup_port, pxDu)) {
		ESP_LOGE(TAG_RMT,"Failed to get PDU's TTL");
		xDuDestroy(pxDu);
		return pdFALSE;
	}*/
	/* end SDU Protection */

	if (!xDuDecap(sizeof(pci_t), pxDu)) {
		LOGE(TAG_RMT, "Could not decap PDU");
		return false;
	}

	LOGI(TAG_RMT, "DU Decap sucessfuly");

    xPduType = PCI_GET(pxDu, PCI_TYPE);
    xDstAddr = PCI_GET(pxDu, PCI_ADDR_DST);
    xQosId = PCI_GET(pxDu, PCI_CONN_QOS_ID);

	if (!pdu_type_is_ok(xPduType) ||
		!is_address_ok(xDstAddr) ||
		!is_qos_id_ok(xQosId))
	{
		LOGE(TAG_RMT, "Wrong PDU type (%u), dst address (%u) or qos_id (%u)",
			 xPduType, xDstAddr, xQosId);
		return false;
	}

    /* Move to the next netbuf in the chain, which is the RINA
     * packet data. */
    pxDuData = pxNetBufNext(pxDu);
    vNetBufFree(pxDu);
    pxDu = NULL;

	/* pdu is for me */
	if (xRmtPduIsAddressedToMe(pxRmt, xDstAddr)) {

		/* pdu is for me */
		switch (xPduType) {
		case PDU_TYPE_MGMT:
			return xRmtProcessMgmtPdu(unPortId, pxDuData);

		case PDU_TYPE_CACK:
		case PDU_TYPE_SACK:
		case PDU_TYPE_NACK:
		case PDU_TYPE_FC:
		case PDU_TYPE_ACK:
		case PDU_TYPE_ACK_AND_FC:
		case PDU_TYPE_RENDEZVOUS:
		case PDU_TYPE_DT:
			/*
			 * (FUTURE)
			 *
             * enqueue PDU in pdus_dt[dest-addr, qos-id]
             * don't process it now ...
             */
            if (!xRmtProcessDtPdu(pxRmt, unPortId, pxDuData)) {
                LOGE(TAG_RMT, "PDU processing failed");
                return false;
            }
            else return true;

		default:
			LOGE(TAG_RMT, "Unknown PDU type %d", xPduType);
			return false;
		}
	}
	/* pdu is not for me. Then release buffer and destroy anything.  A
	 * forwarding to next hop will be consider in other version. */
	else {
		if (!xDstAddr)
			return xRmtProcessMgmtPdu(unPortId, pxDuData);
		else
		{
			LOGI(TAG_RMT, "PDU is not for me");
			return false;
		}
	}
	return true;
}

static bool_t xRmtN1PortWriteDu(struct rmt_t *pxRmt,
								rmtN1Port_t *pxN1Port,
								du_t *pxDu)
{
	bool_t xStatus = true;

	LOGI(TAG_RMT, "Sending SDU to port %u", pxN1Port->unPort);

	CALL_IPCP_CHECK(xStatus, pxN1Port->pxN1Ipcp, duWrite, pxN1Port->unPort, pxDu, false)
	{
        LOGE(TAG_RMT, "Failed to write SDU to port %d", pxN1Port->unPort);
        return false;
	} else {
		/* // n1_port_lock(n1_port); */
		/* if (pxN1Port->pxPendingDu) { */
		/* 	LOGE(TAG_RMT, "Already a pending SDU present for port %d", pxN1Port->unPort); */
		/* 	pxN1Port->xStats.plen--; */
		/* } */

		/* pxN1Port->pxPendingDu = pxDu; */
		/* pxN1Port->xStats.plen++; */

		/* if (pxN1Port->eState == eN1_PORT_STATE_DO_NOT_DISABLE) */
		/* 	pxN1Port->eState = eN1_PORT_STATE_ENABLED; */
		/* else */
		/* 	pxN1Port->eState = eN1_PORT_STATE_DISABLED; */

        /* return true; */
        return true;
    }
}

bool_t xRmtSendPortId(struct rmt_t *pxRmt, portId_t unPort, du_t *pxDu)
{
	rmtN1Port_t *pxN1Port;
	int cases;
	bool_t ret;
	bool_t xMustEnqueue;

	/*ps = container_of(rcu_dereference(instance->base.ps),
			  struct rmt_ps, base);

	if (!ps || !ps->rmt_enqueue_policy) {
		rcu_read_unlock();
		LOG_ERR("PS or enqueue policy null, dropping pdu");
		du_destroy(du);
		return -1;
	}*/

    if (!(pxN1Port = prvRmtPortFind(pxRmt, unPort))) {
		LOGE(TAG_RMT, "Could not find the N-1 port: %u", unPort);
		return false;
	}

	xMustEnqueue = false;

	if (pxN1Port->xStats.plen || pxN1Port->uxBusy || pxN1Port->eState == eN1_PORT_STATE_DISABLED)
		xMustEnqueue = true;

	// ret = ps->rmt_enqueue_policy(ps, n1_port, du, must_enqueue);
	cases = 0; // Send Default policy, change later.
	// rcu_read_unlock();
	switch (cases)
	{
#if 0
	case RMT_PS_ENQ_SCHED:
		n1_port->stats.plen++;
		tasklet_hi_schedule(&instance->egress_tasklet);
		ret = 0;
		break;
	case RMT_PS_ENQ_DROP:
		n1_port->stats.drop_pdus++;
		LOG_ERR("PDU dropped while enqueing");
		ret = 0;
		break;
	case RMT_PS_ENQ_ERR:
		n1_port->stats.err_pdus++;
		LOG_ERR("Some error occurred while enqueuing PDU");
		ret = 0;
		break;
#endif
	case RMT_PS_ENQ_SEND:
		if (xMustEnqueue) {
			LOGE(TAG_RMT, "Wrong behaviour of the policy");
			pxN1Port->xStats.errPdus++;
			LOGI(TAG_RMT, "Policy should have enqueue, returned SEND");
			ret = false;
			break;
		}

		pxN1Port->uxBusy = true;

		// n1_port_unlock(n1_port);
		LOGI(TAG_RMT, "PDU ready to be sent, no need to enqueue");
		ret = xRmtN1PortWriteDu(pxRmt, pxN1Port, pxDu);
		/*FIXME LB: This is just horrible, needs to be rethinked */
		// N1_port_lock(n1_port);
		pxN1Port->uxBusy = false;
		if (cases >= 0)	{
			stats_inc(tx, pxN1Port, ret);
			ret = true;
		}
		break;
	default:
		LOGE(TAG_RMT, "rmt_enqueu_policy returned wrong value");
		break;
		ret = true;
	}

	// n1_port_unlock(n1_port);
	// n1pmap_release(instance, n1_port);
	return ret;
}

bool_t xRmtSend(struct rmt_t *pxRmt, du_t *pxDu)
{
    RsAssert(pxRmt);
    RsAssert(pxDu);

    /* In IRATI, this code tries to guess where to send the packet
     * using the NHOP policy table. We do not have this table yet so
     * let's... just try our best, I guess? */

    /* FIXME: I HAVE NO IDEA WHAT TO DO HERE. */
#if 0
    if (xRmtPduIsAddressedToMe

	if (!xRmtSendPortId(pxRmt, /*pxRmtInstance->pxN1Port->xPortId*/xPortIdTable[0].pxPortN1->xPortId, pxDu))
		LOGE(TAG_RMT, "Failed to send a PDU to port %d", xPortIdTable[0].pxPortN1->xPortId);
#endif

	return true;
}
