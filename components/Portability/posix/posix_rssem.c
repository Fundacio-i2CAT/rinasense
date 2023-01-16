#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/semaphore.h"
#include "FreeRTOS_POSIX/errno.h"
#else
#include <semaphore.h>
#endif

#include "portability/port.h"

#include "common/rinasense_errors.h"
#include "common/error.h"

rsErr_t xRsSemInit(RsSem_t *pxSem, size_t unNb)
{
    if (sem_init(&pxSem->xSem, 0, unNb) != 0)
        return ERR_SET_ERRNO;

    return SUCCESS;
}

rsErr_t xRsSemPost(RsSem_t *pxSem)
{
    if (sem_post(&pxSem->xSem) != 0)
        return ERR_SET_ERRNO;

    return SUCCESS;
}

rsErr_t xRsSemWait(RsSem_t *pxSem)
{
    if (sem_wait(&pxSem->xSem) != 0)
        return ERR_SET_ERRNO;

    return SUCCESS;
}

rsErr_t xRsSemTimedWait(RsSem_t *pxSem, useconds_t unTimeout)
{
    struct timespec xTs = {0};
    int n;

    if (!rstime_waitusec(&xTs, unTimeout))
        return ERR_SET_ERRNO;

    if ((n = sem_timedwait(&pxSem->xSem, &xTs)) != 0) {
        if (errno == ETIMEDOUT)
            return ERR_SET(ERR_TIMEDOUT);
        else
            return ERR_SET_ERRNO;
    }

    return SUCCESS;
}

void vRsSemDebug(const char *pcTag, const char *pcName, RsSem_t *pxSem)
{
    int n;

    sem_getvalue(&pxSem->xSem, &n);
    LOGD("[debug]", "Semaphore %s, value: %d", pcName, n);
}
