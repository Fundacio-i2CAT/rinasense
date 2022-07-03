

#include "efcpStructures.h"

struct connection_t * pxConnectionCreate(void)
{
        struct connection_t * pxTmp;

        pxTmp = pvRsMemAlloc(sizeof(*pxTmp));
        if (!pxTmp)
                return NULL;

        return pxTmp;
}




bool_t xConnectionDestroy(struct connection_t * pxConn)
{
        if (!pxConn)
                return false;
        vRsMemFree(pxConn);
        return true;
}
