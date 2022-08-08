#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "portability/port.h"
#include "EFCP.h"
#include "rmt.h"
#include "pci.h"
#include "du.h"
#include "efcpStructures.h"
#include "dtp.h"
#include "connection.h"
#include "configSensor.h"
#include "IPCP_instance.h"
#include "num_mgr.h"
#include "rina_common_port.h"
#include "FlowAllocator_api.h"

/* CHeck if a Connection is ok*/
static bool_t is_candidate_connection_ok(const struct connection_t *pxConnection);

/* Create and instance of efcp*/
static struct efcp_t *pxEfcpCreate(void);

/* Destroy an EFCP instance*/
static bool_t xEfcpDestroy(struct efcp_t *pxInstance);

/* Receive DU from a Rmt into the EFCP instance*/
bool_t xEfcpReceive(struct efcp_t *pxEfcp, struct du_t *pxDu);

/* Write DU from Application into a EFCP instance*/
static bool_t xEfcpWrite(struct efcp_t *pxEfcp, struct du_t *pxDu);

/* ----- Container ----- */
/* Create an EFCP container */
struct efcpContainer_t *pxEfcpContainerCreate(void);

/* Destroy an EFCP Container */
bool_t xEfcpContainerDestroy(struct efcpContainer_t *pxContainer);

/* Receive DU from Rmt into the EFCPContainer */
bool_t xEfcpContainerReceive(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, struct du_t *pxDu);

/* Write a du from User App into a Container */
bool_t xEfcpContainerWrite(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, struct du_t *pxDu);

/* ----- EFCP MAP ------*/

/** @brief The IMAP table.
 * Array of imap Rows. The type fcpImapRow_t has been set at efcpStructures.h */
static efcpImapRow_t xEfcpImapTable[EFCP_IMAP_ENTRIES];

bool_t xEfcpImapCreate(void);

bool_t xEfcpImapDestroy(void);

struct efcp_t *pxEfcpImapFind(cepId_t xCepIdKey);

bool_t xEfcpImapAdd(cepId_t xCepId, struct efcp_t *pxEfcp);

bool_t xEfcpImapRemove(cepId_t xCepId);

bool_t xEfcpEnqueue(struct efcp_t *pxEfcp, portId_t xPort, struct du_t *pxDu);

/* --------- CODE ----------*/

static bool_t is_candidate_connection_ok(const struct connection_t *pxConnection)
{
        /* FIXME: Add checks for policy params */

        if (!pxConnection ||
            !is_cep_id_ok(pxConnection->xSourceCepId) ||
            !is_port_id_ok(pxConnection->xPortId))
                return false;

        return true;
}

static struct efcp_t *pxEfcpCreate(void)
{
        struct efcp_t *pxEfcpInstance;

        pxEfcpInstance = pvRsMemAlloc(sizeof(*pxEfcpInstance));

        if (!pxEfcpInstance)
                return NULL;

        pxEfcpInstance->xState = eEfcpAllocated;

        pxEfcpInstance->pxDelim = NULL;

        LOGI(TAG_EFCP, "EFCP Instance %pK initialized successfully", pxEfcpInstance);

        return pxEfcpInstance;
}

static bool_t xEfcpDestroy(struct efcp_t *pxInstance)
{
        if (!pxInstance)
        {
                LOGE(TAG_EFCP, "Bogus instance passed, bailing out");
                return false;
        }

        if (pxInstance->pxUserIpcp)
        {
                pxInstance->pxUserIpcp->pxOps->flowUnbindingIpcp(
                    pxInstance->pxUserIpcp->pxData,
                    pxInstance->pxConnection->xPortId);
        }

        if (pxInstance->pxDtp)
        {
                /*
                 * FIXME:
                 *   Shouldn't we check for flows running, before
                 *   unbinding dtp, dtcp, cwq and rtxw ???
                 */
                xDtpDestroy(pxInstance->pxDtp);
        }
        else
                LOGE(TAG_EFCP, "No DT instance present");

        if (pxInstance->pxConnection)
        {
                /* FIXME: Connection should release the cep id */
                if (is_cep_id_ok(pxInstance->pxConnection->xSourceCepId))
                {
                        RsAssert(pxInstance->pxEfcpContainer);
                        // configASSERT(pxInstance->pxContainer->cidm);

                        /*cidm_release(pxInstance->pxEfcpContainer->cidm,
                                     pxInstance->pxConnection->xSourceCepId);*/
                }
                /* FIXME: Should we release (actually the connection) release
                 * the destination cep id? */

                xConnectionDestroy(pxInstance->pxConnection);
        }

        /*if (pxInstance->delim) {
                delim_destroy(pxInstance->delim);
        }*/

        /*robject_del(&instance->robj);*/
        vRsMemFree(pxInstance);

        LOGI(TAG_EFCP, "EFCP instance %pK finalized successfully", pxInstance);

        return true;
}

