/*Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

/* RINA includes. */
#include "ARP826.h"
#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "ShimIPCP.h"
#include "configSensor.h"

#include "esp_log.h"





const MACAddress_t xlocalMACAddress = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };
MACAddress_t xBroadcastMACAddress = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };

IPCPAddress_t xlocalIPCPAddress = { {0x00, 0x00, 0x00, 0x00} };



#define LOCAL_IPCP_ADDRESS_POINTER    ( ( uint32_t * ) &( xlocalIPCPAddress) )



/*-----------------------------------------------------------*/

void vARPGenerateRequestPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer );

ARPPacket_t * vCastPointerTo_ARPPacket_t(void * pvArgument);

EthernetHeader_t * vCastPointerTo_EthernetPacket_t(const void * pvArgument);

/*-----------------------------------------------*/
/*
 * If ulIPAddress is already in the ARP cache table then reset the age of the
 * entry back to its maximum value.  If ulIPAddress is not already in the ARP
 * cache table then add it - replacing the oldest current entry if there is not
 * a free space available.
 */


void vARPRefreshCacheEntry( const MACAddress_t * pxMACAddress,
		const uint32_t ulIPAddress );


/** @brief The ARP cache.
 * Array of ARPCacheRows. The type ARPCacheRow_t has been set at ARP.h */
static ARPCacheRow_t xARPCache[ ARP_CACHE_ENTRIES ];





/*-----------------------------------------------------------*/

/**
 * @brief Process the ARP packets.
 *
 * @param[in] pxARPFrame: The ARP Frame (the ARP packet).
 *
 * @return An enum which says whether to return the frame or to release it.
 * The eFrameProcessingResult_t is defined in the Shim_IPCP.h
 *
 */
eFrameProcessingResult_t eARPProcessPacket( ARPPacket_t * const pxARPFrame )
{

    eFrameProcessingResult_t eReturn = eReleaseBuffer;
    ARPHeader_t * pxARPHeader;
    uint32_t ulTpa, ulSpa;


    const void * pvCopySource;
    void * pvCopyDest;

    //Get ARPHeader from the ARPFrame
    pxARPHeader = &( pxARPFrame->xARPHeader );

    /* The field ulSpa is badly aligned, copy byte-by-byte. */


    pvCopySource = pxARPHeader->ucSpa;
    pvCopyDest = &ulSpa;
    ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( ulSpa ) );
    /* The field ulTpa is well-aligned, a 32-bits copy. */
    ulTpa = pxARPHeader->ulTpa;


    /* Don't do anything if the local IPCP address is zero because
     * that means the enrollment has not completed. */
    if( *LOCAL_IPCP_ADDRESS_POINTER != 0UL ) //Local IPCP address should be set anytime.
    {
        switch( pxARPHeader->usOperation )
        {
            case ARP_REQUEST:

                /* The packet contained an ARP request.  Verify it this packet target is
                 * for the local IoT node */
                if( ulTpa == *LOCAL_IPCP_ADDRESS_POINTER )
                {


                    /* The request is for the address of this IoT node.  Add the
                     * entry into the ARP cache, or refresh the entry if it
                     * already exists. */
                    vARPRefreshCacheEntry( &( pxARPHeader->xSha ), ulSpa );

                    /* Generate a reply payload in the same buffer. */
                    pxARPHeader->usOperation = ( uint16_t ) ARP_REPLY;

                    if( ulTpa == ulSpa )
                    {
                        /* A double IPCP address is detected! */
                        /* Give the sources MAC address the value of the broadcast address, will be swapped later */

                        /*
                         * Use helper variables for memcpy() to remain
                         * compliant with MISRA Rule 21.15.  These should be
                         * optimized away.
                         */
                        pvCopySource = xBroadcastMACAddress.ucBytes;
                        pvCopyDest = pxARPFrame->xEthernetHeader.xSourceAddress.ucBytes;
                        ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( xBroadcastMACAddress ) );

                        ( void ) memset( pxARPHeader->xTha.ucBytes, 0, sizeof( MACAddress_t ) );
                        pxARPHeader->ulTpa = 0UL;
                    }
                    else
                    {
                        /*
                         * Use helper variables for memcpy() to remain
                         * compliant with MISRA Rule 21.15.  These should be
                         * optimized away.
                         */
                        pvCopySource = pxARPHeader->xSha.ucBytes;
                        pvCopyDest = pxARPHeader->xTha.ucBytes;
                        ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( MACAddress_t ) );
                        pxARPHeader->ulTpa = ulSpa;
                    }

                    /*
                     * Use helper variables for memcpy() to remain
                     * compliant with MISRA Rule 21.15.  These should be
                     * optimized away.
                     */
                    pvCopySource = xlocalMACAddress.ucBytes;
                    pvCopyDest = pxARPHeader->xSha.ucBytes;
                    ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( MACAddress_t ) );
                    pvCopySource = LOCAL_IPCP_ADDRESS_POINTER;
                    pvCopyDest = pxARPHeader->ucSpa;
                    ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( pxARPHeader->ucSpa ) );

                    eReturn = eReturnEthernetFrame;
                }

                break;

            case ARP_REPLY:

                vARPRefreshCacheEntry( &( pxARPHeader->xSha ), ulSpa );


                break;

            default:
                /* Invalid. */
                break;
        }
    }

    return eReturn;
}

