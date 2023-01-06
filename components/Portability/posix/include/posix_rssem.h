#ifndef _PORTABILITY_POSIX_RSSEM_H_INCLUDED
#define _PORTABILITY_POSIX_RSSEM_H_INCLUDED

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/semaphore.h"
#else
#include <semaphore.h>
#endif

typedef struct {
    sem_t xSem;
} RsSem_t;

#endif /* _PORTABILITY_POSIX_RSSEM_H_INCLUDED */
