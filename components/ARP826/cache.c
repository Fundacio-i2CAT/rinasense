#include <limits.h>
#include <string.h>

/* Code for a RINA/ARP cache that aims to be independent of the
 * hardware protocol. */

#include "ARP826_cache_defs.h"
#include "ARP826_cache_api.h"

#include "common/mac.h"
#include "common/rina_gpha.h"
#include "common/rsrc.h"
#include "portability/rsmem.h"

gha_t xBroadcastHa = {
    .xType = MAC_ADDR_802_3,
    .xAddress.ucBytes = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
};

/**
 * Structure for one row in the ARP cache table.
 */
struct ARPCacheRow {
    /**< The IPCP address of an ARP cache entry. */
    gpa_t *pxPa;

    /**< The MAC address of an ARP cache entry. */
    gha_t *pxHa;

    uint8_t ucAge;
    uint8_t ucValid;
};

/* Cache object data. */
struct ARPCache {
    /* Number of entries in the cache. */
    num_t nEntries;

    /* Type of hardware address this cache holds. */
    eGHAType_t eType;

    /* The set of cache rows. */
    struct ARPCacheRow *pxRows;
};

struct ARPCache *pxARPCacheCreate(eGHAType_t eType, num_t nEntries)
{
    rsrcPoolP_t xPool;
    struct ARPCacheRow *pxRows;
    struct ARPCache *pxCache;
    size_t unMem;

    unMem = sizeof(struct ARPCache) + (sizeof(struct ARPCacheRow) * nEntries);

    if (!(pxCache = pvRsMemAlloc(unMem))) {
        LOGE(TAG_ARP, "Failed to allocate memory for ARP cache");
        return NULL;
    }

    /* The ARP cache rows will be after the cache struct, avoid an
     * extra allocations. */
    pxCache->pxRows = (void *)pxCache + sizeof(struct ARPCache);
    pxCache->nEntries = nEntries;
    pxCache->eType = eType;

    /* Reset the cache to 0. */
    vARPRemoveAll(pxCache);

	LOGI(TAG_ARP, "ARP Cache create");

    return pxCache;
}

void prvARPCacheFreeRow(struct ARPCacheRow *pxRow)
{
    if (pxRow->pxHa)
        vGHADestroy(pxRow->pxHa);

    if (pxRow->pxPa)
        vGPADestroy(pxRow->pxPa);

    memset(pxRow, 0, sizeof(struct ARPCacheRow));
}

void vARPPrintMACAddress(const gha_t *pxGha)
{
	LOGI(TAG_ARP, "MACAddress: %02x:%02x:%02x:%02x:%02x:%02x\n",
         pxGha->xAddress.ucBytes[0],
		 pxGha->xAddress.ucBytes[1],
		 pxGha->xAddress.ucBytes[2],
		 pxGha->xAddress.ucBytes[3],
		 pxGha->xAddress.ucBytes[4],
		 pxGha->xAddress.ucBytes[5]);
}

/**
 * @brief A call to this function will clear the all ARP cache.
 */
void vARPRemoveAll(struct ARPCache *pxCache)
{
	memset(pxCache->pxRows, 0, sizeof(struct ARPCacheRow) * pxCache->nEntries);
}

void prvARPCachePrintRow(string_t sPrefix, struct ARPCacheRow *pxRow, int nIndex)
{
    stringbuf_t sAddr[25] = { 0 };

    memcpy(sAddr, pxRow->pxPa->pucAddress, pxRow->pxPa->uxLength);

    LOGD(TAG_ARP, "%s Entry %i: %3d - %s - %02x:%02x:%02x:%02x:%02x:%02x",
         sPrefix,
         nIndex,
         pxRow->ucAge,
         sAddr,
         pxRow->pxHa->xAddress.ucBytes[0],
         pxRow->pxHa->xAddress.ucBytes[1],
         pxRow->pxHa->xAddress.ucBytes[2],
         pxRow->pxHa->xAddress.ucBytes[3],
         pxRow->pxHa->xAddress.ucBytes[4],
         pxRow->pxHa->xAddress.ucBytes[5]);
}

