#include <asm-generic/errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>

#include "portability/port.h"

#include "common/rinasense_errors.h"
#include "common/rsrc.h"

#include "common/error.h"
#include "common/list.h"
#include "common/queue.h"

struct queueItem {
    RsListItem_t xListItem;
    uint8_t xItemBuf[];
};

rsErr_t xQueueInit(const char *sQueueName, RsQueue_t *pxQueue, RsQueueParams_t *pxQueueParams)
{
    int n;
    size_t unSz;
    pthread_mutexattr_t xMutexAttr;

    memcpy(&pxQueue->xParams, pxQueueParams, sizeof(RsQueueParams_t));

    vRsListInit(&pxQueue->xQueueList);

    unSz = sizeof(struct queueItem) + pxQueue->xParams.unItemSz;
    pxQueue->xPool = pxRsrcNewPool(sQueueName, unSz, 0, 1, pxQueue->xParams.unMaxItemCount);

    if (!pxQueue->xPool)
        return ERR_SET_OOM;

    vRsrcSetMutex(pxQueue->xPool, &pxQueue->xMutex);

    pthread_mutexattr_init(&xMutexAttr);

    if ((n = pthread_mutexattr_settype(&xMutexAttr, PTHREAD_MUTEX_RECURSIVE)))
        return ERR_SET_PTHREAD(n);

    if ((n = pthread_mutex_init(&pxQueue->xMutex, &xMutexAttr) != 0))
        return ERR_SET_PTHREAD(n);

    if (!pxQueue->xPool)
        return ERR_SET_OOM;

    if (!pxQueue->xParams.xNoBlock) {
        /* This semaphore will count the items in the queue */
        if (sem_init(&pxQueue->xSemItems, 0, 0))
            return ERR_SET_ERRNO;

        /* If the queue has a limited size, prepare a semaphore that will
         * control free places in the queue */
        if (pxQueueParams->unMaxItemCount != 0)
            if (sem_init(&pxQueue->xSemPlaces, 0, pxQueue->xParams.unMaxItemCount) != 0)
                return ERR_SET_ERRNO;
    }

    return SUCCESS;
}

static rsErr_t prvQueueSendToBack(RsQueue_t *pxQueue, void *pxItem, useconds_t pxTimeout)
{
    struct queueItem *pxQueueItem;
    struct timespec xTs = {0};

    if (pxTimeout > 0 && !rstime_waitusec(&xTs, pxTimeout))
        return ERR_SET_ERRNO;

    if (pxQueue->xParams.unMaxItemCount > 0) {

        /* Wait for a free space if needed */
        if (!pxQueue->xParams.xNoBlock) {
            if (!pxTimeout) {
                if (sem_wait(&pxQueue->xSemPlaces) != 0)
                    return ERR_SET_ERRNO;
            } else {
                if (sem_timedwait(&pxQueue->xSemPlaces, &xTs) != 0) {
                    if (errno == ETIMEDOUT)
                        return ERR_SET(ERR_TIMEOUT);
                    else
                        return ERR_SET_ERRNO;
                }
            }
        }
        /* Return an error if the list size is bounded and that it's full */
        else if (unRsListLength(&pxQueue->xQueueList) == pxQueue->xParams.unMaxItemCount)
            return ERR_SET(ERR_OVERFLOW);
    }

    /* We have a place, enqueue an item */
    if (!(pxQueueItem = pxRsrcAlloc(pxQueue->xPool, __FUNCTION__)))
        return ERR_SET_OOM;

    pthread_mutex_lock(&pxQueue->xMutex);

    memcpy(pxQueueItem->xItemBuf, pxItem, pxQueue->xParams.unItemSz);
    vRsListInitItem(&pxQueueItem->xListItem, pxQueueItem);
    vRsListInsertEnd(&pxQueue->xQueueList, &pxQueueItem->xListItem);

    pthread_mutex_unlock(&pxQueue->xMutex);

    /* Notify that there is an item available through the semaphore */
    if (!pxQueue->xParams.xNoBlock && sem_post(&pxQueue->xSemItems) != 0)
        return ERR_SET_OOM;

    return SUCCESS;
}

