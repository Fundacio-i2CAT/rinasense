#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#include "portability/rslog.h"

void vRsLogWrite(RsLogLevel_t level, const char* tag, const char* format, ...)
{
    va_list list;
    va_start(list, format);
    vRsLogWritev(level, tag, format, list);
    va_end(list);
}

void vRsLogWritev(RsLogLevel_t level, const char* tag, const char* format, va_list args)
{
    vprintf(format, args);
}

uint32_t ulRsLogTimestamp(void)
{
}

