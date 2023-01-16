#ifndef _mock_EFCP_EFCP_H
#define _mock_EFCP_EFCP_H

#include "rina_ids.h"

#ifdef ESP_PLATFORM

#define xEfcpEnqueue           mock_EFCP_xEfcpEnqueue
#define xEfcpContainerReceive  mock_EFCP_xEfcpContainerReceive
#define xEfcpReceive           mock_EFCP_xEfcpReceive
#define xEfcpContainerWrite    mock_EFCP_xEfcpContainerWrite
#define xEfcpConnectionDestroy mock_EFCP_xEfcpConnectionDestroy
#define xEfcpConnectionCreate  mock_EFCP_xEfcpConnectionCreate

bool_t mock_EFCP_xEfcpEnqueue(struct efcp_t * pxEfcp, portId_t xPort, du_t * pxDu);
bool_t mock_EFCP_xEfcpContainerReceive( struct efcpContainer_t * pxContainer, cepId_t xCepId, du_t * pxDu);
bool_t mock_EFCP_xEfcpReceive(struct efcp_t * pxEfcp,  struct du_t *  pxDu);
struct efcpContainer_t * mock_EFCP_pxEfcpContainerCreate(void);
bool_t mock_EFCP_xEfcpContainerWrite(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, du_t *pxDu);

bool_t_t mock_EFCP_xEfcpConnectionDestroy(struct efcpContainer_t * pxContainer, cepId_t xId);

cepId_t mock_EFCP_xEfcpConnectionCreate(struct efcpContainer_t * pxContainer,
                                        address_t               xSrcAddr,
                                        address_t               xDstAddr,
                                        portId_t               xPortId,
                                        qosId_t                xQosId,
                                        cepId_t                xSrcCepId,
                                        cepId_t                xDstCepId,
                                        dtpConfig_t *          pxDtpCfg,
                                        struct dtcpConfig_t *    pxDtcpCfg);
#else

bool_t xEfcpEnqueue(struct efcp_t * pxEfcp, portId_t xPort, du_t * pxDu);
bool_t xEfcpContainerReceive( struct efcpContainer_t * pxContainer, cepId_t xCepId, du_t * pxDu);
bool_t xEfcpReceive(struct efcp_t * pxEfcp,  du_t *  pxDu);
struct efcpContainer_t * pxEfcpContainerCreate(void);
bool_t xEfcpContainerWrite(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, du_t *pxDu);

bool_t_t xEfcpConnectionDestroy(struct efcpContainer_t * pxContainer, cepId_t xId);

cepId_t xEfcpConnectionCreate(struct efcpContainer_t * pxContainer,
                                address_t               xSrcAddr,
                                address_t               xDstAddr,
                                portId_t               xPortId,
                                qosId_t                xQosId,
                                cepId_t                xSrcCepId,
                                cepId_t                xDstCepId,
                                dtpConfig_t *          pxDtpCfg,
                                struct dtcpConfig_t *    pxDtcpCfg);

#endif

#endif // _mock_EFCP_EFCP_H
