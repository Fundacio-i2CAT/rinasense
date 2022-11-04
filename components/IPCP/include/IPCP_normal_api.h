#ifndef NORMALIPCP_H_
#define NORMALIPCP_H_

#include "IpcManager.h"
#include "rmt.h"
#include "IPCP_normal_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "IPCP_normal_defs.h"

/* NEW AND REVIEWED DECLARATIONS START HERE. */

struct ipcpInstance_t *pxNormalCreate(ipcProcessId_t unIpcpId);

bool_t xNormalDuEnqueue(struct ipcpInstance_t *pxIpcp,
                        portId_t xN1PortId,
                        struct du_t *pxDu);

const rname_t *xNormalGetIpcpName(struct ipcpInstance_t *pxSelf);

const rname_t *xNormalGetDifName(struct ipcpInstance_t *pxSelf);


/* OLD DECLARATIONS START HERE. */

bool_t xNormalFlowBinding(struct ipcpInstance_t *pxIpcp,
                          portId_t xPid,
                          struct ipcpInstance_t *pxN1Ipcp);

cepId_t xNormalConnectionCreateRequest(struct ipcpInstance_t *pxIpcp,
                                       struct efcpContainer_t *pxEfcpc,
                                       portId_t xPortId,
                                       address_t xSource,
                                       address_t xDest,
                                       qosId_t xQosId,
                                       dtpConfig_t *pxDtpCfg,
                                       dtcpConfig_t *pxDtcpCfg);

bool_t xNormalTest(struct ipcpInstance_t *pxIpcp,
                   struct ipcpInstance_t *pxN1Ipcp);

bool_t xNormalRegistering(struct ipcpInstance_t *pxShimInstance,
                          rname_t *pxDifName,
                          rname_t *pxName);

bool_t xNormalFlowAllocationRequest(struct ipcpInstance_t *pxIpcpFrom,
                                    struct ipcpInstance_t *pxIpcpTo,
                                    portId_t xShimPortId);

bool_t xNormalFlowPrebind(struct ipcpInstance_t *pxIpcp,
                          flowAllocateHandle_t *pxFlowHandle);

bool_t xNormalMgmtDuWrite(struct ipcpInstance_t *pxIpcp, portId_t xPortId, struct du_t *pxDu);

bool_t xNormalMgmtDuPost(struct ipcpInstance_t *pxIpcp, portId_t xPortId, struct du_t *pxDu);

bool_t xNormalIsFlowAllocated(struct ipcpInstance_t *pxIpcp, portId_t xPortId);

bool_t xNormalUpdateFlowStatus(struct ipcpInstance_t *pxIpcp, portId_t xPortId,
                               eNormalFlowState_t eNewFlowstate);

bool_t xNormalDuWrite(struct ipcpInstance_t *pxIpcp,
                      portId_t xAppPortId,
                      NetworkBufferDescriptor_t *pxNetworkBuffer);

bool_t xNormalUpdateCepIdFlow(struct ipcpInstance_t *pxIpcp,
                              portId_t xPortId, cepId_t xCepId);

bool_t xNormalConnectionUpdate(struct ipcpInstance_t *pxIpcp,
                               portId_t xAppPortId, cepId_t xSrcCepId, cepId_t xDstCepId);

bool_t xNormalConnectionModify(struct ipcpInstance_t *pxIpcp,
                               cepId_t xCepId,
                               address_t xSrc,
                               address_t xDst);

#ifdef __cplusplus
}
#endif

#endif
