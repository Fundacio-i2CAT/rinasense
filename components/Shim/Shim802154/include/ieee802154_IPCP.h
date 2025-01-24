

#ifndef IEEE802154_IPCP_H__INCLUDED
#define IEEE802154_IPCP_H__INCLUDED

#include "common/rina_ids.h"
#include "common/rina_gpha.h"

// #include "Arp826.h"
// #include "Arp826_defs.h"
// #include "du.h"
#include "common/rina_common_port.h"
#include "shim_IPCP_events.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ADDR_MODE_NONE (0)     // PAN ID and address fields are not present
#define ADDR_MODE_RESERVED (1) // Reseved
#define ADDR_MODE_SHORT (2)    // Short address (16-bit)
#define ADDR_MODE_LONG (3)     // Extended address (64-bit)

    struct ipcpInstance_t *pxShim802154Create(ipcProcessId_t xIpcpId);
    bool_t xFlowAllocateResponse(struct ipcpInstanceData_t *pxShimInstanceData,
                                 portId_t xPortId);

    bool_t xShim802154EnrollToDIF(struct ipcpInstanceData_t *pxShimInstanceData);

#ifdef __cplusplus
}
#endif

#endif /* _COMPONENTS_SHIM_802154_INCLUDE_802154_IPCP_H*/
