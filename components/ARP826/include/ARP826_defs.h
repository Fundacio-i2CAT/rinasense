#ifndef _ARP_DEFS_H_INCLUDED
#define _ARP_DEFS_H_INCLUDED

#include <stdint.h>

#include "common/mac.h"
#include "common/rina_gpha.h"
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
} ARP_t;

typedef enum {
    SHA = 0,
    SPA,
    THA,
    TPA,
    PACKET_END
} eARPPacketPtrType_t;

// Structure Ethernet Header
typedef struct __attribute__((packed))
{
	MACAddress_t xDestinationAddress; /**< Destination address  0 + 6 = 6  */
	MACAddress_t xSourceAddress;	  /**< Source address       6 + 6 = 12 */
	uint16_t usFrameType;			  /**< The EtherType field 12 + 2 = 14 */
} EthernetHeader_t;

// Generic ARP Header
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
} ARPHeader_t;

/*
 * This is the headers of a ethernet + ARP packet
 */
typedef struct __attribute__((packed))
{
	EthernetHeader_t xEthernetHeader; /**< The ethernet header of an ARP Packet  0 + 14 = 14 */
	ARPHeader_t xARPHeader;			  /**< The ARP header of an ARP Packet       14 + 28 = 42 */
} ARPPacket_t;

/*
 * All the data of an ARP packet including pointers to ARJeP addresses stored in the packet.
 */
typedef struct {
    ARPPacket_t *pxARPPacket;

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
