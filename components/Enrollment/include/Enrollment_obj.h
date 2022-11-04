#ifndef _ENROLLMENT_OBJ_H_DEFINED
#define _ENROLLMENT_OBJ_H_DEFINED

#include "SerDesNeighbor.h"
#include "SerDesEnrollment.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    neighborInfo_t *pxNeighborInfo;
    bool_t xValid;

} neighborsTableRow_t;

typedef struct
{
    neighborsTableRow_t xNeighborsTable[NEIGHBOR_TABLE_SIZE];

    NeighborSerDes_t xNeighborSD;

    EnrollmentSerDes_t xEnrollmentSD;

} Enrollment_t;

#ifdef __cplusplus
}
#endif

#endif /* ENROLLMENT_OBJ_H_DEFINED */
