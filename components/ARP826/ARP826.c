#include <sys/param.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common/list.h"
#include "common/mac.h"
#include "common/rina_gpha.h"
#include "common/rina_name.h"
#include "common/macros.h"

#include "ARP826.h"
#include "ARP826_defs.h"
#include "ARP826_cache_defs.h"
#include "ARP826_cache_api.h"
#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "configSensor.h"
#include "IPCP_api.h"
#include "IPCP_frames.h"
#include "portability/rslog.h"
#include "rina_buffers.h"

static void prvARPGeneratePacket(NetworkBufferDescriptor_t *const pxNetworkBuffer,
								 const gha_t *pxSha, const gpa_t *pxSpa, const gpa_t *pxTpa, uint16_t usPtype);

/**
 * Return a pointer to a specific item in the ARP header.
 */
buffer_t pxARPHeaderGetPtr(eARPPacketPtrType_t ePtrType, ARPHeader_t *const pxBase)
{
    size_t n = sizeof(ARPHeader_t);
    void *p = (void *)pxBase + n;

    if (ePtrType == SHA)
        return p;
    if (ePtrType >= SPA)
        p += pxBase->ucHALength;
    if (ePtrType >= THA)
        p += pxBase->ucPALength;
    if (ePtrType >= TPA)
        p += pxBase->ucHALength;

    /* Only ARP replies have space beyond for the target protocol address. */
    if (ePtrType >= PACKET_END && pxBase->usOperation == ARP_REPLY)
        p += pxBase->ucPALength;

    return p;
}

void prvARPFreeRequestData(ARPPacketData_t *pxARPData)
{
    if (pxARPData->pxSpa)
        vGPADestroy(pxARPData->pxSpa);
    if (pxARPData->pxSha)
        vGHADestroy(pxARPData->pxSha);
    if (pxARPData->pxTpa)
        vGPADestroy(pxARPData->pxTpa);
    if (pxARPData->pxTha)
        vGHADestroy(pxARPData->pxTha);
}

bool_t prvARPExtractRequestData(ARPPacket_t *const pxBase, ARPPacketData_t *pxARPData)
{
    size_t nPlen;

    /* Those really should be checked somewhere else but they really
       need to be true for this function to work at all. */
    RsAssert(pxBase->xARPHeader.usHType == RsHtoNS(ARP_HARDWARE_TYPE_ETHERNET));
    RsAssert(pxBase->xARPHeader.ucHALength == sizeof(MACAddress_t));

    nPlen = pxBase->xARPHeader.ucPALength;

    pxARPData->pxARPPacket = pxBase;
    pxARPData->ucSha = pxARPHeaderGetPtr(SHA, &pxBase->xARPHeader);
    pxARPData->ucSpa = pxARPHeaderGetPtr(SPA, &pxBase->xARPHeader);
    pxARPData->ucTha = pxARPHeaderGetPtr(THA, &pxBase->xARPHeader);
    pxARPData->ucTpa = pxARPHeaderGetPtr(TPA, &pxBase->xARPHeader);

    if ((pxARPData->pxSha = pxCreateGHA(MAC_ADDR_802_3, (MACAddress_t *)pxARPData->ucSha)) == NULL)
        goto fail;

    if ((pxARPData->pxSpa = pxCreateGPA(pxARPData->ucSpa, nPlen)) == NULL)
        goto fail;

    if ((pxARPData->pxTha = pxCreateGHA(MAC_ADDR_802_3, (MACAddress_t *)pxARPData->ucTha)) == NULL)
        goto fail;
    else
        if (!xGPAShrink(pxARPData->pxSpa, 0x00))
            goto fail;

    if (pxARPData->ucTpa) {
        if ((pxARPData->pxTpa = pxCreateGPA(pxARPData->ucTpa, nPlen)) == NULL)
            goto fail;
        else
            if (!xGPAShrink(pxARPData->pxTpa, 0x00))
                goto fail;
    }

    return true;

    fail:
    LOGE(TAG_ARP, "Memory or parse error extracting information for ARP request.");

    prvARPFreeRequestData(pxARPData);

    return false;
}

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
/*-----------------------------------------------------------*/

