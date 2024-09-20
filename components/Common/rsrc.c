/*
 *  rsrc.h
 *  PNA-server
 *
 *  Created by Steve Bunch on 4/26/2012
 *  Copyright 2012, 2022 TRIA Network Systems. See LICENSE.
 *
 */

#include <stdlib.h> // abort
#include <stddef.h> // offsetof
#include <string.h> // memset
#include "common/rsrc.h"

#include "portability/port.h"

/**
 * @brief Platform debug/print routines. DEBUGPRINTF normally nulled out.  logPrintf normally prints to stderr or equivalent.
 */
// #define DEBUGPRINTF					ESP_LOGI
#define DEBUGPRINTF(f, x...)
#define logPrintf LOGI

#define RESFREE_MAGIC ((void *)0xdeadbeef) // impossible value (detect double-free)

static const char *TAG = "rsrc";

/** ----------------------------------------------------------------------------------
 * @brief rsrcPools This is the list head for the list of all rsrcPool structs, and the initial pool.
 * @note Other than the rsrcPoolPool struct, all other rsrcPool structs are dynamically
 * allocated as resources using pxRsrcNewPool.  Because the rsrcPool struct is
 * private to this file, prviRsrcInitPool is a private function for now, but could be made public.
 *
 * NB: resource pool structs are NOT FREED back to the heap.  So you risk degrading
 * the heap with long-lived objects if you don't allocate them early, and extend in
 * reasonable-sized blocks instead of onesey-twosey's, which can fragment it badly.
 */
Link_t rsrcPools;
struct rsrcPool xRsrcPoolPool;

/** ----------------------------------------------------------------------------------
 * @brief vRsrcOOMfn Can be set to your own Out-Of-Memory handling function.  If NULL, OOM results in abort()
 * @param [in] pool that the request was made to
 * @param [in] number of resources that were requested form the pool
 * @note If the user function returns at all, the request for allocation of a pool or resource will return NULL.
 * If there is no user function, the pool name and number of resources are printed before abort()'ing
 */
void (*vRsrcOOMfn)(rsrcPoolP_t, int);
static int prviRsrcOOM(rsrcPoolP_t pxPool, unsigned int uiRequest); // internal OOM routine. forward

/** ----------------------------------------------------------------------------------
 * @brief eRsrcClearResOnAlloc Debug/test feature: set to force new resources to be cleared/set on alloc/free
 * @note Similar to using SetToOnes/Zeros helper functions for alloc/free, only global for all allocation/free operations.
 * Can be set to rsrcNOCLEAR for highest efficiency after debugging if CPU cycles are tight.  Note that any user-suppied
 * alloc/free helpers are executed AFTER clearing on alloc, and BEFORE clearing on free.
 */
enum rsrcClearOnAllocSetting_t eRsrcClearResOnAlloc = rsrcCLEAR_ON_BOTH;

// forwards
static int prviRsrcAdd2Pool(rsrcPoolP_t pxPool, unsigned int uiCount, size_t xRsrcsize);
static int prviRsrcInitPool(rsrcPoolP_t pxPool, const char *pcName, size_t xRsrcsize,
							unsigned int uiInitalloc, unsigned int uiIncrement, unsigned int uiMaxnumres);

/** ----------------------------------------------------------------------------------
 * @brief If not already aligned, round up a size_t to the next-higher rsrcRES_ALIGN_TO bounday
 */
static size_t privuxRsrcAlignUp(size_t uxSize)
{
	// add padding to each item for alignment, if needed
	if (uxSize & (rsrcRES_ALIGN_TO - 1)) // assumes rsrcRES_ALIGN_TO is a power of 2
		uxSize += (rsrcRES_ALIGN_TO - (uxSize & (rsrcRES_ALIGN_TO - 1)));
	return uxSize;
}

