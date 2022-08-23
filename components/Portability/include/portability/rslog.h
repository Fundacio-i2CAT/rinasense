#ifndef _PORTABILITY_RS_LOG_H
#define _PORTABILITY_RS_LOG_H

#include <stdarg.h>
#include "portability/port.h"

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL  200 //CONFIG_LOG_MAXIMUM_LEVEL
#endif

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D
#define LOG_COLOR_V

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " %s: " format LOG_RESET_COLOR "\n"

typedef enum {
    LOG_NONE,       /*!< No log output */
    LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    LOG_INFO,       /*!< Information messages which describe normal flow of events */
    LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output
. */
} RsLogLevel_t;

void vRsLogWrite(RsLogLevel_t, const char*, const char*, ...) __attribute__ ((format (printf, 3, 4)));

void vRsLogWritev(RsLogLevel_t, const char*, const char*, va_list args);

#define LOGE( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_ERROR,   tag, format, ##__VA_ARGS__)
#define LOGW( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_WARN,    tag, format, ##__VA_ARGS__)
#define LOGI( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_INFO,    tag, format, ##__VA_ARGS__)
#define LOGD( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_DEBUG,   tag, format, ##__VA_ARGS__)
#define LOGV( tag, format, ... ) LOG_LEVEL_LOCAL(LOG_VERBOSE, tag, format, ##__VA_ARGS__)

#define LOG_LEVEL(level, tag, format, ...) do {                     \
        if (level==LOG_ERROR )          { vRsLogWrite(LOG_ERROR,      tag, LOG_FORMAT(E, format), tag, ##__VA_ARGS__); } \
        else if (level==LOG_WARN )      { vRsLogWrite(LOG_WARN,       tag, LOG_FORMAT(W, format), tag, ##__VA_ARGS__); } \
        else if (level==LOG_DEBUG )     { vRsLogWrite(LOG_DEBUG,      tag, LOG_FORMAT(D, format), tag, ##__VA_ARGS__); } \
        else if (level==LOG_VERBOSE )   { vRsLogWrite(LOG_VERBOSE,    tag, LOG_FORMAT(V, format), tag, ##__VA_ARGS__); } \
        else                            { vRsLogWrite(LOG_INFO,       tag, LOG_FORMAT(I, format), tag, ##__VA_ARGS__); } \
    } while(0)

#define LOG_LEVEL_LOCAL(level, tag, format, ...) do {               \
        if ( LOG_LOCAL_LEVEL >= level ) LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
    } while(0)


/* */
void vRsLogInit();

void vRsLogSetLevel(const string_t cTagName, RsLogLevel_t eLogLevel);

#endif // _PORTABILITY_RS_LOG_H
