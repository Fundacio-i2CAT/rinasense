#ifndef _RIB_OBJECT_H_INCLUDED
#define _RIB_OBJECT_H_INCLUDED

#include "portability/port.h"
#include "common/error.h"
#include "common/rina_ids.h"
#include "common/rina_name.h"

#include "SerDes.h"
#include "IPCP_instance.h"

#include "Ribd_msg.h"
#include "Ribd_connections.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIB_REQUEST_METHOD_ARGS                 \
        struct xRIBOBJ *pxThis,                 \
        appConnection_t *pxAppCon,              \
        messageCdap_t *pxMsg

#define RIB_REPLY_METHOD_ARGS                   \
        struct xRIBOBJ *pxThis,                 \
        appConnection_t *pxAppCon,              \
        messageCdap_t *pxMsg,                   \
        void *ppxResp

#define RIB_INIT_METHOD_ARGS                    \
        struct xRIBD *pxRibd,                   \
        void *pxOwner,                          \
        struct xRIBOBJ *pxThis

#define DECLARE_RIB_OBJECT_REQUEST_METHOD(x) \
    rsErr_t x(RIB_REQUEST_METHOD_ARGS)

#define DECLARE_RIB_OBJECT_REPLY_METHOD(x) \
    rsErr_t x(RIB_REPLY_METHOD_ARGS)

#define DECLARE_RIB_OBJECT_INIT_METHOD(x) \
    rsErr_t x(RIB_INIT_METHOD_ARGS);

typedef rsErr_t (*ribObjectReqMethod)(RIB_REQUEST_METHOD_ARGS);

typedef rsErr_t (*ribObjectResMethod)(RIB_REPLY_METHOD_ARGS);

typedef bool_t (*ribObjectShow)(struct ipcpInstanceData_t *pxData,
                                struct xRIBOBJ *pxThis);

typedef void (*ribObjectFree)(struct xRIBOBJ *pxThis);

typedef rsErr_t (*ribObjectInit)(RIB_INIT_METHOD_ARGS);

struct xRIBOBJ
{
    /* Set to the owning RIBD object only when added */
    struct xRIBD *pxRibd;

    /* Owner object: can be things like an Enrollment_t structure */
    void *pvOwner;

    /* This is private data that can be set on initialization */
    void *pvPriv;

    string_t ucObjName;
    string_t ucObjClass;
    long ulObjInst;

    ribObjectReqMethod fnStart;
    ribObjectReqMethod fnStop;
    ribObjectReqMethod fnCreate;
    ribObjectReqMethod fnDelete;
    ribObjectReqMethod fnRead;
    ribObjectReqMethod fnWrite;

    ribObjectResMethod fnStartReply;
    ribObjectResMethod fnStopReply;
    ribObjectResMethod fnCreateReply;
    ribObjectResMethod fnDeleteReply;
    ribObjectResMethod fnReadReply;
    ribObjectResMethod fnWriteReply;

    ribObjectShow fnShow;

    ribObjectInit fnInit;
    ribObjectFree fnFree;
};

typedef struct xRIBOBJ ribObject_t;

#ifdef __cplusplus
}
#endif

#endif /* _RIB_OBJECT_H_INCLUDED */
