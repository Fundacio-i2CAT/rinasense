#include <stdio.h>
#include <string.h>

#include "portability/port.h"
#include "common/list.h"
#include "common/netbuf.h"
#include "common/rina_ids.h"
#include "common/rina_name.h"
#include "common/rsrc.h"

#include "configSensor.h"

#include "FlowAllocator_api.h"
#include "EFCP.h"
#include "efcpStructures.h"
#include "rmt.h"
#include "rina_common_port.h"
#include "du.h"
#include "pci.h"
#include "configRINA.h"
#include "IpcManager.h"
#include "FlowAllocator_defs.h"
#include "Ribd_defs.h"
#include "Ribd_api.h"
#include "IPCP.h"
#include "IPCP_api.h"
#include "IPCP_normal_defs.h"
#include "IPCP_normal_api.h"
#include "RINA_API_flows.h"

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
    flowAllocateHandle_t *pxFlowHandle;
    RsListItem_t xFlowListItem;
};

/* REVIEWED/REFACTORED CODE STARTS HERE */

static struct normalFlow_t *prvFindFlowCepid(struct ipcpInstanceData_t *pxData, cepId_t xCepId)
{
    struct normalFlow_t *pxFlow;
    RsListItem_t *pxListItem;

    if (unRsListLength(&(pxData->xFlowsList)) == 0) {
        LOGE(TAG_IPCPNORMAL, "Flow list is empty");
        return NULL;
    }

    /* Find a way to iterate in the list and compare the addesss*/
    pxListItem = pxRsListGetFirst(&(pxData->xFlowsList));

    while (pxListItem != NULL) {
        pxFlow = (struct normalFlow_t *)pxRsListGetItemOwner(pxListItem);

        if (!pxFlow)
            return NULL;

        if (pxFlow->xActive == xCepId)
            return pxFlow;

        pxListItem = pxRsListGetNext(pxListItem);
    }

    return NULL;
}

static struct normalFlow_t *prvNormalFindFlow(struct ipcpInstanceData_t *pxData,
                                              portId_t xPortId)
{
    struct normalFlow_t *pxFlow;
    RsListItem_t *pxListItem;

    /* Find a way to iterate in the list and compare the addesss*/
    pxListItem = pxRsListGetFirst(&pxData->xFlowsList);

    while (pxListItem != NULL) {
        pxFlow = (struct normalFlow_t *)pxRsListGetItemOwner(pxListItem);

        if (!pxFlow)
            return false;

        if (pxFlow->xPortId == xPortId)
            return pxFlow;

        pxListItem = pxRsListGetNext(pxListItem);
    }

    return NULL;
}


/* Correct implementation based on IRATI's kernel module */
bool_t xNormalDuEnqueue(struct ipcpInstance_t *pxIpcp,
                        portId_t xN1PortId,
                        du_t *pxDu)
{
    if (!xRmtReceive(&pxIpcp->pxData->xRmt, &pxIpcp->pxData->xEfcpContainer, pxDu, xN1PortId)) {
        LOGE(TAG_IPCPNORMAL, "Failed to enqueue the SDU to the RMT");
        return false;
    }

    return true;
}

