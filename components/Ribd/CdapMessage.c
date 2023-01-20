#include "portability/port.h"

#include "common/error.h"
#include "common/rina_name.h"

#include "configRINA.h"

#include "CdapMessage.h"
#include "Ribd_defs.h"
#include "Ribd_msg.h"
#include "SerDes.h"

char *opcodeNamesTable[] = {
    [M_CONNECT] = "M_CONNECT",
    [M_CONNECT_R] = "M_CONNECT_R",
    [M_RELEASE] = "M_RELEASE",
    [M_RELEASE_R] = "M_RELEASE_R",
    [M_CREATE] = "M_CREATE",
    [M_CREATE_R] = "M_CREATE_R",
    [M_DELETE] = "M_DELETE",
    [M_DELETE_R] = "M_DELETE_R",
    [M_READ] = "M_READ",
    [M_READ_R] = "M_READ_R",
    [M_CANCELREAD] = "M_CANCELREAD",
    [M_CANCELREAD_R] = "M_CANCELREAD_R",
    [M_WRITE] = "M_WRITE",
    [M_WRITE_R] = "M_WRITE_R",
    [M_START] = "M_START",
    [M_START_R] = "M_START_R",
    [M_STOP] = "M_STOP",
    [M_STOP_R] = "M_STOP_R"};

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
        pxMsg->pcObjName = px;
        strcpy(pxMsg->pcObjName, pcObjName);
        px += unSzObjName;
    }

    if (pcObjClass != NULL) {
        pxMsg->pcObjClass = px;
        strcpy(pxMsg->pcObjClass, pcObjClass);
        px += unSzObjClass;
    }

    if (pxNext) *pxNext = px;
}

size_t prvCountCommonItems(string_t pcObjClass, string_t pcObjName, serObjectValue_t *pxObjValue,
                           size_t *pxUnSzObjClass, size_t *pxUnSzObjName)
{
    size_t unSz = 0;

    if (pcObjName != NULL)
        unSz += (*pxUnSzObjName = (strlen(pcObjName) + 1));

    if (pcObjClass != NULL)
        unSz += (*pxUnSzObjClass = (strlen(pcObjClass) + 1));

    if (pxObjValue != NULL)
        unSz += sizeof(serObjectValue_t) + pxObjValue->xSerLength;

    return unSz;
}

messageCdap_t *pxRibCdapMsgCreateResponse(Ribd_t *pxRibd,
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

    if (!(pxMsg = pxRibCdapMsgCreate(pxRibd, sizeof(messageCdap_t) + unSz)))
        return ERR_SET_OOM_NULL;

    pxMsg->nInvokeID = nInvokeId;
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

messageCdap_t *pxRibCdapMsgCreateRequest(Ribd_t *pxRibd,
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

    if (!(pxMsg = pxRibCdapMsgCreate(pxRibd, unSz)))
        return NULL;

    pxMsg->nInvokeID = get_next_invoke_id();
    pxMsg->eOpCode = eOpCode;
    pxMsg->objInst = objInst;

    /* Initialize the strings and pointers */

    if (unSz > 0)
        prvCopyCommonStrings(pxMsg, pcObjClass, pcObjName, pxObjValue, unSzObjClass, unSzObjName, NULL);

    return pxMsg;
}

messageCdap_t *pxRibCdapMsgCreate(Ribd_t *pxRibd, size_t unSz)
{
    messageCdap_t *pxMsg = NULL;

    if (!(pxMsg = pxRsrcVarAlloc(pxRibd->xMsgPool, "CDAP Message", unSz))) {
        LOGE(TAG_RIB, "Failed to allocate memory for CDAP message");
        return NULL;
    }

    bzero(pxMsg, unSz);

    /* Init to Default Values*/

    pxMsg->version = 0x01;
    pxMsg->eOpCode = -1; // No code
    pxMsg->nInvokeID = 1; // by default
    pxMsg->objInst = -1;
    pxMsg->pcObjClass = NULL;
    pxMsg->pcObjName = NULL;
    pxMsg->pxObjValue = NULL;
    pxMsg->result = 0;

    if (ERR_CHK(xNameAssignFromPartsDup(&pxMsg->xDestinationInfo, "", CFG_MANAGEMENT_AE, "", "")))
        goto fail;

    if (ERR_CHK(xNameAssignFromPartsDup(&pxMsg->xSourceInfo, "", CFG_MANAGEMENT_AE, "", "")))
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

void vRibCdapMsgFree(messageCdap_t *pxMsg)
{
    vNameFree(&pxMsg->xDestinationInfo);
    vNameFree(&pxMsg->xSourceInfo);

    vRsrcFree(pxMsg);
}
