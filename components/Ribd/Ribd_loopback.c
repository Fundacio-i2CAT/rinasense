#include "portability/port.h"

#include "private/Ribd_internal.h"
#include "Ribd_defs.h"
#include "Ribd_msg.h"
#include "Ribd_api.h"

rsErr_t prvRibLoopbackOutgoing(Ribd_t *pxRibd,
                               messageCdap_t *pxOutgoingCdap,
                               portId_t unPort)
{
    return pxRibd->fnRibInput(pxRibd, pxOutgoingCdap, unPort);
}

rsErr_t xRibLoopbackInit(Ribd_t *pxRibd)
{
    pxRibd->fnRibInput = &xRibNormalIncoming;
    pxRibd->fnRibOutput = &prvRibLoopbackOutgoing;
    pxRibd->xDoLock = false;

    return xRibCommonInit(pxRibd);
}
