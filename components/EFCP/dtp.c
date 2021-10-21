/*
 * dtp.c
 *
 *  Created on: 11 oct. 2021
 *      Author: i2CAT
 */

#include "du.h"

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

BaseType_t xDtpReceive( dtp_t * pxInstance, du_t * pxDu)
{
        return pdTRUE;
}