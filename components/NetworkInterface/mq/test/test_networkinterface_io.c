#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "NetworkInterface_mq.h"
#include "portability/port.h"
#include "IPCP_api.h"

const MACAddress_t mac = {
    1, 2, 3, 4, 5, 6
};

struct timespec ts = { 0 };

/* Test that we can write to the MQ underlying the NetworkInterface
 * and have the NetworkInterface API get triggered. */
void testMqInterfaceRead() {
    char dataIn[1500] = "12345";
    RINAStackEvent_t *ev;

    RsAssert((xMqNetworkInterfaceWriteInput(dataIn, sizeof(dataIn))));
    RsAssert((ev = pxMockGetLastSentEvent()) != NULL);
    RsAssert(ev->eEventType == eNetworkRxEvent);
}

/* Test that we can write to the NetworkInterface API and get the
 * output. */
void testMqInterfaceWrite()
{
    NetworkBufferDescriptor_t *netbuf;
    string_t dataIn = "12345";
    char dataOut[1500];

    RsAssert((netbuf = pxGetNetworkBufferWithDescriptor(5, &ts)) != NULL);
    memcpy(netbuf->pucEthernetBuffer, dataIn, sizeof(dataIn));
    RsAssert(xNetworkInterfaceOutput(netbuf, false));
    RsAssert(xMqNetworkInterfaceReadOutput(dataOut, sizeof(dataOut)) > 0);
    RsAssert(dataOut[3] == dataIn[3]);
}

void main() {
    RsAssert(xMockIPCPInit());
    RsAssert(xNetworkBuffersInitialise());
    RsAssert(xNetworkInterfaceInitialise(&mac));
    RsAssert(xNetworkInterfaceConnect());

    testMqInterfaceWrite();
    testMqInterfaceRead();

    RsAssert(xNetworkInterfaceDisconnect());

    vMockIPCPClean();

    exit(0);
}
