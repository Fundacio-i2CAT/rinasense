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

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "Rmt.h"
#include "du.h"
#include "pci.h"
#include "IPCP.h"
#include "BufferManagement.h"

#define TAG_DTP "[DTP]"

BaseType_t xDuDestroy(struct du_t *pxDu)
{
	/* If there is an NetworkBuffer then release and release memory */
	if (pxDu->pxNetworkBuffer)
	{
		ESP_LOGI(TAG_DTP, "Destroying du struct and releasing Buffer");
		vReleaseNetworkBufferAndDescriptor(pxDu->pxNetworkBuffer);
	}
	// vPortFree(pxDu);

	return pdTRUE;
}

BaseType_t xDuDecap(struct du_t *pxDu)
{
	ESP_LOGI(TAG_DTP, "Decapsulating the DU");

	pduType_t xType;
	pci_t *pxPciTmp;
	size_t uxPciLen;
	// NetworkBufferDescriptor_t *pxNewBuffer;
	uint8_t *pucData;
	size_t uxBufferSize;

	/* Extract PCI from buffer*/
	pxPciTmp = vCastPointerTo_pci_t(pxDu->pxNetworkBuffer->pucRinaBuffer);

	// vPciPrint(pxPciTmp);

	xType = pxPciTmp->xType;
	if (unlikely(!pdu_type_is_ok(xType)))
	{
		ESP_LOGE(TAG_DTP, "Could not decap DU. Type is not ok");
		return pdTRUE;
	}

	uxPciLen = (size_t)(14); /* PCI defined static for this initial stage = 14Bytes*/

	uxBufferSize = pxDu->pxNetworkBuffer->xRinaDataLength - uxPciLen;

	// ESP_LOGE(TAG_ARP, "Taking Buffer to copy the SDU from the RINA PDU: DuDecap");
	// pxNewBuffer = pxGetNetworkBufferWithDescriptor(xBufferSize, (TickType_t)0U);
	// if (pxNewBuffer == NULL)
	//{
	//	ESP_LOGE(TAG_DTP, "NO buffer was allocated to do the Decap");
	//	return pdFALSE;
	//}
	// pxNewBuffer->xDataLength = xBufferSize;

	pucData = (uint8_t *)pxPciTmp + 14;
	pxDu->pxNetworkBuffer->xDataLength = uxBufferSize;
	pxDu->pxNetworkBuffer->pucDataBuffer = pucData;

	// memcpy(pxNewBuffer->pucEthernetBuffer, pucPtr, xBufferSize);

	pxDu->pxPci = pxPciTmp;

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

	return pdFALSE;
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

void print_bytes(void *ptr, int size)
{
	unsigned char *p = ptr;
	int i;
	for (i = 0; i < size; i++)
	{
		printf("%02hhX ", p[i]);
	}
	printf("\n");
}

BaseType_t xDuEncap(struct du_t *pxDu, pduType_t xType)
{
	size_t uxPciLen;
	NetworkBufferDescriptor_t *pxNewBuffer;
	uint8_t *pucDataPtr;
	size_t xBufferSize;
	pci_t *pxPciTmp;

	uxPciLen = (size_t)(14); /* PCI defined static for this initial stage = 14Bytes*/

	/* New Size = Data Size more the PCI size defined by default. */
	xBufferSize = pxDu->pxNetworkBuffer->xDataLength + uxPciLen;

	// ESP_LOGE(TAG_DTP, "Taking Buffer to encap PDU");
	pxNewBuffer = pxGetNetworkBufferWithDescriptor(xBufferSize, (TickType_t)0U);

	if (!pxNewBuffer)
	{
		ESP_LOGE(TAG_DTP, " Buffer was not allocated properly.");
		return pdFALSE;
	}

	pucDataPtr = (uint8_t *)(pxNewBuffer->pucEthernetBuffer + 14);

	memcpy(pucDataPtr, pxDu->pxNetworkBuffer->pucEthernetBuffer,
		   pxDu->pxNetworkBuffer->xDataLength);

	// ESP_LOGE(TAG_DTP, "Releasing Buffer after encap PDU");
	// vReleaseNetworkBufferAndDescriptor(pxDu->pxNetworkBuffer);

	pxNewBuffer->xDataLength = xBufferSize;

	pxDu->pxNetworkBuffer = pxNewBuffer;

	pxPciTmp = vCastPointerTo_pci_t(pxDu->pxNetworkBuffer->pucEthernetBuffer);

	pxDu->pxPci = pxPciTmp;

	return pdTRUE;
}

BaseType_t xDuIsOk(const struct du_t *pxDu)
{
	return (pxDu && pxDu->pxNetworkBuffer ? pdTRUE : pdFALSE);
}