bool_t xEfcpReceive(struct efcp_t *pxEfcp, struct du_t *pxDu)
{
        pduType_t xPduType;

        xPduType = pxDu->pxPci->xType;

        /* Verify the type of PDU*/
        if (pdu_type_is_control(xPduType))
        {
                if (!pxEfcp->pxDtp || !pxEfcp->pxDtp->pxDtcp)
                {
                        LOGE(TAG_EFCP, "No DTCP instance available");
                        xDuDestroy(pxDu);
                        return false;
                }

                /* if (dtcp_common_rcv_control(pxEfcp->pxDtp.>pxDtcp, pxDu))
                        return pdFALSE;*/

                return true;
        }

        if (!xDtpReceive(pxEfcp->pxDtp, pxDu))
        {
                LOGE(TAG_EFCP, "DTP cannot receive this PDU");
                return false;
        }

        return true;
}

static bool_t xEfcpWrite(struct efcp_t *pxEfcp, struct du_t *pxDu)
{
        // struct delim_ps *delim_ps = NULL;
        // struct du_list_item *next_du = NULL;

        /* Handle fragmentation here */

#if 0
	if (efcp->delim) {
		delim_ps = container_of(rcu_dereference(efcp->delim->base.ps),
						        struct delim_ps,
						        base);

		if (delim_ps->delim_fragment(delim_ps, du,
					     efcp->delim->tx_dus)) {
			LOGE(TAG_EFCP,"Error performing SDU fragmentation");
			du_list_clear(efcp->delim->tx_dus, true);
			return -1;
		}

	        list_for_each_entry(next_du, &(efcp->delim->tx_dus->dus),
	        		    next) {
	                if (dtp_write(efcp->dtp, next_du->du)) {
	                	LOGE(TAG_EFCP,"Could not write SDU fragment to DTP");
	                	/* TODO, what to do here? */
	                }
	        }

	        du_list_clear(efcp->delim->tx_dus, false);

		return 0;
	}
#endif

        /* No fragmentation */
        if (!xDtpWrite(pxEfcp->pxDtp, pxDu))
        {
                LOGE(TAG_EFCP, "Could not write SDU to DTP");
                return false;
        }

        return true;
}

struct efcpContainer_t *pxEfcpContainerCreate(void)
{
        struct efcpContainer_t *pxEfcpContainer;

        pxEfcpContainer = pvRsMemAlloc(sizeof(*pxEfcpContainer));

        if (!pxEfcpContainer)
        {
                LOGE(TAG_EFCP, "Failed to allocate memory for EFCP container object");
                return NULL;
        }

        pxEfcpContainer->pxCidm = pxNumMgrCreate(MAX_CEP_ID);

        if (!xEfcpImapCreate() || pxEfcpContainer->pxCidm == NULL)
        {
                LOGE(TAG_EFCP, "Failed to init EFCP container instances");
                xEfcpContainerDestroy(pxEfcpContainer);
                return NULL;
        }

        LOGI(TAG_EFCP, "EFCP container instance %p created", pxEfcpContainer);

        return pxEfcpContainer;
}

bool_t xEfcpContainerDestroy(struct efcpContainer_t *pxEfcpContainer)
{
        if (!pxEfcpContainer)
        {
                LOGE(TAG_EFCP, "Bogus container passed, bailing out");
                return false;
        }

        if (pxEfcpContainer->pxEfcpImap)
                xEfcpImapDestroy();

        if (pxEfcpContainer->pxCidm)
                vNumMgrDestroy(pxEfcpContainer->pxCidm);

        // if (pxEfcpContainer->pxConfig)     efcp_config_free(container->config);

        // if (container->rset)       rset_unregister(container->rset);
        vRsMemFree(pxEfcpContainer);

        return true;
}

