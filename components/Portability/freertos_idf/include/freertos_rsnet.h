#ifndef _PORT_IDF_RSNET_H
#define _PORT_IDF_RSNET_H

#include "freertos/FreeRTOS.h"
#include "lwip/inet.h"

#define RsNtoHS(x) ntohs(x)
#define PORT_HAS_NTOHS

#define RsNtoHL(x) ntohl(x)
#define PORT_HAS_NTOHL

#define RsHtoNL(x) htonl(x)
#define PORT_HAS_HTONL

#define RsHtoNS(x) htons(x)
#define PORT_HAS_HTONS

#endif // _PORT_IDF_RSNET_H
