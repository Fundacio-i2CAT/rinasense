#ifndef _NETWORK_INTERFACE_MQ_H
#define _NETWORK_INTERFACE_MQ_H

/* Read data that has been written through the network interface API */
bool_t xMqNetworkInterfaceReadOutput(string_t data, size_t sz);

/* Make some data available to the network interface. */
bool_t xMqNetworkInterfaceWriteInput(string_t data, size_t sz);

#endif // _NETWORK_INTERFACE_MQ
