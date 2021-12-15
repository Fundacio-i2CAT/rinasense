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

struct du_t {

	//Configuration of EFCP (Policies, QoS, etc)
	efcpConfig_t *		        	pxCfg;
	pci_t	* 						pxPci;
	NetworkBufferDescriptor_t * 	pxNetworkBuffer;

};

BaseType_t xDuDestroy(struct du_t * pxDu);
size_t xDuLen(const struct du_t * pxDu);
BaseType_t xDuDecap(struct du_t * pxDu);
ssize_t xDuDataLen(const  struct du_t * pxDu);
BaseType_t xDuEncap(struct du_t * pxDu, pduType_t xType);

BaseType_t xDuIsOk(const struct du_t * pxDu);

#endif /* COMPONENTS_RMT_INCLUDE_DU_H_ */

