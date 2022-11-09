/*
 * du.c
 *
 *  Created on: 30 sept. 2021
 *      Author: i2CAT
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common/rsrc.h"
#include "rmt.h"
#include "du.h"
#include "pci.h"
#include "BufferManagement.h"

#define TAG_DTP "[DTP]"

bool_t xDuDestroy(struct du_t *pxDu)
{
	/* If there is an NetworkBuffer then release and release memory */
	if (pxDu->pxNetworkBuffer)
		vReleaseNetworkBufferAndDescriptor(pxDu->pxNetworkBuffer);

	vRsrcFree(pxDu);

	return true;
}

bool_t xDuDecap(struct du_t *pxDu)
{
	pduType_t xType;
	pci_t *pxPciTmp;
	size_t uxPciLen;
	NetworkBufferDescriptor_t *pxNewBuffer;
	uint8_t *pucData;
	size_t uxBufferSize;

	/* Extract PCI from buffer*/
	pxPciTmp = (pci_t *)pxDu->pxNetworkBuffer->pucRinaBuffer;

	xType = pxPciTmp->xType;
	if (!pdu_type_is_ok(xType)) {
		LOGE(TAG_DTP, "Could not decap DU of type %d", xType);
		return true;
	}

	uxBufferSize = pxDu->pxNetworkBuffer->xRinaDataLength - sizeof(pci_t);

	pxNewBuffer = pxGetNetworkBufferWithDescriptor(uxBufferSize, 1000);
	if (pxNewBuffer == NULL) {
		LOGE(TAG_DTP, "Failed to allocate buffer for DU decap");
		return false;
	}
	pxNewBuffer->xDataLength = uxBufferSize;

	pucData = (uint8_t *)pxPciTmp + 14;
	pxDu->pxNetworkBuffer->xDataLength = uxBufferSize;
	pxDu->pxNetworkBuffer->pucDataBuffer = pucData;

	// memcpy(pxNewBuffer->pucEthernetBuffer, pucPtr, xBufferSize);

	pxDu->pxPci = pxPciTmp;

#if 0
	ESP_LOGI(TAG_DTP, "------------ PCI DT DECAP-----------");
	ESP_LOGI(TAG_DTP, "PCI Version: 0x%04x", pxDu->pxPci->ucVersion);
	ESP_LOGI(TAG_DTP, "PCI SourceAddress: 0x%04x", pxDu->pxPci->xSource);
	ESP_LOGI(TAG_DTP, "PCI DestinationAddress: 0x%04x", pxDu->pxPci->xDestination);
	ESP_LOGI(TAG_DTP, "PCI QoS: 0x%04x", pxDu->pxPci->connectionId_t.xQosId);
	ESP_LOGI(TAG_DTP, "PCI CEP Source: 0x%04x", pxDu->pxPci->connectionId_t.xSource);
	ESP_LOGI(TAG_DTP, "PCI CEP Destination: 0x%04x", pxDu->pxPci->connectionId_t.xDestination);
	ESP_LOGI(TAG_DTP, "PCI FLAG: 0x%04x", pxDu->pxPci->xFlags);
	ESP_LOGI(TAG_DTP, "PCI Type: 0x%04x", pxDu->pxPci->xType);
	ESP_LOGI(TAG_DTP, "PCI SequenceNumber: 0x%08x", pxDu->pxPci->xSequenceNumber);
	ESP_LOGI(TAG_DTP, "PCI xPDULEN: 0x%04x", pxDu->pxPci->xPduLen);
#endif
	/*
		pxDu->pxPci->ucVersion = pxPciTmp->ucVersion;
		pxDu->pxPci->xSource = pxPciTmp->xSource;
		pxDu->pxPci->xDestination = pxPciTmp->xDestination;
		pxDu->pxPci->xFlags = pxPciTmp->xFlags;
		pxDu->pxPci->xPduLen = pxPciTmp->xPduLen;
		pxDu->pxPci->xSequenceNumber = pxPciTmp->xSequenceNumber;
		pxDu->pxPci->xType = pxPciTmp->xType;
		pxDu->pxPci->connectionId_t = pxPciTmp->connectionId_t;*/

	// ESP_LOGE(TAG_DTP, "Releasing Buffer after copy the SDU from the RINA PDU:DuDcap");
	// vReleaseNetworkBufferAndDescriptor(pxDu->pxNetworkBuffer);
	// pxDu->pxNetworkBuffer = pxNewBuffer;

	return false;
}

ssize_t xDuDataLen(const struct du_t *pxDu)
{
	if (pxDu->pxPci->xPduLen != pxDu->pxNetworkBuffer->xRinaDataLength) /* up direction */
		return pxDu->pxNetworkBuffer->xRinaDataLength;

	return (pxDu->pxNetworkBuffer->xDataLength - pxDu->pxPci->xPduLen); /* down direction */
}

size_t xDuLen(const struct du_t *pxDu)
{
	return pxDu->pxNetworkBuffer->xRinaDataLength;
}

bool_t xDuEncap(struct du_t *pxDu, pduType_t xType)
{
	NetworkBufferDescriptor_t *pxNewBuffer;
	uint8_t *pucDataPtr;
	size_t xBufferSize;
	pci_t * pxPciTmp;

	/* New Size = Data Size more the PCI size defined by default. */
	xBufferSize = pxDu->pxNetworkBuffer->xDataLength + sizeof(pci_t);

	pxNewBuffer = pxGetNetworkBufferWithDescriptor(xBufferSize, 1000);
	if (!pxNewBuffer) {
		LOGE(TAG_DTP, "Failed to allocated DU buffer for encap");
		return false;
	}

	pucDataPtr = (uint8_t *)(pxNewBuffer->pucEthernetBuffer + sizeof(pci_t));

    /* Copy the RINA packet inside the new buffer. */
	memcpy(pucDataPtr, pxDu->pxNetworkBuffer->pucEthernetBuffer,
		   pxDu->pxNetworkBuffer->xDataLength);

	// ESP_LOGE(TAG_DTP, "Releasing Buffer after encap PDU");
	vReleaseNetworkBufferAndDescriptor(pxDu->pxNetworkBuffer);

	pxNewBuffer->xDataLength = xBufferSize;

	pxDu->pxNetworkBuffer = pxNewBuffer;

    pxPciTmp = (pci_t *)pxDu->pxNetworkBuffer->pucEthernetBuffer;

	pxDu->pxPci = pxPciTmp;

	return true;
}

bool_t xDuIsOk(const struct du_t *pxDu)
{
	return (pxDu && pxDu->pxNetworkBuffer ? true : false);
}
