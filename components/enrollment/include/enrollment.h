/*
 * Enrollment.h
 *
 *  Created on: 14 february. 2022
 *      Author: i2CAT
 */



#ifndef ENROLLMENT_H_INCLUDED
#define ENROLLMENT_H_INCLUDED



/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/

typedef enum{
    eENROLLMENT_NONE = 0,
    eENROLLMENT_IN_PROGRESS,
    eENROLLED,
    
}enrollmentState_t;

typedef struct xNEIGHBOR_INFO{
    /*States of enrollment process*/
    enrollmentState_t eEnrollmentState;

    /*Address of the Neighbor*/
    string_t xAPName;

    /*N-1 Port associated to tis neighbor*/
    portId_t xN1Port;

    /*List Item to add into the Neighbor List*/
    ListItem_t xNeighborItem;

}neighborInfo_t;


typedef struct xENROLLMENT_MESSAGE{
    /*Address*/
    uint64_t ullAddress;

    string_t xSupportingDifs;

    BaseType_t xStartEarly;

    string_t xToken;

}enrollmentMessage_t;

typedef struct xSER_OBJECT_VALUE{
    /*Buffer with the message*/
    void * pvSerBuffer;

    /*Size of the buffer*/
    size_t xSerLength;

}serObjectValue_t;

void xEnrollmentInit(portId_t xPortId );

BaseType_t xEnrollmentHandleConnectR(string_t xRemoteProcessName, portId_t xN1Port);

#endif /* ENROLLMENT_H_ */