/** ----------------------------------------------------------------------------------
 * @brief Allocate a resource pool and add it to the list of known ones.  If called with any uint arg >64K will abort().
 *
 * @param pcName [in] Name of Pool (non-null, lifetime >= that of pool)
 * @param xRsrcSize [in] Size of resource (will be rounded up if necessary for alignment)
 * @param uiInitalloc [in] Number of resources initially allocated to pool freelist, range 0..64K
 * @param uiIncrement [in] Number of resources added when pool is empty and one is requested, range 0..64K
 * @param uiMaxNumRes [in] Maximum number of resources pool can reach (<64K), 0 == unlimited
 *
 * @return [out]  Allocated pool.  NULL if one cannot be allocated. May abort() if called wrong.
 * @note Variable pools do not have a size or freelist, since we don't know how long each should be.  So the initial
 * allocation must be zero, and the increment must be one.  Since there is no freelist, variable-length resources
 * are returned immediately to the heap upon being freed.
 */
rsrcPoolP_t pxRsrcNewPool(const char *pcName, size_t xRsrcSize,
						  unsigned int uiInitalloc, unsigned int uiIncrement, unsigned int uiMaxNumRes)
{
	rsrcPoolP_t pxPool;

	if (!pcName || (uiInitalloc | uiIncrement | uiMaxNumRes) > UINT16_MAX || uiIncrement + uiInitalloc == 0)
	{
		abort();
	}
	if (xRsrcSize == 0 && (uiInitalloc > 1 || uiIncrement > 1)) // variable pool specified wrong?
		return (NULL);

	if (rsrcPools.pxNext == NULL)
	{ // First pool to be added?  Initialize.
		int success;

		listINIT_HEAD(&rsrcPools);
		DEBUGPRINTF(TAG, "Initializing rsrcPools\n");
		// initialize the Pool of Pools
		success = prviRsrcInitPool(&xRsrcPoolPool, "Pools", sizeof(struct rsrcPool),
								   rsrcINIT_NUM_POOLS, rsrcONE_RESOURCE, rsrcNO_MAX_LIMIT);
		if (success <= 0)
		{
			return (NULL);
		}
		vRsrcSetPrintHelper(&xRsrcPoolPool, vRsrcDefaultPrintPoolHelper);
	}
	pxPool = pxRsrcAlloc(&xRsrcPoolPool, pcName);
	if (!pxPool || prviRsrcInitPool(pxPool, pcName, xRsrcSize, uiInitalloc, uiIncrement, uiMaxNumRes) < 0)
		return (NULL);
	return (pxPool);
}

/** ----------------------------------------------------------------------------------
 * @brief Allocate a Variable resource pool and add it to the list of known ones.  If called with maxnumres >64K will abort().
 *
 * @param pcName [in] Name of Pool (non-null, lifetime >= that of pool)
 * @param uiMaxNumRes [in] Maximum number of resources pool can reach (<64K), 0 == unlimited
 *
 * @return [out]  Allocated pool.  NULL if one cannot be allocated. May abort() if called wrong.
 * @note Variable pools do not have a fixed size or a freelist, so variable-length resources
 * must be returned immediately to the free pool upon being freed.
 */
rsrcPoolP_t pxRsrcNewVarPool(const char *pcName, unsigned int uiMaxNumRes)
{
	return (pxRsrcNewPool(pcName, (size_t)0, rsrcNONE, rsrcONE_RESOURCE, uiMaxNumRes));
}

/** ----------------------------------------------------------------------------------
 * @brief Allocate a Dynamic resource pool and add it to the list of known ones.  If called with maxnumres >64K will abort().
 *
 * @param pcName [in] Name of Pool (non-null, lifetime >= that of pool)
 * @param xSizeEach [in] Size of resources that belong to this pool (>0)
 * @param uiMaxNumRes [in] Maximum number of resources pool can reach (<64K), 0 == unlimited
 *
 * @return [out]  Allocated pool.  NULL if one cannot be allocated. May abort() if called wrong.
 * @note Dynamic pool resources are returned immediately to the free pool upon being freed.
 */
