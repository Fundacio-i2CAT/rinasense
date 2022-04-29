
#include "freertos/FreeRTOS.h"

#include "efcpStructures.h"

connection_t * pxConnectionCreate(void)
{
        connection_t * pxTmp;

        pxTmp = pvPortMalloc(sizeof(*pxTmp));
        if (!pxTmp)
                return NULL;

        return pxTmp;
}




BaseType_t xConnectionDestroy(connection_t * pxConn)
{
        if (!pxConn)
                return pdFALSE;
        vPortFree(pxConn);
        return pdTRUE;
}