#ifndef RSTR_H__INCLUDED
#define RSTR_H__INCLUDED

void vRstrNameDestroy(name_t *pxName);
name_t *pxRstrNameDup(const name_t *pxSrc);
BaseType_t xRstringDup(const string_t *pxSrc, string_t **pxDst);
name_t *pxRStrNameCreate(void);
BaseType_t xRinaNameFromString(const string_t pcString, name_t *pxName);
void vRstrNameFree(name_t *xName);

#endif