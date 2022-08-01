#ifndef _COMPONENTS_IPCP_IPCP_NORMAL_H
#define _COMPONENTS_IPCP_IPCP_NORMAL_H

#include "rina_ids.h"
#include "rina_name.h"
#include "list.h"

typedef enum eNormal_Flow_State
{
    ePORT_STATE_NULL = 1,
    ePORT_STATE_PENDING,
    ePORT_STATE_ALLOCATED,
    ePORT_STATE_DEALLOCATED,
    ePORT_STATE_DISABLED
} eNormalFlowState_t;

struct ipcpInstanceData_t
{
    /* FIXME: add missing needed attributes */

    /* IPCP Instance's Name */
    name_t *pxName;

    /* IPCP Instance's DIF Name */
    name_t *pxDifName;

    /* IPCP Instance's List of Flows created */
    RsList_t xFlowsList;

    /*  FIXME: Remove it as soon as the kipcm_kfa gets removed*/
    // struct kfa *            kfa;

    /* Efcp Container asociated at the IPCP Instance */
    struct efcpContainer_t *pxEfcpc;

    /* RMT asociated at the IPCP Instance */
    struct rmt_t *pxRmt;

    /* SDUP asociated at the IPCP Instance */
    // struct sdup *           sdup;

    address_t xAddress;

    // ipcManager_t *pxIpcManager;
};

#endif // _