/*-----------------------------------------------------------*/

/**
 * @brief Generate an ARP request packet by copying various constant details to
 *        the buffer.
 *
 * @param[in,out] pxNetworkBuffer: Pointer to the buffer which has to be filled with
 *                             the ARP request packet details.
 */
void vARPGenerateRequestPacket( NetworkBufferDescriptor_t * const pxNetworkBuffer )
{
/* Part of the Ethernet and ARP headers are always constant when sending an
 * ARP packet. This array defines the constant parts, allowing this part of the
 * packet to be filled in using a simple memcpy() instead of individual writes. */
    static const uint8_t xDefaultPartARPPacketHeader[] =
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* Ethernet destination address. */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Ethernet source address. */
        0x08, 0x06,                         /* Ethernet frame type (ARP_FRAME_TYPE). */
        0x00, 0x01,                         /* usHardwareType (ARP_HARDWARE_TYPE_ETHERNET). */
        0x08, 0x00,                         /* usProtocolType. */
        MAC_ADDRESS_LENGTH_BYTES,         /* ucHardwareAddressLength. */
        IPCP_ADDRESS_LENGTH_BYTES,          /* ucProtocolAddressLength. */
        0x00, 0x01,                         /* usOperation (ARP_REQUEST). */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* xSha. */
        0x00, 0x00, 0x00, 0x00,             /* ulSpa. */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* xTha. */
    };

    ARPPacket_t * pxARPPacket;

/* memcpy() helper variables for MISRA Rule 21.15 compliance*/
    const void * pvCopySource;
    void * pvCopyDest;

    /* Buffer allocation ensures that buffers always have space
     * for an ARP packet.  */
    configASSERT( pxNetworkBuffer != NULL );
    configASSERT( pxNetworkBuffer->xDataLength >= sizeof( ARPPacket_t ) );

    pxARPPacket = vCastPointerTo_ARPPacket_t( pxNetworkBuffer->pucEthernetBuffer );

    /* memcpy the const part of the header information into the correct
     * location in the packet.  This copies:
     *  xEthernetHeader.ulDestinationAddress
     *  xEthernetHeader.usFrameType;
     *  xARPHeader.usHardwareType;
     *  xARPHeader.usProtocolType;
     *  xARPHeader.ucHardwareAddressLength;
     *  xARPHeader.ucProtocolAddressLength;
     *  xARPHeader.usOperation;
     *  xARPHeader.xTargetHardwareAddress;
     */

    /*
     * Use helper variables for memcpy() to remain
     * compliant with MISRA Rule 21.15.  These should be
     * optimized away.
     */
    pvCopySource = xDefaultPartARPPacketHeader;
    pvCopyDest = pxARPPacket;
    ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( xDefaultPartARPPacketHeader ) );

    pvCopySource = xlocalMACAddress.ucBytes;
    pvCopyDest = pxARPPacket->xEthernetHeader.xSourceAddress.ucBytes;
    ( void ) memcpy( pvCopyDest, pvCopySource, MAC_ADDRESS_LENGTH_BYTES );

    pvCopySource = xlocalMACAddress.ucBytes;
    pvCopyDest = pxARPPacket->xARPHeader.xSha.ucBytes;
    ( void ) memcpy( pvCopyDest, pvCopySource, MAC_ADDRESS_LENGTH_BYTES );

    pvCopySource = LOCAL_IPCP_ADDRESS_POINTER;
    pvCopyDest = pxARPPacket->xARPHeader.ucSpa;
    ( void ) memcpy( pvCopyDest, pvCopySource, sizeof( pxARPPacket->xARPHeader.ucSpa ) );
    pxARPPacket->xARPHeader.ulTpa = pxNetworkBuffer->ulIPCPAddress;

    pxNetworkBuffer->xDataLength = sizeof( ARPPacket_t );


}
/*-----------------------------------------------------------*/

