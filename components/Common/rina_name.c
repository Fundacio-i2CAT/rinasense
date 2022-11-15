#include <stdio.h>
#include <string.h>

#include "portability/port.h"

#include "common/rina_name.h"

#define DELIMITER '|'

void prvNameCopyAssignParts(rname_t *pxDst,
                            string_t pxData,
                            string_t pcProcessName,
                            string_t pcProcessInstance,
                            string_t pcEntityName,
                            string_t pcEntityInstance)
{
    string_t px = pxData;

    pxDst->pcProcessName = px;
    while ((*px++ = *pcProcessName++));
    pxDst->pcProcessInstance = px;
    while ((*px++ = *pcProcessInstance++));
    pxDst->pcEntityName = px;
    while ((*px++ = *pcEntityName++));
    pxDst->pcEntityInstance = px;
    while ((*px++ = *pcEntityInstance++));
}

bool_t prvBreakdownNameString(rname_t *pxDst, string_t pxStr)
{
    char *c;

    pxDst->pcProcessName = pxStr;

    for (c = pxStr; *c != DELIMITER && *c != 0; c++);
    if (*c == 0) return false; /* Bad string */
    if (*c == '|') *c = 0;

    pxDst->pcProcessInstance = c + 1;

    for (c++; *c != DELIMITER && *c != 0; c++);
    if (*c == 0) return false; /* Bad string */
    if (*c == '|') *c = 0;

    pxDst->pcEntityName = c + 1;

    for (c++; *c != DELIMITER && *c != 0; c++);
    if (*c == '|') *c = 0;

    pxDst->pcEntityInstance = c + 1;

    return true;
}

/* *** */

rname_t *pxNameNewFromString(const string_t pcNmStr)
{
    size_t len;
    rname_t *pxDst;
    string_t pxData;

    len = strlen(pcNmStr);

    if (!(pxDst = pvRsMemAlloc(sizeof(rname_t) + len + 1)))
        return NULL;

    pxDst->unPostLn = len;
    pxData = (void *)pxDst + sizeof(rname_t);
    strcpy(pxData, pcNmStr);

    if (!prvBreakdownNameString(pxDst, pxData)) {
        vRsMemFree(pxDst);
        return NULL;
    }
    else return pxDst;
}

rname_t *pxNameNewFromParts(string_t pcProcessName,
                            string_t pcProcessInstance,
                            string_t pcEntityName,
                            string_t pcEntityInstance)
{
	size_t len;
	string_t pxData;
    rname_t *pxDst;

    len = strlen(pcProcessName)
        + strlen(pcProcessInstance)
        + strlen(pcEntityName)
        + strlen(pcEntityInstance);

    if (!(pxDst = pvRsMemAlloc(sizeof(rname_t) + len + 4)))
        return false;

    pxDst->unPostLn = len;

    /* Copy the name components in memory after the struct */
    pxData = (void *)pxDst + sizeof(rname_t);

    prvNameCopyAssignParts(pxDst, pxData,
                           pcProcessName, pcProcessInstance,
                           pcEntityName, pcEntityInstance);

    return pxDst;
}

void vNameFree(rname_t *pxNm)
{
    /* If there is no memory reserved after the struct, free the first
       component. This will take care of the rest. Otherwise, free the
       whole struct. */

    if (!pxNm->unPostLn)
        vRsMemFree(pxNm->pcProcessName);
    else
        vRsMemFree(pxNm);
}

void vNameAssignFromPartsStatic(rname_t *pxDst,
                                string_t pcProcessName,
                                string_t pcProcessInstance,
                                string_t pcEntityName,
                                string_t pcEntityInstance)
{
    pxDst->pcProcessName = pcProcessName;
    pxDst->pcProcessInstance = pcProcessInstance;
    pxDst->pcEntityName = pcEntityName;
    pxDst->pcEntityInstance = pcEntityInstance;

    /* No memory reserved after the struct */
    pxDst->unPostLn = 0;
}

