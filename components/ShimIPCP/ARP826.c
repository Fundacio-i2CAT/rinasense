#include <sys/param.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "portability/port.h"
#include "common/rsrc.h"
#include "common/list.h"
#include "common/mac.h"
#include "common/eth.h"
#include "common/netbuf.h"
#include "common/rina_gpha.h"
#include "common/rina_name.h"
#include "common/macros.h"

#include "ARP826.h"
#include "ARP826_defs.h"
#include "ARP826_cache_defs.h"
#include "ARP826_cache_api.h"
#include "NetworkInterface.h"
#include "configRINA.h"
#include "IPCP_api.h"
#include "IPCP_frames.h"
#include "private/eth.h"

bool_t xARPInit(ARP_t *pxARP)
{
    memset(pxARP, 0, sizeof(ARP_t));

    if (!(pxARP->pxCache = pxARPCacheCreate(MAC_ADDR_802_3, CFG_ARP_CACHE_ENTRIES))) {
        LOGE(TAG_ARP, "Failed to create ARP cache zone");
        return false;
    }

    /* Memory pools for ARP packets and ethernet headers */
    if (!(pxARP->xArpPool = pxRsrcNewVarPool("ARP packets memory pool",
                                             CFG_ARP_POOL_MAX_RES))) {
        LOGE(TAG_ARP, "Failed to create ARP packets memory pool");
        return false;
    }

    if (!(pxARP->xEthPool = pxRsrcNewPool("ARP ethernet headers pool",
                                          sizeof(EthernetHeader_t),
                                          CFG_ARP_ETH_POOL_INIT_ALLOC,
                                          CFG_ARP_ETH_POOL_INCR_ALLOC,
                                          CFG_ARP_ETH_POOL_MAX_RES))) {
        LOGE(TAG_ARP, "Failed to create ARP ethernet headers memory pool");
        return false;
    }

    //vRsListInit(&pxARP->xRegisteredAppHandles);

    return true;
}

/**
 * Return a pointer to a specific item in the ARP packet data.
 */
