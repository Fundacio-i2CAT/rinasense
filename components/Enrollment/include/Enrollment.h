/*
 * Enrollment.h
 *
 *  Created on: 14 february. 2022
 *      Author: i2CAT
 */

#ifndef ENROLLMENT_H_INCLUDED
#define ENROLLMENT_H_INCLUDED

#include "Rib.h"
#include "IPCP_normal_defs.h"

/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/

#define NEIGHBOR_TABLE_SIZE (5)
typedef enum
{
    eENROLLMENT_NONE = 0,
    eENROLLMENT_IN_PROGRESS,
    eENROLLED,

} enrollmentState_t;

typedef struct xNEIGHBOR_INFO
{
    /*States of enrollment process*/
    enrollmentState_t eEnrollmentState;

    /*Address of the Neighbor*/
    string_t pcApName;

    /*N-1 Port associated to this neighbor*/
    portId_t xN1Port;

    /*Neighbor Address*/
    address_t xNeighborAddress;

    /*List Item to add into the Neighbor List*/
    ListItem_t xNeighborItem;

    /*Token*/
    string_t pcToken;

} neighborInfo_t;

typedef struct xENROLLMENT_MESSAGE
{
    /*Address*/
    uint64_t ullAddress;

    string_t pcSupportingDifs;

    BaseType_t xStartEarly;

    string_t pcToken;

} enrollmentMessage_t;

typedef struct xNEIGHBOR
{
    string_t pcApName;
    string_t pcApInstance;
    string_t pcAeName;
    string_t pcAeInstance;
    uint64_t ullAddress;
    string_t pcSupportingDifs;
} neighborMessage_t;

typedef struct xNEIGHBOR_ROW
{
    neighborInfo_t *pxNeighborInfo;
    BaseType_t xValid;

} neighborsTableRow_t;

void xEnrollmentInit(struct ipcpNormalData_t *pxIpcpData, portId_t xPortId);

BaseType_t xEnrollmentEnroller(struct ribObject_t *pxEnrRibObj, serObjectValue_t *pxObjValue, string_t pcRemoteApName,
                               string_t pcLocalApName, int invokeId, portId_t xN1Port);

BaseType_t xEnrollmentHandleConnectR(struct ipcpNormalData_t *pxData, string_t pcRemoteProcessName, portId_t xN1Port);
BaseType_t xEnrollmentHandleStartR(string_t pcRemoteApName, serObjectValue_t *pxSerObjValue);
BaseType_t xEnrollmentHandleStopR(string_t pcRemoteApName);

BaseType_t xEnrollmentHandleStop(struct ribObject_t *pxEnrRibObj,
                                 serObjectValue_t *pxObjValue, string_t pcRemoteApName,
                                 string_t pcLocalProcessName, int invokeId, portId_t xN1Port);

BaseType_t xEnrollmentHandleOperationalStart(struct ribObject_t *pxOperRibObj, serObjectValue_t *pxSerObjectValue, string_t pcRemoteApName,
                                             string_t pxLocalApName, int invokeId, portId_t xN1Port);

address_t xEnrollmentGetNeighborAddress(string_t pcRemoteApName);

#endif /* ENROLLMENT_H_ */
