#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "IPCP.h"
#include "EFCP.h"
#include "Rmt.h"
#include "common.h"
#include "du.h"
#include "factoryIPCP.h"
#include "configRINA.h"
#include "Ribd.h"
#include "rstr.h"
#include "FlowAllocator.h"
#include "BufferManagement.h"

#include "esp_log.h"

typedef enum eNormal_Flow_State
{
        ePORT_STATE_NULL = 1,
        ePORT_STATE_PENDING,
        ePORT_STATE_ALLOCATED,
        ePORT_STATE_DEALLOCATED,
        ePORT_STATE_DISABLED
} eNormalFlowState_t;

struct cepIdsEntry_t
{
        cepId_t xCepId;
        ListItem_t CepIdListItem;
};

struct normalFlow_t
{
        portId_t xPortId;
        cepId_t xActive;
        List_t xCepIdsList;
        eNormalFlowState_t eState;
        ipcpInstance_t *pxUserIpcp;
        ListItem_t xFlowListItem;
};

struct ipcpInstanceData_t
{
        /* FIXME: add missing needed attributes */
        /* Unique IPCP Instance Identifier (uint16_t) */
        ipcProcessId_t xId;

        /* IPCP Instance's Name */
        name_t *pxName;

        /* IPCP Instance's DIF Name */
        name_t *pxDifName;

        /* IPCP Instance's List of Flows created */
        List_t xFlowsList;

        /*  Flow Allocator */
        flowAllocator_t *pxFlowAllocator;

        /* Efcp Container asociated at the IPCP Instance */
        struct efcpContainer_t *pxEfcpc;

        /* RMT asociated at the IPCP Instance */
        struct rmt_t *pxRmt;

        /* SDUP asociated at the IPCP Instance */
        // struct sdup *           sdup;

        address_t xAddress;
        address_t xOldAddress;

        /// spinlock_t              lock;

        /* Instance List Item to be add into the Factory List*/
        ListItem_t xInstanceListItem;

        /* Timers required for the address change procedure
        struct {
                struct timer_list use_naddress;
            struct timer_list kill_oaddress;
        } timers;*/
};

struct ipcpFactoryData_t
{
        List_t xInstancesNormalList;
};

static struct ipcpFactoryData_t xFactoryNormalData;

static BaseType_t pvNormalAssignToDif(struct ipcpInstanceData_t *pxData, name_t *pxDifName);

static BaseType_t xNormalInit(struct ipcpFactoryData_t *pxData)
{
        // memset(&xNormalData, 0, sizeof(xNormalData));
        vListInitialise(&pxData->xInstancesNormalList);
        return pdTRUE;
}

static ipcpInstance_t *prvNormalIPCPFindInstance(struct ipcpFactoryData_t *pxFactoryData,
                                                 ipcpInstanceType_t xType);

