#ifndef _COMPONENTS_IPCP_IPCP_NORMAL_H
#define _COMPONENTS_IPCP_IPCP_NORMAL_H

#include "common/rina_ids.h"
#include "common/rina_name.h"
#include "common/list.h"

#include "FlowAllocator_api.h"
#include "FlowAllocator_defs.h"
#include "Ribd_api.h"
#include "Enrollment_api.h"
#include "efcpStructures.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum eNormal_Flow_State
{
    ePORT_STATE_NULL = 1,
    ePORT_STATE_PENDING,
    ePORT_STATE_ALLOCATED,
    ePORT_STATE_DEALLOCATED,
    ePORT_STATE_DISABLED
} eNormalFlowState_t;

#ifndef IPCP_INSTANCE_DATA_TYPE
#define IPCP_INSTANCE_DATA_TYPE "normal"

/**
 * This is the instance data for the NORMAL IPC.
 */
struct ipcpInstanceData_t
{
#ifndef NDEBUG
    /* Used to assert on the type of instance data we're address is
     * correct. */
    uint8_t unInstanceDataType;
#endif

    /* Pool */
    rsrcPoolP_t xPciPool;

    /* IPCP instance interface. */
    struct ipcpInstance_t *pxIpcp;

    /* IPCP Instance's Name */
    rname_t xName;

    /* IPCP Instance's DIF Name */
    rname_t xDifName;

    /* IPCP Instance's List of Flows created */
    RsList_t xFlowsList;

    /*  FIXME: Remove it as soon as the kipcm_kfa gets removed*/
    // struct kfa *            kfa;

    /* Efcp Container asociated at the IPCP Instance */
    struct efcpContainer_t xEfcpContainer;

    /* RMT asociated at the IPCP Instance */
    struct rmt_t xRmt;

    /* Enrollment component */
    Enrollment_t xEnrollment;

    /* Flow allocator associated to the IPCP instance. */
    flowAllocator_t xFA;

    /* RIB object */
    Ribd_t xRibd;

    /* SDUP asociated at the IPCP Instance */
    // struct sdup *           sdup;

    address_t xAddress;

    // ipcManager_t *pxIpcManager;
};

#else
#error An #include mishap happened: IPCP_INSTANCE_DATA_TYPE is already defined!
#endif

#ifdef __cplusplus
}
#endif

#endif // _
