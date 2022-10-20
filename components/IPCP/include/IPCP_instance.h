#ifndef _COMPONENTS_IPCP_IPCP_INSTANCE_H
#define _COMPONENTS_IPCP_IPCP_INSTANCE_H

#include "common/rina_name.h"
#include "common/rina_ids.h"

#include "rina_common_port.h"
#include "RINA_API_flows.h"

#include "efcpStructures.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Forward definition of EFCP structures
 *
 * For some reasons I can't explain, this has to be excluded when a
 * C++ compiler is used (ie: for Arduino programs).
 */
#ifndef __cplusplus
struct cwq_t;
struct rtxqueue_t;
struct rtxq_t;
struct rttEntry_t;
struct rttq_t;
struct dtpSv_t;
struct dtcpConfig_t;
struct efcpContainer_t;
struct ipcpInstanceData_t;
#endif

typedef enum TYPE_IPCP_INSTANCE
{
    eShimWiFi = 0,
    eNormal

} ipcpInstanceType_t;

/*Structure of a IPCP instance. Could be type Normal or Shim*/
struct ipcpInstance_t
{
    ipcpInstanceId_t xId;
    ipcpInstanceType_t xType;
    struct ipcpInstanceData_t *pxData;
    struct ipcpInstanceOps_t *pxOps;
    RsListItem_t xInstanceItem;
};

/*
 * Contains all the information associated to an instance of a
 *  IPC Process
 */

/* Operations available in an IPCP*/
struct ipcpInstanceOps_t
{
    bool_t (*flowAllocateRequest)(struct ipcpInstance_t *pxIpcp,
                                  name_t *pxSource,
                                  name_t *pxDest,
                                  // struct flowSpec_t *    		pxFlowSpec,
                                  portId_t xId);
    bool_t (*flowAllocateResponse)(struct ipcpInstance_t *pxIpcp,
                                   portId_t xPortId
                                   /*int                         Result*/);
    bool_t (*flowDeallocate)(struct ipcpInstance_t *pxIpcp,
                             portId_t xId);

    bool_t (*applicationRegister)(struct ipcpInstance_t *pxIpcp,
                                  const name_t *pxSource,
                                  const name_t *pxDafName);

    bool_t (*applicationUnregister)(struct ipcpInstance_t *pxIpcp,
                                    const name_t *pxSource);

    bool_t (*assignToDif)(struct ipcpInstance_t *pxIpcp,
                          const name_t *pxDifName,
                          const string_t *type
                          /*dif_config * config*/);

    bool_t (*updateDifConfig)(struct ipcpInstance_t *pxIpcp
                              /*const struct dif_config *   configuration*/);

    /* Takes the ownership of the passed SDU */
    bool_t (*duWrite)(struct ipcpInstance_t *pxIpcp,
                      portId_t xId,
                      struct du_t *pxDu,
                      bool_t uxBlocking);

    cepId_t (*connectionCreate)(struct ipcpInstance_t *pxIpcp,
                                struct efcpContainer_t *pxEfcpc,
                                portId_t xPortId,
                                address_t xSource,
                                address_t xDest,
                                qosId_t xQosId,
                                dtpConfig_t *dtp_config,
                                struct dtcpConfig_t *dtcp_config);

    bool_t (*connectionUpdate)(struct ipcpInstance_t *pxIpcp,
                               portId_t xPortId,
                               cepId_t xSrcId,
                               cepId_t xDstId);

    bool_t (*connectionModify)(struct ipcpInstance_t *pxIpcp,
                               cepId_t xSrcCepId,
                               address_t xSrcAddress,
                               address_t xDstAddress);

    bool_t (*connectionDestroy)(struct ipcpInstance_t *pxIpcp,
                                cepId_t xSrcId);

    cepId_t (*connectionCreateArrived)(struct ipcpInstance_t *pxIpcp,
                                       portId_t xPortId,
                                       address_t xSource,
                                       address_t xDest,
                                       qosId_t xQosId,
                                       cepId_t xDstCepId
                                       /*struct dtp_config *         dtp_config,
                                       struct dtcp_config *        dtcp_config*/
    );

    bool_t (*flowPrebind)(struct ipcpInstance_t *pxIpcp,
                          flowAllocateHandle_t *pxFlowHandle);

    bool_t (*flowBindingIpcp)(struct ipcpInstance_t *pxIpcp,
                              portId_t xPortId,
                              struct ipcpInstance_t *pxN1Ipcp);

    bool_t (*flowUnbindingIpcp)(struct ipcpInstance_t *pxIpcp,
                                portId_t xPortId);

    bool_t (*flowUnbindingUserIpcp)(struct ipcpInstance_t *pxIpcp,
                                    portId_t xPortId);

    bool_t (*nm1FlowStateChange)(struct ipcpInstance_t *pxIpcp,
                                     portId_t xPortId, bool_t up);

    bool_t (*duEnqueue)(struct ipcpInstance_t *pxIpcp,
                        portId_t xId,
                        struct du_t *pxDu);

    /* Takes the ownership of the passed sdu */
    bool_t (*mgmtDuWrite)(struct ipcpInstance_t *pxIpcp,
                          portId_t xPortId,
                          struct du_t *pxDu);

    /* Takes the ownership of the passed sdu */
    bool_t (*mgmtDuPost)(struct ipcpInstance_t *pxIpcp,
                         portId_t xPortId,
                         struct du_t *xDu);

    bool_t (*pffAdd)(struct ipcpInstance_t *pxIpcp
                         /*struct mod_pff_entry	  * pxEntry*/);

    bool_t (*pffRemove)(struct ipcpInstance_t *pxIpcp
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

    const name_t *(*ipcpName)(struct ipcpInstance_t *pxIpcp);
    const name_t *(*difName)(struct ipcpInstance_t *pxIpcp);
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

    bool_t (*enableWrite)(struct ipcpInstance_t *pxData, portId_t xId);
    bool_t (*disableWrite)(struct ipcpInstance_t *pxIpcp, portId_t xId);

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

#ifdef __cplusplus
}
#endif

#endif // _COMPONENTS_IPCP_IPCP_INSTANCE_H
