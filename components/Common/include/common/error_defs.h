#ifndef _COMMON_ERROR_DEFS_INCLUDED_H
#define _COMMON_ERROR_DEFS_INCLUDED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Generic error type. */
typedef struct xRS_ERR { uint32_t c; } rsErr_t;

/* Functions returning 'memErr_t' are expected to only fail in case of
 * memory problems */
typedef rsErr_t rsMemErr_t;

struct xERR_INFO;

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_ERROR_DEFS_INCLUDED_H */
