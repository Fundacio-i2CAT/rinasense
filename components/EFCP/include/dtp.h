/*
 * dtp.h
 *
 *  Created on: 11 oct. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_EFCP_INCLUDE_DTP_H_
#define COMPONENTS_EFCP_INCLUDE_DTP_H_

#ifdef __cplusplus
extern "C" {
#endif

bool_t xDtpWrite(dtp_t *pxDtpInstance, du_t *pxDu);
bool_t xDtpReceive(dtp_t *pxInstance, du_t *pxDu);
bool_t xDtpDestroy(dtp_t *pxInstance);

dtp_t *pxDtpCreate(struct efcp_t *pxEfcp,
                   struct rmt_t *pxRmt,
                   dtpConfig_t * pxDtpCfg);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_EFCP_INCLUDE_DTP_H_ */
