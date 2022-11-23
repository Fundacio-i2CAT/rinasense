#ifndef _NETWORK_INTERFACE_H
#define _NETWORK_INTERFACE_H

#include "portability/port.h"

#include "common/netbuf.h"
#include "common/rina_gpha.h"

#include "IPCP_instance.h"

#ifdef __cplusplus
extern "C" {
#endif

bool_t xNetworkInterfaceInitialise(struct ipcpInstance_t *pxSelf, MACAddress_t *phyDev);

bool_t xNetworkInterfaceOutput(netbuf_t *pxNbFrame);
bool_t xNetworkInterfaceInput(void *buffer, uint16_t len, void *eb);

bool_t xNetworkInterfaceConnect(void);
bool_t xNetworkInterfaceDisconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* _NETWORK_INTERFACE_H */
