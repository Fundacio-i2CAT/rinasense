#ifndef _PORTABILITY_RSTHREAD_H_INCLUDED
#define _PORTABILITY_RSTHREAD_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "portability/port.h"

/* This returns TRUE if we can set the thread name after it's started
 * (xRsThreadSetName). If this is FALSE then the thread name has to
 * be preset with xRsThreadPresetName() */
bool_t xRsThreadCanSetName();

/* On ESP32, we can set a thread name in a default configuration
 * structure. This will create the thread with the right name. On
 * Linux, the pthread_setname_np call exists, which sets the thread
 * name after its creation. */

/* Sets the name to use for the thread just BEFORE creating it. */
rsErr_t xRsThreadPresetName(const string_t pcThreadName);

/* Clear the preset name. Should be called after a thread is
 * successfully created */
rsErr_t xRsThreadPresetNameClear();

/* Sets the name to use for the thread given a created thread handle. */
rsErr_t xRsThreadSetName(pthread_t *pxThread, const string_t pcThreadName);

#ifdef __cplusplus
}
#endif

#endif /* _PORTABILITY_RSTHREAD_H_INCLUDED */

