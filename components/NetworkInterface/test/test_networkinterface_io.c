#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "NetworkInterface.h"
#include "NetworkInterface_mq.h"
#include "portability/port.h"
#include "IPCP_frames.h"
#include "IPCP_api.h"
#include "mock_IPCP.h"

#include "unity.h"
#include "common/unity_fixups.h"

const MACAddress_t mac = {
    {1, 2, 3, 4, 5, 6}
};

RS_TEST_CASE_SETUP(test_networkinterface_io) {
    TEST_ASSERT(xMockIPCPInit());
    TEST_ASSERT(xNetworkBuffersInitialise());
    TEST_ASSERT(xNetworkInterfaceInitialise(&mac));
    TEST_ASSERT(xNetworkInterfaceConnect());

    sleep(1);
}

RS_TEST_CASE_TEARDOWN(test_networkinterface_io) {
    TEST_ASSERT(xNetworkInterfaceDisconnect());
    vMockIPCPClean();
}

/* Test that we can write to the MQ underlying the NetworkInterface
 * and have the NetworkInterface API get triggered. */
RS_TEST_CASE(MqInterfaceRead, "[mq]")
{
    char dataIn[1500] = "12345";
    RINAStackEvent_t *ev;

    RS_TEST_CASE_BEGIN(test_networkinterface_io);

    TEST_ASSERT((xMqNetworkInterfaceWriteInput(dataIn, sizeof(dataIn))));
    TEST_ASSERT((ev = pxMockGetLastSentEvent()) != NULL);
    TEST_ASSERT(ev->eEventType == eNetworkRxEvent);

    RS_TEST_CASE_END(test_networkinterface_io);
}

/* Test that we can write to the NetworkInterface API and get the
 * output. */
RS_TEST_CASE(MqInterfaceWrite, "[mq]")
{
    NetworkBufferDescriptor_t *netbuf;
    string_t dataIn = "12345";
    char dataOut[1500];

    RS_TEST_CASE_BEGIN(test_networkinterface_io);

    xMqNetworkInterfaceOutputDiscard();

    TEST_ASSERT((netbuf = pxGetNetworkBufferWithDescriptor(5, 0)) != NULL);
    memcpy(netbuf->pucEthernetBuffer, dataIn, strlen(dataIn));
    TEST_ASSERT(xNetworkInterfaceOutput(netbuf, false));
    TEST_ASSERT(xMqNetworkInterfaceReadOutput(dataOut, sizeof(dataOut), NULL) > 0);
    TEST_ASSERT(dataOut[3] == dataIn[3]);

    RS_TEST_CASE_END(test_networkinterface_io);
}

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();
    RS_RUN_TEST(MqInterfaceWrite);
    RS_RUN_TEST(MqInterfaceRead);
    exit(UNITY_END());
}
#endif