static rsErr_t prvQueueReceive(RsQueue_t *pxQueue, void *pxItem, useconds_t pxTimeout)
{
    RsListItem_t *pxListItem;
    struct queueItem *pxQueueItem;
    struct timespec xTs = {0};

    if (pxTimeout > 0 && !rstime_waitusec(&xTs, pxTimeout))
        return ERR_SET_ERRNO;

    /* Wait for a free item */
    if (!pxQueue->xParams.xNoBlock) {
        if (!pxTimeout) {
            if (sem_wait(&pxQueue->xSemItems) != 0)
                return ERR_SET_ERRNO;
        } else {
            if (sem_timedwait(&pxQueue->xSemItems, &xTs) != 0) {
                if (errno == ETIMEDOUT)
                    return ERR_SET(ERR_TIMEOUT);
                else
                    return ERR_SET_ERRNO;
            }
        }
    }

    pthread_mutex_lock(&pxQueue->xMutex);

    pxListItem = pxRsListGetFirst(&pxQueue->xQueueList);

    if (pxListItem) {
        pxQueueItem = (struct queueItem *)pxListItem->pvOwner;
        memcpy(pxItem, pxQueueItem->xItemBuf, pxQueue->xParams.unItemSz);
    }
    /* Returns an underflow if the list is empty */
    else return ERR_SET(ERR_UNDERFLOW);

    pthread_mutex_unlock(&pxQueue->xMutex);

    vRsListRemove(pxListItem);

    /* Notify that there is an extra place available */
    if (!pxQueue->xParams.xNoBlock && sem_post(&pxQueue->xSemPlaces) != 0)
        return ERR_SET_OOM;

    vRsrcFree(pxQueueItem);

    return SUCCESS;
}

rsErr_t xQueueSendToBackWait(RsQueue_t *pxQueue, void *pxItem)
{
    return prvQueueSendToBack(pxQueue, pxItem, 0);
}

rsErr_t xQueueSendToBackTimed(RsQueue_t *pxQueue, void *pxItem, useconds_t pxTimeout)
{
    return prvQueueSendToBack(pxQueue, pxItem, pxTimeout);
}

rsErr_t xQueueReceiveWait(RsQueue_t *pxQueue, void *pxItem)
{
    return prvQueueReceive(pxQueue, pxItem, 0);
}

rsErr_t xQueueReceiveTimed(RsQueue_t *pxQueue, void *pxItem, useconds_t pxTimeout)
{
    return prvQueueReceive(pxQueue, pxItem, pxTimeout);
}

rsErr_t xQueueSendToBack(RsQueue_t *pxQueue, void *pxItem)
{
    rsErr_t xStatus = SUCCESS;

    pthread_mutex_lock(&pxQueue->xMutex);

    /* This is a non blocking call so look at the queue length before
     * enqueue anything. Return an overflow error in case we're full. */
    if (pxQueue->xParams.unMaxItemCount != 0)
        if (unRsListLength(&pxQueue->xQueueList) == pxQueue->xParams.unMaxItemCount)
            xStatus = ERR_SET(ERR_OVERFLOW);

    if (!ERR_CHK(xStatus))
        xStatus = prvQueueSendToBack(pxQueue, pxItem, 0);

    pthread_mutex_unlock(&pxQueue->xMutex);

    return ERR_SET(xStatus);
}

rsErr_t xQueueReceive(RsQueue_t *pxQueue, void *pxItem)
{
    rsErr_t xStatus = SUCCESS;;

    pthread_mutex_lock(&pxQueue->xMutex);

    /* This is a non-blocking call so look at the queue length before
     * trying to dequeu anything. Return an underflow error if the
     * queue is empty */
    if (unRsListLength(&pxQueue->xQueueList) == 0)
        xStatus = ERR_SET(ERR_UNDERFLOW);
    else
        xStatus = prvQueueReceive(pxQueue, pxItem, 0);

    pthread_mutex_unlock(&pxQueue->xMutex);

    return ERR_SET(xStatus);
}