bool_t xEfcpContainerReceive(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, struct du_t *pxDu)
{

        struct efcp_t *pxEfcp;
        bool_t ret = true;
        pduType_t xPduType;

        if (!is_cep_id_ok(xCepId))
        {
                LOGE(TAG_EFCP, "Bad cep-id, cannot write into container");
                xDuDestroy(pxDu);
                return false;
        }

        pxEfcp = pxEfcpImapFind(xCepId);
        if (!pxEfcp)
        {
                // spin_unlock_bh(&container->lock);
                LOGE(TAG_EFCP, "Cannot find the requested instance cep-id: %d",
                     xCepId);
                xDuDestroy(pxDu);
                return false;
        }
        if (pxEfcp->xState == eEfcpDeallocated)
        {
                // spin_unlock_bh(&container->lock);
                xDuDestroy(pxDu);
                LOGI(TAG_EFCP, "EFCP already deallocated");
                return true;
        }

        // atomic_inc(&efcp->pending_ops);

        xPduType = pxDu->pxPci->xType; // Check this
        if (xPduType == PDU_TYPE_DT &&
            pxEfcp->pxConnection->xDestinationCepId < 0)
        {
                /* Check that the destination cep-id is set to avoid races,
                 * otherwise set it*/
                pxEfcp->pxConnection->xDestinationCepId = pxDu->pxPci->connectionId_t.xDestination;
        }

        // spin_unlock_bh(&container->lock);

        ret = xEfcpReceive(pxEfcp, pxDu);
#if 0
        //spin_lock_bh(&container->lock);
        if (atomic_dec_and_test(&efcp->pending_ops) &&
        		efcp->state == EFCP_DEALLOCATED) {
                spin_unlock_bh(&container->lock);
		wake_up_interruptible(&container->del_wq);
                return ret;
        }
        spin_unlock_bh(&container->lock);

        return ret;
#endif
        return ret;
}

bool_t xEfcpContainerWrite(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, struct du_t *pxDu)
{
        struct efcp_t *pxEfcp;
        bool_t ret;

        if (!is_cep_id_ok(xCepId))
        {
                LOGE(TAG_EFCP, "Bad cep-id, cannot write into container");
                xDuDestroy(pxDu);
                return false;
        }

        pxDu->pxCfg = pxEfcpContainer->pxConfig;

        pxEfcp = pxEfcpImapFind(xCepId);
        if (!pxEfcp)
        {

                LOGE(TAG_EFCP, "There is no EFCP bound to this cep-id %d", xCepId);
                xDuDestroy(pxDu);
                return false;
        }
        if (pxEfcp->xState == eEfcpDeallocated)
        {

                xDuDestroy(pxDu);
                LOGE(TAG_EFCP, "EFCP already deallocated");
                return false;
        }

        ret = xEfcpWrite(pxEfcp, pxDu);

        return ret;
}

bool_t xEfcpEnqueue(struct efcp_t *pxEfcp, portId_t xPort, struct du_t *pxDu)
{
        // struct delim_ps * delim_ps = NULL;
        // struct du_list_item * next_du = NULL;

        if (!pxEfcp)
        {
                LOGE(TAG_EFCP, "Flow is being deallocated, dropping SDU");
                xDuDestroy(pxDu);
                return false;
        }

        /* Reassembly goes here */
#if 0
        if (pxEfcp->pxDelim) {
		delim_ps = container_of(rcu_dereference(efcp->delim->base.ps),
						        struct delim_ps,
						        base);

		if (delim_ps->delim_process_udf(delim_ps, pxDu,
						pxEfcp->pxDelim->rx_dus)) {
			LOGE( TAG_EFCP,"Error processing EFCP UDF by delimiting");
			du_list_clear(efcp->delim->rx_dus, true);
			return false;
		}

	        list_for_each_entry(next_du, &(efcp->delim->rx_dus->dus),
	        		    next) {
	                if (efcp->user_ipcp->ops->du_enqueue(efcp->user_ipcp->data,
	                                                     port,
	                                                     next_du->du)) {
	                        LOGE( TAG_EFCP,"Upper ipcp could not enqueue sdu to port: %d", port);
	                        return false;
	                }
	        }

	        du_list_clear(efcp->delim->rx_dus, false);

		return true;
        }
#endif
        if (!xFlowAllocatorDuPost(xPort, pxDu))
        {
                LOGE(TAG_EFCP, "Upper ipcp could not enqueue sdu to port: %d", xPort);
                return false;
        }

        return true;
}

