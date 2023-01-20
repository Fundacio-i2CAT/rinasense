#ifndef _COMMON_BLOCKING_QUEUE_H_INCLUDED
#define _COMMON_BLOCKING_QUEUE_H_INCLUDED

#include "portability/port.h"
#include "common/rsrc.h"
#include "common/list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* Set this to the maximum number of items that the list should
     * contain. */
    size_t unMaxItemCount;

    /* Set this to false if you want the queue to return nulls instead
     * of blocking */
    bool_t xBlock;

    /* Size of the items stored in the list */
    size_t unItemSz;

} RsQueueParams_t;

typedef struct xRSQUEUE_T {
    RsList_t xQueueList;

    rsrcPoolP_t xPool;

    /* Semaphore for items in the queue */
    RsSem_t xSemItems;

    /* Semaphore for places left in the queue. */
    RsSem_t xSemPlaces;

    pthread_mutex_t xMutex;

    const RsQueueParams_t xParams;
} RsQueue_t;

rsErr_t xRsQueueInit(const char *sQueueName, RsQueue_t *pxQueue, RsQueueParams_t *pxQueueParams);

void xRsQueueFini(RsQueue_t *pxQueue);

rsErr_t xRsQueueSendToBack(RsQueue_t *pxQueue, void *pxItem);

rsErr_t xRsQueueReceive(RsQueue_t *pxQueue, void *pxItem);

bool_t xRsQueueCanPush(RsQueue_t *pxQueue);

/* Blocks forever if the queue as reached its maximum size. */
rsErr_t xRsQueueSendToBackWait(RsQueue_t *pxQueue, void *pxItem);

/* Blocks for 'pxTimeout' */
rsErr_t xRsQueueSendToBackTimed(RsQueue_t *pxQueue, void *pxItem, useconds_t pxTimeout);

/* Blocks forever */
rsErr_t xRsQueueReceiveWait(RsQueue_t *pxQueue, void *pxItem);

/* Blocks for 'pxTimeout' */
rsErr_t xRsQueueReceiveTimed(RsQueue_t *pxQueue, void *pxItem, useconds_t pxTimeout);

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_BLOCKING_QUEUE_H_INCLUDED */
