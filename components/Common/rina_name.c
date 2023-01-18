#include <stdio.h>
#include <string.h>

#include "portability/port.h"

#include "common/error.h"
#include "common/rina_name.h"

#include "configRINA.h"

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

    for (c = pxStr; *c != CFG_DELIMITER && *c != 0; c++);
    if (*c == 0) return false; /* Bad string */
    if (*c == CFG_DELIMITER) *c = 0;

    pxDst->pcProcessInstance = c + 1;

    for (c++; *c != CFG_DELIMITER && *c != 0; c++);
    if (*c == 0) return false; /* Bad string */
    if (*c == CFG_DELIMITER) *c = 0;

    pxDst->pcEntityName = c + 1;

    for (c++; *c != CFG_DELIMITER && *c != 0; c++);

    /* We're taking into account the possibility of a missing last
     * delimiter here. If the last character we went over isn't the
     * delimiter, we presume the string continues past it. Otherwise,
     * we just point EntityInstance at the 0. */
    if (*c == CFG_DELIMITER) {
        *c = 0;
        pxDst->pcEntityInstance = c + 1;
    }
    else pxDst->pcEntityInstance = c;

    return true;
}

/* *** */

rname_t *pxNameNewFromString(const string_t pcNmStr)
{
    size_t len;
    rname_t *pxDst;
    string_t pxData;

    len = strlen(pcNmStr);

    /* Allocate enough memory for the whole string past the
       structure */
    if (!(pxDst = pvRsMemAlloc(sizeof(rname_t) + len + 1)))
        return ERR_SET_OOM_NULL;

    /* Keep track of how much memory we have past it */
    pxDst->unPostLn = len;

    /* Copy the string after the struct */
    pxData = (void *)pxDst + sizeof(rname_t);
    pxDst->pxFree = pxDst;
    strcpy(pxData, pcNmStr);

    if (!prvBreakdownNameString(pxDst, pxData)) {
        /* Error? Entirely wipe the name_t object to make sure it's
         * not usable. */
        bzero(pxDst, sizeof(rname_t));
        vRsMemFree(pxDst);
        return ERR_SET_OOM_NULL;
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

    /* Figure out the memory we need past the structure size */
    len = strlen(pcProcessName)
        + strlen(pcProcessInstance)
        + strlen(pcEntityName)
        + strlen(pcEntityInstance);

    /* Allocate the structure + memory past it */
    if (!(pxDst = pvRsMemAlloc(sizeof(rname_t) + len + 4)))
        return ERR_SET_OOM_NULL;

    /* Keep track of how much memory we have past it */
    pxDst->unPostLn = len;

    /* Copy the name components in memory after the struct */
    pxData = (void *)pxDst + sizeof(rname_t);
    pxDst->pxFree = pxDst;

    prvNameCopyAssignParts(pxDst, pxData,
                           pcProcessName, pcProcessInstance,
                           pcEntityName, pcEntityInstance);

    return pxDst;
}

void vNameFree(rname_t *pxNm)
{
    if (pxNm->pxFree)
        vRsMemFree(pxNm->pxFree);
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
    pxDst->pxFree = NULL;

    /* No memory reserved after the struct */
    pxDst->unPostLn = 0;
}

rsMemErr_t xNameAssignFromPartsDup(rname_t *pxDst,
                                   string_t pcProcessName,
                                   string_t pcProcessInstance,
                                   string_t pcEntityName,
                                   string_t pcEntityInstance)
{
    size_t len;

    /* 1 char for the '0' */
    len = (pcProcessName     ? strlen(pcProcessName)     : 1)
        + (pcProcessInstance ? strlen(pcProcessInstance) : 1)
        + (pcEntityName      ? strlen(pcEntityName)      : 1)
        + (pcEntityInstance  ? strlen(pcEntityInstance)  : 1);

    if (!(pxDst->pxFree = pvRsMemAlloc(len + 4 + 1)))
        return ERR_SET_OOM;

    prvNameCopyAssignParts(pxDst, pxDst->pxFree,
                           (pcProcessName     ? pcProcessName     : ""),
                           (pcProcessInstance ? pcProcessInstance : ""),
                           (pcEntityName      ? pcEntityName      : ""),
                           (pcEntityInstance  ? pcEntityInstance  : ""));

    /* No memory reserved after the struct */
    pxDst->unPostLn = 0;

    return SUCCESS;
}

rsErr_t xNameAssignFromString(rname_t *pxDst, const string_t pxNmStr)
{
    string_t pxNewStr;
    size_t unStrSz;
    char *c;

    /* Get the amount of memory we need */
    unStrSz = strlen(pxNmStr);

    /* Allocate enough memory for the string */
    if (!(pxDst->pxFree = pvRsMemAlloc(unStrSz + 1)))
        return ERR_SET_OOM;

    /* Copy */
    strcpy(pxDst->pxFree, pxNmStr);

    /* Nothing is allocate past the structure size */
    pxDst->unPostLn = 0;

    if (!prvBreakdownNameString(pxDst, pxDst->pxFree)) {
        /* Error? Entirely wipe the name_t object to make sure it's
         * not usable. */
        bzero(pxDst, sizeof(rname_t));
        vRsMemFree(pxDst->pxFree);
        return ERR_SET_OOM;
    }
    else return SUCCESS;
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
rsMemErr_t xNameAssignDup(rname_t *pxDst, const rname_t *pxSrc)
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

    /* Keep a place for the final NUL */
    unSzMax = unSzBuf - 1;

    px = pcBuf;

    for (int i = 0; i < 4; i++) {
        if ((unSzSrc = strlen(strings[i])) < unSzMax)
            unSzCopy = unSzSrc;
        else
            unSzCopy = unSzMax;

        strncpy(px, strings[i], unSzCopy);
        *(px + unSzCopy) = 0;
        px += unSzCopy + 1;

        if (i < 3)
            *(px - 1) = CFG_DELIMITER;

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
        unDestLn += 4; /* 3 '/' characters ++ NUL */

        if (!(pcDest = pvRsMemAlloc(unDestLn)))
            return ERR_SET_OOM_NULL;

        vNameToStringBuf(pxDst, pcDest, unDestLn);
    }
    /* If there are some bytes after the structure, we already have
       the length we need and the strings are supposed to be all one
       after each other after the structure. */
    else {
        unDestLn = pxDst->unPostLn + 1;

        if (!(pcDest = pvRsMemAlloc(unDestLn)))
            return ERR_SET_OOM_NULL;

        memcpy(pcDest, pxDst->pcProcessName, unDestLn);

        for (c = pcDest; c < pcDest + unDestLn - 1; c++)
            if (*c == 0) *c = CFG_DELIMITER;
    }

    return pcDest;
}