bool_t xNameAssignFromPartsDup(rname_t *pxDst,
                               string_t pcProcessName,
                               string_t pcProcessInstance,
                               string_t pcEntityName,
                               string_t pcEntityInstance)
{
    size_t len;
    string_t newq;

    /* 1 char for the '0' */
    len = (pcProcessName     ? strlen(pcProcessName)     : 1)
        + (pcProcessInstance ? strlen(pcProcessInstance) : 1)
        + (pcEntityName      ? strlen(pcEntityName)      : 1)
        + (pcEntityInstance  ? strlen(pcEntityInstance)  : 1);

    if (!(newq = pvRsMemAlloc(len + 4 + 1)))
        return false;

    prvNameCopyAssignParts(pxDst, newq,
                           (pcProcessName     ? pcProcessName     : ""),
                           (pcProcessInstance ? pcProcessInstance : ""),
                           (pcEntityName      ? pcEntityName      : ""),
                           (pcEntityInstance  ? pcEntityInstance  : ""));

    /* No memory reserved after the struct */
    pxDst->unPostLn = 0;

    return true;
}

bool_t xNameAssignFromString(rname_t *pxDst, const string_t pxNmStr)
{
    string_t pxNewStr;
    size_t unStrSz;
    char *c;

    /* Copy the string */
    unStrSz = strlen(pxNmStr);
    pxNewStr = pvRsMemAlloc(unStrSz + 1);
    strcpy(pxNewStr, pxNmStr);

    pxDst->unPostLn = 0;

    if (!prvBreakdownNameString(pxDst, pxNewStr)) {
        /* Error? Entirely wipe the name_t object to make sure it's
         * not usable. */

        bzero(pxDst, sizeof(rname_t));
        vRsMemFree(pxNewStr);
        return false;
    }
    else return true;
}

/**
 * Make pxDst the same as pxSrc, copying only components pointers
 */
void vNameAssignStatic(rname_t *pxDst, const rname_t *pxSrc)
{
    vNameAssignFromPartsStatic(pxDst,
                               pxSrc->pcProcessName, pxSrc->pcProcessInstance,
                               pxSrc->pcEntityName, pxSrc->pcProcessInstance);
}

/**
 * Make pxDst the same as pxSrc, copying pxSrc content.
 */
bool_t xNameAssignDup(rname_t *pxDst, const rname_t *pxSrc)
{
    return xNameAssignFromPartsDup(pxDst,
                                   pxSrc->pcProcessName, pxSrc->pcProcessInstance,
                                   pxSrc->pcEntityName, pxSrc->pcProcessInstance);
}

void vNameToStringBuf(const rname_t *pxDst, string_t pcBuf, size_t unSzBuf)
{
    char *strings[] = {
        pxDst->pcProcessName,
        pxDst->pcProcessInstance,
        pxDst->pcEntityName,
        pxDst->pcEntityInstance
    };
    size_t unSzCopy, unSzSrc, unSzMax;
    char *px;

    unSzMax = unSzBuf - 1;
    px = pcBuf;

    for (int i = 0; i < 4; i++) {
        if ((unSzSrc = strlen(strings[i])) < unSzMax)
            unSzCopy = unSzSrc;
        else
            unSzCopy = unSzMax;

        strncpy(px, strings[i], unSzCopy);
        px += unSzCopy + 1;

        if (i < 3)
            *(px - 1) = '|';

        unSzMax -= unSzCopy;

        if (unSzMax == 0) {
            pcBuf[unSzBuf - 1] = 0;
            break;
        }
    }
}

string_t pcNameToString(const rname_t *pxDst)
{
    string_t pcDest, c;
    size_t unDestLn;

    /* If there is nothing allocated after the structure in memory,
       calculate the length of each string components. */
    if (!pxDst->unPostLn) {
        unDestLn = 0;

        unDestLn = strlen(pxDst->pcProcessName);
        unDestLn += strlen(pxDst->pcProcessInstance);
        unDestLn += strlen(pxDst->pcEntityName);
        unDestLn += strlen(pxDst->pcEntityInstance);
        unDestLn += 4; /* Null bytes */

        if (!(pcDest = pvRsMemAlloc(unDestLn)))
            return NULL;

        vNameToStringBuf(pxDst, pcDest, unDestLn);
    }
    /* If there are some bytes after the structure, we already have
       the length we need and the strings are supposed to be all one
       after each other after the structure. */
    else {
        unDestLn = pxDst->unPostLn;

        if (!(pcDest = pvRsMemAlloc(unDestLn)))
            return NULL;

        memcpy(pcDest, pxDst->pcProcessName, unDestLn);

        for (c = pcDest; c < pcDest + unDestLn; c++)
            if (*c == 0) *c = '|';
    }

    return pcDest;
}
