/*
 * pci.c
 *
 *  Created on: 1 oct. 2021
 *      Author: i2CAT
 */


#include <stdio.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "RMT.h"
#include "ConfigSensor.h"
#include "common.h"
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

BaseType_t xPCIGetterNoDtc (pci_t * pxPci);

pduType_t xPciType(const pci_t * pxPci)
{ PCI_GETTER_NO_DTC(pxPci, PCI_BASE_TYPE, TYPE_SIZE, pduType_t); }


#endif


BaseType_t xPciIsOk(const pci_t *pxPci)
{

	if (pxPci && sizeof(pxPci) > 0 && pdu_type_is_ok(pxPci->xType))
		return true;
	return false;
}

#if 0
cepId_t xPciCepSource(const pci_t * pxPci)
{ PCI_GETTER(pxPci, PCI_BASE_SRC_CEP, cep_id_length, cepId_t); }
#endif

