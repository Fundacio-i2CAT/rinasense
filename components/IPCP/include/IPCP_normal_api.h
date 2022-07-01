#ifndef NORMALIPCP_H_
#define NORMALIPCP_H_

#include "IpcManager.h"
#include "rmt.h"
#include "IPCP_normal_defs.h"

BaseType_t xNormalFlowBinding(struct ipcpNormalData_t *pxUserData,
                              portId_t xPid,
                              ipcpInstance_t *pxN1Ipcp);

cepId_t xNormalConnectionCreateRequest(struct efcpContainer_t *pxEfcpc,
                                       portId_t xPortId,
                                       address_t xSource,
                                       address_t xDest,
                                       qosId_t xQosId,
                                       dtpConfig_t *pxDtpCfg,
                                       struct dtcpConfig_t *pxDtcpCfg);

BaseType_t xNormalTest(ipcpInstance_t *pxNormalInstance, ipcpInstance_t *pxN1Ipcp);

BaseType_t xNormalRegistering(ipcpInstance_t *pxShimInstance, name_t *pxDifName, name_t *pxName);

BaseType_t xNormalFlowAllocationRequest(struct ipcpInstance_t *pxInstanceFrom, struct ipcpInstance_t *pxInstanceTo, portId_t xShimPortId);

BaseType_t xNormalFlowPrebind(struct ipcpInstanceData_t *pxData,
                              portId_t xPortId);

BaseType_t xNormalMgmtDuWrite(struct rmt_t *pxRmt, portId_t xPortId, struct du_t *pxDu);

BaseType_t xNormalDuEnqueue(struct ipcpNormalData_t *pxData,
                            portId_t xN1PortId,
                            struct du_t *pxDu);

BaseType_t xNormalMgmtDuPost(struct ipcpNormalData_t *pxData, portId_t xPortId, struct du_t *pxDu);

#endif
