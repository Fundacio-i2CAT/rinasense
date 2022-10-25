/*
 * du.h
 *
 *  Created on: 30 sept. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_RMT_INCLUDE_DU_H_
#define COMPONENTS_RMT_INCLUDE_DU_H_

#include "portability/port.h"
#include "rina_buffers.h"
#include "pci.h"
#include "rina_common_port.h"

#ifdef __cplusplus
extern "C" {
#endif

struct du_t
{

	// Configuration of EFCP (Policies, QoS, etc)
	efcpConfig_t *pxCfg;						/*> EFCP Configuration associate to the RINA packet */
	pci_t *pxPci;								/*> Header of the RINA packet */
	NetworkBufferDescriptor_t *pxNetworkBuffer; /*> Pointer of the Network Buffer */
	uint8_t *pxPDU;								/*> Pointer to the firts bit of Data Unit */
};

bool_t xDuDestroy(struct du_t * pxDu);
size_t xDuLen(const struct du_t * pxDu);
bool_t xDuDecap(struct du_t * pxDu);
ssize_t xDuDataLen(const  struct du_t * pxDu);
bool_t xDuEncap(struct du_t * pxDu, pduType_t xType);

bool_t xDuIsOk(const struct du_t * pxDu);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_RMT_INCLUDE_DU_H_ */
