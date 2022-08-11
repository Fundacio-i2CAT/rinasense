#ifndef NORMALIPCP_H_
#define NORMALIPCP_H_

#include "IpcManager.h"
#include "rmt.h"
#include "IPCP_normal_defs.h"

bool_t xNormalFlowBinding(struct ipcpInstanceData_t *pxUserData,
                          portId_t xPid,
                          struct ipcpInstance_t *pxN1Ipcp);

cepId_t xNormalConnectionCreateRequest(struct efcpContainer_t *pxEfcpc,
                                       portId_t xPortId,
                                       address_t xSource,
                                       address_t xDest,
                                       qosId_t xQosId,
                                       dtpConfig_t *pxDtpCfg,
                                       struct dtcpConfig_t *pxDtcpCfg);

bool_t xNormalTest(struct ipcpInstance_t *pxNormalInstance,
                   struct ipcpInstance_t *pxN1Ipcp);

bool_t xNormalRegistering(struct ipcpInstance_t *pxShimInstance,
                          name_t *pxDifName,
                          name_t *pxName);

bool_t xNormalFlowAllocationRequest(struct ipcpInstance_t *pxInstanceFrom,
                                    struct ipcpInstance_t *pxInstanceTo,
                                    portId_t xShimPortId);

bool_t xNormalFlowPrebind(struct ipcpInstanceData_t *pxData,
                          portId_t xPortId);

bool_t xNormalMgmtDuWrite(struct rmt_t *pxRmt, portId_t xPortId, struct du_t *pxDu);

bool_t xNormalDuEnqueue(struct ipcpInstanceData_t *pxData,
                        portId_t xN1PortId,
                        struct du_t *pxDu);

bool_t xNormalMgmtDuPost(struct ipcpInstanceData_t *pxData, portId_t xPortId, struct du_t *pxDu);

bool_t xNormalIsFlowAllocated(portId_t xPortId);

bool_t xNormalUpdateFlowStatus(portId_t xPortId, eNormalFlowState_t eNewFlowstate);

bool_t xNormalDuWrite(struct ipcpInstanceData_t *pxData,
                      portId_t xAppPortId,
                      NetworkBufferDescriptor_t *pxNetworkBuffer);

bool_t xNormalUpdateCepIdFlow(portId_t xPortId, cepId_t xCepId);

bool_t xNormalConnectionUpdate(portId_t xAppPortId, cepId_t xSrcCepId, cepId_t xDstCepId);

bool_t xNormalConnectionModify(cepId_t xCepId,
                               address_t xSrc,
                               address_t xDst);

#endif