static ipcpInstance_t *prvNormalIPCPFindInstance(struct ipcpFactoryData_t *pxFactoryData,
                                                 ipcpInstanceType_t xType)
{

        ipcpInstance_t *pxInstance;

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        /* Find a way to iterate in the list and compare the addesss*/

        pxListEnd = listGET_END_MARKER(&pxFactoryData->xInstancesNormalList);
        pxListItem = listGET_HEAD_ENTRY(&pxFactoryData->xInstancesNormalList);

        while (pxListItem != pxListEnd)
        {

                pxInstance = (ipcpInstance_t *)listGET_LIST_ITEM_OWNER(pxListItem);

                if (pxInstance)
                {

                        if (pxInstance->xType == xType)
                        {
                                // ESP_LOGI(TAG_IPCPMANAGER, "Instance founded %p, Type: %d", pxInstance, pxInstance->xType);
                                return pxInstance;
                        }
                }
                else
                {
                        return NULL;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        return NULL;
}

static struct normalFlow_t *prvNormalFindFlow(struct ipcpInstanceData_t *pxData,
                                              portId_t xPortId)
{

        struct normalFlow_t *pxFlow;
        // shimFlow_t *pxFlowNext;

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        /* Find a way to iterate in the list and compare the addesss*/
        pxListEnd = listGET_END_MARKER(&pxData->xFlowsList);
        pxListItem = listGET_HEAD_ENTRY(&pxData->xFlowsList);

        while (pxListItem != pxListEnd)
        {

                pxFlow = (struct normalFlow_t *)listGET_LIST_ITEM_OWNER(pxListItem);

                if (pxFlow && pxFlow->xPortId == xPortId)
                {

                        // ESP_LOGI(TAG_IPCPNORMAL, "Flow founded %p, portID: %d, portState:%d", pxFlow, pxFlow->xPortId, pxFlow->eState);
                        return pxFlow;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        ESP_LOGI(TAG_IPCPNORMAL, "Flow not founded");
        return NULL;
}

BaseType_t xNormalDuWrite(struct ipcpInstanceData_t *pxData,
                          portId_t xId,
                          struct du_t *pxDu)
{
        ESP_LOGI(TAG_IPCPNORMAL, "xNormalDuWrite");

        struct normalFlow_t *pxFlow;

        pxFlow = prvNormalFindFlow(pxData, xId);
        if (!pxFlow || pxFlow->eState != ePORT_STATE_ALLOCATED)
        {

                ESP_LOGE(TAG_IPCPNORMAL, "Write: There is no flow bound to this port_id: %d",
                         xId);
                xDuDestroy(pxDu);
                return pdFALSE;
        }

        if (xEfcpContainerWrite(pxData->pxEfcpc, pxFlow->xActive, pxDu))
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Could not send sdu to EFCP Container");
                return pdFALSE;
        }

        return pdTRUE;
}

/*static BaseType_t xNormalDeallocate(struct ipcpInstanceData * pxData)
{
        struct normalFlow_t * pxFlow, * pxNextFlow;

        list_for_each_entry_safe(flow, next, &(data->flows), list) {
                if (remove_all_cepid(data, flow))
                        LOG_ERR("Some efcp structures could not be destroyed"
                                "in flow %d", flow->port_id);

                list_del(&flow->list);
                rkfree(flow);
        }

        return pdTRUE;
}*/

BaseType_t xNormalFlowPrebind(struct ipcpInstanceData_t *pxData,
                              portId_t xPortId)
{

        struct normalFlow_t *pxFlow;

        if (!pxData)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Wrong input parameters...");
                return pdFALSE;
        }

        pxFlow = pvPortMalloc(sizeof(*pxFlow));
        if (!pxFlow)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Could not create a flow in normal-ipcp to pre-bind");
                return pdFALSE;
        }

        pxFlow->xPortId = xPortId;
        pxFlow->eState = ePORT_STATE_PENDING;
        /*KFA should be the user. Implement this when the KFA is implemented*/
        // pxFlow->pxUserIpcp = kfa;

        // ESP_LOGI(TAG_IPCPNORMAL, "Flow: %p portID: %d portState: %d", pxFlow, pxFlow->xPortId, pxFlow->eState);
        vListInitialiseItem(&(pxFlow->xFlowListItem));
        listSET_LIST_ITEM_OWNER(&(pxFlow->xFlowListItem), (void *)pxFlow);
        vListInsert(&(pxData->xFlowsList), &(pxFlow->xFlowListItem));

        return pdTRUE;
}

cepId_t xNormalConnectionCreateRequest(struct ipcpInstanceData_t *pxData,
                                       portId_t xPortId,
                                       address_t xSource,
                                       address_t xDest,
                                       qosId_t xQosId,
                                       dtpConfig_t *pxDtpCfg,
                                       struct dtcpConfig_t *pxDtcpCfg)
{
        cepId_t xCepId;
        struct normalFlow_t *pxFlow;
        struct cepIdsEntry_t *pxCepEntry;
        ipcpInstance_t *pxIpcp;

        xCepId = xEfcpConnectionCreate(pxData->pxEfcpc, xSource, xDest,
                                       xPortId, xQosId,
                                       cep_id_bad(), cep_id_bad(),
                                       pxDtpCfg, pxDtcpCfg);
        if (!is_cep_id_ok(xCepId))
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Failed EFCP connection creation");
                return cep_id_bad();
        }

        pxCepEntry = pvPortMalloc(sizeof(*pxCepEntry)); // Error

        if (!pxCepEntry)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Could not create a cep_id entry, bailing out");
                xEfcpConnectionDestroy(pxData->pxEfcpc, xCepId);
                return cep_id_bad();
        }

        // vListInitialise(&pxCepEntry->CepIdListItem);
        pxCepEntry->xCepId = xCepId;

        /*/ipcp = kipcm_find_ipcp(default_kipcm, data->id);
        if (!ipcp) {
                ESP_LOGE(TAG_IPCPNORMALNORMAL,"KIPCM could not retrieve this IPCP");
                xEfcpConnectionDestroy(pxData->efcpc, cep_id);
                return cep_id_bad();
        }*/

        // configASSERT(xUserIpcp->xOps);
        // configASSERT(xUserIpcp->xOps->flow_binding_ipcp);
        // spin_lock_bh(&data->lock);
        pxFlow = prvNormalFindFlow(pxData, xPortId);

        /*
                if (!pxFlow) {
                        //spin_unlock_bh(&data->lock);
                        ESP_LOGE(TAG_IPCPNORMAL,"Could not retrieve normal flow to create connection");
                        xEfcpConnectionDestroy(pxData->efcpc, xCepId);
                        return cep_id_bad();
                }

                if (user_ipcp->ops->flow_binding_ipcp(user_ipcp->data,
                                                      port_id,
                                                      ipcp)) {
                        spin_unlock_bh(&data->lock);
                        ESP_LOGE(TAG_IPCPNORMAL,"Could not bind flow with user_ipcp");
                        efcp_connection_destroy(data->efcpc, cep_id);
                        return cep_id_bad();
                }

                list_add(&cep_entry->list, &flow->cep_ids_list);*/

        pxFlow->xActive = xCepId;
        // pxFlow->eState = ePORTSTATEPENDING;

        // spin_unlock_bh(&data->lock);

        return xCepId;
}

