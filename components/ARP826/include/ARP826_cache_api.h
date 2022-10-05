#ifndef _COMPONENTS_ARP826_CACHE_API_H
#define _COMPONENTS_ARP826_CACHE_API_H

#include "common/mac.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "common/rina_gpha.h"

#include "ARP826_cache_defs.h"

struct ARPCache *pxARPCacheCreate(eGHAType_t eType, num_t nEntries);

void vARPPrintCache(struct ARPCache *pxCache);

void vARPRemoveAll(struct ARPCache *pxCache);

void vARPRefreshCacheEntry(gpa_t *pxGpa, gha_t *pxMACAddress);

void vARPRemoveCacheEntry(const struct ARPCache *pxCache,
                          const gpa_t *pxGpa,
                          const gha_t *pxMACAddress);

eARPLookupResult_t eARPCacheLookupGPA(const struct ARPCache *pxCache,
                                      const gpa_t *pxGpaToLookup,
                                      const gha_t **pxGhaResult);

const gha_t *pxARPCacheLookupGHA(struct ARPCache *pxCache, const gpa_t *pxPa);

bool_t xARPCacheRemoveHandle(struct ARPCache *pxCache, ARPCacheHandle nCacheHndl);

bool_t xARPCacheRemove(struct ARPCache *pxCache, const gpa_t *pxPa);

ARPCacheHandle xARPCacheAdd(struct ARPCache *pxCache, const gpa_t *pxPa, const gha_t *pxHa);

void vARPCacheRefresh(struct ARPCache *pxCache, const gpa_t *pxPa, const gha_t *pxHa);

#ifdef __cplusplus
}
#endif

#endif // _COMPONENTS_ARP826_CACHE_API_H
