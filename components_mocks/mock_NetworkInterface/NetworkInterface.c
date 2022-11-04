#include "common/mac.h"

#include "rina_buffers.h"
#include "IPCP_instance.h"

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
bool_t mock_NetworkInterface_xNetworkInterfaceInitialise(struct ipcpInstance_t *pxSelf, const MACAddress_t *phyDev)
#else
    bool_t xNetworkInterfaceInitialise(struct ipcpInstance_t *pxSelf, MACAddress_t *phyDev)
#endif
{
    LOGI(TAG_WIFI, "Mock-NetworkInterface initialized");
    return true;
}

#ifdef ESP_PLATFORM
bool_t mock_NetworkInterface_xNetworkInterfaceConnect(void)
#else
bool_t xNetworkInterfaceConnect(void)
#endif
{
    LOGI(TAG_WIFI, "Mock-NetworkInterface connecting");
    return true;
}

/* Mock utilities */

NetworkBufferDescriptor_t *pxMockGetLastBufferOutput() {
    return lastBufferOutput;
}

void vMockClearLastBufferOutput() {
    lastBufferOutput = NULL;
}
