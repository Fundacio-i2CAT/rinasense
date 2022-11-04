#ifndef SERDES_NEIGHBOR_H_INCLUDED
#define SERDES_NEIGHBOR_H_INCLUDED

#include "portability/port.h"
#include "common/list.h"
#include "common/rsrc.h"
#include "common/rina_ids.h"

#include "SerDes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEIGHBOR_TABLE_SIZE (5)
typedef enum
{
    eENROLLMENT_NONE = 0,
    eENROLLMENT_IN_PROGRESS,
    eENROLLED,

} enrollmentState_t;

typedef struct
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

typedef struct
{
    string_t pcApName;
    string_t pcApInstance;
    string_t pcAeName;
    string_t pcAeInstance;
    uint64_t ullAddress;
    string_t pcSupportingDifs;
} neighborMessage_t;

typedef struct
{
    rsrcPoolP_t xEncPool;
    rsrcPoolP_t xDecPool;

} NeighborSerDes_t;

bool_t xSerDesNeighborInit(NeighborSerDes_t *pxSD);

serObjectValue_t *pxSerDesNeighborEncode(NeighborSerDes_t *pxSD, neighborMessage_t *pxMsg);

neighborMessage_t *pxSerDesNeighborDecode(NeighborSerDes_t *pxSD, uint8_t *pucBuffer, size_t xMessageLength);

#ifdef __cplusplus
}
#endif

#endif /* SERDES_NEIGHBOR_H_INCLUDE */
