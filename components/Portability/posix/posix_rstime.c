#include <assert.h>
#include <time.h>
#include <string.h>

#include "portability/port.h"

#define SEC_TO_NS(sec) ((sec)*1000000000)

#ifdef CLOCK_MONOTONIC
#define TIMEOUT_CLOCK CLOCK_MONOTONIC
#else
#define TIMEOUT_CLOCK CLOCK_REALTIME
#endif

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

    /* Prevent overflow in tv_nsec. */
    if (ts->tv_nsec >= 1000000000) {
        ts->tv_nsec = ts->tv_nsec - 1000000000;
        ts->tv_sec++;
    }

    RsAssert(ts->tv_nsec <= 999999999);

    return true;
}

bool_t xRsTimeSetTimeOut(struct RsTimeOut *pTimeOut)
{
    if (clock_gettime(TIMEOUT_CLOCK, &(pTimeOut->xTimespec)))
        return false;

    return true;
}

bool_t xRsTimeCheckTimeOut(struct RsTimeOut *pTimeOut,
                           useconds_t *pxTimeLeft)
{
    struct timespec xCheckTimespec;
    uint64_t unNsTimeNow, unNsTimeThen, unNsTimeDone;
    int64_t nNsTimeLeft;

    /* It makes no sense to call this function without wanting some
     * kind of result back */
    RsAssert(pxTimeLeft);

    if (clock_gettime(TIMEOUT_CLOCK, &xCheckTimespec) < 0)
        return false;

    /* unTimeThen -- Time at the first call of xRsSetTimeOut or
       previous call to xRsTimeCheckTimeOut */
    unNsTimeThen = SEC_TO_NS(pTimeOut->xTimespec.tv_sec) + pTimeOut->xTimespec.tv_nsec;

    /* unTimeNow -- Time at this moment. */
    unNsTimeNow = SEC_TO_NS(xCheckTimespec.tv_sec) + xCheckTimespec.tv_nsec;

    /* unTimeDone -- Time at which the timeout will be expired. */
    unNsTimeDone = unNsTimeThen + (*pxTimeLeft * 1000);

    /* Update the time left with in the timeout */
    nNsTimeLeft = unNsTimeDone - unNsTimeNow;
    *pxTimeLeft = nNsTimeLeft > 0 ? nNsTimeLeft / 1000 : 0;

    /* Save the new time. */
    memcpy(&pTimeOut->xTimespec, &xCheckTimespec, sizeof(struct timespec));

    return true;
}
