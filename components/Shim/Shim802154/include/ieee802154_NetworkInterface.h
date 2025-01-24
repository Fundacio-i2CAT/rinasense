#ifndef _COMPONENTS_SHIM_IEEE802154_INCLUDE_IEEE802154_NETWORK_INTERFACE_H
#define _COMPONENTS_SHIM_IEEE802154_INCLUDE_IEEE802154_NETWORK_INTERFACE_H

#include "portability/port.h"
#include "common/rina_gpha.h"

#include "configSensor.h"
#include "BufferManagement.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // #include "ShimIPCP.h"

    /* INTERNAL API FUNCTIONS. */
    bool_t xIeee802154NetworkInterfaceInitialise(MACAddress_t *phyDev);
    bool_t xIeee802154NetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                                             bool_t xReleaseAfterSend);

    bool_t xIeee802154NetworkInterfaceConnect(void);
    bool_t xIeee802154NetworkInterfaceDisconnect(void);
    bool_t xIeee802154NetworkInterfaceInput(void *buffer, uint16_t len, void *eb);

    
    /* The following functionS are defined only when BufferAllocation_1.c is linked in the project. 
    void vIeee802154NetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t pxNetworkBuffers[NUM_NETWORK_BUFFER_DESCRIPTORS]);
    bool_t xIeee802154GetPhyLinkStatus(void);
    */

#ifdef __cplusplus
}
#endif

#endif /* _NETWORK_INTERFACE_H */
