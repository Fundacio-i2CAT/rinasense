#ifndef _COMMON_PORTABILITY_RSTIME_H
#define _COMMON_PORTABILITY_RSTIME_H

#include <stdint.h>
#include "portability/port.h"

/* FIXME: For some reason, those functions name don't match the coding
 * standard. They should be renamed. */

#define rstime_waitmsec(ts, n) \
    rstime_waitnsec(ts, n * 1000000)

#define rstime_waitusec(ts, n) \
    rstime_waitnsec(ts, n * 1000)

/* Now + N seconds */
bool_t rstime_waitsec(struct timespec *ts, uint32_t n);

/* Now + N nanoseconds */
bool_t rstime_waitnsec(struct timespec *ts, uint64_t n);

extern struct timespec TIMESPEC_ZERO;

/* Timeout functions inspired by FreeRTOS timeout functions */

struct RsTimeOut {
    struct timespec xTimespec;
};

bool_t xRsTimeSetTimeOut(struct RsTimeOut *pTimeOut);

bool_t xRsTimeCheckTimeOut(struct RsTimeOut *pTimeOut,
                           useconds_t *pxTimeLeft);

#endif // _COMMON_PORTABILITY_RSTIME_H
