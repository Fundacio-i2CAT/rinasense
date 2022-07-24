#include "rina_buffers.h"
#include "mac.h"

static NetworkBufferDescriptor_t *lastBufferOutput = NULL;

#ifdef ESP_PLATFORM
bool_t mock_NetworkInterface_xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                                                     bool_t xReleaseAfterSend)
#else
bool_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                               bool_t xReleaseAfterSend)
#endif
{
    RsAssert(pxNetworkBuffer != NULL);
    lastBufferOutput = pxNetworkBuffer;
    LOGI(TAG_ARP, "inside xNetworkInterfaceOutput: %p", lastBufferOutput);
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_NetworkInterface_xNetworkInterfaceInitialise(const MACAddress_t *phyDev)
#else
bool_t xNetworkInterfaceInitialise(const MACAddress_t *phyDev)
#endif
{
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_NetworkInterface_xNetworkInterfaceConnect(void)
#else
bool_t xNetworkInterfaceConnect(void)
#endif
{
    return true;
}

/* Mock utilities */

NetworkBufferDescriptor_t *pxMockGetLastBufferOutput() {
    return lastBufferOutput;
}

void vMockClearLastBufferOutput() {
    lastBufferOutput = NULL;
}