/**
 * @brief Flow binding the N-1 Instance and the Normal IPCP by the
 * portId (N-1 port Id).
 *
 * @param pxUserData Normal IPCP in this case
 * @param xPid The PortId of the N-1 DIF
 * @param pxN1Ipcp Ipcp Instance N-1 DIF
 * @return BaseType_t
 */
BaseType_t xNormalFlowBinding(struct ipcpInstanceData_t *pxUserData,
                              portId_t xPid,
                              ipcpInstance_t *pxN1Ipcp)
{
        return xRmtN1PortBind(pxUserData->pxRmt, xPid, pxN1Ipcp);
}

BaseType_t xNormalTest(ipcpInstance_t *pxNormalInstance, ipcpInstance_t *pxN1Ipcp)
{
        ESP_LOGI(TAG_IPCPNORMAL, "Test");

        portId_t xId = 1;

        /* Data User */
        struct du_t *testDu;
        NetworkBufferDescriptor_t *pxNetworkBuffer;
        size_t xBufferSize;

        /* String to send*/
        char *ucStringTest = "Temperature:22";

        /*Getting the buffer Descriptor*/
        xBufferSize = strlen(ucStringTest);
        pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(xBufferSize, (TickType_t)0U); // sizeof length DataUser packet.

        ESP_LOGI(TAG_IPCPNORMAL, "BufferSize DU:%d", xBufferSize);

        /*Copy the string to the Buffer Network*/
        memcpy(pxNetworkBuffer->pucEthernetBuffer, ucStringTest, xBufferSize);

        pxNetworkBuffer->xDataLength = xBufferSize;

        // ESP_LOGI(TAG_IPCPNORMALNORMAL, "Size of NetworkBuffer: %d",pxNetworkBuffer->xDataLength);
        /*Integrate the buffer to the Du structure*/
        testDu = pvPortMalloc(sizeof(*testDu));
        testDu->pxNetworkBuffer = pxNetworkBuffer;
        ESP_LOGI(TAG_IPCPNORMAL, "Du Filled");

        ESP_LOGI(TAG_IPCPNORMAL, "Normal Instance: %p", pxNormalInstance);

        if (xNormalFlowBinding(pxNormalInstance->pxData, xId, pxN1Ipcp))
        {
                ESP_LOGI(TAG_IPCPNORMAL, "FlowBinding");
        }

        /*Call to Normalwrite function to send data*/
        if (xNormalDuWrite(pxNormalInstance->pxData, xId, testDu))
        {
                ESP_LOGI(TAG_IPCPNORMAL, "Wrote packet on the shimWiFi");
                return pdTRUE;
        }

        return pdFALSE;
}

