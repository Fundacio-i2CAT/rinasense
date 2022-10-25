#ifndef _COMMON_MAC_H
#define _COMMON_MAC_H

#include <stdint.h>

#include "configSensor.h"
#include "portability/port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAC2STR_MIN_BUFSZ 18

//Structure MAC ADDRESS
typedef struct xMAC_ADDRESS
{
	uint8_t ucBytes[ MAC_ADDRESS_LENGTH_BYTES ]; /**< Byte array of the MAC address */
} MACAddress_t;

//enum MAC Address
typedef enum {
    MAC_ADDR_802_3
} eGHAType_t;

void mac2str(const MACAddress_t *, string_t, const size_t);

#ifdef __cplusplus
}
#endif

#endif // _COMMON_MAC_H
