/*
 * connection.h
 *
 *  Created on: 15 Nov. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_EFCP_INCLUDE_CONNECTION_H_
#define COMPONENTS_EFCP_INCLUDE_CONNECTION_H_


#include "common.h"
#include "FreeRTOS/FreeRTOS.h"

connection_t *  pxConnectionCreate(void);

BaseType_t xConnectionDestroy(connection_t * pxConn);




#endif /* COMPONENTS_EFCP_INCLUDE_CONNECTION_H_ */