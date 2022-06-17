#ifndef _mock_NETWORKINTERFACE_H
#define _mock_NETWORKINTERFACE_H

#include "portability/port.h"
#include "buffer.h"

/* PUBLIC API */

bool_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                               bool_t xReleaseAfterSend);

/* Mock utilities */

NetworkBufferDescriptor_t *pxMockGetLastBufferOutput();

void vMockClearLastBufferOutput();

#endif // _mock_NETWORKINTERFACE_H
