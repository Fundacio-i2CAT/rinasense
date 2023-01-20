#include "efcpStructures.h"

connection_t * pxConnectionCreate(void)
{
    connection_t * pxTmp;

    pxTmp = pvRsMemAlloc(sizeof(*pxTmp));
    if (!pxTmp)
        return NULL;

    return pxTmp;
}

bool_t xConnectionDestroy(connection_t * pxConn)
{
    if (!pxConn)
        return false;
    vRsMemFree(pxConn);
    return true;
}
