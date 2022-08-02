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
    size_t s = 10, n = 0;
    NetworkBufferDescriptor_t *pB1, *pB2;
    struct timespec ts;

    RS_TEST_CASE_BEGIN(test_buffer_management);

    /* Make sure we can randomly allocate 2 buffers with descriptors. */
    n = uxGetNumberOfFreeNetworkBuffers();
    TEST_ASSERT((pB1 = pxGetNetworkBufferWithDescriptor(s, 1000)) != NULL);
    TEST_ASSERT((pB2 = pxGetNetworkBufferWithDescriptor(s, 1000)) != NULL);
    TEST_ASSERT(uxGetNumberOfFreeNetworkBuffers() == n - 2);

    /* Free those buffers. */
    vReleaseNetworkBufferAndDescriptor(pB1);
    vReleaseNetworkBufferAndDescriptor(pB2);

    /* Make sure things have been updated. */
    TEST_ASSERT(uxGetNumberOfFreeNetworkBuffers() == n);

    RS_TEST_CASE_END(test_buffer_management);
}

RS_TEST_CASE(NetworkBufferMassAllocate, "[buffer]")
{
    NetworkBufferDescriptor_t *pB[NUM_NETWORK_BUFFER_DESCRIPTORS + 1];
    size_t s = 10;
    int n;

    RS_TEST_CASE_BEGIN(test_buffer_management);

    /* Allocate the maximum amount of buffers. */
    n = uxGetNumberOfFreeNetworkBuffers();
    for (int i = 0; i < n; i++)
        TEST_ASSERT((pB[i] = pxGetNetworkBufferWithDescriptor(s, 1000)) != NULL);

    /* This should fail. */
    TEST_ASSERT((pB[n] = pxGetNetworkBufferWithDescriptor(s, 10)) == NULL);

    /* Free a single buffer so we can retry. */
    vReleaseNetworkBufferAndDescriptor(pB[0]);

    /* This should succeed now. */
    TEST_ASSERT((pB[n] = pxGetNetworkBufferWithDescriptor(s, 10)) != NULL);

    /* Release all the buffers we just taken. */
    for (int i = 0; i < n; i++)
        vReleaseNetworkBufferAndDescriptor(pB[i]);

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
