#ifndef IPCP_H_
#define IPCP_H_

#include <stdlib.h>

#include "configSensor.h"
#include "common/rina_name.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Constant to be used at for normal DIF data structure
 * initialisation. */
#define IPCP_INSTANCE_DATA_NORMAL 0x1

#ifndef NDEBUG
#define ASSERT_INSTANCE_DATA(ptr, type)         \
    do { \
        if (*(uint8_t *)ptr != type) {                                  \
            LOGE("[ABORT]", "Instance data type 0x%X, expected 0x%X", *(uint8_t *)ptr, type); \
            abort();                                                    \
        }                                                               \
    } while (0);
#else
#define ASSERT_INSTANCE_DATA(ptr, type)
#endif

#define CALL_IPCP(ipcp, ipcp_fn, ...)                                   \
    RsAssert((*ipcp).pxOps && (*ipcp).pxOps->ipcp_fn != NULL);          \
    LOGD("*IPC*", "CALL_IPCP FN: %s ID: %u", #ipcp_fn, (*ipcp).xId);    \
    (*ipcp).pxOps->ipcp_fn(ipcp, __VA_ARGS__)

#define CALL_IPCP_CHECK(sv, ipcp, ipcp_fn, ...)                         \
    RsAssert((*ipcp).pxOps && (*ipcp).pxOps->ipcp_fn != NULL);          \
    LOGD("*IPC*", "CALL_IPCP FN: %s ID: %u", #ipcp_fn, (*ipcp).xId);    \
    sv = ((*ipcp).pxOps->ipcp_fn(ipcp, __VA_ARGS__));                   \
    if (sv)                                                             \
        LOGD("*IPC*", "CALL_IPCP FN: %s SUCCESS", #ipcp_fn);            \
    else                                                                \
        LOGD("*IPC*", "CALL_IPCP FN: %s FAILED", #ipcp_fn);             \
    if (!sv)

#define CALL_IPCP_FOR_CEPID(sv, ipcp, ipcp_fn, ...) \
    RsAssert((*ipcp).pxOps && (*ipcp).pxOps->ipcp_fn != NULL);          \
    LOGD("*IPC*", "CALL_IPCP_FOR_CEPID FN: %s ID: %u", #ipcp_fn, (*ipcp).xId); \
    sv = ((*ipcp).pxOps->ipcp_fn(ipcp, __VA_ARGS__));                   \
    if (sv != CEP_ID_WRONG)                                             \
        LOGD("*IPC*", "CALL_IPCP_FOR_CEPID FN: %s SUCCESS", #ipcp_fn);  \
    else                                                                \
        LOGD("*IPC*", "CALL_IPCP_FOR_CEPID FN: %s FAILED", #ipcp_fn);   \
    if (sv == CEP_ID_WRONG)

#define GET_IPCP_NAME(ipcp, nm)                 \
    RsAssert((*ipcp).pxOps && (*ipcp).pxOps->ipcpName != NULL); \
    nm = (*ipcp).pxOps->ipcpName(ipcp);         \
    RsAssert(nm != NULL)

#define GET_DIF_NAME(ipcp, nm)                  \
    RsAssert((*ipcp).pxOps && (*ipcp).pxOps->difName != NULL);   \
    nm = (*ipcp).pxOps->difName(ipcp);          \
    RsAssert(nm != NULL)

#ifdef __cplusplus
}
#endif

#endif
