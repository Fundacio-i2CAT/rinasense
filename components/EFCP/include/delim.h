/*
 * delim.h
 *
 *  Created on: 27 oct. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_EFCP_INCLUDE_DELIM_H_
#define COMPONENTS_EFCP_INCLUDE_DELIM_H_

#include "portability/port.h"
#include "efcpStructures.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xDELIM
{
	/* The delimiting module instance */
	// struct rina_component base;

	/* The parent EFCP object instance */
	struct efcp_t *pxEfcp;

	/* The maximum fragment size for the DIF */
	uint32_t max_fragment_size;

} delim_t;

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_EFCP_INCLUDE_DELIM_H_ */
