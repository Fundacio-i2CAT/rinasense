#ifndef ENROLLMENT_API_H_INCLUDED
#define ENROLLMENT_API_H_INCLUDED

#include "common/rina_ids.h"
#include "common/error.h"

#include "Ribd_defs.h"
#include "IPCP_normal_defs.h"
#include "Enrollment_obj.h"
#include "SerDesNeighbor.h"
#include "SerDesEnrollment.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TAG_ENROLLMENT "[ENROLLMENT]"

rsErr_t xEnrollmentInit(Enrollment_t *pxEnrollment, Ribd_t *pxRibd);


rsErr_t xEnrollmentHandleConnect(struct ipcpInstanceData_t *pxData,
                                 string_t pcRemoteName,
                                 portId_t unPort,
                                 invokeId_t unInvokeId);

bool_t xEnrollmentHandleConnectR(struct ipcpInstanceData_t *pxData,
                                 string_t pcRemoteName,
                                 portId_t unPort);

/* Neighbor manipulation */

address_t xEnrollmentGetNeighborAddress(Enrollment_t *pxEnrollment, string_t pcRemoteApName);

neighborInfo_t *pxEnrollmentFindNeighbor(Enrollment_t *pxEnrollment, string_t pcRemoteApName);

bool_t xEnrollmentAddNeighborEntry(Enrollment_t *pxEnrollment, neighborInfo_t *pxNeighbor);

neighborInfo_t *pxEnrollmentCreateNeighInfo(Enrollment_t *pxEnrollment, string_t pcApName, portId_t xN1Port);

/* RIB Handlers */

bool_t xEnrollmentNeighborsRead(struct ipcpInstanceData_t *pxData,
                                ribObject_t *pxThis,
                                serObjectValue_t *pxObjValue,
                                rname_t *pxRemoteName,
                                rname_t *pxLocalName,
                                invokeId_t invokeId,
                                portId_t unPort);


bool_t xEnrollmentHandleStartR(struct ipcpInstanceData_t *pxData,
                               string_t pcRemoteApName,
                               serObjectValue_t *pxSerObjValue);

bool_t xEnrollmentHandleStopR(struct ipcpInstanceData_t *pxData,
                              string_t pcRemoteApName);


#ifdef __cplusplus
}
#endif

#endif // ENROLLMENT_API_H_INCLUDED
