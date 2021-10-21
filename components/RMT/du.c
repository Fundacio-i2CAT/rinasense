/*
 * du.c
 *
 *  Created on: 30 sept. 2021
 *      Author: i2CAT
 */



#include <stdio.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "RMT.h"
#include "du.h"
#include "pci.h"
#include "IPCP.h"
#include "BufferManagement.h"

BaseType_t xDuDestroy(du_t * pxDu)
{
	/*If there is an NetworkBuffer then release and */
	if (pxDu->pxNetworkBuffer)
		vReleaseNetworkBuffer( pxDu->pxNetworkBuffer->pucEthernetBuffer );

	vPortFree(pxDu);

	return pdTRUE;
}


BaseType_t xDuDecap(du_t * pxDu)
{
	pduType_t xType;
	size_t uxPciLen;

	pxDu->xPci.pucH = pxDu->pxNetworkBuffer->pucEthernetBuffer;
	xType = pci_type(&pxDu->xPci);
	if (unlikely(!pdu_type_is_ok(xType))) {
		LOG_ERR("Could not decap DU. Type is not ok");
		return -1;
	}



	xPciLen = pci_calculate_size(pxDu->cfg, xType);
	if (uxPciLen <= 0) {
		LOG_ERR("Could not decap DU. PCI len is < 0");
		return -1;
	}

	/* Make up for tail padding introduced at lower layers. */
	if (pxDu->skb->len > pci_length(&du->pci)) {
		du_tail_shrink(pxDu, pxDu->skb->len - pci_length(&pxDu->pci));
	}

	skb_pull(pxDu->skb, uxPciLen);
	pxDu->pci.len = uxPciLen;

	return 0;
}


