#ifndef _ARP_DEFS_H_INCLUDED
#define _ARP_DEFS_H_INCLUDED

#include <stdint.h>

#include "common/mac.h"
#include "configSensor.h"

// Structure Ethernet Header
typedef struct __attribute__((packed))
{
	MACAddress_t xDestinationAddress; /**< Destination address  0 + 6 = 6  */
	MACAddress_t xSourceAddress;	  /**< Source address       6 + 6 = 12 */
	uint16_t usFrameType;			  /**< The EtherType field 12 + 2 = 14 */
} EthernetHeader_t;

// Structure ARP Header
typedef struct __attribute__((packed))
{
	uint16_t usHType;	  /**< Network Link Protocol type                     0 +  2 =  2 */
	uint16_t usPType;	  /**< The internetwork protocol                      2 +  2 =  4 */
	uint8_t ucHALength;	  /**< Length in octets of a hardware address         4 +  1 =  5 */
	uint8_t ucPALength;	  /**< Length in octets of the internetwork protocol  5 +  1 =  6 */
	uint16_t usOperation; /**< Operation that the sender is performing        6 +  2 =  8 */
						  // MACAddress_t xSha;             /**< Media address of the sender                    8 +  6 = 14 */
						  // uint32_t ucSpa;            		/**< Internetwork address of sender                14 +  4 = 18  */
						  // MACAddress_t xTha;             /**< Media address of the intended receiver        18 +  6 = 24  */
						  // uint32_t ulTpa;                /**< Internetwork address of the intended receiver 24 +  4 = 28  */
} ARPHeader_t;

// Structure ARP Packet
typedef struct __attribute__((packed))
{
	EthernetHeader_t xEthernetHeader; /**< The ethernet header of an ARP Packet  0 + 14 = 14 */
	ARPHeader_t xARPHeader;			  /**< The ARP header of an ARP Packet       14 + 28 = 42 */
} ARPPacket_t;

#endif // _ARP_DEFS_H_INCLUDED