rsrcPoolP_t pxRsrcNewDynPool(const char *pcName, size_t xSizeEach, unsigned int uiMaxNumRes)
{
	return (pxRsrcNewPool(pcName, xSizeEach, rsrcNONE, rsrcONE_RESOURCE, uiMaxNumRes));
}
/** ----------------------------------------------------------------------------------
 * @brief Initialize an existing pool and add it to the list of known ones.
 *
 * @param pxPool [in] Pointer to an existing pool struct
 * @param pcName [in] Name of Pool (non-null, lifetime >= that of pool)
 * @param xRsrcSize [in] Size of resource (will be rounded up if necessary for alignment)
 * @param uiInitAlloc [in] Number of resources initially allocated to pool freelist, range 0..65K
 * @param uiIncrement [in] Number of resources added when pool is empty and one is requested, range 0..65K
 * @param uiMaxNumRes [in] Maximum number of resources pool can reach (<65K), 0 == unlimited
 *
 * @return [out]  Number of resources allocated initially (may be zero). Does not fail.
 */
static int prviRsrcInitPool(rsrcPoolP_t pxPool, const char *pcName, size_t xRsrcSize,
							unsigned int uiInitAlloc, unsigned int uiIncrement, unsigned int uiMaxNumRes)
{
	listADD(&rsrcPools, &pxPool->xPools); // link into the global list of pools
	listINIT_HEAD(&pxPool->xActive);
	pxPool->uxSizeEach = xRsrcSize; // resource size, struct rsrc overhead accounted for elsewhere
	pxPool->pxFreelist = NULL;
	pxPool->pxAllocHelper = NULL;
	pxPool->pxFreeHelper = NULL;
	pxPool->pxPrintHelper = NULL;
	pxPool->pcName = pcName;
	pxPool->ulTotalAllocs = pxPool->uiNumFree = pxPool->uiNumInUse = 0;
	pxPool->uiIncrement = uiIncrement;
	pxPool->uiFreeOnFree = (xRsrcSize == 0 || (uiIncrement <= 1 && uiInitAlloc <= 1));
	pxPool->uiMaxNumRes = uiMaxNumRes;
	pxPool->uiLowWater = uiInitAlloc;
	pxPool->uiHiWater = 0;
	DEBUGPRINTF(TAG, "New pool '%s' added, sizeeach %lu, #init %d, #inc %d, at 0x%p\n",
				pool->pcName, pool->uxSizeEach, initalloc, increment, pool);
	if (uiInitAlloc)
	{
		return (prviRsrcAdd2Pool(pxPool, uiInitAlloc, privuxRsrcAlignUp(sizeof(struct rsrc) + xRsrcSize)));
	}
	return (0);
}

/** ----------------------------------------------------------------------------------
 * @brief Allocate a variable-length resource from a pool
 *
 * @param pxPool [in] Pool to allocate from
 * @param pcRequestor [in] Name or other ID of reqeustor, for logging/debug, lifetime >= resource lifetime, non-NULL
 * @param xPayloadLen [in] Length of payload of resource desired
 *
 * @return Pointer to resource, or NULL if none allocated for any reason
 * @note  The standard usage pattern is to define an inline function that calls pxRsrcVarAlloc for a specific pool
 * and returns the result from psRsrcVarAlloc, cast appropriately for the pool.  pxRsrcVarAlloc is normally
 * for use with dynamic (variable) pools, but if used with a fixed-size pool, the requested size must match the
 * pool's resource size.  (Note that in that case, the request is identical to calling pxRsrcAlloc, which internally
 * calls this function.)
 * Any Helper functions that operate on variable length resources MUST understand what they're doing.  This
 * function does know the length to clear when a resource is allocated, but later vRsrcFree does not know the
 * length and cannot clear the residue.
 * No casting is required when using the returned resource with vRsrcFree or vRsrcRenameRsrc.
 */
