#ifndef COMMON_RINA_NAME_H
#define COMMON_RINA_NAME_H

#include "portability/port.h"

typedef struct xName_info
{
	char *pcProcessName;  		/*> Process Name*/
	char *pcProcessInstance;		/*> Process Instance*/
	char *pcEntityName;			/*> Entity Name*/
	char *pcEntityInstance;		/*> Entity Instance*/
} name_t;

name_t *xRINANameInitFrom(name_t *pxDst,
                          const char *process_name,
                          const char *process_instance,
                          const char *entity_name,
                          const char *entity_instance);

void vRstrNameFini(name_t *n);

name_t *pxRStrNameCreate(void);

name_t *pxRstrNameDup(const name_t *pxSrc);

void vRstrNameDestroy(name_t *pxName);

bool_t xRstringDup(const char *pxSrc, string_t *pxDst);

name_t *xRinaNameCreate(void);

bool_t xRinaNameFromString(const char *pcString, name_t * xName);

void xRinaNameFree(name_t *xName);

bool_t xRINAStringDup(const char *pcSrc, char *pcDst);

name_t *xRINAstringToName(const char *pxInput);

string_t pcNameToString(const name_t *n);

#endif // COMMON_RINA_NAME_H