cepId_t xNormalConnectionCreateRequest(struct ipcpInstance_t *pxIpcp,
                                       struct efcpContainer_t *pxEfcpc,
                                       portId_t xAppPortId,
                                       address_t xSource,
                                       address_t xDest,
                                       qosId_t xQosId,
                                       dtpConfig_t *pxDtpCfg,
                                       dtcpConfig_t *pxDtcpCfg)
{
    cepId_t xCepId;
    struct normalFlow_t *pxFlow;
    struct cepIdsEntry_t *pxCepEntry;

    RsAssert(pxIpcp);
    RsAssert(pxEfcpc);
    RsAssert(xAppPortId != PORT_ID_WRONG);
    RsAssert(xSource != ADDRESS_WRONG);
    RsAssert(xDest != ADDRESS_WRONG);
    RsAssert(xQosId != QOS_ID_WRONG);
    RsAssert(pxDtpCfg);
    RsAssert(pxDtcpCfg);

    xCepId = xEfcpConnectionCreate(pxEfcpc, xSource, xDest,
                                   xAppPortId, xQosId,
                                   CEP_ID_WRONG, CEP_ID_WRONG,
                                   pxDtpCfg, pxDtcpCfg);
    if (!is_cep_id_ok(xCepId)) {
        LOGE(TAG_IPCPNORMAL, "Failed EFCP connection creation");
        return CEP_ID_WRONG;
    }

    

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

bool_t xNormalFlowPrebind(struct ipcpInstance_t *pxIpcp,
                          flowAllocateHandle_t *pxFlowHandle)
{
    struct normalFlow_t *pxFlow;

    RsAssert(pxIpcp);
    RsAssert(pxFlowHandle);

    LOGI(TAG_IPCPNORMAL, "Binding the flow with port ID: %u", pxFlowHandle->xPortId);

    if (!(pxFlow = pvRsMemAlloc(sizeof(*pxFlow)))) {
        LOGE(TAG_IPCPNORMAL, "Could not create a flow in normal-ipcp to pre-bind");
        return false;
    }

    pxFlow->xPortId = pxFlowHandle->xPortId;
    pxFlow->eState = ePORT_STATE_PENDING;

    LOGD(TAG_IPCPNORMAL, "Flow: %p portID: %d portState: %d", pxFlow, pxFlow->xPortId, pxFlow->eState);

    vRsListInitItem(&(pxFlow->xFlowListItem), pxFlow);
    vRsListInsert(&(pxIpcp->pxData->xFlowsList), &(pxFlow->xFlowListItem));

    vRsListInit(&(pxFlow->xCepIdsList));

    return true;
}


/* Called from the IPCP Task to the NormalIPCP register as APP into the SHIM DIF
 * depending on the Type of ShimDIF.*/
bool_t xNormalRegistering(struct ipcpInstance_t *pxShimInstance,
                          rname_t *pxDifName,
                          rname_t *pxName)
{
    bool_t xStatus;

    CALL_IPCP_CHECK(xStatus, pxShimInstance, applicationRegister, pxName, pxDifName) {
        LOGE(TAG_IPCPNORMAL, "Failed to register normal IPC to shim ID %u", pxShimInstance->xId);
        return false;
    } else {
        LOGI(TAG_IPCPNORMAL, "Normal Instance Registered into the shim ID %u", pxShimInstance->xId);
        return true;
    }
}

bool_t xNormalDuWrite(struct ipcpInstance_t *pxIpcp,
                      portId_t xAppPortId,
                      du_t *pxDu)
{
    struct normalFlow_t *pxFlow;

    RsAssert(pxIpcp);
    RsAssert(is_port_id_ok(xAppPortId));
    RsAssert(pxDu);

    pxFlow = prvNormalFindFlow(pxIpcp->pxData, xAppPortId);

    if (!pxFlow || pxFlow->eState != ePORT_STATE_ALLOCATED) {
        LOGE(TAG_IPCPNORMAL, "Write: There is no flow bound to this port ID: %u", xAppPortId);
        return false;
    }

    if (!xEfcpContainerWrite(&pxIpcp->pxData->xEfcpContainer, pxFlow->xActive, pxDu)) {
        LOGE(TAG_IPCPNORMAL, "Could not send sdu to EFCP Container");
        return false;
    }

    return true;
}

static bool_t xNormalConnectionDestroy(struct ipcpInstanceData_t *pxData, cepId_t xSrcCepId)
{
    struct normalFlow_t *pxFlow;

    if (xEfcpConnectionDestroy(&pxData->xEfcpContainer, xSrcCepId))
        LOGE(TAG_EFCP, "Could not destroy EFCP instance: %d", xSrcCepId);

    /* FIXME: The condition below is always TRUE.
    // CRITICAL
    if (!(&pxIpcpData->xFlowsList))
    {
    // CRITICAL
    LOGE(TAG_EFCP, "Could not destroy EFCP instance: %d", xSrcCepId);
    return false;
    }
    */

    pxFlow = prvFindFlowCepid(pxData, xSrcCepId);
    if (!pxFlow) {
        // CRITICAL
        LOGE(TAG_IPCPNORMAL, "Could not retrieve flow by cep_id :%d", xSrcCepId);
        return false;
    }
        /*if (remove_cep_id_from_flow(flow, src_cep_id))
                LOG_ERR("Could not remove cep_id: %d", src_cep_id);

        if (list_empty(&flow->cep_ids_list))
        {
                list_del(&flow->list);
                rkfree(flow);
        }*/
        // CRITICAL

    return true;
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
bool_t xNormalFlowBinding(struct ipcpInstance_t *pxIpcp,
                          portId_t unPort,
                          struct ipcpInstance_t *pxN1Ipcp)
{
    RsAssert(pxIpcp);
    RsAssert(is_port_id_ok(unPort));
    RsAssert(pxN1Ipcp);

    return xRmtN1PortBind(&pxIpcp->pxData->xRmt, unPort, pxN1Ipcp);
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
bool_t xNormalMgmtDuWrite(struct ipcpInstance_t *pxIpcp, portId_t xPortId, du_t *pxDataDu)
{
    size_t sz;
    buffer_t xPci;
    du_t *pxDu;

    RsAssert(pxIpcp);
    RsAssert(is_port_id_ok(xPortId));
    RsAssert(pxDataDu);
    RsAssert(eNetBufType(pxDataDu) == NB_RINA_DATA);

    LOGI(TAG_IPCPNORMAL, "Passing SDU to be written to N-1 port %d ", xPortId);

    /* Allocate some memory for the PCI header */
    if (!(xPci = pxRsrcAlloc(pxIpcp->pxData->xPciPool, "Normal IPC Management PCI"))) {
        LOGE(TAG_IPCPNORMAL, "Failed to allocate memory for PCI");
        return false;
    }

    // pxDu->pxCfg = pxData->pxEfcpc->pxConfig;
    /* SET BUT NOT USED: sbytes = xDuLen(pxDu); */

    sz = unNetBufTotalSize(pxDataDu);

    if (!(pxDu = xDuEncap(xPci, sizeof(pci_t), pxDataDu))) {
        LOGE(TAG_IPCPNORMAL, "Failed to encap DU");
        return false;
    }

    /* Fill the PCI */
    PCI_SET(pxDu, PCI_VERSION, 0x01);
    PCI_SET(pxDu, PCI_CONN_SRC_ID, 0);
    PCI_SET(pxDu, PCI_CONN_DST_ID, 1);
    PCI_SET(pxDu, PCI_CONN_QOS_ID, 0);
    PCI_SET(pxDu, PCI_ADDR_DST, 0);
    PCI_SET(pxDu, PCI_FLAGS, 0);
    PCI_SET(pxDu, PCI_TYPE, PDU_TYPE_MGMT);
    PCI_SET(pxDu, PCI_SEQ_NO, 0);
    PCI_SET(pxDu, PCI_PDU_LEN, sz + sizeof(pci_t));
    PCI_SET(pxDu, PCI_ADDR_SRC, LOCAL_ADDRESS);

    // vPciPrint(pxDu->pxPci);

    // pxRmt = pxIpcpGetRmt();

    if (!xRmtSendPortId(&pxIpcp->pxData->xRmt, xPortId, pxDu)) {
        LOGE(TAG_IPCPNORMAL, "Could not sent to RMT");
        return false;
    }

    return true;
}

bool_t xNormalUpdateFlowStatus(struct ipcpInstance_t *pxIpcp, portId_t xPortId, eNormalFlowState_t eNewFlowstate)
{
    struct normalFlow_t *pxFlow = NULL;

    RsAssert(is_port_id_ok(xPortId));

    pxFlow = prvNormalFindFlow(pxIpcp->pxData, xPortId);
    if (!pxFlow) {
        LOGE(TAG_IPCPNORMAL, "Flow not found");
        return false;
    }

    pxFlow->eState = eNewFlowstate;
    LOGI(TAG_IPCPNORMAL, "Flow state updated");

    return true;
}

bool_t xNormalMgmtDuPost(struct ipcpInstance_t *pxIpcp, portId_t xPortId, du_t *pxDu)
{
    RsAssert(pxIpcp);
    RsAssert(is_port_id_ok(xPortId));
    RsAssert(pxDu);

    /* Send to the RIB Daemon */
    if (ERR_CHK(xRibIncoming(&pxIpcp->pxData->xRibd, pxDu, xPortId))) {
        LOGI(TAG_IPCPNORMAL, "Failed to preocess management PDU");
        return false;
    }

    return true;
}

#if 0
/* FIXME: UNSURE WHAT THIS IS FOR!! */
bool_t xNormalTest(struct ipcpInstance_t *pxNormalInstance, struct ipcpInstance_t *pxN1Ipcp)
{
    portId_t xId = 1;

    /* Data User */
    du_t *testDu;
    NetworkBufferDescriptor_t *pxNetworkBuffer;
    size_t xBufferSize;

    /* String to send*/
    char *ucStringTest = "Temperature:22";
    
    /*Getting the buffer Descriptor*/
    xBufferSize = strlen(ucStringTest);
    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(xBufferSize, 1000); // sizeof length DataUser packet.

    LOGI(TAG_IPCPNORMAL, "BufferSize DU: %zu", xBufferSize);

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
    if (xNormalDuWrite(pxNormalInstance, xId, testDu)) {
        LOGI(TAG_IPCPNORMAL, "Wrote packet on the shimWiFi");
        return true;
    }

    return false;
}
#endif

bool_t xNormalIsFlowAllocated(struct ipcpInstance_t *pxIpcp, portId_t xPortId)
{
    struct normalFlow_t *pxFlow = NULL;

    pxFlow = prvNormalFindFlow(pxIpcp->pxData, xPortId);
    if (!pxFlow) {
        LOGE(TAG_IPCPNORMAL, "Flow not found");
        return false;
    }

    if (pxFlow->eState == ePORT_STATE_ALLOCATED) {
        LOGI(TAG_IPCPNORMAL, "Flow status: Allocated");
        return true;
    }

    return false;
}

bool_t xNormalConnectionModify(struct ipcpInstance_t *pxIpcp,
                               cepId_t xCepId,
                               address_t xSrc,
                               address_t xDst)
{
    struct efcpContainer_t *pxEfcpContainer;

    if (!xEfcpConnectionModify(&pxIpcp->pxData->xEfcpContainer, xCepId, xSrc, xDst))
        return false;

    return true;
}

bool_t xNormalUpdateCepIdFlow(struct ipcpInstance_t *pxIpcp, portId_t xPortId, cepId_t xCepId)
{
    struct normalFlow_t *pxFlow = NULL;

    pxFlow = prvNormalFindFlow(pxIpcp->pxData, xPortId);
    if (!pxFlow) {
        LOGE(TAG_IPCPNORMAL, "Flow not found");
        return false;
    }

    pxFlow->xActive = xCepId;

    return true;
}

bool_t xNormalConnectionUpdate(struct ipcpInstance_t *pxIpcp,
                               portId_t xAppPortId,
                               cepId_t xSrcCepId,
                               cepId_t xDstCepId)
{
    struct efcpContainer_t *pxEfcpContainer;
    struct normalFlow_t *pxFlow = NULL;

    pxEfcpContainer = &pxIpcp->pxData->xEfcpContainer;

    if (!xEfcpConnectionUpdate(pxEfcpContainer,
                               xSrcCepId,
                               xDstCepId))
        return false;

    pxFlow = prvNormalFindFlow(pxIpcp->pxData, xAppPortId);
    if (!pxFlow) {
        LOGE(TAG_IPCPNORMAL, "Flow not found");
        return false;
    }
    pxFlow->eState = ePORT_STATE_ALLOCATED;
    LOGI(TAG_IPCPNORMAL, "Flow state updated");

    return true;
}

const rname_t *xNormalGetIpcpName(struct ipcpInstance_t *pxSelf)
{
    return &pxSelf->pxData->xName;
}

const rname_t *xNormalGetDifName(struct ipcpInstance_t *pxSelf)
{
    return &pxSelf->pxData->xDifName;
}

/* OLD CODE STARTS HERE. */


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

        if (pxInstanceTo->pxOps->
            flowAllocateRequest == NULL)
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

bool_t xNormalAppFlowAllocationRequestHandle(ipcpInstance_t)



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


static bool_t prvRemoveCepIdFromFlow(struct normalFlow_t *pxFlow,
                                     cepId_t xCepId)
{
#if 0
        

        ESP_LOGI(TAG_IPCPNORMAL, "Finding a Flow in the normal IPCP list");

        struct normalFlow_t *pxFlow;

        // shimFlow_t *pxFlowNext;

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        if (listLIST_IS_EMPTY(&(pxFlow->xCepIdsList)) == pdTRUE)
        {
                ESP_LOGI(TAG_IPCPNORMAL, "Flow CepIds list is empty");
                return NULL;
        }

        pxFlow = pvPortMalloc(sizeof(*pxFlow));

        /* Find a way to iterate in the list and compare the addesss*/
        //pxListEnd = listGET_END_MARKER(&(pxIpcpData->xFlowsList));
        //pxListItem = listGET_HEAD_ENTRY(&(pxIpcpData->xFlowsList));

        while (pxListItem != pxListEnd)
        {

                pxFlow = (struct normalFlow_t *)listGET_LIST_ITEM_OWNER(pxListItem);

                if (pxFlow == NULL)
                        return pdFALSE;

                if (!pxFlow)
                        return pdFALSE;

                if (pxFlow && pxFlow->xActive == xCepId)
                {

                        // ESP_LOGI(TAG_IPCPNORMAL, "Flow founded %p, portID: %d, portState:%d", pxFlow, pxFlow->xPortId, pxFlow->eState);
                        return pxFlow;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        ESP_LOGI(TAG_IPCPNORMAL, "Flow not founded");
        return NULL;

        struct cep_ids_entry *pos, *next;

        list_for_each_entry_safe(pos, next, &(flow->cep_ids_list), list)
        {
                if (pos->cep_id == id)
                {
                        list_del(&pos->list);
                        rkfree(pos);
                        return 0;
                }
        }
        return -1;
#endif
        return false;
}

static struct ipcpInstanceOps_t xNormalInstanceOps = {
    .flowAllocateRequest = NULL,
    .flowAllocateResponse = NULL,
    .flowDeallocate = NULL,
    .flowPrebind = xNormalFlowPrebind,
    .flowBindingIpcp = xNormalFlowBinding,
    .flowUnbindingIpcp = NULL,
    .flowUnbindingUserIpcp = NULL,
    .nm1FlowStateChange = NULL,

    .applicationRegister = NULL,
    .applicationUnregister = NULL,

    .assignToDif = NULL,
    .updateDifConfig = NULL,

    .connectionCreate = xNormalConnectionCreateRequest,
    .connectionUpdate = NULL,
    .connectionDestroy = NULL,
    .connectionCreateArrived = NULL,
    .connectionModify = NULL,

    .duEnqueue = xNormalDuEnqueue,
    .duWrite = NULL,

    .mgmtDuWrite = xNormalMgmtDuWrite,
    .mgmtDuPost = NULL,

    .pffAdd = NULL,
    .pffRemove = NULL,
    //.pff_dump                  = NULL,
    //.pff_flush                 = NULL,
    //.pff_modify		   		   = NULL,

    //.query_rib		  		   = NULL,

    .ipcpName = xNormalGetIpcpName,
    .difName = xNormalGetDifName,
    //.ipcp_id		  		   = NULL,

    //.set_policy_set_param      = NULL,
    //.select_policy_set         = NULL,
    //.update_crypto_state	   = NULL,
    //.address_change            = NULL,
    //.dif_name		   		   = NULL,
    .maxSduSize = NULL
};


struct ipcpInstance_t *pxNormalCreate(ipcProcessId_t unIpcpId)
{
    struct ipcpInstance_t *pxIpcp;
    struct ipcpInstanceData_t *pxData;
    struct rmt_t *pxRmt;
    struct efcpContainer_t *pxEfcp;
    flowAllocator_t *pxFA;

    pxIpcp = pvRsMemCAlloc(1, sizeof(struct ipcpInstance_t));
    pxData = pvRsMemCAlloc(1, sizeof(struct ipcpInstanceData_t));
    if (!pxIpcp || !pxData)
        goto fail;

    /* Memory pool for PCI objects. */
    if (!(pxData->xPciPool = pxRsrcNewPool("PCI pool", sizeof(pci_t), 1, 1, 0))) {
        LOGE(TAG_IPCPNORMAL, "Failed to allocate PCI object pool");
        goto fail;
    }

    /* Initialize the RIB */
    if (ERR_CHK(xRibNormalInit(&pxData->xRibd)))
        goto fail;

    /* Initialise Enrollment component */
    if (ERR_CHK(xEnrollmentInit(&pxData->xEnrollment, &pxData->xRibd)))
        goto fail;

    /* Initialize flow allocator */
    if (!xFlowAllocatorInit(&pxData->xFA, &pxData->xEnrollment, &pxData->xRibd)) {
        LOGE(TAG_IPCPNORMAL, "Failed initialisation of flow allocator");
        goto fail;
    }

    /* Initialize EFCP container */
    if (!xEfcpContainerInit(&pxData->xEfcpContainer, pxData->xPciPool)) {
        LOGE(TAG_IPCPNORMAL, "Failed initialisation of EFCP container");
        goto fail;
    }

    /* Initialize RMT */
    if (!xRmtInit(&pxData->xRmt)) {
        LOGE(TAG_IPCPNORMAL, "Failed initialisation of RMT component");
        goto fail;
    }

    vNameAssignFromPartsStatic(&pxData->xName,
                               NORMAL_PROCESS_NAME, NORMAL_PROCESS_INSTANCE,
                               NORMAL_ENTITY_NAME, NORMAL_ENTITY_INSTANCE);

    /* FIXME: THIS IS A TEMPORARY SUBSTITUTE FOR DIF ASSIGNATION. */
    vNameAssignFromPartsStatic(&pxData->xDifName,
                               NORMAL_DIF_NAME, "",
                               "", "");

#ifndef NDEBUG
    pxData->unInstanceDataType = IPCP_INSTANCE_DATA_NORMAL;
#endif
    pxData->xAddress = LOCAL_ADDRESS;
    pxData->pxIpcp = pxIpcp;

    /*Initialialise flows list*/
    vRsListInit(&(pxData->xFlowsList));

    pxIpcp->pxData = pxData;
    pxIpcp->xId = unIpcpId;
    pxIpcp->xType = eNormal;
    pxIpcp->pxOps = &xNormalInstanceOps;

    return pxIpcp;

    fail:
    /*vFlowAllocatorFini(&pxData->xFA);*/
    /*vEfcpContainerFini(&pxData->xEfcpContainer);*/
    vRmtFini(&pxData->xRmt);

    if (pxIpcp)
        vRsMemFree(pxIpcp);
    if (pxData)
        vRsMemFree(pxData);

    return NULL;
}
