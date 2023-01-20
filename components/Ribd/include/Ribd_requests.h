#ifndef _RIBD_REQUESTS_H_INCLUDED
#define _RIBD_REQUESTS_H_INCLUDED

#include "portability/port.h"
#include "common/error.h"
#include "common/rina_ids.h"

#include "RibObject.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Response to posted requests are delivered through this kind of
 * callback. The content of 'pxRes' depends on the type of response. */
typedef rsErr_t (*ribResponseCb)(struct xRIBD *pxRibd,
                                 struct xRIBOBJ *pxRibObj,
                                 void *pxResp);

typedef enum {
    /* Sets an invoke ID and a callback. The RIB calls the callback
     * once a result is available. */
    RIB_QUERY_TYPE_ASYNC,

    /* Sets an invoke ID, no callback. The invokeID can be used to
     * synchronously wait for an answer. */
    RIB_QUERY_TYPE_SYNC,

    /* Sets neither callbacks nor invokeID. The request will be sent
     * directly to the receiver and no answer will be expected */
    RIB_QUERY_TYPE_CMD

} ribQueryType_t;

typedef struct {
    ribQueryType_t eType;

    union {
        ribResponseCb fnCb;
        invokeId_t *pxInvokeId;
    };
} ribQueryTypeInfo_t;

typedef struct xRESPONSE_HANDLER_ROW
{
    /* Number with which the request was sent */
    invokeId_t unInvokeID;

    /* Type of request we're waiting for */
    opCode_t eReqOpCode;

    /* Callback to which deliver the response. Might be NULL */
    ribResponseCb xRibResCb;

    /* Wait on this condition to change to wait for an answer */
    pthread_cond_t xWaitCond;

    pthread_mutex_t xMutex;

    /* The actual response content */
    void *pxResp;

    /* The name of the object that can handle the response. Apparently
     * IRATI keeps track of that and doesn't send it on replies. */
    char *pcObjName;

} ribPendingReq_t;

#ifdef __cplusplus
}
#endif

#endif /* _RIBD_REQUEST_H_INCLUDED */
