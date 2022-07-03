#include "rina_buffers.h"

static NetworkBufferDescriptor_t *lastBufferOutput = NULL;

bool_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                               bool_t xReleaseAfterSend)
{
    lastBufferOutput = pxNetworkBuffer;
    return true;
}

/* Mock utilities */

NetworkBufferDescriptor_t *pxMockGetLastBufferOutput() {
    return lastBufferOutput;
}

void vMockClearLastBufferOutput() {
    lastBufferOutput = NULL;
}
