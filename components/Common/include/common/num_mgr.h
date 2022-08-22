#ifndef _COMMON_NUM_MGR
#define _COMMON_NUM_MGR

#include <stdint.h>
#include <stddef.h>

#include "bit_array.h"
#include "portability/port.h"

typedef struct NUMMGR_T {
    /* Number of numbers to deal with. */
    size_t numCnt;

    /* Last number that was allocated. */
    uint32_t lastAllocated;

    /* Bit array storing the numbers. */
    bitarray_t *ba;

} NumMgr_t;

NumMgr_t *pxNumMgrCreate(size_t numCnt);

bool_t xNumMgrInit(NumMgr_t *im, size_t numCnt);

void vNumMgrFini(NumMgr_t *im);

void vNumMgrDestroy(NumMgr_t *im);

uint32_t ulNumMgrAllocate(NumMgr_t *im);

bool_t xNumMgrRelease(NumMgr_t *im, uint32_t n);

bool_t xNumMgrIsAllocated(NumMgr_t *im, uint32_t n);

#endif // _COMMON_NUM_MGR
