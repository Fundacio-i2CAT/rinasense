#include <string.h>

#include "common/eth.h"
#include "common/netbuf.h"

#include "ARP826_defs.h"

netbuf_t *pxEthAllocHeader(rsrcPoolP_t xEthPool, const gha_t *pxSha, const gha_t *pxTha)
{
    netbuf_t *pxEthNb;
    EthernetHeader_t *pxHdr;

    /* Allocates a buffer for the ethernet header content */
    if (!(pxHdr = pxRsrcAlloc(xEthPool, "")))
        return NULL;

    /* New netbuf pointing to that buffer*/
    if (!(pxEthNb = pxNetBufNew(xEthPool, NB_ETH_HDR,
                                (buffer_t)pxHdr, sizeof(EthernetHeader_t),
                                NETBUF_FREE_POOL)))
        return NULL;

    pxHdr = (EthernetHeader_t *)pvNetBufPtr(pxEthNb);

    pxHdr->usFrameType = RsHtoNS(ETH_P_RINA_ARP);

	memcpy(pxHdr->xSourceAddress.ucBytes, pxSha->xAddress.ucBytes, sizeof(pxSha->xAddress));
	memcpy(pxHdr->xDestinationAddress.ucBytes, pxTha->xAddress.ucBytes, sizeof(pxTha->xAddress));

    return pxEthNb;
}
