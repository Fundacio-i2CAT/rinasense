#ifndef _mock_RIBD_RIBD_H
#define _mock_RIBD_RIBD_H

#include "rmt.h"
#include "IPCP_instance.h"
#include "rina_ids.h"

bool_t xRibdProcessLayerManagementPDU(struct ipcpInstanceData *pxData,
                                      portId_t xN1flowPortId,
                                      struct du_t *pxDu);

#endif // _mock_RIBD_RIBD_H
