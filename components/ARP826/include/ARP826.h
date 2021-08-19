/*
 * ARP.h
 *
 *  Created on: 5 ago. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_SHIM_IPCP_INCLUDE_ARP_H_
#define COMPONENTS_SHIM_IPCP_INCLUDE_ARP_H_

#include "shimIPCP.h"
#include "configSensor.h"

/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/



//Structure IPCP ADDRESS
typedef struct xIPCP_ADDRESS
{
	uint8_t ucBytes[ IPCP_ADDRESS_LENGTH_BYTES ]; /**< Byte array of the IPCP address */
}IPCPAddress_t;


//Structure MAC ADDRESS
typedef struct xMAC_ADDRESS
{
	uint8_t ucBytes[ MAC_ADDRESS_LENGTH_BYTES ]; /**< Byte array of the MAC address */
}MACAddress_t;



//Structure Ethernet Header
typedef struct xETH_HEADER
{
	MACAddress_t xDestinationAddress; /**< Destination address  0 + 6 = 6  */
	MACAddress_t xSourceAddress;      /**< Source address       6 + 6 = 12 */
	uint16_t usFrameType;             /**< The EtherType field 12 + 2 = 14 */
}EthernetHeader_t;



//Structure ARP Header
typedef struct xARP_HEADER
{
	uint16_t usHType;              /**< Network Link Protocol type                     0 +  2 =  2 */
	uint16_t usPType;              /**< The internetwork protocol                      2 +  2 =  4 */
	uint8_t ucHALength;            /**< Length in octets of a hardware address         4 +  1 =  5 */
	uint8_t ucPALength;            /**< Length in octets of the internetwork protocol  5 +  1 =  6 */
	uint16_t usOperation;          /**< Operation that the sender is performing        6 +  2 =  8 */
	MACAddress_t xSha;             /**< Media address of the sender                    8 +  6 = 14 */
	uint8_t ucSpa[ 4 ];            /**< Internetwork address of sender                14 +  4 = 18  */
	MACAddress_t xTha;             /**< Media address of the intended receiver        18 +  6 = 24  */
	uint32_t ulTpa;                /**< Internetwork address of the intended receiver 24 +  4 = 28  */
}ARPHeader_t;

//Structure ARP Packet
typedef struct xARP_PACKET
{
	EthernetHeader_t xEthernetHeader; /**< The ethernet header of an ARP Packet  0 + 14 = 14 */
	ARPHeader_t xARPHeader;           /**< The ARP header of an ARP Packet       14 + 28 = 42 */
}ARPPacket_t;



/**
 * Structure for one row in the ARP cache table.
 */
typedef struct xARP_CACHE_TABLE_ROW
{
	uint32_t ulIPCPAddress;     /**< The IPCP address of an ARP cache entry. */
	MACAddress_t xMACAddress; /**< The MAC address of an ARP cache entry. */
	uint8_t ucAge;            /**< A value that is periodically decremented but can also be refreshed by active communication.  The ARP cache entry is removed if the value reaches zero. */
	uint8_t ucValid;          /**< pdTRUE: xMACAddress is valid, pdFALSE: waiting for ARP reply */
} ARPCacheRow_t;

typedef enum
{
	eARPCacheMiss = 0, /* 0 An ARP table lookup did not find a valid entry. */
	eARPCacheHit,      /* 1 An ARP table lookup found a valid entry. */
	eCantSendPacket    /* 2 There is no IPCP address, or an ARP is still in progress, so the packet cannot be sent. */
} eARPLookupResult_t;



/* Ethernet frame types. */
#define ARP_FRAME_TYPE                   ( 0x0608U )


/* ARP related definitions. */
#define ARP_PROTOCOL_TYPE                ( 0x0008U )
#define ARP_HARDWARE_TYPE_ETHERNET       ( 0x0100U )
#define ARP_REQUEST                      ( 0x0100U )
#define ARP_REPLY                        ( 0x0200U )




#if ( ipconfigUSE_ARP_REMOVE_ENTRY != 0 )

/*
 * In some rare cases, it might be useful to remove a ARP cache entry of a
 * known MAC address to make sure it gets refreshed.
 */
uint32_t ulARPRemoveCacheEntryByMac( const MACAddress_t * pxMACAddress );

#endif /* ipconfigUSE_ARP_REMOVE_ENTRY != 0 */


/************** ARP and Ethernet events handle *************************/
/*
 * Look for ulIPCPAddress in the ARP cache.  If the IPCP address exists, copy the
 * associated MAC address into pxMACAddress, refresh the ARP cache entry's
 * age, and return eARPCacheHit.  If the IPCP address does not exist in the ARP
 * cache return eARPCacheMiss.
 */
eARPLookupResult_t eARPGetCacheEntry( uint32_t * pulIPAddress,
		MACAddress_t * const pxMACAddress );

eFrameProcessingResult_t eConsiderFrameForProcessing( const uint8_t * const pucEthernetBuffer );


/*************** RINA ***************************/
//UpdateMACAddress
void RINA_vARPUpdateMACAddress( const uint8_t ucMACAddress[ MAC_ADDRESS_LENGTH_BYTES ] );

//Request ARP for a mapping of a network address to a MAC address.
void RINA_vARPMapping( uint32_t ulIPCPAddress );

// Adds a mapping of application name to MAC address in the ARP cache.
void RINA_vARPAdd( uint32_t ulIPCPAddress );

// Remove an ARP entry in the ARP cache. Return ARPCacheHit
BaseType_t RINA_xARPRemove( uint32_t ulIPCPAddress );

#endif /* COMPONENTS_SHIM_IPCP_INCLUDE_ARP_H_ */