void *pxRsrcVarAlloc(rsrcPoolP_t pxPool, const char *pcRequestor, size_t xPayloadLen)
{
	struct rsrc *pxRsrc;

	if (!pxPool || !pcRequestor || xPayloadLen == 0 || (pxPool->uiMaxNumRes && pxPool->uiNumInUse + pxPool->uiNumFree >= pxPool->uiMaxNumRes) || (pxPool->uxSizeEach && xPayloadLen != pxPool->uxSizeEach))
	{
		return (NULL);
	}
	if (pxPool->pxFreelist == 0)
	{
		size_t rsrcsize = privuxRsrcAlignUp(sizeof(struct rsrc) + xPayloadLen);

		if (prviRsrcAdd2Pool(pxPool, pxPool->uiIncrement, rsrcsize) <= 0)
		{
			return (NULL);
		}
	}
	pxRsrc = (struct rsrc *)pxPool->pxFreelist; // remove from freelist
	pxPool->pxFreelist = pxRsrc->pxRight;
	if (--pxPool->uiNumFree < pxPool->uiLowWater)
		pxPool->uiLowWater = pxPool->uiNumFree;

	pxPool->ulTotalAllocs++; // add to in-use list
	listADD(&pxPool->xActive, &pxRsrc->xLinks);
	if (++pxPool->uiNumInUse > pxPool->uiHiWater) // track highest number ever in use
		pxPool->uiHiWater = pxPool->uiNumInUse;

	pxRsrc->pcResName = pcRequestor;
	pxRsrc->pxPool = pxPool;

	if (eRsrcClearResOnAlloc != rsrcNOCLEAR)
	{
		memset(&pxRsrc->ucPayload, 0, xPayloadLen);
	}
	if (pxPool->pxAllocHelper)
	{
		pxPool->pxAllocHelper(pxPool, &pxRsrc->ucPayload);
	}
	return (&pxRsrc->ucPayload);
}
/** ----------------------------------------------------------------------------------
 * @brief Allocate a resource from a pool
 *
 * @param pxPool [in] Pool to allocate from
 * @param pcRequestor [in] Name or other ID of reqeustor, for logging/debug, lifetime >= resource lifetime, non-NULL
 *
 * @return Pointer to resource, or NULL if none allocated for any reason
 * @note  The standard usage pattern is to define an inline function that calls pxRsrcAlloc for a specific pool
 * and returns the result from psRsrcAlloc, cast appropriately for the pool.
 * No casting is required when using the returned resource with vRsrcFree or vRsrcRenameRsrc.
 */
void *pxRsrcAlloc(rsrcPoolP_t pxPool, const char *pcRequestor)
{
	if (pxPool)
		return (pxRsrcVarAlloc(pxPool, pcRequestor, pxPool->uxSizeEach));
	else
		return (NULL);
}

/** ----------------------------------------------------------------------------------
 * @brief Common routine for adding a previously-used resource or a new one to freelist.
 * @param pxPool [in] The pool that the resource belongs to
 * @param pxRsrc [in] Pointer to the struct rsrc that is being put on the freelist
 * @param iCallFreeHelper [in] If non-zero, the pool's Free Helper routine is called; if 0, it is not
 * @note The Free Helper routine is not called when a resource is first created, before ever being used, as the helper
 * routines are invoked only on resources that might need cleaning of residue.  (If starting with a cleared resource for
 * first usage is important, use an Alloc helper that clears or sets it).  Any needed residue cleaning is thus the responsibility
 * of the owner of the resource.
 */
static void prvvFreeResTail(rsrcPoolP_t pxPool, struct rsrc *pxRsrc, int iCallFreeHelper)
{
	pxRsrc->pvFlag = RESFREE_MAGIC; // to detect double-free
	pxRsrc->pxRight = pxPool->pxFreelist;
	pxPool->pxFreelist = pxRsrc;
	pxPool->uiNumFree++;
	if (pxPool->pxFreeHelper && iCallFreeHelper)
		pxPool->pxFreeHelper(pxPool, &pxRsrc->ucPayload);
	if (eRsrcClearResOnAlloc == rsrcCLEAR_ON_BOTH && pxPool->uxSizeEach)
		memset(&pxRsrc->ucPayload, 0xff, pxPool->uxSizeEach);
}

//
/** ----------------------------------------------------------------------------------
 * @brief Return a resource to the resource pool it's associated with.  Will invoke abort() on double-free attempt
 * @param pvResPayload [in] The resource to be freed.  If NULL, nothing is done
 *
 * @note If the pool has no freelist, as determined by the resource length being zero ("unknown") for the pool, OR
 * neither the initial allocation nor the increment are >1 (we can't free groups of >1 resource as we can't tell when
 * both are free without an expensive operation), then the resource is simply freed with the platform's heap free().
 */
