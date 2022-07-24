#include "rina_ids.h"
#include "du.h"

bool_t xEfcpEnqueue(struct efcp_t * pxEfcp, portId_t xPort, struct du_t * pxDu)
{
    return true;
}

bool_t xEfcpContainerReceive( struct efcpContainer_t * pxContainer, cepId_t xCepId, struct du_t * pxDu)
{
    return true;
}

bool_t xEfcpReceive(struct efcp_t * pxEfcp,  struct du_t *  pxDu)
{
    return true;
}

struct efcpContainer_t * pxEfcpContainerCreate(void)
{
    return NULL;
}

bool_t xEfcpContainerWrite(struct efcpContainer_t *pxEfcpContainer, cepId_t xCepId, struct du_t *pxDu)
{
    return true;
}

bool_t xEfcpConnectionDestroy(struct efcpContainer_t * pxContainer, cepId_t xId)
{
    return true;
}

cepId_t xEfcpConnectionCreate(struct efcpContainer_t * pxContainer,
                              address_t                xSrcAddr,
                              address_t                xDstAddr,
                              portId_t                 xPortId,
                              qosId_t                  xQosId,
                              cepId_t                  xSrcCepId,
                              cepId_t                  xDstCepId,
                              dtpConfig_t *            pxDtpCfg,
                              struct dtcpConfig_t *    pxDtcpCfg)
{
    return true;
}

