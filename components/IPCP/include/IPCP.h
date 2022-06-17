#ifndef IPCP_H_
#define IPCP_H_

#include "configSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ARP826.h"
#include "pci.h"
#include "rina_common.h"
//#include "Rmt.h"

/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/

#define IS_PORT_ID_OK(id) (id >= 0 ? pdTRUE : pdFALSE)

typedef struct xQUEUE_FIFO
{
    QueueHandle_t xQueue;

} rfifo_t;


// typedef uint16_t ipcProcessId_t;

typedef uint16_t ipcpInstanceId_t;

typedef int32_t portId_t;

/*
 * Contains all the information associated to an instance of a
 *  IPC Process
 */

struct ipcpInstance_t;
struct ipcpInstanceData_t;
struct du_t;

/* Operations available in an IPCP*/
struct ipcpInstanceOps_t
{
    BaseType_t (*flowAllocateRequest)(struct ipcpInstanceData_t *pxData,
                                      name_t *pxSource,
                                      name_t *pxDest,
                                      // struct flowSpec_t *    		pxFlowSpec,
                                      portId_t xId);
    BaseType_t (*flowAllocateResponse)(struct ipcpInstanceData_t *pxData,
                                       portId_t xPortId
                                       /*int                         Result*/);
    BaseType_t (*flowDeallocate)(struct ipcpInstanceData_t *pxData,
                                 portId_t xId);

    BaseType_t (*applicationRegister)(struct ipcpInstanceData_t *pxData,
                                      name_t *pxSource,
                                      name_t *pxDafName);
    BaseType_t (*applicationUnregister)(struct ipcpInstanceData_t *pxData,
                                        const name_t *pxSource);

    BaseType_t (*assignToDif)(struct ipcpInstanceData_t *pxData,
                              const name_t *pxDifName,
                              const string_t *type
                              /*dif_config * config*/);

    BaseType_t (*updateDifConfig)(struct ipcpInstanceData_t *data
                                  /*const struct dif_config *   configuration*/);

    /* Takes the ownership of the passed SDU */
    BaseType_t (*duWrite)(struct ipcpInstanceData_t *pxData,
                          portId_t xId,
                          struct du_t *pxDu,
                          BaseType_t uxBlocking);

    cepId_t (*connectionCreate)(struct efcpContainer_t *pxEfcpc,
                                // struct ipcpInstance_t *      pxUserIpcp,
                                portId_t xPortId,
                                address_t xSource,
                                address_t xDest,
                                qosId_t xQosId,
                                dtpConfig_t *dtp_config,
                                struct dtcpConfig_t *dtcp_config);

    BaseType_t (*connectionUpdate)(struct ipcpInstanceData_t *pxData,
                                   portId_t xPortId,
                                   cepId_t xSrcId,
                                   cepId_t xDstId);

    BaseType_t (*connectionModify)(struct ipcpInstanceData_t *pxData,
                                   cepId_t xSrcCepId,
                                   address_t xSrcAddress,
                                   address_t xDstAddress);

    BaseType_t (*connectionDestroy)(struct ipcpInstanceData_t *pxData,
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

    BaseType_t (*flowPrebind)(struct ipcpInstanceData_t *pxData,
                              // struct ipcpInstance_t *   	pxUserIpcp,
                              portId_t xPortId);

    BaseType_t (*flowBindingIpcp)(struct ipcpInstanceData_t *pxUserData,
                                  portId_t xPortId,
                                  struct ipcpInstance_t *xN1Ipcp);

    BaseType_t (*flowUnbindingIpcp)(struct ipcpInstanceData_t *pxUserData,
                                    portId_t xPortId);
    BaseType_t (*flowUnbindingUserIpcp)(struct ipcpInstanceData_t *pxUserData,
                                        portId_t xPortId);
    BaseType_t (*nm1FlowStateChange)(struct ipcpInstanceData_t *pxData,
                                     portId_t xPortId, BaseType_t up);

    BaseType_t (*duEnqueue)(struct ipcpInstanceData_t *pxData,
                            portId_t xId,
                            struct du_t *pxDu);

    /* Takes the ownership of the passed sdu */
    BaseType_t (*mgmtDuWrite)(struct ipcpInstanceData_t *pxData,
                              portId_t xPortId,
                              struct du_t *pxDu);

    /* Takes the ownership of the passed sdu */
    BaseType_t (*mgmtDuPost)(struct ipcpInstanceData_t *pxData,
                             portId_t xPortId,
                             struct du_t *xDu);

    BaseType_t (*pffAdd)(struct ipcpInstanceData_t *pxData
                         /*struct mod_pff_entry	  * pxEntry*/);

    BaseType_t (*pffRemove)(struct ipcpInstanceData_t *pxData
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

    BaseType_t (*enableWrite)(struct ipcpInstanceData_t *pxData, portId_t xId);
    BaseType_t (*disableWrite)(struct ipcpInstanceData_t *pxData, portId_t xId);

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
    ListItem_t xInstanceItem;
} ipcpInstance_t;

/**
 * The software timer struct for various IPCP functions
 */
typedef struct xIPCP_TIMER
{
    uint32_t
        bActive : 1,            /**< This timer is running and must be processed. */
        bExpired : 1;           /**< Timer has expired and a task must be processed. */
    TimeOut_t xTimeOut;         /**< The timeout value. */
    TickType_t ulRemainingTime; /**< The amount of time remaining. */
    TickType_t ulReloadTime;    /**< The value of reload time. */
} IPCPTimer_t;

/*
 * Send the event eEvent to the IPCP task event queue, using a block time of
 * zero.  Return pdPASS if the message was sent successfully, otherwise return
 * pdFALSE.
 */
BaseType_t xSendEventToIPCPTask(eRINAEvent_t eEvent);

/* Returns pdTRUE is this function is called from the IPCP-task */
BaseType_t xIsCallingFromIPCPTask(void);

BaseType_t xSendEventStructToIPCPTask(const RINAStackEvent_t *pxEvent,
                                      TickType_t uxTimeout);

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer);

BaseType_t RINA_IPCPInit(void);
struct rmt_t *pxIPCPGetRmt(void);

#endif
