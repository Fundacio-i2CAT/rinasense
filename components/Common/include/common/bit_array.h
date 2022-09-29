#ifndef _COMMON_BIT_ARRAY_H
#define _COMMON_BIT_ARRAY_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "portability/port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _BITARRAY {
    uint32_t nbbits;
} bitarray_t;

#define BITS_PER_UNIT 32
#define UNIT uint32_t

#define BIT_INDEX(n) \
    uint8_t bit = n % BITS_PER_UNIT;\
    uint32_t index = (n - bit) / BITS_PER_UNIT

static inline bitarray_t *pxBitArrayAlloc(uint32_t nbbits)
{
    bitarray_t *p;
    size_t nbUnits = nbbits / BITS_PER_UNIT + 1;
    size_t sz = sizeof(bitarray_t) + nbUnits * sizeof(UNIT);
    p = (bitarray_t *)pvRsMemAlloc(sz);
    if (p) {
        memset(p, 0, sz);
        p->nbbits = nbbits;
    }
    return p;
}

static inline void vBitArrayFree(bitarray_t *ba)
{
    vRsMemFree(ba);
}

static inline void vBitArraySetBit(bitarray_t *ba, uint32_t n)
{
    BIT_INDEX(n);
    if (n < ba->nbbits)
        *(UNIT *)((void *)ba + sizeof(bitarray_t) + index * sizeof(UNIT)) |= (UNIT)(1 << bit);
}

static inline void vBitArrayClearBit(bitarray_t *ba, uint32_t n)
{
    BIT_INDEX(n);
    if (n < ba->nbbits)
        *(UNIT *)((void *)ba + sizeof(bitarray_t) + index * sizeof(UNIT)) &= (UNIT)~(1 << bit);
}

static inline bool_t xBitArrayGetBit(bitarray_t *ba, uint32_t n)
{
    BIT_INDEX(n);
    if (n < ba->nbbits)
        return (*(UNIT *)((void *)ba + sizeof(bitarray_t) + index * sizeof(UNIT)) & (UNIT)(1 << bit)) > 0;
    else
        return false;
}

#ifdef __cplusplus
}
#endif

#endif // _COMMON_BIT_ARRAY_H
