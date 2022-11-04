#ifndef _NETWORK_INTERFACE_H
#define _NETWORK_INTERFACE_H

#include "portability/port.h"
#include "common/rina_gpha.h"
#include "rina_common_port.h"

#include "configSensor.h"
#include "BufferManagement.h"

#ifdef __cplusplus
extern "C" {
#endif


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
    //#include "ShimIPCP.h"

    /* INTERNAL API FUNCTIONS. */
bool_t xNetworkInterfaceInitialise(struct ipcpInstance_t *pxSelf, MACAddress_t *phyDev);
    bool_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                                   bool_t xReleaseAfterSend);

    bool_t xNetworkInterfaceConnect(void);
    bool_t xNetworkInterfaceDisconnect(void);
    bool_t xNetworkInterfaceInput(void *buffer, uint16_t len, void *eb);

    /* The following function is defined only when BufferAllocation_1.c is linked in the project. */
    void vNetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t pxNetworkBuffers[NUM_NETWORK_BUFFER_DESCRIPTORS]);

    /* The following function is defined only when BufferAllocation_1.c is linked in the project. */
    bool_t xGetPhyLinkStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* _NETWORK_INTERFACE_H */
