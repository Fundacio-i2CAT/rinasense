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


#include "common.h"
#include "IPCP.h"


/* Endian related definitions. */
//#if ( ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN )

/* FreeRTOS_htons / FreeRTOS_htonl: some platforms might have built-in versions
 * using a single instruction so allow these versions to be overridden. */
//#ifndef FreeRTOS_htons
#define FreeRTOS_htons( usIn )    ( ( uint16_t ) ( ( ( usIn ) << 8U ) | ( ( usIn ) >> 8U ) ) )
//#endif

#ifndef FreeRTOS_htonl
#define FreeRTOS_htonl( ulIn )                          \
		(                                                               \
				( uint32_t )                                                \
				(                                                           \
						( ( ( ( uint32_t ) ( ulIn ) ) ) << 24 ) |               \
						( ( ( ( uint32_t ) ( ulIn ) ) & 0x0000ff00UL ) << 8 ) | \
						( ( ( ( uint32_t ) ( ulIn ) ) & 0x00ff0000UL ) >> 8 ) | \
						( ( ( ( uint32_t ) ( ulIn ) ) ) >> 24 )                 \
				)                                                           \
		)
#endif /* ifndef FreeRTOS_htonl */

//#else /* ipconfigBYTE_ORDER */

//#define FreeRTOS_htons( x )    ( ( uint16_t ) ( x ) )
//#define FreeRTOS_htonl( x )    ( ( uint32_t ) ( x ) )

//#endif /* ipconfigBYTE_ORDER == pdFREERTOS_LITTLE_ENDIAN */

#define FreeRTOS_ntohs( x )    FreeRTOS_htons( x )
#define FreeRTOS_ntohl( x )    FreeRTOS_htonl( x )




//Structure MAC ADDRESS
typedef struct xMAC_ADDRESS
{
	uint8_t ucBytes[ MAC_ADDRESS_LENGTH_BYTES ]; /**< Byte array of the MAC address */
}MACAddress_t;


//enum MAC Address
typedef enum {
        MAC_ADDR_802_3
} eGHAType_t;

//Structure Generic Protocol Address
typedef struct xGENERIC_PROTOCOL_ADDRESS
{
	uint8_t * 	ucAddress;
	size_t		uxLength;
}gpa_t;

//Structure Generic Hardware Address
typedef struct xGENERIC_HARDWARE_ADDRESS
{
	eGHAType_t 		xType;
	MACAddress_t 	xAddress;
}gha_t;

//Structure Ethernet Header
typedef struct xETH_HEADER
{
	MACAddress_t xDestinationAddress; /**< Destination address  0 + 6 = 6  */
	MACAddress_t xSourceAddress;      /**< Source address       6 + 6 = 12 */
	uint16_t usFrameType;             /**< The EtherType field 12 + 2 = 14 */
}EthernetHeader_t;


extern DECL_CAST_PTR_FUNC_FOR_TYPE( EthernetHeader_t );
extern DECL_CAST_CONST_PTR_FUNC_FOR_TYPE( EthernetHeader_t );

//Structure ARP Header
typedef struct xARP_HEADER
{
	uint16_t usHType;              /**< Network Link Protocol type                     0 +  2 =  2 */
	uint16_t usPType;              /**< The internetwork protocol                      2 +  2 =  4 */
	uint8_t ucHALength;            /**< Length in octets of a hardware address         4 +  1 =  5 */
	uint8_t ucPALength;            /**< Length in octets of the internetwork protocol  5 +  1 =  6 */
	uint16_t usOperation;          /**< Operation that the sender is performing        6 +  2 =  8 */
	//MACAddress_t xSha;             /**< Media address of the sender                    8 +  6 = 14 */
	//uint32_t ucSpa;            		/**< Internetwork address of sender                14 +  4 = 18  */
	//MACAddress_t xTha;             /**< Media address of the intended receiver        18 +  6 = 24  */
	//uint32_t ulTpa;                /**< Internetwork address of the intended receiver 24 +  4 = 28  */
}ARPHeader_t;

//Structure ARP Packet
typedef struct xARP_PACKET
{
	EthernetHeader_t xEthernetHeader; /**< The ethernet header of an ARP Packet  0 + 14 = 14 */
	ARPHeader_t xARPHeader;           /**< The ARP header of an ARP Packet       14 + 28 = 42 */
}ARPPacket_t;

extern DECL_CAST_PTR_FUNC_FOR_TYPE( ARPPacket_t );
extern DECL_CAST_CONST_PTR_FUNC_FOR_TYPE( ARPPacket_t );

extern DECL_CAST_PTR_FUNC_FOR_TYPE( MACAddress_t );
extern DECL_CAST_CONST_PTR_FUNC_FOR_TYPE( MACAddress_t );

/**
 * Structure for one row in the ARP cache table.
 */
typedef struct xARP_CACHE_TABLE_ROW
{
	gpa_t *  pxProtocolAddress;     /**< The IPCP address of an ARP cache entry. */
	gha_t *  pxMACAddress; /**< The MAC address of an ARP cache entry. */
	uint8_t ucAge;            /**< A value that is periodically decremented but can also be refreshed by active communication.  The ARP cache entry is removed if the value reaches zero. */
	uint8_t ucValid;          /**< pdTRUE: xMACAddress is valid, pdFALSE: waiting for ARP reply */
} ARPCacheRow_t;

typedef enum
{
	eARPCacheMiss = 0, /* 0 An ARP table lookup did not find a valid entry. */
	eARPCacheHit,      /* 1 An ARP table lookup found a valid entry. */
	eCantSendPacket    /* 2 There is no IPCP address, or an ARP is still in progress, so the packet cannot be sent. */
} eARPLookupResult_t;

typedef struct RINA_ARP_HANDLE
{
	gpa_t * 	pxPa;
	gha_t * 	pxHa;
}rinarpHandle_t;

/* Ethernet frame types. */
#define ARP_FRAME_TYPE                   ( 0x0608U )


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
eARPLookupResult_t eARPGetCacheEntry( gpa_t * pulIPAddress,
		gha_t * const pxMACAddress );


eFrameProcessingResult_t eARPProcessPacket( ARPPacket_t * const pxARPFrame );


/*************** RINA ***************************/
//UpdateMACAddress
void vARPUpdateMACAddress( const uint8_t ucMACAddress[ MAC_ADDRESS_LENGTH_BYTES ], const MACAddress_t * pxPhyDev);

//Request ARP for a mapping of a network address to a MAC address.
void RINA_vARPMapping( uint32_t ulIPCPAddress );

// Adds a mapping of application name to MAC address in the ARP cache.
int vARPSendRequest( const gpa_t * tpa, const gpa_t * spa, const gha_t * sha );

// Remove all ARP entry in the ARP cache.
void vARPRemoveAll( void);



eARPLookupResult_t eARPLookupGPA(const  gpa_t * gpaToLookup);

void vARPRefreshCacheEntry( const gpa_t * ulIPCPAddress, const gha_t * pxMACAddress );
void vARPRemoveCacheEntry( const gpa_t * ulIPCPAddress, const gha_t * pxMACAddress );


BaseType_t  xARPRemove(const gpa_t * pxPa, const gha_t * pxHa);

rinarpHandle_t * pxARPAdd(gpa_t * pxPa, gha_t * pxHa);

BaseType_t xARPResolveGPA(const gpa_t * tpa, const gpa_t * spa, const gha_t * sha);


void vARPInitCache( void );

void vARPPrintCache (void);

void vPrintMACAddress(const gha_t * gha);

#endif /* ARP_H_ */

