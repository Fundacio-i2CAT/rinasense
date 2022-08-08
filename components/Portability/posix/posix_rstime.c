#include <assert.h>
#include <time.h>
#include "portability/port.h"

bool_t rstime_waitsec(struct timespec *ts, uint32_t n)
{
    if (clock_gettime(CLOCK_REALTIME, ts) < 0)
        return false;

    ts->tv_sec += n;
    return true;
}

bool_t rstime_waitnsec(struct timespec *ts, uint64_t n) {
    if (clock_gettime(CLOCK_REALTIME, ts) < 0)
        return false;

    /* Per spec, tv_nsec cannot be higher than a billion. See
     * nanosleep(1) */
    ts->tv_sec += (time_t)(n / 1000000000);
    ts->tv_nsec += (n % 1000000000);

    return true;
}
