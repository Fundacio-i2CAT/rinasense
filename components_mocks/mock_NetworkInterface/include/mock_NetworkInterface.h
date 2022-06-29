#ifndef _mock_NETWORKINTERFACE_H
#define _mock_NETWORKINTERFACE_H

#include "portability/port.h"
#include "rina_buffers.h"

/* Mock utilities */

NetworkBufferDescriptor_t *pxMockGetLastBufferOutput();

void vMockClearLastBufferOutput();

#endif // _mock_NETWORKINTERFACE_H
