#ifndef _mock_MOCK_NETWORKINTERFACE_H
#define _mock_MOCK_NETWORKINTERFACE_H

#include "portability/port.h"
#include "common/rina_buffers.h"

/* Mock utilities */

NetworkBufferDescriptor_t *pxMockGetLastBufferOutput();

void vMockClearLastBufferOutput();

#endif // _mock_MOCK_NETWORKINTERFACE_H