/**
 * @brief Add/update the ARP cache entry MAC-address to IPCP-address mapping.
 *
 * @param[in] pxMACAddress: Pointer to the MAC address whose mapping is being
 *                          updated.
 * @param[in] ulIPCPAddress: 32-bit representation of the IPCP-address whose mapping
 *                         is being updated.
 */
void vARPRefreshCacheEntry( const MACAddress_t * pxMACAddress,
                            const uint32_t ulIPCPAddress )
{
	BaseType_t x = 0;
	BaseType_t xIpcpEntry = -1;
	BaseType_t xMacEntry = -1;
	BaseType_t xUseEntry = 0;
	uint8_t ucMinAgeFound = 0U;


	/* Start with the maximum possible number. */
	ucMinAgeFound--;

	/* For each entry in the ARP cache table. */
	for( x = 0; x < ARP_CACHE_ENTRIES; x++ ) //lookup in the cache for the MacAddress
	{
		BaseType_t xMatchingMAC;

		if( pxMACAddress != NULL ) //Check if pointer is not empty
		{
			//Comparison between Cache MACAddress and the MacAddress looking up for.
			if( memcmp( xARPCache[ x ].xMACAddress.ucBytes, pxMACAddress->ucBytes, sizeof( pxMACAddress->ucBytes ) ) == 0 )
			{
				xMatchingMAC = pdTRUE;
			}
			else
			{
				xMatchingMAC = pdFALSE;
			}
		}
		else
		{
			xMatchingMAC = pdFALSE;
		}

		/* Check if the IPCPAddress is already on the Cache */
		if( xARPCache[ x ].ulIPCPAddress == ulIPCPAddress )
		{
			if( pxMACAddress == NULL )
			{
				/* In case the parameter pxMACAddress is NULL, an entry will be reserved to
				 * indicate that there is an outstanding ARP request, This entry will have
				 * "ucValid == pdFALSE". */
				xIpcpEntry = x;
				break; //break an keep position to update the ARP request.
			}

			/* See if the MAC-address also matches. */
			if( xMatchingMAC != pdFALSE )
			{
				/* This function will be called for each received packet
				 * As this is by far the most common path the coding standard
				 * is relaxed in this case and a return is permitted as an
				 * optimisation. */
				xARPCache[ x ].ucAge = ( uint8_t ) MAX_ARP_AGE; //update ARP Age
				xARPCache[ x ].ucValid = ( uint8_t ) pdTRUE; //update ucValid
				return; // Do not update cache entry because there is already an entry in cache.
			}

			/* Found an entry containing ulIPCPAddress, but the MAC address
			 * doesn't match.  Might be an entry with ucValid=pdFALSE, waiting
			 * for an ARP reply.  Still want to see if there is match with the
			 * given MAC address.ucBytes.  If found, either of the two entries
			 * must be cleared. */
			xIpcpEntry = x;
		}
		else if( xMatchingMAC != pdFALSE )
		{
			/* Found an entry with the given MAC-address, but the IPCP-address
			 * is different.  Continue looping to find a possible match with
			 * ulIPCPAddress. */

			xMacEntry = x;

		}

		/* _HT_
		 * Shouldn't we test for xARPCache[ x ].ucValid == pdFALSE here ? */
		else if( xARPCache[ x ].ucAge < ucMinAgeFound )
		{
			/* As the table is traversed, remember the table row that
			 * contains the oldest entry (the lowest age count, as ages are
			 * decremented to zero) so the row can be re-used if this function
			 * needs to add an entry that does not already exist. */
			ucMinAgeFound = xARPCache[ x ].ucAge;
			xUseEntry = x;
		}
		else
		{
			/* Nothing happens to this cache entry for now. */
		}
	}

	if( xMacEntry >= 0 )
	{
		xUseEntry = xMacEntry;

		if( xIpcpEntry >= 0 )
		{
			/* Both the MAC address as well as the IPCP address were found in
			 * different locations: clear the entry which matches the
			 * IPCP-address */
			( void ) memset( &( xARPCache[ xIpcpEntry ] ), 0, sizeof( ARPCacheRow_t ) );
		}
	}
	else if( xIpcpEntry >= 0 )
	{
		/* An entry containing the IPCP-address was found, but it had a different MAC address */
		xUseEntry = xIpcpEntry;
	}
	else
	{
		/* No matching entry found. */
	}

	/* If the entry was not found, we use the oldest entry and set the IPCPaddress */
	xARPCache[ xUseEntry ].ulIPCPAddress = ulIPCPAddress;

	if( pxMACAddress != NULL )
	{
		( void ) memcpy( xARPCache[ xUseEntry ].xMACAddress.ucBytes, pxMACAddress->ucBytes, sizeof( pxMACAddress->ucBytes ) );


		/* And this entry does not need immediate attention */
		xARPCache[ xUseEntry ].ucAge = ( uint8_t ) MAX_ARP_AGE;
		xARPCache[ xUseEntry ].ucValid = ( uint8_t ) pdTRUE;
	}
	else if( xIpcpEntry < 0 )
	{
		xARPCache[ xUseEntry ].ucAge = ( uint8_t ) MAX_ARP_RETRANSMISSIONS;
		xARPCache[ xUseEntry ].ucValid = ( uint8_t ) pdFALSE;
	}
	else
	{
		/* Nothing will be stored. */
	}

}



