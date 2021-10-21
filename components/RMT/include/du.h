/*
 * du.h
 *
 *  Created on: 30 sept. 2021
 *      Author: i2CAT
 */


#ifndef COMPONENTS_RMT_INCLUDE_DU_H_
#define COMPONENTS_RMT_INCLUDE_DU_H_

#include "RMT.h"
#include "IPCP.h"
#include "common.h"
#include "pci.h"


typedef struct xDU {

	//Configuration of EFCP (Policies, QoS, etc)
	efcpConfig_t *					pxCfg;
	pci_t 							xPci;
	NetworkBufferDescriptor_t * 	pxNetworkBuffer;

}du_t;

BaseType_t xDuDestroy(du_t * pxDu);

BaseType_t xDuDecap(du_t * pxDu);

#endif /* COMPONENTS_RMT_INCLUDE_DU_H_ */

