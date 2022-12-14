/*
 * ARP.h
 *
 *  Created on: 5 ago. 2021
 *      Author: i2CAT
 */

#ifndef ARP_H_INCLUDED
#define ARP_H_INCLUDED

#include "portability/port.h"

#include "common/mac.h"
#include "common/rina_gpha.h"
#include "common/netbuf.h"

#include "ARP826_defs.h"
#include "ARP826_cache_defs.h"
#include "IPCP_frames.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TAG for logging */
#define TAG_ARP "[ARP]"

/* Ethernet frame types. */
#define ARP_FRAME_TYPE (0x0608U)

/* ARP related definitions. */
#define ARP_PROTOCOL_TYPE                ( 0x0008U )
#define ARP_HARDWARE_TYPE_ETHERNET       ( 0x0001U )

/* Those are already in network order! */
#define ARP_REQUEST                      ( 0x0100U )
#define ARP_REPLY                        ( 0x0200U )

/* Initialise the ARP component data */
bool_t xARPInit(ARP_t *pxArp);

/* Adds a mapping of application name to MAC address in the ARP
 * cache. */
bool_t vARPSendRequest(ARP_t *pxArp, const gpa_t *pxTpa, const gpa_t *pxSpa, const gha_t *pxSha);

/************** ARP and Ethernet events handle *************************/

/*
 * Look for ulIPCPAddress in the ARP cache.  If the IPCP address exists, copy the
 * associated MAC address into pxMACAddress, refresh the ARP cache entry's
 * age, and return eARPCacheHit.  If the IPCP address does not exist in the ARP
 * cache return eARPCacheMiss.
 */
eARPLookupResult_t eARPGetCacheEntry(gpa_t *pulIPAddress,
									 gha_t *const pxMACAddress);

eFrameProcessingResult_t eARPProcessPacket(ARP_t *pxARP, netbuf_t *pxNbEth);

/*************** RINA ***************************/
// UpdateMACAddress
void vARPUpdateMACAddress(const uint8_t ucMACAddress[MAC_ADDRESS_LENGTH_BYTES], const MACAddress_t *pxPhyDev);

bool_t xARPResolveGPA(ARP_t *pxArp, const gpa_t *pxTpa, const gpa_t *pSpa, const gha_t *pSha);

/* Add an application mapping to the ARP cache */
ARPCacheHandle xARPAddApplication(ARP_t *pxARP, const gpa_t *pxPa, const gha_t *pxHa);

/* Remove an application mapping from the ARP cache. */
bool_t xARPRemoveApplication(ARP_t *pxARP, const gpa_t *pxPa);

/* Returns the GPA matching a certain HA from the cache */
const gha_t *pxARPLookupGHA(ARP_t *pxARP, const gpa_t *pxPa);

#ifdef __cplusplus
}
#endif

#endif /* ARP_H_ */
