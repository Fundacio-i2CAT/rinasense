#ifndef _ARP_DEFS_H_INCLUDED
#define _ARP_DEFS_H_INCLUDED

#include <stdint.h>

#include "common/mac.h"
#include "common/eth.h"
#include "common/rina_gpha.h"
#include "common/rsrc.h"
#include "configSensor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ARP component data. */
typedef struct xARP {
    struct ARPCache *pxCache;

    /* Keep track of all the registered applications to be able to
       unregister them. */
    //RsList_t xRegisteredAppHandles;

    rsrcPoolP_t xEthPool;
    rsrcPoolP_t xArpPool;

} ARP_t;

typedef enum {
    SHA = 0,
    SPA,
    THA,
    TPA,
    PACKET_END
} eARPPacketPtrType_t;

/* This is the STATIC, non-changing, part of an ARP packet. There is data past this
 * header but the later elements change in size. */
typedef struct __attribute__((packed))
{
	uint16_t usHType;	  /**< Network Link Protocol type                     0 +  2 =  2 */
	uint16_t usPType;	  /**< The internetwork protocol                      2 +  2 =  4 */
	uint8_t ucHALength;	  /**< Length in octets of a hardware address         4 +  1 =  5 */
	uint8_t ucPALength;	  /**< Length in octets of the internetwork protocol  5 +  1 =  6 */
	uint16_t usOperation; /**< Operation that the sender is performing        6 +  2 =  8 */

						  // MACAddress_t xSha;             /z**< Media address of the sender                    8 +  6 = 14 */
						  // uint32_t ucSpa;            		/**< Internetwork address of sender                14 +  4 = 18  */
						  // MACAddress_t xTha;             /**< Media address of the intended receiver        18 +  6 = 24  */
						  // uint32_t ulTpa;                /**< Internetwork address of the intended receiver 24 +  4 = 28  */
} ARPStaticHeader_t;

/*
 * All the data of an ARP packet including pointers to AP addresses stored in the packet.
 */
typedef struct {
    ARPStaticHeader_t *pxARPHdr;

    /* Pointers to the ARP request data within the packet. */
    buffer_t ucSpa;
    buffer_t ucSha;
    buffer_t ucTpa;
    buffer_t ucTha;

    /* Intepreted version of the ARP request data. Those members are
     * copy of the packet content and need to be free. */
    gpa_t *pxSpa;
    gha_t *pxSha;
    gpa_t *pxTpa;
    gha_t *pxTha;

} ARPPacketData_t;

#ifdef __cplusplus
}
#endif

#endif // _ARP_DEFS_H_INCLUDED
