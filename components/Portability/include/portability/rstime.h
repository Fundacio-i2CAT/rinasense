#ifndef _COMMON_PORTABILITY_RSTIME_H
#define _COMMON_PORTABILITY_RSTIME_H

/* Now + N milliseconds */
bool_t rstime_waitmsec(struct timespec *ts, size_t n);

/* Now + N seconds */
bool_t rstime_waitsec(struct timespec *ts, size_t n);

#endif // _COMMON_PORTABILITY_RSTIME_H
