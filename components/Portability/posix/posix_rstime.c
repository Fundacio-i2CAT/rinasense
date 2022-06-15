#include "portability/port.h"

bool_t rstime_waitmsec(struct timespec *ts, size_t n)
{
    if (clock_gettime(CLOCK_REALTIME, ts) < 0)
        return false;
    ts->tv_nsec += (n * 1000000);
    return true;
}

bool_t rstime_waitsec(struct timespec *ts, size_t n)
{
    if (clock_gettime(CLOCK_REALTIME, ts) < 0)
        return false;
    ts->tv_sec += n;
    return true;
}
