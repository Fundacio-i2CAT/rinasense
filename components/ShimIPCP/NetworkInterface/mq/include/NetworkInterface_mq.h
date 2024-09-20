#ifndef _NETWORK_INTERFACE_MQ_H
#define _NETWORK_INTERFACE_MQ_H

#include "portability/port.h"

/* Read data that has been written through the network interface API */
bool_t xMqNetworkInterfaceReadOutput(string_t data, size_t sz, ssize_t *pnBytesRead);

/* Make some data available to the network interface. */
bool_t xMqNetworkInterfaceWriteInput(string_t data, size_t sz);

/* Get the number of messages in the output queue */
long xMqNetworkInterfaceOutputCount();

/* Flush the output queue. */
void xMqNetworkInterfaceOutputDiscard();

#endif // _NETWORK_INTERFACE_MQ
