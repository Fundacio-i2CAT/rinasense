#include <string.h>

#include "common/mac.h"
#include "common/rina_gpha.h"

#include "ARP826_cache_defs.h"
#include "ARP826_cache_api.h"

#include "unity.h"
#include "common/unity_fixups.h"

struct ARPCache *pxCache;

const MACAddress_t mac1 = {
    {1, 2, 3, 4, 5, 6}
};

const MACAddress_t mac2 = {
    {2, 3, 4, 5, 6, 7}
};

RS_TEST_CASE_SETUP(test_arp826_cache)
{
    TEST_ASSERT((pxCache = pxARPCacheCreate(MAC_ADDR_802_3, 2)) != NULL);
}

RS_TEST_CASE_TEARDOWN(test_arp826_cache) {}

RS_TEST_CASE(ARPCacheAddRemove, "[arp]")
{
    gpa_t *gpa1, *gpa2;
    gha_t *gha1, *gha2;
    string_t addr1 = "1|2|3|4";
    string_t addr2 = "2|3|4|5";
    eARPLookupResult_t r;

    RS_TEST_CASE_BEGIN(test_arp826_cache);

    /* Make sure we can lookup a GPA in the cache. */
    TEST_ASSERT((gpa1 = pxCreateGPA((buffer_t)addr1, strlen(addr1))) != NULL);
    TEST_ASSERT((gha1 = pxCreateGHA(MAC_ADDR_802_3, &mac1)) != NULL);
    vARPPrintCache(pxCache);
    TEST_ASSERT(xARPCacheAdd(pxCache, gpa1, gha1) != ARP_CACHE_INVALID_HANDLE);
    vARPPrintCache(pxCache);
    TEST_ASSERT((r = eARPCacheLookupGPA(pxCache, gpa1, NULL)) == eARPCacheHit);

    /* Make sure a unknown GPA will return a cache miss. */
    TEST_ASSERT((gpa2 = pxCreateGPA((buffer_t)addr2, strlen(addr2))) != NULL);
    TEST_ASSERT((r = eARPCacheLookupGPA(pxCache, gpa2, NULL)) == eARPCacheMiss);

    /* Make sure we can remove an entry from the cache. */
    xARPCacheRemove(pxCache, gpa1);
    TEST_ASSERT_EQUAL_INT(eARPCacheMiss, eARPCacheLookupGPA(pxCache, gpa1, NULL));

    RS_TEST_CASE_END(test_arp826_cache);
}


#ifndef TEST_CASE
int main() {
    RS_SUITE_BEGIN();

    RS_RUN_TEST(ARPCacheAddRemove);

    RS_SUITE_END();
}
#endif
