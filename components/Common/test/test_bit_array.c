#include "bit_array.h"
#include "portability/port.h"

void testBitArrayBasics()
{
    bitarray_t *ba;

    ba = pxBitArrayAlloc(33);

    vBitArraySetBit(ba, 0);
    vBitArraySetBit(ba, 1);
    vBitArraySetBit(ba, 2);
    vBitArraySetBit(ba, 32);

    /* Not allowed since the array has 33 bits and is 0 indexed, but
     * should be an harmless no-op. */
    vBitArraySetBit(ba, 100);

    RsAssert(xBitArrayGetBit(ba, 1));
    RsAssert(xBitArrayGetBit(ba, 2));
    RsAssert(xBitArrayGetBit(ba, 32));

    /* Cheating on an opaque data structure! Don't do that! */
    RsAssert(*(uint32_t *)((void *)ba + sizeof(bitarray_t)) == 7);

    vBitArrayClearBit(ba, 1);
    vBitArrayClearBit(ba, 2);

    RsAssert(*(uint32_t *)((void *)ba + sizeof(bitarray_t)) == 1);

    RsAssert(xBitArrayGetBit(ba, 32));

    vBitArrayFree(ba);
}

void testBitArrayLarge()
{
    bitarray_t *ba;

    ba = pxBitArrayAlloc(10000);

    for (int i = 0; i < 10000; i++)
        vBitArraySetBit(ba, i);

    for (int i = 0; i < (10000 / 32) - 1; i++)
        RsAssert(*(uint32_t *)((void *)ba + sizeof(bitarray_t) + i * sizeof(uint32_t)) == -1);

    vBitArrayFree(ba);
}

int main() {
    testBitArrayBasics();
    testBitArrayLarge();
}
