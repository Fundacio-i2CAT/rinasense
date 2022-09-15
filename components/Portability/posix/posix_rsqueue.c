#include <dirent.h> // For NAME_MAX
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "portability/port.h"
#include "portability/posix/mqueue.h"
#include "posix_rsqueue.h"

#define TAG "[RsQueue]"

/**
 * There are some specific rules you must follow for sQueueName. See
 * the "mq_overview" manual page for the rules. Don't prefix the queue
 * name with / as this implementation handles this.
 */
RsQueue_t *pxRsQueueCreate(const char *sQueueName,
                           const size_t uxQueueLength,
                           const size_t uxItemSize)
{
    RsQueue_t *pQueue;
    mqd_t xQueue;
    char buf[NAME_MAX];
    struct mq_attr mqInitAttr = {
        .mq_flags = 0,
        .mq_maxmsg = uxQueueLength,
        .mq_msgsize = uxItemSize,
        .mq_curmsgs = 0};

    if (!snprintf(buf, sizeof(buf), "/%s", sQueueName))
    {
        LOGE(TAG, "Error setting up queue name");
        return NULL;
    }

    xQueue = mq_open(buf, O_RDWR | O_CREAT, 0644, &mqInitAttr);
    if (xQueue == (mqd_t)-1)
    {
        LOGE(TAG, "mq_open failed (errno: %d)", errno);
        return NULL;
    }

    pQueue = pvRsMemAlloc(sizeof(RsQueue_t));
    if (!pQueue)
    {
        LOGE(TAG, "Out of memory allocating queue object");
        goto err;
    }

    if ((pQueue->sQueueName = strdup(buf)) == NULL)
    {
        LOGE(TAG, "Out of memory allocating queue name");
        goto err;
    }

    pQueue->xQueue = xQueue;
    pQueue->uxItemSize = uxItemSize;
    pQueue->uxQueueLength = uxQueueLength;

    return pQueue;

err:
    mq_close(xQueue);
    mq_unlink(buf);

    return NULL;
}

void vRsQueueDelete(RsQueue_t *pQueue)
{
    mq_close(pQueue->xQueue);
    mq_unlink(pQueue->sQueueName);

    vRsMemFree(pQueue->sQueueName);
    vRsMemFree(pQueue);
}

/** This puts some data in the queue.
 *
 * FreeRTOS xQueueSendToBack returns immediately if the timeout is set
 * to 0, so this functions tries to imitate this semantic.
 */
bool_t xRsQueueSendToBack(RsQueue_t *pQueue,
                          const void *pvItemToQueue,
                          const size_t unItemSize,
                          const useconds_t xTimeOutUS)
{
    struct timespec ts;
    struct mq_attr attr;

    if (xTimeOutUS == 0)
    {
        /* If there is no timeout specified, we need to first check if
           the call to mq_send will block or not. If it does, we must not
           call it. */
        if (mq_getattr(pQueue->xQueue, &attr) == 0)
        {

            /* Check if the queue is full. */
            if (attr.mq_curmsgs == attr.mq_maxmsg)
            {
                LOGE(TAG, "Queue is full! Cannot send without timeout");
                return false;
            }

            /* Queue not full? mq_send is not supposed to block here. */
            if (mq_send(pQueue->xQueue, pvItemToQueue, unItemSize, 0) != 0)
            {
                LOGE(TAG, "mq_send failed (errno: %d)", errno);
                return false;
            }
        }
        else
        {
            LOGE(TAG, "mq_getattr failed");
            return false;
        }
    }
    else
    {
        /* If there is a timeout, then the semantic follow the
           behaviour of mq_timesend */
        if (!rstime_waitusec(&ts, xTimeOutUS))
            return false;

        if (mq_timedsend(pQueue->xQueue, pvItemToQueue, unItemSize, 0, &ts) != 0)
        {
            LOGE(TAG, "mq_timedsend failed (errno: %d)", errno);
            return false;
        }
    }

    LOGD(TAG, "Sent %zu byte(s) on %s", pQueue->uxItemSize, pQueue->sQueueName);

    return true;
}

bool_t xRsQueueReceive(RsQueue_t *pQueue, void *pvBuffer, const size_t unBufferSz, useconds_t xTimeOutUS)
{
    struct timespec ts;

    if (!rstime_waitusec(&ts, xTimeOutUS))
        return false;
    printf("Timeout: %lld.%.9ld\n", (long long)ts.tv_sec, ts.tv_nsec);

    if (mq_receive(pQueue->xQueue, pvBuffer, unBufferSz, 0) < 0)
    {
        if (errno != ETIMEDOUT)
            LOGE(TAG, "mq_timedreceive error: %d", errno);

        return false;
    }
    /*if (mq_timedreceive(pQueue->xQueue, pvBuffer, unBufferSz, 0, &ts) < 0)
    {
        if (errno != ETIMEDOUT)
            LOGE(TAG, "mq_timedreceive error: %d", errno);

        return false;
    }*/

    return true;
}
