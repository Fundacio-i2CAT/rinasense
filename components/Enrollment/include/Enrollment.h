/*
 * Enrollment.h
 *
 *  Created on: 14 february. 2022
 *      Author: i2CAT
 */

#ifndef ENROLLMENT_H_INCLUDED
#define ENROLLMENT_H_INCLUDED

#include "Rib.h"

/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/

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
    string_t xAPName;

    /*N-1 Port associated to this neighbor*/
    portId_t xN1Port;

    /*Neighbor Address*/
    address_t xNeighborAddress;

    /*List Item to add into the Neighbor List*/
    ListItem_t xNeighborItem;

} neighborInfo_t;

typedef struct xENROLLMENT_MESSAGE
{
    /*Address*/
    uint64_t ullAddress;

    string_t xSupportingDifs;

    BaseType_t xStartEarly;

    string_t xToken;

} enrollmentMessage_t;

typedef struct xNEIGHBOR
{
    string_t ucApName;
    string_t ucApInstance;
    string_t ucAeName;
    string_t ucAeInstance;
    uint64_t ullAddress;
    string_t ucSupportingDifs;
} neighborMessage_t;

void xEnrollmentInit(portId_t xPortId);

BaseType_t xEnrollmentHandleConnectR(string_t xRemoteProcessName, portId_t xN1Port);
BaseType_t xEnrollmentHandleStartR(string_t xRemoteApName, serObjValue_t *pxSerObjValue);

BaseType_t xEnrollmentHandleCreate(struct ribObject_t * pxRibOject, serObjectValue_t *pxObjValue, string_t xRemoteProcessName,
                                   string_t xLocalProcessName, int invokeId, portId_t xN1Port);

#endif /* ENROLLMENT_H_ */
