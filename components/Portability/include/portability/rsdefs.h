#ifndef _PORT_DEFS_H
#define _PORT_DEFS_H

/* typedef ... bool_t */
#ifndef PORT_HAS_BOOL_T
#error Port undefined: bool_t
#endif

/* typedef ... num_t */
#ifndef PORT_HAS_NUM_T
#error Port undefined: num_t
#endif

#ifndef PORT_HAS_STRING_T
#error Port undefined: string_t
#endif

#ifndef PORT_HAS_BUFFER_T
#error Port undefined: buffer_t
#endif

#endif // _PORT_DEFS_H