bool_t xEfcpConnectionModify(struct efcpContainer_t *pxContainer,
                             cepId_t xCepId,
                             address_t xSrc,
                             address_t xDst)
{
        struct efcp_t *pxEfcp;

        if (!pxContainer)
        {
                LOGE(TAG_EFCP, "Bogus container passed, bailing out");
                return false;
        }

        pxEfcp = pxEfcpImapFind(xCepId);
        if (!pxEfcp)
        {

                LOGE(TAG_EFCP, "Cannot get instance %d from container %pK",
                     xCepId, pxContainer);
                return false;
        }
        if (pxEfcp->xState == eEfcpDeallocated)
        {

                if (xEfcpDestroy(pxEfcp))
                {
                        LOGE(TAG_EFCP, "Cannot destroy instance %d, instance lost",
                             xCepId);
                        return false;
                }
                return true;
        }
        pxEfcp->pxConnection->xSourceAddress = xSrc;
        pxEfcp->pxConnection->xDestinationAddress = xDst;

        return true;
}

bool_t xEfcpConnectionUpdate(struct efcpContainer_t *pxContainer,
                             cepId_t from,
                             cepId_t to)
{
        struct efcp_t *pxEfcp;

        if (!pxContainer)
        {
                LOGE(TAG_EFCP, "Bogus container passed, bailing out");
                return false;
        }
        if (!is_cep_id_ok(from))
        {
                LOGE(TAG_EFCP, "Bad from cep-id, cannot update connection");
                return false;
        }
        if (!is_cep_id_ok(to))
        {
                LOGE(TAG_EFCP, "Bad to cep-id, cannot update connection");
                return false;
        }

        pxEfcp = pxEfcpImapFind(from);

        if (!pxEfcp)
        {
                LOGE(TAG_EFCP, "Cannot get instance %d from container %pK",
                     from, pxContainer);
                return false;
        }
        if (pxEfcp->xState == eEfcpDeallocated)
        {

                if (xEfcpDestroy(pxEfcp))
                {
                        LOGE(TAG_EFCP, "Cannot destroy instance %d, instance lost", from);
                        return false;
                }
                return true;
        }
        pxEfcp->pxConnection->xDestinationCepId = to;

        LOGI(TAG_EFCP, "Connection updated");
        LOGI(TAG_EFCP, "  Source address:     %d",
             pxEfcp->pxConnection->xSourceAddress);
        LOGI(TAG_EFCP, "  Destination address %d",
             pxEfcp->pxConnection->xDestinationAddress);
        LOGI(TAG_EFCP, "  Destination cep id: %d",
             pxEfcp->pxConnection->xDestinationCepId);
        LOGI(TAG_EFCP, "  Source cep id:      %d",
             pxEfcp->pxConnection->xSourceCepId);

        return true;
}

