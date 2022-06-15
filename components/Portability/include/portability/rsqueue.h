#ifndef _COMMON_PORTABILITY_RSQUEUE_H
#define _COMMON_PORTABILITY_RSQUEUE_H

#include "port.h"
#include "posix_defaults.h"

/* Minimal queue API based on what is required in the IPCP. */

RsQueue_t *pxRsQueueCreate(const char *sQueueName,
                          size_t uxQueueLength, size_t uxItemSize);

void vRsQueueDelete(RsQueue_t *pQueue);

bool_t xRsQueueSendToBack(RsQueue_t *pxQueue,
                         const void *pvItemToQueue, struct timespec *ts);

bool_t xRsQueueReceive(RsQueue_t *pxQueue,
                      void *pvBuffer, struct timespec *ts);

#endif // _COMMON_PORTABILITY_RSQUEUE_H


