#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "NetworkInterface.h"
#include "NetworkInterface_mq.h"
#include "common/netbuf.h"
#include "common/rsrc.h"
#include "portability/port.h"
#include "IPCP_frames.h"
#include "IPCP_api.h"
#include "mock_IPCP.h"

#include "unity.h"
#include "common/unity_fixups.h"

MACAddress_t mac = {
    {1, 2, 3, 4, 5, 6}
};

rsrcPoolP_t xPool;

netbuf_t *pxNb;

void mqHandler(struct ipcpInstance_t *pxItsNull, netbuf_t *px)
{
    pxNb = px;
}

RS_TEST_CASE_SETUP(test_networkinterface_io) {
    TEST_ASSERT(xMockIPCPInit());
    TEST_ASSERT((xPool = xNetBufNewPool("test_networkinterface_io")));
    TEST_ASSERT(xNetworkInterfaceInitialise(NULL, &mac, xPool));
    TEST_ASSERT(xNetworkInterfaceConnect());

    xMqNetworkInterfaceSetHandler(&mqHandler);

    usleep(200000);
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
    char dataOut[1500];
    RINAStackEvent_t *ev;

    RS_TEST_CASE_BEGIN(test_networkinterface_io);

    xMqNetworkInterfaceWriteInput(dataIn, sizeof(dataIn));

    usleep(2000000);

    TEST_ASSERT(pxNb != NULL);
    TEST_ASSERT(unNetBufRead(pxNb, dataOut, 0, sizeof(dataOut)) > 0);
    TEST_ASSERT(memcmp(dataIn, dataOut, strlen(dataIn)) == 0);

    RS_TEST_CASE_END(test_networkinterface_io);
}

/* Test that we can write to the NetworkInterface API and get the
 * output. */
RS_TEST_CASE(MqInterfaceWrite, "[mq]")
{
    netbuf_t *pxNb;
    string_t dataIn = "12345";
    char dataOut[1500];

    RS_TEST_CASE_BEGIN(test_networkinterface_io);

    xMqNetworkInterfaceOutputDiscard();

    TEST_ASSERT((pxNb = pxNetBufNew(xPool, NB_UNKNOWN, (buffer_t)dataIn, sizeof(dataIn), NETBUF_FREE_DONT)));
    TEST_ASSERT(xNetworkInterfaceOutput(pxNb));
    TEST_ASSERT(xMqNetworkInterfaceReadOutput(dataOut, sizeof(dataOut), NULL) > 0);
    TEST_ASSERT(dataOut[3] == dataIn[3]);

    RS_TEST_CASE_END(test_networkinterface_io);
}

#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(MqInterfaceWrite);
    RS_RUN_TEST(MqInterfaceRead);
    RS_SUITE_END();
}
#endif
