#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "Freertos/FreeRTOS.h"
#include "Freertos/task.h"
#include "Freertos/queue.h"
#include "Freertos/semphr.h"

#include "IPCP.h"
#include "EFCP.h"
#include "RMT.h"
#include "common.h"
#include "du.h"
#include "factoryIPCP.h"

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
        ipcProcessId_t xId;
        //irati_msg_port_t        irati_port;
        name_t xName;
        name_t xDifName;
        List_t xFlowsList;
        /*  FIXME: Remove it as soon as the kipcm_kfa gets removed*/
        //struct kfa *            kfa;
        struct efcpContainer_t *pxEfcpc;
        struct rmt_t *pxRmt;
        //struct sdup *           sdup;
        address_t xAddress;
        address_t xOldAddress;
        ///spinlock_t              lock;
        ListItem_t xInstanceListItem;
        /* Timers required for the address change procedure 
        struct {
        	struct timer_list use_naddress;
            struct timer_list kill_oaddress;
        } timers;*/
};

static struct ipcpInstanceOps_t xNormalInstanceOps = {
    .flowAllocateRequest = NULL,   //ok
    .flowAllocateResponse = NULL,  //ok
    .flowDeallocate = NULL,        //ok
    .flowPrebind = NULL,           //ok
    .flowBindingIpcp = NULL,       //ok
    .flowUnbindingIpcp = NULL,     //ok
    .flowUnbindingUserIpcp = NULL, //ok
    .nm1FlowStateChange = NULL,    //ok

    .applicationRegister = NULL,   //ok
    .applicationUnregister = NULL, //ok

    .assignToDif = NULL,     //ok
    .updateDifConfig = NULL, //ok

    .connectionCreate = NULL,        //ok
    .connectionUpdate = NULL,        //ok
    .connectionDestroy = NULL,       //ok
    .connectionCreateArrived = NULL, // ok
    .connectionModify = NULL,        //ok

    .duEnqueue = NULL, //ok
    .duWrite = NULL,   //ok

    .mgmtDuWrite = NULL, //ok
    .mgmtDuPost = NULL,  //ok

    .pffAdd = NULL,    //ok
    .pffRemove = NULL, //ok
    //.pff_dump                  = NULL,
    //.pff_flush                 = NULL,
    //.pff_modify		   		   = NULL,

    //.query_rib		  		   = NULL,

    .ipcpName = NULL, //ok
    .difName = NULL,  //ok
    //.ipcp_id		  		   = NULL,

    //.set_policy_set_param      = NULL,
    //.select_policy_set         = NULL,
    //.update_crypto_state	   = NULL,
    //.address_change            = NULL,
    //.dif_name		   		   = NULL,
    .maxSduSize = NULL};

