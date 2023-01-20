#include <HardwareSerial.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "portability/rslog.h"

pthread_mutex_t xLogMutex;

stringbuf_t pcLogBuf[256] = {0};

void vRsLogInit()
{
    Serial.begin(115200);

    pthread_mutexattr_t xLogMutexAttr;

    pthread_mutexattr_init(&xLogMutexAttr);
    RsAssert(pthread_mutexattr_settype(&xLogMutexAttr, PTHREAD_MUTEX_RECURSIVE) == 0);
    RsAssert(pthread_mutex_init(&xLogMutex, &xLogMutexAttr) == 0);

    pthread_mutexattr_destroy(&xLogMutexAttr);
}

void vRsLogSetLevel(const string_t pcTagName, RsLogLevel_t eLogLevel)
{
    /* No-op for the moment. */
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
    int n;

    LOG_LOCK();
    vsnprintf(pcLogBuf, sizeof(pcLogBuf), format, args);

    Serial.print(pcLogBuf);
    Serial.print("\r");
    LOG_UNLOCK();
}