ARPCacheHandle xARPCacheAdd(struct ARPCache *pxCache, const gpa_t *pxPa, const gha_t *pxHa)
{
    num_t i = 0;
    gpa_t *pxDupPa;
    gha_t *pxDupHa;

    /* Duplicate the names */
    if (!(pxDupPa = pxDupGPA(pxPa, true, 0))) {
        LOGE(TAG_ARP, "Failed to duplicate protocol address");
        return ARP_CACHE_INVALID_HANDLE;
    }

    if (!(pxDupHa = pxDupGHA(pxHa))) {
        LOGE(TAG_ARP, "Failed to duplicate the hardware address");
        return ARP_CACHE_INVALID_HANDLE;
    }

    for (i = 0; i < pxCache->nEntries; i++) {
        if (pxCache->pxRows[i].ucValid == 0) {
            pxCache->pxRows[i].ucAge = 1;
            pxCache->pxRows[i].ucValid = 1;
            pxCache->pxRows[i].pxHa = pxDupHa;
            pxCache->pxRows[i].pxPa = pxDupPa;

#ifndef NDEBUG
            prvARPCachePrintRow("New Entry: ", &pxCache->pxRows[i], i);
#endif

            return true;
        }
    }

    return false;
}

bool_t xARPCacheRemove(struct ARPCache *pxCache, const gpa_t *pxPa)
{
    num_t i;

    if (!xIsGPAOK(pxPa)) {
        LOGW(TAG_ARP, "Protocol address that was passed is invalid");
        return false;
    }

    for (i = 0; i < pxCache->nEntries; i++)
        if (pxCache->pxRows[i].ucValid && xGPACmp(pxCache->pxRows[i].pxPa, pxPa))
                prvARPCacheFreeRow(&pxCache->pxRows[i]);

    return true;
}

bool_t xARPCacheRemoveHandle(struct ARPCache *pxCache, ARPCacheHandle nCacheHndl)
{
    num_t i = 0;

    if (nCacheHndl < pxCache->nEntries)
        prvARPCacheFreeRow(&pxCache->pxRows[nCacheHndl]);

    return true;
}

void vARPPrintCache(struct ARPCache *pxCache)
{
	num_t i, xCount = 0;

    LOGD(TAG_ARP, "---- ARP CACHE START ----");

	for (i = 0; i < pxCache->nEntries; i++) {
		if ((pxCache->pxRows[i].pxPa != NULL) && (pxCache->pxRows[i].ucValid != 0)) {
            prvARPCachePrintRow("\t", &pxCache->pxRows[i], i);
            xCount++;
        }
	}

	LOGD(TAG_ARP, "ARP cache has %d entries", xCount);

    LOGD(TAG_ARP, "---- ARP CACHE END ----");
}

eARPLookupResult_t eARPCacheLookupGPA(const struct ARPCache *pxCache,
                                      const gpa_t *pxGpaToLookup,
                                      const gha_t **pxGhaResult)
{
	num_t x;
	eARPLookupResult_t eReturn = eARPCacheMiss;

	/* Loop through each entry in the ARP cache. */
	for (x = 0; x < pxCache->nEntries; x++)	{
        /* Skip empty entries. */
        if (pxCache->pxRows[x].ucAge == 0)
            continue;

#ifndef NDEBUG
        {
            string_t pcLoopAddr;

            RsAssert((pcLoopAddr = xGPAAddressToString(pxCache->pxRows[x].pxPa)) != NULL);
            LOGD(TAG_ARP, "eARPLookupGPA -> %s", pcLoopAddr);
            vRsMemFree(pcLoopAddr);
        }
#endif

        /* Compare the PA to the ARP cache key. */
		if (xGPACmp(pxCache->pxRows[x].pxPa, pxGpaToLookup)) {

			/* A matching valid entry was found, make sure it's valid. */
			if (pxCache->pxRows[x].ucValid == (uint8_t) false) {
				/* This entry is waiting an ARP reply, so is not valid. */
				eReturn = eCantSendPacket;

			} else {
				/* A valid entry was found. */
				eReturn = eARPCacheHit;

                if (pxGhaResult != NULL)
                    *pxGhaResult = pxCache->pxRows[x].pxHa;
			}

			break;
		}
	}

	return eReturn;
}

const gha_t *pxARPCacheLookupGHA(struct ARPCache *pxCache, const gpa_t *pxPa)
{
    num_t x;

    if (!xIsGPAOK(pxPa))
        return NULL;

    /* Loop through each entry in the ARP cache. */
    for (x = 0; x < pxCache->nEntries; x++)
        if (pxCache->pxRows[x].ucAge > 0)
            if (xGPACmp(pxCache->pxRows[x].pxPa, pxPa))
                return pxCache->pxRows[x].pxHa;

    return NULL;
}

/**
 * @brief Add/update the ARP cache entry MAC-address to IPCP-address mapping.
 */
