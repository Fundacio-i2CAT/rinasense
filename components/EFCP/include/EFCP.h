
/*
 * EFCP
 */

#ifndef EFCP_H__INCLUDED
#define EFCP_H__INCLUDED

#include "rmt.h"
#include "pci.h"
#include "du.h"
#include "efcpStructures.h"
#include "FlowAllocator_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TAG_EFCP "[EFCP]"

bool_t xEfcpContainerInit(struct efcpContainer_t *pxEfcpContainer, rsrcPoolP_t xPciPool);

bool_t xEfcpEnqueue(struct efcp_t * pxEfcp, portId_t xPort, du_t *pxDu);

bool_t xEfcpContainerReceive( struct efcpContainer_t * pxContainer, cepId_t xCepId, du_t *pxDu);

bool_t xEfcpReceive(struct efcp_t * pxEfcp, du_t *pxDu);

bool_t xEfcpContainerWrite(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, du_t *pxDu);

bool_t xEfcpConnectionDestroy(struct efcpContainer_t * pxContainer, cepId_t xId);

bool_t xEfcpConnectionUpdate(struct efcpContainer_t *pxContainer,
                             cepId_t from,
                             cepId_t to);

cepId_t xEfcpConnectionCreate(struct efcpContainer_t *pxEfcpContainer,
                              address_t xSrcAddr,
                              address_t xDstAddr,
                              portId_t xPortId,
                              qosId_t xQosId,
                              cepId_t xSrcCepId,
                              cepId_t xDstCepId,
                              dtpConfig_t *pxDtpCfg,
                              dtcpConfig_t *pxDtcpCfg);

bool_t xEfcpConnectionModify(struct efcpContainer_t *pxContainer,
                             cepId_t xCepId,
                             address_t xSrc,
                             address_t xDst);

#ifdef __cplusplus
}
#endif

#endif /* EFCP_H__*/
