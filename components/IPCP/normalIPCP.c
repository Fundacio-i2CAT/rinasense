#include <stdio.h>
#include <string.h>

#include "EFCP.h"
#include "portability/rslist.h"
#include "rmt.h"
#include "rina_common_port.h"
#include "du.h"
#include "configRINA.h"
#include "rina_name.h"
#include "IpcManager.h"
#include "BufferManagement.h"
#include "Ribd.h"
#include "Ribd_api.h"

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
        RsListItem_t CepIdListItem;
};

struct normalFlow_t
{
        portId_t xPortId;
        cepId_t xActive;
        RsList_t xCepIdsList;
        eNormalFlowState_t eState;
        ipcpInstance_t *pxUserIpcp;
        RsListItem_t xFlowListItem;
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
        address_t xOldAddress;

        /// spinlock_t              lock;

        /* Instance List Item to be add into the Factory List*/
        RsListItem_t xInstanceListItem;

        /* Timers required for the address change procedure
        struct {
                struct timer_list use_naddress;
            struct timer_list kill_oaddress;
        } timers;*/
};

struct ipcpFactoryData_t
{
        RsList_t xInstancesNormalList;
};

static struct ipcpFactoryData_t xFactoryNormalData;

static bool_t pvNormalAssignToDif(struct ipcpInstanceData_t *pxData, name_t *pxDifName);

static bool_t xNormalInit(struct ipcpFactoryData_t *pxData)
{
        // memset(&xNormalData, 0, sizeof(xNormalData));
        vRsListInit(&pxData->xInstancesNormalList);
        return true;
}

static ipcpInstance_t *prvNormalIPCPFindInstance(struct ipcpFactoryData_t *pxFactoryData,
                                                 ipcpInstanceType_t xType);

static ipcpInstance_t *prvNormalIPCPFindInstance(struct ipcpFactoryData_t *pxFactoryData,
                                                 ipcpInstanceType_t xType)
{
        ipcpInstance_t *pxInstance;
        RsListItem_t *pxListItem;
        RsListItem_t const *pxListEnd;

        pxInstance = pvRsMemAlloc(sizeof(*pxInstance));

        /* Find a way to iterate in the list and compare the addesss*/

        pxListEnd = pRsListGetEndMarker(&pxFactoryData->xInstancesNormalList);
        pxListItem = pRsListGetHeadEntry(&pxFactoryData->xInstancesNormalList);

        while (pxListItem != pxListEnd)
        {
                pxInstance = (struct ipcpInstance *)pRsListGetListItemOwner(pxListItem);

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

                pxListItem = pRsListGetNext(pxListItem);
        }
}

static struct normalFlow_t *prvNormalFindFlow(struct ipcpInstanceData_t *pxData,
                                              portId_t xPortId)
{
        struct normalFlow_t *pxFlow;
        RsListItem_t *pxListItem;
        RsListItem_t const *pxListEnd;

        pxFlow = pvRsMemAlloc(sizeof(*pxFlow));

        /* Find a way to iterate in the list and compare the addesss*/
        pxListEnd = pRsListGetEndMarker(&pxData->xFlowsList);
        pxListItem = pRsListGetHeadEntry(&pxData->xFlowsList);

        while (pxListItem != pxListEnd)
        {
                pxFlow = (struct normalFlow_t *)pRsListGetListItemOwner(pxListItem);

                if (pxFlow && pxFlow->xPortId == xPortId)
                {

                        // ESP_LOGI(TAG_IPCPNORMAL, "Flow founded %p, portID: %d, portState:%d", pxFlow, pxFlow->xPortId, pxFlow->eState);
                        return pxFlow;
                }

                pxListItem = pRsListGetNext(pxListItem);
        }

        LOGI(TAG_IPCPNORMAL, "Flow not founded");
        return NULL;
}

bool_t xNormalDuWrite(struct ipcpInstanceData_t *pxData,
                      portId_t xId,
                      struct du_t *pxDu)
{
        LOGI(TAG_IPCPNORMAL, "xNormalDuWrite");

        struct normalFlow_t *pxFlow;

        pxFlow = prvNormalFindFlow(pxData, xId);

        if (!pxFlow || pxFlow->eState != ePORT_STATE_ALLOCATED)
        {

                LOGE(TAG_IPCPNORMAL, "Write: There is no flow bound to this port_id: %d",
                     xId);
                xDuDestroy(pxDu);
                return false;
        }

