#ifndef RSTR_H__INCLUDED
#define RSTR_H__INCLUDED

name_t *pxRstrNameDup(const name_t *pxSrc);
void vRstrNameDestroy(name_t *pxName);

BaseType_t xRstringDup(const string_t *pxSrc, string_t **pxDst);


#endif