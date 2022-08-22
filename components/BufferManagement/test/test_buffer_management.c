#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

#include "BufferManagement.h"
#include "portability/port.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_buffer_management) {}
RS_TEST_CASE_TEARDOWN(test_buffer_management) {}

/*void testGetNetworkBuffer()*/
RS_TEST_CASE(GetNetworkBuffer, "[buffer]")
{
    size_t s = 10;
    uint8_t *p1, *p2, *p3;
    NetworkBufferDescriptor_t *pB1, *pB2;
    struct timespec ts;

    RS_TEST_CASE_BEGIN(test_buffer_management);

    TEST_ASSERT((p1 = pucGetNetworkBuffer(&s)) != NULL);
    TEST_ASSERT((p2 = pucGetNetworkBuffer(&s)) != NULL);
    TEST_ASSERT((p3 = pucGetNetworkBuffer(&s)) != NULL);

    RS_TEST_CASE_END(test_buffer_management);
}

/*void testGetNetworkBufferWithDescriptors()*/
RS_TEST_CASE(GetNetworkBufferWithDescriptor, "[buffer]")
{
    size_t s = 10;
    NetworkBufferDescriptor_t *pB1, *pB2;
    struct timespec ts;

    RS_TEST_CASE_BEGIN(test_buffer_management);

    TEST_ASSERT((pB1 = pxGetNetworkBufferWithDescriptor(s, 1000)) != NULL);
    TEST_ASSERT((pB2 = pxGetNetworkBufferWithDescriptor(s, 1000)) != NULL);

    RS_TEST_CASE_END(test_buffer_management);
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
