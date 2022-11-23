#ifndef _RIBD_API_H_INCLUDED
#define _RIBD_API_H_INCLUDED

#include "Ribd_obj.h"

#ifdef __cplusplus
extern "C" {
#endif

appConnection_t *pxRibdFindAppConnection(Ribd_t *pxRibd, portId_t unPort);

bool_t xRibdInit(Ribd_t *pxRibd);

bool_t xRibdConnectToIpcp(Ribd_t *pxRibd,
                          struct ipcpInstanceData_t *pxIpcpData,
                          rname_t *pxSource,
                          rname_t *pxDestInfo,
                          portId_t xN1flowPortId,
                          authPolicy_t *pxAuth);

bool_t xRibdDisconnectToIpcp(portId_t xN1flowPortId);

bool_t xRibdProcessLayerManagementPDU(struct ipcpInstanceData_t *pxData,
                                      portId_t xN1flowPortId,
                                      du_t *pxDu);

bool_t xRibdSendRequest(Ribd_t *pxRibd,
                        string_t pcObjClass,
                        string_t pcObjName,
                        long objInst,
                        opCode_t eOpCode,
                        portId_t xN1flowPortId,
                        serObjectValue_t *pxObjVal);

bool_t xRibdSendResponse(Ribd_t *pxRibd,
                         string_t pcObjClass,
                         string_t pcObjName,
                         long objInst,
                         int result,
                         string_t pcResultReason,
                         opCode_t eOpCode,
                         int invokeId,
                         portId_t xN1Port,
                         serObjectValue_t *pxObjVal);

#ifdef __cplusplus
}
#endif

#endif // _RIBD_API_H_INCLUDED
