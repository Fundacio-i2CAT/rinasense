#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#include "esp_log.h"

#include "portability/rslog.h"

void vRsLogInit()
{
    /* No operations needed here. */
}

void vRsLogSetLevel(const string_t pcTagName, RsLogLevel_t eLogLevel)
{
    esp_log_level_set(pcTagName, eLogLevel);
}

void vRsLogWrite(RsLogLevel_t level, const char* tag, const char* format, ...)
{
    va_list list;
    va_start(list, format);
    vRsLogWritev(level, tag, format, list);
    va_end(list);
}

void vRsLogWritev(RsLogLevel_t level, const char* tag, const char* format, va_list args)
{
    esp_log_writev(level, tag, format, args);
}

uint32_t ulRsLogTimestamp(void)
{
    return esp_log_timestamp();
}