bool_t xEfcpConnectionDestroy(struct efcpContainer_t *pxContainer,
                              cepId_t xId)
{
        struct efcp_t *pxEfcp;
        // BaseType_t retval;

        LOGI(TAG_EFCP, "EFCP connection destroy called");

        /* FIXME: should wait 3*delta-t before destroying the connection */

        if (!pxContainer)
        {
                LOGE(TAG_EFCP, "Bogus container passed, bailing out");
                return false;
        }
        if (!is_cep_id_ok(xId))
        {
                LOGE(TAG_EFCP, "Bad cep-id, cannot destroy connection");
                return false;
        }

        pxEfcp = pxEfcpImapFind(xId);
        if (!pxEfcp)
        {
                LOGE(TAG_EFCP, "Cannot find instance %d in container %pK",
                     xId, pxContainer);
                return false;
        }

        if (xEfcpImapRemove(xId))
        {
                // spin_unlock_bh(&container->lock);
                LOGE(TAG_EFCP, "Cannot remove instance %d from container %pK",
                     xId, pxContainer);
                return false;
        }
        pxEfcp->xState = eEfcpDeallocated;

#if 0
	if (atomic_read(&efcp->pending_ops) != 0) {
		spin_unlock_bh(&container->lock);
		retval = wait_event_interruptible(container->del_wq,
						  atomic_read(&efcp->pending_ops) == 0 &&
						  efcp->state == EFCP_DEALLOCATED);
		if (retval != 0)
			LOG_ERR("EFCP destroy even interrupted (%d)", retval);
               	if (efcp_destroy(efcp)) {
               	        LOG_ERR("Cannot destroy instance %d, instance lost", id);
               	        return -1;
               	}
		return 0;
	}
        spin_unlock_bh(&container->lock);
#endif
        if (xEfcpDestroy(pxEfcp))
        {
                LOGE(TAG_EFCP, "Cannot destroy instance %d, instance lost", xId);
                return false;
        }
        return true;
}

cepId_t xEfcpConnectionCreate(struct efcpContainer_t *pxEfcpContainer,
                              address_t xSrcAddr,
                              address_t xDstAddr,
                              portId_t xPortId,
                              qosId_t xQosId,
                              cepId_t xSrcCepId,
                              cepId_t xDstCepId,
                              dtpConfig_t *pxDtpCfg,
                              struct dtcpConfig_t *pxDtcpCfg)
{
        LOGE(TAG_EFCP, "xEfcpConnectionCreate");

        struct efcp_t *pxEfcp;
        struct connection_t *pxConnection = NULL;
        cepId_t xCepId;
        struct dtcp_t *pxDtcp;
        // struct cwq *        cwq;
        // struct rtxq *       rtxq;
        // uint_t              mfps, mfss;
        // timeout_t           mpl, a, r = 0, tr = 0;
        // struct dtp_ps       * dtp_ps;
        // bool                dtcp_present;
        // struct rttq *       rttq;
        // struct delim * delim;

        if (!pxEfcpContainer)
        {
                LOGE(TAG_EFCP, "Bogus container passed, bailing out");
                return cep_id_bad();
        }

        LOGE(TAG_EFCP, "xEfcpConnectionCreate: ConnectionCreate");
#ifdef __FREERTOS__
        size_t Test = xPortGetFreeHeapSize();
        LOGE(TAG_EFCP, "Memory size:%d", (int)Test);
#endif
        pxConnection = pxConnectionCreate();
        RsAssert(pxConnection);

        if (!pxConnection)
                return cep_id_bad();

        RsAssert(pxDtpCfg);

        pxConnection->xDestinationAddress = xDstAddr;
        pxConnection->xPortId = xPortId;
        pxConnection->xQosId = xQosId;
        pxConnection->xSourceCepId = xSrcCepId;
        pxConnection->xDestinationCepId = xDstCepId;
        pxConnection->xSourceAddress = LOCAL_ADDRESS;

        LOGE(TAG_EFCP, "xEfcpConnectionCreate: EfcpCreate");
        pxEfcp = pxEfcpCreate();

        if (!pxEfcp)
        {
                xConnectionDestroy(pxConnection);
                return cep_id_bad();
        }

        // xCepId = cidm_allocate(container->cidm);
        //  hardcode to test
        xCepId = 1;
        if (!is_cep_id_ok(xCepId))
        {
                LOGE(TAG_EFCP, "CIDM generated wrong CEP ID");
                xEfcpDestroy(pxEfcp);
                return cep_id_bad();
        }

        /* We must ensure that the DTP is instantiated, at least ... */

        pxEfcp->pxEfcpContainer = pxEfcpContainer;
        pxConnection->xSourceCepId = xCepId;

        if (!is_candidate_connection_ok((const struct connection_t *)pxConnection))
        {
                LOGE(TAG_EFCP, "Bogus connection passed, bailing out");
                xEfcpDestroy(pxEfcp);
                return cep_id_bad();
        }

