#ifndef _RIBD_CONNECTION_H_INCLUDED
#define _RIBD_CONNECTION_H_INCLUDED

#include "portability/port.h"

#include "common/rina_ids.h"
#include "common/rina_name.h"

#include "private/Ribd_fwd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CONNECTION_RELEASED,
    CONNECTION_IN_PROGRESS,
    CONNECTION_OK,
} connectionStatus_t;

typedef struct xAPP_CONNECTION
{
    rname_t xSourceInfo;
    rname_t xDestinationInfo;
    uint8_t uCdapVersion;
    uint8_t uRibVersion;
    connectionStatus_t xStatus;
    portId_t unPort;

} appConnection_t;

rsMemErr_t xRibConnectionAdd(struct xRIBD *pxRibd, rname_t *pxSrc, rname_t *pxDst, portId_t unPort);

appConnection_t *pxRibConnectionFind(struct xRIBD *pxRibd, portId_t unPort);

void vRibConnectionRelease(struct xRIBD *pxRibd, appConnection_t *pxAppCon);

#ifdef __cplusplus
}
#endif

#endif /* _RIBD_CONNECTION_H_INCLUDED */
