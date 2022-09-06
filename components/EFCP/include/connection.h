/*
 * connection.h
 *
 *  Created on: 15 Nov. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_EFCP_INCLUDE_CONNECTION_H_
#define COMPONENTS_EFCP_INCLUDE_CONNECTION_H_

#include "portability/port.h"

#ifdef __cplusplus
extern "C" {
#endif

struct connection_t *  pxConnectionCreate(void);

bool_t xConnectionDestroy(struct connection_t * pxConn);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_EFCP_INCLUDE_CONNECTION_H_ */
