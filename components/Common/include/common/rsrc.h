/*
 *  rsrc.h
 *
 *  Created by Steve Bunch on 4/26/2012
 *  Copyright 2012, 2022 TRIA Network Systems, see LICENSE.
 *
 */

#ifndef _RSRC_H_
#define _RSRC_H_

#include <stdint.h>
#include "common/private/iot_doubly_linked_list.h"				// Amazon FreeRTOS library

/**
 * @brief Heap allocation routines for the target platform.  calloc/malloc/free semantics and calling sequence
 */
#ifndef rsrcRES_ALLOC
#define rsrcRES_ALLOC(num,size) pvRsMemAlloc((num)*(size)) // add space to a pool
//#define rsrcRES_ALLOC(num,size)	calloc(num,size) // add cleared space to a pool
#endif

#ifndef rsrcRES_FREE
#define rsrcRES_FREE(res)	vRsMemFree(res)			// platform heap free routine
#endif

//#define rsrcTEST_FORCE_OOM 100				// (TESTING) <= this many mallocs will succeed, then NULL

#define rsrcRES_ALIGN_TO	(sizeof (size_t))	// this address alignment forced for resources
//define rsrcRES_ALIGN_TO 	(sizeof (double))	// Some CPUs want or require this alignment
#define rsrcINIT_NUM_POOLS	5					// initial # of pool structs, increment is 1
// given a resource pointer, back up to the rsrc struct in front of it
#define RESADDR(p)			((struct rsrc *)((void *)p - offsetof(struct rsrc, ucPayload)))
#define rsrcONE_RESOURCE	1					// used in argument lists for clarification
#define rsrcNONE			0
#define rsrcNO_MAX_LIMIT	0					// used as upper bound on pool size when unlimited

struct rsrcPool;								// forward, for scoping in typedefs
/**
 *@brief Pointer to a struct rsrcPool, used as a handle to a pool
 */
typedef struct rsrcPool * rsrcPoolP_t;

typedef void rsrcAllocHelper_t (rsrcPoolP_t, void *);	// "alloc" helper signature
typedef void rsrcFreeHelper_t (rsrcPoolP_t, void *);	// "free" helper signature
typedef void rsrcPrintHelper_t (rsrcPoolP_t, void *);	// "print" helper" signature

/**
 * @brief The header for a resource pool, containing the free and active lists for the resource type, etc.
 * @note Static pools are a set of resources the same size, allocated but NEVER FREED from the heap.  They
 * should ideally be allocated at the beginning of a run.  Dynamic pools have no standing freelist.  Any time a new
 * resource is added, at resource allocation time a new resource is allocated from the heap, and when
 * a dynamic resource is freed, it goes back to the heap instead of freelist.  Otherwise variable and statically
 * allocated pools are virtually the same, they are differentiated by the allocation amount, which must be 1.
 * Variable-length dynamic resources behave the same way.
 * They are differentiated by the resource size, uxSizeEach, being zero.
 */
struct rsrcPool {
	Link_t		xPools;				// resource pools
	Link_t		xActive;			// active resources
	struct rsrc	*pxFreelist;		// first free resource.  LIFO
	rsrcAllocHelper_t	*pxAllocHelper;
	rsrcFreeHelper_t	*pxFreeHelper;
	rsrcPrintHelper_t	*pxPrintHelper;
	const char	*pcName;			// lifetime must be >= that of the rsrcPool
	uint64_t	ulTotalAllocs;
	size_t		uxSizeEach;			// sizeof of the resource type, 0 if variable
	uint16_t	uiIncrement;		// additional to allocate when free list empty
	uint16_t	uiFreeOnFree;		// nonzero for dynamic pools
	uint16_t	uiMaxNumRes;		// max inuse+free is allowed reach
	uint16_t	uiNumInUse;
	uint16_t	uiNumFree;
	uint16_t	uiLowWater;			// lowest value in freelist (after containing > 0)
	uint16_t	uiHiWater;			// largest number ever in active list
};

