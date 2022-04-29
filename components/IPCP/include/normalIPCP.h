#ifndef NORMALIPCP_H_
#define NORMALIPCP_H_

//ipcpInstance_t * pxNormalCreate(name_t *pxName, ipcProcessId_t xId);

ipcpInstance_t *pxNormalCreate(struct ipcpFactory_t *pxFactory, ipcProcessId_t xIpcpId);

BaseType_t xNormalFlowBinding(struct ipcpInstanceData_t *pxUserData,
                              portId_t xPid,
                              ipcpInstance_t *pxN1Ipcp);

cepId_t xNormalConnectionCreateRequest(struct ipcpInstanceData_t *pxData,
                                       portId_t xPortId,
                                       address_t xSource,
                                       address_t xDest,
                                       qosId_t xQosId,
                                       dtpConfig_t *pxDtpCfg,
                                       struct dtcpConfig_t *pxDtcpCfg);

BaseType_t xNormalTest(ipcpInstance_t *pxNormalInstance, ipcpInstance_t *pxN1Ipcp);

BaseType_t xNormalIPCPInitFactory(factories_t *pxFactoriesList);

BaseType_t xNormalRegistering(ipcpInstance_t *pxInstanceFrom, ipcpInstance_t *pxInstanceTo);
BaseType_t xNormalFlowAllocationRequest(struct ipcpInstance_t *pxInstanceFrom, struct ipcpInstance_t *pxInstanceTo, portId_t xShimPortId);

BaseType_t xNormalFlowPrebind(struct ipcpInstanceData_t *pxData,
                              portId_t xPortId);

BaseType_t xNormalMgmtDuWrite(struct ipcpInstanceData_t *pxData, portId_t xPortId, struct du_t *pxDu);

#endif