void vRsrcFree(void *pvResPayload)
{
	if (!pvResPayload)
		return;

	struct rsrc *pxRsrc = RESADDR(pvResPayload);
	struct rsrcPool *pxPool = pxRsrc->pxPool;

	if (pxRsrc->pvFlag == RESFREE_MAGIC)
	{
		logPrintf(TAG, "freeRsrc: double-free attempt on object at %p, trying to print:",
				  pvResPayload);
		logPrintf(TAG, "rsrc %p, rsrc->name '%s', rsrc->pool->name %s", pxRsrc,
				  (pxRsrc->pcResName ? pxRsrc->pcResName : "NULL"), (pxPool->pcName ? pxPool->pcName : "NULL"));
		// invoke the debugger here
		abort();
	}
	listREMOVE(&pxRsrc->xLinks);
	pxPool->uiNumInUse--;
	if (pxPool->uiFreeOnFree)
	{
		// there is no freelist for unknown-size resources, just return the resource via Free
		DEBUGPRINTF(TAG, "Freeing resource @rsrc=%p\n", rsrc);
		if (pxPool->pxFreeHelper)
			pxPool->pxFreeHelper(pxPool, &pxRsrc->ucPayload);
		rsrcRES_FREE(pxRsrc);
	}
	else
	{
		prvvFreeResTail(pxPool, pxRsrc, 1);
	}
}

/** ----------------------------------------------------------------------------------
 * @brief Internal routine to add more entries to a pool.  May abort() or invoke user's out-of-memory function
 * @param pxPool [in] The pool that resources are to be added to
 * @param uiCount [in] The number of resources to be added to the pool
 * @param xResSize [in]
 * @return Number of resources added.  -1 if there was a problemm and none were added.
 * @note If an attempt is made to malloc more space and it fails, the private out-of-memory
 * function prviRsrcOOM is called to deal with it.  If that function returns, this function fails and its caller
 * will eventually return a NULL resource.
 */
static int prviRsrcAdd2Pool(rsrcPoolP_t pxPool, unsigned int uiCount, size_t xResSize)
{
	void *space;

	DEBUGPRINTF(TAG, "Pool %s adding %ld bytes\n",
				pool->pcName, count * rsrcsize);
	if (uiCount == 0)
		return (0);
#ifdef rsrcTEST_FORCE_OOM // force out-of-memory condition after rsrcTEST_FORCE_OOM times
	static int iMaxAllocs = 0;
	if (iMaxAllocs >= rsrcTEST_FORCE_OOM)
		return (-1);
	iMaxAllocs++;
#endif
	space = rsrcRES_ALLOC(uiCount, xResSize);
	if (!space)
	{
		prviRsrcOOM(pxPool, uiCount);
		return (-1);
	}
	if (pxPool->uiNumInUse + pxPool->uiNumFree == 0)
		pxPool->uiLowWater = uiCount; // only happens once, at first allocation

	for (unsigned int i = 0; i < uiCount; i++, space += xResSize)
	{
		struct rsrc *rsrc = (struct rsrc *)space;

		prvvFreeResTail(pxPool, rsrc, 0); // FIXME! Is 0 right on new ones?
	}
	return (uiCount);
}

/** ----------------------------------------------------------------------------------
 * @brief Change the name associated with a resource
 * @param pvRsrcPayload [in] The resource whose name is to be changed
 * @param pcNewName [in] the new name that will be stored with the resource. Lifetime >= that of the resource.
 * @note It is unwise to give a resource a NULL name, as it makes it necessary to be cautious whenever it is
 * printed or otherwise manipulated.  If a NULL pointer is passed, the string "(NULL)" will be assigned to the rsrc.
 */
void vRsrcRenameRsrc(void *pvRsrcPayload, const char *pcNewName)
{
	struct rsrc *rsrc = RESADDR(pvRsrcPayload);

	if (pcNewName)
		rsrc->pcResName = pcNewName;
	else
		rsrc->pcResName = "(NULL)";
}

