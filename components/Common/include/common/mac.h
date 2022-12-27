#ifndef _COMMON_MAC_H
#define _COMMON_MAC_H

#include <stdint.h>

#include "configSensor.h"
#include "portability/port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAC2STR_MIN_BUFSZ 18

#define ETH_P_RINA     0xD1F0
#define ETH_P_RINA_ARP 0x4305

#define MAC_ADDRESS_LENGTH_BYTES 6

//Structure MAC ADDRESS
typedef struct xMAC_ADDRESS
{
	uint8_t ucBytes[MAC_ADDRESS_LENGTH_BYTES]; /**< Byte array of the MAC address */
} MACAddress_t;

//enum MAC Address
typedef enum {
    MAC_ADDR_802_3
} eGHAType_t;

void mac2str(const MACAddress_t *, string_t, const size_t);

bool_t xIsBroadcastMac(const MACAddress_t *pxMac);

#ifdef __cplusplus
}
#endif

#endif // _COMMON_MAC_H
