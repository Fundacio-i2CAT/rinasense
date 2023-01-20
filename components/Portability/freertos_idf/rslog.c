#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#include "esp_log.h"

#include "portability/rslog.h"

pthread_mutex_t xLogMutex;

void vRsLogInit()
{
    pthread_mutexattr_t xLogMutexAttr;

    pthread_mutexattr_init(&xLogMutexAttr);
    RsAssert(pthread_mutexattr_settype(&xLogMutexAttr, PTHREAD_MUTEX_RECURSIVE) == 0);
    RsAssert(pthread_mutex_init(&xLogMutex, &xLogMutexAttr) == 0);

    pthread_mutexattr_destroy(&xLogMutexAttr);
}

void vRsLogSetLevel(const string_t pcTagName, RsLogLevel_t eLogLevel)
{
    esp_log_level_set(pcTagName, eLogLevel);
}

void vRsLogWrite(RsLogLevel_t level, const char* tag, const char* format, ...)
{
    va_list list;
    va_start(list, format);
    vRsLogWritev(level, tag, format, list);
    va_end(list);
}

void vRsLogWritev(RsLogLevel_t level, const char* tag, const char* format, va_list args)
{
    LOG_LOCK();
    esp_log_writev(level, tag, format, args);
    LOG_UNLOCK();
}

uint32_t ulRsLogTimestamp(void)
{
    return esp_log_timestamp();
}

