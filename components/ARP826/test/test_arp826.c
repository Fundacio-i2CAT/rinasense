#include <stdio.h>
#include <string.h>

#include "ARP826.h"
#include "configSensor.h"
#include "rina_gpha.h"
#include "IPCP_api.h"
#include "mock_IPCP.h"
#include "mock_NetworkInterface.h"
#include "NetworkInterface.h"
#include "BufferManagement.h"

#include "unity.h"
#include "unity_fixups.h"

ARPPacket_t p1 = {
    /* Ethernet Header */
    {
        {{ 0, 0, 0, 0, 0 }}, /* xDestinationAddress */
        {{ 0, 0, 0, 0, 1 }}, /* xSourceAddress */
        0                    /* usFrameType */
    },

    /* ARP header */
    {
        0,                   /* usHType */
        0,                   /* usPType */
        6,                   /* ucHALLength */
        8,                   /* ucPALLength */
        ARP_REQUEST          /* usOperation */
    }
};

const MACAddress_t mac1 = {
    {1, 2, 3, 4, 5, 6}
};

const MACAddress_t mac2 = {
    {2, 3, 4, 5, 6, 7}
};

RS_TEST_CASE_SETUP(test_arp826)
{
    /* Initialize the test packet. */
    p1.xEthernetHeader.usFrameType = RsHtoNS(ETH_P_ARP);
    p1.xARPHeader.usHType = RsHtoNS(0x0001);
    p1.xARPHeader.usPType = RsHtoNS(ETH_P_RINA);

    vARPInitCache();
    xMockIPCPInit();
    xNetworkBuffersInitialise();
}

RS_TEST_CASE_TEARDOWN(test_arp826)
{
    vMockIPCPClean();
}

RS_TEST_CASE(ARPCache, "ARP -- ARP cache manipulations")
{
    gpa_t *gpa1, *gpa2;
    gha_t *gha1, *gha2;
    string_t addr1 = "1|2|3|4";
    string_t addr2 = "2|3|4|5";
    struct rinarpHandle_t *rh;
    eARPLookupResult_t r;

    RS_TEST_CASE_BEGIN(test_arp826);

    /* Make sure we can lookup a GPA in the cache. */
    TEST_ASSERT((gpa1 = pxCreateGPA((buffer_t)addr1, strlen(addr1))) != NULL);
    TEST_ASSERT((gha1 = pxCreateGHA(MAC_ADDR_802_3, &mac1)) != NULL);
    vARPPrintCache();
    TEST_ASSERT((rh = pxARPAdd(gpa1, gha1)) != NULL);
    vARPPrintCache();
    TEST_ASSERT((r = eARPLookupGPA(gpa1)) == eARPCacheHit);
    //RsAssert((gha2 = pxARPLookupGHA(gpa1)) != NULL);
    //RsAssert(gha2 != gha1);

    /* Make sure a unknown GPA will return a cache miss. */
    TEST_ASSERT((gpa2 = pxCreateGPA((buffer_t)addr2, strlen(addr2))) != NULL);
    TEST_ASSERT((r = eARPLookupGPA(gpa2)) == eARPCacheMiss);

    /* Make sure we can remove an entry from the cache. */
    vARPRemoveCacheEntry(gpa1, gha1);
    TEST_ASSERT_EQUAL_INT(eARPCacheMiss, eARPLookupGPA(gpa1));

    RS_TEST_CASE_END(test_arp826);
}

RS_TEST_CASE(ARPSendRequest, "ARP -- Sending request")
{
    gpa_t *gpaSrc, *gpaTrg;
    gha_t *ghaSrc;
    string_t addr1 = "1|2|3|4";
    string_t addr2 = "2|3|4|5";
    RINAStackEvent_t *ev;
    NetworkBufferDescriptor_t *buf;

    RS_TEST_CASE_BEGIN(test_arp826);

    TEST_ASSERT((gpaSrc = pxCreateGPA((buffer_t)addr1, strlen(addr1))) != NULL);
    TEST_ASSERT((gpaTrg = pxCreateGPA((buffer_t)addr2, strlen(addr2))) != NULL);
    TEST_ASSERT((ghaSrc = pxCreateGHA(MAC_ADDR_802_3, &mac1)) != NULL);

    vMockClearLastSentEvent();
    vMockClearLastBufferOutput();
    vMockSetIsCallingFromIPCPTask(false);

    /* See if the ARP module send the packet through the stack */
    TEST_ASSERT(vARPSendRequest(gpaTrg, gpaSrc, ghaSrc));
    TEST_ASSERT((ev = pxMockGetLastSentEvent()) != NULL);
    TEST_ASSERT(ev->eEventType == eNetworkTxEvent);
    TEST_ASSERT(ev->pvData != NULL);

    /* Pretend that we send the ARP request as the IPCP. This is
     * another code path */
    vMockClearLastSentEvent();
    vMockClearLastBufferOutput();
    vMockSetIsCallingFromIPCPTask(true);
    TEST_ASSERT(vARPSendRequest(gpaTrg, gpaSrc, ghaSrc));
    TEST_ASSERT(pxMockGetLastSentEvent() == NULL);
    TEST_ASSERT((buf = pxMockGetLastBufferOutput()) != NULL);

    RS_TEST_CASE_END(test_arp826)
}

#ifndef TEST_CASE
int main() {
    UNITY_BEGIN();
    RS_RUN_TEST(ARPCache);
    RS_RUN_TEST(ARPSendRequest);
    return UNITY_END();
}
#endif
