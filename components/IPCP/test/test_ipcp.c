#include <unistd.h>
#include <string.h>

#include "ARP826_defs.h"
#include "BufferManagement.h"
#include "IPCP_api.h"
#include "IPCP_events.h"
#include "NetworkInterface.h"
#include "NetworkInterface_mq.h"
#include "configSensor.h"
#include "portability/port.h"

#include "rina_buffers.h"
#include "unity.h"
#include "unity_fixups.h"

RS_TEST_CASE_SETUP(test_ipcp) {}

RS_TEST_CASE_TEARDOWN(test_ipcp) {}

RS_TEST_CASE(IPCPSendEvent, "[ipcp]")
{
    RINAStackEvent_t ev;
    NetworkBufferDescriptor_t *buf;
    const char txt[] = "hello world";
    long nb;

    RS_TEST_CASE_BEGIN(test_ipcp);

    TEST_ASSERT((buf = pxGetNetworkBufferWithDescriptor(strlen(txt), 100)) != NULL);
    memcpy(buf->pucEthernetBuffer, txt, sizeof(txt));

    ev.eEventType = eNetworkTxEvent;
    ev.pvData = buf;

    /* Send the bogus data */
    TEST_ASSERT(xSendEventStructToIPCPTask(&ev, 100));

    sleep(1);

    /* Check that there is something to read in the outbound
     * queue. There will more than the bogus data but we're happy if
     * there is anything to read */
    nb = xMqNetworkInterfaceOutputCount();
    LOGD("[test_ipcp]", "Number of messages in queue: %ld", nb);
    TEST_ASSERT(nb >= 1);

    RS_TEST_CASE_END(test_ipcp);
}

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();
    RS_RUN_TEST(IPCPSendEvent);
    return UNITY_END();
}
#endif // TEST_CASE
