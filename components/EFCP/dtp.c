/*
 * dtp.c
 *
 *  Created on: 11 oct. 2021
 *      Author: i2CAT
 */

#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "du.h"
#include "common.h"

#define TAG_DTP         "DTP"

/*
static dtp_sv default_sv = {
        .seq_nr_to_send                = 0,
        .max_seq_nr_sent               = 0,
        .seq_number_rollover_threshold = 0,
        .max_seq_nr_rcv                = 0,
	.stats = {
		.drop_pdus = 0,
		.err_pdus = 0,
		.tx_pdus = 0,
		.tx_bytes = 0,
		.rx_pdus = 0,
		.rx_bytes = 0 },
        .rexmsn_ctrl                   = false,
        .rate_based                    = false,
        .window_based                  = false,
        .drf_required                  = true,
        .rate_fulfiled                 = false,
        .max_flow_pdu_size    = UINT_MAX,
        .max_flow_sdu_size    = UINT_MAX,
        .MPL                  = 1000,
        .R                    = 100,
        .A                    = 0,
        .tr                   = 0,
        .rcv_left_window_edge = 0,
        .window_closed        = false,
        .drf_flag             = true,
};*/

BaseType_t xDtpReceive( dtp_t * pxInstance, struct du_t * pxDu)
{
        //struct dtp_ps *  ps;
        dtcp_t *                pxDtcp;
        struct dtcp_ps *        dtcp_ps;
        seqNum_t                xSeqNum;
       // timeout_t        a, r, mpl;
        seqNum_t                xLWE;
        BaseType_t             xInOrder;
        BaseType_t             xRtxCtrl = false;
        seqNum_t                xMaxSduGap;
	int                     sbytes;
	efcp_t *	         pxEfcp = 0;

        ESP_LOGI(TAG_DTP, "DTP receive started...");

        pxDtcp = pxInstance->pxDtcp;
        pxEfcp = pxInstance->pxEfcp;

        //spin_lock_bh(&instance->sv_lock);

        //a           = instance->sv->A;
        //r 	    = instance->sv->R;
        //mpl	    = instance->sv->MPL;
        xLWE         = pxInstance->pxDtpStateVector->xRcvLeftWindowEdge;

       
        xInOrder    = pdTRUE; //HardCode for completing the phase one
        xMaxSduGap = pxInstance->pxDtpCfg->xMaxSduGap;
       /* if (pxDtcp) {
                dtcp_ps = dtcp_ps_get(dtcp);
                rtx_ctrl = dtcp_ps->rtx_ctrl;
        }*/
        

        xSeqNum = pxDu->pxPci->xSequenceNumber;
	sbytes = xDuDataLen(pxDu);

        //LOG_DBG("local_soft_irq_pending: %d", local_softirq_pending());
        /*LOG_DBG("DTP Received PDU %u (CPU: %d)",
                seq_num, smp_processor_id());*/

        if (pxInstance->pxDtpStateVector->xDrfRequired) {
                /* Start ReceiverInactivityTimer */
               /* if (rtimer_restart(&instance->timers.receiver_inactivity,
                                   2 * (mpl + r + a))) {
                        ESP_LOGE(TAG_DTP,"Failed to start Receiver Inactivity timer");
                        
                        xDuDestroy(pxDu);
                        return pdFALSE;
                }*/

                if (pxDu->pxPci->xFlags & PDU_FLAGS_DATA_RUN)) {
                	ESP_LOGI( TAG_DTP, "Data Run Flag");

                	pxInstance->pxDtpStateVector->xDrfRequired = pdFALSE;
                        pxInstance->pxDtpStateVector->xRcvLeftWindowEdge = xSeqNum;
                        //dtp_squeue_flush(pxInstance);
                        /*if (instance->rttq) {
                        	rttq_flush(pxInstance->pxRttq);
                        }*/

                        if (pxDtcp) {
                                if (dtcp_sv_update(pxDtcp, &pxDu->xPci)) {
                                        ESP_LOGE(TAG_DTP, "Failed to update dtcp sv");
                                        return pdFALSE;
                                }
                        }

                       // dtp_send_pending_ctrl_pdus(instance);
                        //pdu_post(instance, du);
			//stats_inc_bytes(rx, instance->sv, sbytes);

                        return pdTRUE;
                }

                ESP_LOGE(TAG_DTP, "Expecting DRF but not present, dropping PDU %d...",
                        xSeqNum);

		//stats_inc(drop, instance->sv);
		//spin_unlock_bh(&instance->sv_lock);

                xDuDestroy(pxDu);
                return pdTRUE;
        }

        /*
         * NOTE:
         *   no need to check presence of in_order or dtcp because in case
         *   they are not, LWE is not updated and always 0
         */
        if ((xSeqNum <= LWE) ||
        		(is_fc_overrun(instance, dtcp, seq_num, sbytes))) {
        	/* Duplicate PDU or flow control overrun */
        	ESP_LOGE(TAG_DTP,"Duplicate PDU or flow control overrun.SN: %u, LWE:%u",
        		 xSeqNum, LWE);
                //stats_inc(drop, instance->sv);

                //spin_unlock_bh(&instance->sv_lock);

                xDuDestroy(pxDu);

                if (pxDtcp) {
                	/* Send an ACK/Flow Control PDU with current window values */
                	/* FIXME: we have to send a Control ACK PDU, not an
                	 * ack flow control one
                	 */
                        /*if (dtcp_ack_flow_control_pdu_send(dtcp, LWE)) {
                                LOG_ERR("Failed to send ack/flow control pdu");
                                return -1;
                        }*/
                }
                return 0;
        }

        /* Start ReceiverInactivityTimer */
       /* if (rtimer_restart(&instance->timers.receiver_inactivity,
                           2 * (mpl + r + a ))) {
        	spin_unlock_bh(&instance->sv_lock);
                LOG_ERR("Failed to start Receiver Inactivity timer");
                du_destroy(du);
                return -1;
        }*/

        /* This is an acceptable data PDU, stop reliable ACK timer */
        if (dtcp->sv->rendezvous_rcvr) {
        	LOG_DBG("RV at receiver put to false");
        	dtcp->sv->rendezvous_rcvr = false;
        	rtimer_stop(&dtcp->rendezvous_rcv);
        }
        if (!a) {
                bool set_lft_win_edge;

                if (!in_order && !dtcp) {
                	spin_unlock_bh(&instance->sv_lock);
                        LOG_DBG("DTP Receive deliver, seq_num: %d, LWE: %d",
                                seq_num, LWE);
                        if (pdu_post(instance, du))
                                return -1;

                        return 0;
                }

                set_lft_win_edge = !(dtcp_rtx_ctrl(dtcp->cfg) &&
                                     ((seq_num -LWE) > (max_sdu_gap + 1)));
                if (set_lft_win_edge) {
                	instance->sv->rcv_left_window_edge = seq_num;
                }

                spin_unlock_bh(&instance->sv_lock);

                if (dtcp) {
                        if (dtcp_sv_update(dtcp, &du->pci)) {
                                LOG_ERR("Failed to update dtcp sv");
                                goto fail;
                        }
                        dtp_send_pending_ctrl_pdus(instance);
                        if (!set_lft_win_edge) {
                                du_destroy(du);
                                return 0;
                        }
                }

                if (pdu_post(instance, du))
                        return -1;

                spin_lock_bh(&instance->sv_lock);
		stats_inc_bytes(rx, instance->sv, sbytes);
		spin_unlock_bh(&instance->sv_lock);
                return 0;

        fail:
                du_destroy(du);
                return -1;
        }

        LWE = instance->sv->rcv_left_window_edge;
        LOG_DBG("DTP receive LWE: %u", LWE);
        if (seq_num == LWE + 1) {
        	instance->sv->rcv_left_window_edge = seq_num;
                ringq_push(instance->to_post, du);
                LWE = seq_num;
        } else {
                seq_queue_push_ni(instance->seqq->queue, du);
        }

        while (are_there_pdus(instance->seqq->queue, LWE)) {
                du = seq_queue_pop(instance->seqq->queue);
                if (!du)
                        break;
                seq_num = pci_sequence_number_get(&du->pci);
                LWE     = seq_num;
                instance->sv->rcv_left_window_edge = seq_num;
                ringq_push(instance->to_post, du);
        }

        spin_unlock_bh(&instance->sv_lock);

        if (dtcp) {
                if (dtcp_sv_update(dtcp, &du->pci)) {
                        LOG_ERR("Failed to update dtcp sv");
                }
        }

        dtp_send_pending_ctrl_pdus(instance);

        if (list_empty(&instance->seqq->queue->head))
                rtimer_stop(&instance->timers.a);
        else
                rtimer_start(&instance->timers.a, a/AF);

        while (!ringq_is_empty(instance->to_post)) {
                du = (struct du_t *) ringq_pop(instance->to_post);
                if (du) {
                	sbytes = du_data_len(du);
                        pdu_post(instance, du);
                        spin_lock_bh(&instance->sv_lock);
			stats_inc_bytes(rx, instance->sv, sbytes);
			spin_unlock_bh(&instance->sv_lock);
		}
        }

        ESP_LOGI(TAG_DTP, "DTP receive ended...");

        return pdTRUE;
}