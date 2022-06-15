#ifndef _PORT_GLIB_RSNET_H
#define _PORT_GLIB_RSNET_H

#include <glib.h>

#define RsNtoHS g_ntohs
#define PORT_HAS_NTOHS

#define RsNtoHL g_ntohl
#define PORT_HAS_NTOL

#define RsHtoNL g_htonl
#define PORT_HAS_HTONL

#define RsHtoNS g_htons
#define PORT_HAS_HTONS

#endif // _PORT_GLIB_RSNET_H
