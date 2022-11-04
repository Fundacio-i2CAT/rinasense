#ifndef COMMON_RINA_NAME_H
#define COMMON_RINA_NAME_H

#include "portability/port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xName_info
{
    size_t unPostLn;

    string_t pcProcessName;
    string_t pcProcessInstance;
    string_t pcEntityName;
    string_t pcEntityInstance;
} rname_t;

rname_t *pxNameNewFromString(const string_t pcNmStr);

rname_t *pxNameNewFromParts(string_t pcProcessName,
                            string_t pcProcessInstance,
                            string_t pcEntityName,
                            string_t pcEntityInstance);

void vNameFree(rname_t *pxNm);

void vNameAssignFromPartsStatic(rname_t *pxDst,
                                string_t pcProcessName,
                                string_t pcProcessInstance,
                                string_t pcEntityName,
                                string_t pcEntityInstance);

bool_t xNameAssignFromPartsDup(rname_t *pxDst,
                               string_t pcProcessName,
                               string_t pcProcessInstance,
                               string_t pcEntityName,
                               string_t pcEntityInstance);

bool_t xNameAssignFromString(rname_t *pxDst, const string_t pxNmStr);

void vNameAssignStatic(rname_t *pxDst, const rname_t *pxSrc);

bool_t xNameAssignDup(rname_t *pxDst, const rname_t *pxSrc);

string_t pcNameToString(const rname_t *pxDst);

void vNameToStringBuf(const rname_t *pxDst, string_t pcBuf, size_t unSzBuf);

#ifdef __cplusplus
}
#endif

#endif // COMMON_RINA_NAME_H
