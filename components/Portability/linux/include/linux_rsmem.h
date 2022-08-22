#ifndef _PORTABILITY_LINUX_RSMEM_H
#define _PORTABILITY_LINUX_RSMEM_H

#ifndef pvRsMemAlloc
#define pvRsMemAlloc(x) malloc(x)
#endif

#ifndef vRsMemFree
#define vRsMemFree(x) free(x)
#endif

#endif // _PORTABILITY_LINUX_RSMEM_H
