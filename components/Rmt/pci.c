/*
 * pci.c
 *
 *  Created on: 1 oct. 2021
 *      Author: i2CAT
 */


#include <stdio.h>

#include "rmt.h"
#include "du.h"

#include "efcpStructures.h"

#if 0
static efcpConfig_t * pxPciGetEfcpConfig(const pci_t * pxPci)
{
	struct du_t * pxPdu;

	pxPdu = container_of(pxPci, du_t, xPci);

	return pxPdu->pxCfg;
}


#define PCI_GETTER_NO_DTC(pci, pci_index, size, type)			\
	{struct efcpConfig_t * cfg;					\
	cfg = pxPciGetEfcpConfig(pci);				\
	switch (size) {							\
	case (1):							\
		return (type) *((uint8_t*)					\
		(pci->h + cfg->pci_offset_table[pci_index]));		\
	case (2):							\
		return (type) *((uint16_t*)				\
		(pci->h + cfg->pci_offset_table[pci_index]));		\
	case (4):							\
		return (type) *((uint32_t*)				\
		(pci->h + cfg->pci_offset_table[pci_index]));		\
	}								\
	return pdFALSE;}

bool_t xPCIGetterNoDtc (pci_t * pxPci);

pduType_t xPciType(const pci_t * pxPci)
{ PCI_GETTER_NO_DTC(pxPci, PCI_BASE_TYPE, TYPE_SIZE, pduType_t); }


#endif


bool_t xPciIsOk(const pci_t *pxPci)
{

	if (pxPci && sizeof(pxPci) > 0 && pdu_type_is_ok(pxPci->xType))
		return true;
	return false;
}

void vPciPrint(const pci_t * pxPciTmp)
{
	LOGE(TAG_RMT, "Printing PCI");
	LOGE(TAG_RMT, "Type: %02x", pxPciTmp->xType);
	LOGE(TAG_RMT, "PDU len: %04x", pxPciTmp->xPduLen);
	LOGE(TAG_RMT, "PDU Address Source: %02x", pxPciTmp->xSource);
	LOGE(TAG_RMT, "PDU Address Destination: %02x", pxPciTmp->xDestination);
	LOGE(TAG_RMT, "CEP destination: %02x",pxPciTmp->connectionId_t.xDestination);
	LOGE(TAG_RMT, "CEP source: %02x",pxPciTmp->connectionId_t.xSource);
	LOGE(TAG_RMT, "QoSid: %02x",pxPciTmp->connectionId_t.xQosId);
	LOGE(TAG_RMT, "PDU Flag : %02x",pxPciTmp->xFlags);
	LOGE(TAG_RMT, "Version: %02x",pxPciTmp->ucVersion);
}

#if 0
cepId_t xPciCepSource(const pci_t * pxPci)
{ PCI_GETTER(pxPci, PCI_BASE_SRC_CEP, cep_id_length, cepId_t); }
#endif

