#include <stdio.h>
#include <string.h>

#include "portability/port.h"

#include "configRINA.h"
#include "common/mac.h"

void mac2str(const MACAddress_t *pxMac, string_t psMac, const size_t p) {
    const uint8_t *m = pxMac->ucBytes;

    RsAssert(p >= MAC2STR_MIN_BUFSZ);
    snprintf(psMac, p, "%02x:%02x:%02x:%02x:%02x:%02x",
             m[0], m[1], m[2], m[3], m[4], m[5]);
}

bool_t xIsBroadcastMac(const MACAddress_t *pxMac)
{
    const MACAddress_t xBroadcastMac = {
        .ucBytes = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
    };

    return memcmp(pxMac, &xBroadcastMac, MAC_ADDRESS_LENGTH_BYTES) == 0;
}