        if (xEfcpContainerWrite(pxData->pxEfcpc, pxFlow->xActive, pxDu))
        {
                LOGE(TAG_IPCPNORMAL, "Could not send sdu to EFCP Container");
                return false;
        }

        return true;
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

bool_t xNormalFlowPrebind(struct ipcpInstanceData_t *pxData,
                          portId_t xPortId)
{

        struct normalFlow_t *pxFlow;

        if (!pxData)
        {
                LOGE(TAG_IPCPNORMAL, "Wrong input parameters...");
                return false;
        }

        pxFlow = pvRsMemAlloc(sizeof(*pxFlow));
        if (!pxFlow)
        {
                LOGE(TAG_IPCPNORMAL, "Could not create a flow in normal-ipcp to pre-bind");
                return false;
        }

        pxFlow->xPortId = xPortId;
        pxFlow->eState = ePORT_STATE_PENDING;
        /*KFA should be the user. Implement this when the KFA is implemented*/
        // pxFlow->pxUserIpcp = kfa;

        // ESP_LOGI(TAG_IPCPNORMAL, "Flow: %p portID: %d portState: %d", pxFlow, pxFlow->xPortId, pxFlow->eState);
        vRsListInitItem(&(pxFlow->xFlowListItem));
        vRsListSetListItemOwner(&(pxFlow->xFlowListItem), (void *)pxFlow);
        vRsListInsert(&(pxData->xFlowsList), &(pxFlow->xFlowListItem));

        return true;
}

cepId_t xNormalConnectionCreateRequest(struct efcpContainer_t *pxEfcpc,
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

        LOGE(TAG_EFCP, "Pointer EFCP:%p", pxEfcpc);

#ifdef __FREERTOS__
        heap_caps_check_integrity(MALLOC_CAP_DEFAULT, pdTRUE);
#endif

        xCepId = xEfcpConnectionCreate(pxEfcpc, xSource, xDest,
                                       xPortId, xQosId,
                                       cep_id_bad(), cep_id_bad(),
                                       pxDtpCfg, pxDtcpCfg);
#ifdef __FREERTOS__
        heap_caps_check_integrity(MALLOC_CAP_DEFAULT, pdTRUE);
#endif

        if (!is_cep_id_ok(xCepId))
        {
                LOGE(TAG_IPCPNORMAL, "Failed EFCP connection creation");
                return cep_id_bad();
        }

#ifdef __FREERTOS__
        heap_caps_check_integrity_all(pdTRUE);
#endif

        /*        pxCepEntry = pvPortMalloc(sizeof(*pxCepEntry)); // error
                if (!pxCepEntry)
                {
                        ESP_LOGE(TAG_IPCPNORMAL, "Could not create a cep_id entry, bailing out");
                        xEfcpConnectionDestroy(pxData->pxEfcpc, xCepId);
                        return cep_id_bad();
                }*/

        // vListInitialise(&pxCepEntry->CepIdListItem);
        // pxCepEntry->xCepId = xCepId;

        /*/ipcp = kipcm_find_ipcp(default_kipcm, data->id);
        if (!ipcp) {
                ESP_LOGE(TAG_IPCPNORMALNORMAL,"KIPCM could not retrieve this IPCP");
                xEfcpConnectionDestroy(pxData->efcpc, cep_id);
                return cep_id_bad();
        }*/

        // configASSERT(xUserIpcp->xOps);
        // configASSERT(xUserIpcp->xOps->flow_binding_ipcp);
        // spin_lock_bh(&data->lock);
        // pxFlow = prvNormalFindFlow(pxData, xPortId);

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

        // pxFlow->xActive = xCepId;
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
bool_t xNormalFlowBinding(struct ipcpNormalData_t *pxUserData,
                              portId_t xPid,
                              ipcpInstance_t *pxN1Ipcp)
{
        return xRmtN1PortBind(pxUserData->pxRmt, xPid, pxN1Ipcp);
}

bool_t xNormalTest(ipcpInstance_t *pxNormalInstance, ipcpInstance_t *pxN1Ipcp)
{
        LOGI(TAG_IPCPNORMAL, "Test");

        portId_t xId = 1;

        /* Data User */
        struct du_t *testDu;
        NetworkBufferDescriptor_t *pxNetworkBuffer;
        size_t xBufferSize;
        struct timespec ts;

        /* String to send*/
        char *ucStringTest = "Temperature:22";

        if (rstime_waitmsec(&ts, 1)) {
            LOGE(TAG_IPCPNORMAL, "rstime_waitmsec failed");
            return false;
        }

        /*Getting the buffer Descriptor*/
        xBufferSize = strlen(ucStringTest);
        pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(xBufferSize, &ts); // sizeof length DataUser packet.

        LOGI(TAG_IPCPNORMAL, "BufferSize DU:%u", xBufferSize);

        /*Copy the string to the Buffer Network*/
        memcpy(pxNetworkBuffer->pucEthernetBuffer, ucStringTest, xBufferSize);

        pxNetworkBuffer->xDataLength = xBufferSize;

        // ESP_LOGI(TAG_IPCPNORMALNORMAL, "Size of NetworkBuffer: %d",pxNetworkBuffer->xDataLength);
        /*Integrate the buffer to the Du structure*/
        testDu = pvRsMemAlloc(sizeof(*testDu));
        testDu->pxNetworkBuffer = pxNetworkBuffer;
        LOGI(TAG_IPCPNORMAL, "Du Filled");

        LOGI(TAG_IPCPNORMAL, "Normal Instance: %p", pxNormalInstance);

        /*if (xNormalFlowBinding(pxNormalInstance->pxData, xId, pxN1Ipcp))
        {
                ESP_LOGI(TAG_IPCPNORMAL, "FlowBinding");
        }*/

        /*Call to Normalwrite function to send data*/
        if (xNormalDuWrite(pxNormalInstance->pxData, xId, testDu))
        {
                LOGI(TAG_IPCPNORMAL, "Wrote packet on the shimWiFi");
                return true;
        }

        return false;
}

static bool_t pvNormalAssignToDif(struct ipcpInstanceData_t *pxData, name_t *pxDifName)
{
        efcpConfig_t *pxEfcpConfig;
        // struct secman_config * sm_config;
        // rmtConfig_t *pxRmtConfig;

        if (!pxDifName)
        {
                LOGE(TAG_IPCPNORMAL, "Failed creation of dif name");

                return false;
        }

        if (!xRstringDup(pxDifName->pcProcessName, &pxData->pxDifName->pcProcessName) ||
            !xRstringDup(pxDifName->pcProcessInstance, &pxData->pxDifName->pcProcessInstance) ||
            !xRstringDup(pxDifName->pcEntityName, &pxData->pxDifName->pcEntityName) ||
            !xRstringDup(pxDifName->pcEntityInstance, &pxData->pxDifName->pcEntityInstance))
        {
                LOGE(TAG_IPCPNORMAL, "Name was not created properly");
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
                LOGE(TAG_IPCPNORMAL, "Could not set local Address to RMT");
                return false;
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

        return true;
}

bool_t xNormalDuEnqueue(struct ipcpNormalData_t *pxData,
                        portId_t xN1PortId,
                        struct du_t *pxDu)
{
        if (!xRmtReceive(pxData, pxDu, xN1PortId))
        {
                LOGE(TAG_IPCPNORMAL, "Could not enqueue SDU into the RMT");
                return false;
        }

        return true;
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
bool_t xNormalMgmtDuWrite(struct rmt_t *pxRmt, portId_t xPortId, struct du_t *pxDu)
{
        ssize_t sbytes;

        LOGI(TAG_IPCPNORMAL, "Passing SDU to be written to N-1 port %d ", xPortId);

        if (!pxRmt)
        {
                LOGE(TAG_IPCPNORMAL, "No RMT passed");
                return false;
        }

        if (!pxDu)
        {
                LOGE(TAG_IPCPNORMAL, "No data passed, bailing out");
                return false;
        }

        // pxDu->pxCfg = pxData->pxEfcpc->pxConfig;
        sbytes = xDuLen(pxDu);

        if (!xDuEncap(pxDu, PDU_TYPE_MGMT))
        {
                LOGE(TAG_IPCPNORMAL, "No data passed, bailing out");
                xDuDestroy(pxDu);
                return false;
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
        pxDu->pxPci->xSource = LOCAL_ADDRESS;

        // vPciPrint(pxDu->pxPci);

        // pxRmt = pxIpcpGetRmt();

        if (xPortId)
        {
                if (!xRmtSendPortId(pxRmt, xPortId, pxDu))
                {
                        LOGE(TAG_IPCPNORMAL, "Could not sent to RMT");
                        return false;
                }
        }
        else
        {
                LOGE(TAG_IPCPNORMAL, "Could not sent to RMT: no portID");
                xDuDestroy(pxDu);
                return false;
        }

        return true;
}

bool_t xNormalMgmtDuPost(struct ipcpNormalData_t *pxData, portId_t xPortId, struct du_t *pxDu)
{

        if (!is_port_id_ok(xPortId))
        {
                LOGE(TAG_IPCPNORMAL, "Wrong port id");
                xDuDestroy(pxDu);
                return false;
        }
        /*if (!IsDuOk(pxDu)) {
                ESP_LOGE(TAG_IPCPNORMAL,"Bogus management SDU");
                xDuDestroy(pxDu);
                return pdFALSE;
        }*/

        /*Send to the RIB Daemon*/
        if (!xRibdProcessLayerManagementPDU(pxData, xPortId, pxDu))
        {
                LOGI(TAG_IPCPNORMAL, "Was not possible to process el Management PDU");
                return false;
        }

        return true;
}

static struct ipcpInstanceOps_t xNormalInstanceOps = {
    .flowAllocateRequest = NULL,       // ok
    .flowAllocateResponse = NULL,      // ok
    .flowDeallocate = NULL,            // ok
    .flowPrebind = xNormalFlowPrebind, // ok
    .flowBindingIpcp = NULL,           // ok
    .flowUnbindingIpcp = NULL,         // ok
    .flowUnbindingUserIpcp = NULL,     // ok
    .nm1FlowStateChange = NULL,        // ok

    .applicationRegister = NULL,   // ok
    .applicationUnregister = NULL, // ok

    .assignToDif = NULL,     // ok
    .updateDifConfig = NULL, // ok

    .connectionCreate = xNormalConnectionCreateRequest, // ok
    .connectionUpdate = NULL,                           // ok
    .connectionDestroy = NULL,                          // ok
    .connectionCreateArrived = NULL,                    // ok
    .connectionModify = NULL,                           // ok

    .duEnqueue = NULL, // ok
    .duWrite = NULL,   // ok

    .mgmtDuWrite = NULL, // ok
    .mgmtDuPost = NULL,  // ok

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

/* Called from the IPCP Task to the NormalIPCP register as APP into the SHIM DIF
 * depending on the Type of ShimDIF.*/
bool_t xNormalRegistering(ipcpInstance_t *pxShimInstance, name_t *pxDifName, name_t *pxName)
{

        if (pxShimInstance->pxOps->applicationRegister == NULL)
        {
                LOGI(TAG_IPCPNORMAL, "There is not Application Register API");
        }
        if (pxShimInstance->pxOps->applicationRegister(pxShimInstance->pxData, pxName, pxDifName))
        {
                LOGI(TAG_IPCPNORMAL, "Normal Instance Registered into the Shim");
                return true;
        }

        return false;
}

/* Normal IPCP request a Flow Allocation to the Shim */

#if 0
bool_t xNormalFlowAllocationRequest(ipcpInstance_t *pxInstanceFrom, ipcpInstance_t *pxInstanceTo, portId_t xShimPortId)
{
        /*This should be proposed by the Flow Allocator?*/
        name_t *destinationInfo = pvRsMemAlloc(sizeof(*destinationInfo));
        destinationInfo->pcProcessName = REMOTE_ADDRESS_AP_NAME;
        destinationInfo->pcEntityName = "";
        destinationInfo->pcProcessInstance = REMOTE_ADDRESS_AP_INSTANCE;
        destinationInfo->pcEntityInstance = "";

        if (pxInstanceTo->pxOps->flowAllocateRequest == NULL)
        {
                LOGI(TAG_IPCPNORMAL, "There is not Flow Allocate Request API");
        }
        if (pxInstanceTo->pxOps->flowAllocateRequest(xShimPortId,
                                                     pxInstanceFrom,
                                                     pxInstanceTo->pxData->pxName,
                                                     destinationInfo,
                                                     pxInstanceTo->pxData))
        {
                LOGI(TAG_IPCPNORMAL, "Flow Request Sended");
                return true;
        }

        return false;
}

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