/**
 * @brief Per-resource info. The resource allocator extra information in front of the payload.
 * @note This version has 4 pointers, which will pass through any alignment provided
 * by the original memory provided by malloc or structure memory allocation, but
 * the allocation amount (payload) might not be a size that maintains that alignment.
 * Adjust RESALIGN_TO, used in the size computation in privuxRsrcAlignUp to round up.  If
 * you ever change this structure, you may also need to add padding before ucPayload to
 * ensure that it starts on the necessary boundary.
 */
struct rsrc {
	union {
		Link_t			xLinks;			// active list
		struct {
			struct rsrc *pxRight;		// free list
			void		*pvFlag;		// set to RESFREE_MAGIC on free resources
		};
	};
	rsrcPoolP_t		pxPool;			// containing pool
	const char		*pcResName;		// name of user of rsrc
	uint8_t			ucPayload[0];	// resource data starts here
};


extern void (* vRsrcOOMfn)(rsrcPoolP_t, int);	// OPTIONALLY set this to your own OOM handler

rsrcPoolP_t pxRsrcNewPool (const char *name, size_t sizeeach,
						   unsigned int initialAllocation, unsigned int incrementalAllocation,
						   unsigned int maxnumres);
rsrcPoolP_t pxRsrcNewVarPool (const char *name, unsigned int maxnumres);
rsrcPoolP_t pxRsrcNewDynPool (const char *name, size_t sizeach, unsigned int maxnumres);
void *pxRsrcAlloc (rsrcPoolP_t, const char *name);		// allocate from a fixed-size pool
void *pxRsrcVarAlloc (rsrcPoolP_t, const char *, size_t); // allocate variable-length resource
void vRsrcFree (void *rsrc);							// return resource to its pool
void vRsrcRenameRsrc (void *, const char *name);		// change the name on the resource

/*
 * short and long pool printing.  Short prints only pool names and counts, Long prints every resource.
 */
void vRsrcPrintShort (rsrcPoolP_t);
void vRsrcPrintLong (rsrcPoolP_t);
void vRsrcPrintResource (const char*, void *);

/*
 * Optional helper routines that can be associated with a pool,
 * Helpers can be set with the setters, and (re-)set to NULL to use default behavior.
 * NB: When cleaning up or printing, be careful of stale pointers.
 */
void vRsrcSetAllocHelper (rsrcPoolP_t, rsrcAllocHelper_t *);
void vRsrcSetFreeHelper (rsrcPoolP_t, rsrcFreeHelper_t *);
void vRsrcSetPrintHelper (rsrcPoolP_t, rsrcPrintHelper_t *);

void vRsrcSetResToOnesHelper (rsrcPoolP_t, void *);	// as helper, sets resource to all-0xff's
void vRsrcSetResToZerosHelper (rsrcPoolP_t, void *);	// as helper, sets resource to all-zeros
void vRsrcDefaultPrintPoolHelper (rsrcPoolP_t, void *); // default rsrcPool struct print helper

/*
 * vRsrcPrintResInHex prints at least a portion of the resource, including the invisible
 * header, in hex for debugging purposes.
 */
void vRsrcPrintResInHexHelper (rsrcPoolP_t, void *);

/**
 * @brief Force the allocator to overwrite residue in resources when allocating/freeing resources.
 * @note Debugging/testing feature, most useful during early testing or if CPU cycles are available.
 * Setting eRsrcClearResOnAlloc to a value other than rsrcNOCLEAR will
 * force every resource to be zeroed on allocation.  Setting it to rsrcCLEAR_ON_BOTH forces
 * clearing to zeros on allocation, and in addition, for fixed-length resources setting to all-1's
 * when freed.  This ensures that stale pointer values, security keys, etc., will not end up in allocated
 * resources, and a pointer de-ref on uninitalized resources will result in a fault.
 */
enum rsrcClearOnAllocSetting_t {rsrcNOCLEAR, rsrcCLEAR_ON_ALLOC, rsrcCLEAR_ON_BOTH};
extern enum rsrcClearOnAllocSetting_t eRsrcClearResOnAlloc;

#endif // _RSRC_H_