static buffer_t pxARPHeaderGetPtr(eARPPacketPtrType_t ePtrType, ARPStaticHeader_t *const pxBase)
{
    size_t n = sizeof(ARPStaticHeader_t);
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

static void prvARPFreeRequestData(ARPPacketData_t *pxARPData)
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

static void prvARPSetupRequestData(ARPStaticHeader_t *const pxARPHdr, ARPPacketData_t *pxARPData)
{
    pxARPData->pxARPHdr = pxARPHdr;
    pxARPData->ucSha = pxARPHeaderGetPtr(SHA, pxARPHdr);
    pxARPData->ucSpa = pxARPHeaderGetPtr(SPA, pxARPHdr);
    pxARPData->ucTha = pxARPHeaderGetPtr(THA, pxARPHdr);
    pxARPData->ucTpa = pxARPHeaderGetPtr(TPA, pxARPHdr);
}

static bool_t prvARPExtractRequestData(ARPStaticHeader_t *const pxARPHdr, ARPPacketData_t *pxARPData)
{
    size_t nPlen;

    /* Those really should be checked somewhere else but they really
       need to be true for this function to work at all. */
    RsAssert(pxARPHdr->usHType == RsHtoNS(ARP_HARDWARE_TYPE_ETHERNET));
    RsAssert(pxARPHdr->ucHALength == sizeof(MACAddress_t));

    nPlen = pxARPHdr->ucPALength;

    prvARPSetupRequestData(pxARPHdr, pxARPData);

    if ((pxARPData->pxSha = pxCreateGHA(MAC_ADDR_802_3, (MACAddress_t *)pxARPData->ucSha)) == NULL)
        goto fail;

    if ((pxARPData->pxSpa = pxCreateGPA(pxARPData->ucSpa, nPlen)) == NULL)
        goto fail;

    if ((pxARPData->pxTha = pxCreateGHA(MAC_ADDR_802_3, (MACAddress_t *)pxARPData->ucTha)) == NULL)
        goto fail;
    else
        if (ERR_CHK(xGPAShrink(pxARPData->pxSpa, 0x00)))
            goto fail;

    if (pxARPData->ucTpa) {
        if ((pxARPData->pxTpa = pxCreateGPA(pxARPData->ucTpa, nPlen)) == NULL)
            goto fail;
        else
            if (ERR_CHK(xGPAShrink(pxARPData->pxTpa, 0x00)))
                goto fail;
    }

    return true;

    fail:
    LOGE(TAG_ARP, "Memory or parse error extracting information for ARP request.");

    prvARPFreeRequestData(pxARPData);

    return false;
}

/* Prepare a formatted ARP request */
static netbuf_t *prvARPGetRequest(ARP_t *pxArp,
                                  const gha_t *pxSha,
                                  const gpa_t *pxSpa,
                                  const gpa_t *pxTpa,
                                  uint16_t usPtype)
{
	ARPStaticHeader_t *pxArpHdr;
	buffer_t pxArpBuf;
	size_t unSz;
    netbuf_t *pxNbArp;
    void *pvArpPtr;
    const gha_t *pxTha = &xBroadcastHa;

	unSz = sizeof(ARPStaticHeader_t) + (pxSpa->uxLength + sizeof(pxSha->xAddress)) * 2;

    /* Allocates a buffer for the ARP content */
    if (!(pxArpBuf = pxRsrcVarAlloc(pxArp->xArpPool, "ARP packet", unSz)))
        return NULL;

    /* Then allocate the netbuf.. */
    if (!(pxNbArp = pxNetBufNew(pxArp->xArpPool, NB_RINA_ARP, pxArpBuf, unSz, NETBUF_FREE_POOL)))
        return NULL;

    pxArpHdr = (ARPStaticHeader_t *)pxArpBuf;

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

	pxArpHdr->usHType = RsHtoNS(0x0001);
	pxArpHdr->usPType = RsHtoNS(ETH_P_RINA);
	pxArpHdr->ucHALength = sizeof(pxSha->xAddress.ucBytes);
	pxArpHdr->ucPALength = pxSpa->uxLength;
	pxArpHdr->usOperation = usPtype;

    /* Don't forget that this work as: (void *)ptr + sizeof(ptr) */
	pvArpPtr = pxArpHdr + 1;

	memcpy(pvArpPtr, pxSha->xAddress.ucBytes, sizeof(pxSha->xAddress));
	pvArpPtr += sizeof(pxSha->xAddress);

	memcpy(pvArpPtr, pxSpa->pucAddress, pxSpa->uxLength);
	pvArpPtr += pxSpa->uxLength;

	/* THA */
	memcpy(pvArpPtr, pxTha->xAddress.ucBytes, sizeof(pxTha->xAddress));
	pvArpPtr += sizeof(pxTha->xAddress);

	/* TPA */
	memcpy(pvArpPtr, pxTpa->pucAddress, pxTpa->uxLength);
	pvArpPtr += pxTpa->uxLength;

    return pxNbArp;
}

/* Prepare a formatted ARP reply */
static netbuf_t *prvARPGetResponse(ARP_t *pxArp,
                                   netbuf_t *pxNbEth,
                                   const ARPPacketData_t *pxARPReq,
                                   const gha_t *pxLookupResult)
{
    netbuf_t *pxNbARPRes, *pxNbARPReq;
    buffer_t pxArpBuf;
    size_t unSzAddr, uxLength;
    EthernetHeader_t *pxEthHdr;
    ARPPacketData_t xARPRes;

    unSzAddr = pxARPReq->pxARPHdr->ucPALength;
	uxLength = sizeof(ARPStaticHeader_t) + (unSzAddr + sizeof(MACAddress_t)) * 2;
    pxEthHdr = (EthernetHeader_t *)pvNetBufPtr(pxNbEth);

    /* Allocates a buffer for the ARP content */
    if (!(pxArpBuf = pxRsrcVarAlloc(pxArp->xArpPool, "ARP packet", uxLength)))
        return NULL;

    /* Allocate a new netbuf for the response */
    if (!(pxNbARPRes = pxNetBufNew(pxNbEth->xPool, NB_RINA_ARP, pxArpBuf, uxLength, NETBUF_FREE_POOL))) {
        vRsrcFree(pxArpBuf);
        return NULL;
    }

    memcpy(pvNetBufPtr(pxNbARPRes), pxARPReq->pxARPHdr, sizeof(ARPStaticHeader_t));

    prvARPSetupRequestData(pvNetBufPtr(pxNbARPRes), &xARPRes);
    xARPRes.pxARPHdr->usOperation = ARP_REPLY;

    /* ARP reply, insert the request target address as source. */
    memset(xARPRes.ucSpa, 0, unSzAddr);
    memcpy(xARPRes.ucSpa, pxARPReq->pxTpa->pucAddress, pxARPReq->pxTpa->uxLength);

    /* Insert the target ethernet packet address as source hardware address */
    memcpy(xARPRes.ucSha, &pxLookupResult->xAddress, MAC_ADDRESS_LENGTH_BYTES);

    /* Insert the request source protocol address as target protocol address */
    memset(xARPRes.ucTpa, 0, unSzAddr);
    memcpy(xARPRes.ucTpa, pxARPReq->pxSpa->pucAddress, pxARPReq->pxSpa->uxLength);

    /* And, finally, the request source hardware address becomes the
       target hardware address. */
    memcpy(xARPRes.ucTha, pxEthHdr->xSourceAddress.ucBytes, MAC_ADDRESS_LENGTH_BYTES);

    /* Frees the netbuf containing the ARP request. */
    pxNbARPReq = pxNetBufNext(pxNbEth);
    vNetBufFree(pxNbARPReq);

    /* Add the new netbuf after the ethernet header that we'llI be
     * reusing */
    vNetBufAppend(pxNbEth, pxNbARPRes);

    return pxNbEth;
}

bool_t vARPSendRequest(ARP_t *pxArp, const gpa_t *pxTpa, const gpa_t *pxSpa, const gha_t *pxSha)
{
	size_t xMaxLen;
    gpa_t *pxGrownSpa = NULL, *pxGrownTpa = NULL;
    netbuf_t *pxNbEth = NULL, *pxNbArp = NULL;

	xMaxLen = MAX(pxSpa->uxLength, pxTpa->uxLength);

    if (!(pxGrownSpa = pxDupGPA(pxSpa, false, 0))) {
        LOGE(TAG_ARP, "Failed to duplicate SPA");
        goto err;
    }

    if (!(pxGrownTpa = pxDupGPA(pxTpa, false, 0))) {
        LOGE(TAG_ARP, "Failed to duplicate TPA");
        goto err;
    }

	if (ERR_CHK(xGPAGrow(pxGrownSpa, xMaxLen, 0x00))) {
		LOGE(TAG_ARP, "Failed to grow SPA");
        goto err;
	}

	if (ERR_CHK(xGPAGrow(pxGrownTpa, xMaxLen, 0x00))) {
		LOGE(TAG_ARP, "Failed to grow TPA");
		goto err;
	}

    if (!(pxNbArp = prvARPGetRequest(pxArp, pxSha, pxSpa, pxTpa, ARP_REQUEST))) {
        LOGE(TAG_ARP, "Failed to prepare ARP request paket");
        goto err;
    }

    if (!(pxNbEth = pxEthAllocHeader(pxArp->xEthPool, pxSha, &xBroadcastHa))) {
        LOGE(TAG_ARP, "Failed to prepare ARP request ethernet header");
        goto err;
    }

    vNetBufLink(pxNbEth, pxNbArp, 0);

    vGPADestroy(pxGrownSpa);
    vGPADestroy(pxGrownTpa);

    if (xIsCallingFromIPCPTask()) {
        LOGI(TAG_ARP, "Sending ARP request directly to network interface");

        xNetworkInterfaceOutput(pxNbEth);

    } else {
        LOGI(TAG_ARP, "Sending ARP request to IPCP");

        /* FIXME: This might block so I'm not sure this is wise. */
        xNetworkInterfaceOutput(pxNbEth);
    }

    return true;

    err:
    if (pxGrownSpa)
        vGPADestroy(pxGrownSpa);
    if (pxGrownTpa)
        vGPADestroy(pxGrownTpa);

    return false;
}

eFrameProcessingResult_t eARPProcessPacket(ARP_t *pxARP, netbuf_t *pxNbEth)
{
	eFrameProcessingResult_t eReturn = eReleaseBuffer;
	ARPStaticHeader_t *pxARPHdr;
    ARPPacketData_t xARPPacketData;
    netbuf_t *pxNbArpPkt, *pxNbArpRes;
    const gha_t *pxLookupResult; /* Do not free! */

    /* We really want 2 linked netbufs at this point. */
    RsAssert(pxNetBufNext(pxNbEth));

	/* Get ARPHeader from the ARPFrame */
    pxNbArpPkt = pxNetBufNext(pxNbEth);

    /* Allow access to the ethernet and ARP headers */
    pxARPHdr = (ARPStaticHeader_t *)pvNetBufPtr(pxNbArpPkt);

    /* Sanity checks on the only address type we deal with: Ethernet */
	if (pxARPHdr->usHType != RsHtoNS(ARP_HARDWARE_TYPE_ETHERNET)) {
		LOGE(TAG_ARP, "Unhandled ARP hardware type 0x%04X", pxARPHdr->usHType);
		return eReturn;
	}
	if (pxARPHdr->ucHALength != sizeof(MACAddress_t)) {
		LOGE(TAG_ARP, "Unhandled ARP hardware address length (%d)", pxARPHdr->ucHALength);
		return eReturn;
	}

    LOGD(TAG_ARP, "ARP Header usOperation: 0x%x", pxARPHdr->usOperation);

	if (pxARPHdr->usOperation != ARP_REPLY && pxARPHdr->usOperation != ARP_REQUEST) {
		LOGE(TAG_ARP, "Unhandled ARP operation 0x%04X", pxARPHdr->usOperation);
		return eReturn;
	}

    /* Extract the data we need to prepare a request */
    if (!prvARPExtractRequestData(pxARPHdr, &xARPPacketData))
        return eReturn;

    switch (pxARPHdr->usOperation) {

	case ARP_REQUEST:
		LOGD(TAG_ARP, "Handling ARP_REQUEST (0x%x)", pxARPHdr->usOperation);

        /* Cache miss */
		if (eARPCacheLookupGPA(pxARP->pxCache, xARPPacketData.pxTpa, &pxLookupResult) == eARPCacheMiss) {
#ifndef NDEBUG
            {
                char *psAddr;

                RsAssert((psAddr = xGPAAddressToString(xARPPacketData.pxTpa)) != NULL);
                LOGE(TAG_ARP, "Nothing found for entry %s for protocol type 0x%04X",
                     psAddr, pxARPHdr->usPType);
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
            if (!(pxNbArpRes = prvARPGetResponse(pxARP, pxNbEth, &xARPPacketData, pxLookupResult))) {
                LOGE(TAG_ARP, "Failed to prepare ARP response packet");
                eReturn = eReleaseBuffer;
            }

            eReturn = eReturnEthernetFrame;
        }
		break;

	case ARP_REPLY:
		LOGD(TAG_ARP, "Handling ARP_REPLY (0x%x)", pxARPHdr->usOperation);

        xARPCacheAdd(pxARP->pxCache, xARPPacketData.pxTpa, xARPPacketData.pxTha);

        /* There is some work to be done here handling pending ARP
         * replies. */

		eReturn = eReleaseBuffer;

		break;
	}

    prvARPFreeRequestData(&xARPPacketData);

	return eReturn;
}

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

