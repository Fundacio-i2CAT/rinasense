#ifndef _PORTABILITY_RS_POSIX_H_

/*
 * vSetMaxTimespec is meant to take the place of the use of
 * (TickType_t)portMAX_DELAY. For Linux, and probably other system,
 * this will be a very long delay in a timespec. On FreeRTOS, this has
 * to be the equivalent to the conversion of (TickType_t)portMAX_DELAY
 * to a timespec structure.
 */

void vSetMaxTimespec(struct timespec *ts);

#endif // _PORTABILITY_RS_POSIX_H_