static BaseType_t pvNormalAssignToDif(struct ipcpInstanceData_t *pxData, name_t *pxDifName)
{
        efcpConfig_t *pxEfcpConfig;
        // struct secman_config * sm_config;
        // rmtConfig_t *pxRmtConfig;

        if (!pxDifName)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Failed creation of dif name");

                return pdFALSE;
        }

        if (!xRstringDup(pxDifName->pcProcessName, &pxData->pxDifName->pcProcessName) ||
            !xRstringDup(pxDifName->pcProcessInstance, &pxData->pxDifName->pcProcessInstance) ||
            !xRstringDup(pxDifName->pcEntityName, &pxData->pxDifName->pcEntityName) ||
            !xRstringDup(pxDifName->pcEntityInstance, &pxData->pxDifName->pcEntityInstance))
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Name was not created properly");
        }

        /*Reading from the RINACONFIG.h*/
        pxData->xAddress = LOCAL_ADDRESS;

        /* FUTURE IMPLEMENTATIONS

        **** SHOULD READ CONFIGS FROM FILE RINACONFIG.H
        pxEfcpConfig =  pxConfig->pxEfcpconfig;
        pxConfig->pxEfcpconfig = 0;

        if (! pxEfcpConfig)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "No EFCP configuration in the dif_info");
                return pdFALSE;
        }

        if (!pxEfcpConfig->pxDtCons)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Configuration constants for the DIF are bogus...");
                efcp_config_free(efcp_config);
                return pdFALSE;
        }

        efcp_container_config_set(pxData->pxEfcpc, pxEfcpConfig);


        pxRmtConfig = pxConfig->pxRmtConfig;
        pxConfig->pxRmtConfig = 0;

        if (!pxRmtConfig)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "No RMT configuration in the dif_info");
                return pdFALSE;
        }*/

        if (!xRmtAddressAdd(pxData->pxRmt, pxData->xAddress))
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Could not set local Address to RMT");
                return pdFALSE;
        }
        /*
                if (rmt_config_set(data->rmt, rmt_config))
                {
                        ESP_LOGE(TAG_IPCPNORMAL, "Could not set RMT conf");
                        return pdFALSE;
                }

                sm_config = config->secman_config;
                config->secman_config = 0;
                if (!sm_config)
                {
                        LOG_INFO("No SDU protection config specified, using default");
                        sm_config = secman_config_create();
                        sm_config->default_profile = auth_sdup_profile_create();
                }
                if (sdup_config_set(data->sdup, sm_config))
                {
                        ESP_LOGE(TAG_IPCPNORMAL, "Could not set SDUP conf");
                        return -1;
                }
                if (sdup_dt_cons_set(data->sdup, dt_cons_dup(efcp_config->dt_cons)))
                {
                        ESP_LOGE(TAG_IPCPNORMAL, "Could not set dt_cons in SDUP");
                        return -1;
                }*/

        return pdTRUE;
}

static BaseType_t xNormalDuEnqueue(struct ipcpInstanceData_t *pxData,
                                   portId_t xN1PortId,
                                   struct du_t *pxDu)
{
        if (!xRmtReceive(pxData->pxRmt, pxDu, xN1PortId))
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Could not enqueue SDU into the RMT");
                return pdFALSE;
        }

        return pdTRUE;
}

/**
 * @brief Write the DU into the IPCP instance
 *
 * @param pxData Ipcp Instance Data (Normal Instance)
 * @param xPortId Port Id N-1 of the flow. Enrollment task know the Port Id when it request a flow to the
 * N-1 DIF.
 * @param pxDu Data Unit to be write into the IPCP instance.
 * @return BaseType_t
 */
