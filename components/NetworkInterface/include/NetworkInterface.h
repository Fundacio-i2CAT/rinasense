#ifndef _NETWORK_INTERFACE_H
#define _NETWORK_INTERFACE_H

#include "portability/port.h"
#include "common/rina_gpha.h"

#include "configSensor.h"
#include "BufferManagement.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
    //#include "ShimIPCP.h"

    /* INTERNAL API FUNCTIONS. */
    bool_t xNetworkInterfaceInitialise(MACAddress_t *phyDev);
    bool_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                                   bool_t xReleaseAfterSend);

    bool_t xNetworkInterfaceConnect(void);
    bool_t xNetworkInterfaceDisconnect(void);
    bool_t xNetworkInterfaceInput(void *buffer, uint16_t len, void *eb);

    /* The following function is defined only when BufferAllocation_1.c is linked in the project. */
    void vNetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t pxNetworkBuffers[NUM_NETWORK_BUFFER_DESCRIPTORS]);

    /* The following function is defined only when BufferAllocation_1.c is linked in the project. */
    bool_t xGetPhyLinkStatus(void);

#endif /* _NETWORK_INTERFACE_H */
