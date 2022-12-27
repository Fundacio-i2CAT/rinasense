#include <string.h>

#include "portability/port.h"

#include "common/error.h"
#include "common/rsrc.h"

__thread rsErrInfo_t *pxErrInfo = NULL;

void _errSetPtr(rsErrInfo_t *pxErr)
{
    pxErrInfo = pxErr;
}

rsErrInfo_t *_errGetPtr()
{
    return pxErrInfo;
}

