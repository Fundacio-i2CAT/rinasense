#ifndef _PORT_IDF_RSNET_H
#define _PORT_IDF_RSNET_H

#include "freertos/FreeRTOS.h"

#define RsNtoHS FreeRTOS_ntohs
#define PORT_HAS_NTOHS

#define RsNtoHL FreeRTOS_ntohl
#define PORT_HAS_NTOHL

#define RsHtoNL FreeRTOS_htonl
#define PORT_HAS_HTONL

#define RsHtoNS FreeRTOS_htons
#define PORT_HAS_HTONS

#endif // _PORT_IDF_RSNET_H
