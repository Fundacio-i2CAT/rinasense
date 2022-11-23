#ifndef _COMPONENTS_ARP826_CACHE_DEFS_H
#define _COMPONENTS_ARP826_CACHE_DEFS_H

#include "common/mac.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "common/rina_gpha.h"

#define ARP_CACHE_INVALID_HANDLE -1

typedef num_t ARPCacheHandle;

typedef enum
{
	eARPCacheMiss = 0, /* 0 An ARP table lookup did not find a valid entry. */
	eARPCacheHit,	   /* 1 An ARP table lookup found a valid entry. */
	eCantSendPacket	   /* 2 There is no IPCP address, or an ARP is still in progress, so the packet cannot be sent. */
} eARPLookupResult_t;

struct ARPCachePair;
struct ARPCacheRow;
struct ARPCache;

extern gha_t xBroadcastHa;

#ifdef __cplusplus
}
#endif

#endif // _COMPONENTS_ARP826_CACHE_DEFS_H