void vARPCacheRefresh(struct ARPCache *pxCache, const gpa_t *pxPa, const gha_t *pxHa)
{
    num_t x = 0;
    num_t xIpcpEntry = -1;
    num_t xMacEntry = -1;
    num_t xUseEntry = 0;
    int8_t ucMinAgeFound = -1;

    /* For each entry in the ARP cache table. */
    for (x = 0; x < pxCache->nEntries; x++) // lookup in the cache for the MacAddress
    {
        bool_t xMatchingMAC;

        if (pxHa != NULL) {
            if (xGHACmp(pxCache->pxRows[x].pxHa, pxHa))
                xMatchingMAC = true;
            else
                xMatchingMAC = false;
        }
        else
            xMatchingMAC = false;

        /* Check if the protocol address is already in the cache */
        if (xGPACmp(pxCache->pxRows[x].pxPa, pxPa)) {
            if (pxHa == NULL) {
                /* In case the parameter pxMACAddress is NULL, an
                 * entry will be reserved to indicate that there is an
                 * outstanding ARPrequest. This will set the entry as
                 * invalid while there is an answer pending. */
                xIpcpEntry = x;
                break; // break an keep position to update the ARP request.
            }

            /* See if the MAC-address also matches. */
            if (xMatchingMAC != false) {
                /* This function will be called for each received
                 * packet As this is by far the most common path the
                 * coding standard is relaxed in this case and a
                 * return is permitted as an optimisation. */
                pxCache->pxRows[x].ucAge = MAX_ARP_AGE;
                pxCache->pxRows[x].ucValid = true;
                return;
            }

            /* Found an entry containing ulIPCPAddress, but the MAC
             * address doesn't match.  Might be an entry with
             * ucValid=false, waiting for an ARP reply.  Still want to
             * see if there is match with the given MAC
             * address.ucBytes.  If found, either of the two entries
             * must be cleared. */
            xIpcpEntry = x;
        }
        else if (xMatchingMAC != false)	{
            /* Found an entry with the given MAC-address, but the
             * IPCP-address is different.  Continue looping to find a
             * possible match with ulIPCPAddress. */
            xMacEntry = x;
        }

        /* _HT_
         * Shouldn't we test for xARPCache[ x ].ucValid == pdFALSE here ? */
        else if (pxCache->pxRows[x].ucAge < ucMinAgeFound) {
            /* As the table is traversed, remember the table row that
             * contains the oldest entry (the lowest age count, as ages are
             * decremented to zero) so the row can be re-used if this function
             * needs to add an entry that does not already exist. */
            ucMinAgeFound = pxCache->pxRows[x].ucAge;
            xUseEntry = x;
        }
    }

    if (xMacEntry >= 0)	{
        xUseEntry = xMacEntry;

        if (xIpcpEntry != CHAR_MAX)
            /* Both the MAC address as well as the IPCP address were found in
             * different locations: clear the entry which matches the
             * IPCP-address */
            prvARPCacheFreeRow(&pxCache->pxRows[xIpcpEntry]);
    }
    else if (xIpcpEntry != CHAR_MAX)
        /* An entry containing the IPCP-address was found, but it had a different MAC address */
        xUseEntry = xIpcpEntry;

    if (pxPa != NULL) {
        gpa_t *pxNewPa;

        pxNewPa = pxDupGPA(pxPa, true, 0);
        if (!pxNewPa) {
            LOGE(TAG_ARP, "Failed to allocate memory for cache refresh");
            return;
        }

        vGPADestroy(pxCache->pxRows[xUseEntry].pxPa);

        pxCache->pxRows[xUseEntry].pxPa = pxNewPa;
    }

    if (pxHa != NULL) {
        gha_t *pxNewHa;

        pxNewHa = pxDupGHA(pxHa);

        if (!pxNewHa) {
            LOGE(TAG_ARP, "Failed to allocate memory for hardware address during cache refresh");
            return;
        }

        vGHADestroy(pxCache->pxRows[xUseEntry].pxHa);

        pxCache->pxRows[xUseEntry].pxHa = pxNewHa;

        /* And this entry does not need immediate attention */
        pxCache->pxRows[xUseEntry].ucAge = (uint8_t)MAX_ARP_AGE;
        pxCache->pxRows[xUseEntry].ucValid = (uint8_t) true;
    }
    else if (xIpcpEntry < 0) {
        pxCache->pxRows[xUseEntry].ucAge = (uint8_t)MAX_ARP_RETRANSMISSIONS;
        pxCache->pxRows[xUseEntry].ucValid = (uint8_t) false;
    }

    LOGI(TAG_ARP, "ARP Cache Refreshed! ");
}

