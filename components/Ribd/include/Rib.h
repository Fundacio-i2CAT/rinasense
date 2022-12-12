/*
 * Rib.h
 *
 *  Created on: 24 Mar. 2022
 *      Author: i2CAT
 */

#ifndef RIB_H_INCLUDED
#define RIB_H_INCLUDED

#include "portability/port.h"
#include "common/rina_ids.h"

#include "configSensor.h"

#include "FlowAllocator_api.h"

#ifdef __cplusplus
extern "C" {
#endif

ribObject_t *pxRibFindObject(Ribd_t *pxRibd, string_t ucRibObjectName);

bool_t xRibAddObjectEntry(Ribd_t *pxRibd, ribObject_t *pxRibObject);

#ifdef __cplusplus
}
#endif

#endif /* RIB_H_ */
