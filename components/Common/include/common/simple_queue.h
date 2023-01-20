#ifndef _COMMON_QUEUE_H
#define _COMMON_QUEUE_H

#include "portability/port.h"

#include "common/rsrc.h"
#include "common/list.h"

/* Simple unblocking queue API based on linked lists. */

typedef struct xRSSIMPLERSQUEUE_T {

    RsList_t xQueueList;

    rsrcPoolP_t xPool;

} RsSimpleQueue_t;

bool_t xSimpleQueueInit(const char *sQueueName, RsSimpleQueue_t *pxQueue);

void vSimpleQueueFini(RsSimpleQueue_t *pxQueue);

/**
 * Adds an item at the back of the queue.
 */
bool_t xSimpleQueueSendToBack(RsSimpleQueue_t *pxQueue, void *pxItem);

/**
 * Return the first item available from the queue, or NULL if nothing
 * available.
 */
void *pvSimpleQueueReceive(RsSimpleQueue_t *pxQueue);

#define unSimpleQueueLength(queue) \
    unRsListLength(queue.xQueueList)

#endif // _COMMON_QUEUE_H
