#ifndef SERDES_H_INCLUDED
#define SERDES_H_INCLUDED

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    /*Buffer with the message*/
    void *pvSerBuffer;

    /*Size of the buffer*/
    size_t xSerLength;

} serObjectValue_t;

/* FIXME: This is the ethernet MTU.
 *
 * Maybe this should be a configuration option, or this value can be
 * fed to this code. It's also not too clear why the MTU should be
 * used here.
 */
#define ENROLLMENT_MSG_SIZE 1500

#define _SERDES_FIELD_COPY(x, i, y)                \
    if (y) {                                       \
        strncpy(x.i, y, sizeof(x.i));              \
        x.has_##i = true;                          \
    }

#define _SERDES_STRING_COPY(x, i, y)            \
    if (y) {                                    \
        strncpy(x.i, y, sizeof(x.i));           \
    }

#ifdef __cplusplus
}
#endif

#endif /* SERDES_H_INCLUDED */
