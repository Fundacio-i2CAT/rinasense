#include "Ribd.h"
#include "CdapMessage.h"
#include "Ribd_msg.h"
#include "SerDes.h"
#include "common/rina_name.h"

void prvCopyCommonStrings(messageCdap_t *pxMsg,
                          string_t pcObjClass, string_t pcObjName, serObjectValue_t *pxObjValue,
                          size_t unSzObjClass, size_t unSzObjName,
                          void **pxNext)
{
    void *px;

    px = (void *)pxMsg + sizeof(messageCdap_t);

    if (pxObjValue != NULL) {
        pxMsg->pxObjValue = px;
        pxMsg->pxObjValue->pvSerBuffer = (px += sizeof(serObjectValue_t));
        pxMsg->pxObjValue->xSerLength = pxObjValue->xSerLength;
        px += pxObjValue->xSerLength;

        memcpy(pxMsg->pxObjValue->pvSerBuffer, pxObjValue->pvSerBuffer, pxObjValue->xSerLength);
    }

    if (pcObjName != NULL) {
        pxMsg->pcObjName = (px += unSzObjName + 1);
        strcpy(pxMsg->pcObjName, pcObjName);
    }

    if (pcObjClass != NULL) {
        pxMsg->pcObjClass = (px += unSzObjClass + 1);
        strcpy(pxMsg->pcObjClass, pcObjClass);
    }

    if (pxNext) *pxNext = px;
}

size_t prvCountCommonItems(string_t pcObjClass, string_t pcObjName, serObjectValue_t *pxObjValue,
                           size_t *pxUnSzObjClass, size_t *pxUnSzObjName)
{
    size_t unSz = 0;

    if (pcObjName != NULL)
        unSz += (*pxUnSzObjName = strlen(pcObjName) + 1);

    if (pcObjClass != NULL)
        unSz += (*pxUnSzObjClass = strlen(pcObjClass) + 1);

    if (pxObjValue != NULL)
        unSz += sizeof(serObjectValue_t) + pxObjValue->xSerLength;

    return unSz;
}

messageCdap_t *pxRibdCdapMsgCreateResponse(Ribd_t *pxRibd,
                                           string_t pcObjClass,
                                           string_t pcObjName,
                                           long nObjInst,
                                           opCode_t eOpCode,
                                           serObjectValue_t *pxObjValue,
                                           int nResultCode,
                                           string_t pcResultReason,
                                           int nInvokeId)
{
    messageCdap_t *pxMsg;
    size_t unSz, unSzObjName = 0, unSzObjClass = 0, unSzResultReason = 0;
    void *px;

    /* Check how much memory we'll need after the structure */
    unSz = sizeof(messageCdap_t);

    unSz += prvCountCommonItems(pcObjClass, pcObjName, pxObjValue, &unSzObjClass, &unSzObjName);

    if (pcResultReason != NULL)
        unSz += (unSzResultReason = strlen(pcResultReason) + 1);

    /* Allocate the memory we need */

    if (!(pxMsg = pxRibdCdapMsgCreate(pxRibd, sizeof(messageCdap_t) + unSz)))
        return NULL;

    pxMsg->invokeID = get_next_invoke_id();
    pxMsg->eOpCode = eOpCode;
    pxMsg->objInst = nObjInst;

    if (unSz > 0) {
        prvCopyCommonStrings(pxMsg, pcObjClass, pcObjName, pxObjValue, unSzObjClass, unSzObjName, &px);

        if (pcResultReason != NULL) {
            pxMsg->pcResultReason = px;
            strcpy(pxMsg->pcResultReason, pcResultReason);
        }
    }

    return pxMsg;
}

messageCdap_t *pxRibdCdapMsgCreateRequest(Ribd_t *pxRibd,
                                          string_t pcObjClass,
                                          string_t pcObjName,
                                          long objInst,
                                          opCode_t eOpCode,
                                          serObjectValue_t *pxObjValue)
{
    messageCdap_t *pxMsg;
    size_t unSz, unSzObjName = 0, unSzObjClass = 0;

    /* Check how much memory we'll need after the structure */
    unSz = sizeof(messageCdap_t);

    unSz += prvCountCommonItems(pcObjClass, pcObjName, pxObjValue, &unSzObjClass, &unSzObjName);

    if (pxObjValue != NULL)
        unSz += sizeof(serObjectValue_t) + pxObjValue->xSerLength;

    /* Allocate the memory we need */

    if (!(pxMsg = pxRibdCdapMsgCreate(pxRibd, unSz)))
        return NULL;

    pxMsg->invokeID = get_next_invoke_id();
    pxMsg->eOpCode = eOpCode;
    pxMsg->objInst = objInst;

    /* Initialize the strings and pointers */

    if (unSz > 0)
        prvCopyCommonStrings(pxMsg, pcObjClass, pcObjName, pxObjValue, unSzObjClass, unSzObjName, NULL);

    return pxMsg;
}

messageCdap_t *pxRibdCdapMsgCreate(Ribd_t *pxRibd, size_t unSz)
{
    messageCdap_t *pxMsg = NULL;
    authPolicy_t *pxAuthPolicy;

    if (!(pxMsg = pxRsrcVarAlloc(pxRibd->xMsgPool, "CDAP Message", unSz))) {
        LOGE(TAG_RIB, "Failed to allocate memory for CDAP message");
        return NULL;
    }

    bzero(pxMsg, unSz);

    /* Init to Default Values*/

    pxMsg->version = 0x01;
    pxMsg->eOpCode = -1; // No code
    pxMsg->invokeID = 1; // by default
    pxMsg->objInst = -1;
    pxMsg->pcObjClass = NULL;
    pxMsg->pcObjName = NULL;
    pxMsg->pxObjValue = NULL;
    pxMsg->result = 0;

    if (!xNameAssignFromPartsDup(&pxMsg->xDestinationInfo, "", MANAGEMENT_AE, "", ""))
        goto fail;

    if (!xNameAssignFromPartsDup(&pxMsg->xSourceInfo, "", MANAGEMENT_AE, "", ""))
        goto fail;

    pxMsg->xAuthPolicy.pcName = NULL;
    pxMsg->xAuthPolicy.pcVersion = NULL;

    return pxMsg;

    fail:
    vNameFree(&pxMsg->xDestinationInfo);
    vNameFree(&pxMsg->xSourceInfo);
    vRsrcFree(pxMsg);

    return NULL;
}

void vRibdCdapMsgFree(messageCdap_t *pxMsg)
{
    vNameFree(&pxMsg->xDestinationInfo);
    vNameFree(&pxMsg->xSourceInfo);

    vRsrcFree(pxMsg);
}