/** ----------------------------------------------------------------------------------
 * @brief Print basic information about a single resource, and if a print helper is provided, invoke it as well
 * @param pcExplanation [in] A message printed along with the resource information, typically to identify the calling point
 * @param pvRsrcpayload [in] The resource to be printed.
 * @note This routine prints the address of the struct rsrc, and the pool and resource name strings for the resource.
 * If a print helper is provided for the pool, it then invokes it to do customized printing of the resource.
 */
void vRsrcPrintResource(const char *pcExplanation, void *pvRsrcpayload)
{
	if (!pvRsrcpayload)
		return;
	struct rsrc *pxRsrc = RESADDR(pvRsrcpayload);
	struct rsrcPool *pxPool = pxRsrc->pxPool;
	char punctuation;

	if (pxPool->pxPrintHelper)
		punctuation = ':';
	else
		punctuation = '.';
	logPrintf(TAG, "%s in pool %s: '%s'(%p) @%p%c", pcExplanation, pxPool->pcName,
			  pxRsrc->pcResName, pxRsrc, pvRsrcpayload, punctuation);
	if (pxPool->pxPrintHelper)
		pxPool->pxPrintHelper(pxPool, pvRsrcpayload);
}

/** ----------------------------------------------------------------------------------
 * @brief Print the summary stats for a srsrPool structure
 * @param pxOwningPool [in] The pool that the resource came from
 * @param pxPool2Print [in] The Pool whose stats are to be printed.
 * @note This is the default print helper froutine for rsrcPool structs
 */
void vRsrcDefaultPrintPoolHelper(rsrcPoolP_t pxOwningPool, void *pxPool2Print)
{
	rsrcPoolP_t pxPool = (rsrcPoolP_t)pxPool2Print;

	logPrintf(TAG, "=== Pool %s: %ju(Tot), %u(A), %u(AHi), %u(F), %u(FLo)",
			  pxPool->pcName, pxPool->ulTotalAllocs, pxPool->uiNumInUse,
			  pxPool->uiHiWater, pxPool->uiNumFree, pxPool->uiLowWater);
}

/** ----------------------------------------------------------------------------------
 * @brief Private short and long statistics printing (short for heartbeat, long to get everything known
 * @param pxPool2Print [in] The pool to print, or NULL for all of them. If non-NULL but not an active pool, nothing is printed
 * @param iPrintRsrcs [in] If non-zero, details of all resources in each printed pool will also be printed
 * @note If the pool has no printhelper, the name and address (of both struct rsrc and the body) are printed.
 * If a print helper routine is provided it will be called instead.  Note that default formatting is somewhat different from
 * printing with vRsrcPrintResource to reduce the amount of overhead text in the output for this case.
 */
void prvvRsrcPrintCom(rsrcPoolP_t pxPool2Print, int iPrintRsrcs)
{
	Link_t *pxPoolWalker; // walks through all the pools.

	listFOR_EACH(pxPoolWalker, (&rsrcPools))
	{
		rsrcPoolP_t pxPool = (rsrcPoolP_t)pxPoolWalker;
		if (pxPool2Print && pxPool2Print != pxPool) // ignore uninteresting pools
			continue;
		DEBUGPRINTF(TAG, "%s: %ju(Tot), %u(A), %u(AHi), %u(F), %u(FLo)\n",
					pxPool->pcName, pxPool->ulTotalAllocs,
					pxPool->uiNumInUse, pxPool->uiHiWater, pxPool->uiNumFree, pxPool->uiLowWater);
		if (xRsrcPoolPool.pxPrintHelper)
		{
			xRsrcPoolPool.pxPrintHelper(&xRsrcPoolPool, pxPool);
		}
		const char *pcComma = "";
		if (iPrintRsrcs && pxPool->uiNumInUse > 0 && pxPool != &xRsrcPoolPool)
		{
			Link_t *pxRsrcWalker;

			listFOR_EACH(pxRsrcWalker, &pxPool->xActive)
			{
				struct rsrc *r = (struct rsrc *)pxRsrcWalker;

				if (pxPool->pxPrintHelper)
					pxPool->pxPrintHelper(pxPool, &r->ucPayload);
				else
					logPrintf(TAG, "%sres '%s' at %p (%p)",
							  pcComma, r->pcResName, &r->ucPayload, r);
				pcComma = ", ";
			}
		}
	}
}