BaseType_t xNormalMgmtDuWrite(struct ipcpInstanceData_t *pxData, portId_t xPortId, struct du_t *pxDu)
{
        ssize_t sbytes;

        ESP_LOGI(TAG_IPCPNORMAL, "Passing SDU to be written to N-1 port %d "
                                 "from IPC Process %d",
                 xPortId, pxData->xId);

        if (!pxDu)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "No data passed, bailing out");
                return pdFALSE;
        }

        pxDu->pxCfg = pxData->pxEfcpc->pxConfig;
        sbytes = xDuLen(pxDu);

        if (!xDuEncap(pxDu, PDU_TYPE_MGMT))
        {
                ESP_LOGE(TAG_IPCPNORMAL, "No data passed, bailing out");
                xDuDestroy(pxDu);
                return pdFALSE;
        }

        /*Fill the PCI*/
        pxDu->pxPci->ucVersion = 0X01;
        pxDu->pxPci->connectionId_t.xDestination = 0;
        pxDu->pxPci->connectionId_t.xQosId = 1;
        pxDu->pxPci->connectionId_t.xSource = 0;
        pxDu->pxPci->xDestination = 0;
        pxDu->pxPci->xFlags = 0;
        pxDu->pxPci->xType = PDU_TYPE_MGMT;
        pxDu->pxPci->xSequenceNumber = 0;
        pxDu->pxPci->xPduLen = pxDu->pxNetworkBuffer->xDataLength;
        pxDu->pxPci->xSource = pxData->xAddress;

        // vPciPrint(pxDu->pxPci);

        if (xPortId)
        {
                if (!xRmtSendPortId(pxData->pxRmt, xPortId, pxDu))
                {
                        ESP_LOGE(TAG_IPCPNORMAL, "Could not sent to RMT");
                        return pdFALSE;
                }
        }
        else
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Could not sent to RMT: no portID");
                xDuDestroy(pxDu);
                return pdFALSE;
        }

        return pdTRUE;
}

static BaseType_t xNormalMgmtDuPost(struct ipcpInstanceData *pxData,
                                    portId_t xPortId,
                                    struct du *pxDu)
{

        if (!pxData)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Bogus instance passed");
                xDuDestroy(pxDu);
                return pdFALSE;
        }

        if (!is_port_id_ok(xPortId))
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Wrong port id");
                xDuDestroy(pxDu);
                return pdFALSE;
        }
        /*if (!IsDuOk(pxDu)) {
                ESP_LOGE(TAG_IPCPNORMAL,"Bogus management SDU");
                xDuDestroy(pxDu);
                return pdFALSE;
        }*/

        /*Send to the RIB Daemon*/
        if (!xRibdProcessLayerManagementPDU(pxData, xPortId, pxDu))
        {
                ESP_LOGI(TAG_IPCPNORMAL, "Was not possible to process el Management PDU");
                return pdFALSE;
        }

        return pdTRUE;
}

static struct ipcpInstanceOps_t xNormalInstanceOps = {
    .flowAllocateRequest = NULL,           // ok
    .flowAllocateResponse = NULL,          // ok
    .flowDeallocate = NULL,                // ok
    .flowPrebind = xNormalFlowPrebind,     // ok
    .flowBindingIpcp = xNormalFlowBinding, // ok
    .flowUnbindingIpcp = NULL,             // ok
    .flowUnbindingUserIpcp = NULL,         // ok
    .nm1FlowStateChange = NULL,            // ok

    .applicationRegister = NULL,   // ok
    .applicationUnregister = NULL, // ok

    .assignToDif = NULL,     // ok
    .updateDifConfig = NULL, // ok

    .connectionCreate = xNormalConnectionCreateRequest, // ok
    .connectionUpdate = NULL,                           // ok
    .connectionDestroy = NULL,                          // ok
    .connectionCreateArrived = NULL,                    // ok
    .connectionModify = NULL,                           // ok

    .duEnqueue = xNormalDuEnqueue, // ok
    .duWrite = xNormalDuWrite,     // ok

    .mgmtDuWrite = xNormalMgmtDuWrite, // ok
    .mgmtDuPost = xNormalMgmtDuPost,   // ok

    .pffAdd = NULL,    // ok
    .pffRemove = NULL, // ok
    //.pff_dump                  = NULL,
    //.pff_flush                 = NULL,
    //.pff_modify		   		   = NULL,

    //.query_rib		  		   = NULL,

    .ipcpName = NULL, // ok
    .difName = NULL,  // ok
    //.ipcp_id		  		   = NULL,

    //.set_policy_set_param      = NULL,
    //.select_policy_set         = NULL,
    //.update_crypto_state	   = NULL,
    //.address_change            = NULL,
    //.dif_name		   		   = NULL,
    .maxSduSize = NULL};

