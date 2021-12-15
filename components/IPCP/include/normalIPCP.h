#ifndef NORMALIPCP_H_
#define NORMALIPCP_H_

//ipcpInstance_t * pxNormalCreate(name_t *pxName, ipcProcessId_t xId);

BaseType_t pxNormalCreate(ipcpFactory_t *pxFactory);

BaseType_t xNormalFlowBinding(struct ipcpInstanceData_t * pxUserData,
                             portId_t                   xPid,
                             ipcpInstance_t *      pxN1Ipcp);

cepId_t xNormalConnectionCreateRequest(struct ipcpInstanceData_t * pxData,
				   struct ipcpInstance_t *      xUserIpcp,
                                   portId_t                   xPortId,
                                   address_t                   xSource,
                                   address_t                   xDest,
                                   qosId_t                    xQosId,
                                   dtpConfig_t *         pxDtpCfg,
                                   struct dtcpConfig_t *        pxDtcpCfg);

BaseType_t xNormalTest(ipcpInstance_t * pxNormalInstance);

#endif