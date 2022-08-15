#include "common/rina_ids.h"
#include "efcpStructures.h"
#include "du.h"
#include "efcpStructures.h"

#ifdef ESP_PLATFORM
bool_t mock_EFCP_xEfcpEnqueue(struct efcp_t * pxEfcp, portId_t xPort, struct du_t * pxDu)
#else
bool_t xEfcpEnqueue(struct efcp_t * pxEfcp, portId_t xPort, struct du_t * pxDu)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_EFCP_xEfcpContainerReceive( struct efcpContainer_t * pxContainer, cepId_t xCepId, struct du_t * pxDu)
#else
bool_t xEfcpContainerReceive(struct efcpContainer_t * pxContainer, cepId_t xCepId, struct du_t * pxDu)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_EFCP_xEfcpReceive(struct efcp_t * pxEfcp,  struct du_t *  pxDu)
#else
bool_t xEfcpReceive(struct efcp_t * pxEfcp,  struct du_t *  pxDu)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
struct efcpContainer_t * mock_EFCP_pxEfcpContainerCreate(void)
#else
struct efcpContainer_t * pxEfcpContainerCreate(void)
#endif
{
    return NULL;
}

#ifdef ESP_PLATFORM
bool_t mock_EFCP_xEfcpContainerWrite(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, struct du_t *pxDu)
#else
bool_t xEfcpContainerWrite(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, struct du_t *pxDu)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_EFCP_xEfcpConnectionDestroy(struct efcpContainer_t * pxContainer, cepId_t xId)
#else
bool_t xEfcpConnectionDestroy(struct efcpContainer_t * pxContainer, cepId_t xId)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
cepId_t mock_EFCP_xEfcpConnectionCreate(struct efcpContainer_t * pxContainer,
                                        address_t                xSrcAddr,
                                        address_t                xDstAddr,
                                        portId_t                 xPortId,
                                        qosId_t                  xQosId,
                                        cepId_t                  xSrcCepId,
                                        cepId_t                  xDstCepId,
                                        dtpConfig_t *            pxDtpCfg,
                                        struct dtcpConfig_t *    pxDtcpCfg)
#else
cepId_t xEfcpConnectionCreate(struct efcpContainer_t * pxContainer,
                              address_t                xSrcAddr,
                              address_t                xDstAddr,
                              portId_t                 xPortId,
                              qosId_t                  xQosId,
                              cepId_t                  xSrcCepId,
                              cepId_t                  xDstCepId,
                              dtpConfig_t *            pxDtpCfg,
                              struct dtcpConfig_t *    pxDtcpCfg)
#endif
{
    return true;
}