ipcpInstance_t *pxNormalCreate(struct ipcpFactoryData_t *pxData, ipcProcessId_t xIpcpId)
{

        //---------------------
        name_t *pxName; /*Name should be send by the IPCPTASK?*/
        name_t *pxDifName;

        ipcpInstance_t *pxNormalInstance;
        struct ipcpInstanceData_t *pxInstanceData;

        /* Create Instance*/

        pxNormalInstance = pvPortMalloc(sizeof(*pxNormalInstance));

        if (!pxNormalInstance)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Could not allocate memory for normal ipc process");
                return pdFALSE;
        }

        pxNormalInstance->pxOps = &xNormalInstanceOps;
        pxNormalInstance->xType = eNormal;
        pxNormalInstance->xId = xIpcpId;

        pxInstanceData = pvPortMalloc(sizeof(*pxInstanceData));

        pxNormalInstance->pxData = pxInstanceData;

        if (!pxNormalInstance->pxData)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Could not allocate memory for normal ipcp internal data");
                vPortFree(pxNormalInstance);
                return pdFALSE;
        }

        pxNormalInstance->pxData->xId = xIpcpId;

        /*Name and DIF copy*/

        /*Create an object name_t and fill it*/
        pxName = pvPortMalloc(sizeof(*pxName));
        if (!pxName)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Failed creation of ipc name");
                vPortFree(pxNormalInstance);
                return pdFALSE;
        }

        pxName->pcEntityInstance = NORMAL_ENTITY_INSTANCE;
        pxName->pcEntityName = NORMAL_ENTITY_NAME;
        pxName->pcProcessInstance = NORMAL_PROCESS_INSTANCE;
        pxName->pcProcessName = NORMAL_PROCESS_NAME;
        pxNormalInstance->pxData->pxName = pxName;

        pxDifName = pvPortMalloc(sizeof(*pxDifName));

        pxDifName->pcProcessName = NORMAL_DIF_NAME;
        pxDifName->pcProcessInstance = "";
        pxDifName->pcEntityInstance = "";
        pxDifName->pcEntityName = "";
        pxNormalInstance->pxData->pxDifName = pxDifName;

        if (!pxName)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Failed creation of ipc name");
                vPortFree(pxNormalInstance);
                return pdFALSE;
        }

        if (!xRstringDup(pxName->pcProcessName, &pxNormalInstance->pxData->pxName->pcProcessName) ||
            !xRstringDup(pxName->pcProcessInstance, &pxNormalInstance->pxData->pxName->pcProcessInstance) ||
            !xRstringDup(pxName->pcEntityName, &pxNormalInstance->pxData->pxName->pcEntityName) ||
            !xRstringDup(pxName->pcEntityInstance, &pxNormalInstance->pxData->pxName->pcEntityInstance))
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Name was not created properly");
        }

        /* ----------------------------------------- */

        pxNormalInstance->pxData->pxFlowAllocator = pxFlowAllocatorInit();

        pxNormalInstance->pxData->pxEfcpc = pxEfcpContainerCreate();
        /*instance->data->efcpc = efcp_container_create(instance->data->kfa,
                                                  &instance->robj);*/
        if (!pxNormalInstance->pxData->pxEfcpc)
        {
                vPortFree(pxNormalInstance->pxData);
                vPortFree(pxNormalInstance);
                return pdFALSE;
        }

        pxNormalInstance->pxData->pxRmt = pxRmtCreate(pxNormalInstance->pxData->pxEfcpc, pxNormalInstance);
        if (!pxNormalInstance->pxData->pxRmt)
        {
                ESP_LOGE(TAG_IPCPNORMAL, "Failed creation of RMT instance");
                // sdup_destroy(instance->data->sdup);
                // efcp_container_destroy(instance->data->efcpc);
                vPortFree(pxNormalInstance->pxData);
                vPortFree(pxNormalInstance);
                return pdFALSE;
        }

        pxNormalInstance->pxData->pxEfcpc->pxRmt = pxNormalInstance->pxData->pxRmt;

        /* rtimer_init(tf_use_naddress,
                &instance->data->timers.use_naddress,
                instance->data);
    rtimer_init(tf_kill_oaddress,
                &instance->data->timers.kill_oaddress,
                instance->data);*/

        if (pvNormalAssignToDif(pxNormalInstance->pxData, pxDifName))
        {
                ESP_LOGI(TAG_IPCPNORMAL, "Normal Instance Assigned to DIF");
        }

        vListInitialise(&(pxNormalInstance->pxData->xFlowsList));

        /*Initialialise instance item and add to the pxFactoryDataList*/
        vListInitialiseItem(&(pxNormalInstance->pxData->xInstanceListItem));
        listSET_LIST_ITEM_OWNER(&(pxNormalInstance->pxData->xInstanceListItem), pxNormalInstance);
        vListInsert(&(pxData->xInstancesNormalList), &(pxNormalInstance->pxData->xInstanceListItem));

        ESP_LOGI(TAG_IPCPNORMAL, "Normal Instance created: %p, IPCP id:%d ", pxNormalInstance, pxNormalInstance->xId);

        ESP_LOGI(TAG_IPCPNORMAL, "Normal IPC process instance created and added to the list");

        return pxNormalInstance;
}

