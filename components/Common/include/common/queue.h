#ifndef _COMMON_BLOCKING_QUEUE_H_INCLUDED
#define _COMMON_BLOCKING_QUEUE_H_INCLUDED

#include <semaphore.h>

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

    /* Set this to true if you want the queue to return null instead
     * of blocking */
    bool_t xNoBlock;

    /* Size of the items stored in the list */
    size_t unItemSz;

} RsQueueParams_t;

typedef struct xRSQUEUE_T {
    RsList_t xQueueList;

    rsrcPoolP_t xPool;

    /* Semaphore for items in the queue */
    sem_t xSemItems;

    /* Semaphore for places left in the queue. */
    sem_t xSemPlaces;

    pthread_mutex_t xMutex;

    RsQueueParams_t xParams;
} RsQueue_t;

rsErr_t xQueueInit(const char *sQueueName, RsQueue_t *pxQueue, RsQueueParams_t *pxQueueParams);

void xQueueFini(RsQueue_t *pxQueue);

rsErr_t xQueueSendToBack(RsQueue_t *pxQueue, void *pxItem);

rsErr_t xQueueReceive(RsQueue_t *pxQueue, void *pxItem);

bool_t xQueueCanPush(RsQueue_t *pxQueue);

/* Blocks forever if the queue as reached its maximum size. */
rsErr_t xQueueSendToBackWait(RsQueue_t *pxQueue, void *pxItem);

/* Blocks for 'pxTimeout' */
rsErr_t xQueueSendToBackTimed(RsQueue_t *pxQueue, void *pxItem, useconds_t pxTimeout);

/* Blocks forever */
rsErr_t xQueueReceiveWait(RsQueue_t *pxQueue, void *pxItem);

/* Blocks for 'pxTimeout' */
rsErr_t xQueueReceiveTimed(RsQueue_t *pxQueue, void *pxItem, useconds_t pxTimeout);

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_BLOCKING_QUEUE_H_INCLUDED */
