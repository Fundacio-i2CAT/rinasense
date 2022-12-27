#include <pthread.h>

#include "portability/port.h"
#include "common/rina_name.h"

#include "Ribd_connections.h"
#include "Ribd_defs.h"

/* Only call this while holding the RIB mutex */
rsMemErr_t xRibConnectionAdd(Ribd_t *pxRibd, rname_t *pxSrc, rname_t *pxDst, portId_t unPort)
{
    num_t x = 0;
    appConnection_t *pxAppCon;

    RsAssert(pxRibd);
    RsAssert(pxSrc);
    RsAssert(pxDst);

    pthread_mutex_lock(&pxRibd->xMutex);

    for (x = 0; x < APP_CONNECTION_TABLE_SIZE; x++) {
        if (pxRibd->xAppConnections[x].xStatus == CONNECTION_RELEASED) {
            pxAppCon = &pxRibd->xAppConnections[x];

            /* ??? */
            pxAppCon->uCdapVersion = 0x01;
            pxAppCon->uRibVersion = 0x01;

            pxAppCon->unPort = unPort;
            pxAppCon->xStatus = CONNECTION_IN_PROGRESS;

            if (ERR_CHK(xNameAssignDup(&pxAppCon->xSourceInfo, pxSrc)))
                goto fail;

            if (ERR_CHK(xNameAssignDup(&pxAppCon->xDestinationInfo, pxDst)))
                goto fail;

            LOGI(TAG_RIB, "Added connection entry at: %p, port: %d", pxAppCon, unPort);

            pthread_mutex_unlock(&pxRibd->xMutex);
            return SUCCESS;
        }
    }

    return ERR_RIB_TOO_MANY_CONNECTIONS;

    fail:
    pthread_mutex_unlock(&pxRibd->xMutex);

    return FAIL;
}

appConnection_t *pxRibConnectionFind(Ribd_t *pxRibd, portId_t unPort)
{
    num_t x = 0;
    appConnection_t *pxAppConnection;

    RsAssert(pxRibd);

    for (x = 0; x < APP_CONNECTION_TABLE_SIZE; x++) {
        if (pxRibd->xAppConnections[x].xStatus != CONNECTION_RELEASED)
            if (pxRibd->xAppConnections[x].unPort == unPort)
                return &pxRibd->xAppConnections[x];
    }

    return NULL;
}

void vRibConnectionRelease(Ribd_t *pxRibd, appConnection_t *pxAppCon)
{
    vNameFree(&pxAppCon->xSourceInfo);
    vNameFree(&pxAppCon->xDestinationInfo);

    LOGI(TAG_RIB, "Release connection entry at index: %p", pxAppCon);

    memset(pxAppCon, 0, sizeof(appConnection_t));
}
