#ifndef _COMPONENTS_IPCP_IPCP_INSTANCE_H
#define _COMPONENTS_IPCP_IPCP_INSTANCE_H

#include "rina_name.h"
#include "rina_ids.h"
#include "rina_common_port.h"

/* Forward definition of EFCP structures. */
struct cwq_t;
struct rtxqueue_t;
struct rtxq_t;
struct rttEntry_t;
struct rttq_t;
struct dtpSv_t;
struct dtcpConfig_t;
struct efcpContainer_t;

struct ipcpInstanceData_t;

typedef enum TYPE_IPCP_INSTANCE
{
    eShimWiFi = 0,
    eNormal

} ipcpInstanceType_t;

/*Structure of a IPCP instance. Could be type Normal or Shim*/
typedef struct xIPCP_INSTANCE
{
    ipcpInstanceId_t xId;
    ipcpInstanceType_t xType;
    struct ipcpInstanceData_t *pxData;
    struct ipcpInstanceOps_t *pxOps;
    RsListItem_t xInstanceItem;
} ipcpInstance_t;

/*
 * Contains all the information associated to an instance of a
 *  IPC Process
 */

/* Operations available in an IPCP*/
struct ipcpInstanceOps_t
{
    bool_t (*flowAllocateRequest)(struct ipcpInstanceData_t *pxData,
                                      name_t *pxSource,
                                      name_t *pxDest,
                                      // struct flowSpec_t *    		pxFlowSpec,
                                      portId_t xId);
    bool_t (*flowAllocateResponse)(struct ipcpInstanceData_t *pxData,
                                       portId_t xPortId
                                       /*int                         Result*/);
    bool_t (*flowDeallocate)(struct ipcpInstanceData_t *pxData,
                                 portId_t xId);

    bool_t (*applicationRegister)(struct ipcpInstanceData_t *pxData,
                                      name_t *pxSource,
                                      name_t *pxDafName);
    bool_t (*applicationUnregister)(struct ipcpInstanceData_t *pxData,
                                        const name_t *pxSource);

    bool_t (*assignToDif)(struct ipcpInstanceData_t *pxData,
                              const name_t *pxDifName,
                              const string_t *type
                              /*dif_config * config*/);

    bool_t (*updateDifConfig)(struct ipcpInstanceData_t *data
                                  /*const struct dif_config *   configuration*/);

    /* Takes the ownership of the passed SDU */
    bool_t (*duWrite)(struct ipcpInstanceData_t *pxData,
                          portId_t xId,
                          struct du_t *pxDu,
                          bool_t uxBlocking);

    cepId_t (*connectionCreate)(struct efcpContainer_t *pxEfcpc,
                                // struct ipcpInstance_t *      pxUserIpcp,
                                portId_t xPortId,
                                address_t xSource,
                                address_t xDest,
                                qosId_t xQosId,
                                dtpConfig_t *dtp_config,
                                struct dtcpConfig_t *dtcp_config);

    bool_t (*connectionUpdate)(struct ipcpInstanceData_t *pxData,
                                   portId_t xPortId,
                                   cepId_t xSrcId,
                                   cepId_t xDstId);

    bool_t (*connectionModify)(struct ipcpInstanceData_t *pxData,
                                   cepId_t xSrcCepId,
                                   address_t xSrcAddress,
                                   address_t xDstAddress);

    bool_t (*connectionDestroy)(struct ipcpInstanceData_t *pxData,
                                    cepId_t xSrcId);

    cepId_t (*connectionCreateArrived)(struct ipcpInstanceData_t *pxData,
                                       struct ipcpInstance_t *pxUserIpcp,
                                       portId_t xPortId,
                                       address_t xSource,
                                       address_t xDest,
                                       qosId_t xQosId,
                                       cepId_t xDstCepId
                                       /*struct dtp_config *         dtp_config,
                                       struct dtcp_config *        dtcp_config*/
    );

    bool_t (*flowPrebind)(struct ipcpInstanceData_t *pxData,
                              // struct ipcpInstance_t *   	pxUserIpcp,
                              portId_t xPortId);

    bool_t (*flowBindingIpcp)(struct ipcpInstanceData_t *pxUserData,
                                  portId_t xPortId,
                                  struct ipcpInstance_t *xN1Ipcp);

    bool_t (*flowUnbindingIpcp)(struct ipcpInstanceData_t *pxUserData,
                                    portId_t xPortId);
    bool_t (*flowUnbindingUserIpcp)(struct ipcpInstanceData_t *pxUserData,
                                        portId_t xPortId);
    bool_t (*nm1FlowStateChange)(struct ipcpInstanceData_t *pxData,
                                     portId_t xPortId, bool_t up);

    bool_t (*duEnqueue)(struct ipcpInstanceData_t *pxData,
                            portId_t xId,
                            struct du_t *pxDu);

    /* Takes the ownership of the passed sdu */
    bool_t (*mgmtDuWrite)(struct ipcpInstanceData_t *pxData,
                              portId_t xPortId,
                              struct du_t *pxDu);

    /* Takes the ownership of the passed sdu */
    bool_t (*mgmtDuPost)(struct ipcpInstanceData_t *pxData,
                             portId_t xPortId,
                             struct du_t *xDu);

    bool_t (*pffAdd)(struct ipcpInstanceData_t *pxData
                         /*struct mod_pff_entry	  * pxEntry*/);

    bool_t (*pffRemove)(struct ipcpInstanceData_t *pxData
                            /*struct mod_pff_entry	  * pxEntry*/);

    /*int (* pff_dump)(struct ipcp_instance_data * data,
                     struct list_head *          entries);

    int (* pff_flush)(struct ipcp_instance_data * data);

    int (* pff_modify)(struct ipcp_instance_data * data,
                       struct list_head * entries);

    int (* query_rib)(struct ipcp_instance_data * data,
                      struct list_head *          entries,
                      const string_t *            object_class,
                      const string_t *            object_name,
                      uint64_t                    object_instance,
                      uint32_t                    scope,
                      const string_t *            filter);*/

    const name_t *(*ipcpName)(struct ipcpInstanceData_t *pxData);
    const name_t *(*difName)(struct ipcpInstanceData_t *pxData);
    /*ipc_process_id_t (* ipcp_id)(ipcpInstanceData_t * pxData);

    int (* set_policy_set_param)(struct ipcp_instance_data * data,
                                 const string_t * path,
                                 const string_t * param_name,
                                 const string_t * param_value);
    int (* select_policy_set)(struct ipcp_instance_data * data,
                              const string_t * path,
                              const string_t * ps_name);

    int (* update_crypto_state)(struct ipcp_instance_data * data,
                    struct sdup_crypto_state * state,
                        port_id_t 	   port_id);*/

    bool_t (*enableWrite)(struct ipcpInstanceData_t *pxData, portId_t xId);
    bool_t (*disableWrite)(struct ipcpInstanceData_t *pxData, portId_t xId);

    /*
     * Start using new address after first timeout, deprecate old
     * address after second timeout
     */
    /*
    int (* address_change)(struct ipcp_instance_data * data,
                   address_t new_address,
               address_t old_address,
               timeout_t use_new_address_t,
               timeout_t deprecate_old_address_t);*/

    /*
     * The maximum size of SDUs that this IPCP will accept
     */
    size_t (*maxSduSize)(struct ipcpInstanceData_t *pxData);
};

#endif // _COMPONENTS_IPCP_IPCP_INSTANCE_H
