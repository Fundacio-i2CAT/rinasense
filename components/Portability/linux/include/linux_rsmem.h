#ifndef _PORTABILITY_LINUX_RSMEM_H
#define _PORTABILITY_LINUX_RSMEM_H

#include <stdlib.h>

#ifndef pvRsMemAlloc
#define pvRsMemAlloc(x) malloc(x)
#endif

#ifndef pvRsMemCAlloc
#define pvRsMemCAlloc(n, sz) calloc(n, sz)
#endif

#ifndef pvRsMemRealloc
#define pvRsMemRealloc(n, sz) realloc(n, sz)
#endif

#ifndef vRsMemFree
#define vRsMemFree(x) free(x)
#endif

#endif // _PORTABILITY_LINUX_RSMEM_H