/** ----------------------------------------------------------------------------------
 * @brief Print only the basic stats of every (or a) pool: name, free, minfree, inuse, maxinuse, totalallocs
 * @param pxPool [in] Either a specific pool, which will print only that pool's stats, or NULL to print all of them
 *
 * @note If a non-null vpPool value is passed in, it must point to a pool on the active list or nothing will happen
 */
void vRsrcPrintShort(rsrcPoolP_t pxPool)
{
	prvvRsrcPrintCom(pxPool, 0);
}

/** ----------------------------------------------------------------------------------
 * @brief Print the basic stats of every (or a) pool, then for each print something about every resource in that pool
 * @param pxPool [in] Either a specific pool, or NULL to print all of them
 * @note If a non-null vpPool value is passed in, it must point to a pool on the active list or nothing will happen
 */
void vRsrcPrintLong(rsrcPoolP_t pxPool)
{
	prvvRsrcPrintCom(pxPool, 1);
}

/** ----------------------------------------------------------------------------------
 * @brief Called internally when a malloc for additional resources fails.  By default, prints a message and aborts
 * @param pxPool [in] The pool that resource(s) is being requested from
 * @param uiRequest [in] The number of resources being requested
 * @note If a user-supplied vRsrcOOMfn function is provided it is called.  If it returns, this function does not abort() but
 * instead returns a code to the allocator that causes it to return NULL for the allocation attempt instead.  Nothing is
 * printed here in that case.
 */
static int prviRsrcOOM(rsrcPoolP_t pxPool, unsigned int uiRequest)
{
	if (vRsrcOOMfn)
	{
		(*vRsrcOOMfn)(pxPool, uiRequest);
		return (1);
	}
	logPrintf(TAG, "Out of Memory!, pool->name %s, requested %u\n",
			  ((struct rsrcPool *)pxPool)->pcName, uiRequest);
	abort();
}

/** ----------------------------------------------------------------------------------
 * @brief Helper routine, can be set as alloc or free helper
 * @param pxPool [in] Pool the resource is from
 * @param pvRsrcPayload [in] Resource body (as returned from psRsrcAlloc)
 * @note Sets the resource size in bytes to 0xff (MAY LEAVE >0 PAD BYTES UNSET).
 * Does not do anything to variable resources - it doesn't know the length.
 */
void vRsrcSetResToOnesHelper(rsrcPoolP_t pxPool, void *pvRsrcPayload)
{
	memset(pvRsrcPayload, 0xff, ((struct rsrcPool *)pxPool)->uxSizeEach);
}

/**
 * @brief Helper routine, can be set as alloc or free helper
 * @param pxPool [in] Pool the resource is from
 * @param pvRsrcPayload [in] Resource body (as returned from psRsrcAlloc)
 * @note Clears the resource size in bytes to 0 (MAY LEAVE >0 PAD BYTES UNCLEARED)
 * Does not do anything to variable resources - it doesn't know the length.
 */
void vRsrcSetResToZerosHelper(rsrcPoolP_t pxPool, void *pvRsrcPayload)
{
	memset(pvRsrcPayload, 0, ((struct rsrcPool *)pxPool)->uxSizeEach);
}

/** ----------------------------------------------------------------------------------
 * @brief Called to print a resource (prefix struct rsrc as well as resource content) in hex during debug
 * @param pxPool [in] Pool the resource is from
 * @param pvRsrcPayload [in] Resource body (as returned from psRsrcAlloc)
 * @note Can be set as a print helper for a pool if desired.  Prints only the first part of the resource
 * to avoid flooding a log.  The amount is set by the internal const variable prvIntsToPrint.  Note that on
 * little-endian machines printing as ints can lead to extra bytes being printed in what seems like the
 * wrong place, and on 64-bit machines the halves of the 64-bit value will (usually) be reversed.
 */
