#include <pthread.h>

#include "portability/port.h"

uint32_t unRsGetCurrentThreadID()
{
    return (uint32_t)pthread_self();
}