        pxEfcp->pxConnection = pxConnection;

#if 0

        if (pxEfcpContainer->config->dt_cons->dif_frag) {
        	delim = delim_create(efcp, &efcp->robj);
        	if (!delim){
        		LOGE(TAG_EFCP,"Problems creating delimiting module");
                        xEfcpDestroy(pxEfcp);
                        return cep_id_bad();
        	}

        	delim->max_fragment_size =
        			container->config->dt_cons->max_pdu_size -
        			pci_calculate_size(container->config, PDU_TYPE_DT)
        			- 1;

        	/* TODO, allow selection of delimiting policy set name */
                if (delim_select_policy_set(delim, "", RINA_PS_DEFAULT_NAME)) {
                        LOGE(TAG_EFCP,"Could not load delimiting PS %s",
                        	RINA_PS_DEFAULT_NAME);
                        delim_destroy(delim);
                        efcp_destroy(efcp);
                        return cep_id_bad();
                }

        	pxEfcp->pxDelim = delim;
        }

#endif
        /* FIXME: dtp_create() takes ownership of the connection parameter */
        pxEfcp->pxDtp = pxDtpCreate(pxEfcp, pxEfcpContainer->pxRmt, pxDtpCfg);
#ifdef _FREERTOS_
        heap_caps_check_integrity(MALLOC_CAP_DEFAULT, pdTRUE);
#endif
        if (!pxEfcp->pxDtp)
        {
                xEfcpDestroy(pxEfcp);
                return cep_id_bad();
        }

        pxDtcp = NULL;
#if 0
        rcu_read_lock();
        dtp_ps = dtp_ps_get(efcp->dtp);
        a = dtp_ps->initial_a_timer;
        dtcp_present = dtp_ps->dtcp_present;
        rcu_read_unlock();
        if (dtcp_present) {
                dtcp = dtcp_create(efcp->dtp,
                                   container->Rmt,
                                   dtcp_cfg,
				   &efcp->robj);
                if (!dtcp) {
                        efcp_destroy(efcp);
                        return cep_id_bad();
                }

                efcp->dtp->dtcp = dtcp;
        }

        if (dtcp_window_based_fctrl(dtcp_cfg) ||
            dtcp_rate_based_fctrl(dtcp_cfg)) {
                cwq = cwq_create();
                if (!cwq) {
                        LOGE(TAG_EFCP,"Failed to create closed window queue");
                        efcp_destroy(efcp);
                        return cep_id_bad();
                }
                efcp->dtp->cwq = cwq;
        }

        if (dtcp_rtx_ctrl(dtcp_cfg)) {
                rtxq = rtxq_create(efcp->dtp, container->Rmt, container,
                		   dtcp_cfg, cep_id);
                if (!rtxq) {
                        LOGE(TAG_EFCP,"Failed to create rexmsn queue");
                        efcp_destroy(efcp);
                        return cep_id_bad();
                }
                efcp->dtp->rtxq = rtxq;
                efcp->dtp->rttq = NULL;
        } else {
        	rttq = rttq_create();
        	if (!rttq) {
        		LOGE(TAG_EFCP,"Failed to create RTT queue");
        		efcp_destroy(efcp);
        		return cep_id_bad();
        	}
        	efcp->dtp->rttq = rttq;
        	efcp->dtp->rtxq = NULL;
        }

#endif

        pxEfcp->pxDtp->pxEfcp = pxEfcp;
        /* FIXME: This is crap and have to be rethinked */

        /* FIXME: max pdu and sdu sizes are not stored anywhere. Maybe add them
         * in connection... For the time being are equal to max_pdu_size in
         * dif configuration
         */
#if 0
        mfps = container->config->dt_cons->max_pdu_size;
        mfss = container->config->dt_cons->max_pdu_size;
        mpl  = container->config->dt_cons->max_pdu_life;
        if (dtcp && dtcp_rtx_ctrl(dtcp_cfg)) {
                tr = dtcp_initial_tr(dtcp_cfg);
                /* tr = msecs_to_jiffies(tr); */
                /* FIXME: r should be passed and must be a bound */
                r  = dtcp_data_retransmit_max(dtcp_cfg) * tr;
        }

