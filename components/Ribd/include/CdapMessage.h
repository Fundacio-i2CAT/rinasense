#ifndef _RIBD_CDAPMESSAGE_H_INCLUDED
#define _RIBD_CDAPMESSAGE_H_INCLUDED

#include "Ribd.h"

#ifdef __cplusplus
extern "C" {
#endif

void vRibdCdapMsgFree(messageCdap_t *pxMsg);

messageCdap_t *pxRibdCdapMsgCreate(Ribd_t *pxRibd, size_t unSz);

messageCdap_t *pxRibdCdapMsgCreateRequest(Ribd_t *pxRibd,
                                          string_t pcObjClass,
                                          string_t pcObjName,
                                          long nObjInst,
                                          opCode_t eOpCode,
                                          serObjectValue_t *pxObjValue);

messageCdap_t *pxRibdCdapMsgCreateResponse(Ribd_t *pxRibd,
                                           string_t pcObjClass,
                                           string_t pcObjName,
                                           long nObjInst,
                                           opCode_t eOpCode,
                                           serObjectValue_t *pxObjValue,
                                           int nResultCode,
                                           string_t pcResultReason,
                                           int nInvokeId);

#ifdef __cplusplus
}
#endif

#endif // _RIBD_CDAPMESSAGE_H_INCLUDED
