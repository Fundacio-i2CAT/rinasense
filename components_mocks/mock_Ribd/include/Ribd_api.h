#ifndef _mock_RIBD_RIBD_H
#define _mock_RIBD_RIBD_H

#include "rmt.h"
#include "IPCP_instance.h"
#include "rina_ids.h"

bool_t xRibdConnectToIpcp(struct ipcpNormalData_t *pxIpcpData, name_t *pxSource, name_t *pxDestInfo, portId_t xN1flowPortId, authPolicy_t *pxAuth);
bool_t xRibdDisconnectToIpcp(portId_t xN1flowPortId);
bool_t xRibdProcessLayerManagementPDU(struct ipcpNormalData_t *pxData, portId_t xN1flowPortId, struct du_t *pxDu);
bool_t xRibdSendRequest(string_t pcObjClass, string_t pcObjName, long objInst,
                        opCode_t eOpCode, portId_t xN1flowPortId, serObjectValue_t *pxObjVal);
bool_t xRibdSendResponse(string_t pcObjClass, string_t pcObjName, long objInst,
                         int result, string_t pcResultReason,
                         opCode_t eOpCode, int invokeId, portId_t xN1Port,
                         serObjectValue_t *pxObjVal);

bool_t xTest(void);

#endif // _mock_RIBD_RIBD_H