bool_t xARPInit(ARP_t *pxARP)
{
    memset(pxARP, 0, sizeof(ARP_t));

    pxARP->pxCache = pxARPCacheCreate(MAC_ADDR_802_3, ARP_CACHE_ENTRIES);
    if (!pxARP->pxCache)
        return false;

    //vRsListInit(&pxARP->xRegisteredAppHandles);

    return true;
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
bool_t vARPSendRequest(ARP_t *pxArp, const gpa_t *pxTpa, const gpa_t *pxSpa, const gha_t *pxSha)
{
	NetworkBufferDescriptor_t *pxNetworkBuffer;
	size_t xMaxLen;
	size_t xBufferSize;
    gpa_t *pxGrownSpa, *pxGrownTpa;
    RINAStackEvent_t xSendEvent;

	xMaxLen = MAX(pxSpa->uxLength, pxTpa->uxLength);
    xBufferSize = sizeof(ARPPacket_t) + (xMaxLen + sizeof(pxSha->xAddress)) * 2;

	/* This is called from the context of the IPCP event task, so a block time
	 * must not be used. */
	if (!(pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(xBufferSize, 0))) {
        LOGE(TAG_SHIM, "Failed to allocate network buffer");
        return false;
    }

    if (!(pxGrownSpa = pxDupGPA(pxSpa, false, 0))) {
        LOGE(TAG_ARP, "Failed to duplicate SPA");
        goto err;
    }

    if (!(pxGrownTpa = pxDupGPA(pxTpa, false, 0))) {
        LOGE(TAG_ARP, "Failed to duplicate TPA");
        goto err;
    }

	if (!xGPAGrow(pxGrownSpa, xMaxLen, 0x00)) {
		LOGE(TAG_ARP, "Failed to grow SPA");
        goto err;
	}

	if (!xGPAGrow(pxGrownTpa, xMaxLen, 0x00)) {
		LOGE(TAG_ARP, "Failed to grow TPA");
		goto err;
	}

    prvARPGeneratePacket(pxNetworkBuffer, pxSha, pxSpa, pxTpa, ARP_REQUEST);

    vGPADestroy(pxGrownSpa);
    vGPADestroy(pxGrownTpa);

    if (xIsCallingFromIPCPTask()) {
        LOGI(TAG_ARP, "Sending ARP request directly to network interface");

        /* Only the IPCP-task is allowed to call this function directly. */
        (void)xNetworkInterfaceOutput(pxNetworkBuffer, true);

    } else {
        LOGI(TAG_ARP, "Sending ARP request to IPCP");

        /* Send a message to the IPCP-task to send this ARP packet. */
        xSendEvent.eEventType = eNetworkTxEvent;
        xSendEvent.xData.PV = (void *)pxNetworkBuffer;

        if (!xSendEventStructToIPCPTask(&xSendEvent, 1000))     {
            /* Failed to send the message, so release the network buffer. */
            vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
            return false;
        }
    }

    return true;

    err:
    if (pxGrownSpa)
        vGPADestroy(pxGrownSpa);
    if (pxGrownTpa)
        vGPADestroy(pxGrownTpa);

    return false;
}

static void prvARPGeneratePacket(NetworkBufferDescriptor_t *const pxNetworkBuffer,
								 const gha_t *pxSha, const gpa_t *pxSpa, const gpa_t *pxTpa, uint16_t usPtype)
{

	ARPPacket_t *pxARPPacket;
	unsigned char *pucArpPtr;
	size_t uxLength;

	//pxTha = pxARPCreateGHAUnknown(MAC_ADDR_802_3);
    const gha_t *pxTha = &xBroadcastHa;

	uxLength = sizeof(*pxARPPacket) + (pxSpa->uxLength + sizeof(pxSha->xAddress)) * 2;

	/* Buffer allocation ensures that buffers always have space
	 * for an ARP packet.  */
	RsAssert(pxNetworkBuffer != NULL);
	RsAssert(pxNetworkBuffer->xDataLength >= uxLength);

	pxARPPacket = (ARPPacket_t *)pxNetworkBuffer->pucEthernetBuffer;

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

	pxARPPacket->xARPHeader.usHType = RsHtoNS(0x0001);
	pxARPPacket->xARPHeader.usPType = RsHtoNS(ETH_P_RINA);
	pxARPPacket->xARPHeader.ucHALength = sizeof(pxSha->xAddress.ucBytes);
	pxARPPacket->xARPHeader.ucPALength = pxSpa->uxLength;
	pxARPPacket->xARPHeader.usOperation = usPtype;
	pxARPPacket->xEthernetHeader.usFrameType = RsHtoNS(ETH_P_RINA_ARP);

	memcpy(pxARPPacket->xEthernetHeader.xSourceAddress.ucBytes, pxSha->xAddress.ucBytes, sizeof(pxSha->xAddress));
	memcpy(pxARPPacket->xEthernetHeader.xDestinationAddress.ucBytes, pxTha->xAddress.ucBytes, sizeof(pxTha->xAddress));

	pucArpPtr = (unsigned char *)(pxARPPacket + 1);

	memcpy(pucArpPtr, pxSha->xAddress.ucBytes, sizeof(pxSha->xAddress));
	pucArpPtr += sizeof(pxSha->xAddress);

	memcpy(pucArpPtr, pxSpa->pucAddress, pxSpa->uxLength);
	pucArpPtr += pxSpa->uxLength;

	/* THA */
	memcpy(pucArpPtr, pxTha->xAddress.ucBytes, sizeof(pxTha->xAddress));
	pucArpPtr += sizeof(pxTha->xAddress);

	/* TPA */
	memcpy(pucArpPtr, pxTpa->pucAddress, pxTpa->uxLength);
	pucArpPtr += pxTpa->uxLength;

	pxNetworkBuffer->xDataLength = uxLength;

	LOGD(TAG_ARP, "Generated Request Packet ");
}

bool_t prvPrepareARPReply(NetworkBufferDescriptor_t *const pxNetBuf,
                          const ARPPacketData_t *pxARPPacketData,
                          const gha_t *pxLookupResult)
{
    const size_t paLength = pxARPPacketData->pxARPPacket->xARPHeader.ucPALength;

    /* Make the operation a reply. */
    pxARPPacketData->pxARPPacket->xARPHeader.usOperation = ARP_REPLY;

    /* ARP reply, insert the request target address as source. */
    memset(pxARPPacketData->ucSpa, 0, paLength);
    memcpy(pxARPPacketData->ucSpa, pxARPPacketData->pxTpa->pucAddress, pxARPPacketData->pxTpa->uxLength);

    /* Insert the target ethernet packet address as source hardware address */
    //memcpy(pxARPPacketData->ucSha,
    //&pxARPPacketData->pxTha->xAddress, MAC_ADDRESS_LENGTH_BYTES);
    memcpy(pxARPPacketData->ucSha, &pxLookupResult->xAddress, MAC_ADDRESS_LENGTH_BYTES);

    /* Insert the request source protocol address as target protocol address */
    memset(pxARPPacketData->ucTpa, 0, paLength);
    memcpy(pxARPPacketData->ucTpa, pxARPPacketData->pxSpa->pucAddress, pxARPPacketData->pxSpa->uxLength);

    /* And, finally, the request source hardware address becomes the
       target hardware address. */
    memcpy(pxARPPacketData->ucTha, &pxARPPacketData->pxARPPacket->xEthernetHeader.xSourceAddress, MAC_ADDRESS_LENGTH_BYTES);

    return true;
}

eFrameProcessingResult_t eARPProcessPacket(ARP_t *pxARP, NetworkBufferDescriptor_t *const pxNetBuf)
{
	eFrameProcessingResult_t eReturn = eReleaseBuffer;
    ARPPacket_t *pxARPFrame;
	ARPHeader_t *pxARPHeader;
	struct rinarpHandle_t *pxHandle;
    ARPPacketData_t xARPPacketData;

    /* Pointer to a cache result. Should not be freed. */
    const gha_t *pxLookupResult;

	/* Get ARPHeader from the ARPFrame */
    pxARPFrame = (ARPPacket_t *)pxNetBuf->pucEthernetBuffer;
	pxARPHeader = &(pxARPFrame->xARPHeader);

    /* Sanity checks on the only address type we deal with: Ethernet */
	if (pxARPHeader->usHType != RsHtoNS(ARP_HARDWARE_TYPE_ETHERNET)) {
		LOGE(TAG_ARP, "Unhandled ARP hardware type 0x%04X", pxARPHeader->usHType);
		return eReturn;
	}
	if (pxARPHeader->ucHALength != 6) {
		LOGE(TAG_ARP, "Unhandled ARP hardware address length (%d)", pxARPHeader->ucHALength);
		return eReturn;
	}

    LOGD(TAG_ARP, "ARP Header usOperation: 0x%x", pxARPHeader->usOperation);

	if (pxARPHeader->usOperation != ARP_REPLY && pxARPHeader->usOperation != ARP_REQUEST) {
		LOGE(TAG_ARP, "Unhandled ARP operation 0x%04X", pxARPHeader->usOperation);
		return eReturn;
	}

    /* Extract the data we need to prepare a request */
    if (!prvARPExtractRequestData(pxARPFrame, &xARPPacketData))
        return eReturn;

    switch (pxARPHeader->usOperation) {

	case ARP_REQUEST:
		LOGD(TAG_ARP, "Handling ARP_REQUEST (0x%x)", pxARPHeader->usOperation);

        /* Cache miss */
		if (eARPCacheLookupGPA(pxARP->pxCache, xARPPacketData.pxTpa, &pxLookupResult) == eARPCacheMiss) {
#ifndef NDEBUG
            {
                char *psAddr;

                RsAssert((psAddr = xGPAAddressToString(xARPPacketData.pxTpa)) != NULL);
                LOGE(TAG_ARP, "Nothing found for entry %s for protocol type 0x%04X",
                     psAddr, pxARPHeader->usPType);
                vRsMemFree(psAddr);
            }
#else
			LOGE(TAG_ARP, "Nothing found for protocol type 0x%04X", usPtype);
#endif
		} else {
            /* The request is for the address of this node.  Add the
             * entry into the ARP cache, or refresh the entry if it
             * already exists. */
            //vARPCacheRefresh(pxARP->pxCache, xARPPacketData.pxTpa, xARPPacketData.pxTha);

            /* Turn the buffer into an ARP reply. */
            if (!prvPrepareARPReply(pxNetBuf, &xARPPacketData, pxLookupResult)) {
                LOGE(TAG_ARP, "Failed to prepare ARP buffer");
                eReturn = eReleaseBuffer;
            }

            eReturn = eReturnEthernetFrame;
        }
		break;

	case ARP_REPLY:
		LOGD(TAG_ARP, "Handling ARP_REPLY (0x%x)", pxARPHeader->usOperation);

        xARPCacheAdd(pxARP->pxCache, xARPPacketData.pxTpa, xARPPacketData.pxTha);

		eReturn = eProcessBuffer;

		break;
	}

    prvARPFreeRequestData(&xARPPacketData);

	return eReturn;
}


/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/**
 * @brief Generate an ARP request packet by copying various constant details to
 *        the buffer.
 *
 * @param[in,out] pxNetworkBuffer: Pointer to the buffer which has to be filled with
 *                             the ARP request packet details.
 */
/*-----------------------------------------------------------*/

/**
 * @brief Create and send an ARP request packet.
 *
 * @param[in] ulIPCPAddress: A 32-bit representation of the IP-address whose
 *                         physical (MAC) address is required.
 */

/*-----------------------------------------------------------*/
/**
 * @brief A call to this function will update the default configuration MAC Address
 * after the WIFI driver is initialized.
 */
void vARPUpdateMACAddress(const uint8_t ucMACAddress[MAC_ADDRESS_LENGTH_BYTES], const MACAddress_t *pxPhyDev)
{

	(void)memcpy((void *)pxPhyDev->ucBytes, ucMACAddress, (size_t)MAC_ADDRESS_LENGTH_BYTES);
	LOGI(TAG_ARP, "MAC Address updated");
}

/*-----------------------------------------------------------*/
/**
 * @brief Decide whether this packet should be processed or not based on the IPCP address in the packet.
 *
 * @param[in] pucEthernetBuffer: The ethernet packet under consideration.
 *
 * @return Enum saying whether to release or to process the packet.
 */

bool_t xARPResolveGPA(ARP_t *pxARP, const gpa_t *pxTpa, const gpa_t *pxSpa, const gha_t *pxSha)
{

	if (!xIsGPAOK(pxTpa) || !xIsGHAOK(pxSha) || !xIsGPAOK(pxSpa)) {
		LOGE(TAG_ARP, "Parameters are not correct, won't resolve GPA");
		return false;
	}

	return vARPSendRequest(pxARP, pxTpa, pxSpa, pxSha);
}

/* Add an application mapping to the ARP cache */
ARPCacheHandle xARPAddApplication(ARP_t *pxARP, const gpa_t *pxPa, const gha_t *pxHa)
{
    ARPCacheHandle nHndl;

    nHndl = xARPCacheAdd(pxARP->pxCache, pxPa, pxHa);

    return nHndl;
}

/* Remove an application mapping from the ARP cache. */
bool_t xARPRemoveApplication(ARP_t *pxARP, const gpa_t *pxPa)
{
    return xARPCacheRemove(pxARP->pxCache, pxPa);
}

const gha_t *pxARPLookupGHA(ARP_t *pxARP, const gpa_t *pxHa) {
    return pxARPCacheLookupGHA(pxARP->pxCache, pxHa);
}

