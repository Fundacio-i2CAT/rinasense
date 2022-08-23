/*
 * ARP.h
 *
 *  Created on: 5 ago. 2021
 *      Author: i2CAT
 */

#ifndef ARP_H_INCLUDED
#define ARP_H_INCLUDED

/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/

#include "common/mac.h"
#include "common/rina_gpha.h"
#include "portability/port.h"

#include "ARP826_defs.h"
#include "IPCP_frames.h"

// DECL_CAST_PTR_FUNC_FOR_TYPE( EthernetHeader_t );
// DECL_CAST_CONST_PTR_FUNC_FOR_TYPE( EthernetHeader_t );

extern DECL_CAST_PTR_FUNC_FOR_TYPE(ARPPacket_t);
extern DECL_CAST_CONST_PTR_FUNC_FOR_TYPE(ARPPacket_t);

extern DECL_CAST_PTR_FUNC_FOR_TYPE(MACAddress_t);
extern DECL_CAST_CONST_PTR_FUNC_FOR_TYPE(MACAddress_t);

/**
 * Structure for one row in the ARP cache table.
 */
typedef struct xARP_CACHE_TABLE_ROW
{
	gpa_t *pxProtocolAddress; /**< The IPCP address of an ARP cache entry. */
	gha_t *pxMACAddress;	  /**< The MAC address of an ARP cache entry. */
	uint8_t ucAge;			  /**< A value that is periodically decremented but can also be refreshed by active communication.  The ARP cache entry is removed if the value reaches zero. */
	uint8_t ucValid;		  /**< pdTRUE: xMACAddress is valid, pdFALSE: waiting for ARP reply */
} ARPCacheRow_t;

typedef enum
{
	eARPCacheMiss = 0, /* 0 An ARP table lookup did not find a valid entry. */
	eARPCacheHit,	   /* 1 An ARP table lookup found a valid entry. */
	eCantSendPacket	   /* 2 There is no IPCP address, or an ARP is still in progress, so the packet cannot be sent. */
} eARPLookupResult_t;

struct rinarpHandle_t
{
	gpa_t *pxPa;
	gha_t *pxHa;
};

/* Ethernet frame types. */
#define ARP_FRAME_TYPE (0x0608U)

/* ARP related definitions. */
#define ARP_PROTOCOL_TYPE                ( 0x0008U )
#define ARP_HARDWARE_TYPE_ETHERNET       ( 0x0001U )
#define ARP_REQUEST                      ( 0x0100U )
#define ARP_REPLY                        ( 0x0200U )

/************** ARP and Ethernet events handle *************************/
/*
 * Look for ulIPCPAddress in the ARP cache.  If the IPCP address exists, copy the
 * associated MAC address into pxMACAddress, refresh the ARP cache entry's
 * age, and return eARPCacheHit.  If the IPCP address does not exist in the ARP
 * cache return eARPCacheMiss.
 */
eARPLookupResult_t eARPGetCacheEntry(gpa_t *pulIPAddress,
									 gha_t *const pxMACAddress);

eFrameProcessingResult_t eARPProcessPacket(ARPPacket_t *const pxARPFrame);

/*************** RINA ***************************/
// UpdateMACAddress
void vARPUpdateMACAddress(const uint8_t ucMACAddress[MAC_ADDRESS_LENGTH_BYTES], const MACAddress_t *pxPhyDev);

// Request ARP for a mapping of a network address to a MAC address.
void RINA_vARPMapping(uint32_t ulIPCPAddress);

// Adds a mapping of application name to MAC address in the ARP cache.
bool_t vARPSendRequest(gpa_t * tpa, gpa_t * spa, gha_t * sha);

// Remove all ARP entry in the ARP cache.
void vARPRemoveAll(void);

eARPLookupResult_t eARPLookupGPA(const gpa_t *gpaToLookup);

void vARPRefreshCacheEntry(gpa_t *ulIPCPAddress, gha_t *pxMACAddress);
void vARPRemoveCacheEntry(const gpa_t *ulIPCPAddress, const gha_t *pxMACAddress);

struct rinarpHandle_t *pxARPAdd(gpa_t *pxPa, gha_t *pxHa);

void vARPInitCache(void);

bool_t xARPRemove(const gpa_t * pxPa, const gha_t * pxHa);

void vARPPrintCache(void);

void vPrintMACAddress(const gha_t *gha);

bool_t xARPResolveGPA(gpa_t * tpa, gpa_t * spa, gha_t * sha);

gha_t *pxARPLookupGHA(const gpa_t *pxGpaToLookup);


#endif /* ARP_H_ */
