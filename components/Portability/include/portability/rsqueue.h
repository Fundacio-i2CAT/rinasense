#ifndef _COMMON_PORTABILITY_RSQUEUE_H
#define _COMMON_PORTABILITY_RSQUEUE_H

#include <unistd.h>

#include "port.h"
#include "posix_defaults.h"

/* Minimal queue API based on what is required in the IPCP. */

RsQueue_t *pxRsQueueCreate(const char *sQueueName,
                           const size_t uxQueueLength,
                           const size_t uxItemSize);

void vRsQueueDelete(RsQueue_t *pQueue);

bool_t xRsQueueSendToBack(RsQueue_t *pxQueue,
                          const void *pvItemToQueue,
                          const size_t unItemSize,
                          const useconds_t xTimeOutUS);

bool_t xRsQueueReceive(RsQueue_t *pxQueue,
                       void *pvBuffer,
                       const size_t unBufferSize,
                       const useconds_t xTimeOutUS);

#endif // _COMMON_PORTABILITY_RSQUEUE_H
