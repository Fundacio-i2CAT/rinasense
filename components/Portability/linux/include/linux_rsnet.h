#ifndef _PORTABILITY_LINUX_RSNET_H
#define _PORTABILITY_LINUX_RSNET_H

#include <arpa/inet.h>

#define RsNtoHS ntohs
#define PORT_HAS_NTOHS

#define RsNtoHL ntohl
#define PORT_HAS_NTOL

#define RsHtoNL htonl
#define PORT_HAS_HTONL

#define RsHtoNS htons
#define PORT_HAS_HTONS

#endif // _PORTABILITY_LINUX_RSNET_H
