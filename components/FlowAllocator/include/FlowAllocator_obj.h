#ifndef _FLOWALLOCATOR_OBJ_H_INCLUDED
#define _FLOWALLOCATOR_OBJ_H_INCLUDED

#include "Ribd_obj.h"
#include "Enrollment_obj.h"
#include "SerDesFlow.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    eFAI_NONE,
    eFAI_PENDING,
    eFAI_ALLOCATED,

} eFaiState_t;

typedef struct
{
    /* Flow allocator Instance Item */
    RsListItem_t xInstanceItem;

    /* PortId associated to this flow Allocator Instance*/
    portId_t xPortId;

    /* State */
    eFaiState_t eFaiState;

    /* Flow Allocator Request*/
    flowAllocateHandle_t *pxFlowAllocatorHandle;

} flowAllocatorInstance_t;

struct FlowRequestRow
{
    flowAllocatorInstance_t *pxFAI;

    bool_t xValid;
};

typedef struct
{
    /* List of FAI*/
    RsList_t xFlowAllocatorInstances;

    struct FlowRequestRow xFlowRequestTable[FLOWS_REQUEST];

    pthread_mutex_t xMux;

    SerDesFlow_t xSD;

    Ribd_t *pxRib;

    /* FIXME: THIS IS ONLY FOR ACCESS TO THE NEIGHBOR DATABASE, WHICH
       SHOULD NOT BE IN THE ENROLLMENT MODULE ANYWAY. */
    Enrollment_t *pxEnrollment;

} flowAllocator_t;

#ifdef __cplusplus
}
#endif

#endif /* _FLOWALLOCATOR_OBJ_H_INCLUDED */