/*-----------------------------------------------------------*/

/**
 * @brief Lookup an IPCP address in the ARP cache.
 *
 * @param[in] ulAddressToLookup: The 32-bit representation of an IP address to
 *                               lookup.
 * @param[out] pxMACAddress: A pointer to MACAddress_t variable where, if there
 *                          is an ARP cache hit, the MAC address corresponding to
 *                          the IP address will be stored.
 *
 * @return When the IPCP-address is found: eARPCacheHit, when not found: eARPCacheMiss,
 *         and when waiting for a ARP reply: eCantSendPacket.
 */
eARPLookupResult_t RINA_prvARPMapping( uint32_t ulAddressToLookup,
                                          MACAddress_t * const pxMACAddress )
{
    BaseType_t x;
    eARPLookupResult_t eReturn = eARPCacheMiss;

    /* Loop through each entry in the ARP cache. */
    for( x = 0; x < ARP_CACHE_ENTRIES; x++ )
    {
        /* Does this row in the ARP cache table hold an entry for the IPCP address
         * being queried? */
        if( xARPCache[ x ].ulIPCPAddress == ulAddressToLookup )
        {
            /* A matching valid entry was found. */
            if( xARPCache[ x ].ucValid == ( uint8_t ) pdFALSE )
            {
                /* This entry is waiting an ARP reply, so is not valid. */
                eReturn = eCantSendPacket;
            }
            else
            {
                /* A valid entry was found. */
                ( void ) memcpy( pxMACAddress->ucBytes, xARPCache[ x ].xMACAddress.ucBytes, sizeof( MACAddress_t ) );
                eReturn = eARPCacheHit;
            }

            break;
        }
    }

    return eReturn;
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/**
 * @brief Create and send an ARP request packet.
 *
 * @param[in] ulIPCPAddress: A 32-bit representation of the IP-address whose
 *                         physical (MAC) address is required.
 */
void RINA_vARPAdd( uint32_t ulIPCPAddress )
{
    NetworkBufferDescriptor_t * pxNetworkBuffer;

    /* This is called from the context of the IPCP event task, so a block time
     * must not be used. */
    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( sizeof( ARPPacket_t ), ( TickType_t ) 0U );

    if( pxNetworkBuffer != NULL )
    {
        pxNetworkBuffer->ulIPCPAddress = ulIPCPAddress;
        vARPGenerateRequestPacket( pxNetworkBuffer );


        if( xIsCallingFromIPCPTask() != pdFALSE )
        {

            /* Only the IPCP-task is allowed to call this function directly. */
            ( void ) xNetworkInterfaceOutput( pxNetworkBuffer, pdTRUE );
        }
        else
        {
            RINAStackEvent_t xSendEvent;

            /* Send a message to the IPCP-task to send this ARP packet. */
            xSendEvent.eEventType = eNetworkTxEvent;
            xSendEvent.pvData = pxNetworkBuffer;

            if( xSendEventStructToIPCPTask( &xSendEvent, ( TickType_t ) portMAX_DELAY ) == pdFAIL )
            {
                /* Failed to send the message, so release the network buffer. */
                vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
            }
        }
    }
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
/**
 * @brief A call to this function will update the default configuration MAC Address
 * after the WIFI driver is initialized.
 */
void RINA_vARPUpdateMACAddress( const uint8_t ucMACAddress[ MAC_ADDRESS_LENGTH_BYTES ] )
{


    ( void ) memcpy( xlocalMACAddress.ucBytes, ucMACAddress, ( size_t ) MAC_ADDRESS_LENGTH_BYTES );

}

/*-----------------------------------------------------------*/
/**
 * @brief A call to this function will update the default configuration MAC Address
 * after the WIFI driver is initialized.
 */
void RINA_vARPUpdateIPCPAddress( const uint32_t ulIPCPAddress[ MAC_ADDRESS_LENGTH_BYTES ] )
{


    ( void ) memcpy( xlocalIPCPAddress.ucBytes, ulIPCPAddress, ( size_t ) IPCP_ADDRESS_LENGTH_BYTES );

}

/*-----------------------------------------------------------*/

/**
 * @brief A call to this function will clear the all ARP cache.
 */
void RINA_vARPremove( void )
{
    ( void ) memset( xARPCache, 0, sizeof( xARPCache ) );
}
/*-----------------------------------------------------------*/

ARPPacket_t * vCastPointerTo_ARPPacket_t(void * pvArgument)
{
	return (void *) (pvArgument);
}

/*-----------------------------------------------------------*/
EthernetHeader_t * vCastPointerTo_EthernetPacket_t(const void * pvArgument)
{
	return (const void *) (pvArgument);
}

/*-----------------------------------------------------------*/
/**
 * @brief Decide whether this packet should be processed or not based on the IPCP address in the packet.
 *
 * @param[in] pucEthernetBuffer: The ethernet packet under consideration.
 *
 * @return Enum saying whether to release or to process the packet.
 */
eFrameProcessingResult_t eConsiderFrameForProcessing( const uint8_t * const pucEthernetBuffer )
{
    eFrameProcessingResult_t eReturn;
    const EthernetHeader_t * pxEthernetHeader;

    /* Map the buffer onto Ethernet Header struct for easy access to fields. */
    pxEthernetHeader = vCastPointerTo_EthernetPacket_t( pucEthernetBuffer );

    if( memcmp( xlocalMACAddress.ucBytes, pxEthernetHeader->xDestinationAddress.ucBytes, sizeof( MACAddress_t ) ) == 0 )
    {
        /* The packet was directed to this node - process it. */
        eReturn = eProcessBuffer;
    }
    else if( memcmp( xBroadcastMACAddress.ucBytes, pxEthernetHeader->xDestinationAddress.ucBytes, sizeof( MACAddress_t ) ) == 0 )
    {
        /* The packet was a broadcast - process it. */
        eReturn = eProcessBuffer;
    }
    else
    {
        /* The packet was not a broadcast, or for this node, just release
         * the buffer without taking any other action. */
        eReturn = eReleaseBuffer;
    }



    return eReturn;
}
