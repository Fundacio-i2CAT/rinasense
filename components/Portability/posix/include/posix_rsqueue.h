#ifndef _PORT_POSIX_RSQUEUE_H
#define _PORT_POSIX_RSQUEUE_H

#include "portability/posix/mqueue.h"

#ifndef PORT_HAS_RSQUEUE_T
typedef struct xRSQUEUE_T {
    mqd_t xQueue;

    size_t uxQueueLength;
    size_t uxItemSize;

    char *sQueueName;
} RsQueue_t;
#define PORT_HAS_RSQUEUE_T
#endif // PORT_HAS_RSQUEUE_T

#endif // _PORT_POSIX_RSQUEUE_H
