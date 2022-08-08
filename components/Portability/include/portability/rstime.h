#ifndef _COMMON_PORTABILITY_RSTIME_H
#define _COMMON_PORTABILITY_RSTIME_H

#include <stdint.h>
#include "portability/port.h"

#define rstime_waitmsec(ts, n) \
    rstime_waitnsec(ts, n * 1000000)

#define rstime_waitusec(ts, n) \
    rstime_waitnsec(ts, n * 1000)

/* Now + N seconds */
bool_t rstime_waitsec(struct timespec *ts, uint32_t n);

/* Now + N nanoseconds */
bool_t rstime_waitnsec(struct timespec *ts, uint64_t n);

extern struct timespec TIMESPEC_ZERO;

#endif // _COMMON_PORTABILITY_RSTIME_H
