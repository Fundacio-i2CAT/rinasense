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

#include "RMT.h"
#include "ConfigSensor.h"

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({         \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type,member) );})


static efcpConfig_t * __pci_efcp_config_get(const struct pci_t * pxPci)
{
	du_t * pxPdu;

	pxPdu = container_of(pxPci, struct du_t, pxPci);

	return pxPdu->pxCfg;
}

#define PCI_GETTER_NO_DTC(pci, pci_index, size, type)			\
	{struct efcpConfig_t * cfg;					\
	cfg = __pci_efcp_config_get(pci);				\
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

pduType_t xPciType(const struct pci_t * pxPci)
{ PCI_GETTER_NO_DTC(pxPci, PCI_BASE_TYPE, TYPE_SIZE, pduType_t); }



BaseType_t xPciIsOk(const struct pci_t * pxPci)
{
	if (pxPci&& pxPci-> && pxPci->len > 0 && pdu_type_is_ok(pci_type(pxPci)))
		return true;
	return false;
}


