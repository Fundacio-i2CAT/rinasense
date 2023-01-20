#ifndef _RIBD_H_INCLUDED
#define _RIBD_H_INCLUDED

#include "portability/port.h"
#include "common/rina_name.h"
#include "common/rsrc.h"

#include "rina_common_port.h"

#include "SerDes.h"
#include "SerDesAData.h"
#include "SerDesMessage.h"

#include "private/Ribd_fwd_defs.h"
#include "Ribd_connections.h"
#include "Ribd_requests.h"
#include "RibObject.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TAG_RIB "[RIB]"

/* FIXME: Those should be moved into configuration */
#define APP_CONNECTION_TABLE_SIZE (5)
#define RESPONSE_HANDLER_TABLE_SIZE (10)
#define RIB_TABLE_SIZE (10)

typedef rsErr_t (*RibIncoming)(struct xRIBD *pxRibd,
                               messageCdap_t *pxDecodeCdap,
                               portId_t unPort);

typedef rsErr_t (*RibOutgoing)(struct xRIBD *pxRibd,
                               messageCdap_t *pxOutgoingCdap,
                               portId_t unPort);

typedef struct xRIBD
{
    MessageSerDes_t xMsgSD;

    ADataSerDes_t xADataSD;

    /* RIB Input/Output */
    RibIncoming fnRibInput;
    RibOutgoing fnRibOutput;

    /* This is true if locking the RIB is needed. This is set at
     * initialization depending on if the RIB is normal or loopback */
    bool_t xDoLock;

    /* Pool for messageCdap_t objects */
    rsrcPoolP_t xMsgPool;

    /* Pool for ribCallbackOps_t objects */
    rsrcPoolP_t xCbPool;

    /* Pool for DU objects */
    rsrcPoolP_t xDuPool;

    /* Table to manage the app connections */
    appConnection_t xAppConnections[APP_CONNECTION_TABLE_SIZE];

    /* Table to manage the pending request to response*/
    ribPendingReq_t xPendingReqs[RESPONSE_HANDLER_TABLE_SIZE];

    struct xRIBOBJ *pxRibObjects[RIB_TABLE_SIZE];

    pthread_mutex_t xMutex;

} Ribd_t;

#ifdef __cplusplus
}
#endif

#endif // _RIBD_H_INCLUDED
