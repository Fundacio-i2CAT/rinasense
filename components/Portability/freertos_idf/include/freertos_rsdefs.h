#ifndef _PORT_IDF_RSDEFS_H
#define _PORT_IDF_RSDEFS_H

#include <stdint.h>

typedef BaseType_t bool_t;
#define PORT_HAS_BOOL_T

typedef BaseType_t num_t;
#define PORT_HAS_NUM_T

typedef char* string_t;
#define PORT_HAS_STRING_T

typedef char  stringbuf_t;
#define PORT_HAS_STRINGBUF_T

typedef uint8_t* buffer_t;
#define PORT_HAS_BUFFER_T

#endif // _PORT_IDF_RSDEFS_H
