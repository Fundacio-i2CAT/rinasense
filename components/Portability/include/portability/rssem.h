#ifndef _PORTABILITY_RSSEM_H_INCLUDED
#define _PORTABILITY_RSSEM_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "port_specifics.h"

#include <sys/types.h>
#include <stdint.h>

#include "common/error_defs.h"

/* Basic portable semaphore API based on what's defined in POSIX's semaphore.h */

rsErr_t xRsSemInit(RsSem_t *pxSem, size_t unSz);

rsErr_t xRsSemPost(RsSem_t *pxSem);

rsErr_t xRsSemWait(RsSem_t *pxSem);

rsErr_t xRsSemTimedWait(RsSem_t *pxSem, useconds_t unTimeout);

void vRsSemDebug(const char *pcTag, const char *pcName, RsSem_t *pxSem);

#ifdef __cplusplus
}
#endif

#endif /* _PORTABILITY_RSSEM_H_INCLUDED */
