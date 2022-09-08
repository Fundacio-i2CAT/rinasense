#ifndef _PORTABILITY_RS_LOG_H
#define _PORTABILITY_RS_LOG_H

#include <stdarg.h>
#include "portability/port.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RS_LOG_LOCAL_LEVEL
#define RS_LOG_LOCAL_LEVEL  200 //CONFIG_LOG_MAXIMUM_LEVEL
#endif

#define _RS_COLOR_BLACK   "30"
#define _RS_COLOR_RED     "31"
#define _RS_COLOR_GREEN   "32"
#define _RS_COLOR_BROWN   "33"
#define _RS_COLOR_BLUE    "34"
#define _RS_COLOR_PURPLE  "35"
#define _RS_COLOR_CYAN    "36"

#define _RS_LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define _RS_LOG_BOLD(COLOR)   "\033[1;" COLOR "m"

#define _RS_LOG_RESET_COLOR   "\033[0m"

#define _RS_LOG_COLOR_E       _RS_LOG_COLOR(_RS_COLOR_RED)
#define _RS_LOG_COLOR_W       _RS_LOG_COLOR(_RS_COLOR_BROWN)
#define _RS_LOG_COLOR_I       _RS_LOG_COLOR(_RS_COLOR_GREEN)
#define _RS_LOG_COLOR_D
#define _RS_LOG_COLOR_V

#define     LOG_FORMAT(letter, format)      LOG_COLOR_ ## letter #letter " %s: " format LOG_RESET_COLOR "\n"
#define _RS_LOG_FORMAT(letter, format)  _RS_LOG_COLOR_ ## letter #letter " %s: " format _RS_LOG_RESET_COLOR "\n"

typedef enum {
    LOG_NONE = 0,   /*!< No log output */
    LOG_ERROR = 1,  /*!< Critical errors, software module can not recover on its own */
    LOG_WARN = 2,   /*!< Error conditions from which recovery measures have been taken */
    LOG_INFO = 3,   /*!< Information messages which describe normal flow of events */
    LOG_DEBUG = 4,  /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    LOG_VERBOSE = 5 /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output
. */
} RsLogLevel_t;

#define _RS_LOG_LEVEL(level, tag, format, ...)  \
    do { \
    switch(level) { \
    case LOG_ERROR: vRsLogWrite(LOG_ERROR, tag, _RS_LOG_FORMAT(E, format), tag, ##__VA_ARGS__); break; \
    case LOG_WARN:  vRsLogWrite(LOG_WARN,  tag, _RS_LOG_FORMAT(W, format), tag, ##__VA_ARGS__); break; \
    case LOG_INFO:  vRsLogWrite(LOG_INFO,  tag, _RS_LOG_FORMAT(I, format), tag, ##__VA_ARGS__); break; \
    case LOG_DEBUG: vRsLogWrite(LOG_DEBUG, tag, _RS_LOG_FORMAT(D, format), tag, ##__VA_ARGS__); break; \
    default:                                                            \
    vRsLogWrite(LOG_VERBOSE, tag, _RS_LOG_FORMAT(V, format), tag, ##__VA_ARGS__); \
    }} while(0)

#define _RS_LOG_LEVEL_LOCAL(level, tag, format, ...) \
    do {                                                                \
        if ( RS_LOG_LOCAL_LEVEL >= level ) _RS_LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
    } while(0)

void vRsLogWrite(RsLogLevel_t, const char*, const char*, ...) __attribute__ ((format (printf, 3, 4)));

void vRsLogWritev(RsLogLevel_t, const char*, const char*, va_list args);

/* Public interface to use */

#define LOGE( tag, format, ... ) _RS_LOG_LEVEL_LOCAL(LOG_ERROR,   tag, format, ##__VA_ARGS__)
#define LOGW( tag, format, ... ) _RS_LOG_LEVEL_LOCAL(LOG_WARN,    tag, format, ##__VA_ARGS__)
#define LOGI( tag, format, ... ) _RS_LOG_LEVEL_LOCAL(LOG_INFO,    tag, format, ##__VA_ARGS__)
#define LOGD( tag, format, ... ) _RS_LOG_LEVEL_LOCAL(LOG_DEBUG,   tag, format, ##__VA_ARGS__)
#define LOGV( tag, format, ... ) _RS_LOG_LEVEL_LOCAL(LOG_VERBOSE, tag, format, ##__VA_ARGS__)

/* To be called once at the start of the program to initialize the
 * logging system. */
void vRsLogInit();

void vRsLogSetLevel(const string_t cTagName, RsLogLevel_t eLogLevel);

#ifdef __cplusplus
}
#endif


#endif // _PORTABILITY_RS_LOG_H
