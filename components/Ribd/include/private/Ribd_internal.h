#ifndef _RIBD_INTERNAL_H_INCLUDED
#define _RIBD_INTERNAL_H_INCLUDED

#include <pthread.h>

#include "Ribd_defs.h"
#include "RibObject.h"

#ifdef __cplusplus
extern "C" {
#endif

ribObjectResMethod xRibGetObjectReplyMethod(ribObject_t *pxRibObj, opCode_t eOpCode);

ribObjectReqMethod xRibGetObjectMethod(ribObject_t *pxRibObj, opCode_t eOpCode);

rsErr_t xRibAddPendingResponse(Ribd_t *pxRibd,
                               string_t pcObjName,
                               invokeId_t unInvokeID,
                               opCode_t eReqOpCode,
                               ribResponseCb xRibResCb);

ribPendingReq_t *xRibFindPendingResponse(Ribd_t *pxRibd, invokeId_t unInvokeID);

void xRibFreePendingResponse(ribPendingReq_t *pxPendingReq);

rsErr_t xRibCommonInit(Ribd_t *pxRibd);

static inline void vRibLock(Ribd_t *pxRibd)
{
    if (pxRibd->xDoLock)
        pthread_mutex_lock(&pxRibd->xMutex);
}

static inline void vRibUnlock(Ribd_t *pxRibd)
{
    if (pxRibd->xDoLock)
        pthread_mutex_unlock(&pxRibd->xMutex);
}

#ifdef __cplusplus
}
#endif

#endif /* _RIBD_INTERNAL_H_INCLUDED */
