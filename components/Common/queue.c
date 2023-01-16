#include <pthread.h>
#include <time.h>
#include <string.h>

#include "portability/port.h"

#include "common/rinasense_errors.h"
#include "common/rsrc.h"

#include "common/error.h"
#include "common/list.h"
#include "common/queue.h"
#include "portability/rssem.h"

struct queueItem {
    RsListItem_t xListItem;
    uint8_t xItemBuf[];
};

rsErr_t xRsQueueInit(const char *sQueueName, RsQueue_t *pxQueue, RsQueueParams_t *pxQueueParams)
{
    int n;
    size_t unSz;
    pthread_mutexattr_t xMutexAttr;

    memcpy((void *)&pxQueue->xParams, pxQueueParams, sizeof(RsQueueParams_t));

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

    /* If the queue is blocking, initialize the semaphores */
    if (pxQueue->xParams.xBlock) {
        if (ERR_CHK(xRsSemInit(&pxQueue->xSemItems, 0)))
            return FAIL;

        /* If the queue has a limited size, prepare a semaphore that will
         * control free places in the queue */
        if (pxQueueParams->unMaxItemCount != 0)
            if (ERR_CHK(xRsSemInit(&pxQueue->xSemPlaces, pxQueue->xParams.unMaxItemCount)))
                return FAIL;
    }

    return SUCCESS;
}

static rsErr_t prvQueueSendToBack(RsQueue_t *pxQueue, void *pxItem, useconds_t unTimeout)
{
    struct queueItem *pxQueueItem;

#if 0
    vRsSemDebug("[Queue-SendToBack]", "Places", &pxQueue->xSemPlaces);
    vRsSemDebug("[Queue-SendToBack]", "Items", &pxQueue->xSemItems);
#endif

    if (pxQueue->xParams.unMaxItemCount > 0) {

        /* Wait for a free space if needed */
        if (pxQueue->xParams.xBlock) {
            if (!unTimeout) {
                if (ERR_CHK(xRsSemWait(&pxQueue->xSemPlaces)))
                    return FAIL;
            } else {
                if (ERR_CHK(xRsSemTimedWait(&pxQueue->xSemPlaces, unTimeout)))
                    return FAIL;
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
    if (pxQueue->xParams.xBlock)
        if (ERR_CHK(xRsSemPost(&pxQueue->xSemItems)))
            return FAIL;

    return SUCCESS;
}

static rsErr_t prvQueueReceive(RsQueue_t *pxQueue, void *pxItem, useconds_t unTimeout)
{
    RsListItem_t *pxListItem;
    struct queueItem *pxQueueItem;

#if 0
    vRsSemDebug("[Queue-Receive]", "Places", &pxQueue->xSemPlaces);
    vRsSemDebug("[Queue-Receive]", "Items", &pxQueue->xSemItems);
#endif

    /* Wait for a free item */
    if (pxQueue->xParams.xBlock) {
        if (!unTimeout) {
            if (ERR_CHK(xRsSemWait(&pxQueue->xSemItems)))
                return FAIL;
        } else {
            if (ERR_CHK(xRsSemTimedWait(&pxQueue->xSemItems, unTimeout)))
                return FAIL;
        }
    }

    pthread_mutex_lock(&pxQueue->xMutex);

    pxListItem = pxRsListGetFirst(&pxQueue->xQueueList);

    if (pxListItem) {
        pxQueueItem = (struct queueItem *)pxListItem->pvOwner;
        memcpy(pxItem, pxQueueItem->xItemBuf, pxQueue->xParams.unItemSz);
    }
    /* Returns an underflow if the list is empty */
    else {
        pthread_mutex_unlock(&pxQueue->xMutex);
        return ERR_SET(ERR_UNDERFLOW);
    }

    vRsListRemove(pxListItem);

    pthread_mutex_unlock(&pxQueue->xMutex);

    /* Notify that there is an extra place available */
    if (pxQueue->xParams.xBlock)
        if (ERR_CHK(xRsSemPost(&pxQueue->xSemPlaces)))
            return FAIL;

    vRsrcFree(pxQueueItem);

    return SUCCESS;
}

rsErr_t xRsQueueSendToBackWait(RsQueue_t *pxQueue, void *pxItem)
{
    return prvQueueSendToBack(pxQueue, pxItem, 0);
}

rsErr_t xRsQueueSendToBackTimed(RsQueue_t *pxQueue, void *pxItem, useconds_t unTimeout)
{
    return prvQueueSendToBack(pxQueue, pxItem, unTimeout);
}

rsErr_t xRsQueueReceiveWait(RsQueue_t *pxQueue, void *pxItem)
{
    return prvQueueReceive(pxQueue, pxItem, 0);
}

rsErr_t xRsQueueReceiveTimed(RsQueue_t *pxQueue, void *pxItem, useconds_t unTimeout)
{
    return prvQueueReceive(pxQueue, pxItem, unTimeout);
}

rsErr_t xRsQueueSendToBack(RsQueue_t *pxQueue, void *pxItem)
{
    rsErr_t xStatus = SUCCESS;

    pthread_mutex_lock(&pxQueue->xMutex);

    /* This is a non blocking call so look at the queue length before
     * enqueue anything. Return an overflow error in case we're full. */
    if (pxQueue->xParams.unMaxItemCount != 0)
        if (unRsListLength(&pxQueue->xQueueList) == pxQueue->xParams.unMaxItemCount)
            xStatus = ERR_OVERFLOW;

    if (!ERR_CHK(xStatus))
        xStatus = prvQueueSendToBack(pxQueue, pxItem, 0);

    pthread_mutex_unlock(&pxQueue->xMutex);

    return ERR_SET(xStatus);
}

rsErr_t xRsQueueReceive(RsQueue_t *pxQueue, void *pxItem)
{
    rsErr_t xStatus = SUCCESS;;

    pthread_mutex_lock(&pxQueue->xMutex);

    /* This is a non-blocking call so look at the queue length before
     * trying to dequeu anything. Return an underflow error if the
     * queue is empty */
    if (unRsListLength(&pxQueue->xQueueList) == 0)
        xStatus = ERR_UNDERFLOW;
    else
        xStatus = prvQueueReceive(pxQueue, pxItem, 0);

    pthread_mutex_unlock(&pxQueue->xMutex);

    return ERR_SET(xStatus);
}
