#include <stdio.h>
#include <string.h>

#include "portability/port.h"
#include "portability/posix/mqueue.h"
#include "posix_rsqueue.h"

RsQueue_t *pxRsQueueCreate(const char *sQueueName,
                          size_t uxQueueLength, size_t uxItemSize)
{
    RsQueue_t *pQueue;
    mqd_t xQueue;
    char buf[NAME_MAX];
    struct mq_attr mqInitAttr = {
        .mq_flags = 0,
        .mq_maxmsg = uxQueueLength,
        .mq_msgsize = uxItemSize,
        .mq_curmsgs = 0
    };

    if (!snprintf(buf, sizeof(buf), "/%s", sQueueName))
        return NULL;

    xQueue = mq_open(buf, O_RDWR | O_CREAT, 0644, &mqInitAttr);
    if (xQueue < 0)
        return NULL;

    pQueue = pvRsMemAlloc(sizeof(RsQueue_t));
    if (!pQueue)
        goto err;

    if ((pQueue->sQueueName = strdup(buf)) == NULL)
        goto err;

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

bool_t xRsQueueSendToBack(RsQueue_t *pQueue, const void *pvItemToQueue, struct timespec *ts)
{
    if (mq_timedsend(pQueue->xQueue, pvItemToQueue, pQueue->uxItemSize, 0, ts))
        return false;

    return true;
}

bool_t xRsQueueReceive(RsQueue_t *pQueue, void *pvBuffer, struct timespec *ts)
{
    if (mq_timedreceive(pQueue->xQueue, pvBuffer, pQueue->uxItemSize, 0, ts) < 0)
        return false;

    return true;
}