        LOGI(TAG_EFCP,"DT SV initialized with:");
        LOGI(TAG_EFCP,"  MFPS: %d, MFSS: %d",   mfps, mfss);
        LOGI(TAG_EFCP,"  A: %d, R: %d, TR: %d", a, r, tr);

        if (dtp_sv_init(efcp->dtp, dtcp_rtx_ctrl(dtcp_cfg),
                        dtcp_window_based_fctrl(dtcp_cfg),
                        dtcp_rate_based_fctrl(dtcp_cfg),
			mfps, mfss, mpl, a, r, tr)) {
                LOGE(TAG_EFCP,"Could not init dtp_sv");
                efcp_destroy(efcp);
                return cep_id_bad();
        }

#endif

        if (!xEfcpImapAdd(xCepId, pxEfcp))
        {
                LOGE(TAG_EFCP, "Cannot add a new instance into container %p",
                     pxEfcpContainer);
                xEfcpDestroy(pxEfcp);
                return cep_id_bad();
        }

        LOGI(TAG_EFCP, "Connection created ("
                       "Source address %d,"
                       "Destination address %d, "
                       "Destination cep-id %d, "
                       "Source cep-id %d)",
             pxConnection->xSourceAddress,
             pxConnection->xDestinationAddress,
             pxConnection->xDestinationCepId,
             pxConnection->xSourceCepId);

        return xCepId;
}

bool_t xEfcpImapCreate(void)
{
        efcpImapRow_t xNullImapRow;

        xNullImapRow.xCepIdKey = 0;
        xNullImapRow.xEfcpValue = NULL;
        xNullImapRow.ucValid = 0;

        int i;
        for (i = 0; i < EFCP_IMAP_ENTRIES; i++)
        {
                xEfcpImapTable[i] = xNullImapRow;
        }

        return true;
}

bool_t xEfcpImapDestroy(void)
{
        (void)memset(xEfcpImapTable, 0, sizeof(xEfcpImapTable));

        return 0;
}

struct efcp_t *pxEfcpImapFind(cepId_t xCepIdKey)
{
        struct efcp_t *xEfcpFounded;
        int x = 0;
        bool_t check = false;

        xEfcpFounded = pvRsMemAlloc(sizeof(struct efcp_t *));

        for (x = 0; x < EFCP_IMAP_ENTRIES; x++) // lookup in the MAP for the EFCP Instance based on cepIdKey
        {

                if (xEfcpImapTable[x].xCepIdKey == xCepIdKey)
                {
                        xEfcpFounded = xEfcpImapTable[x].xEfcpValue;
                        xEfcpImapTable[x].ucValid = 1;
                        LOGD(TAG_EFCP, "EFCP Instance founded");
                        check = true;

                        break;
                }
        }
        if (check == true)
        {
                return xEfcpFounded;
        }
        return NULL;
}

bool_t xEfcpImapAdd(cepId_t xCepId, struct efcp_t *pxEfcp)
{
        efcpImapRow_t xImapEntry;
        int x = 0;

        xImapEntry.ucValid = 1;
        xImapEntry.xCepIdKey = xCepId;
        xImapEntry.xEfcpValue = pxEfcp;

        for (x = 0; x < EFCP_IMAP_ENTRIES; x++)
        {
                xEfcpImapTable[x].xCepIdKey = xImapEntry.xCepIdKey;
                xEfcpImapTable[x].xEfcpValue = xImapEntry.xEfcpValue;
                xEfcpImapTable[x].ucValid = xImapEntry.ucValid;
                LOGI(TAG_EFCP, "EFCP Entry successful");
                return true;
        }

        return false;
}

bool_t xEfcpImapRemove(cepId_t xCepId)
{
        int x;
        for (x = 0; x < EFCP_IMAP_ENTRIES; x++)
        {
                if (xEfcpImapTable[x].xCepIdKey == xCepId)
                {
                        xEfcpImapTable[x].xCepIdKey = 0;
                        xEfcpImapTable[x].xEfcpValue = NULL;
                        xEfcpImapTable[x].ucValid = 0;

                        break;
                }
        }

        return true;
}
