#ifndef SERDES_ADATA_H_INCLUDED
#define SERDES_ADATA_H_INCLUDED

#include <stdint.h>

#include "SerDes.h"
#include "common/rsrc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    rsrcPoolP_t xPool;

} ADataSerDes_t;

typedef struct
{
    int32_t xSourceAddress;
    int32_t xDestinationAddress;
    serObjectValue_t *pxMsgCdap;
} aDataMsg_t;

bool_t xSerDesADataInit(ADataSerDes_t *pxSD);

aDataMsg_t *pxSerDesADataDecode(ADataSerDes_t *pxSD, uint8_t *pucBuffer, size_t xMessageLength);

#ifdef __cplusplus
}
#endif

#endif /* SERDES_ADATA_H_INCLUDED */
