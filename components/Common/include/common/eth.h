#ifndef _COMMON_ETH_H_INCLUDED
#define _COMMON_ETH_H_INCLUDED

#include "mac.h"

#ifdef __cplusplus
extern "C" {
#endif

// Structure Ethernet Header
typedef struct __attribute__((packed))
{
	MACAddress_t xDestinationAddress; /**< Destination address  0 + 6 = 6  */
	MACAddress_t xSourceAddress;	  /**< Source address       6 + 6 = 12 */
	uint16_t usFrameType;			  /**< The EtherType field 12 + 2 = 14 */
} EthernetHeader_t;

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_ETH_H_INCLUDED */