void vRsrcPrintResInHexHelper(rsrcPoolP_t pxPool, void *pvRsrcPayload)
{
	struct rsrc *pxRsrc = RESADDR(pvRsrcPayload);
	uint32_t *pi;
	const unsigned int prvIntsToPrint = 32; // adjust to your liking

	logPrintf(TAG, "-----%16p: Prev %p Next %p pool '%s', name '%s'\n",
			  pxRsrc, pxRsrc->xLinks.pxPrev, pxRsrc->xLinks.pxNext, pxPool->pcName, pxRsrc->pcResName);

	long lNumIntsToPrint = (pxPool->uxSizeEach + (sizeof(uint32_t) - 1)) / (sizeof(uint32_t));
	// for benefit of variable allocations, print a line or so
	if (lNumIntsToPrint == 0)
	{
		logPrintf(TAG, "    PrintInHex: variable resource, using %d\n", prvIntsToPrint);
		lNumIntsToPrint = prvIntsToPrint;
	}
	if (lNumIntsToPrint > prvIntsToPrint)
	{
		logPrintf(TAG, "     PrintInHex: resource size > %u ints (%lu), using %u\n", prvIntsToPrint, lNumIntsToPrint, prvIntsToPrint);
		lNumIntsToPrint = prvIntsToPrint;
	}

	const int iIntsPerLine = 4;
	for (pi = (uint32_t *)pvRsrcPayload; lNumIntsToPrint > 0; lNumIntsToPrint -= 4, pi += 4)
	{
		char c;
		int numtoprint = lNumIntsToPrint > iIntsPerLine ? iIntsPerLine : (int)lNumIntsToPrint;

		logPrintf(TAG, "%16p: ", pi);
		for (int i = 0; i < numtoprint; i++)
		{
			logPrintf(TAG, "%08x ", pi[i]);
		}
		for (unsigned int i = 0; i < (numtoprint * (sizeof(int))); i++)
		{
			c = (*pi >> (24 - i * 8)) & 0xff;
			if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
				(c >= '0' && c <= '9'))
			{
				logPrintf(TAG, "%c", c);
			}
			else
			{
				logPrintf(TAG, ".");
			}
		}
		logPrintf(TAG, "\n");
	}
}

/** ----------------------------------------------------------------------------------
 * @brief Sets the Alloc Helper Helper for a pool
 * @param pxPool [iin] The pool to set the helper for
 * @param pxHelper [in] the function to set as the helper
 * @note If set, called after any forced clearing takes place.
 */
void vRsrcSetAllocHelper(rsrcPoolP_t pxPool, rsrcAllocHelper_t *pxHelper)
{
	((struct rsrcPool *)(pxPool))->pxAllocHelper = pxHelper;
}

/** ----------------------------------------------------------------------------------
 * @brief Sets the Free Helper routine for a pool
 * @param pxPool [iin] The pool to set the helper for
 * @param pxHelper [in] the function to set as the helper
 * @note If set, called BEFORE any forced clearing takes place.  Can be used to clean up a resource, freeing
 * dynamically-allocates storage pointed to by it, etc.  BE CAREFUL, since you may get called when the resource
 * is in any state at all.  Pointers may be unset, etc.
 */
void vRsrcSetFreeHelper(rsrcPoolP_t pxPool, rsrcFreeHelper_t *pxHelper)
{
	((struct rsrcPool *)(pxPool))->pxFreeHelper = pxHelper;
}

/** ----------------------------------------------------------------------------------
 * @brief Sets the Print Helper routine for a pool
 * @param pvPool [iin] The pool to set the helper for
 * @param pxHelper [in] the function to set as the helper
 * @note Print Helper need not print anything unless it wants to.  It could, for example, only print something if
 * it looks like there's something odd about the resource, etc.  BE CAREFUL, since you may get called when the resource
 * is in any state at all.  Pointers may be unset, etc.
 */
void vRsrcSetPrintHelper(rsrcPoolP_t pvPool, rsrcPrintHelper_t *pxHelper)
{
	((struct rsrcPool *)(pvPool))->pxPrintHelper = pxHelper;
}
