/*
 * delim.h
 *
 *  Created on: 27 oct. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_EFCP_INCLUDE_DELIM_H_
#define COMPONENTS_EFCP_INCLUDE_DELIM_H_

#include "freertos/FreeRTOS.h"
#include "common.h"
#include "efcpStructures.h"

typedef struct xDELIM
{
	/* The delimiting module instance */
	// struct rina_component base;

	/* The parent EFCP object instance */
	struct efcp_t *pxEfcp;

	/* The maximum fragment size for the DIF */
	uint32_t max_fragment_size;

} delim_t;

#endif /* COMPONENTS_EFCP_INCLUDE_DELIM_H_ */