static ipcpFactoryOps_t xFactoryNormalOps = {
    .init = xNormalInit,
    .fini = NULL,
    .create = pxNormalCreate,
    //.destroy = NULL,
};

/* Init xNormalFactory, register into the FactoryList of the IPCPTask*/
BaseType_t xNormalIPCPInitFactory(factories_t *pxFactoriesList);

BaseType_t xNormalIPCPInitFactory(factories_t *pxFactoriesList)

{
        ESP_LOGI(TAG_IPCPNORMAL, "Registering Normal Factory");
        if (xFactoryIPCPRegister(pxFactoriesList, eFactoryNormal, &xFactoryNormalData, &xFactoryNormalOps))
        {
                return pdTRUE;
        }
        return pdFALSE;
}

/* Called from the IPCP Task to the NormalIPCP register as APP into the SHIM DIF
 * depending on the Type of ShimDIF.*/
BaseType_t xNormalRegistering(ipcpInstance_t *pxInstanceFrom, ipcpInstance_t *pxInstanceTo)
{

        if (pxInstanceTo->pxOps->applicationRegister == NULL)
        {
                ESP_LOGI(TAG_IPCPNORMAL, "There is not Application Register API");
        }
        if (pxInstanceTo->pxOps->applicationRegister(pxInstanceTo->pxData, pxInstanceFrom->pxData->pxName,
                                                     pxInstanceFrom->pxData->pxDifName))
        {
                ESP_LOGI(TAG_IPCPNORMAL, "Normal Instance Registered");
                return pdTRUE;
        }

        return pdFALSE;
}

/* Normal IPCP request a Flow Allocation to the Shim */
BaseType_t xNormalFlowAllocationRequest(ipcpInstance_t *pxInstanceFrom, ipcpInstance_t *pxInstanceTo, portId_t xShimPortId)
{

        /*This should be proposed by the Flow Allocator?*/
        name_t *destinationInfo = pvPortMalloc(sizeof(*destinationInfo));
        destinationInfo->pcProcessName = REMOTE_ADDRESS_AP_NAME;
        destinationInfo->pcEntityName = "";
        destinationInfo->pcProcessInstance = REMOTE_ADDRESS_AP_INSTANCE;
        destinationInfo->pcEntityInstance = "";

        if (pxInstanceTo->pxOps->flowAllocateRequest == NULL)
        {
                ESP_LOGI(TAG_IPCPNORMAL, "There is not Flow Allocate Request API");
        }
        if (pxInstanceTo->pxOps->flowAllocateRequest(xShimPortId,
                                                     pxInstanceFrom,
                                                     pxInstanceTo->pxData->pxName,
                                                     destinationInfo,
                                                     pxInstanceTo->pxData))
        {
                ESP_LOGI(TAG_IPCPNORMAL, "Flow Request Sended");
                return pdTRUE;
        }

        return pdFALSE;
}

#if 0
BaseType_t xNormalAppFlowAllocationRequestHandle(ipcpInstance_t)



    /* Call to xNormalFlowBind */

    /* Call to xNormalConnectionCreate */
    xNormalFlowPrebind(pxNormalInstance->pxData, xPortId);

address_t xSource = 10;
address_t xDest = 3;
qosId_t xQosId = 1;
dtpConfig_t *pxDtpCfg = pvPortMalloc(sizeof(*pxDtpCfg));
struct dtcpConfig_t *pxDtcpCfg = pvPortMalloc(sizeof(*pxDtcpCfg));

xNormalConnectionCreateRequest(pxNormalInstance->pxData, xPortId,
                               xSource, xDest, xQosId, pxDtpCfg,
                               pxDtcpCfg);

#endif
