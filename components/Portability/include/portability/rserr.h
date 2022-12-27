#ifndef _PORTABILITY_RSERR_H_INCLUDED
#define _PORTABILITY_RSERR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "common/error.h"

void _errSetPtr(rsErrInfo_t *pxErr);

rsErrInfo_t *_errGetPtr();

#ifdef __cplusplus
}
#endif

#endif /* _PORTABILITY_RSERR_H_INCLUDED */
