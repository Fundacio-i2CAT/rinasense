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

#ifdef __cplusplus
extern "C" {
#endif

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
    RsListItem_t xNeighborItem;

    /*Token*/
    string_t pcToken;

} neighborInfo_t;

typedef struct xENROLLMENT_MESSAGE
{
    /*Address*/
    uint64_t ullAddress;

    string_t pcSupportingDifs;

    bool_t xStartEarly;

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
    bool_t xValid;

} neighborsTableRow_t;

#ifdef __cplusplus
}
#endif

#endif /* ENROLLMENT_H_ */
