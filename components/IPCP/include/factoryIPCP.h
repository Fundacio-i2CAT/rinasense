#ifndef FACTORYIPCP_H_
#define FACTORYIPCP_H_

#include "IPCP.h"

typedef struct xIPCP_FACTORY{
    List_t  xIPCPInstancesList;
}ipcpFactory_t;

BaseType_t xFactoryIPCPInit(ipcpFactory_t * pxFactory);

ipcpInstance_t * xFactoryIPCPFindInstance(ipcpFactory_t  * pxFactory, ipcpInstanceType_t xType);

#endif