static struct normalFlow_t *prvNormalFindFlow(struct ipcpInstanceData_t *pxData,
                                              portId_t xPortId)
{

        struct normalFlow_t *pxFlow;
        //shimFlow_t *pxFlowNext;

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        pxFlow = pvPortMalloc(sizeof(*pxFlow));

        /* Find a way to iterate in the list and compare the addesss*/
        pxListEnd = listGET_END_MARKER(&pxData->xFlowsList);
        pxListItem = listGET_HEAD_ENTRY(&pxData->xFlowsList);

        while (pxListItem != pxListEnd)
        {

                pxFlow = (struct normalFlow_t *)listGET_LIST_ITEM_OWNER(pxListItem);

                if (pxFlow && pxFlow->xPortId == xPortId)
                {

                        //ESP_LOGI(TAG_IPCP, "Flow founded %p, portID: %d, portState:%d", pxFlow, pxFlow->xPortId, pxFlow->eState);
                        return pxFlow;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        ESP_LOGI(TAG_IPCP, "Flow not founded");
        return NULL;
}

BaseType_t xNormalDuWrite(struct ipcpInstanceData_t *pxData,
                          portId_t xId,
                          struct du_t *pxDu)
{
        ESP_LOGI(TAG_IPCP, "xNormalDuWrite");

        struct normalFlow_t *pxFlow;

        pxFlow = prvNormalFindFlow(pxData, xId);
        if (!pxFlow || pxFlow->eState != ePORT_STATE_ALLOCATED)
        {

                ESP_LOGE(TAG_IPCP, "Write: There is no flow bound to this port_id: %d",
                         xId);
                xDuDestroy(pxDu);
                return pdFALSE;
        }

        if (xEfcpContainerWrite(pxData->pxEfcpc, pxFlow->xActive, pxDu))
        {
                ESP_LOGE(TAG_IPCP, "Could not send sdu to EFCP Container");
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

//ipcpInstance_t *pxNormalCreate(name_t *pxName, ipcProcessId_t xId);

//BaseType_t * pxNormalCreate(ipcpFactory_t *pxFactory)

static BaseType_t xNormalFlowPrebind(struct ipcpInstanceData_t *pxData,
                                     portId_t xPortId);

static BaseType_t xNormalFlowPrebind(struct ipcpInstanceData_t *pxData,
                                     portId_t xPortId)
{
        ESP_LOGI(TAG_IPCP, "normalFlowPrebinding");

        struct normalFlow_t *pxFlow;

        if (!pxData)
        {
                ESP_LOGE(TAG_IPCP, "Wrong input parameters...");
                return pdFALSE;
        }
        pxFlow = prvNormalFindFlow(pxData, xPortId);

        if (!pxFlow)
        {
                pxFlow = pvPortMalloc(sizeof(*pxFlow));
                if (!pxFlow)
                {
                        ESP_LOGE(TAG_IPCP, "Could not create a flow in normal-ipcp to pre-bind");
                        return pdFALSE;
                }

                pxFlow->xPortId = xPortId;
                pxFlow->eState = ePORT_STATE_ALLOCATED;

                ESP_LOGI(TAG_IPCP, "Flow: %p portID: %d portState: %d", pxFlow, pxFlow->xPortId, pxFlow->eState);
                vListInitialiseItem(&(pxFlow->xFlowListItem));
                listSET_LIST_ITEM_OWNER(&(pxFlow->xFlowListItem), (void *)pxFlow);
                vListInsert(&(pxData->xFlowsList), &(pxFlow->xFlowListItem));

                ESP_LOGI(TAG_IPCP, "normalFlowPrebinded");
        }

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

        /*if (!pxUserIpcp)
                return cep_id_bad();*/

        xCepId = xEfcpConnectionCreate(pxData->pxEfcpc, xSource, xDest,
                                       xPortId, xQosId,
                                       cep_id_bad(), cep_id_bad(),
                                       pxDtpCfg, pxDtcpCfg);
        if (!is_cep_id_ok(xCepId))
        {
                ESP_LOGE(TAG_IPCP, "Failed EFCP connection creation");
                return cep_id_bad();
        }

        pxCepEntry = pvPortMalloc(sizeof(*pxCepEntry));
        if (!pxCepEntry)
        {
                ESP_LOGE(TAG_IPCP, "Could not create a cep_id entry, bailing out");
                xEfcpConnectionDestroy(pxData->pxEfcpc, xCepId);
                return cep_id_bad();
        }

        //vListInitialise(&pxCepEntry->CepIdListItem);
        pxCepEntry->xCepId = xCepId;

        /*/ipcp = kipcm_find_ipcp(default_kipcm, data->id);
        if (!ipcp) {
                ESP_LOGE(TAG_IPCP,"KIPCM could not retrieve this IPCP");
                xEfcpConnectionDestroy(pxData->efcpc, cep_id);
                return cep_id_bad();
        }*/

        //configASSERT(xUserIpcp->xOps);
        //configASSERT(xUserIpcp->xOps->flow_binding_ipcp);
        //spin_lock_bh(&data->lock);
        pxFlow = prvNormalFindFlow(pxData, xPortId);

#if 0
        if (!pxFlow) {
                //spin_unlock_bh(&data->lock);
                ESP_LOGE(TAG_IPCP,"Could not retrieve normal flow to create connection");
                xEfcpConnectionDestroy(pxData->efcpc, xCepId);
                return cep_id_bad();
        }

        if (user_ipcp->ops->flow_binding_ipcp(user_ipcp->data,
                                              port_id,
                                              ipcp)) {
                spin_unlock_bh(&data->lock);
                ESP_LOGE(TAG_IPCP,"Could not bind flow with user_ipcp");
                efcp_connection_destroy(data->efcpc, cep_id);
                return cep_id_bad();
        }

        list_add(&cep_entry->list, &flow->cep_ids_list);
#endif
        pxFlow->xActive = xCepId;
        // pxFlow->eState = ePORTSTATEPENDING;

        //spin_unlock_bh(&data->lock);

        return xCepId;
}

BaseType_t xNormalFlowBinding(struct ipcpInstanceData_t *pxUserData,
                              portId_t xPid,
                              ipcpInstance_t *pxN1Ipcp)
{
        return xRmtN1PortBind(pxUserData->pxRmt, xPid, pxN1Ipcp);
}

BaseType_t xNormalTest(ipcpInstance_t *pxNormalInstance, ipcpInstance_t *pxN1Ipcp)
{
        ESP_LOGI(TAG_IPCP, "Test");

        portId_t xId = 1;

        /* Data User */
        struct du_t *testDu;
        NetworkBufferDescriptor_t *pxNetworkBuffer;
        size_t xBufferSize;

        /* String to send*/
        char *ucStringTest = "Temperature:22";

        /*Getting the buffer Descriptor*/
        xBufferSize = strlen(ucStringTest);
        pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(xBufferSize, (TickType_t)0U); //sizeof length DataUser packet.

        ESP_LOGI(TAG_IPCP,"BufferSize DU:%d",xBufferSize);

        /*Copy the string to the Buffer Network*/
        memcpy(pxNetworkBuffer->pucEthernetBuffer, ucStringTest, xBufferSize);

        pxNetworkBuffer->xDataLength = xBufferSize;

        //ESP_LOGI(TAG_IPCP, "Size of NetworkBuffer: %d",pxNetworkBuffer->xDataLength);
        /*Integrate the buffer to the Du structure*/
        testDu = pvPortMalloc(sizeof(*testDu));
        testDu->pxNetworkBuffer = pxNetworkBuffer;
        ESP_LOGI(TAG_IPCP, "Du Filled");

        ESP_LOGI(TAG_IPCP, "Normal Instance: %p", pxNormalInstance);

        if (xNormalFlowBinding(pxNormalInstance->pxData, xId, pxN1Ipcp))
        {
                ESP_LOGI(TAG_IPCP, "FlowBinding");
        }

        /*Call to Normalwrite function to send data*/
        if (xNormalDuWrite(pxNormalInstance->pxData, xId, testDu))
        {
                ESP_LOGI(TAG_IPCP, "Wrote packet on the shimWiFi");
                return pdTRUE;
        }

        return pdFALSE;
}




BaseType_t pxNormalCreate(ipcpFactory_t *pxFactory)
{
        //ipcpInstance_t *pxNormalInstance;
        /*HardCoded*/
        //---------------------
        name_t *pxName;
        ipcProcessId_t xId = 1;
        ipcpInstance_t *pxNormalInstance;
        portId_t xPortId = 1;

        pxNormalInstance = pvPortMalloc(sizeof(*pxNormalInstance));
        pxNormalInstance->pxData = pvPortMalloc(sizeof(struct ipcpInstanceData_t));

        pxName = pvPortMalloc(sizeof(*pxName));

        pxName->pcEntityInstance = "1";
        pxName->pcEntityName = "2";
        pxName->pcProcessInstance = "1";
        pxName->pcProcessName = "ar1.mobile";

        //printf("pxNAME:%p\n", pxName);
        //----------------------------

        /* Allocate instance */
        // pxNormalInstance = pvPortMalloc(sizeof(*pxNormalInstance));
        if (!pxNormalInstance)
        {
                ESP_LOGE(TAG_IPCP, "Could not allocate memory for normal ipc process");
                return pdFALSE;
        }

        pxNormalInstance->pxOps = &xNormalInstanceOps;
        pxNormalInstance->xType = eNormal;

        if (!pxNormalInstance->pxData)
        {
                ESP_LOGE(TAG_IPCP, "Could not allocate memory for normal ipcp internal data");
                vPortFree(pxNormalInstance);
                return pdFALSE;
        }

        pxNormalInstance->pxData->xId = xId;

        if (pxName)
        {

                memcpy(&pxNormalInstance->pxData->xName.pcEntityInstance, pxName->pcEntityInstance,
                       strlen(pxName->pcEntityInstance));
                memcpy(&pxNormalInstance->pxData->xName.pcEntityName, pxName->pcEntityName,
                       strlen(pxName->pcEntityName));
                memcpy(&pxNormalInstance->pxData->xName.pcProcessInstance, pxName->pcProcessInstance,
                       strlen(pxName->pcProcessInstance));
                memcpy(&pxNormalInstance->pxData->xName.pcProcessName, pxName->pcProcessName,
                       strlen(pxName->pcProcessName));
        }
        else
        {
                ESP_LOGE(TAG_IPCP, "Failed creation of ipc name");
                vPortFree(pxNormalInstance);
                return pdFALSE;
        }

        pxNormalInstance->pxData->pxEfcpc = pxEfcpContainerCreate();
        /*instance->data->efcpc = efcp_container_create(instance->data->kfa,
                                                  &instance->robj);*/
        if (!pxNormalInstance->pxData->pxEfcpc)
        {
                vPortFree(pxNormalInstance->pxData);
                vPortFree(pxNormalInstance);
                return pdFALSE;
        }

        pxNormalInstance->pxData->pxRmt = pxRmtCreate(pxNormalInstance->pxData->pxEfcpc);
        if (!pxNormalInstance->pxData->pxRmt)
        {
                ESP_LOGE(TAG_IPCP, "Failed creation of RMT instance");
                //sdup_destroy(instance->data->sdup);
                //efcp_container_destroy(instance->data->efcpc);
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

        vListInitialise(&(pxNormalInstance->pxData->xFlowsList));

        /*Initialialise instance item and add to the pxFactory*/
        vListInitialiseItem(&(pxNormalInstance->pxData->xInstanceListItem));
        listSET_LIST_ITEM_OWNER(&(pxNormalInstance->pxData->xInstanceListItem), pxNormalInstance);
        vListInsert(&(pxFactory->xIPCPInstancesList), &(pxNormalInstance->pxData->xInstanceListItem));

        ESP_LOGI(TAG_IPCP, "Normal Instance created: %p ", pxNormalInstance);

        /*INIT_LIST_HEAD(&instance->data->flows);
    INIT_LIST_HEAD(&instance->data->list);
    spin_lock_init(&instance->data->lock);
    list_add(&(instance->data->list), &(data->instances));*/

        ESP_LOGI(TAG_IPCP, "Normal IPC process instance created and added to the list");

        if (!pxNormalInstance)
        {
                return pdFALSE;
        }

        /*Testing*/
        xNormalFlowPrebind(pxNormalInstance->pxData, xPortId);

        address_t xSource = 10;
        address_t xDest = 3;
        qosId_t xQosId = 1;
        dtpConfig_t *pxDtpCfg = pvPortMalloc(sizeof(*pxDtpCfg));
        struct dtcpConfig_t *pxDtcpCfg = pvPortMalloc(sizeof(*pxDtcpCfg));

        xNormalConnectionCreateRequest(pxNormalInstance->pxData, xPortId,
                                       xSource, xDest, xQosId, pxDtpCfg,
                                       pxDtcpCfg);

        

      

            return pdTRUE;
}


/*
static BaseType_t pvNormalAssignToDif(struct ipcpInstanceData_t * pxData,
		                const name_t * pxDifName,
				const string_t * pxType,
				struct dif_config * pxConfig)
{
        struct efcp_config * efcp_config;
        struct secman_config * sm_config;
        struct rmt_config *  rmt_config;

        if (name_cpy(dif_name, &data->dif_name)) {
                LOG_ERR("%s: name_cpy() failed", __func__);
                return -1;
        }

        data->address  = config->address;

        efcp_config = config->efcp_config;
        config->efcp_config = 0;

        if (!efcp_config) {
                LOG_ERR("No EFCP configuration in the dif_info");
                return -1;
        }

        if (!efcp_config->dt_cons) {
                LOG_ERR("Configuration constants for the DIF are bogus...");
                efcp_config_free(efcp_config);
                return -1;
        }

        efcp_container_config_set(data->efcpc, efcp_config);

        rmt_config = config->rmt_config;
        config->rmt_config = 0;
        if (!rmt_config) {
        	LOG_ERR("No RMT configuration in the dif_info");
        	return -1;
        }

        if (rmt_address_add(data->rmt, data->address)) {
		LOG_ERR("Could not set local Address to RMT");
                return -1;
	}

        if (rmt_config_set(data-> rmt, rmt_config)) {
                LOG_ERR("Could not set RMT conf");
		return -1;
        }

        sm_config = config->secman_config;
        config->secman_config = 0;
	if (!sm_config) {
		LOG_INFO("No SDU protection config specified, using default");
		sm_config = secman_config_create();
		sm_config->default_profile = auth_sdup_profile_create();
	}
	if (sdup_config_set(data->sdup, sm_config)) {
                LOG_ERR("Could not set SDUP conf");
		return -1;
	}
	if (sdup_dt_cons_set(data->sdup, dt_cons_dup(efcp_config->dt_cons))) {
                LOG_ERR("Could not set dt_cons in SDUP");
		return -1;
	}

        return 0;
}*/
