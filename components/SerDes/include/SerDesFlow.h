#ifndef SERDES_FLOW_H_INCLUDED
#define SERDES_FLOW_H_INCLUDED

#include "common/rsrc.h"

#include "FlowAllocator_defs.h"
#include "SerDes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    rsrcPoolP_t xPool;
} SerDesFlow_t;

rsErr_t xSerDesFlowInit(SerDesFlow_t *pxSD);

serObjectValue_t *pxSerDesFlowEncode(SerDesFlow_t *pxSD, flow_t *pxFlow);

flow_t *pxSerDesFlowDecode(SerDesFlow_t *pxSD, uint8_t *pucBuffer, size_t xMessageLength);

#ifdef __cplusplus
}
#endif

#endif /* SERDES_FLOW_H_INCLUDED */
