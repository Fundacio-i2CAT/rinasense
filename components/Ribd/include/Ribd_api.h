#ifndef _RIBD_API_H_INCLUDED
#define _RIBD_API_H_INCLUDED

#include "private/Ribd_fwd_defs.h"
#include "CdapMessage.h"

#ifdef __cplusplus
extern "C" {
#endif

ribObject_t *pxRibObjectFind(Ribd_t *pxRibd, string_t ucRibObjectName);

rsErr_t xRibObjectAdd(Ribd_t *pxRibd, void *pvOwner, ribObject_t *pxRibObject);

rsErr_t xRibObjectReqWait(Ribd_t *pxRibd, invokeId_t unInvokeId, void **pxResp);

rsErr_t xRibNormalInit(Ribd_t *pxRibd);

rsErr_t xRibLoopbackInit(Ribd_t *pxRibd);

rsErr_t xRibIncoming(Ribd_t *pxRibd, du_t *pxDu, portId_t unPort);

rsErr_t xRibNormalIncoming(Ribd_t *pxRibd,
                           messageCdap_t *pxDecodeCdap,
                           portId_t unPort);

rsErr_t xRibObjectErr(Ribd_t *pxRibd,
                      ribObject_t *pxRibObj,
                      opCode_t eOpCode,
                      portId_t unPort,
                      invokeId_t unInvokeId,
                      int nResultCode,
                      string_t pcResultReason);

rsErr_t xRibObjectQuery(Ribd_t *pxRibd,
                        ribObject_t *pxRibObj,
                        opCode_t eOpCode,
                        portId_t unPort,
                        serObjectValue_t *pxObjVal,
                        ribQueryTypeInfo_t xRibQueryTypeInfo);

rsErr_t xRibObjectReply(Ribd_t *pxRibd,
                        ribObject_t *pxRibObj,
                        opCode_t eOpCode,
                        portId_t unPort,
                        invokeId_t unInvokeId,
                        int nResultCode,
                        string_t pcResultReason,
                        serObjectValue_t *pxObjVal);

rsErr_t xRibObjectWaitReply(Ribd_t *pxRibd,
                            invokeId_t unInvokeId,
                            const struct timespec *pxTimeout,
                            void **pxResp);

/* CONNECT */
#define xRibCONNECT_REPLY(rib, obj, port, id, val)                  \
    xRibObjectReply(rib, obj, M_CONNECT_R, port, id, 0, NULL, NULL)

/* RELEASE */
#define xRibRELEASE_REPLY(rib, obj, port, id)  \
    xRibObjectReply(rib, obj, M_RELEASE_R, port, id, 0, NULL, NULL)

/* CREATE */
#define xRibCREATE_CMD(rib, obj, port, val)                             \
    xRibObjectQuery(rib, obj, M_CREATE, port, val,                      \
                    (ribQueryTypeInfo_t){.eType = RIB_QUERY_TYPE_CMD})

/* START */
#define xRibSTART_QUERY_SYNC(rib, obj, port, val, iid)                  \
    xRibObjectQuery(rib, obj, M_START, port, val,                       \
                    (ribQueryTypeInfo_t){.eType = RIB_QUERY_TYPE_SYNC, .pxInvokeId = iid})

#define xRibSTART_QUERY_ASYNC(rib, obj, port, val, cb)    \
    xRibObjectQuery(rib, obj, M_START, port, val, \
                    (ribQueryTypeInfo_t){.eType = RIB_QUERY_TYPE_ASYNC, .fnCb = cb })

#define xRibSTART_REPLY(rib, obj, port, id, val)                    \
    xRibObjectReply(rib, obj, M_START_R, port, id, 0, NULL, val)

#define xRibSTART_CMD(rib, obj, port, val)                              \
    xRibObjectQuery(rib, obj, M_START, port, val,                       \
                    (ribQueryTypeInfo_t){.eType = RIB_QUERY_TYPE_CMD})


/* STOP */
#define xRibSTOP_QUERY_SYNC(rib, obj, port, val, iid)                        \
    xRibObjectQuery(rib, obj, M_STOP, port, val,                        \
                    (ribQueryTypeInfo_t){.eType = RIB_QUERY_TYPE_SYNC, .pxInvokeId = iid})


#ifdef __cplusplus
}
#endif

#endif // _RIBD_API_H_INCLUDED
