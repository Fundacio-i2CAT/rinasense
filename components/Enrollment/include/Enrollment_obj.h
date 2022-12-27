#ifndef _ENROLLMENT_OBJ_H_DEFINED
#define _ENROLLMENT_OBJ_H_DEFINED

#include "RibObject.h"

#include "SerDesNeighbor.h"
#include "SerDesEnrollment.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Using explicity structure name to avoid circular dependencies here */

extern struct xRIBOBJ xEnrollmentNeighborsObject;

extern struct xRIBOBJ xEnrollmentRibObject;

extern struct xRIBOBJ xOperationalStatus;

typedef struct
{
    neighborInfo_t *pxNeighborInfo;
    bool_t xValid;

} neighborsTableRow_t;

typedef struct
{
    pthread_mutex_t xNeighborMutex;

    neighborsTableRow_t xNeighborsTable[NEIGHBOR_TABLE_SIZE];

    NeighborSerDes_t xNeighborSD;

    EnrollmentSerDes_t xEnrollmentSD;

} Enrollment_t;

#ifdef __cplusplus
}
#endif

#endif /* ENROLLMENT_OBJ_H_DEFINED */
