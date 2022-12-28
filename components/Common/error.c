#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "portability/port.h"

#include "common/error.h"
#include "common/rsrc.h"
#include "common/macros.h"

#define TAG_ERR "[Error]"

rsrcPoolP_t xErrPool = NULL;

pthread_mutex_t xPoolMutex = PTHREAD_MUTEX_INITIALIZER;

/* Initialize the memory pool if needed. */
static void prvErrorCheckPool()
{
    if (xErrPool) return;

    if (!(xErrPool = pxRsrcNewPool("Thread Error Pool", sizeof(rsErrInfo_t), 1, 1, 0))) {
        LOGE(TAG_ERR, "Out of memory for managing errors");
        abort();
    }
}

static rsErrInfo_t *prvErrorSetNew(string_t pcFile, uint32_t unLine, rsErr_t unErrCode, string_t pcMsg,
                                   rsErrInfo_t *pxNext)
{
    rsErrInfo_t *pxErrInfo;

    if (!(pxErrInfo = pxRsrcAlloc(xErrPool, "Thread Error Info"))) {
        LOGE(TAG_ERR, "Out of memory for managing errors");
        abort();
    }

    pxErrInfo->unLine = unLine;
    pxErrInfo->unErrCode = unErrCode;
    pxErrInfo->pxNext = pxNext;

    if (pcFile)
        strncpy(pxErrInfo->pcFile, pcFile, sizeof(pxErrInfo->pcFile));

    _errSetPtr(pxErrInfo);

    return pxErrInfo;
}

static void prvErrorSetNew_s(string_t pcFile, uint32_t unLine, rsErr_t unErrCode, string_t pcMsg,
                             rsErrInfo_t *pxNext)
{
    rsErrInfo_t *pxErrInfo;

    pxErrInfo = prvErrorSetNew(pcFile, unLine, unErrCode, pcMsg, pxNext);

    if (pcMsg)
        strncpy(pxErrInfo->pcErrMsg, pcMsg, sizeof(pxErrInfo->pcErrMsg));
}

static void prvErrorSetNew_v(string_t pcFile, uint32_t unLine, rsErr_t unErrCode, string_t pcFmt,
                             rsErrInfo_t *pxNext, va_list xParams)
{
    rsErrInfo_t *pxErrInfo;

    pxErrInfo = prvErrorSetNew(pcFile, unLine, unErrCode, pcFmt, pxNext);

    if (pcFmt)
        vsnprintf(pxErrInfo->pcErrMsg, member_size(rsErrInfo_t, pcErrMsg), pcFmt, xParams);
}

static void prvErrorClearLocked()
{
    rsErrInfo_t *px, *pxNext;

    px = _errGetPtr();
    while (px != NULL) {
        pxNext = px->pxNext;
        vRsrcFree(px);
        px = pxNext;
    }
}

void vErrorClear() {
    pthread_mutex_lock(&xPoolMutex);

    prvErrorClearLocked();

    pthread_mutex_unlock(&xPoolMutex);
}

rsErr_t xErrorPush(string_t pcFile, uint32_t unLine, rsErr_t unErrCode, string_t pcMsg)
{
    pthread_mutex_lock(&xPoolMutex);

    prvErrorCheckPool();
    prvErrorSetNew_s(pcFile, unLine, unErrCode, pcMsg, _errGetPtr());

    pthread_mutex_unlock(&xPoolMutex);

    return unErrCode;
}

rsErr_t xErrorSet(string_t pcFile, uint32_t unLine, rsErr_t unErrCode, string_t pcMsg)
{
    pthread_mutex_lock(&xPoolMutex);

    /* Clear the error if there is one. */
    if (_errGetPtr())
        prvErrorClearLocked();

    prvErrorCheckPool();
    prvErrorSetNew_s(pcFile, unLine, unErrCode, pcMsg, NULL);

    pthread_mutex_unlock(&xPoolMutex);

    return unErrCode;
}

rsErr_t xErrorSetf(string_t pcFile, uint32_t unLine, rsErr_t unErrCode, const string_t pcFmt, ...)
{
    va_list ap;

    pthread_mutex_lock(&xPoolMutex);

    /* Clear the error if there is one. */
    if (_errGetPtr())
        prvErrorClearLocked();

    prvErrorCheckPool();

    va_start(ap, pcFmt);
    prvErrorSetNew_v(pcFile, unLine, unErrCode, pcFmt, NULL, ap);
    va_end(ap);

    pthread_mutex_unlock(&xPoolMutex);

    return unErrCode;
}

rsErrInfo_t *xErrorGet()
{
    return _errGetPtr();
}

string_t xErrorGetMsg(rsErr_t xErr)
{
    uint32_t unLevel1, unLevel2;

    unLevel1 = xErr.c >> 24;
    unLevel2 = xErr.c & 0xFF;

    return ErrorMessages[unLevel1][unLevel2 - 1];
}

void vErrorLog(const string_t pcTag, const string_t pcTitle)
{
    rsErrInfo_t *pxErrInfo;

    LOGE(pcTag, "---- START: %s ----", pcTitle);

    /* Weird situation here */
    if (!(pxErrInfo = xErrorGet())) {
        LOGE(pcTag, "ERROR LOG CALLED BUT NO ERROR SET");
        return;
    }

    while (pxErrInfo) {
        LOGE(pcTag, "Error code: %xd", pxErrInfo->unErrCode.c);
        LOGE(pcTag, "File: %s:%d", pxErrInfo->pcFile, pxErrInfo->unLine);
        LOGE(pcTag, "Message: %s", pxErrInfo->pcErrMsg);

        pxErrInfo = pxErrInfo->pxNext;

        /* FIXME: Not too sure this is very pretty ... */
        if (pxErrInfo)
            LOGE(pcTag, "");
    }

    LOGE(pcTag, "---- END: %s ----", pcTitle);
}
