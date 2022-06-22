#ifndef _COMMON_MAC_H
#define _COMMON_MAC_H

#include <stdint.h>

#include "configSensor.h"

//Structure MAC ADDRESS
typedef struct xMAC_ADDRESS
{
	uint8_t ucBytes[ MAC_ADDRESS_LENGTH_BYTES ]; /**< Byte array of the MAC address */
} MACAddress_t;

//enum MAC Address
typedef enum {
    MAC_ADDR_802_3
} eGHAType_t;

#endif // _COMMON_MAC_H
