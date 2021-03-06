#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

#include "BufferManagement.h"
#include "portability/port.h"

#include "unity.h"
#include "unity_fixups.h"

/*void testGetNetworkBuffer()*/
RS_TEST_CASE(GetNetworkBuffer, "[buffer]")
{
    size_t s = 10;
    uint8_t *p1, *p2, *p3;
    NetworkBufferDescriptor_t *pB1, *pB2;
    struct timespec ts;

    TEST_ASSERT((p1 = pucGetNetworkBuffer(&s)) != NULL);
    TEST_ASSERT((p2 = pucGetNetworkBuffer(&s)) != NULL);
    TEST_ASSERT((p3 = pucGetNetworkBuffer(&s)) != NULL);
}

/*void testGetNetworkBufferWithDescriptors()*/
RS_TEST_CASE(GetNetworkBufferWithDescriptor, "[buffer]")
{
    int n;
    size_t s = 10;
    NetworkBufferDescriptor_t *pB1, *pB2;
    struct timespec ts;

    n = rstime_waitmsec(&ts, 10);
    TEST_ASSERT_EQUAL_INT(true, n);
    TEST_ASSERT_EQUAL_INT(0, errno);

    TEST_ASSERT((pB1 = pxGetNetworkBufferWithDescriptor(s, &ts)) != NULL);
    TEST_ASSERT((pB2 = pxGetNetworkBufferWithDescriptor(s, &ts)) != NULL);
}

#ifndef TEST_CASE
int main() {
    xNetworkBuffersInitialise();

    UNITY_BEGIN();
    RUN_TEST(test_GetNetworkBuffer);
    RUN_TEST(test_GetNetworkBufferWithDescriptor);
    return UNITY_END();
}
#endif
