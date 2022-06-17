#include "buffer.h"

static NetworkBufferDescriptor_t *lastBufferOutput = NULL;

bool_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                               bool_t xReleaseAfterSend)
{
    lastBufferOutput = pxNetworkBuffer;
}

/* Mock utilities */

NetworkBufferDescriptor_t *pxMockGetLastBufferOutput() {
    return lastBufferOutput;
}

void vMockClearLastBufferOutput() {
    lastBufferOutput = NULL;
}
