#ifndef COMMON_RINA_NAME_H
#define COMMON_RINA_NAME_H

#include "portability/port.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set all the component of a name from static string. Pay attention
 * not to free a name that is set that way.
 */
#define NAME_SET(nm, pe, pi, en, ei)             \
    (*nm).pcProcessName = pe;                    \
    (*nm).pcProcessInstance = pi;                \
    (*nm).pcEntityName = en;                     \
    (*nm).pcEntityInstance = ei;

typedef struct xName_info
{
	string_t pcProcessName;  		/*> Process Name*/
	string_t pcProcessInstance;		/*> Process Instance*/
	string_t pcEntityName;			/*> Entity Name*/
	string_t pcEntityInstance;		/*> Entity Instance*/
} name_t;

name_t *xRINANameInitFrom(name_t *pxDst,
                          const string_t process_name,
                          const string_t process_instance,
                          const string_t entity_name,
                          const string_t entity_instance);

void vRstrNameFini(name_t *n);

void vRstrNameFree(name_t *xName);

name_t *pxRStrNameCreate(void);

name_t *pxRstrNameDup(const name_t *pxSrc);

void vRstrNameDestroy(name_t *pxName);

bool_t xRstringDup(const string_t pxSrc, string_t *pxDst);

name_t *xRinaNameCreate(void);

bool_t xRinaNameFromString(const string_t pcString, name_t * xName);

void xRinaNameFree(name_t *xName);

bool_t xRINAStringDup(const char *pcSrc, char *pcDst);

name_t *xRINAstringToName(const string_t pxInput);

string_t pcNameToString(const name_t *n);

void pcNameToStrBuf(const name_t *pxName, stringbuf_t *pcBuf, size_t unBufSz);

#ifdef __cplusplus
}
#endif

#endif // COMMON_RINA_NAME_H
