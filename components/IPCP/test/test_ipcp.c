#include "ARP826_defs.h"
#include "IPCP_api.h"
#include "IPCP_events.h"
#include "portability/port.h"

#include "unity.h"
#include "unity_fixups.h"
#include <unistd.h>

RS_TEST_CASE_SETUP(test_ipcp) {}
RS_TEST_CASE_TEARDOWN(test_ipcp) {}

/* EthernetHeader_t pkt1 = { */
/*     /\* Ethernet Header *\/ */
/*     { */
/*         {{ 0, 0, 0, 0, 0 }}, /\* xDestinationAddress *\/ */
/*         {{ 0, 0, 0, 0, 1 }}, /\* xSourceAddress *\/ */
/*         0                    /\* usFrameType *\/ */
/*     } */
/*     }; */


RS_TEST_CASE(IPCPSendEvent, "[ipcp]")
{
    RINAStackEvent_t ev;
    char *buf;

    //    buf = pxGetNetworkBufferWithDescriptor(

    RS_TEST_CASE_BEGIN(test_ipcp);

    ev.eEventType = eNetworkTxEvent;
    ev.pvData = buf;

    TEST_ASSERT(xSendEventStructToIPCPTask(&ev, 100));

    sleep(1);

    RS_TEST_CASE_END(test_ipcp);
}

#ifndef TEST_CASE
int main()
{
    RINA_IPCPInit();

    sleep(2);

    UNITY_BEGIN();
    RS_RUN_TEST(IPCPSendEvent);
    return UNITY_END();
}
#endif // TEST_CASE
