#include "bit_array.h"
#include "portability/port.h"

#include "unity.h"
#include "unity_fixups.h"

#ifndef TEST_CASE
void setUp() {}
void tearDown() {}
#endif

RS_TEST_CASE(BitArrayBasics, "Short bit array manipulations")
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

    TEST_ASSERT(xBitArrayGetBit(ba, 1));
    TEST_ASSERT(xBitArrayGetBit(ba, 2));
    TEST_ASSERT(xBitArrayGetBit(ba, 32));

    /* Cheating on an opaque data structure! Don't do that! */
    TEST_ASSERT(*(uint32_t *)((void *)ba + sizeof(bitarray_t)) == 7);

    vBitArrayClearBit(ba, 1);
    vBitArrayClearBit(ba, 2);

    TEST_ASSERT(*(uint32_t *)((void *)ba + sizeof(bitarray_t)) == 1);

    TEST_ASSERT(xBitArrayGetBit(ba, 32));

    vBitArrayFree(ba);
}

RS_TEST_CASE(BitArrayLarge, "Tests on a larger bit array")
{
    bitarray_t *ba;

    ba = pxBitArrayAlloc(10000);

    for (int i = 0; i < 10000; i++)
        vBitArraySetBit(ba, i);

    for (int i = 0; i < (10000 / 32) - 1; i++)
        TEST_ASSERT(*(uint32_t *)((void *)ba + sizeof(bitarray_t) + i * sizeof(uint32_t)) == -1);

    vBitArrayFree(ba);
}

#ifndef TEST_CASE
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_BitArrayBasics);
    RUN_TEST(test_BitArrayLarge);
    return UNITY_END();
}
#endif
