#ifndef _mock_NETWORKINTERFACE_H
#define _mock_NETWORKINTERFACE_H

/* PUBLIC API */

#define xNetworkInterfaceOutput mock_NetworkInterface_xNetworkInterfaceOutput

bool_t mock_NetworkInterface_xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                                                     bool_t xReleaseAfterSend);

#endif // _mock_NETWORKINTERFACE_H
