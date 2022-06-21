#include <stdio.h>

#include "ARP826.h"
#include "configSensor.h"
#include "rina_gpha.h"
#include "IPCP_api.h"
#include "NetworkInterface.h"
#include "BufferManagement.h"

const ARPPacket_t p1 = {
    // Ethernet Header
    {
        { 0, 0, 0, 0, 0 }, /* xDestinationAddress */
        { 0, 0, 0, 0, 1 }, /* xSourceAddress */
        RsHtoNS(ETH_P_ARP) /* usFrameType */
    },

    // ARP header
    {
        RsHtoNS(0x0001),     /* usHType */
        RsHtoNS(ETH_P_RINA), /* usPType */
        6,                   /* ucHALLength */
        8,                   /* ucPALLength */
        ARP_REQUEST          /* usOperation */
    }
};

const MACAddress_t mac1 = {
    1, 2, 3, 4, 5, 6
};

const MACAddress_t mac2 = {
    2, 3, 4, 5, 6, 7
};

void testARPCache() {
    gpa_t *gpa1, *gpa2;
    gha_t *gha1, *gha2;
    string_t addr1 = "1|2|3|4";
    string_t addr2 = "2|3|4|5";
    struct rinarpHandle_t *rh;
    eARPLookupResult_t r;

    // Make sure we can lookup a GPA in the cache.
    RsAssert((gpa1 = pxCreateGPA(addr1, sizeof(addr1))) != NULL);
    RsAssert((gha1 = pxCreateGHA(MAC_ADDR_802_3, &mac1)) != NULL);
    RsAssert((rh = pxARPAdd(gpa1, gha1)) != NULL);
    vARPPrintCache();
    RsAssert((r = eARPLookupGPA(gpa1)) == eARPCacheHit);
    //RsAssert((gha2 = pxARPLookupGHA(gpa1)) != NULL);
    //RsAssert(gha2 != gha1);

    // Make sure a unknown GPA will return a cache miss.
    RsAssert((gpa2 = pxCreateGPA(addr2, sizeof(addr2))) != NULL);
    RsAssert((r = eARPLookupGPA(gpa2)) == eARPCacheMiss);

    // Make sure we can remove an entry from the cache.
    vARPRemoveCacheEntry(gpa1, gha1);
    RsAssert((r = eARPLookupGPA(gpa1)) == eARPCacheMiss);
}

void testARPSendRequest() {
    gpa_t *gpaSrc, *gpaTrg;
    gha_t *ghaSrc;
    string_t addr1 = "1|2|3|4";
    string_t addr2 = "2|3|4|5";
    RINAStackEvent_t *ev;
    NetworkBufferDescriptor_t *buf;

    RsAssert((gpaSrc = pxCreateGPA(addr1, sizeof(addr1))) != NULL);
    RsAssert((gpaTrg = pxCreateGPA(addr2, sizeof(addr2))) != NULL);
    RsAssert((ghaSrc = pxCreateGHA(MAC_ADDR_802_3, &mac1)) != NULL);

    vMockClearLastSentEvent();
    vMockClearLastBufferOutput();
    vMockSetIsCallingFromIPCPTask(false);

    /* See if the ARP module send the packet through the stack */
    RsAssert(vARPSendRequest(gpaTrg, gpaSrc, ghaSrc));
    RsAssert((ev = pxMockGetLastSentEvent()) != NULL);
    RsAssert(ev->eEventType == eNetworkTxEvent);
    RsAssert(ev->pvData != NULL);

    /* Pretend that we send the ARP request as the IPCP. This is
     * another code path */
    vMockClearLastSentEvent();
    vMockClearLastBufferOutput();
    vMockSetIsCallingFromIPCPTask(true);
    RsAssert(vARPSendRequest(gpaTrg, gpaSrc, ghaSrc));
    RsAssert(pxMockGetLastSentEvent() == NULL);
    RsAssert((buf = pxMockGetLastBufferOutput()) != NULL);
}

int main() {
    vARPInitCache();
    RsAssert(xMockIPCPInit());
    RsAssert(xNetworkBuffersInitialise());

    testARPCache();
    testARPSendRequest();

    vMockIPCPClean();

    return 0;
}
