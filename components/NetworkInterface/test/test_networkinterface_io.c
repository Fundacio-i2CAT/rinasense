#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "NetworkInterface_mq.h"
#include "portability/port.h"
#include "IPCP_frames.h"
#include "IPCP_api.h"
#include "mock_IPCP.h"

#include "unity.h"
#include "unity_fixups.h"

const MACAddress_t mac = {
    {1, 2, 3, 4, 5, 6}
};

struct timespec ts = { 0 };

/* Test that we can write to the MQ underlying the NetworkInterface
 * and have the NetworkInterface API get triggered. */
void testMqInterfaceRead()
{
    char dataIn[1500] = "12345";
    RINAStackEvent_t *ev;

    TEST_ASSERT((xMqNetworkInterfaceWriteInput(dataIn, sizeof(dataIn))));
    TEST_ASSERT((ev = pxMockGetLastSentEvent()) != NULL);
    TEST_ASSERT(ev->eEventType == eNetworkRxEvent);
}

/* Test that we can write to the NetworkInterface API and get the
 * output. */
void testMqInterfaceWrite()
{
    NetworkBufferDescriptor_t *netbuf;
    string_t dataIn = "12345";
    char dataOut[1500];

    TEST_ASSERT((netbuf = pxGetNetworkBufferWithDescriptor(5, &ts)) != NULL);
    memcpy(netbuf->pucEthernetBuffer, dataIn, strlen(dataIn));
    TEST_ASSERT(xNetworkInterfaceOutput(netbuf, false));
    TEST_ASSERT(xMqNetworkInterfaceReadOutput(dataOut, sizeof(dataOut)) > 0);
    TEST_ASSERT(dataOut[3] == dataIn[3]);
}

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();

    TEST_ASSERT(xMockIPCPInit());
    TEST_ASSERT(xNetworkBuffersInitialise());
    TEST_ASSERT(xNetworkInterfaceInitialise(&mac));
    TEST_ASSERT(xNetworkInterfaceConnect());

    RUN_TEST(testMqInterfaceWrite);
    RUN_TEST(testMqInterfaceRead);

    TEST_ASSERT(xNetworkInterfaceDisconnect());
    vMockIPCPClean();

    exit(UNITY_END());
}
#endif
