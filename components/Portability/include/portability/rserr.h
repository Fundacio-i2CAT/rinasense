#ifndef _PORTABILITY_RSERR_H_INCLUDED
#define _PORTABILITY_RSERR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "common/error_defs.h"

void _errSetPtr(struct xERR_INFO *pxErr);

struct xERR_INFO *_errGetPtr();

#ifdef __cplusplus
}
#endif

#endif /* _PORTABILITY_RSERR_H_INCLUDED */
