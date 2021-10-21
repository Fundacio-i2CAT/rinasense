

#ifndef EFCP_H__INCLUDED
#define EFCP_H__INCLUDED

#include "RMT.h"
#include "pci.h"
#include "du.h"
#include "efcpStructures.h"


BaseType_t xEfcpContainerReceive(efcpContainer_t * pxContainer, cepId_t xCepId, du_t * pxDu);
BaseType_t xEfcpReceive(efcp_t * pxEfcp, du_t *  pxDu);



/*
 * EFCP
 */



#endif /* EFCP_H__*/


