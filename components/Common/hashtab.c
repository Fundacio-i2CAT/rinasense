/*
 *  hashtab.c
 *
 *  Copyright 2010,2022 TRIA Network Systems. See LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>

#ifndef _LISTUTILS_H_
#define dlList_t 	Link_t		// compatible libraries, different names...
#define LLINKSINIT	listINIT_HEAD
#define lInsert		listADD
#define lDelete		listREMOVE
#define right		pxNext
#define left		pxPrev
#endif

// For use in FreeRTOS, translate POSIX APIs
//#define POSIX 1
#ifdef POSIX // ----- POSIX ------
#include <stdio.h>
#include "hashtab.h"
#include "rsrc.h"	// hash tables are allocated from a pool

#define DEBUGPRINTF(tag,format,x...)	printf("%s " format "\n",TAG,x)
#define logPrintf(tag,format,x...)		printf("%s " format "\n",TAG,x)

#else // ---- FreeRTOS -----
#include "portability/port.h"
#include "Common/hashtab.h"
#include "Common/rsrc.h"

#define malloc(x)	pvRsMemAlloc(x)
#define free(res)	vRsMemFree(res)
#define DEBUGPRINTF			LOGI
#define logPrintf			LOGI

#endif // POSIX

#define htOVERWRITE 1		// overwrite existing entry with same key?
#define htNOOVERWRITE 0
#define htPRINTSTATS	1	// if set, detailed statistics printing enabled

static const char* TAG = "[hashtab]"; // labels log message origin

// allocate and add more entries to freelist, if allowed and if malloc succeeds
static int prvMorefree(hashtab_t *tab, unsigned num2add)
{
	int i;
	hashent_t *e;
	unsigned size = sizeof(hashent_t);
	
	size += sizeof (size_t) - (size & (sizeof (size_t) - 1));
	
	// check if we're allowed to add more, and if so, can acquire space
	if ((tab->ulMaxEntries && tab->ulCurEntries + num2add > tab->ulMaxEntries)
		 || !(e = (hashent_t *) malloc(size * num2add))) {
		return (0);
	}
	for (i = 0; i < num2add; i++) {
		e->pxFreelist = tab->pxFreelist; // put entry on free list
		tab->pxFreelist = e;
		void *alignit = e; alignit += size; e = alignit; // nuts
	}
	return (tab->ulAllocSize);
}

// Get a free entry from the freelist of a table to add to the table
static hashent_t *prvNewhashent(hashtab_t *table)
{
	hashent_t *e = table->pxFreelist;
	
	if (e) {
		table->pxFreelist = (hashent_t *)e->pxFreelist;
	} else {
		if (prvMorefree(table, table->ulAllocSize))
			return (prvNewhashent(table));
	}
	table->ulCurEntries++;
	LLINKSINIT((dlList_t *)e);
	return(e);
}
static void prvFreehashent (hashtab_t *table, hashent_t *entry)
{
	entry->pxFreelist = table->pxFreelist;
	table->pxFreelist = entry;
	table->ulCurEntries--;
}

// Hashing functions.  Feel free to improve this, it's ad-hoc
static inline unsigned prvHashedName(const char *name)
{
	unsigned hash = 0;
	
	for (; *name; name++)
		hash = (hash >> 16) + ((hash << 5) ^ (*name));
	return (hash);
}
static inline unsigned prvHashedInt (unsigned key)
{
	return (277 * key + key + 12345);
}

// Hash table lookup common routine.  Used to find the correct listhead, and if
// the entry is present, the correct hash entry.  Returns non-zero if the entry
// was found.  The listhead arg is where we return the list it should have been in.
// if name is NULL, the key is used to determine a match. If not, strcmp is used
static int prvHashLookupCom (hashtab_t *table, unsigned key, const char *name,
				   dlList_t **listheadp, hashent_t **entry)
{
	dlList_t *listhead;		// correct list for this name
	hashent_t *e;			// the roamer through the list off the head
	int bucketno = (name ? prvHashedName (name) : prvHashedInt(key) ) % table->ulBucketCount;
	
	*listheadp = listhead = &table->pxBuckets[bucketno];
	
	e = (hashent_t *)listhead->right;
	for (; (dlList_t *)e != listhead; e = (hashent_t *)e->xLinks.right) {
		if (!name && key != e->ulKey)
			continue;
		if (name && strcmp(name, e->pcName) != 0)
			continue;
		*entry = e;
		return (1);
	}
	*entry = NULL;
	return (0);
}
hashent_t * pxHtIFindEntry (hashtab_t *table, unsigned key)
{
	dlList_t *listhead;
	hashent_t *e;
	
	if (prvHashLookupCom(table, key, NULL, &listhead, &e))
		return (e);
	return (NULL);
}
hashent_t * pxHtSFindEntry (hashtab_t *table, const char *name)
{
	dlList_t *listhead;
	hashent_t *e;
	
	if (prvHashLookupCom(table, 0, name, &listhead, &e))
		return (e);
	return (NULL);
}

// Hash lookup general lookup routines.  Pass a name, get back a value, or not.
void *pvHtSGetVal (hashtab_t *table, const char *name)
{
	dlList_t *listhead;		// dumping ground - don't need this
	hashent_t *e;
	
	(void) prvHashLookupCom(table, 0, name, &listhead, &e);
	return (e ? e->pxValue : NULL);
}
void *pvHtIGetVal (hashtab_t *table, unsigned key)
{
	dlList_t *listhead;		// dumping ground - don't need this
	hashent_t *e;
	
	(void) prvHashLookupCom(table, key, NULL, &listhead, &e);
	return (e ? e->pxValue : NULL);
}

// Finds an entry, selectively rewrites its value if found, adds it if not
static int prvHtISAddVal (hashtab_t *table, int overwrite, unsigned key, const char *name, void *value)
{
	dlList_t *listhead;
	hashent_t *e;
	
	if (prvHashLookupCom(table, key, name, &listhead, &e)) {
		if (overwrite == htOVERWRITE) {
			e->pxValue = value;
			return (1);
		}
		else
			return (0);					//   already there, we don't touch it
	}
	e = prvNewhashent(table);				// new entry, create and fill it
	if (name)
		e->pcName = name;
	else
		e->ulKey = key;
	e->pxValue = value;
//	DEBUGPRINTF(TAG,"entry %p, head %p (%p, %p): ", e, listhead, listhead->pxNext, listhead->pxPrev);
	lInsert(listhead, (dlList_t *) e);
//	DEBUGPRINTF(TAG,"now: entry (%p, %p), head (%p, %p)", ((dlList_t *)e)->pxNext, ((dlList_t *)e)->pxPrev,listhead->pxNext, listhead->pxPrev);
	return (1);
}
int iHtIAddVal (hashtab_t *table, unsigned key, void *value)
{
	return prvHtISAddVal (table, htNOOVERWRITE, key, NULL, value);
}
int iHtSAddVal (hashtab_t *table, const char *name, void *value)
{
	return prvHtISAddVal(table, htNOOVERWRITE, 0, name, value);
}
int iHtISetVal (hashtab_t *table, unsigned key, void *value)
{
	return prvHtISAddVal(table, htOVERWRITE, key, NULL, value);
}
int iHtSSetVal (hashtab_t *table, const char *name, void *value)
{
	return prvHtISAddVal(table, htOVERWRITE, 0, name, value);
}

// delete an entry from the hash table, if found. returns # freed, 0 or 1
int iHtISDelete (hashtab_t *table, unsigned key, const char *name)
{
	dlList_t *listhead;
	hashent_t *e;

	if (prvHashLookupCom(table, key, name, &listhead, &e)) {
		lDelete ((dlList_t *) e);// unlink it
		prvFreehashent (table, e);			// put entry on free list
		return (1);
	}
	return (0);
}
int iHtSDelete (hashtab_t *table, const char *name)
{
	return (iHtISDelete (table, 0, name));
}
int iHtIDelete (hashtab_t *table, unsigned key)
{
	return (iHtISDelete (table, key, NULL));
}
void vHtEDelete (hashent_t *entry)
{
	lDelete ((dlList_t *)entry);
}

static void prvNextentry (htIterator_t *it) {
	// step through the buckets, and for each, step through the chain
	while (it->ulBucket < it->pxTable->ulBucketCount) {
		hashent_t *curbucket = (hashent_t *)&it->pxTable->pxBuckets[it->ulBucket];
		
		if((it->pxNext = (hashent_t *)((dlList_t *)it->pxNext)->right) != curbucket) {
			return;
		}
		if (++it->ulBucket < it->pxTable->ulBucketCount)
			it->pxNext = (hashent_t *)&it->pxTable->pxBuckets[it->ulBucket];
	}
	it->pxNext = NULL;
}
void vHtInitIterator (htIterator_t *it, hashtab_t *table)
{
	it->pxTable = table;
	it->ulBucket = 0;
	it->pxNext = (hashent_t *)&table->pxBuckets[0];
	// find the next/first entry, if there are any
	prvNextentry(it);
}
hashent_t *pxHtIteratorNext (htIterator_t *it)
{
	hashent_t *retval = it->pxNext;
	
	prvNextentry(it);
	return (retval);
}

// **************************************************
// We are making hash tables into a rsrc resource.  We can in the future,
// once we keep track of hash table mallocs, make it possible to free them
// up.  For now, we create a pool of hash tables -- once -- and then allocate
// new ones from that pool when they are created.  Hash entry blocks could also
// be allocated using rsrc, which might make sense, but we don't for now.
// ***************************************************

rsrcPoolP_t xHashTablePool;	// allocated once, when first needed

// The "long" print routine for hash table resources calls this
static void prvPrintStatWrapper (rsrcPoolP_t pool, void *body)
{
	vHtPrintStats ((hashtab_t *)body);
}
void static inline initHashtabPool ()
{
	if (xHashTablePool)
		return;
	xHashTablePool = pxRsrcNewDynPool("HashTables", sizeof (hashtab_t), 0);
	if (!xHashTablePool) {
		logPrintf(TAG,"Can't allocate HASHTables rsrc pool, aborting now%s", "");
		abort();
	}
	vRsrcSetPrintHelper(xHashTablePool, prvPrintStatWrapper);
}

// Allocate and initialize a hash table
hashtab_t *pxHtNewHashTable (const char *tablename, unsigned initentries, unsigned maxentries, unsigned entryincrement, unsigned numbuckets)
{
	hashtab_t *tab;
	dlList_t *listheads;
	int i;
	
	initHashtabPool();
	
	if (entryincrement > htMAX_ALLOCSIZE) {
		entryincrement = htMAX_ALLOCSIZE;
	}
	tab = pxRsrcAlloc(xHashTablePool, tablename);
	numbuckets |= 1;	// avoid degenerate case of even bucket count
	if (tab == NULL
		|| (listheads = (dlList_t *)malloc(sizeof (dlList_t) * numbuckets)) == NULL) {
		DEBUGPRINTF(TAG,"unable to allocate buckets/entries for hashtable%s", "");
		if (tab)
			vRsrcFree(tab);	// safe to delete, no storage will be lost
		return (NULL);
	}
	for (i = 0; i < numbuckets; i++) {
		LLINKSINIT(&listheads[i]);
	}
	tab->pcTablename = tablename;
	tab->ulMaxEntries = maxentries;
	tab->ulCurEntries = 0;
	tab->ulAllocSize = entryincrement;
	tab->ulBucketCount = numbuckets;
	tab->pxBuckets = listheads;
	tab->pxFreelist = NULL;
	prvMorefree(tab, initentries);
	return (tab);
}

#ifdef htPRINTSTATS
#define MAXCHAINLEN 32
static int prvListLength (dlList_t *list)
{
	int len = 0;
	
	// loop through list, add one for each member
	hashent_t *e;			// the roamer through the list off the head
	
	e = (hashent_t *)list->right;
	for (; (dlList_t *)e != list; e = (hashent_t *)e->xLinks.right) {
		len++;
	}
	return (len);
}
void vHtPrintStats(hashtab_t *table)
{
	int chainlengths[MAXCHAINLEN]; // number chains with each length
	int overmax = 0;		// length over the most we're istogramming
	float idealchainlen = (float) table->ulCurEntries / (float) table->ulBucketCount;
	int longest = 0;
	
	memset(chainlengths, 0, sizeof chainlengths);
	// loop through buckets, create histogram of chain lengths
	// The ideal is that chain actual lengths should cluster closely around
	// the ideal -- which is the number of entries divided by nmber of buckets
	for (int i = 0; i < table->ulBucketCount; i++) {
		int len = prvListLength(&table->pxBuckets[i]);

		if (len > longest)
			longest = len;
		if (len >= MAXCHAINLEN) {
			overmax++;
		} else {
			chainlengths[len]++;
		}
	}
	
	// summarize findings
	logPrintf(TAG,"\nTABLE \"%s\"", table->pcTablename);
	logPrintf(TAG,"BUCKETS: %d, MAX_ENTRIES %d, CUR_ENTRIES %d, INCREMENT %d", table->ulBucketCount, table->ulMaxEntries, table->ulCurEntries, table->ulAllocSize);
	logPrintf(TAG,"CHAIN  CHAIN%s", "");
	logPrintf(TAG,"LENGTH COUNT%s", "");
	for (int i = 0; i < MAXCHAINLEN; i++) {
		if (chainlengths[i]) {
			logPrintf(TAG,"%6d: %d", i, chainlengths[i]);
		}
	}
	logPrintf(TAG,"Ideal average chain length: %7.2f", idealchainlen);
	logPrintf(TAG,"Longest chain %d", longest);
	logPrintf(TAG,"CHAINS OVER %d: %d", MAXCHAINLEN, overmax);
	logPrintf(TAG,"EMPTY BUCKETS: %d", chainlengths[0]);
}
#else
void htPrintStats(hashtab_t *table)
{
}
#endif
